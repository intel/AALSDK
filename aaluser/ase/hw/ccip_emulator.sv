/* ****************************************************************************
 * Copyright(c) 2011-2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither the name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * **************************************************************************
 *
 * Module Info: CCI Emulation top-level - SystemVerilog Module
 * Language   : System{Verilog}
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * MAJOR UPGRADES:
 * Wed Aug 10 22:17:28 PDT 2011   | Completed FIFO'ing all channels in all directions
 * Tue Jun 17 16:46:06 PDT 2014   | Started cleaning up code to add latency model
 *                                | Connect up new transactions CCI 1.8
 * Tue Dec 23 11:01:28 PST 2014   | Optimizing ASE for performance
 *                                | Added return path FIFOs for marshalling
 * Tue Oct 21 13:33:34 PDT 2015   | CCIP migration
 *
 */

import ase_pkg::*;
import ccip_if_pkg::*;

`include "platform.vh"

// CCI to Memory translator module
module ccip_emulator
  (
   // CCI-P Clocks and Resets
   output logic       pClk, // 400MHz - CCI-P clock domain. Primary interface clock
   output logic       pClkDiv2, // 200MHz - CCI-P clock domain.
   output logic       pClkDiv4, // 100MHz - CCI-P clock domain.
   // User clocks
   output logic       uClk_usr, // User clock domain. Refer to clock programming guide
   output logic       uClk_usrDiv2, // User clock domain. Half the programmed frequency
   // Power & error states
   output logic       pck_cp2af_softReset, // CCI-P ACTIVE HIGH Soft Reset
   output logic [1:0] pck_cp2af_pwrState, // CCI-P AFU Power State
   output logic       pck_cp2af_error, // CCI-P Protocol Error Detected
   // Interface structures
   output 	      t_if_ccip_Rx pck_cp2af_sRx, // CCI-P Rx Port
   input 	      t_if_ccip_Tx pck_af2cp_sTx  // CCI-P Tx Port
   );


   // Power and error state
   assign pck_cp2af_pwrState = 2'b0;
   assign pck_cp2af_error    = 1'b0;


   /*
    * CCIP breakout
    */
   // Clock/reset
   logic 			      Clk16UI ;
   logic 			      Clk32UI ;
   logic 			      Clk64UI ;
   logic 			      SoftReset;
   // Tx0 & bookkeeper
   ASETxHdr_t                         ASE_C0TxHdr;
   TxHdr_t                            C0TxHdr;
   logic 	                      C0TxValid;
   // Tx1 & bookkeeper
   ASETxHdr_t                         ASE_C1TxHdr;
   TxHdr_t                            C1TxHdr;
   logic [CCIP_DATA_WIDTH-1:0]        C1TxData;
   logic 		              C1TxValid;
   // Tx2
   MMIOHdr_t                          C2TxHdr;
   logic                              C2TxMmioRdValid;
   logic [CCIP_MMIO_RDDATA_WIDTH-1:0] C2TxData;
   // Rx0 & bookkeeper
   logic 			      C0RxMmioWrValid;
   logic 			      C0RxMmioRdValid;
   logic [CCIP_DATA_WIDTH-1:0] 	      C0RxData;
   RxHdr_t                            C0RxHdr;
   logic 			      C0RxRspValid;
   // Rx1 & bookkeeper
   RxHdr_t                            C1RxHdr;
   logic 			      C1RxRspValid;
   // Almost full signals
   logic 			      C0TxAlmFull;
   logic 			      C1TxAlmFull;


   /*
    * Local valid/debug breakout signals
    */
   logic 			      C0TxRdValid;
   logic 			      C1TxWrValid;
   logic 			      C0RxRdValid;
   logic 			      C0RxUMsgValid;
   logic 			      C1RxWrValid;
   logic 			      C1RxIntrValid;

   // Valid setting
   always @(*) begin
      // --------------------------------------------------------- //
      // Read Request
      if (C0TxValid &&
	  ((C0TxHdr.reqtype == ASE_RDLINE_S)||(C0TxHdr.reqtype == ASE_RDLINE_I)) )
	C0TxRdValid <= 1;
      else
	C0TxRdValid <= 0;
      // --------------------------------------------------------- //
      // Read Response
      // if (C0RxRspValid && (C0RxHdr.resptype == ASE_RD_RSP) )
      // 	C0RxRdValid <= 1;
      // else
      // 	C0RxRdValid <= 0;
      // --------------------------------------------------------- //
      // Write Request
      if (C1TxValid &&
	  ( (C1TxHdr.reqtype == ASE_WRFENCE)||(C1TxHdr.reqtype == ASE_WRLINE_I)||(C1TxHdr.reqtype == ASE_WRLINE_M)) )
	C1TxWrValid <= 1;
      else
	C1TxWrValid <= 0;
      // --------------------------------------------------------- //
      // Write response
      // if (C1RxRspValid && (C1RxHdr.resptype == ASE_WR_RSP) )
      // 	C1RxWrValid <= 1;
      // else
      // 	C1RxWrValid <= 0;
      // --------------------------------------------------------- //
      // Umsg response
      // if (C0RxRspValid && (C0RxHdr.resptype == ASE_UMSG))
      // 	C0RxUMsgValid <= 1;
      // else
      // 	C0RxUMsgValid <= 0;
      // --------------------------------------------------------- //
   end


   /*
    * Request/Response Type conversion functions
    */
   // ccip_tx0_to_ase_tx0: Convert from CCIP -> ASE Tx0
   function ASETxHdr_t ccip_tx0_to_ase_tx0(t_ccip_c0_ReqMemHdr inhdr);
      ASETxHdr_t txasehdr;
      begin
	 txasehdr.txhdr = TxHdr_t'(inhdr);
	 txasehdr.channel_id = 0;
	 case (inhdr.req_type)
	   eREQ_RDLINE_I : txasehdr.txhdr.reqtype = ASE_RDLINE_I;
	   eREQ_RDLINE_S : txasehdr.txhdr.reqtype = ASE_RDLINE_S;
	 endcase // case (inhdr.req_type)
	 return txasehdr;
      end
   endfunction

   // ccip_tx1_to_ase_tx1: Convert from CCIP -> ASE Tx1
   function ASETxHdr_t ccip_tx1_to_ase_tx1(t_ccip_c1_ReqMemHdr inhdr);
      ASETxHdr_t txasehdr;
      begin
	 txasehdr.txhdr = TxHdr_t'(inhdr);
	 txasehdr.channel_id = 1;
	 case (inhdr.req_type)
	   eREQ_WRLINE_I : txasehdr.txhdr.reqtype = ASE_WRLINE_I;
	   eREQ_WRLINE_M : txasehdr.txhdr.reqtype = ASE_WRLINE_M;
	   eREQ_WRFENCE  : txasehdr.txhdr.reqtype = ASE_WRFENCE;
	   eREQ_INTR     : txasehdr.txhdr.reqtype = ASE_INTR_REQ;
	 endcase // case (inhdr.req_type)
	 return txasehdr;
      end
   endfunction

   // ase_rx0_to_ccip_rx0: Convert from ASE -> CCIP RX0
   function t_ccip_c0_RspMemHdr ase_rx0_to_ccip_rx0(ASERxHdr_t inhdr);
      t_ccip_c0_RspMemHdr rxasehdr;
      begin
	 rxasehdr = RxHdr_t'(inhdr.rxhdr);
	 case (inhdr.rxhdr.resptype)
	   ASE_RD_RSP : rxasehdr.resp_type = eRSP_RDLINE;
	   ASE_UMSG   : rxasehdr.resp_type = eRSP_UMSG;
	 endcase
	 return rxasehdr;
      end
   endfunction

   // ase_rx1_to_ccip_rx1: Convert from ASE -> CCIP RX1
   function t_ccip_c1_RspMemHdr ase_rx1_to_ccip_rx1(ASERxHdr_t inhdr);
      t_ccip_c1_RspMemHdr rxasehdr;
      begin
	 rxasehdr = RxHdr_t'(inhdr.rxhdr);
	 case (inhdr.rxhdr.resptype)
	   ASE_WR_RSP      : rxasehdr.resp_type = eRSP_WRLINE;
	   ASE_WRFENCE_RSP : rxasehdr.resp_type = eRSP_WRFENCE;
	   ASE_INTR_RSP    : rxasehdr.resp_type = eRSP_INTR;
	 endcase
	 return rxasehdr;
      end
   endfunction


   // ASE's internal reset signal
   logic 			      sys_reset;

   /*
    * Remapping ASE CCIP to cvl_pkg struct
    */
   // Clocks 16ui, 32ui, 64ui
   assign pClk = Clk16UI;
   assign pClkDiv2 = Clk32UI;
   assign pClkDiv4 = Clk64UI;

   // Reset out
   assign pck_cp2af_softReset = SoftReset;

   // Rx/Tx mapping
   always @(*) begin
      // Rx OUT (CH0)
      pck_cp2af_sRx.c0.hdr         <= ase_rx0_to_ccip_rx0(t_ccip_c0_RspMemHdr'(C0RxHdr));
      pck_cp2af_sRx.c0.data        <= t_ccip_clData'(C0RxData);
      pck_cp2af_sRx.c0.rspValid    <= C0RxRspValid;
      pck_cp2af_sRx.c0.mmioRdValid <= C0RxMmioRdValid;
      pck_cp2af_sRx.c0.mmioWrValid <= C0RxMmioWrValid;
      // Rx OUT (CH1)
      pck_cp2af_sRx.c1.hdr         <= ase_rx1_to_ccip_rx1(t_ccip_c1_RspMemHdr'(C1RxHdr));
      pck_cp2af_sRx.c1.rspValid    <= C1RxRspValid;
      // Tx OUT (CH0)
      ASE_C0TxHdr                  <= ccip_tx0_to_ase_tx0( pck_af2cp_sTx.c0.hdr );
      C0TxHdr                      <= ASE_C0TxHdr.txhdr;
      C0TxValid                    <= pck_af2cp_sTx.c0.valid;
      // Tx OUT (CH1)
      ASE_C1TxHdr                  <= ccip_tx1_to_ase_tx1(pck_af2cp_sTx.c1.hdr);
      C1TxHdr                      <= ASE_C1TxHdr.txhdr;
      C1TxData                     <= pck_af2cp_sTx.c1.data;
      C1TxValid                    <= pck_af2cp_sTx.c1.valid;
      // Tx OUT (CH2)
      C2TxHdr                      <= MMIOHdr_t'(pck_af2cp_sTx.c2.hdr);
      C2TxData                     <= pck_af2cp_sTx.c2.data;
      C2TxMmioRdValid              <= pck_af2cp_sTx.c2.mmioRdValid;
      // Almost full signals
      pck_cp2af_sRx.c0TxAlmFull    <= C0TxAlmFull;
      pck_cp2af_sRx.c1TxAlmFull    <= C1TxAlmFull;
   end


   /*
    * DPI import/export functions
    */
   // Scope function
   import "DPI-C" function void scope_function();

   // ASE Initialize function
   import "DPI-C" context task ase_init();
   // Indication that ASE is ready
   import "DPI-C" function void ase_ready();

   // Global listener function
   import "DPI-C" context task ase_listener();

   // ASE config data exchange (read from ase.cfg)
   export "DPI-C" task ase_config_dex;

   // Unordered message dispatch
   export "DPI-C" task umsg_dispatch;

   // MMIO dispatch
   export "DPI-C" task mmio_dispatch;

   // Start simulation structures teardown
   import "DPI-C" context task start_simkill_countdown();
   // Signal to kill simulation
   export "DPI-C" task simkill;

   // Transaction count update ping/pong
   export "DPI-C" task count_error_flag_ping;
   import "DPI-C" function void count_error_flag_pong(int flag);

   // CONFIG, SCRIPT DEX operations
   import "DPI-C" function void sv2c_config_dex(string str);
   import "DPI-C" function void sv2c_script_dex(string str);

   // Data exchange for READ, WRITE system
   import "DPI-C" function void rd_memline_dex(inout cci_pkt foo );
   import "DPI-C" function void wr_memline_dex(inout cci_pkt foo );

   // MMIO response
   import "DPI-C" function void mmio_response(inout mmio_t mmio_pkt);
   mmio_t mmio_resp_pkt;

   // Software controlled process - run clocks
   export "DPI-C" task run_clocks;

   // Software controlled process - Run AFU Reset
   export "DPI-C" task afu_softreset_trig;

   // cci_logger buffer message
   export "DPI-C" task buffer_msg_inject;

   // Page table called status
   logic rd_memline_dex_called;   
   logic wr_memline_dex_called;   
   
   // Scope generator
   initial scope_function();


   /*
    * Multi-instance multi-user +CONFIG,+SCRIPT instrumentation
    * RUN =>
    * cd <work>
    * ./<simulator> +CONFIG=<path_to_cfg> +SCRIPT=<path_to_run_SEE_README>
    *
    */
   string config_filepath;
   string script_filepath;
`ifdef ASE_DEBUG
   initial begin
      if ($value$plusargs("CONFIG=%S", config_filepath)) begin
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("  [DEBUG]  Config = %s", config_filepath);
	 `END_YELLOW_FONTCOLOR;
      end
   end

   initial begin
      if ($value$plusargs("SCRIPT=%S", script_filepath)) begin
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("  [DEBUG]  Script = %s", script_filepath);
	 `END_YELLOW_FONTCOLOR;
      end
   end
`else
   initial $value$plusargs("CONFIG=%S", config_filepath);
   initial $value$plusargs("SCRIPT=%S", script_filepath);
`endif


   // Finish logger command
   int finish_logger = 0;


   /* ***************************************************************************
    * CCI signals declarations
    * ***************************************************************************
    *
    *                          -------------------
    *   tx0_header     ---61-->|                 |---18---> rx0_header
    *   tx0_valid      ------->|                 |---512--> rx0_data
    *   tx0_almostfull <-------|                 |--------> rx0_rdvalid
    *   tx1_header     ---61-->|      ASE        |--------> rx0_wrvalid
    *   tx1_data       --512-->|     BLOCK       |--------> rx0_cfgvalid
    *   tx1_valid      ------->|                 |--------> rx0_umsgvalid (TBD)
    *   tx1_almostfull <-------|                 |--------> rx0_intrvalid (TBD)
    *   tx1_intrvalid  ------->|                 |---18---> rx1_header
    *                          |                 |--------> rx1_intrvalid (TBD)
    *                          |                 |--------> rx1_wrvalid
    *                          |                 |--------> lp_initdone
    *                          |                 |--------> reset
    *                          |                 |--------> clk
    *                          -------------------
    *
    * ***************************************************************************/

   logic                          clk   ;

   // Internal 800 Mhz clock (for creating synchronized clocks)
   logic 			  Clk8UI;

   /*
    * Overflow/underflow signal checks
    */
   logic 			  tx0_underflow;
   logic 			  tx1_underflow;
   logic 			  tx0_overflow;
   logic 			  tx1_overflow;

   /*
    * State indicators
    */
   typedef enum 		  {
				   RxIdle,
				   RxMMIOForward,
				   RxUMsgForward,
				   RxReadResp,
				   RxWriteResp
				   } RxOutState;
   RxOutState rx0_state;
   RxOutState rx1_state;

   /*
    * Clock process: Operates the CAFU clock
    */
   logic [2:0] 			  ase_clk_rollover = 3'b111;

   // ASE clock
   assign clk = Clk16UI;
   assign Clk16UI = ase_clk_rollover[0];
   assign Clk32UI = ase_clk_rollover[1];
   assign Clk64UI = ase_clk_rollover[2];

   // 800 Mhz internal reference clock
   initial begin : clk8ui_proc
      begin
   	 Clk8UI = 0;
   	 forever begin
   	    #`CLK_8UI_TIME;
   	    Clk8UI = 0;
   	    #`CLK_8UI_TIME;
   	    Clk8UI = 1;
   	 end
      end
   end

   // 200 Mhz clock
   always @(posedge Clk8UI) begin : clk_rollover_ctr
      ase_clk_rollover	<= ase_clk_rollover - 1;
   end

   // Reset management
   logic 			  sw_reset_trig ;

   // AFU Soft Reset Trigger
   task afu_softreset_trig( int value );
      begin
	 sw_reset_trig = value;
      end
   endtask


   /*
    * AFU reset - software & system resets
    */
   //
   //       0        |     0               0     |
   //       1        |     0               1     |
   //       1        |     1               0     |
   //       1        |     1               1     | Initial reset
   assign SoftReset = sys_reset | sw_reset_trig;


   /* ******************************************************************
    *
    * run_clocks : Run 'n' clocks
    * Software controlled event trigger for watching signals
    *
    * *****************************************************************/
   task run_clocks (int num_clks);
      int clk_iter;
      begin
	 for (clk_iter = 0; clk_iter < num_clks; clk_iter = clk_iter + 1) begin
	    @(posedge clk);
	 end
      end
   endtask


   /* ***************************************************************************
    * Buffer message injection into ccip_logger
    * ---------------------------------------------------------------------------
    * Task sets buffer message to be posted into ccip_transactions.tsv log
    *
    * ***************************************************************************/
   string buffer_msg;
   logic  buffer_msg_en;

   // Inject task
   task buffer_msg_inject (string logstr);
      begin
	 buffer_msg = logstr;
	 buffer_msg_en = 1;
	 @(posedge clk);
	 buffer_msg_en = 0;
	 @(posedge clk);
      end
   endtask


   /* ******************************************************************
    * DUMMY BLOCK
    *
    * *****************************************************************/



   /* ******************************************************************
    *
    * MMIO block
    * CSR Write/Read is managed through this interface.
    *
    * *****************************************************************/
   // MMIO read tid counter
   logic [CCIP_MMIO_TID_WIDTH-1:0] 	     mmio_tid_counter;

   // TID:Address tuple storage
   int 					     unsigned tid_array[*];

   /*
    * CSR Read/Write infrastructure
    * csr_write_dispatch: A Single task to dispatch CSR Writes
    * Storage format = <wrvalid, rdvalid, hdr_width, data_width>
    *
    */
   parameter int 			     MMIOREQ_FIFO_WIDTH = 2 + CCIP_CFG_HDR_WIDTH + CCIP_DATA_WIDTH;

   logic [MMIOREQ_FIFO_WIDTH-1:0] 	     mmioreq_din;
   logic 				     mmioreq_write;
   logic 				     mmioreq_read;
   logic 				     mmioreq_valid;
   logic 				     mmioreq_full;
   logic 				     mmioreq_empty;
   logic [4:0] 				     mmioreq_count;

   logic [CCIP_CFG_HDR_WIDTH-1:0] 	     cwlp_header;
   logic [CCIP_DATA_WIDTH-1:0] 		     cwlp_data;
   logic 				     cwlp_wrvalid;
   logic 				     cwlp_rdvalid;

   logic 				     mmio_wrvalid;
   logic 				     mmio_rdvalid;
   logic [CCIP_DATA_WIDTH-1:0] 		     mmio_data512;
   logic [CCIP_CFG_HDR_WIDTH-1:0] 	     mmio_hdrvec;


   // MMIO dispatch unit
   task mmio_dispatch (int initialize, mmio_t mmio_pkt);
      CfgHdr_t hdr;
      begin
	 if (initialize) begin
	    cwlp_wrvalid = 0;
	    cwlp_rdvalid = 0;
	    cwlp_header  = 0;
	    cwlp_data    = 0;
	    mmio_tid_counter  = 0;
	 end
	 else begin
	    if (mmio_pkt.write_en == MMIO_WRITE_REQ) begin
	       hdr.index  = {2'b0, mmio_pkt.addr[15:2]};
	       hdr.poison = 1'b0;
	       hdr.tid    = 9'b0;
	       if (mmio_pkt.width == MMIO_WIDTH_32) begin
		  hdr.len = 2'b00;
		  cwlp_data = {480'b0, mmio_pkt.qword[0][31:0]};
	       end
	       else if (mmio_pkt.width == MMIO_WIDTH_64) begin
		  hdr.len = 2'b01;
		  cwlp_data = {448'b0, mmio_pkt.qword[0][63:0]};
	       end
	       cwlp_header = CCIP_CFG_HDR_WIDTH'(hdr);
	       cwlp_wrvalid = 1;
	       cwlp_rdvalid = 0;
	       mmio_pkt.resp_en = 1;
	       @(posedge clk);
	       cwlp_wrvalid = 0;
	       cwlp_rdvalid = 0;
	       run_clocks(`MMIO_WRITE_LATRANGE);
	    end
	    else if (mmio_pkt.write_en == MMIO_READ_REQ) begin
	       cwlp_data    = 0;
	       hdr.index    = {2'b0, mmio_pkt.addr[15:2]};
	       if (mmio_pkt.width == MMIO_WIDTH_32) begin
		  hdr.len      = 2'b00;
	       end
	       else if (mmio_pkt.width == MMIO_WIDTH_64) begin
		  hdr.len      = 2'b01;
	       end
	       hdr.poison   = 1'b0;
	       hdr.tid      = mmio_tid_counter;
	       cwlp_header  = CCIP_CFG_HDR_WIDTH'(hdr);
	       cwlp_wrvalid = 0;
	       cwlp_rdvalid = 1;
    	       mmio_tid_counter  = mmio_tid_counter + 1;
	       @(posedge clk);
	       cwlp_wrvalid = 0;
	       cwlp_rdvalid = 0;
	       mmio_resp_pkt = mmio_pkt;
	       run_clocks(`MMIO_READ_LATRANGE);
	    end
	 end
      end
   endtask

   // CSR readreq/write FIFO data
   assign mmioreq_din = {cwlp_wrvalid, cwlp_rdvalid, cwlp_header, cwlp_data};
   assign mmioreq_write = cwlp_wrvalid | cwlp_rdvalid;

   // Request staging
   ase_fifo
     #(
       .DATA_WIDTH     ( MMIOREQ_FIFO_WIDTH ),
       .DEPTH_BASE2    ( 4 ),
       .ALMFULL_THRESH ( 12 )
       )
   mmioreq_fifo
     (
      .clk        ( clk ),
      .rst        ( sys_reset ),
      .wr_en      ( mmioreq_write ),
      .data_in    ( mmioreq_din ),
      .rd_en      ( mmioreq_read & ~mmioreq_empty ),
      .data_out   ( {mmio_wrvalid, mmio_rdvalid, mmio_hdrvec, mmio_data512} ),
      .data_out_v ( mmioreq_valid ),
      .alm_full   ( mmioreq_full ),
      .full       (  ),
      .empty      ( mmioreq_empty ),
      .count      ( mmioreq_count ),
      .overflow   (  ),
      .underflow  (  )
      );

   // Debug config header
   CfgHdr_t DBG_cfgheader;
   logic DBG_cfgvld;
   assign DBG_cfgheader = CfgHdr_t'(C0RxHdr);
   assign DBG_cfgvld = C0RxMmioWrValid | C0RxMmioRdValid;


   /*
    * MMIO Read response
    */
   parameter int MMIORESP_FIFO_WIDTH = CCIP_MMIO_TID_WIDTH + CCIP_MMIO_RDDATA_WIDTH;

   logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_dout;
   // logic 			   mmioresp_write;
   logic 			   mmioresp_read;
   logic 			   mmioresp_valid;
   logic 			   mmioresp_full;
   logic 			   mmioresp_empty;

   // Response staging FIFO
   ase_fifo
     #(
       .DATA_WIDTH     ( MMIORESP_FIFO_WIDTH ),
       .DEPTH_BASE2    ( 3 ),
       .ALMFULL_THRESH ( 5 )
       )
   mmioresp_fifo
     (
      .clk        ( clk ),
      .rst        ( sys_reset ),
      .wr_en      ( C2TxMmioRdValid ),
      .data_in    ( {CCIP_MMIO_TID_WIDTH'(C2TxHdr), C2TxData} ),
      .rd_en      ( mmioresp_read & ~mmioresp_empty ),
      .data_out   ( mmioresp_dout ),
      .data_out_v ( mmioresp_valid ),
      .alm_full   ( mmioresp_full ),
      .full       (  ),
      .empty      ( mmioresp_empty ),
      .count      (  ),
      .overflow   (  ),
      .underflow  (  )
      );

   // MMIO Response mask (act by reference)
   function void mmio_rsp_mask( ref mmio_t mmio_in );
      begin
	 // Data
	 mmio_in.qword[0] = mmioresp_dout[CCIP_MMIO_RDDATA_WIDTH-1:0];
	 if (mmio_in.width == 32) begin
	    mmio_in.qword[0][63:32] = 32'b0;
	 end
	 mmio_in.qword[1] = 0;
	 mmio_in.qword[2] = 0;
	 mmio_in.qword[3] = 0;
	 mmio_in.qword[4] = 0;
	 mmio_in.qword[5] = 0;
	 mmio_in.qword[6] = 0;
	 mmio_in.qword[7] = 0;
	 // Response flag
	 mmio_in.resp_en  = 1;
	 // Return
      end
   endfunction

   // MMIO Response trigger
   always @(posedge clk) begin
      mmioresp_read <= ~mmioresp_empty;
   end

   // FIFO writes to memory
   always @(posedge clk) begin : dpi_mmio_response
      if (mmioresp_valid) begin
	 mmio_rsp_mask(mmio_resp_pkt);
   	 mmio_response ( mmio_resp_pkt );
      end
   end

   /*
    * MMIO timeout/exit process
    */ 
   int mmioread_timeout_cnt;
   logic mmioread_cycle;
   logic mmioread_cycle_q;
      
   // MMIO Read activity in progress
   always @(posedge clk) begin : mmiocycle_proc
      mmioread_cycle_q <= mmioread_cycle;      
      if (sys_reset) begin
	 mmioread_cycle <= 0;	 
      end
      else begin
	 if (C0RxMmioRdValid) begin
	    mmioread_cycle <= 1;	    
	 end
	 else if (C2TxMmioRdValid) begin
	    mmioread_cycle <= 0;	    
	 end
	 else begin
	    mmioread_cycle <= mmioread_cycle_q;
	 end	 
      end
   end
   
   // MMIO counter
   always @(posedge clk) begin : mmioread_timeout_ctr
      if (sys_reset) begin
	 mmioread_timeout_cnt <= 0;	 
      end
      else if (mmioread_cycle) begin
	 mmioread_timeout_cnt <= mmioread_timeout_cnt + 1;	 
      end
      else begin
	 mmioread_timeout_cnt <= 0;	 
      end	
   end

   // MMIO Timeout simkill
   always @(posedge clk) begin : mmio_timeout_simkill
      if (mmioread_timeout_cnt >= `MMIO_RESPONSE_TIMEOUT) begin
	 `BEGIN_RED_FONTCOLOR;	 
	 $display("SIM-SV: ASE timed out waiting for MMIO Read response to arrive !!");
	 $display("        Please check CCI-spec, MMIO Read responses must return in %d cycles\n", 
		  `MMIO_RESPONSE_TIMEOUT);
	 `END_RED_FONTCOLOR;  
	 start_simkill_countdown();
      end
   end
   
   
   /* ******************************************************************
    *
    * Unordered Messages Engine
    * umsg_dispatch: Single push process triggering UMSG machinery
    *
    * *****************************************************************/

   parameter int UMSG_FIFO_WIDTH = CCIP_RX_HDR_WIDTH + CCIP_DATA_WIDTH;

   UMsgHdr_t                   umsgfifo_hdr_in;
   logic [CCIP_DATA_WIDTH-1:0] umsgfifo_data_in;

   logic [CCIP_DATA_WIDTH-1:0]     umsgfifo_data_out;
   logic [ASE_UMSG_HDR_WIDTH-1:0] umsgfifo_hdrvec_out;
   // UMsgHdr_t                       umsgfifo_hdr_out;

   logic 		       umsgfifo_write;
//   logic 		       umsgfifo_pop;
   logic 		       umsgfifo_read;
   logic 		       umsgfifo_valid;
   logic 		       umsgfifo_full;
   logic 		       umsgfifo_empty;
   // logic 		       umsgfifo_overflow;
   // logic 		       umsgfifo_underflow;

   // Data store
   logic [CCIP_DATA_WIDTH-1:0] umsg_latest_data_array [0:NUM_UMSG_PER_AFU-1];

   // Umsg engine
   umsg_t umsg_array[NUM_UMSG_PER_AFU];

   // UMSG dispatch function
   task umsg_dispatch (int init, umsgcmd_t umsg_pkt);
      int ii;
      begin
	 if (init) begin
   	    for (ii = 0; ii < NUM_UMSG_PER_AFU; ii = ii + 1) begin
   	       umsg_latest_data_array[ii]   <= {CCIP_DATA_WIDTH{1'b0}};
	       umsg_array[ii].line_accessed <= 0;
	       umsg_array[ii].hint_enable   <= 0;
   	    end
	 end
	 else begin
	    umsg_array[ umsg_pkt.id ].line_accessed = 1;
	    umsg_array[ umsg_pkt.id ].hint_enable   = umsg_pkt.hint;
	    umsg_latest_data_array[umsg_pkt.id][  63:00  ] = umsg_pkt.qword[0] ;
	    umsg_latest_data_array[umsg_pkt.id][ 127:64  ] = umsg_pkt.qword[1] ;
	    umsg_latest_data_array[umsg_pkt.id][ 191:128 ] = umsg_pkt.qword[2] ;
	    umsg_latest_data_array[umsg_pkt.id][ 255:192 ] = umsg_pkt.qword[3] ;
	    umsg_latest_data_array[umsg_pkt.id][ 319:256 ] = umsg_pkt.qword[4] ;
	    umsg_latest_data_array[umsg_pkt.id][ 383:320 ] = umsg_pkt.qword[5] ;
	    umsg_latest_data_array[umsg_pkt.id][ 447:384 ] = umsg_pkt.qword[6] ;
	    umsg_latest_data_array[umsg_pkt.id][ 511:448 ] = umsg_pkt.qword[7] ;
	    run_clocks(1);
	    umsg_array[ umsg_pkt.id ].line_accessed = 0;
	 end
      end
   endtask

   // Umsg slot/hint selector
   int 			       umsg_data_slot;
   int 			       umsg_hint_slot;
   int 			       umsg_data_slot_old = 255;
   int 			       umsg_hint_slot_old = 255;

   // UMSG Hint-to-Data time emulator (toaster style)
   // New Umsg hints to same location are ignored
   // If Data is same, hints dont get generated
   genvar ii;
   generate
      for ( ii = 0; ii < NUM_UMSG_PER_AFU; ii = ii + 1 ) begin : umsg_engine

	 // State machine
	 always @(posedge clk) begin
	    if (sys_reset) begin
	       umsg_array[ii].hint_timer <= 0;
	       umsg_array[ii].data_timer <= 0;
	       umsg_array[ii].hint_ready <= 0;
	       umsg_array[ii].data_ready <= 0;
	       umsg_array[ii].state      <= UMsgIdle;
	    end
	    else begin
	       case (umsg_array[ii].state)
		 // Wait here until activated
		 UMsgIdle:
		   begin
		      umsg_array[ii].hint_ready <= 0;
		      umsg_array[ii].data_ready <= 0;
		      if (umsg_array[ii].line_accessed && umsg_array[ii].hint_enable) begin
			 umsg_array[ii].hint_timer <= $urandom_range(`UMSG_START2HINT_LATRANGE);
			 umsg_array[ii].data_timer <= 0;
			 umsg_array[ii].state      <= UMsgHintWait;
		      end
		      else if (umsg_array[ii].line_accessed && ~umsg_array[ii].hint_enable) begin
			 umsg_array[ii].hint_timer <= 0;
		 	 umsg_array[ii].data_timer <= $urandom_range(`UMSG_START2DATA_LATRANGE);
			 umsg_array[ii].state      <= UMsgDataWait;
		      end
		      else begin
			 umsg_array[ii].hint_timer <= 0;
			 umsg_array[ii].data_timer <= 0;
			 umsg_array[ii].state      <= UMsgIdle;
		      end
		   end

		 // Wait to send out hint, go to UMsgSendHint after t_hint ticks
		 UMsgHintWait:
		   begin
		      umsg_array[ii].hint_ready <= 0;
		      umsg_array[ii].data_ready <= 0;
		      if (umsg_array[ii].hint_timer <= 0) begin
			 umsg_array[ii].state      <= UMsgSendHint;
		      end
		      else begin
			 umsg_array[ii].hint_timer <= umsg_array[ii].hint_timer - 1;
			 umsg_array[ii].state      <= UMsgHintWait;
		      end
		   end

		 // Wait until hint popped by event queue
		 UMsgSendHint:
		   begin
		      // umsg_array[ii].hint_ready <= 1;
		      if (umsg_array[ii].hint_pop) begin
			 umsg_array[ii].hint_ready <= 0;
			 umsg_array[ii].data_timer <= $urandom_range(`UMSG_HINT2DATA_LATRANGE);
			 umsg_array[ii].state      <= UMsgDataWait;
		      end
		      else begin
			 umsg_array[ii].hint_ready <= 1;
			 umsg_array[ii].data_timer <= 0;
			 umsg_array[ii].state      <= UMsgSendHint;
		      end
		   end

		 // Wait to send out data, go to UMsgSendData after t_data ticks
		 UMsgDataWait:
		   begin
		      umsg_array[ii].hint_ready    <= 0;
		      umsg_array[ii].data_ready    <= 0;
		      if (umsg_array[ii].data_timer <= 0) begin
			 umsg_array[ii].state      <= UMsgSendData;
		      end
		      else begin
			 // umsg_array[ii].hint_ready <= 0;
			 // umsg_array[ii].data_ready <= 0;
			 umsg_array[ii].data_timer <= umsg_array[ii].data_timer - 1;
			 umsg_array[ii].state      <= UMsgDataWait;
		      end
		   end

		 // Wait until popped by event queue
		 UMsgSendData:
		   begin
		      if (umsg_array[ii].data_pop) begin
			 umsg_array[ii].hint_ready <= 0;
			 umsg_array[ii].data_ready <= 0;
			 umsg_array[ii].state      <= UMsgIdle;
		      end
		      else begin
			 umsg_array[ii].hint_ready <= 0;
			 umsg_array[ii].data_ready <= 1;
			 umsg_array[ii].state      <= UMsgSendData;
		      end
		   end

		 // lala-land
		 default:
		   begin
		      umsg_array[ii].hint_timer <= 0;
		      umsg_array[ii].data_timer <= 0;
		      umsg_array[ii].hint_ready <= 0;
		      umsg_array[ii].data_ready <= 0;
		      umsg_array[ii].state      <= UMsgIdle;
		   end
	       endcase
	    end
	 end

      end
   endgenerate


   // Find UMSG Hintable slot
   function int find_umsg_hint ();
      int ret_hint_slot;
      int slot;
      int start_iter;
      int end_iter;
      begin
	 start_iter = 0;
	 end_iter   = start_iter + NUM_UMSG_PER_AFU;
   	 ret_hint_slot = 255;
   	 for (slot = start_iter ; slot < end_iter ; slot = slot + 1) begin
   	    if (umsg_array[slot].hint_ready && ~umsg_array[slot].data_ready) begin
   	       ret_hint_slot = slot;
	       umsg_hint_slot_old = ret_hint_slot;
   	       break;
   	    end
   	 end
   	 return ret_hint_slot;
      end
   endfunction

   // Find UMSG Data slot to send
   function int find_umsg_data();
      int ret_data_slot;
      int slot;
      int start_iter;
      int end_iter;
      begin
	 start_iter = 0;
	 end_iter   = start_iter + NUM_UMSG_PER_AFU;
   	 ret_data_slot = 255;
   	 for (slot = start_iter ; slot < end_iter ; slot = slot + 1) begin
   	    if (umsg_array[slot].data_ready) begin
   	       ret_data_slot = slot;
	       umsg_data_slot_old = ret_data_slot;
   	       break;
   	    end
   	 end
   	 return ret_data_slot;
      end
   endfunction

   // Calculate slots for UMSGs
   always @(posedge clk) begin : umsg_slot_finder_proc
      umsg_data_slot = find_umsg_data();
      umsg_hint_slot = find_umsg_hint();
   end

   // Pop HINT/DATA
   typedef enum {UPopIdle, UPopHint, UPopData, UPopWait} UmsgPopStateMachine;
   UmsgPopStateMachine upop_state;

   always @(posedge clk) begin : umsgfifo_pop_write
      if (sys_reset) begin
   	 umsgfifo_hdr_in    <= {ASE_UMSG_HDR_WIDTH{1'b0}};
   	 umsgfifo_data_in   <= {UMSG_FIFO_WIDTH{1'b0}};
   	 umsgfifo_write     <= 0;
   	 for(int jj = 0; jj < NUM_UMSG_PER_AFU; jj = jj + 1) begin
   	    umsg_array[jj].hint_pop <= 1'b0;
   	    umsg_array[jj].data_pop <= 1'b0;
   	 end
	 upop_state <= UPopIdle;
      end
      else begin
	 case (upop_state)
	   UPopIdle:
	     begin
   		umsgfifo_write     <= 0;
   		for(int jj = 0; jj < NUM_UMSG_PER_AFU; jj = jj + 1) begin
   		   umsg_array[jj].hint_pop <= 1'b0;
   		   umsg_array[jj].data_pop <= 1'b0;
   		end
		if (~umsgfifo_full && (umsg_hint_slot != 255)) begin
		   upop_state <= UPopHint;
		end
   		else if (~umsgfifo_full && (umsg_data_slot != 255)) begin
		   upop_state <= UPopData;
		end
		else begin
		   upop_state <= UPopIdle;
		end
	     end

	   UPopHint:
	     begin
   		umsgfifo_hdr_in.poison              <= 1'b0;
   		umsgfifo_hdr_in.resp_type           <= ASE_UMSG;
   		umsgfifo_hdr_in.umsg_type           <= 1'b1;
   		umsgfifo_hdr_in.umsg_id             <= umsg_hint_slot;
   		umsgfifo_data_in                    <= {CCIP_DATA_WIDTH{1'b0}};
   		umsgfifo_write                      <= 1'b1;
   		umsg_array[umsg_hint_slot].hint_pop <= 1'b1;
		upop_state                          <= UPopWait;
	     end

	   UPopData:
	     begin
   		umsgfifo_hdr_in.poison              <= 1'b0;
   		umsgfifo_hdr_in.resp_type           <= ASE_UMSG;
   		umsgfifo_hdr_in.umsg_type           <= 1'b0;
   		umsgfifo_hdr_in.umsg_id             <= umsg_data_slot;
   		umsgfifo_data_in                    <= umsg_latest_data_array[umsg_data_slot];
   		umsgfifo_write                      <= 1'b1;
   		umsg_array[umsg_data_slot].data_pop <= 1'b1;
		upop_state                          <= UPopWait;
	     end

	   UPopWait:
	     begin
   		umsgfifo_write     <= 0;
   		for(int jj = 0; jj < NUM_UMSG_PER_AFU; jj = jj + 1) begin
   		   umsg_array[jj].hint_pop <= 1'b0;
   		   umsg_array[jj].data_pop <= 1'b0;
   		end
		upop_state         <= UPopIdle;
	     end

	   default:
	     begin
   		umsgfifo_write                      <= 1'b0;
   		for(int jj = 0; jj < NUM_UMSG_PER_AFU; jj = jj + 1) begin
   		   umsg_array[jj].hint_pop <= 1'b0;
   		   umsg_array[jj].data_pop <= 1'b0;
   		end
		upop_state <= UPopIdle;
	     end

	 endcase
      end
   end

   // UMSG events queue
   ase_fifo
     #(
       .DATA_WIDTH     (UMSG_FIFO_WIDTH),
       .DEPTH_BASE2    (4),
       .ALMFULL_THRESH (12)
       )
   umsg_fifo
     (
      .clk        ( clk ),
      .rst        ( sys_reset ),
      .wr_en      ( umsgfifo_write ),
      .data_in    ( { ASE_UMSG_HDR_WIDTH'(umsgfifo_hdr_in), umsgfifo_data_in} ),
      .rd_en      ( umsgfifo_read & ~umsgfifo_empty ),
      .data_out   ( { umsgfifo_hdrvec_out, umsgfifo_data_out} ),
      .data_out_v ( umsgfifo_valid ),
      .alm_full   ( umsgfifo_full ),
      .full       (  ),
      .empty      ( umsgfifo_empty ),
      .count      (  ),
      .overflow   (  ),
      .underflow  (  )
      );

   // assign umsgfifo_hdr_out = UmsgHdr_t'(umsgfifo_hdrvec_out);


   /* ******************************************************************
    *
    * Config data exchange - Supplied by ase.cfg
    * Configuration of ASE managed by a text file, modifiable runtime
    *
    * *****************************************************************/
   task ase_config_dex(ase_cfg_t cfg_in);
      begin
	 cfg.ase_mode                 = cfg_in.ase_mode         ;
	 cfg.ase_timeout              = cfg_in.ase_timeout      ;
	 cfg.ase_num_tests            = cfg_in.ase_num_tests    ;
	 cfg.enable_reuse_seed        = cfg_in.enable_reuse_seed;
	 cfg.enable_cl_view           = cfg_in.enable_cl_view   ;
	 cfg.phys_memory_available_gb = cfg_in.phys_memory_available_gb;
	 // cfg.num_umsg_log2         = cfg_in.num_umsg_log2    ;
	 // cfg.enable_capcm       = cfg_in.enable_capcm     ;
	 // cfg.memmap_sad_setting = cfg_in.memmap_sad_setting    ;
      end
   endtask


   /* ******************************************************************
    * Count transactions
    * Live count of transactions to be printed at end of simulation
    *
    * ******************************************************************/
   int ase_rx0_mmiowrreq_cnt ;
   int ase_rx0_mmiordreq_cnt ;
   int ase_tx2_mmiordrsp_cnt ;
   int ase_tx0_rdvalid_cnt   ;
   int ase_rx0_rdvalid_cnt   ;
   int ase_tx1_wrvalid_cnt   ;
   // int ase_rx0_wrvalid_cnt   ;
   int ase_rx1_wrvalid_cnt   ;
   int ase_tx1_wrfence_cnt   ;
   int ase_rx1_wrfence_cnt   ;
   int ase_rx0_umsghint_cnt  ;
   int ase_rx0_umsgdata_cnt  ;

   // Remap UmsgHdr for count purposes
   UMsgHdr_t ase_umsghdr_map;
   assign ase_umsghdr_map = UMsgHdr_t'(C0RxHdr);

   // process
   always @(posedge clk) begin : transact_cnt_proc
      if (sys_reset) begin
	 ase_rx0_mmiowrreq_cnt <= 0 ;
	 ase_rx0_mmiordreq_cnt <= 0 ;
	 ase_tx2_mmiordrsp_cnt <= 0 ;
	 ase_tx0_rdvalid_cnt <= 0 ;
	 ase_rx0_rdvalid_cnt <= 0 ;
	 ase_tx1_wrvalid_cnt <= 0 ;
	 // ase_rx0_wrvalid_cnt <= 0 ;
	 ase_rx1_wrvalid_cnt <= 0 ;
	 ase_tx1_wrfence_cnt <= 0 ;
	 ase_rx1_wrfence_cnt <= 0 ;
	 ase_rx0_umsghint_cnt <= 0 ;
	 ase_rx0_umsgdata_cnt <= 0 ;
      end
      else begin
	 // MMIO counts
	 if (C0RxMmioWrValid)
	   ase_rx0_mmiowrreq_cnt = ase_rx0_mmiowrreq_cnt + 1;
	 if (C0RxMmioRdValid)
	   ase_rx0_mmiordreq_cnt = ase_rx0_mmiordreq_cnt + 1;
	 if (C2TxMmioRdValid)
	   ase_tx2_mmiordrsp_cnt = ase_tx2_mmiordrsp_cnt + 1;
	 // Read counts
	 if (C0TxRdValid)
	   ase_tx0_rdvalid_cnt = ase_tx0_rdvalid_cnt + (C0TxHdr.len + 1);
	 if (C0RxRdValid)
	   ase_rx0_rdvalid_cnt = ase_rx0_rdvalid_cnt + 1;
	 // Write counts
	 if (C1TxWrValid && (C1TxHdr.reqtype != ASE_WRFENCE))
	   ase_tx1_wrvalid_cnt = ase_tx1_wrvalid_cnt + 1;
	 if (C1RxWrValid && (C1RxHdr.resptype == ASE_WR_RSP) )
	   ase_rx1_wrvalid_cnt = ase_rx1_wrvalid_cnt + 1;
	 if (C1TxWrValid && (C1TxHdr.reqtype == ASE_WRFENCE))
	   ase_tx1_wrfence_cnt = ase_tx1_wrfence_cnt + 1;
	 if (C1RxWrValid && (C1RxHdr.resptype == ASE_WRFENCE_RSP))
	   ase_rx1_wrfence_cnt = ase_rx1_wrfence_cnt + 1;	 
	 // UMsg counts
	 if (C0RxUMsgValid && ase_umsghdr_map.umsg_type )
	   ase_rx0_umsghint_cnt = ase_rx0_umsghint_cnt + 1;
	 if (C0RxUMsgValid && ~ase_umsghdr_map.umsg_type )
	   ase_rx0_umsgdata_cnt = ase_rx0_umsgdata_cnt + 1;
      end
   end


   /*
    * Count error flag
    */
   int count_error_flag;
   always @(posedge clk) begin
      if (sys_reset) begin
	 count_error_flag <= 0;
      end
      else begin
	 if (ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt)
	   count_error_flag <= 1;
	 else if (ase_tx1_wrvalid_cnt != ase_rx1_wrvalid_cnt)
	   count_error_flag <= 1;
	 else if (ase_tx2_mmiordrsp_cnt != ase_rx0_mmiordreq_cnt)
	   count_error_flag <= 1;
	 else if (ase_tx1_wrfence_cnt != ase_rx1_wrvalid_cnt)
	   count_error_flag <= 1;
	 else
	   count_error_flag <= 0;
      end
   end // always @ (posedge clk)

   
   // Ping to get error flag
   task count_error_flag_ping();
      begin
	 count_error_flag_pong(count_error_flag);
      end
   endtask


   /* *******************************************************************
    *
    * Unified message watcher daemon
    * - Looks for MMIO Requests, buffer requests
    *
    * *******************************************************************/
   always @(posedge clk) begin : daemon_proc
      ase_listener();
   end


   /* *******************************************************************
    *
    * TX to RX channel FULFILLMENT
    *
    * -------------------------------------------------------------------
    * stg0       | stg1       | stg2
    * -------------------------------------
    * latbuf_out | cast & DEX | Response
    *            | tx_pkt     | tx_pkt_q
    *
    * *******************************************************************/
   // Read response staging signals
   logic [CCIP_DATA_WIDTH-1:0] rdrsp_data_in, rdrsp_data_out;
   RxHdr_t                     rdrsp_hdr_in, rdrsp_hdr_out;
   logic 		       rdrsp_write;
   logic 		       rdrsp_read;
   logic 		       rdrsp_full;
   logic 		       rdrsp_empty;
   logic 		       rdrsp_valid;

   // Write response 1 staging signals
   RxHdr_t                     wr1rsp_hdr_in, wr1rsp_hdr_out;
   logic 		       wr1rsp_write;
   logic 		       wr1rsp_read;
   logic 		       wr1rsp_full;
   logic 		       wr1rsp_empty;
   logic 		       wr1rsp_valid;

   // Declare packets for each channel
   cci_pkt Tx0toRx0_pkt;
   //   cci_pkt Tx1toRx0_pkt;
   cci_pkt Tx1toRx1_pkt;
   cci_pkt Tx0toRx0_pkt_q;
   // cci_pkt Tx1toRx0_pkt_q;
   // cci_pkt Tx1toRx1_pkt_q;

   logic Tx0toRx0_pkt_vld;
   // logic Tx1toRx0_pkt_vld;
   logic Tx1toRx1_pkt_vld;
   // logic Tx0toRx0_pkt_vld_q;
   // logic Tx1toRx0_pkt_vld_q;
   // logic Tx1toRx1_pkt_vld_q;


   /*
    * FUNCTION: Cast TxHdr_t to cci_pkt
    */
   function automatic void cast_txhdr_to_ccipkt (ref   cci_pkt               pkt,
						 input int                   write_en,
						 input TxHdr_t               txhdr,
						 input [CCIP_DATA_WIDTH-1:0] txdata);
      begin
	 // Write fence
	 if (txhdr.reqtype == ASE_WRFENCE)
	   pkt.wrfence  = 1;
	 else
	   pkt.wrfence  = 0;	 
	 // Write enable
	 pkt.write_en = int'(write_en);
	 // Metadata
	 pkt.vc       = int'(txhdr.vc);
	 pkt.mdata    = int'(txhdr.mdata);
	 // cache line address
	 pkt.cl_addr  = longint'(txhdr.addr);
	 // Qword assignment
	 pkt.qword[0] =  txdata[  63:00 ];
	 pkt.qword[1] =  txdata[ 127:64  ];
	 pkt.qword[2] =  txdata[ 191:128 ];
	 pkt.qword[3] =  txdata[ 255:192 ];
	 pkt.qword[4] =  txdata[ 319:256 ];
	 pkt.qword[5] =  txdata[ 383:320 ];
	 pkt.qword[6] =  txdata[ 447:384 ];
	 pkt.qword[7] =  txdata[ 511:448 ];
	 // Response enable
	 pkt.resp_en  = 0;
	 // Response channel
	 if (write_en) begin
	    pkt.resp_channel = 1;	    
// wrresp_tx2rx_chsel();
	 end
	 else begin
	    pkt.resp_channel = 0;
	 end
      end
   endfunction


   // rdreq/wrreq checker
   // logic rdreq_flag;
   // logic wrreq_flag;

   // always @(*) begin 
   //    if (sys_reset) begin
   // 	 rdreq_flag <= 0;
   // 	 wrreq_flag <= 0;
   //    end
   //    else begin
   // 	 // RdReq type
   // 	 case (cf2as_latbuf_tx0hdr.reqtype)
   // 	   ASE_RDLINE_S : rdreq_flag = 1;
   // 	   ASE_RDLINE_I : rdreq_flag = 1;
   // 	   default           : rdreq_flag = 0;
   // 	 endcase
   // 	 // WrReq type
   // 	 case (cf2as_latbuf_tx1hdr.reqtype)
   // 	   ASE_WRLINE_I : wrreq_flag = 1;
   // 	   ASE_WRLINE_M : wrreq_flag = 1;
   // 	   default           : wrreq_flag = 0;
   // 	 endcase
   //    end
   // end

   // cf2as_latbuf_ch0 signals
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_tx0hdr_vec;
   TxHdr_t                       cf2as_latbuf_tx0hdr;
   RxHdr_t                       cf2as_latbuf_rx0hdr;
   logic                         cf2as_latbuf_ch0_empty;
   logic                         cf2as_latbuf_ch0_read;
   int 				 cf2as_latbuf_ch0_count;
   // logic 			 cf2as_latbuf_ch0_pop;

   // cf2as_latbuf_ch1 signals
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_tx1hdr_vec;
   logic [CCIP_DATA_WIDTH-1:0]   cf2as_latbuf_tx1data;
   TxHdr_t                       cf2as_latbuf_tx1hdr;
   RxHdr_t                       cf2as_latbuf_rx1hdr;
   logic 		         cf2as_latbuf_ch1_empty;
   logic 		         cf2as_latbuf_ch1_read;
   int 				 cf2as_latbuf_ch1_count;
   logic 			 cf2as_latbuf_ch1_valid;
   // logic 			 cf2as_latbuf_ch1_pop;

   RxHdr_t                       cf2as_latbuf_rx0hdr_q;
   RxHdr_t                       cf2as_latbuf_rx1hdr_q;

   /*
    * CAFU->ASE CH0 (TX0)
    * Formed as {TxHdr_t}
    * Latency scoreboard (for latency modeling and shuffling)
    */
   outoforder_wrf_channel
     #(
       .DEBUG_LOGNAME       ("latbuf_ch0.log"),
       .NUM_WAIT_STATIONS   (LATBUF_NUM_TRANSACTIONS),
       .COUNT_WIDTH         (LATBUF_COUNT_WIDTH),
       .UNROLL_ENABLE       (1)
       )
   cf2as_latbuf_ch0
     (
      .clk		( clk ),
      .rst		( sys_reset ),
      .hdr_in		( C0TxHdr ),
      .data_in		( {CCIP_DATA_WIDTH{1'b0}} ),
      .write_en		( C0TxRdValid ),
      .txhdr_out	( cf2as_latbuf_tx0hdr ),
      .rxhdr_out        ( cf2as_latbuf_rx0hdr ),
      .data_out		(  ),
      .valid_out	( cf2as_latbuf_ch0_valid ),
      .read_en		( cf2as_latbuf_ch0_read ),
      .empty		( cf2as_latbuf_ch0_empty ),
      .full             ( C0TxAlmFull )
      );

   // assign cf2as_latbuf_ch0_pop = ~cf2as_latbuf_ch0_empty && cf2as_latbuf_ch0_read;

   // always @(posedge clk) begin
   //    if (sys_reset) begin
   // 	 cf2as_latbuf_ch0_read <= 0;
   //    end
   // end

   // Read TX0
   always @(posedge clk) begin
      if (~cf2as_latbuf_ch0_empty && ~rdrsp_full) begin
	 cf2as_latbuf_ch0_read <= 1;
      end
      else begin
	 cf2as_latbuf_ch0_read <= 0;
      end
   end

   // Tx0 process
   always @(posedge clk) begin
      if (sys_reset) begin
   	 Tx0toRx0_pkt_vld <= 0;
	 rd_memline_dex_called <= 0;	 
      end
      else if (cf2as_latbuf_ch0_valid) begin
   	 cast_txhdr_to_ccipkt( Tx0toRx0_pkt,
   			       0,
   			       cf2as_latbuf_tx0hdr,
   			       {CCIP_DATA_WIDTH{1'b0}} );
	 rd_memline_dex(Tx0toRx0_pkt);
	 Tx0toRx0_pkt_vld <= cf2as_latbuf_ch0_valid;
	 cf2as_latbuf_rx0hdr_q <= cf2as_latbuf_rx0hdr;
	 rd_memline_dex_called <= 1;	 
      end
      else begin
   	 Tx0toRx0_pkt_vld <= 0;
	 rd_memline_dex_called <= 0;	 
      end
   end

   always @(posedge clk) begin
      Tx0toRx0_pkt_q <= Tx0toRx0_pkt;
   end

   // RdRsp in
   always @(posedge clk) begin
      if (sys_reset) begin
   	 rdrsp_data_in <= {CCIP_DATA_WIDTH{1'b0}};
   	 rdrsp_hdr_in <= {CCIP_RX_HDR_WIDTH{1'b0}};
   	 rdrsp_write <= 0;
      end
      else begin
   	 rdrsp_data_in         <= unpack_ccipkt_to_vector(Tx0toRx0_pkt_q);
   	 rdrsp_hdr_in          <= cf2as_latbuf_rx0hdr_q;
   	 rdrsp_write           <= Tx0toRx0_pkt_vld;
      end
   end



   /*
    * CAFU->ASE CH1 (TX1)
    * Formed as {TxHdr_t, <data_512>}
    * Latency scoreboard (latency modeling and shuffling)
    */
   outoforder_wrf_channel
     #(
       .DEBUG_LOGNAME       ("latbuf_ch1.log"),
       .NUM_WAIT_STATIONS   (LATBUF_NUM_TRANSACTIONS),
       .COUNT_WIDTH         (LATBUF_COUNT_WIDTH),
       .UNROLL_ENABLE       (0)
       )
   cf2as_latbuf_ch1
     (
      .clk		( clk ),
      .rst		( sys_reset ),
      .hdr_in		( C1TxHdr ),
      .data_in		( C1TxData ),
      .write_en		( C1TxWrValid ),
      .txhdr_out	( cf2as_latbuf_tx1hdr ),
      .rxhdr_out        ( cf2as_latbuf_rx1hdr ),
      .data_out		( cf2as_latbuf_tx1data ),
      .valid_out	( cf2as_latbuf_ch1_valid),
      .read_en		( cf2as_latbuf_ch1_read  ),
      .empty		( cf2as_latbuf_ch1_empty ),
      .full             ( C1TxAlmFull )
      );


   //assign cf2as_latbuf_tx1hdr = TxHdr_t'(cf2as_latbuf_tx1hdr_vec);
   // assign cf2as_latbuf_ch1_pop = ~cf2as_latbuf_ch1_empty && cf2as_latbuf_ch1_read;

   // TX-CH1 must select RX-CH0 or RX-CH1 channels for fulfillment
   // Since requests on TX1 can return either via RX0 or RX1, this is needed
   // function automatic int wrresp_tx2rx_chsel();
   //    begin
   // 	 // return 0;
   // 	 return 1;
   // 	 // return (abs_val($random) % 2);
   //    end
   // endfunction

   // Read TX1
   always @(posedge clk) begin
      // if (~cf2as_latbuf_ch1_empty && ~wr0rsp_full && ~wr1rsp_full) begin
      if (~cf2as_latbuf_ch1_empty && ~wr1rsp_full) begin
	 cf2as_latbuf_ch1_read <= 1;
      end
      else begin
	 cf2as_latbuf_ch1_read <= 0;
      end
   end
   
   // TX1 process
   always @(posedge clk) begin
      if (sys_reset) begin
	 Tx1toRx1_pkt_vld <= 0;
	 wr_memline_dex_called <= 0;	 
      end
      else if (cf2as_latbuf_ch1_valid) begin
   	 cast_txhdr_to_ccipkt(Tx1toRx1_pkt, 
			      1, 
			      cf2as_latbuf_tx1hdr, 
			      cf2as_latbuf_tx1data);
   	 wr_memline_dex(Tx1toRx1_pkt);
   	 Tx1toRx1_pkt_vld <= cf2as_latbuf_ch1_valid;
	 cf2as_latbuf_rx1hdr_q <= cf2as_latbuf_rx1hdr;
	 wr_memline_dex_called <= 1;	 
      end
      else begin
	 Tx1toRx1_pkt_vld <= 0;
	 wr_memline_dex_called <= 0;	 	 
      end
   end

   // always @(posedge clk) begin
   //    Tx1toRx1_pkt_q <= Tx1toRx1_pkt;      
   // end
   
   // Wr1Rsp_in
   always @(posedge clk) begin
      if (sys_reset) begin
   	 wr1rsp_hdr_in  <= {CCIP_RX_HDR_WIDTH{1'b0}};
   	 wr1rsp_write   <= 0;
      end
      else begin
	 wr1rsp_hdr_in  <= cf2as_latbuf_rx1hdr; // cf2as_latbuf_rx1hdr_q;
	 wr1rsp_write   <= cf2as_latbuf_ch1_valid; // Tx1toRx1_pkt_vld;
      end
   end


   /* *******************************************************************
    * RESPONSE PATHS
    * -------------------------------------------------------------------
    * as2cf_rdresp_fifo    | Read Response staging
    * as2cf_wrresp_fifo    | Write Response staging
    * as2cf_umsg_fifo      | Unordered message staging *FIXME*
    *
    * *******************************************************************/

   logic [CCIP_RX_HDR_WIDTH-1:0] rdrsp_hdr_out_vec;
   logic [CCIP_RX_HDR_WIDTH-1:0] wr0rsp_hdr_out_vec;
   logic [CCIP_RX_HDR_WIDTH-1:0] wr1rsp_hdr_out_vec;

   /*
    * RX0 Read Response staging
    */
   ase_fifo
     #(
       .DATA_WIDTH     ( CCIP_RX_HDR_WIDTH + CCIP_DATA_WIDTH ),
       .DEPTH_BASE2    ( 8 ),
       .ALMFULL_THRESH ( 250 )
       )
   rdrsp_fifo
     (
      .clk             ( clk ),
      .rst             ( sys_reset ),
      .wr_en           ( rdrsp_write ),
      .data_in         ( { CCIP_RX_HDR_WIDTH'(rdrsp_hdr_in), rdrsp_data_in } ),
      .rd_en           ( ~rdrsp_empty && rdrsp_read ),
      .data_out        ( { rdrsp_hdr_out_vec, rdrsp_data_out } ),
      .data_out_v      ( rdrsp_valid ),
      .alm_full        ( rdrsp_full ),
      .full            (),
      .empty           ( rdrsp_empty ),
      .count           (),
      .overflow        (),
      .underflow       ()
      );

   assign rdrsp_hdr_out = RxHdr_t'(rdrsp_hdr_out_vec);

   /*
    * RX0 Write Response staging
    */
   // ase_fifo
   //   #(
   //     .DATA_WIDTH     ( CCIP_RX_HDR_WIDTH ),
   //     .DEPTH_BASE2    ( 7 ),
   //     .ALMFULL_THRESH ( 120 )
   //     )
   // wr0rsp_fifo
   //   (
   //    .clk             ( clk ),
   //    .rst             ( sys_reset ),
   //    .wr_en           ( wr0rsp_write ),
   //    .data_in         ( CCIP_RX_HDR_WIDTH'(wr0rsp_hdr_in) ),
   //    .rd_en           ( ~wr0rsp_empty && wr0rsp_read ),
   //    .data_out        ( wr0rsp_hdr_out_vec ),
   //    .data_out_v      ( wr0rsp_valid ),
   //    .alm_full        ( wr0rsp_full ),
   //    .full            (),
   //    .empty           ( wr0rsp_empty ),
   //    .count           (),
   //    .overflow        (),
   //    .underflow       ()
   //    );

   // assign wr0rsp_hdr_out = RxHdr_t'(wr0rsp_hdr_out_vec);

   /*
    * RX1 Write Response staging
    */
   ase_fifo
     #(
       .DATA_WIDTH     ( CCIP_RX_HDR_WIDTH ),
       .DEPTH_BASE2    ( 7 ),
       .ALMFULL_THRESH ( 120 )
       )
   wr1rsp_fifo
     (
      .clk             ( clk ),
      .rst             ( sys_reset ),
      .wr_en           ( wr1rsp_write ),
      .data_in         ( CCIP_RX_HDR_WIDTH'(wr1rsp_hdr_in) ),
      .rd_en           ( ~wr1rsp_empty && wr1rsp_read ),
      .data_out        ( wr1rsp_hdr_out_vec ),
      .data_out_v      ( wr1rsp_valid ),
      .alm_full        ( wr1rsp_full ),
      .full            (),
      .empty           ( wr1rsp_empty ),
      .count           (),
      .overflow        (),
      .underflow       ()
      );

   assign wr1rsp_hdr_out = RxHdr_t'(wr1rsp_hdr_out_vec);


   /* *******************************************************************
    * RX0 Channel management
    * -------------------------------------------------------------------
    * - MMIO Request management
    *   When request is seen in mmioreq_fifo, it is forwarded to
    *   CCIP-RX0
    * - Read Response
    *   When response is seen in as2cf_rdresp_fifo, it is forwarded to
    *   CCIP-RX0
    * - Write response
    *   When response is seen in as2cf_wrresp_fifo & tx2rx_chsel == 0, it
    *   is forwarded to CCIP-RX0
    *
    * *******************************************************************/
   // Output channel
   always @(posedge clk) begin
      if (sys_reset|SoftReset) begin
   	 C0RxMmioWrValid <= 1'b0;
   	 C0RxMmioRdValid <= 1'b0;
//   	 C0RxWrValid     <= 1'b0;
   	 C0RxRdValid     <= 1'b0;
   	 C0RxUMsgValid   <= 1'b0;
   	 C0RxHdr         <= RxHdr_t'({CCIP_RX_HDR_WIDTH{1'b0}});
   	 C0RxData        <= {CCIP_DATA_WIDTH{1'b0}};
	 umsgfifo_read   <= 1'b0;
   	 mmioreq_read    <= 1'b0;
   	 rdrsp_read      <= 1'b0;
   	 // wr0rsp_read     <= 1'b0;
	 rx0_state       <= RxIdle;
      end
      else begin
	 case (rx0_state)
	   RxIdle:
	     begin
		C0RxMmioWrValid <= 1'b0;
		C0RxMmioRdValid <= 1'b0;
		// C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		umsgfifo_read   <= 1'b0;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		// wr0rsp_read     <= 1'b0;
		if (~mmioreq_empty) begin
		   // mmioreq_read    <= ~mmioreq_empty;
		   rx0_state <= RxMMIOForward;
		end
		else if (~umsgfifo_empty) begin
		   rx0_state <= RxUMsgForward;
		end
		else if (~rdrsp_empty) begin
		   rx0_state <= RxReadResp;
		end
		// else if (~wr0rsp_empty) begin
		//    rx0_state <= RxWriteResp;
		// end
		else begin
		   rx0_state <= RxIdle;
		end
	     end

	   RxMMIOForward:
	     begin
		C0RxMmioWrValid <= mmio_wrvalid && mmioreq_valid;
		C0RxMmioRdValid <= mmio_rdvalid && mmioreq_valid;
		// C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		C0RxHdr         <= RxHdr_t'(mmio_hdrvec);
		C0RxData        <= mmio_data512;
		umsgfifo_read   <= 1'b0;
		mmioreq_read    <= ~mmioreq_empty;
		rdrsp_read      <= 1'b0;
		// wr0rsp_read     <= 1'b0;
		if (~mmioreq_empty) begin
		   rx0_state <= RxMMIOForward;
		end
		else begin
		   rx0_state <= RxIdle;
		end
	     end

	   RxUMsgForward:
	     begin
		C0RxMmioWrValid <= 1'b0;
		C0RxMmioRdValid <= 1'b0;
		// C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= umsgfifo_valid;
		C0RxHdr         <= RxHdr_t'(umsgfifo_hdrvec_out);
		C0RxData        <= umsgfifo_data_out;
		umsgfifo_read   <= ~umsgfifo_empty;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		// wr0rsp_read     <= 1'b0;
		if (~umsgfifo_empty) begin
		   rx0_state <= RxUMsgForward;
		end
		else begin
		   rx0_state <= RxIdle;
		end
	     end

	   RxReadResp:
	     begin
		C0RxMmioWrValid <= 1'b0;
		C0RxMmioRdValid <= 1'b0;
		// C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= rdrsp_valid;
		C0RxUMsgValid   <= 1'b0;
		C0RxHdr         <= rdrsp_hdr_out;
		C0RxData        <= rdrsp_data_out;
		umsgfifo_read   <= 1'b0;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= ~rdrsp_empty;
		// wr0rsp_read     <= 1'b0;
		if (~rdrsp_empty) begin
		   rx0_state <= RxReadResp;
		end
		else begin
		   rx0_state <= RxIdle;
		end
	     end

	   // RxWriteResp:
	   //   begin
	   // 	C0RxMmioWrValid <= 1'b0;
	   // 	C0RxMmioRdValid <= 1'b0;
	   // 	// C0RxWrValid     <= wr0rsp_valid;
	   // 	C0RxRdValid     <= 1'b0;
	   // 	C0RxUMsgValid   <= 1'b0;
	   // 	C0RxHdr         <= wr0rsp_hdr_out;
	   // 	C0RxData        <= {CCIP_DATA_WIDTH{1'b0}};
	   // 	umsgfifo_read   <= 1'b0;
	   // 	mmioreq_read    <= 1'b0;
	   // 	rdrsp_read      <= 1'b0;
	   // 	wr0rsp_read     <= ~wr0rsp_empty;
	   // 	if (~wr0rsp_empty) begin
	   // 	   rx0_state <= RxWriteResp;
	   // 	end
	   // 	else begin
	   // 	   rx0_state <= RxIdle;
	   // 	end
	   //   end

	   default:
	     begin
		C0RxMmioWrValid <= 1'b0;
		C0RxMmioRdValid <= 1'b0;
		// C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		umsgfifo_read   <= 1'b0;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		// wr0rsp_read     <= 1'b0;
		rx0_state       <= RxIdle;
	     end

	 endcase
      end
   end // always @ (posedge clk)

   // C0Rx Valid aggregate
   assign C0RxRspValid = C0RxRdValid | C0RxUMsgValid;


   /* *******************************************************************
    * RX1 Channel management
    * --------------------------------------------------------------
    * - Write response
    *   When response is seen in as2cf_wrresp_fifo & tx2rx_chsel == 1, it
    *   is forwarded to CCIP-RX1
    *
    * *******************************************************************/
   always @(posedge clk) begin
      if (sys_reset | SoftReset) begin
   	 C1RxHdr <= {CCIP_RX_HDR_WIDTH{1'b0}};
   	 C1RxWrValid <= 1'b0;
   	 C1RxIntrValid <= 1'b0;
   	 wr1rsp_read <= 1'b0;
   	 rx1_state <= RxIdle;
      end
      else begin
   	 case (rx1_state)
   	   RxIdle:
   	     begin
      		C1RxWrValid   <= 1'b0;
      		C1RxIntrValid <= 1'b0;
      		wr1rsp_read   <= 1'b0;
   		if (~wr1rsp_empty) begin
   		   rx1_state <= RxWriteResp;
   		end
   		else begin
   		   rx1_state <= RxIdle;
   		end
   	     end

   	   RxWriteResp:
   	     begin
      		C1RxHdr       <= wr1rsp_hdr_out;
      		C1RxWrValid   <= wr1rsp_valid;
      		C1RxIntrValid <= 1'b0;
      		wr1rsp_read   <= ~wr1rsp_empty;
   		if (~wr1rsp_empty) begin
   		   rx1_state <= RxWriteResp;
   		end
   		else begin
   		   rx1_state <= RxIdle;
   		end
   	     end

   	   default:
   	     begin
      		C1RxWrValid   <= 1'b0;
      		C1RxIntrValid <= 1'b0;
      		wr1rsp_read   <= 1'b0;
   		rx1_state     <= RxIdle;
   	     end

   	 endcase
      end
   end

   // always @(posedge clk) begin
   //    if (sys_reset|SoftReset) begin
   // 	 wr1rsp_read <= 0;	 
   //    end
   //    else if (~wr1rsp_empty) begin
   // 	 wr1rsp_read <= ~wr1rsp_empty;	 
   //    end
   // end
   
   // always @(posedge clk) begin
   //    if (sys_reset|SoftReset) begin
   // 	 C1RxHdr       <= {CCIP_RX_HDR_WIDTH{1'b0}};
   // 	 C1RxWrValid   <= 0;
   // 	 C1RxIntrValid <= 0;
   //    end
   //    else if (wr1rsp_valid) begin
   // 	 C1RxHdr       <= wr1rsp_hdr_out;	 
   // 	 C1RxWrValid   <= wr1rsp_valid;	 
   // 	 C1RxWrValid   <= 0;	 
   //    end
   //    else begin
   // 	 C1RxWrValid   <= 0;
   // 	 C1RxIntrValid <= 0;
   //    end
   // end
   
   // Rx1 aggregate valid
   assign C1RxRspValid = C1RxWrValid | C1RxIntrValid;


   /* *******************************************************************
    * Inactivity management block
    *
    * DESCRIPTION: Running ASE simulations for too long can cause
    *              large dump-files to be formed. To prevent this, the
    *              inactivity counter will close down the simulation
    *              when CCI transactions are not seen for a long
    *              duration of time.
    *
    * This feature can be disabled, if desired.
    *
    * *******************************************************************/
   logic 	    first_transaction_seen = 0;
   logic [31:0]     inactivity_counter;
   logic 	    any_valid;
   logic 	    inactivity_found;


   // Inactivity management - Sense first transaction
   assign any_valid =    C0RxUMsgValid
			 || C0RxRdValid
			 || C0RxMmioWrValid
			 || C0RxMmioRdValid
			 || C1RxWrValid
			 || C0TxRdValid
			 || C1TxWrValid ;

   // || C0RxWrValid

   // Check for first transaction
   always @(posedge clk, any_valid) begin : first_transaction_watcher
      if(any_valid) begin
	 first_transaction_seen	<= 1;
      end
   end

   // Inactivity management - killswitch
   always @(posedge clk) begin : call_simkill_countdown
      if((inactivity_found==1) && (cfg.ase_timeout != 0)) begin
	 $display("SIM-SV: Inactivity timeout reached !!\n");
	 start_simkill_countdown();
      end
   end

   // Inactivity management - counter
   counter
     #(
       .COUNT_WIDTH (32)
       )
   inact_ctr
     (
      .clk          (clk),
      .rst          ( first_transaction_seen && any_valid ),
      .cnt_en       (1'b1),
      .load_cnt     (32'b0),
      .max_cnt      (cfg.ase_timeout),
      .count_out    (inactivity_counter),
      .terminal_cnt (inactivity_found)
      );


   /*
    * Initialization procedure
    *
    * DESCRIPTION: This procedural block is called when ./simv is
    *              kicked off, helps put the simulation in a known
    *              state.
    *
    * STEPS:
    * - Print startup info
    * - Send initial system reset, cleaning up state machines
    * - Initialize ASE (ase_init executes in SW)
    *   - Set up message queues for IPC (done in SW)
    *   - Set up memory management structure (called in SW)
    * - If ENABLED, start the CA-private memory region (emulated with
    *   software
    * - Then set up the QLP InitDone signal to go indicate readiness
    * - SIMULATION is ready to begin
    *
    */
   initial begin : ase_entry_point

      $display("SIM-SV: Simulator started...");
      // Initialize data-structures
      mmio_dispatch (1, '{0, 0, 0, '{0,0,0,0,0,0,0,0}, 0});
      umsg_dispatch (1, '{0, 0, '{0,0,0,0,0,0,0,0}});

      // Globally write CONFIG, SCRIPT paths
      if (config_filepath.len() != 0) begin
	 sv2c_config_dex(config_filepath);
      end
      if (script_filepath.len() != 0) begin
	 sv2c_script_dex(script_filepath);
      end

      // Initialize SW side of ASE
      ase_init();

      // Initial signal values *FIXME*
      $display("SIM-SV: Sending initial reset...");
      sys_reset = 1;
      sw_reset_trig = 1;
      #100ns;
      sys_reset = 0;
      sw_reset_trig = 0;
      #100ns;

      // Setting up CA-private memory
      // if (cfg.enable_capcm) begin
      // 	 $display("SIM-SV: Enabling structures for CA Private Memory... ");
      // 	 capcm_init();
      // end

      // Link layer ready signal
      // wait (lp_initdone == 1);
      // $display("SIM-SV: CCI InitDone is HIGH...");

      // Indicate to APP that ASE is ready
      ase_ready();

   end


   /*
    * ASE Flow control error monitoring
    */
   // Flow simkill
   task flowerror_simkill(int sim_time, int channel) ;
      begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV: ASE has detected a possible OVERFLOW or UNDERFLOW error.");
	 $display("SIM-SV: Check simulation around time, t = %d in Channel %d", sim_time, channel);
   	 $display("SIM-SV: Simulation will end now");
	 `END_RED_FONTCOLOR;
	 start_simkill_countdown();
      end
   endtask

   // Flow error messages
   // always @(posedge clk) begin : overflow_error
   //    if (tx0_overflow) begin
   // 	 flowerror_simkill($time, 0);
   //    end
   //    if (tx0_underflow) begin
   // 	 flowerror_simkill($time, 0);
   //    end
   //    if (tx1_overflow) begin
   // 	 flowerror_simkill($time, 1);
   //    end
   //    if (tx1_underflow) begin
   // 	 flowerror_simkill($time, 1);
   //    end
   // end


   /*
    * CCI Sniffer
    * Aggregate point for all ASE checkers
    * - XZ checker
    * - Data hazard warning
    */
   // ccip_sniffer ccip_sniffer
   //   (
   //    .clk            (clk               ),
   //    .sys_reset_n    (sys_reset_n       ),
   //    .sw_reset_n     (SoftReset        ),
   //    .C0TxHdr        (C0TxHdr           ),
   //    .C0TxRdValid    (C0TxRdValid       ),
   //    .C0TxAlmFull    (C0TxAlmFull       ),
   //    .C1TxHdr        (C1TxHdr           ),
   //    .C1TxData       (C1TxData          ),
   //    .C1TxWrValid    (C1TxWrValid       ),
   //    .C1TxAlmFull    (C1TxAlmFull       ),
   //    .C1TxIntrValid  (C1TxIntrValid     ),
   //    .CfgRdData      (CfgRdData         ),
   //    .CfgRdDataValid (CfgRdDataValid    ),
   //    .CfgHeader      (CfgHeader         ),
   //    .CfgWrValid     (CfgWrValid        ),
   //    .CfgRdValid     (CfgRdValid        ),
   //    .C0RxHdr        (C0RxHdr           ),
   //    .C0RxData       (C0RxData          ),
   //    .C0RxRdValid    (C0RxRdValid       ),
   //    .C0RxWrValid    (C0RxWrValid       ),
   //    .C0RxUmsgValid  (C0RxUmsgValid     ),
   //    .C0RxIntrValid  (C0RxIntrValid     ),
   //    .C1RxHdr        (C1RxHdr           ),
   //    .C1RxWrValid    (C1RxWrValid       ),
   //    .C1RxIntrValid  (C1RxIntrValid     )
   //    );



   // Stream-checker for ASE
`ifdef ASE_DEBUG
   // Read response checking
   longint unsigned read_check_array[*];
   always @(posedge clk) begin : read_array_checkproc
      if (C0TxRdValid) begin
	 read_check_array[C0TxHdr.mdata] = C0TxHdr.addr;
      end
      if (C0RxRdValid) begin
	 if (read_check_array.exists(C0RxHdr.mdata))
	   read_check_array.delete(C0RxHdr.mdata);
      end
   end

   // Write response checking
   longint unsigned write_check_array[*];
   always @(posedge clk) begin : write_array_checkproc
      if (C1TxWrValid && (C1TxHdr.mdata != ASE_WRFENCE)) begin
	 write_check_array[C1TxHdr.mdata] = C1TxHdr.addr;
      end
      // if (C0RxWrValid) begin
      // 	 if (write_check_array.exists(C0RxHdr.mdata))
      // 	   write_check_array.delete(C0RxHdr.mdata);
      // end
      if (C1RxWrValid) begin
	 if (write_check_array.exists(C1RxHdr.mdata))
	   write_check_array.delete(C1RxHdr.mdata);
      end
   end
`endif


   /*
    * CCI Logger module
    */
   ccip_logger ccip_logger
     (
      // Logger control
      .enable_logger    (cfg.enable_cl_view),
      .finish_logger    (finish_logger     ),
      // Buffer message injection
      .log_string_en    (buffer_msg_en     ),
      .log_string       (buffer_msg        ),
      // CCIP ports
      .clk              (clk             ),
      .SoftReset        (SoftReset       ),
      .C0TxHdr          (C0TxHdr         ),
      .C0TxRdValid      (C0TxRdValid     ),
      .C1TxHdr          (C1TxHdr         ),
      .C1TxData         (C1TxData        ),
      .C1TxWrValid      (C1TxWrValid     ),
      .C1TxIntrValid    (1'b0   ),
      .C2TxHdr          (C2TxHdr         ),
      .C2TxMmioRdValid  (C2TxMmioRdValid ),
      .C2TxData         (C2TxData        ),
      .C0RxMmioWrValid  (C0RxMmioWrValid ),
      .C0RxMmioRdValid  (C0RxMmioRdValid ),
      .C0RxData         (C0RxData        ),
      .C0RxHdr          (C0RxHdr         ),
      .C0RxRdValid      (C0RxRdValid     ),
      .C0RxWrValid      (1'b0     ),
      .C0RxUMsgValid    (C0RxUMsgValid   ),
      .C1RxHdr          (C1RxHdr         ),
      .C1RxWrValid      (C1RxWrValid     ),
      .C1RxIntrValid    (C1RxIntrValid   ),
      .C0TxAlmFull      (C0TxAlmFull     ),
      .C1TxAlmFull      (C1TxAlmFull     )
      );


   /* ******************************************************************
    *
    * This call is made on ERRORs requiring a shutdown
    * simkill is called from software, and is the final step before
    * graceful closedown
    *
    * *****************************************************************/
   // Flag
   logic       simkill_started = 0;
   logic       memreq_outstanding;

   // Check if memory requests are outstanding
   // always (posedge clk) begin
   //    if (sys_reset) begin
   // 	 memreq_outstanding <= 0;
   //    end
   //    else begin
   // 	 if ((ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt) && (ase_tx1_wrvalid_cnt != (ase_rx0_wrvalid_cnt + ase_rx1_wrvalid_cnt))) begin
   // 	    memreq_outstanding <= 1;
   // 	 end
   // 	 else begin
   // 	    memreq_outstanding <= 0;
   // 	 end
   //    end
   // end

   // Simkill progress
   task simkill();
   // function void simkill();
      begin
	 simkill_started = 1;
	 $display("SIM-SV: Simulation kill command received...");
	 // $display("        Waiting for outstanding transactions to complete...");
	 // while( ~rdrsp_empty &&
	 // 	~wr0rsp_empty && ~wr1rsp_empty &&
	 // 	~umsgfifo_empty &&
	 // 	~mmioresp_empty && ~mmioreq_empty
	 // 	&& ~cf2as_latbuf_ch0_empty && ~cf2as_latbuf_ch1_empty )
	 //   begin
	 //      @(posedge clk);
	 //   end
	 // run_clocks(100);
	 // $display("SIM-SV: Simkill will proceed");

	 // Print transactions
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("Transaction counts => ");
	 $display("\tMMIO WrReq = %d", ase_rx0_mmiowrreq_cnt );
	 $display("\tMMIO RdReq = %d", ase_rx0_mmiordreq_cnt );
	 $display("\tMMIO RdRsp = %d", ase_tx2_mmiordrsp_cnt );
	 $display("\tRdReq      = %d", ase_tx0_rdvalid_cnt   );
	 $display("\tRdResp     = %d", ase_rx0_rdvalid_cnt   );
	 $display("\tWrReq      = %d", ase_tx1_wrvalid_cnt   );
	 // $display("\tWrResp     = %d", ase_rx0_wrvalid_cnt   );
	 $display("\tWrResp     = %d", ase_rx1_wrvalid_cnt   );
	 $display("\tWrFence    = %d", ase_tx1_wrfence_cnt   );
	 $display("\tWrFenceRsp = %d", ase_rx1_wrfence_cnt   );
	 $display("\tUMsgHint   = %d", ase_rx0_umsghint_cnt  );
	 $display("\tUMsgData   = %d", ase_rx0_umsgdata_cnt  );
	 `END_YELLOW_FONTCOLOR;

	 // Valid Count
`ifdef ASE_DEBUG
	 // Print errors
	 `BEGIN_RED_FONTCOLOR;
	 if (ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt)
	   $display("\tREADs   : Response counts dont match request count !!");
	 if (ase_tx1_wrvalid_cnt != ase_rx1_wrvalid_cnt)
	   $display("\tWRITEs  : Response counts dont match request count !!");
	 if (ase_tx2_mmiordrsp_cnt != ase_rx0_mmiordreq_cnt)
	   $display("\tMMIORd  : Response counts dont match request count !!");
	 if (ase_tx1_wrfence_cnt != ase_rx1_wrfence_cnt)
	   $display("\tWrFence : Response counts dont match request count !!");
	 `END_RED_FONTCOLOR;
	 // Dropped transactions
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("cf2as_latbuf_ch0 dropped =>");
	 $display(ase_top.ccip_emulator.cf2as_latbuf_ch0.checkunit.check_array);
	 $display("cf2as_latbuf_ch1 dropped =>");
	 $display(ase_top.ccip_emulator.cf2as_latbuf_ch1.checkunit.check_array);
	 $display("Read Response checker =>");
	 $display(read_check_array);
	 $display("Write Response checker =>");
	 $display(write_check_array);
	 `END_YELLOW_FONTCOLOR;
`endif
	 // $fclose(log_fd);
	 finish_logger = 1;

	 // Command to close logfd
	 $finish;
      end
   // endfunction
   endtask



endmodule // cci_emulator
