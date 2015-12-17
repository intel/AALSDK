/* ****************************************************************************
 * Copyright (c) 2011-2014, Intel Corporation
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
// import ase_top_pkg::*;

`include "platform.vh"

// CCI to Memory translator module
module ccip_emulator
  (
   // Clocks and reset
   output logic       vl_clk_LPdomain_64ui,
   output logic       vl_clk_LPdomain_32ui,
   output logic       vl_clk_LPdomain_16ui,
   output logic       ffs_LP16ui_afu_SoftReset_n,
   // Power and error   
   output logic [1:0] ffs_LP16ui_afu_PwrState, // CCI-P AFU Power State
   output logic       ffs_LP16ui_afu_Error, // CCI-P Protocol Error Detected   
   // Data ports
   input 	      t_if_ccip_Tx ffs_LP16ui_sTxData_afu,
   output 	      t_if_ccip_Rx ffs_LP16ui_sRxData_afu
   );
   
   // Power and error // TBD
    assign ffs_LP16ui_afu_PwrState = 2'b0;
   assign ffs_LP16ui_afu_Error = 1'b0;
   
   

   /*
    * CCIP breakout
    */
   // Clock/reset
   logic 			      Clk16UI ;
   logic 			      Clk32UI ;
   logic 			      Clk64UI ;
   logic 			      SoftReset_n;
   // Tx0
   TxHdr_t                            C0TxHdr;
   logic 	                      C0TxRdValid;
   // Tx1
   TxHdr_t                            C1TxHdr;
   logic [CCIP_DATA_WIDTH-1:0]        C1TxData;
   logic 		              C1TxWrValid;
   logic 		              C1TxIntrValid;
   // Tx2
   MMIOHdr_t                          C2TxHdr;
   logic                              C2TxMMIORdValid;
   logic [CCIP_MMIO_RDDATA_WIDTH-1:0] C2TxData;
   // Rx0
   logic 			      C0RxMMIOWrValid;
   logic 			      C0RxMMIORdValid;
   logic [CCIP_DATA_WIDTH-1:0] 	      C0RxData;
   RxHdr_t                            C0RxHdr;
   logic 			      C0RxRdValid;
   logic 			      C0RxWrValid;
   logic 			      C0RxUMsgValid;
   // Rx1
   RxHdr_t                            C1RxHdr;
   logic 			      C1RxWrValid;
   logic 			      C1RxIntrValid;
   // Almost full signals
   logic 			      C0TxAlmFull;
   logic 			      C1TxAlmFull;


   /*
    * ASE's reset signal
    */
   logic 			      sys_reset_n;

   /*
    * Remapping ASE CCIP to cvl_pkg struct
    */
   assign vl_clk_LPdomain_16ui = Clk16UI;
   assign vl_clk_LPdomain_32ui = Clk32UI;
   assign vl_clk_LPdomain_64ui = Clk64UI;

   assign ffs_LP16ui_afu_SoftReset_n = SoftReset_n;

   // Rx/Tx mappint
   always @(*) begin
      // Rx OUT
      ffs_LP16ui_sRxData_afu.C0Hdr <= t_ccip_RspMemHdr'(C0RxHdr);
      ffs_LP16ui_sRxData_afu.C0Data <= C0RxData;
      ffs_LP16ui_sRxData_afu.C0WrValid <= C0RxWrValid;
      ffs_LP16ui_sRxData_afu.C0RdValid <= C0RxRdValid;
      ffs_LP16ui_sRxData_afu.C0UMsgValid <= C0RxUMsgValid;
      ffs_LP16ui_sRxData_afu.C0MmioRdValid <= C0RxMMIORdValid;
      ffs_LP16ui_sRxData_afu.C0MmioWrValid <= C0RxMMIOWrValid;
      ffs_LP16ui_sRxData_afu.C1Hdr <= t_ccip_RspMemHdr'(C1RxHdr);
      ffs_LP16ui_sRxData_afu.C1WrValid <= C1RxWrValid;
      ffs_LP16ui_sRxData_afu.C1IntrValid <= C1RxIntrValid;
      // Tx OUT
      C0TxHdr <= TxHdr_t'(ffs_LP16ui_sTxData_afu.C0Hdr);
      C0TxRdValid <= ffs_LP16ui_sTxData_afu.C0RdValid;
      C1TxHdr <= TxHdr_t'(ffs_LP16ui_sTxData_afu.C1Hdr);
      C1TxData <= ffs_LP16ui_sTxData_afu.C1Data;
      C1TxWrValid <= ffs_LP16ui_sTxData_afu.C1WrValid;
      C1TxIntrValid <= ffs_LP16ui_sTxData_afu.C1IntrValid;
      C2TxHdr <= MMIOHdr_t'(ffs_LP16ui_sTxData_afu.C2Hdr);
      C2TxData <= ffs_LP16ui_sTxData_afu.C2Data;
      C2TxMMIORdValid <= ffs_LP16ui_sTxData_afu.C2MmioRdValid;
      // Almost full signals
      ffs_LP16ui_sRxData_afu.C0TxAlmFull = C0TxAlmFull;
      ffs_LP16ui_sRxData_afu.C1TxAlmFull = C1TxAlmFull;
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
   // export "DPI-C" task umsg_dispatch;
   // MMIO dispatch
   export "DPI-C" task mmio_dispatch;

   // Start simulation structures teardown
   import "DPI-C" context task start_simkill_countdown();
   // Signal to kill simulation
   export "DPI-C" task simkill;

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

   // Scope generator
   // initial ccip_emulator_scope_function();
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

   // LP initdone & reset registered signals
   // logic 			  lp_initdone_q;
   logic 			  lp_initdone;

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
   logic 			  sw_reset_trig;

   /*
    * AFU reset - software & system resets
    */
   //       0        |     0               0     | Initial reset
   //       0        |     0               1     |
   //       0        |     1               0     |
   //       1        |     1               1     |
   assign SoftReset_n = sys_reset_n && sw_reset_trig;


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
   logic 				     mmioreq_pop;
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
	       mmio_response(mmio_pkt);
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
      .rst        ( ~sys_reset_n ),
      .wr_en      ( mmioreq_write ),
      .data_in    ( mmioreq_din ),
      .rd_en      ( mmioreq_read /* && ~mmioreq_empty */ ),
      .data_out   ( {mmio_wrvalid, mmio_rdvalid, mmio_hdrvec, mmio_data512} ),
      .data_out_v ( mmioreq_valid ),
      .alm_full   ( mmioreq_full ),
      .full       (  ),
      .empty      ( mmioreq_empty ),
      .count      ( mmioreq_count ),
      .overflow   (  ),
      .underflow  (  )
      );

   CfgHdr_t DBG_cfgheader;
   assign DBG_cfgheader = CfgHdr_t'(cwlp_header);


   // CSR activity engine *FIXME*
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 sw_reset_trig <= 0;
      end
      else begin
	 sw_reset_trig <= 1;
      end
   end

   /*
    * MMIO Read response
    */
   parameter int MMIORESP_FIFO_WIDTH = CCIP_MMIO_TID_WIDTH + CCIP_MMIO_RDDATA_WIDTH;

   logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_din;
   logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_dout;
   logic 			   mmioresp_write;
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
      .rst        ( ~sys_reset_n ),
      .wr_en      ( C2TxMMIORdValid ),
      .data_in    ( {CCIP_MMIO_TID_WIDTH'(C2TxHdr), C2TxData} ),
      .rd_en      ( mmioresp_read /* & mmioresp_empty */ ),
      .data_out   ( mmioresp_dout ),
      .data_out_v ( mmioresp_valid ),
      .alm_full   ( mmioresp_full ),
      .full       (  ),
      .empty      ( mmioresp_empty ),
      .count      (  ),
      .overflow   (  ),
      .underflow  (  )
      );

   // FIFO writes to memory
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 mmioresp_read <= 0;
      end
      else begin
   	 if (~mmioresp_empty) begin
   	    mmioresp_read <= ~mmioresp_empty;
   	    mmio_response ( mmio_resp_pkt );
   	 end
   	 else begin
   	    mmioresp_read <= 0;
   	 end
      end
   end



   /* ******************************************************************
    *
    * Unordered Messages Engine
    * umsg_dispatch: Single push process triggering UMSG machinery
    *
    * *****************************************************************/

   // parameter int UMSG_FIFO_WIDTH = CCIP_RX_HDR_WIDTH + CCIP_DATA_WIDTH;

   // logic [UMSG_FIFO_WIDTH-1:0] umsgff_din;
   // logic [UMSG_FIFO_WIDTH-1:0] umsgff_dout;
   // logic 		       umsgff_write;
   // logic 		       umsgff_pop;
   // logic 		       umsgff_read;
   // logic 		       umsgff_valid;
   // logic 		       umsgff_full;
   // logic 		       umsgff_empty;
   // logic 		       umsgff_overflow;
   // logic 		       umsgff_underflow;

   // int 			       umsg_data_slot;
   // int 			       umsg_hint_slot;
   // int 			       umsg_data_slot_old = 255;
   // int 			       umsg_hint_slot_old = 255;
   // umsg_t                      umsg_array[`UMSG_MAX_MSG];

   // logic [0:`UMSG_MAX_MSG-1]   umsgff_write_array;
   // logic [0:`UMSG_MAX_MSG-1]   umsg_valid;


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
   int ase_rx0_wrvalid_cnt   ;
   int ase_rx1_wrvalid_cnt   ;
   int ase_tx1_wrfence_cnt   ;
   int ase_rx0_umsghint_cnt  ;
   int ase_rx0_umsgdata_cnt  ;

   // process
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 ase_rx0_mmiowrreq_cnt <= 0 ;
	 ase_rx0_mmiordreq_cnt <= 0 ;
	 ase_tx2_mmiordrsp_cnt <= 0 ;
	 ase_tx0_rdvalid_cnt <= 0 ;
	 ase_rx0_rdvalid_cnt <= 0 ;
	 ase_tx1_wrvalid_cnt <= 0 ;
	 ase_rx0_wrvalid_cnt <= 0 ;
	 ase_rx1_wrvalid_cnt <= 0 ;
	 ase_tx1_wrfence_cnt <= 0 ;
	 ase_rx0_umsghint_cnt <= 0 ;
	 ase_rx0_umsgdata_cnt <= 0 ;
      end
      else begin
	 if (C0RxMMIOWrValid) 
	   ase_rx0_mmiowrreq_cnt = ase_rx0_mmiowrreq_cnt + 1;
	 if (C0RxMMIORdValid) 
	   ase_rx0_mmiordreq_cnt = ase_rx0_mmiordreq_cnt + 1;
	 if (C2TxMMIORdValid) 
	   ase_tx2_mmiordrsp_cnt = ase_tx2_mmiordrsp_cnt + 1;
	 if (C0TxRdValid)
	   ase_tx0_rdvalid_cnt = ase_tx0_rdvalid_cnt + 1;
	 if (C0RxRdValid)
	   ase_rx0_rdvalid_cnt = ase_rx0_rdvalid_cnt + 1;
	 if (C1TxWrValid && (C1TxHdr.reqtype != CCIP_TX1_WRFENCE))
	   ase_tx1_wrvalid_cnt = ase_tx1_wrvalid_cnt + 1;
	 if (C0RxWrValid)
	   ase_rx0_wrvalid_cnt = ase_rx0_wrvalid_cnt + 1;
	 if (C1RxWrValid)
	   ase_rx1_wrvalid_cnt = ase_rx1_wrvalid_cnt + 1;
	 if (C1TxWrValid && (C1TxHdr.reqtype == CCIP_TX1_WRFENCE))
	   ase_tx1_wrfence_cnt = ase_tx1_wrfence_cnt + 1;	 
      end
   end
   

   /* ******************************************************************
    *
    * This call is made on ERRORs requiring a shutdown
    * simkill is called from software, and is the final step before
    * graceful closedown
    *
    * *****************************************************************/
   task simkill();
      begin
	 $display("SIM-SV: Simulation kill command received...");
	 // Print transactions
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("Transaction counts => ");
	 $display("\tMMIO WrReq = %d", ase_rx0_mmiowrreq_cnt );
	 $display("\tMMIO RdReq = %d", ase_rx0_mmiordreq_cnt );
	 $display("\tMMIO RdRsp = %d", ase_tx2_mmiordrsp_cnt );
	 $display("\tRdReq      = %d", ase_tx0_rdvalid_cnt   );
	 $display("\tRdResp     = %d", ase_rx0_rdvalid_cnt   );
	 $display("\tWrReq      = %d", ase_tx1_wrvalid_cnt   );
	 $display("\tWrResp-CH0 = %d", ase_rx0_wrvalid_cnt   );
	 $display("\tWrResp-CH1 = %d", ase_rx1_wrvalid_cnt   );
	 $display("\tWrFence    = %d", ase_tx1_wrfence_cnt   );
	 $display("\tUMsgHint   = %d", ase_rx0_umsghint_cnt  );
	 $display("\tUMsgData   = %d", ase_rx0_umsgdata_cnt  );
	 `END_YELLOW_FONTCOLOR;

	 // Valid Count
// `ifdef ASE_DEBUG
//  `ifdef ASE_RANDOMIZE_TRANSACTIONS
// 	 // Print errors
	 `BEGIN_RED_FONTCOLOR;
	 if (ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt)
	   $display("\tREADs  : Response counts dont match request count !!");
	 if (ase_tx1_wrvalid_cnt != (ase_rx0_wrvalid_cnt + ase_rx1_wrvalid_cnt))
	   $display("\tWRITEs : Response counts dont match request count !!");
	 `END_RED_FONTCOLOR;
// 	 // Dropped transactions
// 	 `BEGIN_YELLOW_FONTCOLOR;
// 	 $display("cf2as_latbuf_ch0 dropped =>");
// 	 $display(cci_emulator.cf2as_latbuf_ch0.checkunit.check_array);
// 	 $display("cf2as_latbuf_ch1 dropped =>");
// 	 $display(cci_emulator.cf2as_latbuf_ch1.checkunit.check_array);
// 	 $display("Read Response checker =>");
// 	 $display(read_check_array);
// 	 $display("Write Response checker =>");
// 	 $display(write_check_array);
// 	 `END_YELLOW_FONTCOLOR;
//  `endif
// `endif
	 // $fclose(log_fd);
	 finish_logger = 1;

	 // Command to close logfd
	 $finish;
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

   // Write response 0 staging signals
   RxHdr_t                     wr0rsp_hdr_in, wr0rsp_hdr_out;
   logic 		       wr0rsp_write;
   logic 		       wr0rsp_read;
   logic 		       wr0rsp_full;
   logic 		       wr0rsp_empty;
   logic 		       wr0rsp_valid;

   // Write response 1 staging signals
   RxHdr_t                     wr1rsp_hdr_in, wr1rsp_hdr_out;
   logic 		       wr1rsp_write;
   logic 		       wr1rsp_read;
   logic 		       wr1rsp_full;
   logic 		       wr1rsp_empty;
   logic 		       wr1rsp_valid;

   // Declare packets for each channel
   cci_pkt Tx0toRx0_pkt;
   cci_pkt Tx1toRx0_pkt;
   cci_pkt Tx1toRx1_pkt;

   logic Tx0toRx0_pkt_vld;
   logic Tx1toRx0_pkt_vld;
   logic Tx1toRx1_pkt_vld;

   
   /*
    * FUNCTION: Cast TxHdr_t to cci_pkt
    */
   function automatic void cast_txhdr_to_ccipkt (ref   cci_pkt               pkt,
						 input int                   write_en,
						 input TxHdr_t               txhdr,
						 input [CCIP_DATA_WIDTH-1:0] txdata);
      begin
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
	    pkt.resp_channel = wrresp_tx2rx_chsel();	    
	 end
	 else begin
	    pkt.resp_channel = 0;	    
	 end
      end
   endfunction

   
   // rdreq/wrreq checker
   logic rdreq_flag;
   logic wrreq_flag;

   always @(*) begin
      if (~sys_reset_n) begin
	 rdreq_flag <= 0;
	 wrreq_flag <= 0;
      end
      else begin
	 // RdReq type
	 case (cf2as_latbuf_tx0hdr.reqtype)
	   CCIP_TX0_RDLINE_S : rdreq_flag = 1;
	   CCIP_TX0_RDLINE_I : rdreq_flag = 1;
	   CCIP_TX0_RDLINE_E : rdreq_flag = 1;
	   default           : rdreq_flag = 0;
	 endcase
	 // WrReq type
	 case (cf2as_latbuf_tx1hdr.reqtype)
	   CCIP_TX1_WRLINE_I : wrreq_flag = 1;
	   CCIP_TX1_WRLINE_M : wrreq_flag = 1;
	   default           : wrreq_flag = 0;
	 endcase
      end
   end

   // cf2as_latbuf_ch0 signals
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_tx0hdr_vec;
   TxHdr_t                       cf2as_latbuf_tx0hdr;
   logic                         cf2as_latbuf_ch0_empty;
   logic                         cf2as_latbuf_ch0_read;
   int 				 cf2as_latbuf_ch0_count;
   logic 			 cf2as_latbuf_ch0_pop;

   // cf2as_latbuf_ch1 signals
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_tx1hdr_vec;
   logic [CCIP_DATA_WIDTH-1:0]   cf2as_latbuf_tx1data;
   TxHdr_t                       cf2as_latbuf_tx1hdr;
   logic 		         cf2as_latbuf_ch1_empty;
   logic 		         cf2as_latbuf_ch1_read;
   int 				 cf2as_latbuf_ch1_count;
   logic 			 cf2as_latbuf_ch1_valid;
   logic 			 cf2as_latbuf_ch1_pop;


   /*
    * CAFU->ASE CH0 (TX0)
    * Formed as {TxHdr_t}
    * Latency scoreboard (for latency modeling and shuffling)
    */
   outoforder_wrf_channel
     #(
       .NUM_WAIT_STATIONS   (LATBUF_NUM_TRANSACTIONS),
       .HDR_WIDTH           (CCIP_TX_HDR_WIDTH),
       .DATA_WIDTH          (CCIP_DATA_WIDTH),
       .COUNT_WIDTH         (LATBUF_COUNT_WIDTH),
       .FIFO_FULL_THRESH    (LATBUF_FULL_THRESHOLD),
       .FIFO_DEPTH_BASE2    (LATBUF_DEPTH_BASE2)
       )
   cf2as_latbuf_ch0
     (
      .clk		( clk ),
      .rst		( ~sys_reset_n ),
      .meta_in		( CCIP_TX_HDR_WIDTH'(C0TxHdr)),
      .data_in		( {CCIP_DATA_WIDTH{1'b0}} ),
      .write_en		( C0TxRdValid ),
      .meta_out		( cf2as_latbuf_tx0hdr_vec ),
      .data_out		(  ),
      .valid_out	( cf2as_latbuf_ch0_valid ),
      .read_en		( cf2as_latbuf_ch0_pop ),
      .empty		( cf2as_latbuf_ch0_empty ),
      .full             ( C0TxAlmFull ),
      .overflow         ( tx0_overflow ),
      .underflow        ( tx0_underflow ),
      .count            ( cf2as_latbuf_ch0_count )
      );

   assign cf2as_latbuf_tx0hdr = TxHdr_t'(cf2as_latbuf_tx0hdr_vec);
   assign cf2as_latbuf_ch0_pop = ~cf2as_latbuf_ch0_empty && cf2as_latbuf_ch0_read;

   // stage 1 - Pop read request
   always @(posedge clk) begin
      if (~sys_reset_n) begin
   	 cf2as_latbuf_ch0_read <= 0;
   	 Tx0toRx0_pkt_vld           <= 0;
      end
      else begin
   	 // TX0 - Read Request
   	 if ( ~cf2as_latbuf_ch0_empty && rdreq_flag && ~rdrsp_full ) begin
   	    cast_txhdr_to_ccipkt( Tx0toRx0_pkt,
   				  0,
   				  cf2as_latbuf_tx0hdr,
   				  {CCIP_DATA_WIDTH{1'b0}} );
	    if (cf2as_latbuf_ch0_valid) begin
   	       rd_memline_dex(Tx0toRx0_pkt);
   	       Tx0toRx0_pkt_vld <= cf2as_latbuf_ch0_valid;
	    end
	    else begin
	       Tx0toRx0_pkt_vld <= 0;
	    end
   	    cf2as_latbuf_ch0_read <= ~cf2as_latbuf_ch0_empty;
   	 end
   	 // Default case
   	 else begin
   	    cf2as_latbuf_ch0_read <= 0;
   	    Tx0toRx0_pkt_vld <= 0;
   	 end
      end
   end


   // Stage 2 - Stage read response
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 rdrsp_data_in <= {CCIP_DATA_WIDTH{1'b0}};
	 rdrsp_hdr_in <= {CCIP_RX_HDR_WIDTH{1'b0}};
	 rdrsp_write <= 0;
      end
      else begin
	 if (Tx0toRx0_pkt_vld) begin
	    rdrsp_data_in         <= unpack_ccipkt_to_vector(Tx0toRx0_pkt);
	    rdrsp_hdr_in.vc       <= Tx0toRx0_pkt.vc;
	    rdrsp_hdr_in.poison   <= 1'b0;
	    rdrsp_hdr_in.hitmiss  <= 1'b0;
	    rdrsp_hdr_in.format   <= 1'b0;
	    rdrsp_hdr_in.rsvd22   <= 1'b0;
	    rdrsp_hdr_in.clnum    <= 2'b0;
	    rdrsp_hdr_in.resptype <= CCIP_RX0_RD_RESP;
	    rdrsp_hdr_in.mdata    <= Tx0toRx0_pkt.mdata;
	    rdrsp_write           <= 1;
	 end
	 else begin
	    rdrsp_write <= 0;
	 end
      end
   end

   /*
    * CAFU->ASE CH1 (TX1)
    * Formed as {TxHdr_t, <data_512>}
    * Latency scoreboard (latency modeling and shuffling)
    */
   outoforder_wrf_channel
     #(
       .NUM_WAIT_STATIONS(LATBUF_NUM_TRANSACTIONS),
       .HDR_WIDTH        (CCIP_TX_HDR_WIDTH),
       .DATA_WIDTH       (CCIP_DATA_WIDTH),
       .COUNT_WIDTH      (LATBUF_COUNT_WIDTH),
       .FIFO_FULL_THRESH (LATBUF_FULL_THRESHOLD),
       .FIFO_DEPTH_BASE2 (LATBUF_DEPTH_BASE2)
       )
   cf2as_latbuf_ch1
     (
      .clk		( clk ),
      .rst		( ~sys_reset_n ),
      .meta_in		( CCIP_TX_HDR_WIDTH'(C1TxHdr) ),
      .data_in		( C1TxData ),
      .write_en		( C1TxWrValid ),
      .meta_out		( cf2as_latbuf_tx1hdr_vec ),
      .data_out		( cf2as_latbuf_tx1data ),
      .valid_out	( cf2as_latbuf_ch1_valid),
      .read_en		( cf2as_latbuf_ch1_pop  ),
      .empty		( cf2as_latbuf_ch1_empty ),
      .full             ( C1TxAlmFull ),
      .overflow         ( tx1_overflow ),
      .underflow        ( tx1_underflow ),
      .count            ( cf2as_latbuf_ch1_count )
      );

   
   assign cf2as_latbuf_tx1hdr = TxHdr_t'(cf2as_latbuf_tx1hdr_vec);
   assign cf2as_latbuf_ch1_pop = ~cf2as_latbuf_ch1_empty && cf2as_latbuf_ch1_read;

   // TX-CH1 must select RX-CH0 or RX-CH1 channels for fulfillment
   // Since requests on TX1 can return either via RX0 or RX1, this is needed
   function automatic int wrresp_tx2rx_chsel();
      begin
	 // return 0;
	 // return 1;
	 return (abs_val($random) % 2);
      end
   endfunction
   
   // TX1 fulfillment flow
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 cf2as_latbuf_ch1_read <= 0;
	 Tx1toRx0_pkt_vld <= 0;
	 Tx1toRx1_pkt_vld <= 0;	 
      end
      else if (~cf2as_latbuf_ch1_empty && ~wr0rsp_full && ~wr1rsp_full && wrreq_flag) begin
	 cf2as_latbuf_ch1_read <= ~cf2as_latbuf_ch1_empty;
	 cast_txhdr_to_ccipkt(Tx1toRx0_pkt, 1, cf2as_latbuf_tx1hdr, cf2as_latbuf_tx1data);
	 cast_txhdr_to_ccipkt(Tx1toRx1_pkt, 1, cf2as_latbuf_tx1hdr, cf2as_latbuf_tx1data);
	 if ((Tx1toRx1_pkt.resp_channel == 0) && cf2as_latbuf_ch1_valid ) begin
	    wr_memline_dex(Tx1toRx0_pkt);
	    Tx1toRx0_pkt_vld <= 1;
	    Tx1toRx1_pkt_vld <= 0;	 
	 end
	 else if ((Tx1toRx1_pkt.resp_channel == 1) && cf2as_latbuf_ch1_valid ) begin
	    wr_memline_dex(Tx1toRx1_pkt);    
	    Tx1toRx0_pkt_vld <= 0;
	    Tx1toRx1_pkt_vld <= 1;	 
	 end
      end
      else begin
	 cf2as_latbuf_ch1_read <= 0;
	 Tx1toRx0_pkt_vld <= 0;
	 Tx1toRx1_pkt_vld <= 0;	 
      end
   end
   
   
   // Stage 2 - Tx1toRx0 write process
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 wr0rsp_hdr_in  <= {CCIP_RX_HDR_WIDTH{1'b0}};
	 wr0rsp_write   <= 0;
      end
      else begin
	 if (Tx1toRx0_pkt_vld) begin
	    wr0rsp_hdr_in.vc       <= Tx1toRx0_pkt.vc;
	    wr0rsp_hdr_in.poison   <= 1'b0;
	    wr0rsp_hdr_in.hitmiss  <= 1'b0;
	    wr0rsp_hdr_in.format   <= 1'b0;
	    wr0rsp_hdr_in.rsvd22   <= 1'b0;
	    wr0rsp_hdr_in.clnum    <= 2'b0;
	    wr0rsp_hdr_in.resptype <= CCIP_RX0_WR_RESP;
	    wr0rsp_hdr_in.mdata    <= Tx1toRx0_pkt.mdata;
	    wr0rsp_write           <= 1;
	 end
	 else begin
	    wr0rsp_write   <= 0;
	 end
      end
   end

   // Stage 2 - Tx1toRx1 write process
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 wr1rsp_hdr_in  <= {CCIP_RX_HDR_WIDTH{1'b0}};
	 wr1rsp_write   <= 1'b0;
      end
      else begin
	 if (Tx1toRx1_pkt_vld) begin
	    wr1rsp_hdr_in.vc       <= Tx1toRx1_pkt.vc;
	    wr1rsp_hdr_in.poison   <= 1'b0;
	    wr1rsp_hdr_in.hitmiss  <= 1'b0;
	    wr1rsp_hdr_in.format   <= 1'b0;
	    wr1rsp_hdr_in.rsvd22   <= 1'b0;
	    wr1rsp_hdr_in.clnum    <= 2'b0;
	    wr1rsp_hdr_in.resptype <= CCIP_RX1_WR_RESP;
	    wr1rsp_hdr_in.mdata    <= Tx1toRx1_pkt.mdata;
	    wr1rsp_write           <= 1;
	 end
	 else begin
	    wr1rsp_write   <= 0;
	 end
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
      .rst             ( ~sys_reset_n ),
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
   ase_fifo
     #(
       .DATA_WIDTH     ( CCIP_RX_HDR_WIDTH ),
       .DEPTH_BASE2    ( 7 ),
       .ALMFULL_THRESH ( 120 )
       )
   wr0rsp_fifo
     (
      .clk             ( clk ),
      .rst             ( ~sys_reset_n ),
      .wr_en           ( wr0rsp_write ),
      .data_in         ( CCIP_RX_HDR_WIDTH'(wr0rsp_hdr_in) ),
      .rd_en           ( ~wr0rsp_empty && wr0rsp_read ),
      .data_out        ( wr0rsp_hdr_out_vec ),
      .data_out_v      ( wr0rsp_valid ),
      .alm_full        ( wr0rsp_full ),
      .full            (),
      .empty           ( wr0rsp_empty ),
      .count           (),
      .overflow        (),
      .underflow       ()
      );

   assign wr0rsp_hdr_out = RxHdr_t'(wr0rsp_hdr_out_vec);

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
      .rst             ( ~sys_reset_n ),
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
      if (~sys_reset_n) begin
   	 C0RxMMIOWrValid <= 1'b0;
   	 C0RxMMIORdValid <= 1'b0;
   	 C0RxWrValid     <= 1'b0;
   	 C0RxRdValid     <= 1'b0;
   	 C0RxUMsgValid   <= 1'b0;
   	 C0RxHdr         <= RxHdr_t'({CCIP_RX_HDR_WIDTH{1'b0}});
   	 C0RxData        <= {CCIP_DATA_WIDTH{1'b0}};
   	 mmioreq_read    <= 1'b0;
   	 rdrsp_read      <= 1'b0;
   	 wr0rsp_read     <= 1'b0;
	 rx0_state       <= RxIdle;	 
      end
      else begin
	 case (rx0_state)
	   RxIdle:
	     begin
		C0RxMMIOWrValid <= 1'b0;
		C0RxMMIORdValid <= 1'b0;
		C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		wr0rsp_read     <= 1'b0;
		if (~mmioreq_empty) begin
		   mmioreq_read    <= ~mmioreq_empty;	    
		   rx0_state <= RxMMIOForward;		   
		end
		else if (~rdrsp_empty) begin
		   rx0_state <= RxReadResp;		   
		end
		else if (~wr0rsp_empty) begin
		   rx0_state <= RxWriteResp;		     
		end
		else begin
		   rx0_state <= RxIdle;		   
		end
	     end
	   
	   RxMMIOForward:
	     begin
		C0RxMMIOWrValid <= mmio_wrvalid && mmioreq_valid;
		C0RxMMIORdValid <= mmio_rdvalid && mmioreq_valid;
		C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		C0RxHdr         <= RxHdr_t'(mmio_hdrvec);
		C0RxData        <= mmio_data512;
		// mmioreq_read    <= ~mmioreq_empty;	    
		mmioreq_read    <= 1'b0;		
		rdrsp_read      <= 1'b0;
		wr0rsp_read     <= 1'b0;
		if (~mmioreq_empty) begin
		   rx0_state <= RxMMIOForward;		   
		end
		else begin
		   rx0_state <= RxIdle;		   
		end
	     end
	   
	   RxReadResp:
	     begin
		C0RxMMIOWrValid <= 1'b0;
		C0RxMMIORdValid <= 1'b0;
		C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= rdrsp_valid;
		C0RxUMsgValid   <= 1'b0;
		C0RxHdr         <= rdrsp_hdr_out;
		C0RxData        <= rdrsp_data_out;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= ~rdrsp_empty;
		wr0rsp_read     <= 1'b0;
		if (~rdrsp_empty) begin
		   rx0_state <= RxReadResp;		   
		end
		else begin
		   rx0_state <= RxIdle;		   
		end
	     end
	   
	   RxWriteResp:
	     begin
		C0RxMMIOWrValid <= 1'b0;
		C0RxMMIORdValid <= 1'b0;
		C0RxWrValid     <= wr0rsp_valid;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		C0RxHdr         <= wr0rsp_hdr_out;
		C0RxData        <= {CCIP_DATA_WIDTH{1'b0}};
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		wr0rsp_read     <= ~wr0rsp_empty;
		if (~wr0rsp_empty) begin
		   rx0_state <= RxWriteResp;		   
		end
		else begin
		   rx0_state <= RxIdle;		   
		end
	     end
	   
	   default:
	     begin
		C0RxMMIOWrValid <= 1'b0;
		C0RxMMIORdValid <= 1'b0;
		C0RxWrValid     <= 1'b0;
		C0RxRdValid     <= 1'b0;
		C0RxUMsgValid   <= 1'b0;
		mmioreq_read    <= 1'b0;
		rdrsp_read      <= 1'b0;
		wr0rsp_read     <= 1'b0;
		rx0_state       <= RxIdle;		
	     end
	   
	 endcase
      end
   end // always @ (posedge clk)
   

   /* *******************************************************************
    * RX1 Channel management
    * --------------------------------------------------------------
    * - Write response
    *   When response is seen in as2cf_wrresp_fifo & tx2rx_chsel == 1, it
    *   is forwarded to CCIP-RX1
    *
    * *******************************************************************/
   always @(posedge clk) begin
      if (~sys_reset_n) begin
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
			 || C0RxWrValid
			 || C0RxRdValid
			 || C1RxWrValid
			 || C0TxRdValid
			 || C1TxWrValid ;


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
      sys_reset_n = 0;
      #100ns;
      sys_reset_n = 1;
      #100ns;

      // Setting up CA-private memory
      // if (cfg.enable_capcm) begin
      // 	 $display("SIM-SV: Enabling structures for CA Private Memory... ");
      // 	 capcm_init();
      // end

      // Link layer ready signal
      wait (lp_initdone == 1);
      $display("SIM-SV: CCI InitDone is HIGH...");

      // Indicate to APP that ASE is ready
      ase_ready();

   end


   /*
    * Latency pipe : For LP_InitDone delay
    * This block simulates the latency between a generic reset and QLP
    * InitDone
    */
   latency_pipe
     #(
       .NUM_DELAY (`LP_INITDONE_READINESS_LATENCY),
       .PIPE_WIDTH (1)
       )
   lp_initdone_lat
     (
      .clk (clk),
      .rst (~sys_reset_n),
      .pipe_in (sys_reset_n),
      .pipe_out (lp_initdone)
      );


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
   always @(posedge clk) begin : overflow_error
      if (tx0_overflow) begin
	 flowerror_simkill($time, 0);
      end
      if (tx0_underflow) begin
	 flowerror_simkill($time, 0);
      end
      if (tx1_overflow) begin
	 flowerror_simkill($time, 1);
      end
      if (tx1_underflow) begin
	 flowerror_simkill($time, 1);
      end
   end


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
   //    .sw_reset_n     (SoftReset_n        ),
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
   int unsigned read_check_array[*];
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
   int unsigned write_check_array[*];
   always @(posedge clk) begin : write_array_checkproc
      if (C1TxWrValid && (C1TxHdr.mdata != CCIP_TX1_WRFENCE)) begin
	 write_check_array[C1TxHdr.mdata] = C1TxHdr.addr;
      end
      if (C1RxWrValid) begin
	 if (write_check_array.exists(C1RxHdr.mdata))
	   write_check_array.delete(C1RxHdr.mdata);
      end
      if (C1RxWrValid) begin
	 if (write_check_array.exists(C0RxHdr.mdata))
	   write_check_array.delete(C0RxHdr.mdata);
      end
   end
`endif

   string dummy_str;

   /*
    * CCI Logger module
    */
   ccip_logger ccip_logger
     (
      // Logger control
      .enable_logger    (cfg.enable_cl_view),
      .finish_logger    (finish_logger     ),
      // Buffer message injection
      .log_string_en    (1'b0),
      .log_string       (dummy_str),
      // CCIP ports
      .clk              (clk             ),
      .SoftReset_n      (SoftReset_n     ),
      .C0TxHdr          (C0TxHdr         ),
      .C0TxRdValid      (C0TxRdValid     ),
      .C1TxHdr          (C1TxHdr         ),
      .C1TxData         (C1TxData        ),
      .C1TxWrValid      (C1TxWrValid     ),
      .C1TxIntrValid    (C1TxIntrValid   ),
      .C2TxHdr          (C2TxHdr         ),
      .C2TxMMIORdValid  (C2TxMMIORdValid ),
      .C2TxData         (C2TxData        ),
      .C0RxMMIOWrValid  (C0RxMMIOWrValid ),
      .C0RxMMIORdValid  (C0RxMMIORdValid ),
      .C0RxData         (C0RxData        ),
      .C0RxHdr          (C0RxHdr         ),
      .C0RxRdValid      (C0RxRdValid     ),
      .C0RxWrValid      (C0RxWrValid     ),
      .C0RxUMsgValid    (C0RxUMsgValid   ),
      .C1RxHdr          (C1RxHdr         ),
      .C1RxWrValid      (C1RxWrValid     ),
      .C1RxIntrValid    (C1RxIntrValid   ),
      .C0TxAlmFull      (C0TxAlmFull     ),
      .C1TxAlmFull      (C1TxAlmFull     )
      );

endmodule // cci_emulator
