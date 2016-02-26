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
 * Module Info: CCI Sniffer (Rules checker, Hazard detector and warning)
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * Description:
 * ************
 * XZ checker:
 * Checks the TX signals in AFU to see if 'X' or 'z' is validated. ASE
 * does not validated 'X', 'z' on RX channels, these will not be
 * checked. When a violation is discovered, it will be printed in a
 * message,
 *
 * Hazard sniffer:
 * In the strictest sense, if there are multiple transactions to a
 * certain cache line
 *
 * All warnings are logged in warnings.log
 *
 * FIXME list
 * - Sop & cllen pattern matching
 * - Atomic must pass through VL0 only, else error out
 */

import ase_pkg::*;
`include "platform.vh"

module ccip_sniffer
  #(
    parameter ERROR_LOGNAME = "ase_errors.log",
    parameter WARN_LOGNAME  = "ase_warnings.log"
    )
   (
    // Configure enable
    input int 				     finish_logger,
    // Buffer message injection
    // input logic 			     log_string_en,
    // ref string 				     log_string,
    // -------------------------------------------------------- //
    // Channel overflow/realfull checks
    input logic 			     cf2as_ch0_realfull,
    input logic 			     cf2as_ch1_realfull,
    // -------------------------------------------------------- //
    ///////////////////// CCI interface //////////////////////////
    input logic 			     clk,
    input logic 			     SoftReset,
    // Tx0 channel
    input 				     TxHdr_t C0TxHdr,
    input logic 			     C0TxValid,
    // Tx1 channel
    input 				     TxHdr_t C1TxHdr,
    input logic [CCIP_DATA_WIDTH-1:0] 	     C1TxData,
    input logic 			     C1TxValid,
    // Tx2 channel
    input 				     MMIOHdr_t C2TxHdr,
    input logic 			     C2TxMmioRdValid,
    input logic [CCIP_MMIO_RDDATA_WIDTH-1:0] C2TxData,
    // Rx0 channel
    input 				     RxHdr_t C0RxHdr,
    input logic [CCIP_DATA_WIDTH-1:0] 	     C0RxData,
    input logic 			     C0RxMmioWrValid,
    input logic 			     C0RxMmioRdValid,
    input logic 			     C0RxRspValid,
    // Rx1 channel
    input 				     RxHdr_t C1RxHdr,
    input logic 			     C1RxRspValid,
    // Almost full signals
    input logic 			     C0TxAlmFull,
    input logic 			     C1TxAlmFull
    );

   // File descriptors
   int 					     fd_warn;
   //int 				     fd_err;
   
   // FD open
   initial begin
      $display ("SIM-SV : Protocol Checker initialized");

      // Open log files
      fd_warn = $fopen(WARN_LOGNAME, "w");
      // fd_err  = $fopen(ERROR_LOGNAME, "w");

      // Wait until finish logger
      wait (finish_logger == 1);

      // Close loggers
      $display ("SIM-SV : Closing Protocol checker");
      $fclose(fd_warn);
   end

   // any valid signals
   logic tx_valid;
   logic rx_valid;

   assign tx_valid = C0TxValid    |
		     C1TxValid    |
		     C2TxMmioRdValid;

   assign rx_valid = C0RxRspValid    |
		     C0RxMmioRdValid |
		     C0RxMmioWrValid |
		     C1RxRspValid;
      

   // If reset is high, and transactions are asserted
   always @(posedge clk) begin
      if (SoftReset) begin
	 if (tx_valid | rx_valid) begin
	    `BEGIN_RED_FONTCOLOR;
	    $display("SIM-SV: [WARN] Transaction was sent when SoftReset was HIGH, this will be ignored");
	    $fwrite(fd_warn, "%d | Transaction was sent when SoftReset was HIGH, this will be ignored\n", $time);
	    `END_RED_FONTCOLOR;
	 end
      end
   end


   /*
    * UNDEF & HIIMP checker
    */
   // Z and X flags
   logic 			   xz_tx0_flag;
   logic 			   xz_tx1_flag;
   logic 			   xz_tx2_flag;
   logic 			   xz_rx0_flag;
   logic 			   xz_rx1_flag;
   logic 			   xz_cfg_flag;

   /*
    * Rd/Wr Valid signals
   //  */ 
   // logic 			   C0TxRdValid;
   // logic 			   C1TxWrValid;
   
   // always @(*) begin
   //    if (C0TxValid && (C0TxHdr.reqtype == ASE_RDLINE_S))
   // 	C0TxRdValid <= 
   // end
   
   
   /*
    * TX checker files
    */
   CfgHdr_t C0RxCfg;
   assign C0RxCfg = CfgHdr_t'(C0RxHdr);

   assign xz_tx0_flag = ( ^{C0TxHdr.vc,              C0TxHdr.len, C0TxHdr.reqtype, C0TxHdr.addr, C0TxHdr.mdata} )   && C0TxValid ;
   assign xz_tx1_flag = ( ^{C1TxHdr.vc, C1TxHdr.sop, C1TxHdr.len, C1TxHdr.reqtype, C1TxHdr.addr, C1TxHdr.mdata} )   && C1TxValid ;
   assign xz_tx2_flag = ( ^{C2TxHdr.tid, C2TxData})                                                   && C2TxMmioRdValid ;

   assign xz_rx0_flag = ( ^{C0RxHdr.vc_used, C0RxHdr.hitmiss,                 C0RxHdr.clnum, C0RxHdr.resptype, C0RxHdr.mdata, C0RxData}  && C0RxRspValid );
   assign xz_rx1_flag = ( ^{C1RxHdr.vc_used, C1RxHdr.hitmiss, C1RxHdr.format, C1RxHdr.clnum, C1RxHdr.resptype, C1RxHdr.mdata}            && C1RxRspValid );

   assign xz_cfg_flag = ( ^{C0RxCfg.index, C0RxCfg.len, C0RxCfg.tid, C0RxData} ) && (C0RxMmioWrValid | C0RxMmioRdValid);


   // FUNCTION: XZ message
   function void print_xz_message ( string channel_name );
      begin
   	 // Set message
   	 `BEGIN_RED_FONTCOLOR;
   	 $display ("SIM-SV: %d | X or Z found on %s is not recommended.", $time, channel_name);
   	 `END_RED_FONTCOLOR;
   	 $fwrite( fd_warn, " %d | X or Z found on %s => \n", $time, channel_name);
   	 $fwrite( fd_warn, "    | This is not recommended, and can have unintended activity\n");
      end
   endfunction


   // Message call
   always @(posedge clk) begin
      if ((xz_tx0_flag == `VLOG_HIIMP) || (xz_tx0_flag == `VLOG_UNDEF)) begin
   	 print_xz_message ( "C0Tx" );
      end
      if ((xz_tx1_flag == `VLOG_HIIMP) || (xz_tx1_flag == `VLOG_UNDEF)) begin	 
   	 print_xz_message ( "C1Tx" );
      end
      if ((xz_tx2_flag == `VLOG_HIIMP) || (xz_tx2_flag == `VLOG_UNDEF)) begin
   	 print_xz_message ( "C2Tx" );
      end
      if ((xz_rx0_flag == `VLOG_HIIMP) || (xz_rx0_flag == `VLOG_UNDEF)) begin
   	 print_xz_message ( "C0Rx" );
      end
      if ((xz_rx1_flag == `VLOG_HIIMP) || (xz_rx1_flag == `VLOG_UNDEF)) begin
   	 print_xz_message ( "C1Rx" );
      end
      if ((xz_cfg_flag == `VLOG_HIIMP) || (xz_cfg_flag == `VLOG_UNDEF)) begin
   	 print_xz_message ( "C0RxMmio" );
      end
   end

   
   /*
    * SOP & clnum protocol not followed
    */
   // C1TxHdr structure printout
   // function void print_c1txhdr_format();
   //    begin
	 
   //    end
   // endfunction

   /*
    * Full {0,1} signaling
    */ 
   always @(posedge clk) begin
      if (cf2as_ch0_realfull && C0TxValid) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV: [WARNING] Transaction was possibly dropped due to C0Tx Overflow");	 
	 $display("        [WARNING] Transaction was pushed in when port was full");	     
	 `END_RED_FONTCOLOR;
	 $fwrite(fd_warn, "SIM-SV: [WARNING] Transaction was possibly dropped due to C0Tx Overflow");
	 $fwrite(fd_warn, "        [WARNING] Transaction was pushed in when port was full");         
      end
      if (cf2as_ch1_realfull && C1TxValid) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV: [WARNING] Transaction was possibly dropped due to C1Tx Overflow");	 
	 $display("        [WARNING] Transaction was pushed in when port was full");	 
	 `END_RED_FONTCOLOR;
	 $fwrite(fd_warn, "SIM-SV: [WARNING] Transaction was possibly dropped due to C1Tx Overflow");
	 $fwrite(fd_warn, "        [WARNING] Transaction was pushed in when port was full");         
      end
   end
   

   /* 
    * C1TX beat check
    */ 
   // logic [1:0] expected_clnum;
   // logic       expected_sop;
   logic       c1tx_beat_in_progress;

   // always @(posedge clk) begin
   //    if (rst) begin
   // 	 c1tx_beat_in_progress <= 0;
   //    end
   //    else if (C1TxWrValid) begin
   // 	 if (C1TxHdr.sop && C1TxHdr.len != ASE_1CL) begin
   // 	    c1tx_beat_in_progress <= 0;
   // 	 end
   // 	 else if (C1TxHdr.sop && C1TxHdr.len == ASE_3CL) begin
   // 	    c1tx_beat_in_progress <= 0;
   // 	 end
   // 	 else if (C1TxHdr.sop && ((C1TxHdr.len == ASE_2CL)||(C1TxHdr.len == ASE_4CL))) begin
   // 	    c1tx_beat_in_progress <= 1;	    
   // 	 end
   // 	 else if (c1tx_beat_in_progress) begin
   // 	 end
   //    end
   //    else begin
   // 	 c1tx_beat_in_progress <= 0;
   //    end
   // end

   /*
    * Illegal request type
    */
   always @(posedge clk) begin
   end

   /*
    * Multi-cache READ line request checks
    */
   always @(posedge clk) begin : c0tx_mcl_check
      // ------------------------------------------------------------ //
      // If Read request
      if ( C0TxValid && ((C1TxHdr.reqtype == ASE_RDLINE_S)||(C0TxHdr.reqtype == ASE_RDLINE_S)) )  begin
	 // ------------------------------------------------------------ //
	 // Tx0 - 3 CL request illegal
	 if (C0TxHdr.len == ASE_3CL) begin
	    `BEGIN_RED_FONTCOLOR;
	    $display(" [ERROR] %d | Read Request on C0Tx for 3 cachelines is ILLEGAL !", $time);
	    $display(" [ERROR] %d | In CCI-P specificaton document, please see Multi-cacheline requests", $time);
	    $display(" [ERROR] %d | Simulator will shut down now !\n", $time);
	    `END_RED_FONTCOLOR;
	    $fwrite(fd_warn, "%d | Read Request on C0Tx for 3 cachelines is ILLEGAL !", $time);
	    $fwrite(fd_warn, "%d | In CCI-P specificaton document, please see Multi-cacheline requests", $time);
	    $fwrite(fd_warn, "%d | Simulator will shut down now !\n", $time);
	    start_simkill_countdown();
	 end // if (C0TxWrValid && (C0TxHdr.clnum == ASE_3CL))
	 // ------------------------------------------------------------ //
	 // Address alignment check
	 if ( (C0TxHdr.addr[0] != 1'b0) && (C0TxHdr.len == ASE_2CL) ) begin
	    `BEGIN_RED_FONTCOLOR;	    
	    $display(" [ERROR] %d | Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ", $time);
	    $display(" [ERROR] %d | Simulator will shut down now !\n", $time);
	    `END_RED_FONTCOLOR;
	    $fwrite(fd_warn, " %d | Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ", $time);
	    $fwrite(fd_warn, " %d | Simulator will shut down now !\n", $time);
	    start_simkill_countdown();	    
	 end
	 else if ( (C0TxHdr.addr[1:0] != 2'b0) && (C0TxHdr.len == ASE_4CL) ) begin
	    `BEGIN_RED_FONTCOLOR;	    
	    $display(" [ERROR] %d | Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ", $time);
	    $display(" [ERROR] %d | Simulator will shut down now !\n", $time);
	    `END_RED_FONTCOLOR;
	    $fwrite(fd_warn, " %d | Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ", $time);
	    $fwrite(fd_warn, " %d | Simulator will shut down now !\n", $time);
	    start_simkill_countdown();	    
	 end
	 // ------------------------------------------------------------ //
      end // if (C0TxValid && ((C1TxHdr.reqtype == ASE_RDLINE_S)||(C1TxHdr.reqtype == ASE_RDLINE_S)))
   end // block: c0tx_mcl_check

   
   /*
    * Check memory transactions in flight, maintain active list
    */


endmodule // cci_sniffer
