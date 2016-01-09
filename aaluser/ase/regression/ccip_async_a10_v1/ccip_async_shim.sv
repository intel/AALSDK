// Note the following assumptions
// - MMIO Read completion window is unaccounted for
// - Error state is TBD
// - PR state is TBD

import ccip_if_pkg::*;

module ccip_async_shim
  #(
    parameter DEPTH_RADIX = 8
    )
  (
   // Blue Bitstream Interface
   input logic 	      bb_softreset_n,
   input logic 	      bb_clk,
   output 	      t_if_ccip_Tx bb_tx,
   input 	      t_if_ccip_Rx bb_rx,
   // AFU interface
   output logic       afu_softreset_n,
   input logic 	      afu_clk,
   input 	      t_if_ccip_Tx afu_tx,
   output 	      t_if_ccip_Rx afu_rx,
   // Error vectors
   output logic [4:0] overflow,
   output logic [4:0] underflow
   );

   localparam TX0_REQ_TOTAL_WIDTH = 1 + $bits(t_ccip_ReqMemHdr) ;
   localparam TX1_REQ_TOTAL_WIDTH = 2 + $bits(t_ccip_ReqMemHdr) + CCIP_CLDATA_WIDTH;
   localparam TX2_REQ_TOTAL_WIDTH = 1 + $bits(t_ccip_Rsp_MmioHdr) + CCIP_MMIODATA_WIDTH;

   localparam RX0_RSP_TOTAL_WIDTH = 5 + $bits(t_ccip_RspMemHdr) + CCIP_CLDATA_WIDTH;
   localparam RX1_RSP_TOTAL_WIDTH = 2 + $bits(t_ccip_RspMemHdr);

   logic 	      tx0_fifo_full;
   logic 	      tx1_fifo_full;
      
// `define PASSTHRU_TEST   
`ifdef PASSTHRU_TEST
   
   assign afu_rx = bb_rx;
   assign bb_tx = afu_tx;
   
`else   

   // Almost full signals
   assign afu_rx.C0TxAlmFull = bb_rx.C0TxAlmFull & ~tx0_fifo_full;
   assign afu_rx.C1TxAlmFull = bb_rx.C1TxAlmFull & ~tx1_fifo_full;

   
   
   /*
    * AFIFO instantiations
    *
    * Interface | From |  To  | Transactions       | Notes
    * -----------------------------------------------------------------------------
    *   Tx0     | AFU  |  BB  | Read Request       |
    *   Tx1     | AFU  |  BB  | Write Request      |
    *   Tx2     | AFU  |  BB  | MMIO Read Response | MMIO Completion Window/Timeout
    *           |      |      |                    | are not accounted for
    *   Rx0     |  BB  | AFU  | Read Response
    *           |      |      | Write Response
    *           |      |      | UMsg
    *           |      |      | MMIO Request
    *   Rx1     |  BB  | AFU  | Write Response
    *
    */

   /***************************************************************/
   // Tx0 channel
   afifo_channel
     #(
       .DATA_WIDTH (TX0_REQ_TOTAL_WIDTH),
       .DEPTH_RADIX(DEPTH_RADIX)
       )
   tx0_fifo
     (
      .rst      ( ~bb_softreset_n ),
      .wrclk    ( afu_clk ),
      .rdclk    ( bb_clk ),
      .data_in  ( {afu_tx.C0Hdr, afu_tx.C0RdValid} ),
      .write_en ( afu_tx.C0RdValid ),
      .data_out ( {bb_tx.C0Hdr, bb_tx.C0RdValid} ),
      .data_vld (),
      .wrfull   ( tx0_fifo_full ),
      .overflow (overflow[0]),
      .underflow(underflow[0])
      );


   /***************************************************************/
   // Tx1 channel
   afifo_channel
     #(
       .DATA_WIDTH (TX1_REQ_TOTAL_WIDTH),
       .DEPTH_RADIX(DEPTH_RADIX)
       )
   tx1_fifo
     (
      .rst      ( ~bb_softreset_n ),
      .wrclk    ( afu_clk ),
      .rdclk    ( bb_clk ),
      .data_in  ( {afu_tx.C1Hdr, afu_tx.C1Data, afu_tx.C1WrValid, afu_tx.C1IntrValid} ),
      .write_en ( afu_tx.C1WrValid | afu_tx.C1IntrValid ),
      .data_out ( {bb_tx.C1Hdr, bb_tx.C1Data, bb_tx.C1WrValid, bb_tx.C1IntrValid} ),
      .data_vld (),
      .wrfull   ( tx1_fifo_full ),
      .overflow (overflow[1]),
      .underflow(underflow[1])
      );


   /***************************************************************/
   // Tx2 channel
   afifo_channel
     #(
       .DATA_WIDTH (TX2_REQ_TOTAL_WIDTH),
       .DEPTH_RADIX(DEPTH_RADIX)
       )
   tx2_fifo
     (
      .rst      ( ~bb_softreset_n ),
      .wrclk    ( afu_clk ),
      .rdclk    ( bb_clk ),
      .data_in  ( {afu_tx.C2Hdr, afu_tx.C2Data, afu_tx.C2MmioRdValid} ),
      .write_en ( afu_tx.C2MmioRdValid ),
      .data_out ( {bb_tx.C2Hdr, bb_tx.C2Data, bb_tx.C2MmioRdValid} ),
      .data_vld (),
      .wrfull   (),
      .overflow (overflow[2]),
      .underflow(underflow[2])
      );

   /***************************************************************/
   // Rx0 channel
   logic [CCIP_RX_MMIOHDR_WIDTH-1:0] afu_rx_C0Hdr_vec;
   logic [CCIP_RX_MMIOHDR_WIDTH-1:0] bb_rx_C0Hdr_vec;
   
   t_ccip_Req_MmioHdr DBG_CfgHdr;
   
   afifo_channel
     #(
       .DATA_WIDTH (RX0_RSP_TOTAL_WIDTH),
       .DEPTH_RADIX(DEPTH_RADIX)
       )
   rx0_fifo
     (
      .rst      ( ~bb_softreset_n ),
      .wrclk    ( bb_clk ),
      .rdclk    ( afu_clk ),
      .data_in  ( {bb_rx.C0WrValid, bb_rx.C0RdValid, bb_rx.C0UMsgValid, bb_rx.C0MmioRdValid, bb_rx.C0MmioWrValid, bb_rx.C0Data, bb_rx_C0Hdr_vec} ),
      .write_en ( bb_rx.C0WrValid | bb_rx.C0RdValid | bb_rx.C0UMsgValid | bb_rx.C0MmioRdValid | bb_rx.C0MmioWrValid ),
      .data_out ( {afu_rx.C0WrValid, afu_rx.C0RdValid, afu_rx.C0UMsgValid, afu_rx.C0MmioRdValid, afu_rx.C0MmioWrValid, afu_rx.C0Data, afu_rx_C0Hdr_vec} ),
      .data_vld (),
      .wrfull   (),
      .overflow (overflow[3]),
      .underflow(underflow[3])
      );

   assign afu_rx.C0Hdr = t_ccip_Req_MmioHdr'(afu_rx_C0Hdr_vec);
   assign bb_rx_C0Hdr_vec = t_ccip_Req_MmioHdr'(bb_rx.C0Hdr);
   
   assign DBG_CfgHdr = t_ccip_Req_MmioHdr'(afu_rx_C0Hdr_vec);
   
   
   /***************************************************************/
   // Rx1 channel
   afifo_channel
     #(
       .DATA_WIDTH (RX1_RSP_TOTAL_WIDTH),
       .DEPTH_RADIX(DEPTH_RADIX)
       )
   rx1_fifo
     (
      .rst      ( ~bb_softreset_n ),
      .wrclk    ( bb_clk ),
      .rdclk    ( afu_clk ),
      .data_in  ( {bb_rx.C1Hdr, bb_rx.C1WrValid, bb_rx.C1IntrValid } ),
      .write_en ( bb_rx.C1WrValid | bb_rx.C1IntrValid ),
      .data_out ( {afu_rx.C1Hdr, afu_rx.C1WrValid, afu_rx.C1IntrValid } ),
      .data_vld (),
      .wrfull   (),
      .overflow (overflow[4]),
      .underflow(underflow[4])
      );

   /***************************************************************/

   always @(posedge afu_clk) begin
      afu_softreset_n <= bb_softreset_n;
   end

   // synthesis translate_off
   always @(posedge bb_clk) begin
      if (overflow != 5'b0)
	$display("ERROR: %m: Possible data loss - overflow %b", overflow);
      if (underflow != 5'b0)
	$display("ERROR: %m: Possible data loss - underflow %b", underflow);      
   end
   // synthesis translate_on

`endif
   
endmodule
