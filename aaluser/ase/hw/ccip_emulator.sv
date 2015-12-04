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
   output logic vl_clk_LPdomain_64ui,
   output logic vl_clk_LPdomain_32ui,
   output logic vl_clk_LPdomain_16ui,
   output logic ffs_LP16ui_afu_SoftReset_n,
   input 	t_if_ccip_Tx ffs_LP16ui_sTxData_afu,
   output 	t_if_ccip_Rx ffs_LP16ui_sRxData_afu
   );


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
      // ffs_LP16ui_sRxData_afu.C0Hdr <= { C0RxHdr.vc,
      // 					C0RxHdr.poison,
      // 					C0RxHdr.hitmiss,
      // 					1'b0,
      // 					C0RxHdr.clnum,
      // 					C0RxHdr.resptype,
      // 					C0RxHdr.mdata
      // 					};
      ffs_LP16ui_sRxData_afu.C0Hdr <= t_ccip_RspMemHdr'(C0RxHdr);
      ffs_LP16ui_sRxData_afu.C0Data <= C0RxData;
      ffs_LP16ui_sRxData_afu.C0WrValid <= C0RxWrValid;
      ffs_LP16ui_sRxData_afu.C0RdValid <= C0RxRdValid;
      ffs_LP16ui_sRxData_afu.C0UMsgValid <= C0RxUMsgValid;
      //    ffs_LP16ui_sRxData_afu.C1Hdr  <= { C1RxHdr.vc,
      // 					 C1RxHdr.poison,
      // 					 C1RxHdr.hitmiss,
      // 					 1'b0,
      // 					 C1RxHdr.clnum,
      // 					 C1RxHdr.resptype,
      // 					 C1RxHdr.mdata
      // 					 };
      ffs_LP16ui_sRxData_afu.C1Hdr <= t_ccip_RspMemHdr'(C1RxHdr);
      ffs_LP16ui_sRxData_afu.C1WrValid <= C1RxWrValid;
      ffs_LP16ui_sRxData_afu.C1IntrValid <= C1RxIntrValid;
      // Tx OUT
      // { C0TxHdr.vc,
      // 	C0TxHdr.sop,
      // 	C0TxHdr.rsvd70,
      // 	C0TxHdr.len,
      // 	C0TxHdr.reqtype,
      // 	C0TxHdr.rsvd63_58,
      // 	C0TxHdr.addr,
      // 	C0TxHdr.mdata  } <= ffs_LP16ui_sTxData_afu.C0Hdr;
      C0TxHdr <= TxHdr_t'(ffs_LP16ui_sTxData_afu.C0Hdr);
      C0TxRdValid <= ffs_LP16ui_sTxData_afu.C0RdValid;
      //    { C1TxHdr.vc,
      // 	C1TxHdr.sop,
      // 	C1TxHdr.rsvd70,
      // 	C1TxHdr.len,
      // 	C1TxHdr.reqtype,
      // 	C1TxHdr.rsvd63_58,
      // 	C1TxHdr.addr,
      // 	C1TxHdr.mdata  } <= ffs_LP16ui_sTxData_afu.C1Hdr;
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
   // import "DPI-C" function void ccip_emulator_scope_function();
   // ASE Initialize function
   import "DPI-C" context task ase_init();
   // Indication that ASE is ready
   import "DPI-C" function void ase_ready();
   // Global listener function
   import "DPI-C" context task ase_listener();

   // ASE config data exchange (read from ase.cfg)
   export "DPI-C" task ase_config_dex;

   // CSR Write Dispatch
   // export "DPI-C" task csr_write_dispatch;
   // Unordered message dispatch
   // export "DPI-C" task umsg_dispatch;

   // MMIO dispatch
   export "DPI-C" task mmio_dispatch;

   // CAPCM initilize
   // import "DPI-C" context task capcm_init();

   // Start simulation structures teardown
   import "DPI-C" context task start_simkill_countdown();
   // Signal to kill simulation
   export "DPI-C" task simkill;

   // Signal to cci_logger to write string to log file
   // export "DPI-C" task buffer_messages;

   // CONFIG, SCRIPT DEX operations
   import "DPI-C" function void sv2c_config_dex(string str);
   import "DPI-C" function void sv2c_script_dex(string str);

   // Data exchange for READ, WRITE system/CAPCM memory line
   import "DPI-C" function void rd_memline_dex(inout cci_pkt foo, inout int cl_addr, inout int mdata );
   import "DPI-C" function void wr_memline_dex(inout cci_pkt foo, inout int cl_addr, inout int mdata, inout bit [511:0] wr_data );

   // Software controlled process - run clocks
   export "DPI-C" task run_clocks;

   // Declare packets for each channel
   cci_pkt rx0_pkt, rx1_pkt;

   // Scope generator
   // initial ccip_emulator_scope_function();
   initial scope_function();


   /*
    * FUNCTION: Convert CAPCM_GB_SIZE to NUM_BYTES
    */
   function automatic longint conv_gbsize_to_num_bytes(int gb_size);
      begin
	 return (gb_size*1024*1024*1024);
      end
   endfunction


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


   /*
    * FUNCTION: Return absolute value
    */
   function automatic int abs_val(int num);
      begin
	 return (num < 0) ? ~num : num;
      end
   endfunction

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
   logic 			  clk_8ui;

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
   typedef enum 		  {RxIdle, RxAFUCSRWrite, RxQLPCSRWrite, RxReadResp, RxWriteResp, RxUmsg, RxIntrResp}
				  RxGlue_StateEnum;
   RxGlue_StateEnum rx0_state;
   RxGlue_StateEnum rx1_state;


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
   	 clk_8ui = 0;
   	 forever begin
   	    #`CLK_8UI_TIME;
   	    clk_8ui = 1'b0;
   	    #`CLK_8UI_TIME;
   	    clk_8ui = 1'b1;
   	 end
      end
   end

   // 200 Mhz clock
   always @(posedge clk_8ui) begin : clk_rollover_ctr
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
   logic [MMIOREQ_FIFO_WIDTH-1:0] 	     mmioreq_dout;
   logic 				     mmioreq_write;
   logic 				     mmioreq_pop;
   logic 				     mmioreq_read;
   logic 				     mmioreq_valid;
   logic 				     mmioreq_full;
   logic 				     mmioreq_empty;

   logic [CCIP_MMIO_INDEX_WIDTH-1:0] 	     cwlp_header;
   logic [CCIP_DATA_WIDTH-1:0] 		     cwlp_data;
   logic 				     cwlp_wrvalid;
   logic 				     cwlp_rdvalid;

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
	    if (mmio_pkt.write_en == MMIO_WRITE) begin
	       hdr.index  = {2'b0, mmio_pkt.addr[15:2]};
	       hdr.poison = 1'b0;
	       hdr.tid    = 9'b0;
	       if (mmio_pkt.width == 32) begin
		  hdr.len = 2'b0;
		  cwlp_header = CCIP_CFG_HDR_WIDTH'(hdr);
		  cwlp_data = {480'b0, mmio_pkt.data[31:0]};
	       end
	       else if (mmio_pkt.width == 64) begin
		  hdr.len = 2'b01;
		  cwlp_header = CCIP_CFG_HDR_WIDTH'(hdr);
		  cwlp_data = {448'b0, mmio_pkt.data[63:0]};
	       end
	       cwlp_wrvalid = 1;
	       cwlp_rdvalid = 0;
	    end
	    else if (mmio_pkt.write_en == MMIO_READ_REQ) begin
	       cwlp_data    = 0;
	       hdr.index    = {2'b0, mmio_pkt.addr[15:2]};
	       hdr.len      = 2'b01;
	       hdr.poison   = 1'b0;
	       hdr.tid      = mmio_tid_counter;
	       cwlp_header  = CCIP_CFG_HDR_WIDTH'(hdr);
	       cwlp_wrvalid = 0;
	       cwlp_rdvalid = 1;
	       tid_array[ mmio_tid_counter ] = hdr.index;
    	       mmio_tid_counter  = mmio_tid_counter + 1;
	    end
	    @(posedge clk);
	    cwlp_wrvalid = 0;
	    cwlp_rdvalid = 0;
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
      .rd_en      ( mmioreq_pop ),
      .data_out   ( mmioreq_dout ),
      .data_out_v ( mmioreq_valid ),
      .alm_full   ( mmioreq_full ),
      .full       (  ),
      .empty      ( mmioreq_empty ),
      .count      (  ),
      .overflow   (  ),
      .underflow  (  )
      );

   assign mmioreq_pop  = 0;
 // ~mmioreq_empty && cfg_pop;
   assign mmio_wrvalid = ~mmioreq_empty;
   assign mmio_data    = mmioreq_dout[CCIP_DATA_WIDTH-1:0];
   assign mmio_hdr     = CfgHdr_t'( mmioreq_dout[(CCIP_DATA_WIDTH+CCIP_CFG_HDR_WIDTH-1):CCIP_DATA_WIDTH] );


   /*
    * MMIO Read response
    */
//    parameter int MMIORESP_FIFO_WIDTH = CCIP_MMIO_TID_WIDTH + CCIP_MMIO_RDDATA_WIDTH;

//    logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_din;
//    logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_dout;
//    logic 			   mmioresp_write;
//    logic 			   mmioresp_pop;
//    logic 			   mmioresp_read;
//    logic 			   mmioresp_valid;
//    logic 			   mmioresp_full;
//    logic 			   mmioresp_empty;

//    import "DPI-C" function void mmio_update_dex(int mmioaddr64, bit [63:0] mmiodata );

//    // Response staging FIFO
//    ase_fifo
//      #(
//        .DATA_WIDTH     ( MMIORESP_FIFO_WIDTH ),
//        .DEPTH_BASE2    ( 4 ),
//        .ALMFULL_THRESH ( 10 )
//        )
//    mmioresp_fifo
//      (
//       .clk        ( clk ),
//       .rst        ( rst ),
//       .wr_en      ( mmioresp_write ),
//       .data_in    ( mmioresp_din ),
//       .rd_en      ( mmioresp_pop ),
//       .data_out   ( mmioresp_dout ),
//       .data_out_v ( mmioresp_valid ),
//       .alm_full   ( mmioresp_full ),
//       .full       (  ),
//       .empty      ( mmioresp_empty ),
//       .count      (  ),
//       .overflow   (  ),
//       .underflow  (  )
//       );

//    assign mmioresp_din = { mmio_rsptid, mmio_rspdata };
//    assign mmioresp_write = mmio_rspvalid;

   // FIFO writes to memory
   // always @(posedge clk) begin
   //    if (rst) begin
   // 	 mmio_dispatch (1, {0, 0, 0, 0, 0});
   //    end
   //    else begin
   // 	 // if (~mmioresp_empty) begin
   // 	 //    mmioresp_read <= 1;
   // 	 //    mmio_update_dex( {tid_array[ mmioresp_dout[MMIORESP_FIFO_WIDTH-1:CCIP_MMIO_RDDATA_WIDTH] ], 2'b0} , mmioresp_dout[CCIP_MMIO_RDDATA_WIDTH-1:0]);
   // 	 // end
   // 	 // else begin
   // 	 //    mmioresp_read <= 0;
   // 	 // end
   //    end
   // end



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
	 cfg.ase_mode           = cfg_in.ase_mode         ;
	 cfg.ase_timeout        = cfg_in.ase_timeout      ;
	 cfg.ase_num_tests      = cfg_in.ase_num_tests    ;
	 cfg.enable_reuse_seed  = cfg_in.enable_reuse_seed;
	 cfg.num_umsg_log2      = cfg_in.num_umsg_log2    ;
	 cfg.enable_cl_view     = cfg_in.enable_cl_view   ;
	 // cfg.enable_capcm       = cfg_in.enable_capcm     ;
	 // cfg.memmap_sad_setting = cfg_in.memmap_sad_setting    ;
      end
   endtask



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
	 // `BEGIN_YELLOW_FONTCOLOR;
	 // $display("Transaction counts => ");
	 // $display("\tConfigs    = %d", ase_rx0_cfgvalid_cnt );
	 // $display("\tRdReq      = %d", ase_tx0_rdvalid_cnt );
	 // $display("\tRdResp     = %d", ase_rx0_rdvalid_cnt );
	 // $display("\tWrReq      = %d", ase_tx1_wrvalid_cnt );
	 // $display("\tWrResp-CH0 = %d", ase_rx0_wrvalid_cnt );
	 // $display("\tWrResp-CH1 = %d", ase_rx1_wrvalid_cnt );
	 // $display("\tWrFence    = %d", ase_tx1_wrfence_cnt );
	 // $display("\tUMsgHint   = %d", ase_rx0_umsghint_cnt );
	 // $display("\tUMsgData   = %d", ase_rx0_umsgdata_cnt );
	 // `END_YELLOW_FONTCOLOR;

	 // Valid Count
// `ifdef ASE_DEBUG
//  `ifdef ASE_RANDOMIZE_TRANSACTIONS
// 	 // Print errors
// 	 `BEGIN_RED_FONTCOLOR;
// 	 if (ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt)
// 	   $display("\tREADs  : Response counts dont match request count !!");
// 	 if (ase_tx1_wrvalid_cnt != (ase_rx0_wrvalid_cnt + ase_rx1_wrvalid_cnt))
// 	   $display("\tWRITEs : Response counts dont match request count !!");
// 	 `END_RED_FONTCOLOR;
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


   /*
    * Task : String logs to cci_logger
    */
   // logic cci_logger_msg_en;
   // string cci_logger_msg;

   // task buffer_messages (int init, string log_string);
   //    begin
   // 	 if (init == 1) begin
   // 	    cci_logger_msg_en = 0;
   // 	 end
   // 	 else begin
   // 	    cci_logger_msg = log_string;
   // 	    cci_logger_msg_en = 1;
   // 	    @(posedge clk);
   // 	    cci_logger_msg_en = 0;
   // 	 end
   //    end
   // endtask


   /*
    * Unified message watcher daemon
    */
   always @(posedge clk) begin : daemon_proc
//      if (lp_initdone) begin
	 ase_listener();
//      end
   end


   /* *******************************************************************
    * Staging incoming requests for fulfillment
    *            | LOWLAT | PCIE
    * OME2       |   1    |  0
    * BDX+FPGA   |   1    |  2
    *
    * CCIP is assumed to be an overall unordered interface with QPI +
    * n*PCIE downstream ports. ASE intends to present one CCIP port to
    * the AFU
    *
    * *******************************************************************/
   // CAFU->ASE CH0
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_ch0_header;
   logic 			 cf2as_latbuf_ch0_pop;
   logic 			 cf2as_latbuf_ch0_read;
   logic 			 cf2as_latbuf_ch0_empty;
   logic 			 cf2as_latbuf_ch0_empty_q;
   logic 			 cf2as_latbuf_ch0_valid;
   logic [31:0] 		 cf2as_latbuf_ch0_claddr;
   logic [13:0] 		 cf2as_latbuf_ch0_meta;

   // CAFU->ASE CH0
   logic [CCIP_TX_HDR_WIDTH-1:0] cf2as_latbuf_ch1_header;
   logic [CCIP_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data;
   logic [CCIP_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data_0;
   logic [CCIP_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data_1;
   logic 			 cf2as_latbuf_ch1_pop;
   logic 			 cf2as_latbuf_ch1_read_0;
   logic 			 cf2as_latbuf_ch1_read_1;
   logic 			 cf2as_latbuf_ch1_read;
   logic 			 cf2as_latbuf_ch1_empty;
   logic 			 cf2as_latbuf_ch1_empty_q;
   logic 			 cf2as_latbuf_ch1_valid;
   logic [41:0] 		 cf2as_latbuf_ch1_claddr;
   logic [41:0] 		 cf2as_latbuf_ch1_claddr_0;
   logic [41:0] 		 cf2as_latbuf_ch1_claddr_1;
   logic [15:0] 		 cf2as_latbuf_ch1_meta;
   logic [15:0] 		 cf2as_latbuf_ch1_meta_0;
   logic [15:0] 		 cf2as_latbuf_ch1_meta_1;

   // CAFU->ASE CH0 (TX0)
   // Composed as {header, data}
   // Latency scoreboard (for latency modeling and shuffling)
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
      .meta_out		( cf2as_latbuf_ch0_header ),
      .data_out		(  ),
      .valid_out	( cf2as_latbuf_ch0_valid ),
      .read_en		( cf2as_latbuf_ch0_pop ),
      .empty		( cf2as_latbuf_ch0_empty ),
      .full             ( tx_c0_almostfull ),
      .overflow         ( tx0_overflow ),
      .underflow        ( tx0_underflow ),
      .count            ( )
      );


   // CAFU->ASE CH1 (TX1)
   // Latency scoreboard (latency modeling and shuffling)
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
      .meta_out		( cf2as_latbuf_ch1_header ),
      .data_out		( cf2as_latbuf_ch1_data ),
      .valid_out	( cf2as_latbuf_ch1_valid ),
      .read_en		( cf2as_latbuf_ch1_pop ),
      .empty		( cf2as_latbuf_ch1_empty ),
      .full             ( tx_c1_almostfull ),
      .overflow         ( tx1_overflow ),
      .underflow        ( tx1_underflow ),
      .count            ( )
      );


   /*
    * Return response channel
    * PROBLEM: MUXing between channels 0 and 1 causes dropped transactions
    *          Replacing with FIFO doesnt seem to change occurance of problem
    *          Restricting write responses to TX1 seems to be a temporary solution
    *
    * DIVE:
    * - Problem seems to be when ch0_write gets dropped, conditions unknown
    */
   int 	 tx_to_rx_channel;

   // TX-CH1 must select RX-CH0 or RX-CH1 channels for fulfillment
   // Since requests on TX1 can return either via RX0 or RX1, this is needed
   // always @(posedge clk) begin
   always @(posedge clk) begin : channel_random_proc
      if (~sys_reset_n) begin
	 tx_to_rx_channel	<= 1;
      end
      else if (cf2as_latbuf_ch1_valid) begin
	 // tx_to_rx_channel	<= abs_val($random) % 2;
	 tx_to_rx_channel	<= 1;
	 // tx_to_rx_channel	<= 0;
      end
   end


   /* *******************************************************************
    * Response path management
    * - as2cf_fifo_ch0
    * - as2cf_fifo_ch1
    *
    * *******************************************************************/
   parameter int 		 ASE_RX0_PATHWIDTH = 5 + CCIP_RX_HDR_WIDTH + CCIP_DATA_WIDTH;
   parameter int 		 ASE_RX1_PATHWIDTH = 2 + CCIP_RX_HDR_WIDTH;

   logic [ASE_RX0_PATHWIDTH-1:0] as2cf_fifo_ch0_din;
   logic [ASE_RX0_PATHWIDTH-1:0] as2cf_fifo_ch0_dout;
   logic 			 as2cf_fifo_ch0_write;
   logic 			 as2cf_fifo_ch0_read;
   logic 			 as2cf_fifo_ch0_full;
   logic 			 as2cf_fifo_ch0_empty;
   logic 			 as2cf_fifo_ch0_overflow;
   logic 			 as2cf_fifo_ch0_underflow;
   logic 			 as2cf_fifo_ch0_valid;

   logic [ASE_RX1_PATHWIDTH-1:0] as2cf_fifo_ch1_din;
   logic [ASE_RX1_PATHWIDTH-1:0] as2cf_fifo_ch1_dout;
   logic 			 as2cf_fifo_ch1_write;
   logic 			 as2cf_fifo_ch1_read;
   logic 			 as2cf_fifo_ch1_full;
   logic 			 as2cf_fifo_ch1_empty;
   logic 			 as2cf_fifo_ch1_overflow;
   logic 			 as2cf_fifo_ch1_underflow;
   logic 			 as2cf_fifo_ch1_valid;


   // CH0 coded as {mmiowrvalid, mmiordvalid, rdvalid, wrvalid, umsgvalid, hdr, data}
   ase_fifo
     #(
       .DATA_WIDTH (ASE_RX0_PATHWIDTH)
       )
   as2cf_fifo_ch0
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( as2cf_fifo_ch0_write ),
      .data_in    ( as2cf_fifo_ch0_din ),
      .rd_en      ( as2cf_fifo_ch0_read ),
      .data_out   ( as2cf_fifo_ch0_dout ),
      .data_out_v ( as2cf_fifo_ch0_valid ),
      .alm_full   ( as2cf_fifo_ch0_full ),
      .full       ( ),
      .empty      ( as2cf_fifo_ch0_empty ),
      .count      ( ),
      .overflow   ( as2cf_fifo_ch0_overflow ),
      .underflow  ( as2cf_fifo_ch0_underflow )
      );

   // CH1 coded as {intrvalid, wrvalid, hdr}
   ase_fifo
     #(
       .DATA_WIDTH (ASE_RX1_PATHWIDTH)
       )
   as2cf_fifo_ch1
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( as2cf_fifo_ch1_write ),
      .data_in    ( as2cf_fifo_ch1_din ),
      .rd_en      ( as2cf_fifo_ch1_read ),
      .data_out   ( as2cf_fifo_ch1_dout ),
      .data_out_v ( as2cf_fifo_ch1_valid ),
      .alm_full   ( as2cf_fifo_ch1_full ),
      .full       ( ),
      .empty      ( as2cf_fifo_ch1_empty ),
      .count      ( ),
      .overflow   ( as2cf_fifo_ch1_overflow ),
      .underflow  ( as2cf_fifo_ch1_underflow )
      );


   /*
    * RX0 Channel management
    * - Read response
    * - Can block due
    */
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 sw_reset_trig <= 1'b0;
      end
      else begin
	 sw_reset_trig <= 1'b0;
      end
   end


   /*
    * RX1 Channel management
    *
    */



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
	 first_transaction_seen	<= 1'b1;
      end
   end

   // Inactivity management - killswitch
   always @(posedge clk) begin : call_simkill_countdown
      if((inactivity_found==1'b1) && (cfg.ase_timeout != 0)) begin
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
      mmio_dispatch (1, '{0, 0, 0, 0, 0});
      
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
      wait (lp_initdone == 1'b1);
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


   /*
    * CCI Logger module
    */
   // ccip_logger ccip_logger
   //   (
   //    // Logger control
   //    .enable_logger    (cfg.enable_cl_view),
   //    .finish_logger    (finish_logger     ),
   //    // CCIP ports
   //    .clk            (clk               ),
   //    .sys_reset_n    (sys_reset_n       ),
   //    .sw_reset_n     (SoftReset_n       ),
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

endmodule // cci_emulator
