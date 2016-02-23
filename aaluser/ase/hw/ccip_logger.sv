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

   /*
    * ASE Hardware Interface (CCI) logger
    * - Logs CCI transaction into a transactions.tsv file
    * - Watch for "*valid", and write transaction to log name
    */
   // Log file descriptor
   int 					    log_fd;

   // Reset management
   logic 				    SoftReset_q;

   // Registers for comparing previous states
   always @(posedge clk) begin
      SoftReset_q	<= SoftReset;
   end

   // Config header
   CfgHdr_t C0RxMmioHdr;
   assign C0RxMmioHdr = CfgHdr_t'(C0RxHdr);

   // Umsg header
   UMsgHdr_t C0RxUMsgHdr;
   assign C0RxUMsgHdr = UMsgHdr_t'(C0RxHdr);

   // Atomics header
   Atomics_t C0RxAtomicHdr;
   assign C0RxAtomicHdr = Atomics_t'(C0RxHdr);


   /*
    * Buffer channels, request and response types
    */
   // Print Channel function
   function string print_channel (ccip_vc_t vc_sel);
      begin
	 case (vc_sel)
	   VC_VA  : return "VA ";
	   VC_VL0 : return "VL0";
	   VC_VH0 : return "VH0";
	   VC_VH1 : return "VH1";
	 endcase
      end
   endfunction

   // Print Req Type
   function string print_reqtype (ccip_reqtype_t req);
      begin
	 case (req)
	   ASE_RDLINE_S   : return "RdLine_S   ";
	   ASE_RDLINE_I   : return "RdLine_I   ";
	   ASE_WRLINE_I   : return "WrLine_I   ";
	   ASE_WRLINE_M   : return "WrLine_M   ";
	   ASE_WRFENCE    : return "WrFence    ";
	   ASE_ATOMIC_REQ : return "AtomicReq  ";
	   ASE_INTR_REQ   : return "IntrReq    ";
	   default        : return "** ERROR %m : Request type unindentified **" ;
	 endcase
      end
   endfunction

   // Print resp type
   function string print_resptype (ccip_resptype_t resp);
      begin
	 case (resp)
	   ASE_RD_RSP      : return "RdResp     ";
	   ASE_WR_RSP      : return "WrResp     ";
	   ASE_WRFENCE_RSP : return "WrFenceRsp ";
	   ASE_INTR_RSP    : return "IntrResp   ";
	   ASE_ATOMIC_RSP  : return "AtomicRsp  ";
	   default         : return "** ERROR %m : Request type unindentified **" ;
	 endcase
      end
   endfunction

   // Print CL number
   function string print_clnum (ccip_len_t len);
      begin
	 case (len)
	   ASE_1CL : return "#1CL";
	   ASE_2CL : return "#2CL";
	   ASE_3CL : return "#3CL";
	   ASE_4CL : return "#4CL";
	   default : return "** ERROR %m : Request type unindentified **" ;
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

   // MMIO Request length
   function int mmioreq_length (logic [1:0] mmio_len);
      begin
	 case (mmio_len)
	   2'b00 : return 4;
	   2'b01 : return 8;
	   2'b10 : return 64;
	 endcase
      end
   endfunction // mmioreq_length

   // Space generator - formatting help
   function string ret_spaces (int num);
      string spaces;
      int    ii;
      begin
	 spaces = "";
	 for (ii = 0; ii < num; ii = ii + 1) begin
	    spaces = {spaces, " "};
	 end
	 return spaces;
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
	 if (SoftReset_q != SoftReset) begin
	    $fwrite(log_fd, "%d\tSoftReset toggled from %b to %b\n", $time, SoftReset_q, SoftReset);
	 end
	 // Buffer messages
	 if (log_string_en) begin
	    $fwrite(log_fd, "-----------------------------------------------------\n");
	    $fwrite(log_fd, "%d\t%s\n", $time, log_string);
	 end
	 /////////////////////// CONFIG CHANNEL TRANSACTIONS //////////////////////////
	 /******************* SW -> AFU MMIO Write *******************/
	 if (C0RxMmioWrValid) begin
	    if (cfg.enable_cl_view)  $display("%d\t   \tMMIOWrReq   \t%x\t%d bytes\t%s\n",
					      $time,
					      C0RxMmioHdr.index,
					      mmioreq_length(C0RxMmioHdr.len),
					      csr_data(mmioreq_length(C0RxMmioHdr.len), C0RxData)  );
	    $fwrite(log_fd, "%d\t   \tMMIOWrReq   \t  \t%x\t%d bytes\t%s\n",
		    $time,
		    C0RxMmioHdr.index,
		    mmioreq_length(C0RxMmioHdr.len),
		    csr_data(mmioreq_length(C0RxMmioHdr.len), C0RxData)  );
	    // $fwrite(log_fd, "%016x\n", C0RxData);
	 end
	 /******************* SW -> AFU MMIO Read *******************/
	 if (C0RxMmioRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\t   \tMMIORdReq   \t%x\t%x\t%d bytes\n",
	    				     $time,
	    				     C0RxMmioHdr.tid,
	    				     C0RxMmioHdr.index,
	    				     mmioreq_length(C0RxMmioHdr.len));
	    $fwrite(log_fd, "%d\t   \tMMIORdReq   \t%x\t%x\t%d bytes\n",
	    	    $time,
	    	    C0RxMmioHdr.tid,
	    	    C0RxMmioHdr.index,
	    	    mmioreq_length(C0RxMmioHdr.len));
	 end
	 //////////////////////// C0 TX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* AFU -> MEM Read Request *****************/
	 if (C0TxRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x\t%s",
	 				     $time,
	 				     print_channel(C0TxHdr.vc),
	 				     print_reqtype(C0TxHdr.reqtype),
	 				     C0TxHdr.mdata,
	 				     C0TxHdr.addr,
					     print_clnum(C0TxHdr.len));
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\t%s\n",
	 	    $time,
	 	    print_channel(C0TxHdr.vc),
	 	    print_reqtype(C0TxHdr.reqtype),
	 	    C0TxHdr.mdata,
	 	    C0TxHdr.addr,
		    print_clnum(C0TxHdr.len));
	 end
	 //////////////////////// C1 TX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* AFU -> MEM Write Request *****************/
	 if (C1TxWrValid && (C1TxHdr.reqtype != ASE_WRFENCE)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%x\t%x\t%s",
	 				     $time,
	 				     print_channel(C1TxHdr.vc),
	 				     print_reqtype(C1TxHdr.reqtype),
	 				     C1TxHdr.mdata,
	 				     C1TxHdr.addr,
	 				     C1TxData,
					     print_clnum(C1TxHdr.len));
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%x\t%x\t%s\n",
	 	    $time,
	 	    print_channel(C1TxHdr.vc),
	 	    print_reqtype(C1TxHdr.reqtype),
	 	    C1TxHdr.mdata,
	 	    C1TxHdr.addr,
	 	    C1TxData,
		    print_clnum(C1TxHdr.len));
	 end // if (C1TxWrValid && (C1TxHdr.reqtype != ASE_WRFENCE))
	 if (C1TxWrValid && (C1TxHdr.reqtype == ASE_WRFENCE)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\tWrFence \t%x\n",
					     $time,
					     print_channel(C1TxHdr.vc),
					     C1TxHdr.mdata);
	    $fwrite(log_fd, "%d\t%s\tWrFence \t%x\n",
		    $time,
		    print_channel(C1TxHdr.vc),
		    C1TxHdr.mdata);
	 end
	 //////////////////////// C2 TX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* AFU -> SW MMIO Read Response *****************/
	 if (C2TxMmioRdValid) begin
	    if (cfg.enable_cl_view) $display("%d\t   \tMMIORdRsp   \t%x\t%x\n",
					     $time,
					     C2TxHdr.tid,
					     C2TxData);
	    $fwrite(log_fd, "%d\t   \tMMIORdRsp   \t%x\t%x\n",
		    $time,
		    C2TxHdr.tid,
		    C2TxData);
	 end
	 //////////////////////// C0 RX CHANNEL TRANSACTIONS //////////////////////////
	 /******************* MEM -> AFU Read Response *****************/
	 if (C0RxRdValid && (C0RxHdr.resptype == ASE_RD_RSP)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%s\t%x",
	 				     $time,
	 				     print_channel(C0RxHdr.vc_used),
	 				     print_resptype(C0RxHdr.resptype),
	 				     C0RxHdr.mdata,
					     print_clnum(C0RxHdr.clnum),
					     // ret_spaces(12),
	 				     C0RxData);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%s\t%x\n",
	 	    $time,
	 	    print_channel(C0RxHdr.vc_used),
	 	    print_resptype(C0RxHdr.resptype),
	 	    C0RxHdr.mdata,
		    print_clnum(C0RxHdr.clnum),
		    // ret_spaces(12),
	 	    C0RxData);
	 end // if (C0RxRdValid && (C0RxHdr.resptype == ASE_RD_RSP))
	 /*********** SW -> MEM -> AFU Atomic responses on C0Rx  **********/
	 if (C0RxRdValid && (C0RxHdr.resptype == ASE_ATOMIC_RSP)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%s\t%x",
					     $time,
					     print_channel(C0RxAtomicHdr.vc_used),
					     print_resptype(C0RxAtomicHdr.resptype),
					     C0RxAtomicHdr.mdata,
					     ((C0RxAtomicHdr.success == 1) ? "Success" : "Fail   "),
					     C0RxData);
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%s\t%x\n",
		    $time,
		    print_channel(C0RxAtomicHdr.vc_used),
		    print_resptype(C0RxAtomicHdr.resptype),
		    C0RxAtomicHdr.mdata,
		    ((C0RxAtomicHdr.success == 1) ? "Success" : "Fail   "),
		    C0RxData);
	 end
	 /*************** SW -> MEM -> AFU Unordered Message  *************/
	 if (C0RxUMsgValid) begin
	    if (C0RxUMsgHdr.umsg_type) begin
	       if (cfg.enable_cl_view) $display("%d\t   \tUMsgHint   \t%d\n",
						$time,
						C0RxUMsgHdr.umsg_id
						);
	       $fwrite(log_fd, "%d\t   \tUMsgHint   \t%d\n",
		       $time,
		       C0RxUMsgHdr.umsg_id
		       );
	    end
	    else if (~C0RxUMsgHdr.umsg_type) begin
	       if (cfg.enable_cl_view) $display("%d\t   \tUMsgData   \t%d\t%x\n",
						$time,
						C0RxUMsgHdr.umsg_id,
						C0RxData
						);
	       $fwrite(log_fd, "%d\t   \tUMsgData   \t%d\t%x\n",
		       $time,
		       C0RxUMsgHdr.umsg_id,
		       C0RxData
		       );
	    end
	 end
	 // /**************** MEM -> AFU Interrupt Response  **************/
	 // if (C0RxIntrValid) begin
	 // end
	 //////////////////////// C1 RX CHANNEL TRANSACTIONS //////////////////////////
	 /****************** MEM -> AFU Write Response  ****************/
	 if (C1RxWrValid && (C1RxHdr.resptype != ASE_WRFENCE_RSP)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\t%s\t%x\t%s",
	 				     $time,
	 				     print_channel(C1RxHdr.vc_used),
	 				     print_resptype(C1RxHdr.resptype),
	 				     C1RxHdr.mdata,
					     print_clnum(C1RxHdr.clnum));
	    $fwrite(log_fd, "%d\t%s\t%s\t%x\t%s\n",
	 	    $time,
	 	    print_channel(C1RxHdr.vc_used),
	 	    print_resptype(C1RxHdr.resptype),
	 	    C1RxHdr.mdata,
		    print_clnum(C1RxHdr.clnum));
	 end // if (C1RxWrValid && (C1RxHdr.resptype != ASE_WRFENCE_RSP))
	 if (C1RxWrValid && (C1RxHdr.resptype == ASE_WRFENCE_RSP)) begin
	    if (cfg.enable_cl_view) $display("%d\t%s\tWrFenceRsp\t%x\n",
					     $time,
					     print_channel(C1RxHdr.vc_used),
					     C1RxHdr.mdata);
	    $fwrite(log_fd, "%d\t%s\tWrFenceRsp\t%x\n",
		    $time,
		    print_channel(C1RxHdr.vc_used),
		    C1RxHdr.mdata);
	 end
	 // /**************** MEM -> AFU Interrupt Response  **************/
	 // if (C1RxIntrValid) begin
	 // end
	 ////////////////////////////// FINISH command ////////////////////////////////
	 if (finish_logger == 1) begin
	    $fclose(log_fd);
	 end
	 //////////////////////////////////////////////////////////////////////////////
	 // Wait till next clock
	 $fflush(log_fd);
	 @(posedge clk);
      end
   end

endmodule
