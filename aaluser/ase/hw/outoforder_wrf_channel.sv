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
 * *********************************************************************************
 * SR-5.0.0-alpha onwardsa implementation
 * ---------------------------------------------------------------------------------
 *
 *                     TBD cachesim link
 *                            ||
 *                       /---------------------------------\
 *             |-->vl0-->|          |           |          |
 *             |         | assign   |   wait    | multi-CL |
 * -->infifo-->|-->vh0-->| delayed  | stattions | breakout |-->outfifo-->
 *             |         | action   |           |          |
 *             |-->vh1-->|          |           |          |
 *                       \---------------------------------/
 *
 * - Input FIFO stages requests and asserts AlmostFull signal             |
 *   -- Feeds 2 high-lat and 1 low-lat lanes                              |
 * - If VA                                                                | Request
 *   -- Round robin between VH and VL lanes                               | Order
 *   -- Response channels (VC_USED) is assigned here                      | Maintain
 * - Assignment of waits is done and pushed to wait stations              |
 * - When ready to pop from wait stations                                 |---------
 *   -- If multi-CL is observed, request is broken out to multiple-single |
 *   -- If single line is observed, it is passed through                  |
 *   -- Unit generates RxHdr output to send response back to AFU          | Request
 * - If fence is observed with:                                           | order
 *   -- VA      : All channels fenced                                     | changed
 *   -- VL0/VHx : Requested channel is fenced                             |
 *
 * *********************************************************************************
 * SR 4.1.x - SR 5.0.0-prealpha implementation
 * ---------------------------------------------------------------------------------
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
 * - NUM_WAIT_STATIONS : Number of transactions in latency buffer
 * - FIFO_FULL_THRESH : FIFO full threshold
 * - FIFO_DEPTH_BASE2 : FIFO depth radix
 *
 */

import ase_pkg::*;

`include "platform.vh"

module outoforder_wrf_channel
  #(
    parameter string DEBUG_LOGNAME  = "channel.log",
    parameter int NUM_WAIT_STATIONS = 16,
    parameter int COUNT_WIDTH = 8,
    parameter int VISIBLE_DEPTH_BASE2 = 7,
    parameter int VISIBLE_FULL_THRESH = 110
    )
   (
    input logic 		       clk,
    input logic 		       rst,
    // Transaction in
    input 			       TxHdr_t hdr_in,
    input logic [CCIP_DATA_WIDTH-1:0]  data_in,
    input logic 		       write_en,
    // Transaction out
    output 			       TxHdr_t txhdr_out,
    output 			       RxHdr_t rxhdr_out,
    output logic [CCIP_DATA_WIDTH-1:0] data_out,
    output logic 		       valid_out,
    input logic 		       read_en,
    // Status signals
    output logic 		       empty,
    output logic 		       full
    );

`ifdef ASE_DEBUG
   int 				       log_fd;
   initial begin
      log_fd = $fopen( DEBUG_LOGNAME, "w");
      $fwrite(log_fd, "Logger for %m transactions\n");
   end
`endif

   localparam TID_WIDTH           = 32;
   localparam FIFO_WIDTH          = TID_WIDTH + CCIP_TX_HDR_WIDTH + CCIP_DATA_WIDTH;
   localparam OUTFIFO_WIDTH       = TID_WIDTH + CCIP_RX_HDR_WIDTH + CCIP_TX_HDR_WIDTH + CCIP_DATA_WIDTH;

   localparam LATBUF_SLOT_INVALID = 255;

   // Visible depth
   localparam VISIBLE_DEPTH = 2**VISIBLE_DEPTH_BASE2;

   // Internal FIFOs are invisible FIFOs inside channel
   localparam INTERNAL_FIFO_DEPTH_RADIX    = 4;
   localparam INTERNAL_FIFO_DEPTH          = 2**INTERNAL_FIFO_DEPTH_RADIX;
   localparam INTERNAL_FIFO_ALMFULL_THRESH = INTERNAL_FIFO_DEPTH - 4;

   // Internal signals
   logic [TID_WIDTH-1:0] 	  tid_in;
   logic [TID_WIDTH-1:0] 	  tid_out;

   // Infifo
   logic [FIFO_WIDTH-1:0] 	  infifo[$:VISIBLE_DEPTH-1];

   // Lanes
   logic [FIFO_WIDTH-1:0] 	  vl0_array[$:INTERNAL_FIFO_DEPTH-1];
   logic [FIFO_WIDTH-1:0] 	  vh0_array[$:INTERNAL_FIFO_DEPTH-1];
   logic [FIFO_WIDTH-1:0] 	  vh1_array[$:INTERNAL_FIFO_DEPTH-1];
   logic [FIFO_WIDTH-1:0] 	  vl0_nowrf_array[$:INTERNAL_FIFO_DEPTH-1];
   logic [FIFO_WIDTH-1:0] 	  vh0_nowrf_array[$:INTERNAL_FIFO_DEPTH-1];
   logic [FIFO_WIDTH-1:0] 	  vh1_nowrf_array[$:INTERNAL_FIFO_DEPTH-1];

   // Outfifo
   logic [OUTFIFO_WIDTH-1:0] 	  outfifo[$:VISIBLE_DEPTH-1];

   // FIFO counts
   int 				  infifo_cnt;
   int 				  vl0_array_cnt;
   int 				  vh0_array_cnt;
   int 				  vh1_array_cnt;
   int 				  outfifo_cnt;

   logic 			  vl0_array_full;
   logic 			  vh0_array_full;
   logic 			  vh1_array_full;

   logic 			  vl0_array_empty;
   logic 			  vh0_array_empty;
   logic 			  vh1_array_empty;

   logic 			  infifo_empty;

   logic 			  outfifo_empty;
   logic 			  outfifo_almfull;

   logic 			  outfifo_write_en;
   logic 			  outfifo_read_en;
   logic [2:0] 			  vc_push;


   logic 			  all_lanes_full;
   assign all_lanes_full = vl0_array_full & vh0_array_full & vh1_array_full;

   // Tracking ID generator
   always @(posedge clk) begin : tid_proc
      if (rst)
	tid_in	<= {TID_WIDTH{1'b0}};
      else if (write_en)
	tid_in	<= tid_in + 1;
   end

   // Counts/fill level
   always @(posedge clk) begin : cnt_proc
      infifo_cnt    <= infifo.size();
      vl0_array_cnt <= vl0_array.size();
      vh0_array_cnt <= vh0_array.size();
      vh1_array_cnt <= vh1_array.size();
      outfifo_cnt   <= outfifo.size();
   end


   assign vl0_array_full  = (vl0_array_cnt > INTERNAL_FIFO_ALMFULL_THRESH) ? 1 : 0;
   assign vh0_array_full  = (vh0_array_cnt > INTERNAL_FIFO_ALMFULL_THRESH) ? 1 : 0;
   assign vh1_array_full  = (vh1_array_cnt > INTERNAL_FIFO_ALMFULL_THRESH) ? 1 : 0;

   assign vl0_array_empty = (vl0_array_cnt == 0) ? 1 : 0;
   assign vh0_array_empty = (vh0_array_cnt == 0) ? 1 : 0;
   assign vh1_array_empty = (vh1_array_cnt == 0) ? 1 : 0;

   // Full signal
   always @(posedge clk) begin : full_proc
      if (rst) begin
	 full <= 1;
      end
      else if (infifo_cnt > VISIBLE_FULL_THRESH  ) begin
	 full <= 1;
      end
      else begin
	 full <= 0;
      end
   end


   //////////////////////////////////////////////////////////////
   // Scoreboard logic
   //////////////////////////////////////////////////////////////
   // Enumerate states
   typedef enum {LatSc_Disabled,
		 LatSc_Countdown,
		 LatSc_DoneReady,
		 LatSc_RecordPopped} latsc_fsmState;

   // Transaction storage
   typedef struct packed
		  {
		     logic [CCIP_TX_HDR_WIDTH-1:0]   hdr;           // in
		     logic [CCIP_DATA_WIDTH-1:0]  data;          // in
		     logic [TID_WIDTH-1:0]   tid;           // in
		     logic [COUNT_WIDTH-1:0] ctr_out;       // out
		     logic 		     record_valid;  // out
		     logic 		     record_ready;  // out
		     logic                   record_push;   // in
		     logic 		     record_pop;    // in
		     latsc_fsmState          state;         // out
		     } transact_t;

   // Array of stored transactions
   transact_t records[NUM_WAIT_STATIONS] ;


   /*
    * Slot setup
    */
   // Setup slot usage order
   // int 				  slot_lookup[NUM_WAIT_STATIONS];

   // Initialize slot usage order
   // initial begin : shuffle_init
   //    int i;
   //    // Serial order
   //    for(i = 0 ; i < NUM_WAIT_STATIONS ; i = i + 1) begin
   //    	 slot_lookup[i] = i;
   //    end
   //    // Shuffle data using internal function
   //    // slot_lookup.shuffle(); // *FIXME*
   // end


   // Infifo, request staging
   always @(posedge clk) begin : infifo_push
      if (write_en) begin
	 `ifdef ASE_DEBUG
	 $fwrite(log_fd, "%d | WRITE : hdr=%x assigned tid = %x\n", $time, hdr_in, tid_in);
	 `endif
	 infifo.push_back({ tid_in, data_in, CCIP_TX_HDR_WIDTH'(hdr_in) });
      end
   end

   
   // Pop infifo, arbitrate between lanes
   logic [CCIP_DATA_WIDTH-1:0] infifo_data_out;
   logic [TID_WIDTH-1:0]       infifo_tid_out;
   logic [CCIP_TX_HDR_WIDTH-1:0] infifo_hdr_out_vec;
   TxHdr_t                infifo_hdr_out;
   logic 			  infifo_vld;

   // logic [CCIP_DATA_WIDTH-1:0] infifo_data_out_q;
   // logic [TID_WIDTH-1:0]  infifo_tid_out_q;
   // logic [CCIP_TX_HDR_WIDTH-1:0]  infifo_hdr_out_vec_q;
   // logic 		  infifo_vld_q;

   ccip_vc_t 		  vc_arb;

   // Select VC
   // function logic [CCIP_TX_HDR_WIDTH-1:0] select_vc(int init, input [CCIP_TX_HDR_WIDTH-1:0] hdr);
   function void select_vc(int init, ref TxHdr_t hdr);
      begin
	 if (init) begin
	    vc_arb = ccip_vc_t'(VC_VL0);
	 end
	 else begin
	    if (hdr.vc == VC_VA) begin
	       case ({vl0_array_full, vh0_array_full, vh1_array_full})
		 3'b000:
		   begin
		      case (vc_arb)
			VC_VA  : hdr.vc = VC_VL0;
			VC_VL0 : hdr.vc = VC_VH0;
			VC_VH0 : hdr.vc = VC_VH1;
			VC_VH1 : hdr.vc = VC_VL0;
		      endcase
		   end
		 3'b001: hdr.vc = VC_VL0;
		 3'b010: hdr.vc = VC_VH1;
		 3'b011: hdr.vc = VC_VL0;
		 3'b100: hdr.vc = VC_VH0;
		 3'b101: hdr.vc = VC_VH0;
		 3'b110: hdr.vc = VC_VH1;
	       endcase
	       vc_arb = ccip_vc_t'(hdr.vc);
	    end // if (hdr.vc == VC_VA)
	 end
	 // return hdr;
      end
   endfunction

   // Infifo empty
   assign infifo_empty = (infifo_cnt == 0) ? 1 : 0;

   // INFIFO->VC_sel
   function void infifo_to_vc_push ();
      begin
	 if (~all_lanes_full & ~infifo_empty) begin
	    {infifo_tid_out, infifo_data_out, infifo_hdr_out_vec} = infifo.pop_front();
	    infifo_hdr_out = TxHdr_t'(infifo_hdr_out_vec);	    
	    // infifo_hdr_out_vec = select_vc (0, infifo_hdr_out_vec);
	    select_vc (0, infifo_hdr_out);	    
	 `ifdef ASE_DEBUG
	    if (infifo_hdr_out.vc == VC_VA) begin
	       $fwrite(log_fd, "%d | select_vc : tid=%x picked VC_VA, this must not happen !!\n", $time, infifo_tid_out);
	    end
	 `endif
	    if (infifo_hdr_out.reqtype == CCIP_WRFENCE) begin
	       // Fence activatd
	       vl0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	       vh0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	       vh1_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	    end
	    else begin
	       // No fence
	       case (infifo_hdr_out.vc)
		 VC_VL0:
		   begin
		      vc_push = 3'b100;
		      vl0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VL0\n", $time, infifo_tid_out);
	 `endif
		   end
		 
		 VC_VH0:
		   begin
		      vc_push = 3'b010;
		      vh0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH0\n", $time, infifo_tid_out);
	 `endif
		   end

		 VC_VH1:
		   begin
		      vc_push = 3'b001;
		      vh1_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH1\n", $time, infifo_tid_out);
	 `endif
		   end
	       endcase	       
	    end // else: !if(infifo_hdr_out.reqtype == CCIP_WRFENCE)
	 end // if (~all_lanes_full & ~infifo_empty)	 
      end
   endfunction

/*
   // Read from infifo
   always @(posedge clk) begin : infifo_pop_proc
      if (~all_lanes_full & ~infifo_empty) begin
	 {infifo_tid_out_q, infifo_data_out_q, infifo_hdr_out_vec_q} <= infifo.pop_front();
	 infifo_vld_q <= 1;
      end
      else begin
	 infifo_vld_q <= 0;
      end
   end

   always @(*) begin : infifo_out_comb
      if (rst) begin
	 infifo_hdr_out_vec <= select_vc (1, infifo_hdr_out_vec_q);
	 infifo_vld         <= 0;
      end
      else begin
	 infifo_tid_out     <= infifo_tid_out_q;
	 infifo_data_out    <= infifo_data_out_q;
	 infifo_hdr_out_vec <= select_vc (0, infifo_hdr_out_vec_q);
	 infifo_vld         <= infifo_vld_q & ~infifo_empty;
      end
   end

   assign infifo_hdr_out = TxHdr_t'(infifo_hdr_out_vec);
*/
   // always @(posedge clk) begin : vc_push_proc
   //    if (rst) begin
   // 	 vc_push <= 3'b000;
   //    end
   //    else if (infifo_vld && (infifo_hdr_out.reqtype == CCIP_WRFENCE)) begin
   // 	 vc_push <= 3'b111;
   // 	 vl0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 vh0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 vh1_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 `ifdef ASE_DEBUG
   // 	 $fwrite(log_fd, "%d | WRFENCE VA : tid=%x \n", $time, infifo_tid_out);
   // 	 `endif
   //    end
   //    else if (infifo_vld && (infifo_hdr_out.reqtype != CCIP_WRFENCE)) begin
   // 	 case (infifo_hdr_out.vc)
   // 	   VC_VL0:
   // 	     begin
   // 		vc_push <= 3'b100;
   // 		vl0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 `ifdef ASE_DEBUG
   // 		$fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VL0\n", $time, infifo_tid_out);
   // 	 `endif
   // 	     end

   // 	   VC_VH0:
   // 	     begin
   // 		vc_push <= 3'b010;
   // 		vh0_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 `ifdef ASE_DEBUG
   // 		$fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH0\n", $time, infifo_tid_out);
   // 	 `endif
   // 	     end

   // 	   VC_VH1:
   // 	     begin
   // 		vc_push <= 3'b001;
   // 		vh1_array.push_back({infifo_tid_out, infifo_data_out, infifo_hdr_out_vec});
   // 	 `ifdef ASE_DEBUG
   // 		$fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH1\n", $time, infifo_tid_out);
   // 	 `endif
   // 	     end
   // 	 endcase
   //    end
   //    else begin
   // 	 vc_push <= 3'b000;
   //    end
   // end

   always @(posedge clk) begin
      if (rst) begin
	 vc_push <= 3'b000;	 
	 select_vc (1, infifo_hdr_out);	 
      end
      else if (~all_lanes_full & ~infifo_empty) begin
	 infifo_to_vc_push();	 
      end
      else begin
	 vc_push <= 3'b000;	 
      end
   end

   
   // Lane pop and latency scoreboard push
   logic vl0_wrfence_flag;
   logic vh0_wrfence_flag;
   logic vh1_wrfence_flag;

   int 	 latbuf_push_ptr;
   int 	 latbuf_pop_ptr;

   logic [0:NUM_WAIT_STATIONS-1] latbuf_used;
   logic [0:NUM_WAIT_STATIONS-1] latbuf_ready;
   int 				 latbuf_cnt;
   logic 			 latbuf_full;
   logic 			 latbuf_almfull;
   logic 			 latbuf_empty;


   // Count used latbuf
   function int update_latbuf_cnt();
      int 			 sum;
      int 			 jj;
      begin
	 sum = 0;
	 for (jj =0 ; jj < NUM_WAIT_STATIONS; jj = jj + 1) begin
	    sum = sum + latbuf_used[jj];
	 end
	 return sum;
      end
   endfunction


   // Count
   always @(posedge clk) begin : latbuf_cnt_proc
      latbuf_cnt <= update_latbuf_cnt();
   end


   assign latbuf_empty   = (latbuf_cnt == 0) ? 1 : 0;
   assign latbuf_full    = (latbuf_cnt == NUM_WAIT_STATIONS) ? 1 : 0;
   assign latbuf_almfull = (latbuf_cnt >= (NUM_WAIT_STATIONS-3)) ? 1 : 0;

   // push_ptr selector
   function integer find_next_push_slot();
      int 				     find_iter;
      int 				     ret_free_slot;
      begin
   	 for(find_iter = latbuf_push_ptr;
	     find_iter < latbuf_push_ptr + NUM_WAIT_STATIONS;
	     find_iter = find_iter + 1) begin
	    // ret_free_slot = slot_lookup[find_iter % NUM_WAIT_STATIONS];
	    ret_free_slot = find_iter % NUM_WAIT_STATIONS;
   	    if (~latbuf_used[ret_free_slot]) begin
   	       return ret_free_slot;
   	    end
   	 end
	 return LATBUF_SLOT_INVALID;
      end
   endfunction


   //////////////////////////////////////////////////////////////////////
   // Latbuf assignment process
   //////////////////////////////////////////////////////////////////////
   // Read and update record in latency scoreboard
   function void read_latbuf_push (ref logic [FIFO_WIDTH-1:0] array[$],
				   ref logic wrfence_flag
				   );
      logic [CCIP_TX_HDR_WIDTH-1:0] 		     array_hdr;
      logic [CCIP_DATA_WIDTH-1:0] 		     array_data;
      logic [TID_WIDTH-1:0] 		     array_tid;
      TxHdr_t                                hdr;
      int 				     ptr;
      begin
	 ptr = find_next_push_slot();
	 latbuf_push_ptr = ptr;
	 if (ptr != LATBUF_SLOT_INVALID) begin
	    {array_tid, array_data, array_hdr} = array.pop_front();
	    hdr = TxHdr_t'(array_hdr);
	    if (hdr.reqtype == CCIP_WRFENCE) begin
	       wrfence_flag = 1;
	    end
	    else begin
	       records[ptr].hdr          = array_hdr;
	       records[ptr].data         = array_data;
	       records[ptr].tid          = array_tid;
	       records[ptr].record_push  = 1;
	       records[ptr].record_valid = 1;
	       latbuf_used[ptr]          = 1;
	    end
	 `ifdef ASE_DEBUG
	 $fwrite(log_fd, "%d | latbuf_push : tid=%x sent to record[%d]\n", $time, infifo_tid_out, ptr);
	 `endif
	 end
      end
   endfunction

  
   // //////////////////////////////////////////////////////////////////////////////
   // States
   typedef enum {Select_VL0, Select_VH0, Select_VH1} lssel_state;
   lssel_state vc_pop;

   always @(posedge clk) begin : latbuf_push_proc
      if (rst) begin
   	 vc_pop <= Select_VL0;
	 latbuf_used <= {NUM_WAIT_STATIONS{1'b0}};
	 vl0_wrfence_flag <= 0;
	 vh0_wrfence_flag <= 0;
	 vh1_wrfence_flag <= 0;
      end
      else begin
	 // If input arrays are available
   	 case (vc_pop)
   	   Select_VL0:
   	     begin
		if (~vl0_wrfence_flag && ~vl0_array_empty && ~latbuf_almfull) begin
		   read_latbuf_push(vl0_array, vl0_wrfence_flag);
		end
   		vc_pop <= Select_VH0;
   	     end

   	   Select_VH0:
   	     begin
		if (~vh0_wrfence_flag && ~vh0_array_empty && ~latbuf_almfull) begin
		   read_latbuf_push(vh0_array, vh0_wrfence_flag);
		end
   		vc_pop <= Select_VH1;
   	     end

   	   Select_VH1:
   	     begin
		if (~vh1_wrfence_flag && ~vh1_array_empty && ~latbuf_almfull) begin
		   read_latbuf_push(vh1_array, vh1_wrfence_flag);
		end
   		vc_pop <= Select_VL0;
   	     end

   	   default:
   	     begin
   		vc_pop <= Select_VL0;
   	     end

   	 endcase
	 // If a fence is set, wait till downstream gets cleared
	 if (vl0_wrfence_flag && latbuf_empty && outfifo_empty) begin
	    vl0_wrfence_flag <= 0;
	 `ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | VL0 write fence popped\n", $time);
	 `endif
	 end
	 if (vh0_wrfence_flag && latbuf_empty && outfifo_empty) begin
	    vh0_wrfence_flag <= 0;
	 `ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | VH0 write fence popped\n", $time);
	 `endif
	 end
	 if (vh1_wrfence_flag && latbuf_empty && outfifo_empty) begin
	    vh1_wrfence_flag <= 0;
	 `ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | VH1 write fence popped\n", $time);
	 `endif
	 end
	 // Release latbuf_used & record_push
	 for(int ii = 0 ; ii < NUM_WAIT_STATIONS ; ii = ii + 1) begin
	    if (records[ii].record_ready) begin
	       latbuf_used[ii] = 0;
	    end
	    if (records[ii].state == LatSc_Countdown) begin
	       records[ii].record_push <= 0;
	    end
	 end
      end
   end


   ///////////////////////////////////////////////////////////////////
   // Latency scoreboard
   // Fixme: Cache simulator output goes here
   ///////////////////////////////////////////////////////////////////
   // Get delay function
   function int get_delay(input TxHdr_t hdr);
      begin
	 return $urandom_range(15, 60);
//	 return 10;
      end
   endfunction


   // Wait station logic
   genvar 				     ii;
   generate
      for ( ii = 0; ii < NUM_WAIT_STATIONS; ii = ii + 1) begin : gen_latsc
	 // Record process
	 always @(posedge clk) begin : record_proc
	    if (rst) begin
	       records[ii].ctr_out      <= 0;
	       records[ii].record_ready <= 0;
	       records[ii].record_valid <= 0;
	    end
	    else begin
	       case (records[ii].state)
		 LatSc_Disabled:
		   begin
		      records[ii].record_ready  <= 0;
		      if (records[ii].record_push) begin
			 records[ii].record_valid <= 1;
			 records[ii].ctr_out      <= get_delay(records[ii].hdr);
			 records[ii].state        <= LatSc_Countdown;
		      end
		      else begin
			 records[ii].record_valid <= 0;
			 records[ii].ctr_out      <= 0;
			 records[ii].state        <= LatSc_Disabled;
		      end
		   end

		 LatSc_Countdown:
		   begin
		      records[ii].record_valid <= 1;
		      records[ii].ctr_out      <= records[ii].ctr_out - 1;
		      if (records[ii].ctr_out == 0) begin
			 records[ii].record_ready <= 1;
			 records[ii].state        <= LatSc_DoneReady;
		      end
		      else begin
			 records[ii].record_ready <= 0;
			 records[ii].state        <= LatSc_Countdown;
		      end
		   end

		 LatSc_DoneReady:
		   begin
		      records[ii].record_valid <= 1;
		      records[ii].ctr_out      <= 0;
		      if (records[ii].record_pop) begin
			 records[ii].record_ready <= 0;
			 records[ii].state        <= LatSc_RecordPopped;
		      end
		      else begin
			 records[ii].record_ready <= 1;
			 records[ii].state        <= LatSc_DoneReady;
		      end
		   end

		 LatSc_RecordPopped:
		   begin
		      records[ii].record_valid <= 0;
		      records[ii].record_ready <= 0;
		      records[ii].ctr_out      <= 0;
		      records[ii].state        <= LatSc_Disabled;
		   end

		 default:
		   begin
		      records[ii].record_valid <= 0;
		      records[ii].record_ready <= 0;
		      records[ii].ctr_out      <= 0;
		      records[ii].state        <= LatSc_Disabled;
		   end
	       endcase
	    end
	 end

      end
   endgenerate


   // Find a transaction to release to output stage
   function integer find_next_pop_slot();
      int ret_pop_slot;
      int pop_iter;
      int sel_slot;
      begin
	 for(pop_iter = latbuf_pop_ptr; pop_iter < latbuf_pop_ptr + NUM_WAIT_STATIONS ; pop_iter = pop_iter + 1) begin
	    sel_slot = pop_iter % NUM_WAIT_STATIONS;
	    if (records[sel_slot].record_ready) begin
	       return sel_slot;
	    end
	 end
	 return LATBUF_SLOT_INVALID;
      end
   endfunction


   logic [CCIP_RX_HDR_WIDTH-1:0] rxhdr_out_vec;
   logic [CCIP_TX_HDR_WIDTH-1:0] txhdr_out_vec;
   logic [OUTFIFO_WIDTH-1:0] 	 outfifo_data_in;

   // Read from latency scoreboard and push to outfifo
   function void latbuf_pop_unroll_outfifo(ref logic [OUTFIFO_WIDTH-1:0] array[$] );
      logic [CCIP_DATA_WIDTH-1:0] data;
      int 			  ptr;
      TxHdr_t                     txhdr;
      RxHdr_t                     rxhdr;
      logic [TID_WIDTH-1:0] 	  tid;
      int 			  line_i;
      logic 			  active;
      begin
	 ptr            = find_next_pop_slot();
	 latbuf_pop_ptr = ptr;
	 active         = 0;
	 if (ptr != LATBUF_SLOT_INVALID) begin
	    active        = 1;
	    // TxHdr
	    txhdr                   = TxHdr_t'(records[ptr].hdr);
	    // RxHdr
	    rxhdr.vc                = txhdr.vc;
	    rxhdr.poison            = 0;
	    rxhdr.hitmiss           = 0; // *FIXME*
	    rxhdr.format            = 0;
	    rxhdr.rsvd22            = 0 ;
	    // rxhdr.clnum will be updated by unroll
	    if ( (txhdr.reqtype == CCIP_RDLINE_S) || (txhdr.reqtype == CCIP_RDLINE_I) ) begin
	       rxhdr.resptype        = CCIP_RD_RESP;
	    end
	    else if ( (txhdr.reqtype == CCIP_WRLINE_I) || (txhdr.reqtype == CCIP_WRLINE_M) ) begin
	       rxhdr.resptype        = CCIP_WR_RESP;
	    end
`ifdef ASE_DEBUG
	    else begin
	       `BEGIN_RED_FONTCOLOR;
	       $display("** ERROR : Unrecognized header %x **", txhdr.reqtype);
	       `END_RED_FONTCOLOR;
	    end
`endif
	    rxhdr.mdata             = txhdr.mdata;
	    // Tid
	    tid                     = records[ptr].tid;
	    // Data
	    data                    = records[ptr].data;
	    // Book-keeping
	    // Unroll multi-line
	    for (line_i = 0 ; line_i < txhdr.len + 1 ; line_i = line_i + 1) begin
	       rxhdr.clnum = line_i;
	       txhdr.addr  = txhdr.addr + line_i;
	       array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(records[ptr].hdr) });
	       outfifo_write_en        = 1;
	       // Record pop
	       if (line_i == txhdr.len) begin
		  records[ptr].record_pop = 1;
		  latbuf_ready[ptr]       = 0;
	       end
	       else begin
		  records[ptr].record_pop = 0;
	       end
	    end
`ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | record[%d] containing tid=%x pushed to outfifo\n", $time, ptr, records[ptr].tid);
`endif

	 end
      end
      //   endtask
      endfunction

   // Latbuf pop_ptr
   always @(posedge clk) begin : latbuf_pop_proc
      if (rst) begin
	 for (int pop_i = 0; pop_i < NUM_WAIT_STATIONS; pop_i = pop_i + 1) begin
	    latbuf_ready[pop_i]       <= 0;
	    records[pop_i].record_pop <= 0;
	 end
	 outfifo_write_en <= 0;
      end
      else if (~outfifo_almfull) begin
	 latbuf_pop_unroll_outfifo(outfifo);
      end
      for(int ready_i = 0; ready_i < NUM_WAIT_STATIONS ; ready_i = ready_i + 1) begin
	 latbuf_ready[ready_i] <= records[ready_i].record_ready;
	 if (records[ready_i].state == LatSc_RecordPopped) begin
	    records[ready_i].record_pop <= 0;
	 end
	 outfifo_write_en <= 0;
      end
   end

   logic [OUTFIFO_WIDTH-1:0] outfifo_data_out;
   logic 		     outfifo_data_vld;

   // Outfifo Full/Empty
   assign outfifo_almfull  = (outfifo_cnt > VISIBLE_FULL_THRESH) ? 1 : 0;
   assign outfifo_empty    = (outfifo_cnt == 0) ? 1 : 0;
   assign outfifo_almempty = (outfifo_cnt < 2) ? 1 : 0;

   assign outfifo_read_en = read_en;

   // Module empty (out)
   assign empty = outfifo_empty;

   assign txhdr_out = TxHdr_t'(txhdr_out_vec);
   assign rxhdr_out = RxHdr_t'(rxhdr_out_vec);

   logic 		     read_en_reg;
   always @(posedge clk)
     read_en_reg <= read_en;
   
   
   function write_output ();
      begin
	 if (outfifo_cnt > 1) begin
	    { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } = outfifo.pop_front();
   	    valid_out                                           = 1;
`ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | tid=%x ejected from channel\n", $time, tid_out);
`endif
	 end
	 else begin
	    valid_out                                           = 0;	    
	 end
      end
   endfunction

   logic op_tog;
      
   always @(posedge clk) begin
      if (rst) begin
	 valid_out <= 0;
      end
      else if (read_en_reg ) begin
	 write_output();
      end
      else begin
	 valid_out <= 0;	 
      end
   end
   
   
   // typedef enum {RdPop_Idle, RdPop_Stream, RdPop_Toggle}  rdpop_state;
   // rdpop_state rdstate;

   // logic 	valid_tmp;

   // Read guard
   // always @(posedge clk) begin
   //    if (rst) begin
   // 	 rdstate <= RdPop_Idle;
   // 	 valid_out  <= 0;
   //    end
   //    else begin
   // 	 // case (rdstate)
   // 	 //   RdPop_Idle   :
   // 	 //     begin
   // 	 	if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b100 ) begin
   // 		   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   // 		   valid_out <= 1;
   // 	 	   rdstate <= RdPop_Stream;
   // 		end
   // 		else if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b110 ) begin
   // 		   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   // 		   valid_out <= 1;
   // 		   rdstate <= RdPop_Toggle;
   // 		end
   // 		else begin
   // 		   valid_out <= 0;
   // 		   rdstate <= RdPop_Idle;
   // 		end
   // 	     // end

   // 	 //   RdPop_Stream :
   // 	 //     begin
   // 	 // 	if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b100 ) begin
   // 	 // 	   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   // 	 // 	   valid_out <= 1;
   // 	 // 	   rdstate <= RdPop_Stream;
   // 	 // 	end
   // 	 // 	else if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b110 ) begin
   // 	 // 	   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   // 	 // 	   valid_out <= 1;
   // 	 // 	   rdstate <= RdPop_Toggle;
   // 	 // 	end
   // 	 // 	else begin
   // 	 // 	   valid_out <= 0;
   // 	 // 	   rdstate <= RdPop_Idle;
   // 	 // 	end
   // 	 //     end

   // 	 //   RdPop_Toggle :
   // 	 //     begin
   // 	 // 	valid_out <= 0;
   // 	 // 	rdstate <= RdPop_Idle;
   // 	 //     end

   // 	 //   default :
   // 	 //     begin
   // 	 // 	valid_out <= 0;
   // 	 // 	rdstate <= RdPop_Idle;
   // 	 //     end
   // 	 // endcase
   //    end
   // end


   /*
    * Transaction IN-OUT checker
    * Sniffs dropped transactions
    */
`ifdef ASE_DEBUG
   stream_checker #(CCIP_TX_HDR_WIDTH, TID_WIDTH)
   checkunit (clk, write_en, hdr_in, tid_in, valid_out, txhdr_out, tid_out);
`endif

endmodule // outoforder_wrf_channel
