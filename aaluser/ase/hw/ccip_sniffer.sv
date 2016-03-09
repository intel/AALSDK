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
    parameter ERR_LOGNAME  = "ccip_warning_and_errors.log"
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

   /*
    * File descriptors
    */
   int 					     fd_errlog;
      
   // FD open
   initial begin
      $display ("SIM-SV : Protocol Checker initialized");

      // Open log files
      fd_errlog = $fopen(ERR_LOGNAME, "w");

      // Wait until finish logger
      wait (finish_logger == 1);

      // Close loggers
      $display ("SIM-SV : Closing Protocol checker");
      $fclose(fd_errlog);
   end

   
   /*
    * Generic print message
    */ 
   // Print string and write to file
   function void print_message_and_log(input integer warn_only, 
				       input string logstr);
      begin
	 if (warn_only == 1) begin
	    `BEGIN_RED_FONTCOLOR;
	    $display(" [WARN]  %d : %s", $time, logstr);	 
	    `END_RED_FONTCOLOR;
	    $fwrite(fd_errlog, " [WARN]  %d : %s\n", $time, logstr);
	 end
	 else begin	    
	    `BEGIN_RED_FONTCOLOR;
	    $display(" [ERROR] %d : %s", $time, logstr);	 
	    `END_RED_FONTCOLOR;
	    $fwrite(fd_errlog, " [ERROR] %d : %s\n", $time, logstr);
	 end
      end
   endfunction

   
   // Print and simkill
   function void print_and_simkill();
      begin
	 `BEGIN_RED_FONTCOLOR;
	 $display(" [ERROR] %d : Simulation will end now", $time);	 
	 `END_RED_FONTCOLOR;
	 $fwrite(fd_errlog, " [ERROR] %d : Simulation will end now\n", $time);
	 start_simkill_countdown();	 
      end
   endfunction
      

   /*
    * Valid aggregate for X, Z checking
    */   
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
	    print_message_and_log(1, "Transaction was sent when SoftReset was HIGH, this will be ignored");
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

   assign xz_tx0_flag = ( ^{C0TxHdr.vc,              C0TxHdr.len, C0TxHdr.reqtype, C0TxHdr.addr, C0TxHdr.mdata} )                        && C0TxValid ;
   assign xz_tx1_flag = ( ^{C1TxHdr.vc, C1TxHdr.sop, C1TxHdr.len, C1TxHdr.reqtype, C1TxHdr.addr, C1TxHdr.mdata} )                        && C1TxValid ;
   assign xz_tx2_flag = ( ^{C2TxHdr.tid, C2TxData})                                                                                      && C2TxMmioRdValid ;

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
   	 $fwrite( fd_errlog, " %d | X or Z found on %s => \n", $time, channel_name);
   	 $fwrite( fd_errlog, "    | This is not recommended, and can have unintended activity\n");
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
    * MMIO Misbehaviour tracker
    */
   // MMIO Timeout management
   int mmioread_timeout_cnt;
   logic mmioread_cycle;

   // Check if MMIO TID returned was correct
   int 	 c0rx_mmiord_tid;
   int 	 c2tx_mmiord_tid;    
      
   // MMIO Read activity in progress
   always @(posedge clk) begin : mmiocycle_proc
      if (SoftReset) begin
	 mmioread_cycle <= 0;
      end
      else begin
	 case ({C0RxMmioRdValid, C2TxMmioRdValid})
	   2'b10   : mmioread_cycle <= 1;
	   2'b01   : mmioread_cycle <= 0;
	   default : mmioread_cycle <= mmioread_cycle;	   
	 endcase	 
      end
   end

   // Sample outgoing/incoming TID as current
   // always @(posedge clk) begin
   //    if (C0RxMmioRdValid) begin
   //    end
   //    if (C2TxMmioRdValid) begin
   //    end
   // end

   
   // MMIO counter
   always @(posedge clk) begin : mmioread_timeout_ctr
      if (SoftReset) begin
	 mmioread_timeout_cnt <= 0;
      end
      else if (mmioread_cycle) begin
	 mmioread_timeout_cnt <= mmioread_timeout_cnt + 1;
      end
      else begin
	 mmioread_timeout_cnt <= 0;
      end
   end

   // MMIO misbehaviour check
   always @(posedge clk) begin : mmio_timeout_simkill
      if (mmioread_timeout_cnt >= `MMIO_RESPONSE_TIMEOUT) begin
	 print_message_and_log(0, "ASE timed out waiting for MMIO Read response to arrive !!");
	 print_message_and_log(0, "MMIO Read responses must return in 512 cycles");
	 print_and_simkill();	 
      end
      if (~mmioread_cycle && C2TxMmioRdValid) begin
      	 print_message_and_log(0, "ASE detected an unsolicited MMIO Read response !!\n");
      	 print_message_and_log(0, "In system, this can cause a crash");
      	 print_and_simkill();	 
      end
   end

   

   /*
    * Full {0,1} signaling
    */
   always @(posedge clk) begin
      if (cf2as_ch0_realfull && C0TxValid) begin
	 print_message_and_log(1, "Transaction was possibly dropped due to C0Tx Overflow");
	 print_message_and_log(1, "Transaction was pushed in when port was FULL");
      end
      if (cf2as_ch1_realfull && C1TxValid) begin
	 print_message_and_log(1, "Transaction was possibly dropped due to C1Tx Overflow");
	 print_message_and_log(1, "Transaction was pushed in when port was FULL");
      end
   end


   /*
    * Illegal transaction checker
    */
   logic c0tx_illegal;
   logic c1tx_illegal;   
   
   // Check illegal transaction IDs in C0Tx
   always @(posedge clk) begin : c0tx_illegal_proc
      if (C0TxValid) begin
	 if ((C0TxHdr.reqtype == ASE_RDLINE_S)||(C0TxHdr.reqtype == ASE_RDLINE_I)) begin
	    c0tx_illegal <= 0;
	 end
	 else begin
	    c0tx_illegal <= 1;
	    print_message_and_log(1, "Illegal transaction request type noticed on C0TxHdr");
	    print_and_simkill();	 
	 end
      end
      else begin
	 c0tx_illegal <= 0;
      end	
   end

   // Check illegal transaction IDs in C1Tx
   always @(posedge clk) begin : c1tx_illegal_proc
      if (C1TxValid) begin
	 if ((C1TxHdr.reqtype == ASE_WRLINE_M)||(C1TxHdr.reqtype == ASE_RDLINE_I)||(C1TxHdr.reqtype == ASE_WRFENCE)) begin
	    c1tx_illegal <= 0;
	 end
	 else begin
	    c1tx_illegal <= 1;
	    print_message_and_log(1, "Illegal transaction request type noticed on C1TxHdr");
	    print_and_simkill();	 
	 end
      end
      else begin
	 c1tx_illegal <= 0;
      end	
   end
   
   
   /*
    * Incoming transaction checker
    */ 
   typedef enum {
		 Exp_1CL,
		 Exp_2CL,
		 Exp_3CL,
		 Exp_4CL
		 } ExpTxState;
   ExpTxState exp_c1state;

   logic [PHYSCLADDR_WIDTH-1:0] exp_c1addr;
   ccip_vc_t                    exp_c1vc;
   ccip_len_t                   exp_c1len;
  

   /*
    * Write TX beat check
    */
   logic 			c1tx_beat_in_progress;
   logic 			wrline_en;

   // Wrline_en
   always @(*) begin
      if (C1TxValid && ((C1TxHdr.reqtype==ASE_WRLINE_M)||(C1TxHdr.reqtype==ASE_WRLINE_I))) begin
	 wrline_en <= 1;	 
      end
      else begin
	 wrline_en <= 0;
      end
   end   

   // Checker state machine
   always @(posedge clk) begin
      if (SoftReset) begin
	 exp_c1state            <= Exp_1CL;
	 exp_c1addr             <= {PHYSCLADDR_WIDTH{1'b0}};	 
	 c1tx_beat_in_progress  <= 0;	 
      end
      else begin
	 case (exp_c1state)
	   // Waiting for a C1Tx transaction
	   Exp_1CL:
	     begin
		if (wrline_en && (C1TxHdr.len == ASE_4CL)) begin
		   if (~C1TxHdr.sop) begin
		      print_message_and_log(0, "C1TxHdr SOP field is not HIGH in the first transaction of multi-line request !");		      
		      print_and_simkill();		      
		   end
		   else begin
		      exp_c1addr <= C1TxHdr.addr;
		      exp_c1vc   <= C1TxHdr.vc;
		      exp_c1len  <= C1TxHdr.len;		      
		   end
		   exp_c1state <= Exp_2CL;		   
		end // if (wrline_en && (C1TxHdr.len == ASE_4CL))
		else if (wrline_en && (C1TxHdr.len == ASE_2CL)) begin
		   if (~C1TxHdr.sop) begin
		      print_message_and_log(0, "C1TxHdr SOP field is not HIGH in the first transaction of multi-line request !");		      
		      print_and_simkill();		      
		   end
		   else begin
		      exp_c1addr <= C1TxHdr.addr;
		      exp_c1vc   <= C1TxHdr.vc;
		      exp_c1len  <= C1TxHdr.len;		      
		   end
		   exp_c1state <= Exp_2CL;		   
		end // if (wrline_en && (C1TxHdr.len == ASE_2CL))
		else if (wrline_en && (C1TxHdr.len == ASE_1CL)) begin
		   if (~C1TxHdr.sop) begin
		      print_message_and_log(0, "C1TxHdr SOP field is not HIGH in the first transaction of multi-line request !");		      
		      print_and_simkill();		      
		   end
		   exp_c1state <= Exp_1CL;		   
		end
		else begin
		   exp_c1state <= Exp_1CL;		   
		end		
	     end

	   // 2nd cache line of multiline request
	   Exp_2CL:
	     begin
		if (wrline_en && (exp_c1len == ASE_2CL)) begin
		   exp_c1state <= Exp_1CL;		   
		end
		else if (wrline_en && (exp_c1len == ASE_4CL)) begin
		   exp_c1state <= Exp_3CL;
		end
		else begin
		   exp_c1state <= Exp_2CL;		   
		end
	     end

	   // 3rd cache line of multiline request -- no return to base from here
	   Exp_3CL:
	     begin
		if (wrline_en && (exp_c1len == ASE_4CL)) begin
		   exp_c1state <= Exp_4CL;
		end
		else begin
		   exp_c1state <= Exp_3CL;		   
		end
	     end

	   // 4th cacheline of multiline request
	   Exp_4CL:
	     begin
		if (wrline_en && (exp_c1len == ASE_4CL)) begin
		   exp_c1state <= Exp_1CL;		
		end
		else begin
		   exp_c1state <= Exp_4CL;		   
		end		
	     end

	   // lala land
	   default:
	     begin
	     end

	 endcase
      end
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
	    print_message_and_log(1, "Request on C0Tx for 3 cachelines is ILLEGAL !");
	    print_message_and_log(1, "In CCI-P specificaton document, see Section on Multi-cacheline requests");
	    print_and_simkill();	    
	 end
	 // ------------------------------------------------------------ //
	 // Address alignment check
	 if ( (C0TxHdr.addr[0] != 1'b0) && (C0TxHdr.len == ASE_2CL) ) begin
	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ");
	    print_message_and_log(0, "Simulator will shut down now !\n");
	    print_and_simkill();	    
	 end
	 else if ( (C0TxHdr.addr[1:0] != 2'b0) && (C0TxHdr.len == ASE_4CL) ) begin
	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ");
	    print_message_and_log(0, "Simulator will shut down now !\n");
	    print_and_simkill();	    
	 end
	 // ------------------------------------------------------------ //
      end
   end // block: c0tx_mcl_check


   /*
    * Multi-cache WRITE line request checks
    */
   always @(posedge clk) begin : c1tx_mcl_check
      // ------------------------------------------------------------ //
      // If Write request
      if ( C1TxValid && ((C1TxHdr.reqtype == ASE_WRLINE_M)||(C1TxHdr.reqtype == ASE_RDLINE_I)) )  begin
   	 // ------------------------------------------------------------ //
   	 // Tx0 - 3 CL request illegal
   	 if ((C1TxHdr.len == ASE_3CL) && C1TxHdr.sop) begin
   	    print_message_and_log(0, "Request on C1Tx for 3 cachelines is ILLEGAL !");
   	    print_message_and_log(0, "In CCI-P specificaton document, please see Multi-cacheline requests");
	    print_and_simkill();	    
   	 end
   	 // ------------------------------------------------------------ //
   	 // Address alignment check
   	 if ( (C1TxHdr.addr[0] != 1'b0) && (C1TxHdr.len == ASE_2CL) && C1TxHdr.sop ) begin
   	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ");
   	    print_message_and_log(0, "Simulator will shut down now !\n");
	    print_and_simkill();	    
   	 end
   	 else if ( (C1TxHdr.addr[1:0] != 2'b0) && (C1TxHdr.len == ASE_4CL) && C1TxHdr.sop ) begin
   	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ");
   	    print_message_and_log(0, "Simulator will shut down now !\n");
	    print_and_simkill();	    
   	 end
   	 // ------------------------------------------------------------ //
      end
   end // block: c1tx_mcl_check


   /*
    * Check memory transactions in flight, maintain active list
    */
   longint rd_active_addr_array[*];
   longint wr_active_addr_array[*];
   string  rd_addr_str;
   string  wr_addr_str;

   // Directory process
   always @(posedge clk) begin
      // Channel 0 valid transaction
      if (C0TxValid) begin
	 // If Valid read request
	 if ((C0TxHdr.reqtype == ASE_RDLINE_I)||(C0TxHdr.reqtype == ASE_RDLINE_S)) begin
	    // If transaction address exists in active list
	    if  ( rd_active_addr_array.exists(C0TxHdr.addr)||wr_active_addr_array.exists(C0TxHdr.addr) ) begin
	       rd_addr_str = {"A request to Address ", (C0TxHdr.addr << 6), " is already in flight, potential condition for data hazard"};	       
	       print_message_and_log(1, rd_addr_str);	       
	    end
	    else begin
	       rd_active_addr_array[C0TxHdr.addr] = C0TxHdr.addr;	       
	    end
	 end	 
      end
      // Channel 1 valid transaction
      if (C1TxValid) begin
	 // If valid write request
	 if ((C1TxHdr.reqtype == ASE_WRLINE_I)||(C1TxHdr.reqtype == ASE_WRLINE_M)) begin
	    // If transaction address exists in active list
	    if  ( rd_active_addr_array.exists(C1TxHdr.addr)||wr_active_addr_array.exists(C1TxHdr.addr) ) begin
	       wr_addr_str = {"A request to Address ", (C1TxHdr.addr << 6), " is already in flight, potential condition for data hazard"};	       
	       print_message_and_log(1, wr_addr_str);	       
	    end
	    else begin
	       wr_active_addr_array[C1TxHdr.addr] = C1TxHdr.addr;	       
	    end
	    
	 end
      end      
   end
   
   

endmodule // cci_sniffer
