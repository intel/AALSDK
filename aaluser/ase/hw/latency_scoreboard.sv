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
 * Module Info: Latency modeling scoreboard system
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * - Transactions are stored when request comes from AFU
 * - Random number generator chooses a delay component between MIN_DELAY & MAX_DELAY
 * - When a request's "time has come", it gets called by cci_emulator
 *   - This is a normal DPI-C call to C functions
 * - When a response is received, the response is queued in normal format
 *
 * THIS COMPONENT
 * - simply re-orders requests and sends them out
 * - May not necessarily be synthesizable
 *
 * OPERATION:
 * - {meta_in, data_in} is validated with write_en signal
 *   - An empty slot is found, a random delay is computed based on pre-known parameters
 *   - The state machine is kicked off.
 *
 *             --------------------------------------------
 *             |               ------------               |
 *             | -----------   | latency  |   ----------- |
 *   CCI port ==>|stg1_fifo|==>|scoreboard|==>|stg3_fifo|==> ASE stubs
 *             | -----------   | buffer   |   ----------- |
 *             |               ------------               |
 *             --------------------------------------------
 *
 * GENERICS:
 * - NUM_TRANSACTIONS : Number of transactions in latency buffer
 * - FIFO_FULL_THRESH : FIFO full threshold
 * - FIFO_DEPTH_BASE2 : FIFO depth radix
 *
 */

`include "ase_global.vh"
`include "platform.vh"

module latency_scoreboard
  #(
    parameter int NUM_TRANSACTIONS = 128,
    parameter int HDR_WIDTH = 61,
    parameter int DATA_WIDTH = 512,
    parameter int COUNT_WIDTH = 8,
    parameter int FIFO_FULL_THRESH = 5,
    parameter int FIFO_DEPTH_BASE2 = 3
    )
   (
    input logic 		  clk,
    input logic 		  rst,
    // Transaction in
    input logic [HDR_WIDTH-1:0]   meta_in,
    input logic [DATA_WIDTH-1:0]  data_in,
    input logic 		  write_en,
    // Transaction out
    output logic [HDR_WIDTH-1:0]  meta_out,
    output logic [DATA_WIDTH-1:0] data_out,
    output logic 		  valid_out,
    input logic 		  read_en,
    // Status signals
    output logic 		  empty,
    output logic 		  full,
    output logic 		  overflow,
    output logic 		  underflow
    );


   /*
    * Declarations
    */
   logic 			  assert_wrfence;
   logic [0:NUM_TRANSACTIONS-1]   latbuf_status;
   logic 			  latbuf_empty;
   logic 			  latbuf_full;

   // Stage 1 outputs
   logic [HDR_WIDTH-1:0] 	  q_meta;
   logic [DATA_WIDTH-1:0] 	  q_data;
   logic 			  q_valid;
   logic [HDR_WIDTH-1:0] 	  qq_meta;
   logic [DATA_WIDTH-1:0] 	  qq_data;
   logic 			  qq_valid;


   logic 			  stg1_empty;
   logic 			  stg1_full;
   logic 			  stg1_pop;

   /*
    * Flow errors
    */
   logic 			  stg1_overflow, stg3_overflow;
   logic 			  stg1_underflow, stg3_underflow;

   assign overflow = stg1_overflow | stg3_overflow;
   assign underflow = stg1_underflow | stg3_underflow;

   // Message processes
   always @(posedge clk) begin
      if (overflow) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV : %m => *** OVERFLOW DETECTED ***");
	 `END_RED_FONTCOLOR;
      end
      if (underflow) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV : %m => *** UNDERFLOW DETECTED ***");
	 `END_RED_FONTCOLOR;
      end
   end


   // Enumerate states
   typedef enum {LatSc_Disabled, LatSc_Enabled, LatSc_Countdown, LatSc_DoneReady, LatSc_PopRecord} latsc_fsmState;

   // PUSH FSM
   //typedef enum {Push_Disabled, Push_Enabled, Push_WriteFence, Push_WriteFence_1, Push_WriteFence_2, Push_WriteFence_3, Push_WaitState_1, Push_WaitState_2}  push_fsmState;
   typedef enum {Push_Enabled, Push_WriteFence, Push_WaitState}  push_fsmState;
   push_fsmState push_state;

   // POP FSM
   typedef enum {Pop_Disabled, Pop_Enabled} pop_fsmState;
   pop_fsmState pop_state;


   // Transaction storage
   typedef struct packed
		  {
		     logic [HDR_WIDTH-1:0]   meta;
		     logic [DATA_WIDTH-1:0]  data;
		     logic [COUNT_WIDTH-1:0] ctr_out;
		     logic 		     ready_to_go;
		     logic 		     record_valid;
//		     logic 		     push_sel;
		     logic 		     pop_sel;
		     latsc_fsmState          state;
		     } transact_t;

   // Array of stored transactions
   transact_t records[NUM_TRANSACTIONS] ;
   // logic [0:NUM_TRANSACTIONS-1] 	     transact_pop_stg1_arr;
   logic [0:NUM_TRANSACTIONS-1] 	     transact_push_stg3_arr;

   // stage 3 signals
   logic [HDR_WIDTH+DATA_WIDTH-1:0] 	     stg3_din;
   logic 				     stg3_wen;
   logic 				     stg3_full;
   int 					     jj;
   int 					     ii;
   int 					     push_slot_num;
   int 					     pop_slot_num;

   /*
    * Find a next free slot
    */
   function integer find_next_free_slot();
      int 				     find_iter;
      int 				     ret_free_slot;
      begin
	 for(find_iter = 0; find_iter < NUM_TRANSACTIONS; find_iter = find_iter + 1) begin
	 // for(find_iter = push_slot_num; find_iter < push_slot_num + NUM_TRANSACTIONS; find_iter = find_iter + 1) begin
	    if (records[find_iter].record_valid == 1'b0) begin
	       ret_free_slot = find_iter;	       
	       break;
	    end
	 end
	 return ret_free_slot;
      end
   endfunction

   /*
    * Find random_delay between MIN_DELAY & MAX_DELAY
    */
   function integer get_random_delay( int meta );
      int ret_random_lat;
      begin
	 // Set random seed
	 // $srandom ($time);
	 // Select a random latency
	 case (meta)
	   // ReadLine
	   `ASE_TX0_RDLINE:
	     begin
	   	ret_random_lat = $urandom_range (`RDLINE_LATRANGE);
	     end

	   // WriteLine
	   `ASE_TX1_WRLINE:
	     begin
		ret_random_lat = $urandom_range (`WRLINE_LATRANGE);
	     end

	   // WriteThru
	   `ASE_TX1_WRTHRU:
	     begin
		ret_random_lat = $urandom_range (`WRTHRU_LATRANGE);
	     end

	   // WriteFence
	   `ASE_TX1_WRFENCE:
	     begin
		ret_random_lat = 1;
	     end

	   // IntrValid
	   `ASE_TX1_INTRVALID:
	     begin
		ret_random_lat = $urandom_range (`INTR_LATRANGE);
	     end

	   // Unspecified type (warn but specify latency
	   default:
	     begin
		`BEGIN_YELLOW_FONTCOLOR;
		$display("SIM-SV: %m =>");
		$display("No Latency model available for meta type %x, using LAT_UNDEFINED", meta[`TX_META_TYPERANGE]);
		`END_YELLOW_FONTCOLOR;
		ret_random_lat = `LAT_UNDEFINED;
	     end

	 endcase // case (meta)
	 // Return random latency value
	 return ret_random_lat;
      end
   endfunction

   /*
    * Find a transaction to release to output stage
    */
   function integer find_pop_ready_slot(int last_popped_slot);
      int ret_ready_slot;
      int start_ptr;
      int pop_iter;
      int sel_slot;
      begin
	 start_ptr = last_popped_slot;
	 for(pop_iter = start_ptr; pop_iter < start_ptr + NUM_TRANSACTIONS; pop_iter = pop_iter + 1) begin
	    sel_slot = pop_iter % NUM_TRANSACTIONS ;
	    if ( records[sel_slot].ready_to_go == 1'b1 ) begin
	       ret_ready_slot = sel_slot;
	       break;
	    end
	 end
	 return ret_ready_slot;
      end
   endfunction

   /*
    * Stage I: INFIFO
    * - This module stages input. A FIFO is used instead of a register
    *   in order to accomodate for bursts on the CCI interface
    */
   ase_fifo
     #(
       .DATA_WIDTH     (HDR_WIDTH + DATA_WIDTH),
       .DEPTH_BASE2    (FIFO_DEPTH_BASE2),
       .ALMFULL_THRESH (FIFO_FULL_THRESH)
       )
   infifo
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (write_en),
      .data_in    ({meta_in, data_in}),
      .rd_en      (stg1_pop),
      .data_out   ({q_meta, q_data}),
      .data_out_v (q_valid),
      .alm_full   (stg1_full),
      .full       (),
      .empty      (stg1_empty),
      .count      (),
      .overflow   (stg1_overflow),
      .underflow  (stg1_underflow)
      );



endmodule // latency_scoreboard
