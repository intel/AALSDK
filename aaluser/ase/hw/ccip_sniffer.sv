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
    // input int 				     enable_logger,
    input int 				     finish_logger,
    // Buffer message injection
    // input logic 			     log_string_en,
    // ref string 			     log_string,
    //////////////////////////////////////////////////////////
    // CCI interface
    input logic 			     clk,
    input logic 			     SoftReset,
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
    input logic 			     C2TxMmioRdValid,
    input logic [CCIP_MMIO_RDDATA_WIDTH-1:0] C2TxData,
    // Rx0 channel
    input logic 			     C0RxMmioWrValid,
    input logic 			     C0RxMmioRdValid,
    input logic [CCIP_DATA_WIDTH-1:0] 	     C0RxData,
    input 				     RxHdr_t C0RxHdr,
    input logic 			     C0RxRdValid,
    // input logic 			     C0RxWrValid,
    input logic 			     C0RxUMsgValid,
    // Rx1 channel
    input 				     RxHdr_t C1RxHdr,
    input logic 			     C1RxWrValid,
    input logic 			     C1RxIntrValid,
    // Almost full signals
    input logic 			     C0TxAlmFull,
    input logic 			     C1TxAlmFull
    );

   // File descriptors
   int 					     fd_sniffer;
   
      
   initial begin
      $display ("SIM-SV : Protocol Checker initialized");

      // Open log files
      fd_sniffer = $fopen(WARN_LOGNAME, "w");
      
      // Wait until finish logger
      wait (finish_logger == 1);
      
      // Close loggers
      $display ("SIM-SV : Closing Protocol checker");
      $fclose(fd_sniffer);
   end
   
   // any valid signals
   logic tx_valid;
   logic rx_valid;

   assign tx_valid = C0TxRdValid    | 
		     C1TxWrValid    | 
		     C1TxIntrValid  |
		     C2TxMmioRdValid;
   
   assign rx_valid = C0RxRdValid     | 
		     C0RxUMsgValid   |  
		     C0RxMmioRdValid | 
		     C0RxMmioWrValid | 
		     C1RxWrValid     |
		     C1RxIntrValid;
   
   // If reset is high, and transactions are asserted
   always @(posedge clk) begin
      if (SoftReset) begin
	 if (tx_valid | rx_valid) begin
	    `BEGIN_RED_FONTCOLOR;	    
	    $display("SIM-SV: [WARN] Transaction was sent when SoftReset was HIGH, this will be ignored");
	    $fwrite(fd_sniffer, "%d | Transaction was sent when SoftReset was HIGH, this will be ignored\n", $time);
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
    * TX checker files
    */
   CfgHdr_t C0RxCfg;   
   assign C0RxCfg = CfgHdr_t'(C0RxHdr);
      
   assign xz_tx0_flag = ( ^{C0TxHdr.vc,              C0TxHdr.len, C0TxHdr.reqtype, C0TxHdr.mdata} )   && C0TxRdValid ;
   assign xz_tx1_flag = ( ^{C1TxHdr.vc, C1TxHdr.sop, C1TxHdr.len, C1TxHdr.reqtype, C1TxHdr.mdata} )   && C1TxWrValid ;
   assign xz_tx2_flag = ( ^{C2TxHdr.tid, C2TxData})                                                   && C2TxMmioRdValid ;
   
   assign xz_rx0_flag = ( ^{C0RxHdr.vc_used, C0RxHdr.hitmiss,                 C0RxHdr.clnum, C0RxHdr.resptype, C0RxHdr.mdata, C0RxData} ) && (C0RxRdValid | C0RxUMsgValid);
   assign xz_rx1_flag = ( ^{C1RxHdr.vc_used, C1RxHdr.hitmiss, C1RxHdr.format, C1RxHdr.clnum, C1RxHdr.resptype, C1RxHdr.mdata}           ) && (C1RxWrValid | C1RxIntrValid);

   assign xz_cfg_flag = ( ^{C0RxCfg.index, C0RxCfg.len, C0RxCfg.tid, C0RxData} ) && (C0RxMmioWrValid | C0RxMmioRdValid);

   
   // FUNCTION: XZ message
   function void print_xz_message ( string channel_name );
      begin
   	 // Set message
   	 `BEGIN_RED_FONTCOLOR;
   	 $display ("Clock ", $time, " => X or Z found on ", channel_name, " is not recommended.");
   	 `END_RED_FONTCOLOR;
   	 $fwrite( fd_sniffer, "Clock ", $time, " => \n");
   	 $fwrite( fd_sniffer, "      X or Z was found, this is not recommended\n");
   	 $fwrite( fd_sniffer, "\n");
      end
   endfunction


   // Message call
   always @(posedge clk) begin
      if ((xz_tx0_flag == `VLOG_HIIMP) || (xz_tx0_flag == `VLOG_UNDEF))
   	print_xz_message ( "C0Tx" );
      if ((xz_tx1_flag == `VLOG_HIIMP) || (xz_tx1_flag == `VLOG_UNDEF))
   	print_xz_message ( "C1Tx" );
      if ((xz_tx2_flag == `VLOG_HIIMP) || (xz_tx2_flag == `VLOG_UNDEF))
   	print_xz_message ( "C2Tx" );
      if ((xz_rx0_flag == `VLOG_HIIMP) || (xz_rx0_flag == `VLOG_UNDEF))
   	print_xz_message ( "C0Rx" );
      if ((xz_rx1_flag == `VLOG_HIIMP) || (xz_rx1_flag == `VLOG_UNDEF))
   	print_xz_message ( "C1Rx" );
      if ((xz_cfg_flag == `VLOG_HIIMP) || (xz_cfg_flag == `VLOG_UNDEF))
   	print_xz_message ( "C0RxMmio" );      
   end

   // initial begin
   //    // log_started = 0;
   //    // fd_sniffer = $fopen("warnings.log", "w");
   //    forever begin
   // 	 // XZ messages
   // 	 if ((xz_tx0_flag == `VLOG_HIIMP) || (xz_tx0_flag == `VLOG_UNDEF))
   // 	   print_xz_message ( "C0Tx" );
   // 	 if ((xz_tx1_flag == `VLOG_HIIMP) || (xz_tx1_flag == `VLOG_UNDEF))
   // 	   print_xz_message ( "C1Tx" );
   // 	 if ((xz_rx0_flag == `VLOG_HIIMP) || (xz_rx0_flag == `VLOG_UNDEF))
   // 	   print_xz_message ( "C0Rx" );
   // 	 if ((xz_rx1_flag == `VLOG_HIIMP) || (xz_rx1_flag == `VLOG_UNDEF))
   // 	   print_xz_message ( "C1Rx" );
   // 	 if ((xz_cfg_flag == `VLOG_HIIMP) || (xz_cfg_flag == `VLOG_UNDEF))
   // 	   print_xz_message ( "Cfg " );
   // 	 // Wait till next clock
   // 	 @(posedge clk);
   //    end
   // end


   
   // Hazard warning
   // function void print_hazard_message ( logic [PHYSCLADDR_WIDTH-1:0] cl_addr );
   //    begin
   // 	 // Set message
   // 	 `BEGIN_RED_FONTCOLOR;
   // 	 $display ("Clock ", $time, " => Possible Data hazard at cache line address %x ", cl_addr);
   // 	 `END_RED_FONTCOLOR;
   // 	 $fwrite( fd_sniffer, "Clock ", $time, " => \n");
   // 	 $fwrite( fd_sniffer, "      Possible Data hazard at cache line addrress => %x\n", cl_addr);
   // 	 $fwrite( fd_sniffer, "\n");
   //    end
   // endfunction


   // /*
   //  * TX checker files
   //  */
   // assign xz_tx0_flag = (^C0TxHdr)                 && C0TxRdValid ;
   // assign xz_tx1_flag = (^C1TxHdr || ^C1TxData)    && C1TxWrValid ;
   // assign xz_rx0_flag = (^C0RxHdr || ^C0RxData)    && (C0RxWrValid || C0RxRdValid) ;
   // assign xz_rx1_flag = (^C1RxHdr)                 && C1RxWrValid ;
   // assign xz_cfg_flag = (^CfgHeader || ^CfgRdData) && (CfgRdDataValid || CfgWrValid || CfgRdValid);


   // /*
   //  * Data hazard warning engine
   //  * - Store address as a searchable primary key
   //  */
   // int unsigned active_addresses[*];

   // always @(posedge clk) begin
   //    // Push function
   //    if (C0TxRdValid) begin
   // 	 if (~active_addresses.exists(C0TxHdr.addr)) begin
   // 	    active_addresses[C0TxHdr.addr] = C0TxHdr.mdata;
   // 	 end
   // 	 else begin
   // 	    print_hazard_message(C0TxHdr.addr);
   // 	    active_addresses[C0TxHdr.addr] = C0TxHdr.mdata;
   // 	 end
   //    end
   //    if (C1TxWrValid) begin
   // 	 if (~active_addresses.exists(C1TxHdr.addr)) begin
   // 	    active_addresses[C1TxHdr.addr] = C1TxHdr.mdata;
   // 	 end
   // 	 else begin
   // 	    print_hazard_message(C1TxHdr.addr);
   // 	    active_addresses[C1TxHdr.addr] = C1TxHdr.mdata;
   // 	 end
   //    end
   //    // Pop function
   //    if (C0RxRdValid) begin
   // 	 if (active_addresses.exists(C0RxHdr.mdata))
   // 	   active_addresses.delete(C0RxHdr.mdata);
   //    end
   //    if (C0RxWrValid) begin
   // 	 if (active_addresses.exists(C0RxHdr.mdata))
   // 	   active_addresses.delete(C0RxHdr.mdata);
   //    end
   //    if (C1RxWrValid) begin
   // 	 if (active_addresses.exists(C1RxHdr.mdata))
   // 	   active_addresses.delete(C1RxHdr.mdata);
   //    end
   // end


endmodule // cci_sniffer
