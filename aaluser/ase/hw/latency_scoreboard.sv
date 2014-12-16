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

   parameter int 		  LATBUF_POP_INVALID = 255;

   /*
    * Setup slot usage order
    */
   int 				  slot_lookup[NUM_TRANSACTIONS];

   // Initialize slot usage order
   initial begin
      int i;
      // Serial order
      for(i = 0 ; i < NUM_TRANSACTIONS ; i = i + 1) begin
      	 slot_lookup[i] = i;
      end
      // Shuffle data using internal function
      slot_lookup.shuffle();
   end

   /*
    * Declarations
    */
   logic 			  assert_wrfence;
   logic 			  wrfence_pop;

   // Latbuf control and status
   logic [0:NUM_TRANSACTIONS-1]   latbuf_status;
   logic [0:NUM_TRANSACTIONS-1]   latbuf_ready;
   logic 			  latbuf_empty;
   logic 			  latbuf_full;
   logic 			  latbuf_push;
   logic 			  latbuf_pop;

   // Stage 1 signals
   logic [HDR_WIDTH-1:0] 	  stg1_meta;
   logic [DATA_WIDTH-1:0] 	  stg1_data;
   logic 			  stg1_valid;
   logic 			  stg1_empty;
   logic 			  stg1_full;
   logic 			  stg1_pop;

   // Stage 1 (write fence filtered)
   logic [HDR_WIDTH-1:0] 	  q_meta;
   logic [DATA_WIDTH-1:0] 	  q_data;
   logic 			  q_valid;
   logic 			  q_full;
   logic 			  q_empty;
   logic 			  q_pop;
   logic 			  q_push;

   // Push/Pop iterator
   int 				  push_ptr;
   int 				  pop_ptr;
   int 				  push_slot_num;
   int 				  pop_slot_num;

   /*
    * Flow errors
    */
   logic 			  stg1_overflow, stg3_overflow;
   logic 			  stg1_underflow, stg3_underflow;
   logic 			  q_overflow, q_underflow;

   assign overflow = stg1_overflow || stg3_overflow || q_overflow;
   assign underflow = stg1_underflow || stg3_underflow || q_underflow;

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

   /*
    * Full-empty status
    */
   assign full = stg1_full;


   /*
    * States & structures
    */
   // Enumerate states
   typedef enum {LatSc_Disabled, LatSc_Enabled, LatSc_Countdown, LatSc_DoneReady, LatSc_PopRecord} latsc_fsmState;

   // PUSH FSM
   typedef enum {Push_Enabled, Push_WriteFence, Push_WaitState}  push_fsmState;
   push_fsmState push_state;

   // POP FSM
   // typedef enum {Pop_Disabled, Pop_Enabled} pop_fsmState;
   // pop_fsmState pop_state;

   // Transaction storage
   typedef struct packed
		  {
		     logic [HDR_WIDTH-1:0]   meta;
		     logic [DATA_WIDTH-1:0]  data;
		     logic [COUNT_WIDTH-1:0] ctr_out;
		     logic 		     ready_to_go;
		     logic 		     record_valid;
		     latsc_fsmState          state;
		     } transact_t;

   // Array of stored transactions
   transact_t records[NUM_TRANSACTIONS] ;

   // stage 3 signals
   logic [HDR_WIDTH+DATA_WIDTH-1:0] 	     stg3_din;
   logic 				     stg3_wen;
   logic 				     stg3_full;
   int 					     jj;
   int 					     ii;

   /*
    * Find a next free slot
    */
   function integer find_next_push_slot();
      int 				     find_iter;
      int 				     ret_free_slot;
      begin
	 if (latbuf_full) begin
	    return LATBUF_POP_INVALID;
	 end
	 else begin
   	    for(find_iter = push_ptr; find_iter < push_ptr + NUM_TRANSACTIONS; find_iter = find_iter + 1) begin
	       ret_free_slot = slot_lookup[find_iter % NUM_TRANSACTIONS];
   	       if (records[ret_free_slot].record_valid == 1'b0) begin
		  push_ptr = find_iter;
   		  return ret_free_slot;
   	       end
   	    end
	 end
      end
   endfunction

   /*
    * Find a transaction to release to output stage
    */
   function integer find_next_pop_slot(int last_popped_slot);
      int ret_pop_slot;
      int start_ptr;
      int pop_iter;
      int sel_slot;
      begin
	 if (~(|latbuf_ready)) begin
	    return LATBUF_POP_INVALID;
	 end
	 else begin
	    for(pop_iter = 0; pop_iter < NUM_TRANSACTIONS; pop_iter = pop_iter + 1) begin
	       sel_slot = pop_iter;
	       if ( records[sel_slot].ready_to_go == 1'b1 ) begin
		  ret_pop_slot = sel_slot;
		  return ret_pop_slot;
	       end
	    end
	 end
      end
   endfunction

   /*
    * Find random_delay between MIN_DELAY & MAX_DELAY
    */
   function integer get_random_delay( int meta );
      int ret_random_lat;
      begin
	 // Select a random latency
	 case ( meta )
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
		`BEGIN_YELLOW_FONTCOLOR;
		$display("SIM-SV: %m =>  WriteFence must not enter latency model");
		`END_YELLOW_FONTCOLOR;
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
		$display("No Latency model available for meta type %x, using LAT_UNDEFINED", meta);
		`END_YELLOW_FONTCOLOR;
		ret_random_lat = `LAT_UNDEFINED;
	     end

	 endcase // case (meta)
	 // Return random latency value
	 return ret_random_lat;
      end
   endfunction


   /*
    * Stage I: Input FIFO
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
      .data_out   ({stg1_meta, stg1_data}),
      .data_out_v (stg1_valid),
      .alm_full   (stg1_full),
      .full       (),
      .empty      (stg1_empty),
      .count      (),
      .overflow   (stg1_overflow),
      .underflow  (stg1_underflow)
      );

   // Assert WriteFence
   always @(*) begin
      if (~stg1_empty && (stg1_meta[`TX_META_TYPERANGE]==`ASE_TX1_WRFENCE))
	assert_wrfence <= 1;
      else
	assert_wrfence <= 0;
   end

   // Filter WRFENCE
   assign stg1_pop = ~stg1_empty && ((~assert_wrfence && ~q_full) || wrfence_pop);
   assign q_push   = ~stg1_empty && (~assert_wrfence && ~q_full);

   // Request FIFO with Write fence filtered out
   ase_fifo
     #(
       .DATA_WIDTH     (HDR_WIDTH + DATA_WIDTH),
       .DEPTH_BASE2    (FIFO_DEPTH_BASE2),
       .ALMFULL_THRESH (FIFO_FULL_THRESH)
       )
   infifo_nowrfence
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (q_push),
      .data_in    ({stg1_meta, stg1_data}),
      .rd_en      (q_pop),
      .data_out   ({q_meta, q_data}),
      .data_out_v (q_valid),
      .alm_full   (q_full),
      .full       (),
      .empty      (q_empty),
      .count      (),
      .overflow   (q_overflow),
      .underflow  (q_underflow)
      );

   // Read from filtered data
   assign q_pop = ~q_empty && latbuf_push;

   /*
    * Push transaction to latency scoreboard
    */
   // STG1_FIFO -> LATBUF
   always @(posedge clk) begin
      if (rst) begin
   	 wrfence_pop <= 0;
   	 latbuf_push <= 0;
   	 push_state  <= Push_Enabled;
      end
      else begin
   	 case (push_state)
   	   // Regular push (non-Wrfence
   	   Push_Enabled:
   	     begin
   		wrfence_pop <= 0;
   		if (~q_empty && ~latbuf_full) begin
   		   push_slot_num <= find_next_push_slot();
   		   latbuf_push <= 1;
   		   push_state <= Push_Enabled;
   		end
   		else if (q_empty && assert_wrfence) begin
   		   latbuf_push <= 0;
   		   push_state <= Push_WriteFence;
   		end
   		else begin
   		   latbuf_push <= 0;
   		   push_state <= Push_Enabled;
   		end
   	     end

   	   // Write fence processing
   	   Push_WriteFence:
   	     begin
   		latbuf_push <= 0;
   		if (~latbuf_empty) begin
   		   wrfence_pop <= 0;
   		   push_state <= Push_WriteFence;
   		end
   		else begin
   		   wrfence_pop <= 1;
   		   push_state <= Push_WaitState;
   		end
   	     end

   	   // Write fence wait state
   	   Push_WaitState:
   	     begin
   		wrfence_pop <= 0;
   		latbuf_push <= 0;
   		push_state <= Push_Enabled;
   	     end

   	   default:
   	     begin
   		wrfence_pop <= 0;
   		latbuf_push <= 0;
   		push_state <= Push_Enabled;
   	     end

   	 endcase
      end
   end

   // LATBUF Full and empty, and if ready to pop
   assign latbuf_empty = (latbuf_status == {NUM_TRANSACTIONS{1'b0}}) ? 1'b1 : 1'b0;
   assign latbuf_full  = (latbuf_status == {NUM_TRANSACTIONS{1'b1}}) ? 1'b1 : 1'b0;


   /*
    * Stage II: Transaction array implementation
    */
   genvar 				     gen_i;
   generate
      // Managing each slot in transaction record
      for (gen_i = 0 ; gen_i < NUM_TRANSACTIONS ; gen_i = gen_i + 1) begin
	 logic record_valid_reg;

	 assign latbuf_status[gen_i] = records[gen_i].record_valid ;
	 assign latbuf_ready[gen_i]  = records[gen_i].ready_to_go;

	 // Record valid Assignment
	 always @(*) begin
	    if (q_valid && (push_slot_num == gen_i)) begin
	       records[gen_i].record_valid <= 1;
	    end
	    else if (latbuf_pop && (pop_slot_num == gen_i)) begin
	       records[gen_i].record_valid <= 0;
	    end
	    else begin
	       records[gen_i].record_valid <= record_valid_reg;
	    end
	 end

	 // Register process
	 always @(posedge clk)
	   record_valid_reg <= records[gen_i].record_valid;


   	 // State management
   	 always @(posedge clk) begin
	    if (rst) begin
   	       records[gen_i].record_valid <= 1'b0;
   	       records[gen_i].ready_to_go <= 1'b0;
      	       records[gen_i].state <= LatSc_Disabled;
	    end
	    else begin
   	       case (records[gen_i].state)

   		 // Disabled, not a valid transaction
   		 LatSc_Disabled:
   		   begin
   		      if ( (push_slot_num == gen_i) && latbuf_push && records[gen_i].record_valid ) begin
   			 records[gen_i].ready_to_go <= 1'b0;
   			 records[gen_i].meta  <= q_meta;
   			 records[gen_i].data  <= q_data;
   			 records[gen_i].ctr_out <= get_random_delay(q_meta[`TX_META_TYPERANGE]);
   			 records[gen_i].state <= LatSc_Countdown;
   		      end
   		      else begin
   			 records[gen_i].ready_to_go <= 1'b0;
   			 records[gen_i].ctr_out <= {COUNT_WIDTH{1'b0}};
   			 records[gen_i].state <= LatSc_Disabled;
   		      end
   		   end

   		 // Start counting down
   		 LatSc_Countdown:
   		   begin
   		      records[gen_i].ready_to_go <= 1'b0;
   		      if (records[gen_i].ctr_out == {COUNT_WIDTH{1'b0}}) begin
   			 records[gen_i].state <= LatSc_DoneReady;
   		      end
   		      else begin
   			 records[gen_i].ctr_out <= records[gen_i].ctr_out - 1;
   			 records[gen_i].state <= LatSc_Countdown;
   		      end
   		   end

   		 // Transaction is ready to be processed out
   		 LatSc_DoneReady:
   		   begin
   		      records[gen_i].ready_to_go <= 1'b1;
   		      if ((gen_i == pop_slot_num) && latbuf_pop) begin
   			 records[gen_i].state <= LatSc_PopRecord;
   		      end
   		      else begin
   			 records[gen_i].state <= LatSc_DoneReady;
   		      end
   		   end

   		 // Pop Record to STG3 FIFO when selected
   		 LatSc_PopRecord:
   		   begin
   		      if (stg3_full != 1'b1) begin
   			 records[gen_i].state <= LatSc_Disabled;
   		      end
   		      else begin
   			 records[gen_i].state <= LatSc_PopRecord;
   		      end
   		   end

   		 // la-la land
   		 default:
   		   begin
   		      records[gen_i].state <= LatSc_Disabled;
   		   end

   	       endcase
   	    end
	 end
      end // end of for
   endgenerate


   /*
    * Pop a transaction
    */
   always @(posedge clk) begin
      if (rst) begin
	 stg3_wen   <= 0;
	 latbuf_pop <= 0;
      end
      else begin
	 if ((pop_slot_num != LATBUF_POP_INVALID) && ~stg3_full && ~latbuf_empty && records[pop_slot_num].record_valid ) begin
	    latbuf_pop <= 1;
	    stg3_wen <= 1;
	 end
	 else begin
	    latbuf_pop <= 0;
	    stg3_wen <= 0;
	 end
      end
   end

   always @(posedge clk)
     pop_slot_num <= find_next_pop_slot(pop_slot_num);


   // Data into outfifo is a concat of data from reordering emulation
   assign stg3_din = {records[pop_slot_num].meta, records[pop_slot_num].data};

   /*
    * Stage III - Output FIFO
    */
   ase_fifo
     #(
       .DATA_WIDTH     (HDR_WIDTH + DATA_WIDTH),
       .DEPTH_BASE2    (FIFO_DEPTH_BASE2),
       .ALMFULL_THRESH (FIFO_FULL_THRESH)
       )
   outfifo
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (stg3_wen),
      .data_in    (stg3_din),
      .rd_en      (read_en),
      .data_out   ({meta_out, data_out}),
      .data_out_v (valid_out),
      .alm_full   (stg3_full),
      .full       (),
      .empty      (empty),
      .count      (),
      .overflow   (stg3_overflow),
      .underflow  (stg3_underflow)
      );


endmodule // latency_scoreboard
