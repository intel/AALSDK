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
    parameter int FIFO_DEPTH_BASE2 = 3,
    parameter int VISIBLE_DEPTH_BASE2 = 7
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
    output logic 		  underflow,
    output logic [31:0] 	  count
    );

   parameter int 		  LATBUF_SLOT_INVALID = 255;
   parameter int 		  TID_WIDTH = 32;
   parameter int 		  FIFO_WIDTH = TID_WIDTH + HDR_WIDTH + DATA_WIDTH;

   // Tracking ID
   logic [TID_WIDTH-1:0] 	  tid_counter;
   logic [TID_WIDTH-1:0] 	  tid_in;
   logic [TID_WIDTH-1:0] 	  tid_out;

   // Tagging process
   always @(posedge clk) begin
      if (rst)
	tid_in <= {TID_WIDTH{1'b0}};
      else if (write_en)
	tid_in <= tid_in + 1;
   end


   // Setup slot usage order
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
   logic 			  latbuf_almfull;
   logic 			  latbuf_full_reg;
   logic 			  latbuf_push;
   logic 			  latbuf_pop;
   int 				  latbuf_count;
   logic 			  latbuf_anyready;

   // always @(posedge clk)
   //   latbuf_pop_reg <= latbuf_pop;

   // Stage 1 signals
   logic [HDR_WIDTH-1:0] 	  stg1_meta;
   logic [DATA_WIDTH-1:0] 	  stg1_data;
   logic [TID_WIDTH-1:0] 	  stg1_tid;
   logic 			  stg1_valid;
   logic 			  stg1_empty;
   logic 			  stg1_full;
   logic 			  stg1_pop;
   logic [FIFO_DEPTH_BASE2:0] 	  stg1_count;

   // Stage 1 (write fence filtered)
   logic [HDR_WIDTH-1:0] 	  q_meta;
   logic [DATA_WIDTH-1:0] 	  q_data;
   logic [TID_WIDTH-1:0] 	  q_tid;
   logic 			  q_valid;
   logic 			  q_full;
   logic 			  q_empty;
   logic 			  q_pop;
   logic 			  q_push;
   logic [FIFO_DEPTH_BASE2:0] 	  q_count;

   logic [HDR_WIDTH-1:0] 	  stg2_meta;
   logic [DATA_WIDTH-1:0] 	  stg2_data;
   logic [TID_WIDTH-1:0] 	  stg2_tid;
   logic 			  stg2_valid;

   logic [FIFO_WIDTH-1:0] 	  latbuf_pop_dout;
   logic 			  latbuf_pop_valid;
      
   // Push/Pop iterator
   int 				  push_ptr;
   int 				  pop_ptr;
   int 				  push_slot_num;
   int 				  pop_slot_num;

   // stage 3 signals
   logic [FIFO_WIDTH-1:0] 	  stg3_din;
   logic 			  stg3_wen;
   logic 			  stg3_full;
   logic 			  stg3_empty;
   logic [FIFO_DEPTH_BASE2:0] 	  stg3_count;

   int 				  jj;
   int 				  ii;

   logic [FIFO_WIDTH-1:0] 	  reg_stg3_din;
   logic 			  reg_stg3_wen;
   logic 			  reg_latbuf_pop;


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

   // FULL
   assign full = stg1_full;


   /*
    * States & structures
    */
   // Enumerate states
   typedef enum {LatSc_Disabled, LatSc_Enabled, LatSc_Countdown, LatSc_DoneReady, LatSc_PopRecord} latsc_fsmState;

   typedef enum {NoWrfencePassThru, WrFenceWaiting, WaitState} WriteFence_checker;
   WriteFence_checker wrf_state;
  

   // Transaction storage
   typedef struct packed
		  {
		     logic [HDR_WIDTH-1:0]   meta;
		     logic [DATA_WIDTH-1:0]  data;
		     logic [TID_WIDTH-1:0]   tid;
		     logic [COUNT_WIDTH-1:0] ctr_out;
		     logic 		     ready_to_go;
		     logic 		     record_valid;
		     latsc_fsmState          state;
		     } transact_t;

   // Array of stored transactions
   transact_t records[NUM_TRANSACTIONS] ;

   /*
    * Find a next free slot
    */
   function integer find_next_push_slot();
      int 				     find_iter;
      int 				     ret_free_slot;
      begin
   	 for(find_iter = push_ptr; find_iter < push_ptr + NUM_TRANSACTIONS; find_iter = find_iter + 1) begin
	    ret_free_slot = slot_lookup[find_iter % NUM_TRANSACTIONS];
   	    if ((records[ret_free_slot].record_valid == 0) && (records[ret_free_slot].state == LatSc_Disabled)) begin
   	       push_ptr = find_iter;
   	       return ret_free_slot;
   	    end
   	 end
	 return LATBUF_SLOT_INVALID;
      end
   endfunction

   /*
    * Find a transaction to release to output stage
    */
   function integer find_next_pop_slot();
      int ret_pop_slot;
      int start_ptr;
      int pop_iter;
      int sel_slot;
      int prev_pop_slot_num;
      begin
	 for(pop_iter = pop_ptr; pop_iter < pop_ptr + NUM_TRANSACTIONS ; pop_iter = pop_iter + 1) begin
	    sel_slot = pop_iter % NUM_TRANSACTIONS;
	    if ( (records[sel_slot].ready_to_go == 1) && (records[sel_slot].state == LatSc_DoneReady) ) begin
	       pop_ptr = pop_iter; 
	       return sel_slot;
	    end
	 end
	 return LATBUF_SLOT_INVALID;
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
`ifdef ASE_DEBUG
		`BEGIN_YELLOW_FONTCOLOR;
		$display("SIM-SV: %m =>  WriteFence must not enter latency model");
		`END_YELLOW_FONTCOLOR;
`endif
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
`ifdef ASE_DEBUG
		`BEGIN_YELLOW_FONTCOLOR;
		$display("SIM-SV: %m =>");
		$display("No Latency model available for meta type %x, using LAT_UNDEFINED", meta);
		`END_YELLOW_FONTCOLOR;
`endif
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
       .DATA_WIDTH     (FIFO_WIDTH),
       .DEPTH_BASE2    (FIFO_DEPTH_BASE2),
       .ALMFULL_THRESH (FIFO_FULL_THRESH)
       )
   infifo
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (write_en),
      .data_in    ({tid_in, meta_in, data_in}),
      .rd_en      (stg1_pop),
      .data_out   ({stg1_tid, stg1_meta, stg1_data}),
      .data_out_v (stg1_valid),
      .alm_full   (stg1_full),
      .full       (),
      .empty      (stg1_empty),
      .count      (stg1_count),
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
   assign stg1_pop = ~stg1_empty && ((~assert_wrfence && ~q_full) || wrfence_pop );
   assign q_push   = ~stg1_empty && (~assert_wrfence && ~q_full);

   // WriteFence passthru/trap FSM
   always @(posedge clk) begin
      if (rst) begin
	 wrf_state <= NoWrfencePassThru;
	 wrfence_pop <= 0;	 
      end
      begin
	 case (wrf_state)
	   NoWrfencePassThru:
	     begin
		wrfence_pop <= 0;		
		if (assert_wrfence) begin
		   wrf_state <= WrFenceWaiting;		   
		end
		else begin
		   wrf_state <= NoWrfencePassThru;		   
		end
	     end

	   WrFenceWaiting:
	     begin
		if (q_empty && stg3_empty && latbuf_empty) begin
		   wrfence_pop <= 1;
		   wrf_state <= WaitState;		     
		end
		else begin
		   wrfence_pop <= 0;	
		   wrf_state <= WrFenceWaiting;		     
		end
	     end

	   WaitState:
	     begin
		wrfence_pop <= 0;		  
		wrf_state <= NoWrfencePassThru;		  
	     end

	   default:
	     begin
		wrfence_pop <= 0;		  
		wrf_state <= NoWrfencePassThru;		  
	     end
	 endcase
      end
   end
   
   
   // Request FIFO with Write fence filtered out
   ase_fifo
     #(
       .DATA_WIDTH     (FIFO_WIDTH),
       .DEPTH_BASE2    (FIFO_DEPTH_BASE2),
       .ALMFULL_THRESH (FIFO_FULL_THRESH)
       )
   infifo_nowrfence
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (q_push),
      .data_in    ({stg1_tid, stg1_meta, stg1_data}),
      .rd_en      (q_pop),
      .data_out   ({q_tid, q_meta, q_data}),
      .data_out_v (q_valid),
      .alm_full   (q_full),
      .full       (),
      .empty      (q_empty),
      .count      (q_count),
      .overflow   (q_overflow),
      .underflow  (q_underflow)
      );

   // Read from filtered data
   assign q_pop = ~q_empty && ~latbuf_almfull && (push_slot_num != LATBUF_SLOT_INVALID);
   always @(posedge clk)
     latbuf_push <= q_pop;

   // Register stg2 output
   always @(posedge clk) begin
      stg2_valid <= q_valid;
      stg2_meta <= q_meta;
      stg2_data <= q_data;
      stg2_tid <= q_tid;
   end


   /*
    * Calculate PUSH and POP slot
    */
   always @(posedge clk) begin
      push_slot_num <= find_next_push_slot();
      pop_slot_num <= find_next_pop_slot();
   end


   // Latency scoreboard counter
   assign latbuf_count = $countones(latbuf_status);

   // Assign latbuf_count & other status signals
   assign latbuf_empty = (latbuf_count == 0) ? 1 : 0;
   assign latbuf_full  = (latbuf_count == NUM_TRANSACTIONS) ? 1 : 0;
   assign latbuf_almfull = (latbuf_count >= (NUM_TRANSACTIONS-1)) ? 1 : 0;
   assign latbuf_anyready = |latbuf_ready;


   /*
    * Stage II: Transaction array implementation
    */
   genvar 				     gen_i;
   generate
      // Managing each slot in transaction record
      for (gen_i = 0 ; gen_i < NUM_TRANSACTIONS ; gen_i = gen_i + 1) begin
	 logic record_valid_reg;
	 logic ready_to_go_reg;
	 
	 assign latbuf_status[gen_i] = records[gen_i].record_valid ;
	 assign latbuf_ready[gen_i]  = records[gen_i].ready_to_go;

	 // Record valid Assignment
	 always @(*) begin
	    if (rst) begin
	       records[gen_i].record_valid <= 0;
	    end
	    else begin
	       if (stg2_valid && (push_slot_num == gen_i) && (push_slot_num != LATBUF_SLOT_INVALID)) begin
	 	  records[gen_i].record_valid <= 1;
	       end
	       else if (latbuf_pop && (pop_slot_num == gen_i)) begin
	 	  records[gen_i].record_valid <= 0;
	       end
	       else begin
	 	  records[gen_i].record_valid <= record_valid_reg;
	       end
	    end
	 end

	 // Ready to go assignment
	 always @(*) begin
	    if (rst) begin
	       records[gen_i].ready_to_go <= 0;	       
	    end
	    else begin
	       if (records[gen_i].record_valid && (records[gen_i].state == LatSc_DoneReady)) begin
	 	  records[gen_i].ready_to_go <= 1;	       		  
	       end
	       else if ((pop_slot_num == gen_i) && latbuf_pop) begin
	 	  records[gen_i].ready_to_go <= 0;	       
	       end
	       else begin
	 	  records[gen_i].ready_to_go <= ready_to_go_reg;	       
	       end
	    end
	 end 
	 
	 // Register process
	 always @(posedge clk) begin
	    record_valid_reg <= records[gen_i].record_valid;
	    ready_to_go_reg  <= records[gen_i].ready_to_go;	    
	 end

   	 // State management
   	 always @(posedge clk) begin
	    if (rst) begin
	       //   	       records[gen_i].ready_to_go <= 1'b0;
      	       records[gen_i].state <= LatSc_Disabled;
	    end
	    else begin
   	       case (records[gen_i].state)

   		 // Disabled, not a valid transaction
   		 LatSc_Disabled:
   		   begin
   		      if ( (push_slot_num == gen_i) && latbuf_push ) begin
`ifdef ASE_DEBUG
			 `BEGIN_YELLOW_FONTCOLOR;
			 $display("INFO (%d) => latbuf_push tid=%x", $time, stg2_tid);
			 `END_YELLOW_FONTCOLOR;
`endif
   			 // records[gen_i].ready_to_go <= 1'b0;
   			 records[gen_i].meta  <= stg2_meta;
   			 records[gen_i].data  <= stg2_data;
			 records[gen_i].tid   <= stg2_tid;
   			 records[gen_i].ctr_out <= get_random_delay(stg2_meta[`TX_META_TYPERANGE]);
   			 records[gen_i].state <= LatSc_Countdown;
   		      end
   		      else begin
   			 // records[gen_i].ready_to_go <= 1'b0;
   			 records[gen_i].ctr_out <= {COUNT_WIDTH{1'b0}};
   			 records[gen_i].state <= LatSc_Disabled;
   		      end
   		   end

   		 // Start counting down
   		 LatSc_Countdown:
   		   begin
   		      // records[gen_i].ready_to_go <= 1'b0;
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
   		      if ((gen_i == pop_slot_num) && latbuf_pop) begin
   			 // records[gen_i].ready_to_go <= 0;
   			 records[gen_i].state <= LatSc_PopRecord;
   		      end
   		      else begin
   			 // records[gen_i].ready_to_go <= 1;
   			 records[gen_i].state <= LatSc_DoneReady;
   		      end
   		   end

   		 // Pop Record to STG3 FIFO when selected
   		 LatSc_PopRecord:
   		   begin
   		      // records[gen_i].ready_to_go <= 0;
   		      if (records[gen_i].record_valid == 0) begin
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
   // Check for valid transactions
   always @(posedge clk) begin
      if (pop_slot_num != LATBUF_SLOT_INVALID) begin
	 latbuf_pop_dout <= {records[pop_slot_num].tid, records[pop_slot_num].meta, records[pop_slot_num].data};
	 latbuf_pop_valid <= records[pop_slot_num].ready_to_go;	 
      end
      else begin
	 latbuf_pop_dout <= {FIFO_WIDTH{1'b0}};
	 latbuf_pop_valid <= 0;	 
      end
   end

   assign latbuf_pop = ~stg3_full && (pop_slot_num != LATBUF_SLOT_INVALID) && latbuf_pop_valid;   
   
   // POP process   
   always @(posedge clk) begin
      if (rst) begin
	 stg3_wen <= 0;
	 stg3_din <= {FIFO_WIDTH{1'b0}};	 
      end
      else if (latbuf_pop_valid) begin
	 stg3_wen <= latbuf_pop_valid;
	 stg3_din <= latbuf_pop_dout;	 
      end
      else begin
	 stg3_wen <= 0;
      end
   end
   
   // always @(posedge clk) begin
   //    if (rst) begin
   // 	 stg3_wen <= 0;
   // 	 stg3_din <= {FIFO_WIDTH{1'b0}};
   // 	 latbuf_pop <= 0;
   //    end
   //    else begin
   // 	 if ((pop_slot_num != LATBUF_SLOT_INVALID) && ~stg3_full && ~latbuf_empty) begin
   // 	    stg3_wen <= 1;
   // 	    stg3_din <= {records[pop_slot_num].tid, records[pop_slot_num].meta, records[pop_slot_num].data};
   // 	    latbuf_pop <= 1;
   // 	 end
   // 	 else begin
   // 	    stg3_wen <= 0;
   // 	    stg3_din <= {FIFO_WIDTH{1'b0}};
   // 	    latbuf_pop <= 0;
   // 	 end
   //    end
   // end


   /*
    * Stage III - Output FIFO
    */
   ase_fifo
     #(
       .DATA_WIDTH     (FIFO_WIDTH),
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
      .data_out   ({tid_out, meta_out, data_out}),
      .data_out_v (valid_out),
      .alm_full   (stg3_full),
      .full       (),
      .empty      (stg3_empty),
      .count      (stg3_count),
      .overflow   (stg3_overflow),
      .underflow  (stg3_underflow)
      );

   assign empty = stg3_empty;

   // Count
   assign count = stg1_count + q_count + latbuf_count + stg3_count;


   /*
    * Transaction IN-OUT checker
    * Sniffs dropped transactions
    */
`ifdef ASE_DEBUG
   int checker_mem[*];
   // Add/remove to checker as things come and go
   always @(posedge clk) begin
      if (write_en) begin
	 checker_mem[tid_in] = tid_in;
      end
      else if (valid_out) begin
	 if (checker_mem.exists(tid_out)) begin
	    checker_mem.delete(tid_out);
	 end
	 else begin
	    `BEGIN_RED_FONTCOLOR;
	    $display("ERROR (%d): tid_out = %x was not found in checker memory !!", $time, tid_out);
	    `END_RED_FONTCOLOR;
	 end
      end
      else if (wrfence_pop) begin
	 if (checker_mem.exists(stg1_tid)) begin
	    checker_mem.delete(stg1_tid);
	 end
	 else begin
	    `BEGIN_RED_FONTCOLOR;
	    $display("ERROR (%d): tid_out = %x was not found in checker memory !!", $time, stg1_tid);
	    `END_RED_FONTCOLOR;
	 end
      end
   end

   // Monitor-print process
   always @(posedge clk) begin
      if (q_pop && latbuf_full) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("ERROR (%d) => Latbuf push and full were HIGH, tid = %d", $time, q_tid);
	 `END_RED_FONTCOLOR;
      end
      if (latbuf_pop && latbuf_empty) begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("ERROR (%d) => Latbuf pop and empty were HIGH, tid = %d", $time, q_tid);
	 `END_RED_FONTCOLOR;
      end
      if (q_pop) begin
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("INFO (%d) => QPOP q_tid = %d", $time, q_tid);
	 `END_YELLOW_FONTCOLOR;
      end
   end

`endif

endmodule // latency_scoreboard
