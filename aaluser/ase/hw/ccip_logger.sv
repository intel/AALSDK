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
    input int 				     enable_logger,
    input int 				     finish_logger,
    // Buffer message injection
    input logic 			     log_string_en,
    ref string 				     log_string,
    //////////////////////////////////////////////////////////
    // CCI interface
    input logic 			     clk,
    input logic 			     SoftReset_n, 
    // Tx0 channel
    input 				     TxHdr_t C0TxHdr,
    input logic 			     C0TxRdValid,
    // Tx1 channel
    input 				     TxHdr_t C1TxHdr,
    input logic [CCIP_DATA_WIDTH-1:0] 	     C1TxData,
    input logic 			     C1TxWrValid,
    input logic 			     C1TxIntrValid,
    // Tx2 channel
    input 				     MMIOHdr_t C2TxHdr,
    input logic 			     C2TxMMIORdValid,
    input logic [CCIP_MMIO_RDDATA_WIDTH-1:0] C2TxData,
    // Rx0 channel
    input logic 			     C0RxMMIOWrValid,
    input logic 			     C0RxMMIORdValid,
    input logic [CCIP_DATA_WIDTH-1:0] 	     C0RxData,
    input 				     RxHdr_t C0RxHdr,
    input logic 			     C0RxRdValid,
    input logic 			     C0RxWrValid,
    input logic 			     C0RxUMsgValid,
    // Rx1 channel
    input 				     RxHdr_t C1RxHdr,
    input logic 			     C1RxWrValid,
    input logic 			     C1RxIntrValid,
    // Almost full signals
    input logic 			     C0TxAlmFull,
    input logic 			     C1TxAlmFull
    );

   /*
    * ASE Hardware Interface (CCI) logger
    * - Logs CCI transaction into a transactions.tsv file
    * - Watch for "*valid", and write transaction to log name
    */
   // Log file descriptor
   int 					    log_fd;

   // Reset management
   logic 				    SoftReset_n_q;

   // Registers for comparing previous states
   always @(posedge clk) begin
      SoftReset_n_q	<= SoftReset_n;
   end


   /*
    * Buffer channels, request and response types
    */
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

   // Print CSR data
   function string csr_data(int num_bytes, logic [CCIP_DATA_WIDTH-1:0] rx0_data);
      string str_4;
      string str_8;
      string str_64;
      begin
	 case (num_bytes)
	   4 :
	     begin
		str_4.hextoa(rx0_data[31:0]);
		return str_4;
	     end
	   8 :
	     begin
		str_8.hextoa(rx0_data[63:0]);
		return str_8;
	     end
	   64 :
	     begin
		str_64.hextoa(rx0_data[511:0]);
		return str_64;
	     end
	 endcase
      end
   endfunction

   /*
    * Format string to print on screen and file
    */
   function automatic string format_string( ref string transact_type,
					    input int channel
					    );
      string 					   str;      
      begin
	 case (transact_type)
	   "MMIOWrite" :
	     begin
	     end

	   "MMIOReadReq":
	     begin
	     end

	   "MMIOReadResp":
	     begin
	     end

	   "ReadReq":
	     begin
	     end

	   "ReadRsp":
	     begin
	     end

	   "WriteReq":
	     begin
	     end
	   
	   "WriteRsp":
	     begin
	     end
	   
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
      log_fd = $fopen(LOGNAME, "w");

      // Watch CCI port
      forever begin
	 // Indicate Software controlled reset
	 if (SoftReset_n_q != SoftReset_n) begin
	    $fwrite(log_fd, "%d\tSoftReset_n toggled from %b to %b\n", $time, SoftReset_n_q, SoftReset_n);
	 end
	 // Buffer messages
	 if (log_string_en) begin
	    $fwrite(log_fd, log_string);
	 end
	 /////////////////////// CONFIG CHANNEL TRANSACTIONS //////////////////////////
	 /******************* SW -> AFU Config Write *******************/
	 // if (CfgWrValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\tCfgWrite\t%x\t%d bytes\t%s",
	 //    				     $time,
	 //    				     CfgHeader.index,
	 //    				     4^(1 + CfgHeader.num_bytes),
	 //    				     csr_data(4^(1 + CfgHeader.num_bytes), C0RxData)  );
	 //    $fwrite(log_fd, "%d\tCfgWrite\t%x\t%d bytes\t%s",
	 //    	    $time,
	 //    	    CfgHeader.index,
	 //    	    4^(1 + CfgHeader.num_bytes),
	 //    	    csr_data(4^(1 + CfgHeader.num_bytes), C0RxData)  );
	 // end
	 // /*************** SW -> AFU Config Read Request ****************/
	 // if (CfgRdValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\tCfgRdReq\t%x",
	 // 				     $time,
	 // 				     CfgHeader.index);
	 //    $fwrite(log_fd, "%d\tCfgRdReq\t%x",
	 // 	    $time,
	 // 	    CfgHeader.index);
	 // end
	 // /*************** AFU -> SW Config Read Response ***************/
	 // if (CfgRdDataValid) begin
	 // end
	 // //////////////////////// C0 TX CHANNEL TRANSACTIONS //////////////////////////
	 // /******************* AFU -> MEM Read Request ******************/
	 // if (C0TxRdValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x",
	 // 				     $time,
	 // 				     print_channel(C0TxHdr.vc),
	 // 				     print_reqtype(C0TxHdr.reqtype),
	 // 				     C0TxHdr.addr,
	 // 				     C0TxHdr.mdata);
	 //    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\n",
	 // 	    $time,
	 // 	    print_channel(C0TxHdr.vc),
	 // 	    print_reqtype(C0TxHdr.reqtype),
	 // 	    C0TxHdr.addr,
	 // 	    C0TxHdr.mdata);
	 // end
	 // //////////////////////// C1 TX CHANNEL TRANSACTIONS //////////////////////////
	 // /******************* AFU -> MEM Write Request *****************/
	 // if (C1TxWrValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x\t%x",
	 // 				     $time,
	 // 				     print_channel(C1TxHdr.vc),
	 // 				     print_reqtype(C1TxHdr.reqtype),
	 // 				     C1TxHdr.addr,
	 // 				     C1TxHdr.mdata,
	 // 				     C1TxData);
	 //    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\t%x\n",
	 // 	    $time,
	 // 	    print_channel(C1TxHdr.vc),
	 // 	    print_reqtype(C1TxHdr.reqtype),
	 // 	    C1TxHdr.addr,
	 // 	    C1TxHdr.mdata,
	 // 	    C1TxData);
	 // end
	 // //////////////////////// C0 RX CHANNEL TRANSACTIONS //////////////////////////
	 // /******************* MEM -> AFU Read Response *****************/
	 // if (C0RxRdValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x",
	 // 				     $time,
	 // 				     print_channel(C0RxHdr.vc),
	 // 				     print_resptype(C0RxHdr.resptype),
	 // 				     C0RxHdr.mdata,
	 // 				     C0RxData);
	 //    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\n",
	 // 	    $time,
	 // 	    print_channel(C0RxHdr.vc),
	 // 	    print_resptype(C0RxHdr.resptype),
	 // 	    C0RxHdr.mdata,
	 // 	    C0RxData);
	 // end
	 // /****************** MEM -> AFU Write Response *****************/
	 // if (C0RxWrValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x",
	 // 				     $time,
	 // 				     print_channel(C0RxHdr.vc),
	 // 				     print_resptype(C0RxHdr.resptype),
	 // 				     C0RxHdr.mdata);
	 //    $fwrite(log_fd, "%d\t%s\t%s\t%x\n",
	 // 	    $time,
	 // 	    print_channel(C0RxHdr.vc),
	 // 	    print_resptype(C0RxHdr.resptype),
	 // 	    C0RxHdr.mdata);
	 // end
	 // /************* SW -> MEM -> AFU Unordered Message  ************/
	 // if (C0RxUmsgValid) begin
	 // end
	 // /**************** MEM -> AFU Interrupt Response  **************/
	 // if (C0RxIntrValid) begin
	 // end
	 // //////////////////////// C1 RX CHANNEL TRANSACTIONS //////////////////////////
	 // /****************** MEM -> AFU Write Response  ****************/
	 // if (C1RxWrValid) begin
	 //    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x",
	 // 				     $time,
	 // 				     print_channel(C1RxHdr.vc),
	 // 				     print_resptype(C1RxHdr.resptype),
	 // 				     C1RxHdr.mdata);
	 //    $fwrite(log_fd, "%d\t%s\t%s\t%x\n",
	 // 	    $time,
	 // 	    print_channel(C1RxHdr.vc),
	 // 	    print_resptype(C1RxHdr.resptype),
	 // 	    C1RxHdr.mdata);
	 // end
	 // /**************** MEM -> AFU Interrupt Response  **************/
	 // if (C1RxIntrValid) begin
	 // end
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
