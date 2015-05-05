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


module cci_sniffer
  #(
    parameter int TX_HDR_WIDTH = 61,
    parameter int RX_HDR_WIDTH = 18,
    parameter int DATA_WIDTH = 512
    )
   (
    // CCI-standard interface
    input logic 		   clk,
    input logic 		   resetb ,
    input logic 		   lp_initdone ,
    input logic [TX_HDR_WIDTH-1:0] tx_c0_header,
    input logic 		   tx_c0_rdvalid,
    input logic [TX_HDR_WIDTH-1:0] tx_c1_header,
    input logic [DATA_WIDTH-1:0]   tx_c1_data,
    input logic 		   tx_c1_wrvalid,
    input logic 		   tx_c1_intrvalid,
    input logic [RX_HDR_WIDTH-1:0] rx_c0_header,
    input logic [DATA_WIDTH-1:0]   rx_c0_data,
    input logic 		   rx_c0_cfgvalid,
    input logic 		   rx_c0_rdvalid,
    input logic 		   rx_c0_wrvalid,
    input logic [RX_HDR_WIDTH-1:0] rx_c1_header,
    input logic 		   rx_c1_wrvalid
    );

   int 				   log_started;   
   int 				   fd_sniffer;

   logic 			   xz_tx0_flag;
   logic 			   xz_tx1_flag;
   logic 			   xz_rx0_flag;
   logic 			   xz_rx1_flag;

   /*
    * FUNCTIONS
    */
   // XZ message
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

   // Hazard warning
   function void print_hazard_message ( logic [`PHYSCLADDR_WIDTH-1:0] cl_addr );
      begin
	 // Set message
	 `BEGIN_RED_FONTCOLOR;
	 $display ("Clock ", $time, " => Possible Data hazard at cache line address %x ", cl_addr);
	 `END_RED_FONTCOLOR;
	 $fwrite( fd_sniffer, "Clock ", $time, " => \n");
	 $fwrite( fd_sniffer, "      Possible Data hazard at cache line addrress => %x\n", cl_addr);
	 $fwrite( fd_sniffer, "\n");
      end
   endfunction


   /*
    * Warning engine
    */
   initial begin
      // log_started = 0;      
      fd_sniffer = $fopen("warnings.log", "w");
      forever begin
	 // XZ messages
	 if ((xz_tx0_flag == `VLOG_HIIMP) || (xz_tx0_flag == `VLOG_UNDEF))
	   print_xz_message ( "CCI-TX0" );
	 if ((xz_tx1_flag == `VLOG_HIIMP) || (xz_tx1_flag == `VLOG_UNDEF))
	   print_xz_message ( "CCI-TX1" );
	 if ((xz_rx0_flag == `VLOG_HIIMP) || (xz_rx0_flag == `VLOG_UNDEF))
	   print_xz_message ( "CCI-RX0" );
	 if ((xz_rx1_flag == `VLOG_HIIMP) || (xz_rx1_flag == `VLOG_UNDEF))
	   print_xz_message ( "CCI-RX1" );
	 // Wait till next clock
	 @(posedge clk);
      end
   end

   /*
    * TX checker files
    */
   assign xz_tx0_flag = (^tx_c0_header)                && tx_c0_rdvalid ;
   assign xz_tx1_flag = (^tx_c1_header || ^tx_c1_data) && tx_c1_wrvalid ;
   assign xz_rx0_flag = (^rx_c0_header || ^rx_c0_data) && (rx_c0_cfgvalid || rx_c0_wrvalid || rx_c0_rdvalid) ;
   assign xz_rx1_flag = ^rx_c1_header                  && rx_c1_wrvalid ;

   /*
    * Data hazard warning engine
    * - Store address as a searchable primary key
    */
   int unsigned active_addresses[*];

   always @(posedge clk) begin
      // Push function
      if (tx_c0_rdvalid) begin
	 if (~active_addresses.exists(tx_c0_header[`TX_CLADDR_BITRANGE])) begin
	    active_addresses[tx_c0_header[`TX_CLADDR_BITRANGE]] = tx_c0_header[`TX_MDATA_BITRANGE];
	 end
	 else begin
	    print_hazard_message(tx_c0_header[`TX_CLADDR_BITRANGE]);
	    active_addresses[tx_c0_header[`TX_CLADDR_BITRANGE]] = tx_c0_header[`TX_MDATA_BITRANGE];
	 end
      end
      if (tx_c1_wrvalid) begin
	 if (~active_addresses.exists(tx_c1_header[`TX_CLADDR_BITRANGE])) begin
	    active_addresses[tx_c1_header[`TX_CLADDR_BITRANGE]] = tx_c1_header[`TX_MDATA_BITRANGE];
	 end
	 else begin
	    print_hazard_message(tx_c1_header[`TX_CLADDR_BITRANGE]);
	    active_addresses[tx_c1_header[`TX_CLADDR_BITRANGE]] = tx_c1_header[`TX_MDATA_BITRANGE];
	 end
      end
      // Pop function
      if (rx_c0_rdvalid) begin
	 if (active_addresses.exists(rx_c0_header[`RX_MDATA_BITRANGE]))
	   active_addresses.delete(rx_c0_header[`RX_MDATA_BITRANGE]);
      end
      if (rx_c0_wrvalid) begin
	 if (active_addresses.exists(rx_c0_header[`RX_MDATA_BITRANGE]))
	   active_addresses.delete(rx_c0_header[`RX_MDATA_BITRANGE]);
      end
      if (rx_c1_wrvalid) begin
	 if (active_addresses.exists(rx_c1_header[`RX_MDATA_BITRANGE]))
	   active_addresses.delete(rx_c1_header[`RX_MDATA_BITRANGE]);
      end
   end


endmodule // cci_sniffer
