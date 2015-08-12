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

module cci_logger
  #(
    parameter INTERFACE_TYPE = "CCIS", // Can be "CCIS" or "SPL"
    parameter LOGNAME   = "ccis_transactions.tsv"
    )
   (
    // Configure enable
    input int                               enable_logger,
    input int                               finish_logger,
    // CCI interface
    input logic                             clk      ,               // out
    input logic                             sys_reset_n,             // out
    input logic                             sw_reset_n,              // out
    input logic 			    lp_initdone ,            // out
    input logic  [`CCI_TX_HDR_WIDTH-1:0]    tx_c0_header,            // in
    input logic 			    tx_c0_rdvalid,           // in
    input logic 			    tx_c0_almostfull,        // out
    input logic [`CCI_TX_HDR_WIDTH-1:0]     tx_c1_header,            // in
    input logic [`CCI_DATA_WIDTH-1:0] 	    tx_c1_data,              // in
    input logic 			    tx_c1_wrvalid,           // in
    input logic 		            tx_c1_almostfull,        // out
    input logic 			    tx_c1_intrvalid,         // in
    input logic [`ASE_CCI_RX_HDR_WIDTH-1:0] rx_c0_header,            // out
    input logic [`CCI_DATA_WIDTH-1:0] 	    rx_c0_data,              // out
    input logic 			    rx_c0_rdvalid,           // out
    input logic 			    rx_c0_wrvalid,           // out
    input logic 			    rx_c0_cfgvalid,          // out
    input logic [`ASE_CCI_RX_HDR_WIDTH-1:0] rx_c1_header,            // out
    input logic 			    rx_c1_wrvalid,           // out
    input logic 			    rx_c0_umsgvalid,         // out
    input logic 			    rx_c0_intrvalid,         // out
    input logic 			    rx_c1_intrvalid          // out   
    );

   /*
    * ASE Hardware Interface (CCI) logger
    * - Logs CCI transaction into a transactions.tsv file
    * - Watch for "*valid", and write transaction to log name
    */
   // Log file descriptor
   int 					    log_fd;

   // Reset management
   logic 				    lp_initdone_q;   
   logic 				    sys_reset_n_q;
   logic 				    sw_reset_n_q;

   // generate
   //    if (INTERFACE_TYPE == "CCIS") begin
   //    end
   //    if (INTERFACE_TYPE == "SPL") begin
   //    end
   // endgenerate

   // Registers for comparing previous states
   always @(posedge clk) begin
      lp_initdone_q	<= lp_initdone;
      sw_reset_n_q	<= sw_reset_n;
      sys_reset_n_q     <= sys_reset_n;
   end

   
   /*
    * Watcher process
    */
   initial begin : logger_proc
      // Display
      $display("SIM-SV: CCI Logger started");

      // Open transactions.tsv file
      log_fd = $fopen("transactions.tsv", "w");

      // Headers
      $fwrite(log_fd, "\tTime\tTransactionType\tChannel\tMetaInfo\tCacheAddr\tData\n");

      // Watch CCI port
      forever begin
	 // If LP_initdone changed, log the event
	 if (lp_initdone_q != lp_initdone) begin
	    $fwrite(log_fd, "%d\tLP_initdone toggled from %b to %b\n", $time, lp_initdone_q, lp_initdone);
	 end
	 // Indicate Software controlled reset
	 if (sw_reset_n_q != sw_reset_n) begin
	    $fwrite(log_fd, "%d\tSoftware reset toggled from %b to %b\n", $time, sw_reset_n_q, sw_reset_n);
	 end
	 // If reset toggled, log the event
	 if (sys_reset_n_q != sys_reset_n) begin
	    $fwrite(log_fd, "%d\tSystem reset toggled from %b to %b\n", $time, sys_reset_n_q, sys_reset_n);
	 end
	 // Watch CCI for valid transactions
	 if (lp_initdone) begin
	    ////////////////////////////// RX0 cfgvalid /////////////////////////////////
	    if (rx_c0_cfgvalid) begin
	       $fwrite(log_fd, "%d\tCSRWrite\t\t\t%x\t%x\n", $time, rx_c0_header[`RX_CSR_BITRANGE], rx_c0_data[31:0]);
	       if (enable_logger) $display("%d\tCSRWrite\t\t\t%x\t%x", $time, rx_c0_header[`RX_CSR_BITRANGE], rx_c0_data[31:0]);
	    end
	    /////////////////////////////// RX0 wrvalid /////////////////////////////////
	    if (rx_c0_wrvalid) begin
	       $fwrite(log_fd, "%d\tWrResp\t\t0\t%x\tNA\tNA\n", $time, rx_c0_header[`RX_MDATA_BITRANGE] );
	       if (enable_logger) $display("%d\tWrResp\t\t0\t%x\tNA\tNA", $time, rx_c0_header[`RX_MDATA_BITRANGE] );
	    end
	    /////////////////////////////// RX0 rdvalid /////////////////////////////////
	    if (rx_c0_rdvalid) begin
	       $fwrite(log_fd, "%d\tRdResp\t\t0\t%x\tNA\t%x\n", $time, rx_c0_header[`RX_MDATA_BITRANGE], rx_c0_data );
	       if (enable_logger) $display("%d\tRdResp\t\t0\t%x\tNA\t%x", $time, rx_c0_header[`RX_MDATA_BITRANGE], rx_c0_data );
	    end
	    ////////////////////////////// RX0 umsgvalid ////////////////////////////////
	    if (rx_c0_umsgvalid) begin
	       if (rx_c0_header[`CCI_UMSG_BITINDEX]) begin              // Umsg Hint
		  $fwrite(log_fd, "%d\tUmsgHint\t0\t%x\n", $time, rx_c0_header[5:0] );
		  if (enable_logger) $display("%d\tUmsgHint\t0\t%x\n", $time, rx_c0_header[5:0] );
	       end
	       else begin                                               // Umsg with data
		  $fwrite(log_fd, "%d\tUmsgData\t0\t%x\t%x\n", $time, rx_c0_header[5:0], rx_c0_data );
		  if (enable_logger) $display("%d\tUmsgData\t0\t%x\n", $time, rx_c0_data );
	       end
	    end
	    /////////////////////////////// RX1 wrvalid /////////////////////////////////
	    if (rx_c1_wrvalid) begin
	       $fwrite(log_fd, "%d\tWrResp\t\t1\t%x\tNA\tNA\n", $time, rx_c1_header[`RX_MDATA_BITRANGE] );
	       if (enable_logger) $display("%d\tWrResp\t\t1\t%x\tNA\tNA", $time, rx_c1_header[`RX_MDATA_BITRANGE] );
	    end
	    /////////////////////////////// TX0 rdvalid /////////////////////////////////
	    if (tx_c0_rdvalid) begin
	       if ((tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_S) || (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE)) begin
		  $fwrite(log_fd, "%d\tRdLineReq_S\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (enable_logger) $display("%d\tRdLineReq_S\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else if (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_I) begin
		  $fwrite(log_fd, "%d\tRdLineReq_I\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (enable_logger) $display("%d\tRdLineReq_I\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else if (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_O) begin
		  $fwrite(log_fd, "%d\tRdLineReq_O\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (enable_logger) $display("%d\tRdLineReq_O\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else begin
		  $fwrite(log_fd, "ReadValid on TX-CH0 validated an UNKNOWN Request type at t = %d \n", $time);
	       end
	    end
	    /////////////////////////////// TX1 wrvalid /////////////////////////////////
	    if (tx_c1_wrvalid) begin
	       if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRTHRU) begin
		  $fwrite(log_fd, "%d\tWrThruReq\t1\t%x\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
		  if (enable_logger) $display("%d\tWrThruReq\t1\t%x\t%x\t%x", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
	       end
	       else if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRLINE) begin
		  $fwrite(log_fd, "%d\tWrLineReq\t1\t%x\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
		  if (enable_logger) $display("%d\tWrLineReq\t1\t%x\t%x\t%x", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
	       end
	       else if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRFENCE) begin
		  $fwrite(log_fd, "%d\tWriteFence\t1\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14]);
		  if (enable_logger) $display("%d\tWriteFence\t1\t%x\t%x", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14]);
	       end
	       else begin
		  $fwrite(log_fd, "WriteValid on TX-CH1 validated an UNKNOWN Request type at t = %d \n", $time);
		  if (enable_logger) $display("WriteValid on TX-CH1 validated an UNKNOWN Request type at t = %d \n", $time);
	       end
	    end
	    ////////////////////////////// FINISH command ////////////////////////////////
	    if (finish_logger == 1) begin
	       $fclose(log_fd);	       
	    end
	 end
	 // Wait till next clock
	 @(posedge clk);
      end
   end

   
   
endmodule
