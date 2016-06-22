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
 * All warnings are logged in ccip_warning_and_errors.log
 *
 * FIXME list
 * - Sop & cllen pattern matching
 * - Atomic must pass through VL0 only, else error out
 */

import ase_pkg::*;
import ccip_if_pkg::*;
`include "platform.vh"

// `define STANDALONE_DEBUG

module ccip_sniffer
  #(
    parameter ERR_LOGNAME  = "ccip_warning_and_errors.log"
    )
   (
    // Configure enable
    input logic finish_logger,
    input logic init_sniffer,
    // -------------------------------------------------------- //
    // Channel overflow/realfull checks
    input logic cf2as_ch0_realfull,
    input logic cf2as_ch1_realfull,
    // -------------------------------------------------------- //
    //          Hazard/Indicator Signals                        //
    input 	ase_haz_if haz_if,
    output 	sniff_code_t error_code,
    // -------------------------------------------------------- //
    //                   CCI-P interface                        //
    input logic clk,
    input logic SoftReset,
    input 	t_if_ccip_Rx ccip_rx,
    input 	t_if_ccip_Tx ccip_tx
    );


   /*
    * File descriptors, codes etc
    */
   int 					     fd_errlog;

   // Initialize sniffer
   always @(posedge clk) begin
      if (init_sniffer) begin
	 decode_error_code(1, 0);
      end
   end

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
    * Controlled kill of simulation
    */
   logic simkill_en = 0;
   int 	 simkill_cnt;

   // Print and simkill
   function void print_and_simkill();
      begin
	 `BEGIN_RED_FONTCOLOR;
	 $display(" [ERROR] %d : Simulation will end now", $time);
	 `END_RED_FONTCOLOR;
	 $fwrite(fd_errlog, " [ERROR] %d : Simulation will end now\n", $time);
	 simkill_en = 1;
      end
   endfunction

   // Enumeration of simulation states
   typedef enum {
		 SimkillIdle,
		 SimkillCountdown,
		 SimkillNow
		 } SimkillEnumStates;
   SimkillEnumStates simkill_state;

   // Simkill countdown and issue simkill
   always @(posedge clk) begin
      if (SoftReset) begin
	 simkill_cnt <= 20;
	 simkill_state <= SimkillIdle;
      end
      else begin
	 case (simkill_state)
	   SimkillIdle:
	     begin
		simkill_cnt <= 20;
		if (simkill_en) begin
		   simkill_state <= SimkillCountdown;
		end
		else begin
		   simkill_state <= SimkillIdle;
		end
	     end
	   SimkillCountdown:
	     begin
		simkill_cnt <= simkill_cnt - 1;
		if (simkill_cnt <= 0) begin
		   simkill_state <= SimkillNow;
		end
		else begin
		   simkill_state <= SimkillCountdown;
		end
	     end
	   SimkillNow:
	     begin
		simkill_state <= SimkillNow;
`ifndef STANDALONE_DEBUG
		start_simkill_countdown();
`endif
	     end
	   default:
	     begin
		simkill_state <= SimkillIdle;
	     end
	 endcase
      end
   end


   /*
    * Helper functions
    */
   // Print string and write to file
   function void print_message_and_log(input logic warn_only,
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
	    print_and_simkill();
	 end
      end
   endfunction

   // Error code enumeration
   function void decode_error_code(
				   input logic init,
				   input       sniff_code_t code
				   );
      string 				       errcode_str;
      string 				       log_str;
      begin
	 if (init) begin
	    error_code = SNIFF_NO_ERROR;
	 end
	 else begin
	    error_code = code;
	    errcode_str = code.name;
	    case (code)
	      // C0TX - Invalid request type
	      SNIFF_C0TX_INVALID_REQTYPE:
		begin
		   $sformat(log_str, "[%s] C0TxHdr was issued with an invalid reqtype !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C0TX_OVERFLOW:
		begin
		   $sformat(log_str, "[%s] Overflow detected on CCI-P Channel 0 !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C0TX_ADDRALIGN_2_ERROR:
		begin
		   $sformat(log_str, "[%s] C0TxHdr Multi-line address request is not aligned 2-CL aligned (C0TxHdr.addr[0] != 1'b0) !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C0TX_ADDRALIGN_4_ERROR:
		begin
		   $sformat(log_str, "[%s] C0TxHdr Multi-line address request is not aligned 4-CL aligned (C0TxHdr.addr[1:0] != 2'b00) !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C0TX_RESET_IGNORED_WARN:
		begin
		   $sformat(log_str, "[%s] C0TxHdr was issued when AFU Reset is HIGH !\n", errcode_str);
		   print_message_and_log(1, log_str);
		end

	      SNIFF_C0TX_XZ_FOUND_WARN:
		begin
		   $sformat(log_str, "[%s] C0TxHdr request contained a 'Z' or 'X' !\n", errcode_str);
		   print_message_and_log(1, log_str);
		end

	      SNIFF_C1TX_INVALID_REQTYPE:
		begin
		   $sformat(log_str, "[%s] C1TxHdr was issued with an invalid reqtype !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_OVERFLOW:
		begin
		   $sformat(log_str, "[%s] Overflow detected on CCI-P Channel 1 !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_ADDRALIGN_2_ERROR:
		begin
		   $sformat(log_str, "[%s] C1TxHdr Multi-line address request is not aligned 2-CL aligned (C1TxHdr.addr[0] != 1'b0) !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_ADDRALIGN_4_ERROR:
		begin
		   $sformat(log_str, "[%s] C1TxHdr Multi-line address request is not aligned 4-CL aligned (C1TxHdr.addr[1:0] != 2'b00) !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_RESET_IGNORED_WARN:
		begin
		   $sformat(log_str, "[%s] C1TxHdr was issued when AFU Reset is HIGH !\n", errcode_str);
		   print_message_and_log(1, log_str);
		end

	      SNIFF_C1TX_XZ_FOUND_WARN:
		begin
		   $sformat(log_str, "[%s] C1TxHdr request contained a 'Z' or 'X' !\n", errcode_str);
		   print_message_and_log(1, log_str);
		end

	      SNIFF_C1TX_UNEXP_VCSEL:
		begin
		   $sformat(log_str, "[%s] C1TxHdr VC-selection must not change in between multi-line beat !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_UNEXP_MDATA:
		begin
		   $sformat(log_str, "[%s] C1TxHdr MDATA-setting must not change in between multi-line beat !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_UNEXP_ADDR:
		begin
		   $sformat(log_str, "[%s] C1TxHdr multi-line beat found unexpected address - addr[1:0] must increment by 1 !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_UNEXP_CLLEN:
		begin
		   $sformat(log_str, "[%s] C1TxHdr multi-line beat found unexpected cllen - cllen must decrement by 1 !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_PAYLOAD_OVERRUN:
		begin
		   $sformat(log_str, "[%s] C1TxHdr multi-line beat detected more than expected number of transactions !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_PAYLOAD_UNDERRUN:
		begin
		   $sformat(log_str, "[%s] C1TxHdr multi-line beat detected less than expected number of transactions !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_SOP_NOT_SET:
		begin
		   $sformat(log_str, "[%s] C1TxHdr First transaction of multi-line beat must set SOP field to HIGH !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      SNIFF_C1TX_SOP_SET_MCL1TO3:
		begin
		   $sformat(log_str, "[%s] C1TxHdr Subsequent transaction of multi-line beat must set SOP field to LOG !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      MMIO_RDRSP_TIMEOUT:
		begin
		   $sformat(log_str, "[%s] MMIO Read Response timed out. AFU must respond to MMIO Read responses within %d clocks !\n", errcode_str, `MMIO_RESPONSE_TIMEOUT);
		   print_message_and_log(0, log_str);
		end

	      MMIO_RDRSP_TID_MISMATCH:
		begin
		   $sformat(log_str, "[%s] MMIO Read Response TID did not match MMIO Read Request !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      MMIO_RDRSP_UNSOLICITED:
		begin
		   $sformat(log_str, "[%s] ASE detected an unsolicited MMIO Response. In system, this can cause unexpected behavior !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      MMIO_RDRSP_XZ_FOUND_WARN:
		begin
		   $sformat(log_str, "[%s] MMIO Response contained a 'Z' or 'X' !\n", errcode_str);
		   print_message_and_log(0, log_str);
		end

	      // Unknown type -- this must not happen
	      default:
		begin
		   print_message_and_log(1, "** ASE Internal Error **: Undefined Error code found !");
		end

	    endcase
	 end
      end
   endfunction



   /*
    * Valid aggregate for X, Z checking
    */
   // any valid signals
   logic tx_valid;
   logic rx_valid;

   assign tx_valid = ccip_tx.c0.valid    |
		     ccip_tx.c1.valid    |
		     ccip_tx.c2.mmioRdValid;

   assign rx_valid = ccip_rx.c0.rspValid    |
		     ccip_rx.c0.mmioRdValid |
		     ccip_rx.c0.mmioWrValid |
		     ccip_rx.c1.rspValid;


   // If reset is high, and transactions are asserted
   // always @(posedge clk) begin
   //    if (SoftReset) begin
   // 	 if (tx_valid | rx_valid) begin
   // 	    print_message_and_log(1, "Transaction was sent when SoftReset was HIGH, this will be ignored");
   // 	 end
   //    end
   // end


   /*
    * Cast MMIO Header
    */
   t_ccip_c0_ReqMmioHdr C0RxCfg;
   assign C0RxCfg = t_ccip_c0_ReqMmioHdr'(ccip_rx.c0.hdr);


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

   // Z-X checker
   // always @(*) begin
   //    if (ccip_tx.c0.valid) begin
   // 	 xz_tx0_flag <= ^{ccip_tx.c0.hdr.vc_sel,                     ccip_tx.c0.hdr.cl_len, ccip_tx.c0.hdr.req_type, ccip_tx.c0.hdr.address, ccip_tx.c0.hdr.mdata};
   //    end
   //    if (ccip_tx.c1.valid) begin
   // 	 xz_tx1_flag <= ^{ccip_tx.c1.hdr.vc_sel, ccip_tx.c1.hdr.sop, ccip_tx.c1.hdr.cl_len, ccip_tx.c1.hdr.req_type, ccip_tx.c1.hdr.address, ccip_tx.c1.hdr.mdata};
   //    end
   //    if (ccip_tx.c2.mmioRdValid) begin
   // 	 xz_tx2_flag <= ^{ccip_tx.c2.hdr.tid, ccip_tx.c2.data};
   //    end
   // end

   assign xz_tx0_flag = ^{ccip_tx.c0.hdr.vc_sel,                     ccip_tx.c0.hdr.cl_len, ccip_tx.c0.hdr.req_type, ccip_tx.c0.hdr.address, ccip_tx.c0.hdr.mdata};

   assign xz_tx1_flag = ^{ccip_tx.c1.hdr.vc_sel, ccip_tx.c1.hdr.sop, ccip_tx.c1.hdr.cl_len, ccip_tx.c1.hdr.req_type, ccip_tx.c1.hdr.address, ccip_tx.c1.hdr.mdata};

   assign xz_tx2_flag = ^{ccip_tx.c2.hdr.tid, ccip_tx.c2.data};

   // Trigger XZ warnings
   always @(posedge clk) begin
      if (ccip_tx.c0.valid && (xz_tx0_flag inside {1'bx, 1'bX, 1'bz, 1'bZ})) begin
	 decode_error_code(0, SNIFF_C0TX_XZ_FOUND_WARN);
      end
      if (ccip_tx.c1.valid && (xz_tx1_flag inside {1'bx, 1'bX, 1'bz, 1'bZ})) begin
	 decode_error_code(0, SNIFF_C1TX_XZ_FOUND_WARN);
      end
      if (ccip_tx.c2.mmioRdValid && (xz_tx2_flag inside {1'bx, 1'bX, 1'bz, 1'bZ})) begin
	 decode_error_code(0, MMIO_RDRSP_XZ_FOUND_WARN);
      end
   end




   // assign xz_rx0_flag = ( ^{ccip_rx.c0.hdr.vc_used, ccip_rx.c0.hdr.hit_miss,                        ccip_rx.c0.hdr.cl_num, ccip_rx.c0.hdr.resp_type, ccip_rx.c0.hdr.mdata, ccip_rx.c0.data}  && ccip_rx.c0.rspValid );
   // assign xz_rx1_flag = ( ^{ccip_rx.c1.hdr.vc_used, ccip_rx.c1.hdr.hit_miss, ccip_rx.c1.hdr.format, ccip_rx.c1.hdr.cl_num, ccip_rx.c1.hdr.resp_type, ccip_rx.c1.hdr.mdata}                   && ccip_rx.c1.rspValid );

   // assign xz_cfg_flag = ( ^{C0RxCfg.address, C0RxCfg.length, C0RxCfg.tid, ccip_rx.c0.data} ) && (ccip_rx.c0.mmioWrValid | ccip_rx.c0.mmioRdValid);


   // FUNCTION: XZ message
   // function void print_xz_message ( string channel_name );
   //    begin
   // 	 // Set message
   // 	 `BEGIN_RED_FONTCOLOR;
   // 	 $display ("SIM-SV: %d | X or Z found on %s is not recommended.", $time, channel_name);
   // 	 `END_RED_FONTCOLOR;
   // 	 $fwrite( fd_errlog, " %d | X or Z found on %s => \n", $time, channel_name);
   // 	 $fwrite( fd_errlog, "    | This is not recommended, and can have unintended activity\n");
   //    end
   // endfunction


   // Message call
   // always @(posedge clk) begin
   //    if ((xz_tx0_flag == `VLOG_HIIMP) || (xz_tx0_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C0Tx" );
   //    end
   //    if ((xz_tx1_flag == `VLOG_HIIMP) || (xz_tx1_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C1Tx" );
   //    end
   //    if ((xz_tx2_flag == `VLOG_HIIMP) || (xz_tx2_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C2Tx" );
   //    end
   //    if ((xz_rx0_flag == `VLOG_HIIMP) || (xz_rx0_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C0Rx" );
   //    end
   //    if ((xz_rx1_flag == `VLOG_HIIMP) || (xz_rx1_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C1Rx" );
   //    end
   //    if ((xz_cfg_flag == `VLOG_HIIMP) || (xz_cfg_flag == `VLOG_UNDEF)) begin
   // 	 print_xz_message ( "C0RxMmio" );
   //    end
   // end


   /*
    * Full {0,1} signaling
    */
   always @(posedge clk) begin
      if (cf2as_ch0_realfull && ccip_tx.c0.valid) begin
	 print_message_and_log(1, "Transaction was possibly dropped due to C0Tx Overflow");
	 print_message_and_log(1, "Transaction was pushed in when port was FULL");
      end
      if (cf2as_ch1_realfull && ccip_tx.c1.valid) begin
	 print_message_and_log(1, "Transaction was possibly dropped due to C1Tx Overflow");
	 print_message_and_log(1, "Transaction was pushed in when port was FULL");
      end
   end


   /*
    * Illegal transaction checker
    */
   always @(posedge clk) begin : illegal_reqproc
      // C0TxHdr reqtype
      if (ccip_tx.c0.valid) begin
	 if (ccip_tx.c0.hdr.req_type inside {eREQ_RDLINE_S, eREQ_RDLINE_I}) begin
	 end
	 else begin
	    decode_error_code(0, SNIFF_C0TX_INVALID_REQTYPE);
	 end
      end
      // C1TxHdr reqtype
      if (ccip_tx.c1.valid) begin
	 if (ccip_tx.c1.hdr.req_type inside {eREQ_WRLINE_M, eREQ_WRLINE_I, eREQ_WRFENCE}) begin
	 end
	 else begin
	    decode_error_code(0, SNIFF_C1TX_INVALID_REQTYPE);
	 end
      end
   end


   // logic c0tx_illegal;
   // logic c1tx_illegal;

   // Check illegal transaction IDs in C0Tx
   // always @(posedge clk) begin : c0tx_illegal_proc
   //    if (ccip_tx.c0.valid) begin
   // 	 if ((ccip_tx.c0.hdr.req_type == eREQ_RDLINE_S)||(ccip_tx.c0.hdr.req_type == eREQ_RDLINE_I)) begin
   // 	    c0tx_illegal <= 0;
   // 	 end
   // 	 else begin
   // 	    c0tx_illegal <= 1;
   // 	    print_message_and_log(1, "Illegal transaction request type noticed on C0TxHdr");
   // 	    print_and_simkill();
   // 	 end
   //    end
   //    else begin
   // 	 c0tx_illegal <= 0;
   //    end
   // end

   // Check illegal transaction IDs in C1Tx
   // always @(posedge clk) begin : c1tx_illegal_proc
   //    if (ccip_tx.c1.valid) begin
   // 	 if ((ccip_tx.c1.hdr.req_type==eREQ_WRLINE_M)||(ccip_tx.c1.hdr.req_type==eREQ_WRLINE_I)||(ccip_tx.c1.hdr.req_type==eREQ_WRFENCE)) begin
   // 	    c1tx_illegal <= 0;
   // 	 end
   // 	 else begin
   // 	    c1tx_illegal <= 1;
   // 	    print_message_and_log(0, "Illegal transaction request type noticed on C1TxHdr");
   // 	    print_and_simkill();
   // 	 end
   //    end
   //    else begin
   // 	 c1tx_illegal <= 0;
   //    end // if (ccip_tx.c1.valid)
   // end


   /*
    * Incoming transaction checker
    */
   // typedef enum {
   // 		 Exp_1CL,
   // 		 Exp_2CL,
   // 		 Exp_3CL,
   // 		 Exp_4CL
   // 		 } ExpTxState;
   // ExpTxState exp_c1state;

   // logic [PHYSCLADDR_WIDTH-1:0] exp_c1addr;
   // t_ccip_vc                    exp_c1vc;
   // t_ccip_clLen                 exp_c1len;
   // logic [15:0] 		exp_c1mdata;


   /*
    * Write TX beat check
    */
   // logic 			c1tx_beat_in_progress;
   logic 			wrline_en;
   logic 			wrfence_en;

   // Wrline_en
   // always @(*) begin
   //    if (ccip_tx.c1.valid && ((ccip_tx.c1.hdr.req_type==eREQ_WRLINE_M)||(ccip_tx.c1.hdr.req_type==eREQ_WRLINE_I))) begin
   // 	 wrline_en <= 1;
   // 	 wrfence_en <= 0;
   //    end
   //    else if (ccip_tx.c1.valid && (ccip_tx.c1.hdr.req_type==eREQ_WRFENCE)) begin
   // 	 wrline_en <= 0;
   // 	 wrfence_en <= 1;
   //    end
   //    else begin
   // 	 wrline_en <= 0;
   // 	 wrfence_en <= 0;
   //    end
   // end

   // isCCIPWriteRequest
   function automatic logic isCCIPWriteRequest(t_ccip_c1_ReqMemHdr hdr);
      begin
	 if ((hdr.req_type==eREQ_WRLINE_M)||(hdr.req_type==eREQ_WRLINE_I)) begin
	    return 1;
	 end
	 else begin
	    return 0;
	 end
      end
   endfunction

   // isCCIPWrFenceRequest
   function automatic logic isCCIPWrFenceRequest(t_ccip_c1_ReqMemHdr hdr);
      begin
	 if (ccip_tx.c1.hdr.req_type==eREQ_WRFENCE) begin
	    return 1;
	 end
	 else begin
	    return 0;
	 end
      end
   endfunction


   // String for writing formatted messages
   // string exp_c1tx_str;

   // Checker state machine
   // always @(posedge clk) begin
   //    if (SoftReset) begin
   // 	 exp_c1state            <= Exp_1CL;
   // 	 exp_c1addr             <= {PHYSCLADDR_WIDTH{1'b0}};
   // 	 // c1tx_beat_in_progress  <= 0;
   //    end
   //    else begin
   // 	 case (exp_c1state)
   // 	   // ----------------------------------------------------------- //
   // 	   // Waiting for a C1Tx transaction
   // 	   Exp_1CL:
   // 	     begin
   // 		exp_c1addr  <= ccip_tx.c1.hdr.address;
   // 		exp_c1vc    <= ccip_tx.c1.hdr.vc_sel;
   // 		exp_c1len   <= ccip_tx.c1.hdr.cl_len;
   // 		exp_c1mdata <= ccip_tx.c1.hdr.mdata;
   // 		// 4 CL MCL request
   // 		if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b11)) begin
   // 		   // exp_c1addr  <= ccip_tx.c1.hdr.address;
   // 		   // exp_c1vc    <= ccip_tx.c1.hdr.vc_sel;
   // 		   // exp_c1len   <= ccip_tx.c1.hdr.cl_len;
   // 		   // exp_c1mdata <= ccip_tx.c1.hdr.mdata;
   // 		   exp_c1state <= Exp_2CL;
   // 		   // c1tx_beat_in_progress <= 1;
   // 		   if (ccip_tx.c1.hdr.address[1:0] != 2'b00) begin
   // 		      print_message_and_log(0, "Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned !");
   // 		      print_and_simkill();
   // 		   end
   // 		end
   // 		// 2 CL MCL request
   // 		else if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b01)) begin
   // 		   // exp_c1addr  <= ccip_tx.c1.hdr.address;
   // 		   // exp_c1vc    <= ccip_tx.c1.hdr.vc_sel;
   // 		   // exp_c1len   <= ccip_tx.c1.hdr.cl_len;
   // 		   // exp_c1mdata <= ccip_tx.c1.hdr.mdata;
   // 		   exp_c1state <= Exp_2CL;
   // 		   // c1tx_beat_in_progress <= 1;
   // 		   if (ccip_tx.c1.hdr.address[0] != 1'b0) begin
   // 		      print_message_and_log(0, "Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned !");
   // 		      print_and_simkill();
   // 		   end
   // 		end
   // 		// If 3-cacheline request is made, thats ILLEGAL
   // 		else if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b10)) begin
   // 		   // c1tx_beat_in_progress <= 0;
   // 		   print_message_and_log(0, "Multi-cacheline request of length=3 is ILLEGAL !");
   // 		   print_and_simkill();
   // 		end
   // 		// 1CL MCL request
   // 		else if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b00)) begin
   // 		   // c1tx_beat_in_progress <= 1;
   // 		   // exp_c1addr  <= ccip_tx.c1.hdr.address;
   // 		   // exp_c1vc    <= ccip_tx.c1.hdr.vc_sel;
   // 		   // exp_c1len   <= ccip_tx.c1.hdr.cl_len;
   // 		   // exp_c1mdata <= ccip_tx.c1.hdr.mdata;
   // 		   exp_c1state <= Exp_1CL;
   // 		end
   // 		else begin
   // 		   // c1tx_beat_in_progress <= 0;
   // 		   exp_c1state <= Exp_1CL;
   // 		end
   // 		// SOP bit check
   // 		if (~ccip_tx.c1.hdr.sop && wrline_en) begin
   // 		   print_message_and_log(0, "C1TxHdr SOP field is not HIGH in the first transaction of multi-line request !");
   // 		   print_and_simkill();
   // 		end
   // 	     end

   // 	   // ----------------------------------------------------------- //
   // 	   // 2nd cache line of multiline request
   // 	   Exp_2CL:
   // 	     begin
   // 		// c1tx_beat_in_progress <= 1;
   // 		// State control
   // 		// if (wrline_en && (exp_c1len == 2'b01)) begin
   // 		//    exp_c1state <= Exp_1CL;
   // 		// end
   // 		// else if (wrline_en && (exp_c1len == 2'b11)) begin
   // 		//    exp_c1state <= Exp_3CL;
   // 		// end
   // 		// else begin
   // 		//    exp_c1state <= Exp_2CL;
   // 		// end
   // 		// Check if SOP bit goes high
   // 		if (ccip_tx.c1.hdr.sop && wrline_en) begin
   // 		   print_message_and_log(0, "C1TxHdr SOP field is must NOT be HIGH on subsequent lines of multi-line Write request !");
   // 		   print_and_simkill();
   // 		end
   // 		// Check if address matches as expected
   // 		if (wrline_en && (ccip_tx.c1.hdr.address != (exp_c1addr + 1)) ) begin
   // 		   print_message_and_log(0, "C1TxHdr ADDR field expected an address incremented by 1 from previous transaction (Multi-line Write Request) !");
   // 		   print_message_and_log(0, "Refer to Multiline Request section of CCI-P specification for signalling details !");
   // 		   print_and_simkill();
   // 		end
   // 		// Just warn if mdata changes in flight
   // 		if (wrline_en && (ccip_tx.c1.hdr.mdata != exp_c1mdata)) begin
   // 		   print_message_and_log(1, "C1TxHdr MDATA field changed, this will be ignored and MDATA of 1st transaction will be used.");
   // 		end
   // 		// VC enforcement
   // 		if (wrline_en && (ccip_tx.c1.hdr.vc_sel != exp_c1vc)) begin
   // 		   print_message_and_log(0, "C1TxHdr VC field must not change while multi-line Write Request is in progress");
   // 		   print_and_simkill();
   // 		end
   // 		// ----------- State transition ----------- //
   // 		if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b01)) begin
   // 		   exp_c1state <= Exp_1CL;
   // 		end
   // 		else if (wrline_en && (ccip_tx.c1.hdr.cl_len == 2'b11)) begin
   // 		   exp_c1state <= Exp_3CL;
   // 		end
   // 		else begin
   // 		   exp_c1state <= Exp_2CL;
   // 		end
   // 	     end

   // 	   // ----------------------------------------------------------- //
   // 	   // 3rd cache line of multiline request -- no return to base from here
   // 	   Exp_3CL:
   // 	     begin
   // 		// c1tx_beat_in_progress <= 1;
   // 		// State control
   // 		// if (wrline_en && (exp_c1len == 2'b11)) begin
   // 		//    exp_c1state <= Exp_4CL;
   // 		// end
   // 		// else begin
   // 		//    exp_c1state <= Exp_3CL;
   // 		// end
   // 		// Check if SOP bit goes high
   // 		if (ccip_tx.c1.hdr.sop && wrline_en) begin
   // 		   print_message_and_log(0, "C1TxHdr SOP field is must NOT be HIGH on subsequent lines of multi-line Write request !");
   // 		   print_and_simkill();
   // 		end
   // 		// Check if address matches as expected
   // 		if (wrline_en && (ccip_tx.c1.hdr.address != (exp_c1addr + 2)) ) begin
   // 		   print_message_and_log(0, "C1TxHdr ADDR field expected an address incremented by 1 from previous transaction (Multi-line Write Request) !");
   // 		   print_message_and_log(0, "Refer to Multiline Request section of CCI-P specification for signalling details !");
   // 		   print_and_simkill();
   // 		end
   // 		// Just warn if mdata changes in flight
   // 		if (wrline_en && (ccip_tx.c1.hdr.mdata != exp_c1mdata)) begin
   // 		   print_message_and_log(1, "C1TxHdr MDATA field changed, this will be ignored and MDATA of 1st transaction will be used.");
   // 		end
   // 		// VC enforcement
   // 		if (wrline_en && (ccip_tx.c1.hdr.vc_sel != exp_c1vc)) begin
   // 		   print_message_and_log(0, "C1TxHdr VC field must not change while multi-line Write Request is in progress");
   // 		   print_and_simkill();
   // 		end
   // 		// ----------- State transition ----------- //

   // 	     end

   // 	   // ----------------------------------------------------------- //
   // 	   // 4th cacheline of multiline request
   // 	   Exp_4CL:
   // 	     begin
   // 		// c1tx_beat_in_progress <= 1;
   // 		// State control
   // 		if (wrline_en && (exp_c1len == 2'b11)) begin
   // 		   exp_c1state <= Exp_1CL;
   // 		end
   // 		else begin
   // 		   exp_c1state <= Exp_4CL;
   // 		end
   // 		// Check if SOP bit goes high
   // 		if (ccip_tx.c1.hdr.sop && wrline_en) begin
   // 		   print_message_and_log(0, "C1TxHdr SOP field is must NOT be HIGH on subsequent lines of multi-line Write request !");
   // 		   print_and_simkill();
   // 		end
   // 		// Check if address matches as expected
   // 		if (wrline_en && (ccip_tx.c1.hdr.address != (exp_c1addr + 3)) ) begin
   // 		   print_message_and_log(0, "C1TxHdr ADDR field expected an address incremented by 1 from previous transaction (Multi-line Write Request) !");
   // 		   print_message_and_log(0, "Refer to Multiline Request section of CCI-P specification for signalling details !");
   // 		   print_and_simkill();
   // 		end
   // 		// Just warn if mdata changes in flight
   // 		if (wrline_en && (ccip_tx.c1.hdr.mdata != exp_c1mdata)) begin
   // 		   print_message_and_log(1, "C1TxHdr MDATA field changed, this will be ignored and MDATA of 1st transaction will be used.");
   // 		end
   // 		// VC enforcement
   // 		if (wrline_en && (ccip_tx.c1.hdr.vc_sel != exp_c1vc)) begin
   // 		   print_message_and_log(0, "C1TxHdr VC field must not change while multi-line Write Request is in progress");
   // 		   print_and_simkill();
   // 		end
   // 	     end

   // 	   // ----------------------------------------------------------- //
   // 	   // lala land
   // 	   default:
   // 	     begin
   // 		// c1tx_beat_in_progress <= 0;
   // 		exp_c1state <= Exp_1CL;
   // 	     end

   // 	   // ----------------------------------------------------------- //
   // 	 endcase
   //    end
   // end

   /*
    * Multi-cache READ line request checks
    */
   // always @(posedge clk) begin : c0tx_mcl_check
   //    // ------------------------------------------------------------ //
   //    // If Read request
   //    if ( ccip_tx.c0.valid && ((ccip_tx.c1.hdr.req_type == eREQ_RDLINE_S)||(ccip_tx.c0.hdr.req_type == eREQ_RDLINE_S)) )  begin
   // 	 // ------------------------------------------------------------ //
   // 	 // Tx0 - 3 CL request illegal
   // 	 if (ccip_tx.c0.hdr.cl_len == 2'b10) begin
   // 	    print_message_and_log(1, "Request on C0Tx for 3 cachelines is ILLEGAL !");
   // 	    print_message_and_log(1, "In CCI-P specificaton document, see Section on Multi-cacheline requests");
   // 	    print_and_simkill();
   // 	 end
   // 	 // ------------------------------------------------------------ //
   // 	 // Address alignment check
   // 	 if ( (ccip_tx.c0.hdr.address[0] != 1'b0) && (ccip_tx.c0.hdr.cl_len == 2'b10) ) begin
   // 	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ");
   // 	    print_message_and_log(0, "Simulator will shut down now !\n");
   // 	    print_and_simkill();
   // 	 end
   // 	 else if ( (ccip_tx.c0.hdr.address[1:0] != 2'b0) && (ccip_tx.c0.hdr.cl_len == 2'b11) ) begin
   // 	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ");
   // 	    print_message_and_log(0, "Simulator will shut down now !\n");
   // 	    print_and_simkill();
   // 	 end
   // 	 // ------------------------------------------------------------ //
   //    end
   // end // block: c0tx_mcl_check


   /*
    * Multi-cache WRITE line request checks
    */
   // always @(posedge clk) begin : c1tx_mcl_check
   //    // ------------------------------------------------------------ //
   //    // If Write request
   //    if ( ccip_tx.c1.valid && ((ccip_tx.c1.hdr.req_type == eREQ_WRLINE_M)||(ccip_tx.c1.hdr.req_type == eREQ_RDLINE_I)) )  begin
   // 	 // ------------------------------------------------------------ //
   // 	 // Tx0 - 3 CL request illegal
   // 	 if ((ccip_tx.c1.hdr.cl_len == 2'b10) && ccip_tx.c1.hdr.sop) begin
   // 	    print_message_and_log(0, "Request on C1Tx for 3 cachelines is ILLEGAL !");
   // 	    print_message_and_log(0, "In CCI-P specificaton document, please see Multi-cacheline requests");
   // 	    print_and_simkill();
   // 	 end
   // 	 // ------------------------------------------------------------ //
   // 	 // Address alignment check
   // 	 if ( (ccip_tx.c1.hdr.address[0] != 1'b0) && (ccip_tx.c1.hdr.cl_len == 2'b10) && ccip_tx.c1.hdr.sop ) begin
   // 	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 2 must be 2-Cacheline Aligned ");
   // 	    print_message_and_log(0, "Simulator will shut down now !\n");
   // 	    print_and_simkill();
   // 	 end
   // 	 else if ( (ccip_tx.c1.hdr.address[1:0] != 2'b0) && (ccip_tx.c1.hdr.cl_len == 2'b11) && ccip_tx.c1.hdr.sop ) begin
   // 	    print_message_and_log(0, "Multi-cacheline request address with cl_len = 4 must be 4-Cacheline Aligned ");
   // 	    print_message_and_log(0, "Simulator will shut down now !\n");
   // 	    print_and_simkill();
   // 	 end
   // 	 // ------------------------------------------------------------ //
   //    end
   // end // block: c1tx_mcl_check


   /*
    * Check memory transactions in flight, maintain active list
    */
   longint rd_active_addr_array[*];
   longint wr_active_addr_array[*];

   string  waw_haz_str;
   string  raw_haz_str;
   string  war_haz_str;

   logic   hazard_found;

   // ------------------------------------------- //
   // Hazard check process
   // - Take in address, check if exists in
   // ------------------------------------------- //
   always @(posedge clk) begin
      // ------------------------------------------- //
      // Read in (unroll necessary)
      // ------------------------------------------- //
      if (haz_if.read_in.valid) begin
	 for (int ii = 0; ii <= haz_if.read_in.hdr.len ; ii = ii + 1) begin
	    rd_active_addr_array[ haz_if.read_in.hdr.addr + ii ] = haz_if.read_in.hdr.addr + ii;
	    // Check for outstanding write request
	    if (wr_active_addr_array.exists(haz_if.read_in.hdr.addr + ii)) begin
	       $sformat(war_haz_str,
			"%d | Potential for Write-after-Read hazard with potential for C0TxHdr=%s arriving earlier than write to same address\n",
			$time,
			return_txhdr(haz_if.read_in.hdr));
	       print_message_and_log(1, war_haz_str);
	    end
	 end
      end
      // ------------------------------------------- //
      // Write in
      // ------------------------------------------- //
      if (haz_if.write_in.valid) begin
	 // Check for outstanding read
	 if (rd_active_addr_array.exists(haz_if.write_in.hdr.addr)) begin
	    $sformat(raw_haz_str,
		     "%d | Potential for Read-after-Write hazard with potential for C1TxHdr=%s arriving earlier than write to same address\n",
		     $time,
		     return_txhdr(haz_if.write_in.hdr));
	    print_message_and_log(1, raw_haz_str);
	 end
	 // Check for outstanding write
	 else if (wr_active_addr_array.exists(haz_if.write_in.hdr.addr)) begin
	    $sformat(waw_haz_str,
		     "%d | Potential for Write-after-Write hazard with potential for C1TxHdr=%s arriving earlier than write to same address\n",
		     $time,
		     return_txhdr(haz_if.write_in.hdr));
	    print_message_and_log(1, waw_haz_str);
	 end
	 // If not, store
	 else begin
	    wr_active_addr_array[haz_if.write_in.hdr.addr] = haz_if.write_in.hdr.addr;
	 end
      end
      // ------------------------------------------- //
      // Read out (delete from active list)
      // ------------------------------------------- //
      if (haz_if.read_out.valid) begin
	 if (rd_active_addr_array.exists( haz_if.read_out.hdr.addr )) begin
	    rd_active_addr_array.delete( haz_if.read_out.hdr.addr );
	 end
      end
      // ------------------------------------------- //
      // Write out (delete from active list)
      // ------------------------------------------- //
      if (haz_if.write_out.valid) begin
	 if (wr_active_addr_array.exists( haz_if.write_out.hdr.addr )) begin
	    wr_active_addr_array.delete( haz_if.write_out.hdr.addr );
	 end
      end
   end


   /*
    * Multiple outstandind MMIO Response tracking
    * - Maintains `MMIO_MAX_OUTSTANDING records tracking activity
    * - Tracking key = MMIO TID
    */
   // parameter int      MMIO_TRACKER_DEPTH = 2**CCIP_CFGHDR_TID_WIDTH;

   // // Tracker structure
   // typedef struct {
   //    // Status management
   //    logic [`MMIO_RESPONSE_TIMEOUT_RADIX-1:0] timer_val;
   //    logic 				       timeout;
   //    logic [CCIP_CFGHDR_TID_WIDTH-1:0]        tid;
   //    logic 				       active;
   // } mmioread_track_t;
   // mmioread_track_t mmioread_tracker[0:MMIO_TRACKER_DEPTH-1];

   // // Tracker status array
   // logic [0:MMIO_TRACKER_DEPTH-1] 	       mmio_tracker_active_array;

   // // Push/pop control process
   // task update_mmio_activity(
   // 			     logic 			       clear,
   // 			     logic 			       push,
   // 			     logic 			       pop,
   // 			     logic [CCIP_CFGHDR_TID_WIDTH-1:0] tid
   // 			     );
   //    begin
   // 	 if (clear) begin
   // 	    mmioread_tracker[tid].active = 0;
   // 	 end
   // 	 else begin
   // 	    // Push TID
   // 	    mmioread_tracker[tid].tid = tid;
   // 	    // Active setting
   // 	    case ({push, pop})
   // 	      2'b10: mmioread_tracker[tid].active = 1;
   // 	      2'b01: mmioread_tracker[tid].active = 0;
   // 	      2'b11:
   // 		begin
   // 		   print_message_and_log(0, "Tracker push-pop occured at the same time, this is an error");
   // 		   print_and_simkill();
   // 		end
   // 	    endcase // case ({push, pop})
   // 	    // If pop occured when not active
   // 	    if (~mmioread_tracker[tid].active && push) begin
   // 	       print_message_and_log(0, "ASE detected an unsolicited MMIO Read response !!\n");
   //    	       print_message_and_log(0, "In system, this can cause a crash");
   //    	       print_and_simkill();
   // 	    end
   // 	 end
   //    end
   // endtask

   // // Push/pop glue
   // always @(posedge clk) begin
   //    if (SoftReset) begin
   // 	 for (int track_i = 0; track_i < MMIO_TRACKER_DEPTH ; track_i = track_i + 1) begin
   // 	    update_mmio_activity(1, 0, 0, 0);
   // 	 end
   //    end
   //    else begin
   // 	 // Push task
   // 	 if (ccip_rx.c0.mmioRdValid) begin
   // 	    update_mmio_activity(0, ccip_rx.c0.mmioRdValid, 0, C0RxCfg.tid);
   // 	 end
   // 	 // Pop task
   // 	 if (ccip_tx.c2.mmioRdValid) begin
   // 	    update_mmio_activity(0, 0, ccip_tx.c2.mmioRdValid, ccip_tx.c2.hdr.tid);
   // 	 end
   //    end
   // end

   // // Tracker block
   // generate
   //    for (genvar ii = 0; ii < MMIO_TRACKER_DEPTH ; ii = ii + 1) begin : mmio_tracker_block
   // 	 // Counter value
   // 	 always @(posedge clk) begin
   // 	    if (SoftReset) begin
   // 	       mmioread_tracker[ii].timer_val <= 0;
   // 	    end
   // 	    else begin
   // 	       if (~mmioread_tracker[ii].active) begin
   // 		  mmioread_tracker[ii].timer_val <= 0;
   // 	       end
   // 	       else if (mmioread_tracker[ii].active) begin
   // 		  mmioread_tracker[ii].timer_val <= mmioread_tracker[ii].timer_val + 1;
   // 	       end
   // 	    end
   // 	 end // always @ (posedge clk)

   // 	 // Timeout flag
   // 	 always @(posedge clk) begin
   // 	    if (SoftReset|~mmioread_tracker[ii].active) begin
   // 	       mmioread_tracker[ii].timeout <= 0;
   // 	    end
   // 	    else if (mmioread_tracker[ii].timer_val >= `MMIO_RESPONSE_TIMEOUT) begin
   // 	       mmioread_tracker[ii].timeout <= 1;
   // 	       print_message_and_log(0, "ASE timed out waiting for MMIO Read response to arrive !!");
   // 	       print_message_and_log(0, "MMIO Read responses must return in 512 cycles");
   // 	       print_and_simkill();
   // 	    end
   // 	 end

   // 	 // Global flag
   // 	 assign mmio_tracker_active_array[ii]  = mmioread_tracker[ii].active;

   //    end
   // endgenerate

endmodule // cci_sniffer
