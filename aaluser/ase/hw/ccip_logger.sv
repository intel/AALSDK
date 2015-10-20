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
 * Module Info: CCI Transactions Logger
 * Language   : System{Verilog}
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 */

import ase_pkg::*;
`include "platform.vh"

module ccip_logger
  #(
    parameter LOGNAME   = "ccip_transactions.tsv"
    )
   (
    // Configure enable
    input int 			      enable_logger,
    input int 			      finish_logger,
    // Buffer message injection
    // input logic                             log_string_en,
    // input string [1023:0]                   log_string ,
    // CCI interface
    input logic 		      clk,
    input logic 		      sys_reset_n,
    input logic 		      sw_reset_n,
    // C0Tx channel
    input 			      TxHdr_t C0TxHdr,
    input logic 		      C0TxRdValid,
    input logic 		      C0TxAlmFull,
    // C1Tx channel
    input 			      TxHdr_t C1TxHdr,
    input logic [CCIP_DATA_WIDTH-1:0] C1TxData,
    input logic 		      C1TxWrValid,
    input logic 		      C1TxAlmFull,
    input logic 		      C1TxIntrValid,
    // Config channel
    input logic 		      CfgRdData,
    input logic 		      CfgRdDataValid,
    input logic 		      CfgHeader,
    input logic 		      CfgWrValid,
    input logic 		      CfgRdValid,
    // C0Rx channel
    input 			      RxHdr_t C0RxHdr,
    input logic [CCIP_DATA_WIDTH-1:0] C0RxData,
    input logic 		      C0RxRdValid,
    input logic 		      C0RxWrValid,
    input logic 		      C0RxUmsgValid,
    input logic 		      C0RxIntrValid,
    // C1Rx channel
    input 			      RxHdr_t C1RxHdr,
    input logic 		      C1RxWrValid,
    input logic 		      C1RxIntrValid
    );

   /*
    * ASE Hardware Interface (CCI) logger
    * - Logs CCI transaction into a transactions.tsv file
    * - Watch for "*valid", and write transaction to log name
    */
   // Log file descriptor
   int 					    log_fd;

   // Reset management
   logic 				    sys_reset_n_q;
   logic 				    sw_reset_n_q;

   // Registers for comparing previous states
   always @(posedge clk) begin
      sw_reset_n_q	<= sw_reset_n;
      sys_reset_n_q     <= sys_reset_n;
   end


   /*
    * Buffer messages
    */
   // export "DPI-C" task buffer_messages;
   // task buffer_messages (string log_string);
   //    begin
   // 	 $fwrite (log_fd, log_string);
   //    end
   // endtask

   // Print Channel function
   function string print_channel (logic [1:0] vc_sel);
      begin
	 case (vc_sel)
	   2'b00: return "VA ";
	   2'b01: return "VL0";
	   2'b10: return "VH0";
	   2'b11: return "VH1";
	 endcase
      end
   endfunction

   // Print Req Type
   function string print_reqtype (logic [3:0] req);
      begin
	 case (req)
	   CCIP_TX0_RDLINE_S : return "RdLine_S ";
	   CCIP_TX0_RDLINE_I : return "RdLine_I ";
	   CCIP_TX0_RDLINE_E : return "RdLine_E ";
	   CCIP_TX1_WRLINE_I : return "WrLine_I ";
	   CCIP_TX1_WRLINE_M : return "WrLine_M ";
	   CCIP_TX1_WRFENCE  : return "WrFence  ";
	   CCIP_TX1_INTRVALID: return "IntrReq  ";
	   default           : return "* ERROR *";	   
	 endcase
      end
   endfunction

   // Print resp type
   function string print_resptype (logic [3:0] resp);
      begin
	 case (resp)
	   CCIP_RX0_RD_RESP   : return "RdResp   ";
	   CCIP_RX0_WR_RESP   : return "WrResp   ";
	   CCIP_RX1_WR_RESP   : return "WrResp   ";
	   CCIP_RX0_INTR_CMPLT: return "IntrResp ";
	   CCIP_RX1_INTR_CMPLT: return "IntrResp ";
	   default            : return "* ERROR *";
	 endcase
      end
   endfunction


   /*
    * Watcher process
    */
   initial begin : logger_proc
      // Display
      $display("SIM-SV: Transaction Logger started");

      // Open transactions.tsv file
      log_fd = $fopen("transactions.tsv", "w");

      // Headers
      // $fwrite(log_fd, "\tTime\tTransactionType\tChannel\tMetaInfo\tCacheAddr\tData\n");

      // Watch CCI port
      forever begin
	 // Indicate Software controlled reset
	 if (sw_reset_n_q != sw_reset_n) begin
	    $fwrite(log_fd, "%d\tSoftware reset toggled from %b to %b\n", $time, sw_reset_n_q, sw_reset_n);
	 end
	 // If reset toggled, log the event
	 if (sys_reset_n_q != sys_reset_n) begin
	    $fwrite(log_fd, "%d\tSystem reset toggled from %b to %b\n", $time, sys_reset_n_q, sys_reset_n);
	 end
	 // Buffer messages
	 // if (log_string_en) begin
	 //    $fwrite(log_fd, log_string);
	 // end
	 /////////////////////// CONFIG CHANNEL TRANSACTIONS //////////////////////////
	 /******************* SW -> AFU Config Write *******************/
	 if (CfgWrValid) begin
	    if (cfg.enable_cl_view) $display("%d\tCfgWrite\t%x\t%d bytes\t%x",
					     $time,
					     CfgHeader.index,
					     4^(1 + CfgHeader.num_bytes),
					     C0RxData[8*4^(1+CfgHeader.num_bytes)-1:0]);
	    $fwrite(log_fd, "%d\tCfgWrite\t%x\t%d bytes\t%x\n",
		    $time,
		    CfgHeader.index,
		    4^(1 + CfgHeader.num_bytes),
		    C0RxData[8*4^(1+CfgHeader.num_bytes)-1:0]);
	 end
	 /*************** SW -> AFU Config Read Request ****************/
	 if (CfgRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\tCfgRdReq\t%x",
					     $time,
					     CfgHeader.index);
	    $fwrite(log_fd, "%d\tCfgRdReq\t%x",
		    $time,
		    CfgHeader.index);
	 end
	 /*************** AFU -> SW Config Read Response ***************/
	 if (CfgRdDataValid) begin
	 end
	 //////////////////////// C0 TX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* AFU -> MEM Read Request ******************/
	 if (C0TxRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x",
					     $time,					     
					     print_channel(C0TxHdr.vc),
					     print_reqtype(C0TxHdr.reqtype),
					     C0TxHdr.addr,
					     C0TxHdr.mdata);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\n",
		    $time,
		    print_channel(C0TxHdr.vc),
		    print_reqtype(C0TxHdr.reqtype),
		    C0TxHdr.addr,
		    C0TxHdr.mdata);
	 end
	 //////////////////////// C1 TX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* AFU -> MEM Write Request *****************/
	 if (C1TxWrValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x\t%x",
					     $time,					     
					     print_channel(C1TxHdr.vc),
					     print_reqtype(C1TxHdr.reqtype),
					     C1TxHdr.addr,
					     C1TxHdr.mdata,
					     C1TxData);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\t%x\n",
		    $time,
		    print_channel(C1TxHdr.vc),
		    print_reqtype(C1TxHdr.reqtype),
		    C1TxHdr.addr,
		    C1TxHdr.mdata,
		    C1TxData);
	 end
	 //////////////////////// C0 RX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* MEM -> AFU Read Response *****************/
	 if (C0RxRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x",
					     $time,
					     print_channel(C0RxHdr.vc),
					     print_resptype(C0RxHdr.resptype),
					     C0RxHdr.mdata,
					     C0RxData);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\n",
		    $time,
		    print_channel(C0RxHdr.vc),
		    print_resptype(C0RxHdr.resptype),
		    C0RxHdr.mdata,
		    C0RxData);    
	 end
	 /****************** MEM -> AFU Write Response *****************/
	 if (C0RxWrValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x",
					     $time,
					     print_channel(C0RxHdr.vc),
					     print_resptype(C0RxHdr.resptype),
					     C0RxHdr.mdata);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\n",
		    $time,
		    print_channel(C0RxHdr.vc),
		    print_resptype(C0RxHdr.resptype),
		    C0RxHdr.mdata);	    
	 end
	 /************* SW -> MEM -> AFU Unordered Message  ************/
	 if (C0RxUmsgValid) begin
	 end
	 /**************** MEM -> AFU Interrupt Response  **************/
	 if (C0RxIntrValid) begin
	 end
	 //////////////////////// C1 RX CHANNEL TRANSACTIONS //////////////////////////
	 /****************** MEM -> AFU Write Response  ****************/
	 if (C1RxWrValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x",
					     $time,
					     print_channel(C1RxHdr.vc),
					     print_resptype(C1RxHdr.resptype),
					     C1RxHdr.mdata);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\n",
		    $time,
		    print_channel(C1RxHdr.vc),
		    print_resptype(C1RxHdr.resptype),
		    C1RxHdr.mdata);	    
	 end
	 /**************** MEM -> AFU Interrupt Response  **************/
	 if (C1RxIntrValid) begin
	 end
	 ////////////////////////////// FINISH command ////////////////////////////////
	 if (finish_logger == 1) begin
	    $fclose(log_fd);
	 end
	 //////////////////////////////////////////////////////////////////////////////
	 // Wait till next clock
	 @(posedge clk);
      end
   end

endmodule
