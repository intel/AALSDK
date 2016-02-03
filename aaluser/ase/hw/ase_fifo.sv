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
 * Module Info:
 * Language   : System{Verilog} 
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 * 
 * FIFO implementation for use in ASE only
 * Generics: 
 * - DEPTH_BASE2    : Radix of element array, used for counting elements
 * - ALMFULL_THRESH : AlmostFull threshold
 * 
 * Description:
 * - WRITE: Data is written when write_enable is HIGH
 * - READ : Data is available in next clock it is in RAM
 *          Empty is an ASYNC signal & must be check
 *          read_enable must be used asynchronously with EMPTY signal
 * - Overflow and underflow signals are asserted
 * 
 */
 
module ase_fifo
  #(
    parameter int DATA_WIDTH = 64,
    parameter int DEPTH_BASE2 = 3,
    parameter int ALMFULL_THRESH = 5
    )
   (
    input logic 		  clk,
    input logic 		  rst,
    input logic 		  wr_en,
    input logic [DATA_WIDTH-1:0]  data_in,
    input logic 		  rd_en,
    output logic [DATA_WIDTH-1:0] data_out,
    output logic 		  data_out_v,
    output logic 		  alm_full,
    output logic 		  full,
    output logic 		  empty,
    output logic [DEPTH_BASE2:0]  count,
    output logic 		  overflow,
    output logic 		  underflow
    );

   logic 			  prog_full;   
   logic 			  mywr_en;
   logic 			  myrd_en;
   logic 			  empty_current;
   logic 			  full_current;
   logic 			  full_next;
   logic [DEPTH_BASE2-1:0] 	  rd_addr;
   logic [DEPTH_BASE2-1:0] 	  wr_addr;
   logic [DEPTH_BASE2:0] 	  counter;
   logic [DEPTH_BASE2:0] 	  counter_reg;
   
   /*
    * Filtering legal transactions
    * Overflow and underflow will not corrupt RAM contents
    */ 
   assign mywr_en = wr_en & (~full_current);
   assign myrd_en = rd_en & (~empty_current);   
   assign count   = counter [DEPTH_BASE2-1:0];

   /*
    * Memory instance
    */ 
   sdp_ram #(
	     .DEPTH_BASE2 (DEPTH_BASE2), 
             .DATA_WIDTH  (DATA_WIDTH)
	     ) 
   RAM_i (
          .clk   (clk),
          .we    (mywr_en),
          .waddr (wr_addr),
          .din   (data_in),
          .raddr (rd_addr),
          .dout  (data_out)
          );

   /*
    * Read & Write address movement
    */
   // writing pointer doesn't change when overflow
   always @(posedge clk) begin
      if (rst == 1'b1)
	wr_addr <= 0;
      else begin
	 if (mywr_en) 
	   wr_addr <= wr_addr + 1'b1;
      end
   end
   
   // reading empty FIFO will not get valid data, reading pointer doesn't change.
   always @(posedge clk) begin
      if (rst == 1'b1)
	rd_addr <= 0;
      else begin
	 if (rd_en & (~empty_current)) 
	   rd_addr <= rd_addr + 1'b1;
      end
   end

   /*
    * Data valid signals
    */ 
   always @(*) begin
      if (rst)
	data_out_v <= 1'b0;	 
      else
	data_out_v <= rd_en && ~empty_current;	 
   end     

   /*
    * Overflow & Underflow signals
    */ 
   // underflow being asserted for one cycle means unsuccessful read
   always @(posedge clk) begin
      if (rst == 1'b1)
	underflow <= 0;
      else
	underflow <= rd_en & empty_current;
   end

   // overflow being asserted for one cycle means one incoming data was discarded
   always @(posedge clk) begin
      if (rst == 1'b1)
	overflow <= 0;
      else
	overflow <= wr_en & full_current;
   end

   /*
    * FIFO Counter
    */
   always @(*) begin
      if (rst == 1'b1)
	counter <= 0;
      else begin
	 // counter <= counter - (rd_en & (~empty_current)) + (wr_en & (~full_current));
	 casex ({(rd_en && ~empty_current), (wr_en && ~full_current)})
	   2'b01  : counter <= counter_reg + 1;
	   2'b10  : counter <= counter_reg - 1;
	   default: counter <= counter_reg;
	 endcase	 
      end
   end

   always @(posedge clk)
     counter_reg <= counter;
   
   /*
    * Asynchronous EMPTY signal
    */ 
   always @(*) begin
      if (rst) begin
	 empty_current <= 1;
      end
      else begin
	 if (counter_reg == 0) begin
	    empty_current <= 1;	    
	 end
	 else begin
	    empty_current <= 0;	    
	 end
      end
   end
   assign empty = empty_current;
   
   /*
    * FIFO full state machine
    */
   always @(*) begin
      case (full_current)
	0: 
	  begin
	     if ((&counter[DEPTH_BASE2-1:0]) & (~counter[DEPTH_BASE2]) & (~rd_en) & (wr_en))
	       full_next = 1;
	     else
	       full_next = 0;
	  end
	1:
	  begin
	     if (rd_en)
	       full_next = 0;
	     else
	       full_next = 1;
	  end
      endcase
   end

   always @(posedge clk) begin
      if (rst == 1'b1)
	full_current <= 0;
      else
	full_current <= full_next;
   end   
   assign full = full_current;

   /*
    * Programmable full signal
    */ 
   always @(*) begin
      if (counter > ALMFULL_THRESH)
	prog_full <= 1;
      else
	prog_full <= 0;      
   end			   
   assign alm_full = prog_full;

   /*
    * Display flow errors
    */ 
   always @(posedge clk) begin
      if (overflow)  $display ("\t%m => *** OVERFLOW DETECTED ***");
      if (underflow) $display ("\t%m => *** UNDERFLOW DETECTED ***");      
   end       
   
endmodule


