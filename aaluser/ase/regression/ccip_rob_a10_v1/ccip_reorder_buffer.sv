/*
 * CCI-reordering Shim
 * DESCRIPTION: Sits on top of CCI-standard, and produces CCI-in-order transactions
 *
 *  ------               -----------             ---------
 *       |---unord_tx0-->|         |---ord_tx0-->|       |
 *       |<--unord_rx0---|   CCI   |<--ord_rx0---|       |
 *   QLP |               | reorder |             |  AFU  |
 *       |---unord_tx1-->|  shim   |---ord_tx1-->|       |
 *       |<--unord_rx0---|         |<--ord_rx0---|       |
 *  ------               -----------		 ---------
 *
 * Note:
 * - Write responses are not reordered
 * - Read Responses are reordered
 * - Module is meant to be transparent to SW
 *
 */

import ccip_if_pkg::*;

module ccip_reorder_buffer
  #(
    parameter CCIP_ROB_RADIX          = 8,
    parameter CCIP_TAG_WIDTH          = 16,
    parameter ROB_ALMFULL_GUARD_COUNT = 6
    )
   (
    // common clock
    input logic  clk,
    // Blue Bitstream Interface
    input logic  bb_softreset_n,
    output 	 t_if_ccip_Tx bb_tx,
    input 	 t_if_ccip_Rx bb_rx,
    // AFU interface
    output logic afu_softreset_n,
    input 	 t_if_ccip_Tx afu_tx,
    output 	 t_if_ccip_Rx afu_rx
    );

   localparam CCIP_ROB_DEPTH = 2**CCIP_ROB_RADIX;
   localparam ROB_ALMFULL_THRESHOLD = CCIP_ROB_DEPTH - ROB_ALMFULL_GUARD_COUNT;

   typedef enum  { RxPassThru, // Everything - MMIO, Umsg, WrResp
		  RxRdResp    // Read Response only (triggers when staged
		  } RxOutState;
   RxOutState rx0_state;

   // Pass through signals
   always @(posedge clk) begin
      afu_softreset_n = bb_softreset_n;
   end


   /* ****************************************************************
    * TX0 read request management
    * ****************************************************************/
   logic [0:CCIP_ROB_DEPTH-1] 			rdrsp_valid_array;

   logic 					rdtag_wren;
   logic [CCIP_TAG_WIDTH-1:0] 			rdtag_din;
   logic [CCIP_ROB_RADIX-1:0] 			rdtag_waddr;
   
   logic [CCIP_ROB_RADIX-1:0] 			rdtag_ctr;
   
   logic [(CCIP_RX_MEMHDR_WIDTH+CCIP_CLDATA_WIDTH-1):0] rddata_din;
   logic [CCIP_ROB_RADIX-1:0] 				rddata_waddr;
   logic 						rddata_wren;
   
   t_ccip_ReqMemHdr                             stg1_tx0hdr;
   logic 					stg1_tx0valid;

   logic [CCIP_ROB_RADIX-1:0] 			rdrsp_raddr;
   logic [CCIP_ROB_RADIX-1:0] 			rdrsp_raddr_q;
   logic 					rdrsp_rden;
   t_ccip_RspMemHdr                             rdrsp_hdr;
   logic [CCIP_RX_MEMHDR_WIDTH-1:0] 		rdrsp_hdrvec;
   logic [CCIP_CLDATA_WIDTH-1:0] 		rdrsp_data;
   logic [CCIP_TAG_WIDTH-1:0] 			rdrsp_mdata;
   logic 					rdrsp_valid;
   

   // Request Stage 1 - Store request, ctrtag is a counter controlled tag
   always @(posedge clk) begin
      stg1_tx0hdr             <= afu_tx.C0Hdr;
      stg1_tx0valid           <= afu_tx.C0RdValid;
      if (~bb_softreset_n) begin
	 rdtag_ctr               <= {CCIP_ROB_RADIX{1'b0}};
	 rdtag_wren              <= 0;
      end
      else if (afu_tx.C0RdValid) begin
	 rdtag_wren              <= 1;
	 rdtag_waddr             <= rdtag_ctr;
	 rdtag_ctr               <= rdtag_ctr + 1;
	 rdtag_din               <= afu_tx.C0Hdr.mdata;
	 stg1_tx0hdr.mdata       <= rdtag_ctr;
      end
      else begin
	 rdtag_wren              <= 0;
      end
   end

   // Request Stage 2 - Send request to BB
   always @(posedge clk) begin
      bb_tx.C0Hdr     <= stg1_tx0hdr;
      bb_tx.C0RdValid <= stg1_tx0valid;
   end

   // Tag RAM -- Store tags before sending out of TX0
   // AFU_TX-0 writes this buffer
   rob_gram_sdp #(
		  .ADDR_WIDTH (CCIP_ROB_RADIX),
		  .DATA_WIDTH (CCIP_TAG_WIDTH)
		  )
   read_tagram
     (
      .clk      ( clk         ),
      .write_en ( rdtag_wren  ),
      .waddr    ( rdtag_waddr ),
      .din      ( rdtag_din   ),
      .read_en  ( rdrsp_rden  ),
      .raddr    ( rdrsp_raddr ),
      .dout     ( rdrsp_mdata  )
      );

   // Read response storage
   // All other responses on CH0 is a pass thru
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 rddata_wren <= 0;
      end
      else if (bb_rx.C0RdValid) begin
	 rddata_wren  <= 1;
	 rddata_waddr <= bb_rx.C0Hdr.mdata[CCIP_ROB_RADIX-1:0];
	 rddata_din   <= {bb_rx.C0Hdr, bb_rx.C0Data};
      end
      else begin
	 rddata_wren <= 0;
      end
   end


   // Data RAM
   // BB_TX0 writes this buffer
   rob_gram_sdp #(
		  .ADDR_WIDTH (CCIP_ROB_RADIX),
		  .DATA_WIDTH (CCIP_RX_MEMHDR_WIDTH + CCIP_CLDATA_WIDTH)
		  )
   read_dram
     (
      .clk      ( clk ),
      .write_en ( rddata_wren  ),
      .waddr    ( rddata_waddr ),
      .din      ( rddata_din   ),
      .read_en  ( rdrsp_rden  ),
      .raddr    ( rdrsp_raddr ),
      .dout     ( {rdrsp_hdrvec, rdrsp_data}  )
      );

   assign rdrsp_hdr = t_ccip_RspMemHdr'(rdrsp_hdrvec);
      
   // RdValid indicator
   always @(posedge clk) begin
      rdrsp_valid <= rdrsp_rden;
   end

   // RdRsp valid array control
   // always @(posedge clk) begin
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 rdrsp_valid_array <= {CCIP_ROB_RADIX{1'b0}};
      end
      else begin
	 // Read Response pushed
	 if (rddata_wren) begin
	    rdrsp_valid_array[rddata_waddr] <= 1;
	 end
	 // Read Response popped
	 if (rdrsp_rden) begin
	    rdrsp_valid_array[rdrsp_raddr] <= 0;
	 end
      end
   end


   /* ****************************************************************
    * TX1 write request management
    * ****************************************************************/
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 bb_tx.C1Hdr         <= {CCIP_TX_MMIOHDR_WIDTH{1'b0}};
	 bb_tx.C1Data        <= {CCIP_MMIODATA_WIDTH{1'b0}};
	 bb_tx.C1WrValid     <= 1'b0;
	 bb_tx.C1IntrValid   <= 1'b0;
      end
      else begin
	 bb_tx.C1Hdr         <= afu_tx.C1Hdr;
	 bb_tx.C1Data        <= afu_tx.C1Data;
	 bb_tx.C1WrValid     <= afu_tx.C1WrValid;
	 bb_tx.C1IntrValid   <= afu_tx.C1IntrValid;
      end
   end

   /* ****************************************************************
    * TX2 write request management
    * ****************************************************************/
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 bb_tx.C2Hdr         <= {CCIP_TX_MMIOHDR_WIDTH{1'b0}};
	 bb_tx.C2MmioRdValid <= 0;
	 bb_tx.C2Data        <= {CCIP_MMIODATA_WIDTH{1'b0}};
      end
      else begin
	 bb_tx.C2Hdr         <= afu_tx.C2Hdr;
	 bb_tx.C2MmioRdValid <= afu_tx.C2MmioRdValid;
	 bb_tx.C2Data        <= afu_tx.C2Data;
      end
   end


   /* ****************************************************************
    * RX0 Response path management (ROB -> CAFU)
    * ****************************************************************/
   // sync FIFO signals
   logic [(5 + CCIP_RX_MEMHDR_WIDTH + CCIP_CLDATA_WIDTH-1):0] tx0fifo_dout;
   logic 						      tx0fifo_empty;
   logic 						      tx0fifo_read;
   logic 						      tx0fifo_pop;
   logic [2:0] 						      tx0fifo_count;
   logic 						      tx0fifo_valid;
   logic 						      tx0fifo_empty_q;
      
   // Synchronous FIFO
   // Stage incoming non-Read Response (SAFETY arrangement)
   sync_fifo
     #(
       .FIFO_DATA_WIDTH  (5 + CCIP_RX_MEMHDR_WIDTH + CCIP_CLDATA_WIDTH),
       .FIFO_DEPTH_RADIX (3)
       )
   tx0_non_rdrsp_fifo
     (
      .clock      (clk),
      .sclr       (~bb_softreset_n),
      .data       ({bb_rx.C0WrValid, bb_rx.C0RdValid, bb_rx.C0UMsgValid, bb_rx.C0MmioRdValid, bb_rx.C0MmioWrValid, bb_rx.C0Hdr, bb_rx.C0Data}),
      .wrreq      ( bb_rx.C0WrValid | bb_rx.C0UMsgValid | bb_rx.C0MmioRdValid | bb_rx.C0MmioWrValid ),
      .rdreq      (tx0fifo_pop),
      .q          (tx0fifo_dout),
      .usedw      (tx0fifo_count),
      .full       (),
      .empty      (tx0fifo_empty)
      );

   always @(posedge clk) begin
      tx0fifo_valid   <= tx0fifo_pop;
      tx0fifo_empty_q <= tx0fifo_empty;      
   end

   assign tx0fifo_pop = tx0fifo_read & ~tx0fifo_empty;
      
   /*
    * AFU_Rx CH0 control
    */
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
   	 afu_rx.C0Hdr         <= {CCIP_RX_MEMHDR_WIDTH{1'b0}};
   	 afu_rx.C0Data        <= {CCIP_CLDATA_WIDTH{1'b0}};
   	 afu_rx.C0WrValid     <= 0;
   	 afu_rx.C0RdValid     <= 0;
   	 afu_rx.C0UMsgValid   <= 0;
   	 afu_rx.C0MmioRdValid <= 0;
   	 afu_rx.C0MmioWrValid <= 0;
      end
      // else if (tx0fifo_valid) begin
      else if (tx0fifo_valid) begin
   	 { afu_rx.C0WrValid, afu_rx.C0RdValid, afu_rx.C0UMsgValid, afu_rx.C0MmioRdValid, afu_rx.C0MmioWrValid, afu_rx.C0Hdr, afu_rx.C0Data } <= tx0fifo_dout;
      end
      else if (rdrsp_valid) begin
	 // Header
      	 afu_rx.C0Hdr.vc_used   <= rdrsp_hdr.vc_used;
      	 afu_rx.C0Hdr.poison    <= rdrsp_hdr.poison   ;
      	 afu_rx.C0Hdr.hit_miss  <= rdrsp_hdr.hit_miss ;
      	 afu_rx.C0Hdr.fmt       <= rdrsp_hdr.fmt      ;
      	 afu_rx.C0Hdr.rsvd0     <= rdrsp_hdr.rsvd0    ;
      	 afu_rx.C0Hdr.cl_num    <= rdrsp_hdr.cl_num   ;
      	 afu_rx.C0Hdr.resp_type <= rdrsp_hdr.resp_type;
      	 afu_rx.C0Hdr.mdata     <= rdrsp_mdata;
	 // Rest of the signals
      	 afu_rx.C0Data        <= rdrsp_data;
      	 afu_rx.C0WrValid     <= 0;
      	 afu_rx.C0RdValid     <= 1;
      	 afu_rx.C0UMsgValid   <= 0;
      	 afu_rx.C0MmioRdValid <= 0;
      	 afu_rx.C0MmioWrValid <= 0;	 
      end
      else begin
   	 afu_rx.C0Hdr         <= {CCIP_RX_MEMHDR_WIDTH{1'b0}};
   	 afu_rx.C0Data        <= {CCIP_CLDATA_WIDTH{1'b0}};
   	 afu_rx.C0WrValid     <= 0;
   	 afu_rx.C0RdValid     <= 0;
   	 afu_rx.C0UMsgValid   <= 0;
   	 afu_rx.C0MmioRdValid <= 0;
   	 afu_rx.C0MmioWrValid <= 0;
      end
   end

   // synthesis translate_off
   always @(posedge clk) begin
      if (tx0fifo_read && rdrsp_rden)
	$display("*** ERROR: FIFO and RdRsp popped at same time"); 
   end
   // synthesis translate_on

   logic [CCIP_ROB_RADIX-1:0] next_rdpop_index;
      
   // Popping function
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
   	 rdrsp_raddr_q  <= 0;
   	 tx0fifo_read <= 0;
   	 rdrsp_rden   <= 0;	 
	 next_rdpop_index <= 0;	 
      end
      else if (~tx0fifo_empty) begin
      	 tx0fifo_read <= 1;	 
      	 rdrsp_rden   <= 0;	 
      end
      else if (rdrsp_valid_array[next_rdpop_index]) begin
      	 tx0fifo_read <= 0;
      	 rdrsp_rden   <= 1;
      	 rdrsp_raddr  <= next_rdpop_index;
	 next_rdpop_index <= next_rdpop_index + 1;	 
      end
      else begin
   	 tx0fifo_read <= 0;
   	 rdrsp_rden   <= 0;	 
      end      
   end
   
   /* ****************************************************************
    * RX1 Response path managment (ROB -> CAFU)
    * ****************************************************************/
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 afu_rx.C1Hdr       <= {CCIP_RX_MEMHDR_WIDTH{1'b0}};
	 afu_rx.C1WrValid   <= 0;
	 afu_rx.C1IntrValid <= 0;
      end
      else begin
	 afu_rx.C1Hdr       <= bb_rx.C1Hdr;
	 afu_rx.C1WrValid   <= bb_rx.C1WrValid;
	 afu_rx.C1IntrValid <= bb_rx.C1IntrValid;
      end
   end


   /* ****************************************************************
    * Almost full indicators
    * ****************************************************************/
   int 	rd_count;

   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 rd_count <= 0;
      end
      else begin
	 if      ( afu_tx.C0RdValid && ~afu_rx.C0RdValid) rd_count <= rd_count + 1;
	 else if ( afu_tx.C0RdValid &&  afu_rx.C0RdValid) rd_count <= rd_count;
	 else if (~afu_tx.C0RdValid && ~afu_rx.C0RdValid) rd_count <= rd_count;
	 else if (~afu_tx.C0RdValid &&  afu_rx.C0RdValid) rd_count <= rd_count - 1;
	 else                                             rd_count <= rd_count;
      end
   end

   // Almostfull
   always @(posedge clk) begin
      if (~bb_softreset_n) begin
	 afu_rx.C0TxAlmFull <= 1;
	 afu_rx.C1TxAlmFull <= 1;
      end
      else begin
	 afu_rx.C0TxAlmFull <= bb_rx.C0TxAlmFull | (rd_count > ROB_ALMFULL_THRESHOLD);
	 afu_rx.C1TxAlmFull <= bb_rx.C1TxAlmFull;
      end
   end

endmodule

