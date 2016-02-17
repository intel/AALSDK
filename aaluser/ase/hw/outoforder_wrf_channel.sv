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
    parameter string DEBUG_LOGNAME       = "channel.log",
    parameter int    NUM_WAIT_STATIONS   = 4,
    parameter int    COUNT_WIDTH         = 8,
    parameter int    VISIBLE_DEPTH_BASE2 = 4,
    parameter int    VISIBLE_FULL_THRESH = 8,
    parameter int    UNROLL_ENABLE       = 1
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

   /*
    * Wrfence response mechanism
    * --------------------------
    *
    * - When Wrfence is observed on infifo, it is applied to
    *   required channel
    * - A response packet is formed and staged
    * THIS ENSURES WRFENCE RESPONSES ARE RETURNED IN ORDER RECEIVED
    * - A compare-wait engine waits pops wrfence_rsp_array, and
    *   waits until wrfence_flag is seen on records_t:interface
    * - Then response is placed on outfifo
    *
    */
   // Wrfence response staging
   logic [(TID_WIDTH+CCIP_RX_HDR_WIDTH+CCIP_TX_HDR_WIDTH-1):0] wrfence_rsp_array[$];

   // Wrfence assert/deassert/status/compare
   logic 						       wrfence_rspvalid;
   logic [TID_WIDTH-1:0] 				       wrfence_rsptid;
   RxHdr_t                                                     wrfence_rsphdr;
   TxHdr_t                                                     wrfence_reqhdr;
   logic 						       vl0_wrfence_deassert;
   logic 						       vh0_wrfence_deassert;
   logic 						       vh1_wrfence_deassert;

   // Outfifo
   logic [OUTFIFO_WIDTH-1:0] 	  outfifo[$:VISIBLE_DEPTH-1];

   // FIFO counts
   int 				  infifo_cnt;
   int 				  vl0_array_cnt;
   int 				  vh0_array_cnt;
   int 				  vh1_array_cnt;
   int 				  outfifo_cnt;
   int 				  wrfence_rsp_cnt;

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

   logic 			  some_lane_full;
   assign some_lane_full = vl0_array_full | vh0_array_full | vh1_array_full;

   // Tracking ID generator
   always @(posedge clk) begin : tid_proc
      if (rst)
	tid_in	<= {TID_WIDTH{1'b0}};
      else if (write_en)
	tid_in	<= tid_in + 1;
   end

   // Counts/fill level
   always @(posedge clk) begin : cnt_proc
      infifo_cnt      <= infifo.size();
      vl0_array_cnt   <= vl0_array.size();
      vh0_array_cnt   <= vh0_array.size();
      vh1_array_cnt   <= vh1_array.size();
      outfifo_cnt     <= outfifo.size();
      wrfence_rsp_cnt <= wrfence_rsp_array.size();
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
      else if (infifo_cnt > VISIBLE_FULL_THRESH ) begin
	 full <= 1;
      end
      else begin
	 full <= 0;
      end
   end

   // Full tracking
   logic full_q;
   always @(posedge clk) begin
      full_q <= full;
   end

   // If Full toggles, log the event
   `ifdef ASE_DEBUG
   always @(posedge clk) begin
      if (full_q != full_q) begin
	 $fwrite(log_fd, "%d | Module full toggled from %b to %b\n", $time, full_q, full);
      end
   end
   `endif


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
		     TxHdr_t hdr;           // in
		     logic [CCIP_DATA_WIDTH-1:0]   data;          // in
		     logic [TID_WIDTH-1:0] 	   tid;           // in
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
	 $fwrite(log_fd, "%d | WRITE : hdr=%x assigned tid=%x\n", $time, hdr_in, tid_in);
	 if (hdr_in.reqtype == ASE_WRFENCE) begin
	    $fwrite (log_fd, "%d | WrFence inserted in channel\n", $time);
	 end
	 `endif
	 infifo.push_back({ tid_in, data_in, CCIP_TX_HDR_WIDTH'(hdr_in) });
      end
   end


   // Pop infifo, arbitrate between lanes
   logic [CCIP_DATA_WIDTH-1:0]   infifo_data_out;
   logic [TID_WIDTH-1:0]         infifo_tid_out;
   logic [CCIP_TX_HDR_WIDTH-1:0] infifo_hdr_out_vec;
   TxHdr_t                       infifo_hdr_out;
   logic 			 infifo_vld;

   ccip_vc_t 		  vc_arb;
   
   logic 			 select_vc_flag;   
   
   // Select VC
   // function logic [CCIP_TX_HDR_WIDTH-1:0] select_vc(int init, input [CCIP_TX_HDR_WIDTH-1:0] hdr);
   function automatic void select_vc(int init, ref TxHdr_t hdr);
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


   // Write fence response generator
   function automatic logic [CCIP_RX_HDR_WIDTH-1:0] prepare_wrfence_response(TxHdr_t wrfence);
      RxHdr_t wrfence_rsp;
      logic [CCIP_RX_HDR_WIDTH-1:0] wrfence_rsp_vec;
      begin
	 // Precast
	 wrfence_rsp = RxHdr_t'(0);
	 // response
	 wrfence_rsp.vc_used  = wrfence.vc;
	 wrfence_rsp.resptype = ASE_WRFENCE_RSP;
	 wrfence_rsp.mdata    = wrfence.mdata;
	 // Cast back and return
	 wrfence_rsp_vec = CCIP_RX_HDR_WIDTH'(wrfence_rsp);
	 return wrfence_rsp_vec;
      end
   endfunction

   /*
    * INFIFO->VC_sel
    * -----------------------------------------
    * - Read infifo contents
    * - If WrFence (either channel)
    *   = Block required channel(s)
    *   = Stage WrFence response in wrfence_rsp_array
    * - Else !wrfence
    *   = Select VC
    *   = Stage into response array
    */
   function automatic void infifo_to_vc_push ();
      begin
	 if (~some_lane_full & ~infifo_empty) begin
	    {infifo_tid_out, infifo_data_out, infifo_hdr_out_vec} = infifo.pop_front();
	    infifo_hdr_out = TxHdr_t'(infifo_hdr_out_vec);
	    // If Write fence is observed
	    if (infifo_hdr_out.reqtype == ASE_WRFENCE) begin
	       case (infifo_hdr_out.vc)
		 // If VA, fence all channels, and stage one coalesced response
		 VC_VA:
		   begin
		      // Fence activatd
		      vl0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      vh0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      vh1_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      // Wrfence response
		      wrfence_rsp_array.push_back( {infifo_tid_out, prepare_wrfence_response(infifo_hdr_out), CCIP_TX_HDR_WIDTH'(infifo_hdr_out) } );
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc: WrFence of tid=%x inserted into VA\n", $time, infifo_tid_out);
	 `endif
		   end

		 // If single channel fence, stage requisite response
	         VC_VL0:
		   begin
		      vl0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      wrfence_rsp_array.push_back( {infifo_tid_out, prepare_wrfence_response(infifo_hdr_out), CCIP_TX_HDR_WIDTH'(infifo_hdr_out) } );
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc: WrFence of tid=%x inserted into VL0\n", $time, infifo_tid_out);
	 `endif
		   end

		 VC_VH0:
		   begin
		      vh0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      wrfence_rsp_array.push_back( {infifo_tid_out, prepare_wrfence_response(infifo_hdr_out), CCIP_TX_HDR_WIDTH'(infifo_hdr_out) } );
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc: WrFence of tid=%x inserted into VH0\n", $time, infifo_tid_out);
	 `endif
		   end

		 VC_VH1:
		   begin
		      vh1_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
		      wrfence_rsp_array.push_back( {infifo_tid_out, prepare_wrfence_response(infifo_hdr_out), CCIP_TX_HDR_WIDTH'(infifo_hdr_out) } );
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc: WrFence of tid=%x inserted into VH1\n", $time, infifo_tid_out);
	 `endif
		   end

	       endcase
	    end
	    // Any other transaction
	    else begin
	       select_vc (0, infifo_hdr_out);
	       // No fence
	       case (infifo_hdr_out.vc)
		 VC_VL0:
		   begin
		      vc_push = 3'b100;
		      vl0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VL0\n", $time, infifo_tid_out);
	 `endif
		   end

		 VC_VH0:
		   begin
		      vc_push = 3'b010;
		      vh0_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH0\n", $time, infifo_tid_out);
	 `endif
		   end

		 VC_VH1:
		   begin
		      vc_push = 3'b001;
		      vh1_array.push_back({infifo_tid_out, infifo_data_out, CCIP_TX_HDR_WIDTH'(infifo_hdr_out)});
	 `ifdef ASE_DEBUG
		      $fwrite(log_fd, "%d | infifo_to_vc : tid=%x sent to VH1\n", $time, infifo_tid_out);
	 `endif
		   end
	       endcase
	    end // else: !if(infifo_hdr_out.reqtype == ASE_WRFENCE)
	 end // if (~some_lane_full & ~infifo_empty)
      end
   endfunction


   // Virtual channel select and push
   always @(posedge clk) begin : vc_selector_proc
      if (rst) begin
	 vc_push <= 3'b000;
	 select_vc (1, infifo_hdr_out);
      end
      else if (~some_lane_full & ~infifo_empty) begin
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

   logic glbl_wrfence_pop_status;

   logic [TID_WIDTH-1:0] vl0_wrfence_tid;
   logic [TID_WIDTH-1:0] vh0_wrfence_tid;
   logic [TID_WIDTH-1:0] vh1_wrfence_tid;

   int 	 latbuf_push_ptr;
   int 	 latbuf_pop_ptr;

   int 	 vl0_records_cnt ;
   int 	 vh0_records_cnt ;
   int 	 vh1_records_cnt;


   logic [0:NUM_WAIT_STATIONS-1] latbuf_used;
   logic [0:NUM_WAIT_STATIONS-1] latbuf_ready;
   int 				 latbuf_cnt;
   logic 			 latbuf_full;
   logic 			 latbuf_almfull;
   logic 			 latbuf_empty;


   // Count used latbuf
   // function int update_latbuf_cnt();
   function void update_latbuf_cnt();
      // int 			 sum;
      int 			 jj;
      begin
	 // sum = 0;
	 vl0_records_cnt = 0;
	 vh0_records_cnt = 0;
	 vh1_records_cnt = 0;
	 for (jj =0 ; jj < NUM_WAIT_STATIONS; jj = jj + 1) begin
	    // sum = sum + latbuf_used[jj];
	    if (records[jj].record_valid) begin
	       case (records[jj].hdr.vc)
		 VC_VL0 : vl0_records_cnt = vl0_records_cnt + 1;
		 VC_VH0 : vh0_records_cnt = vh0_records_cnt + 1;
		 VC_VH1 : vh1_records_cnt = vh1_records_cnt + 1;
		 default:
		   begin
		      `BEGIN_RED_FONTCOLOR;
		      $display("ERROR : records[%02d] has a VC_VA, this is not expected ", jj);
	 `ifdef ASE_DEBUG
		      $finish;
	 `endif;
		      `END_RED_FONTCOLOR;
		   end
	       endcase
	    end
	 end
	 // return sum;
      end
   endfunction

   // Count
   always @(posedge clk) begin : latbuf_cnt_proc
      // latbuf_cnt <= update_latbuf_cnt();
      update_latbuf_cnt();
   end

   // Total count
   assign latbuf_cnt = vl0_records_cnt + vh0_records_cnt + vh1_records_cnt;

   // Latbuf status signal
   assign latbuf_empty   = (latbuf_cnt == 0) ? 1 : 0;
   assign latbuf_full    = (latbuf_cnt == NUM_WAIT_STATIONS) ? 1 : 0;
   assign latbuf_almfull = (latbuf_cnt >= (NUM_WAIT_STATIONS-3)) ? 1 : 0;

   // push_ptr selector
   function automatic integer find_next_push_slot();
      int 				     find_iter;
      int 				     ret_free_slot;
      begin
   	 for(find_iter = latbuf_push_ptr;
	     find_iter < latbuf_push_ptr + NUM_WAIT_STATIONS;
	     find_iter = find_iter + 1) begin
	    // ret_free_slot = slot_lookup[find_iter % NUM_WAIT_STATIONS];
	    ret_free_slot = find_iter % NUM_WAIT_STATIONS;
   	    if (~latbuf_used[ret_free_slot] &&
		~records[ret_free_slot].record_valid &&
		~records[ret_free_slot].record_pop ) begin
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
   function automatic void read_vc_latbuf_push (ref logic [FIFO_WIDTH-1:0] array[$:INTERNAL_FIFO_DEPTH-1],
				      ref logic 	   	 wrfence_flag,
				      ref logic [TID_WIDTH-1:0]  wrfence_tid
				      );
      logic [CCIP_TX_HDR_WIDTH-1:0] 		     array_hdr;
      logic [CCIP_DATA_WIDTH-1:0] 		     array_data;
      logic [TID_WIDTH-1:0] 			     array_tid;
      TxHdr_t                                        hdr;
      int 					     ptr;
      begin
	 ptr = find_next_push_slot();
	 latbuf_push_ptr = ptr;
	 // if (ptr != LATBUF_SLOT_INVALID) begin // *FIXME*: Potential catastrophe here
	 if (~latbuf_almfull) begin
	    {array_tid, array_data, array_hdr} = array.pop_front();
	    hdr = TxHdr_t'(array_hdr);
	    if (hdr.reqtype == ASE_WRFENCE) begin
	       wrfence_flag = 1;
	       wrfence_tid  = array_tid;
	 `ifdef ASE_DEBUG
	       $fwrite(log_fd, "%d | latbuf_push : saw Wrfence on tid=%x on channel %d\n", $time, array_tid, hdr.vc);
	 `endif
	    end
	    else begin
	       if (ptr != LATBUF_SLOT_INVALID) begin
		  records[ptr].hdr          = hdr;
		  records[ptr].data         = array_data;
		  records[ptr].tid          = array_tid;
		  records[ptr].record_push  = 1;
		  records[ptr].record_valid = 1;
		  latbuf_used[ptr]          = 1;
	 `ifdef ASE_DEBUG
		  $fwrite(log_fd, "%d | latbuf_push : tid=%x sent to record[%02d]\n", $time, array_tid, ptr);
		  $fwrite(log_fd, "%d | latbuf_push : tid=%x cause latbuf_used=%x\n", $time, array_tid, latbuf_used);
	 `endif
	       end // if (ptr != LATBUF_SLOT_INVALID)
	       else begin
		  array.push_front({array_tid, array_data, array_hdr});
	 `ifdef ASE_DEBUG
		  $fwrite(log_fd, "%d | latbuf_used : backtrace tid=%x, latbuf slot unavailable\n", $time, array_tid);
	 `endif
	       end // else: !if(ptr != LATBUF_SLOT_INVALID)
	    end // else: !if(hdr.reqtype == ASE_WRFENCE)
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
		   read_vc_latbuf_push(vl0_array, vl0_wrfence_flag, vl0_wrfence_tid );
		end
   		vc_pop <= Select_VH0;
   	     end

   	   Select_VH0:
   	     begin
		if (~vh0_wrfence_flag && ~vh0_array_empty && ~latbuf_almfull) begin
		   read_vc_latbuf_push(vh0_array, vh0_wrfence_flag, vh0_wrfence_tid );
		end
   		vc_pop <= Select_VH1;
   	     end

   	   Select_VH1:
   	     begin
		if (~vh1_wrfence_flag && ~vh1_array_empty && ~latbuf_almfull) begin
		   read_vc_latbuf_push(vh1_array, vh1_wrfence_flag, vh1_wrfence_tid );
		end
   		vc_pop <= Select_VL0;
   	     end

   	   default:
   	     begin
   		vc_pop <= Select_VL0;
   	     end

   	 endcase
	 // If a fence is set, wait till downstream gets cleared
	 // if (vl0_wrfence_flag && (vl0_records_cnt == 0) && outfifo_empty) begin
	 if (vl0_wrfence_flag && (vl0_records_cnt == 0) && vl0_wrfence_deassert) begin
	    vl0_wrfence_flag <= 0;
	 `ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | VL0 write fence popped\n", $time);
	 `endif
	 end
	 // if (vh0_wrfence_flag && (vh0_records_cnt == 0) && outfifo_empty) begin
	 if (vh0_wrfence_flag && (vh0_records_cnt == 0) && vh0_wrfence_deassert) begin
	    vh0_wrfence_flag <= 0;
	 `ifdef ASE_DEBUG
	    $fwrite(log_fd, "%d | VH0 write fence popped\n", $time);
	 `endif
	 end
	 // if (vh1_wrfence_flag && (vh1_records_cnt == 0) && outfifo_empty) begin
	 if (vh1_wrfence_flag && (vh1_records_cnt == 0) && vh1_wrfence_deassert) begin
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
	 `ifdef ASE_DEBUG
	 if (hdr.reqtype == ASE_WRFENCE) begin
	    $fwrite(log_fd, "ERROR : WrFence must not enter latency scoreboard");
	    $finish;
	 end
	 `endif
	 return $urandom_range(15, 60);
	 // return 10;
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
   logic 			 unroll_active;

   // Read from latency scoreboard and push to outfifo
   task automatic latbuf_pop_unroll_outfifo(ref logic [OUTFIFO_WIDTH-1:0] array[$:VISIBLE_DEPTH-1] );
      logic [CCIP_DATA_WIDTH-1:0] data;
      int 			  ptr;
      TxHdr_t                     txhdr;
      RxHdr_t                     rxhdr;
      logic [TID_WIDTH-1:0] 	  tid;
      int 			  line_i;
      logic [41:0] 		  base_addr;
      begin
	 ptr            = find_next_pop_slot();
	 latbuf_pop_ptr = ptr;
	 unroll_active         = 0;
	 outfifo_write_en        = 0;
	 if (ptr != LATBUF_SLOT_INVALID) begin
	    unroll_active        = 1;
	    // TxHdr
	    txhdr                   = records[ptr].hdr;
	    base_addr               = txhdr.addr;
	    // RxHdr
	    rxhdr.vc_used           = txhdr.vc;
	    rxhdr.poison            = 0;
	    rxhdr.hitmiss           = 0; // *FIXME*
	    rxhdr.format            = 0;
	    rxhdr.rsvd22            = 0 ;
	    // rxhdr.clnum will be updated by unroll
	    if ( (txhdr.reqtype == ASE_RDLINE_S) || (txhdr.reqtype == ASE_RDLINE_I) ) begin
	       rxhdr.resptype        = ASE_RD_RSP;
	    end
	    else if ( (txhdr.reqtype == ASE_WRLINE_I) || (txhdr.reqtype == ASE_WRLINE_M) ) begin
	       rxhdr.resptype        = ASE_WR_RSP;
	    end
            `ifdef ASE_DEBUG
	    else begin
	       `BEGIN_RED_FONTCOLOR;
	       $display("** ERROR : Unrecognized header %x **", txhdr.reqtype);
	       $finish;
	       `END_RED_FONTCOLOR;
	    end
            `endif
	    rxhdr.mdata             = txhdr.mdata;
	    // Tid
	    tid                     = records[ptr].tid;
	    // Data
	    data                    = records[ptr].data;
	    // Dumbing down Unroll multi-line
	    line_i = 0;

	    if (UNROLL_ENABLE == 1) begin
	       case (txhdr.len)
		 // Single cache line
		 ASE_1CL:
		   begin
		      outfifo_write_en        = 1;
		      rxhdr.clnum             = ASE_1CL;
		      txhdr.addr              = base_addr + 0;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
	       	      records[ptr].record_pop = 1;
	       	      latbuf_ready[ptr]       = 0;
		      unroll_active = 0;
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      outfifo_write_en        = 0;
		   end

		 // 2 cache lines
		 ASE_2CL:
		   begin
		      outfifo_write_en        = 1;
		      rxhdr.clnum             = ASE_1CL;
		      txhdr.addr              = base_addr + 0;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
		      outfifo_write_en        = 1;
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      rxhdr.clnum             = ASE_2CL;
		      txhdr.addr              = base_addr + 1;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
	       	      records[ptr].record_pop = 1;
	       	      latbuf_ready[ptr]       = 0;
		      unroll_active           = 0;
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      outfifo_write_en        = 0;
		   end

		 // 3 cache line requests (not supported)
		 ASE_3CL :
		   begin
		      $display("** ERROR (%m): txhdr.len = %b is not valid **", txhdr.len);
		      `ifdef ASE_DEBUG
		      $fwrite(log_fd, "** ERROR (%m): txhdr.len = %b is not valid **", txhdr.len);
		      `endif
		   end

		 // 4 cache lines
		 ASE_4CL:
		   begin
		      outfifo_write_en        = 1;
		      rxhdr.clnum             = ASE_1CL;
		      txhdr.addr              = base_addr + 0;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      rxhdr.clnum             = ASE_2CL;
		      txhdr.addr              = base_addr + 1;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      rxhdr.clnum             = ASE_3CL;
		      txhdr.addr              = base_addr + 2;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      rxhdr.clnum             = ASE_4CL;
		      txhdr.addr              = base_addr + 3;
		      array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
	       	      records[ptr].record_pop = 1;
	       	      latbuf_ready[ptr]       = 0;
		      unroll_active           = 0;
                      `ifdef ASE_DEBUG
	    	      $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
                      `endif
		      @(posedge clk);
		      outfifo_write_en        = 0;
		   end
	       endcase // case (txhdr.len)
	    end // if (UNROLL_ENABLE == 1)
	    else begin
	       outfifo_write_en        = 1;
	       rxhdr.clnum             = txhdr.len;
	       txhdr.addr              = base_addr + 0;
	       array.push_back({ records[ptr].tid, records[ptr].data, CCIP_RX_HDR_WIDTH'(rxhdr), CCIP_TX_HDR_WIDTH'(txhdr) });
	       records[ptr].record_pop = 1;
	       latbuf_ready[ptr]       = 0;
	       unroll_active = 0;
               `ifdef ASE_DEBUG
	       $fwrite(log_fd, "%d | record[%02d] with tid=%x multiline unroll %x\n", $time, ptr, records[ptr].tid, txhdr.addr);
	       $fwrite(log_fd, "%d | latbuf_pop : tid=%x out of record[%02d]\n", $time, records[ptr].tid, ptr);
	       $fwrite(log_fd, "%d | latbuf_pop : tid=%x cause latbuf_used=%x\n", $time, records[ptr].tid, latbuf_used);
	       `endif
	       @(posedge clk);
	       outfifo_write_en        = 0;
	    end
	 end // if (ptr != LATBUF_SLOT_INVALID)
	 outfifo_write_en        = 0;
      end
   endtask

   // Wrfence response monitor
   always @(posedge clk) begin
      if (rst) begin
	 wrfence_rspvalid <= 0;
      end
      else if (~wrfence_rspvalid & (wrfence_rsp_cnt != 0)) begin
	 wrfence_rspvalid <= 1;
	 {wrfence_rsptid, wrfence_rsphdr, wrfence_reqhdr } = wrfence_rsp_array.pop_front();
      end
      else if (wrfence_rspvalid & (vl0_wrfence_deassert|vh0_wrfence_deassert|vh1_wrfence_deassert) ) begin
	 wrfence_rspvalid <= 0;
      end
   end


   logic [2:0] latbuf_pop_proc_status;

   // Latbuf pop_ptr
   always @(posedge clk) begin : latbuf_pop_proc
      if (rst) begin
	 for (int pop_i = 0; pop_i < NUM_WAIT_STATIONS; pop_i = pop_i + 1) begin
	    latbuf_ready[pop_i]       <= 0;
	    records[pop_i].record_pop <= 0;
	 end
	 vl0_wrfence_deassert <= 0;
	 vh0_wrfence_deassert <= 0;
	 vh1_wrfence_deassert <= 0;
	 latbuf_pop_proc_status	<= 3'b000;
	 glbl_wrfence_pop_status <= 0;
      end
      // empty outfifo on normal transactions
      else if (~outfifo_almfull && ~latbuf_empty ) begin
	 latbuf_pop_unroll_outfifo(outfifo);
	 vl0_wrfence_deassert <= 0;
	 vh0_wrfence_deassert <= 0;
	 vh1_wrfence_deassert <= 0;
	 latbuf_pop_proc_status	<= 3'b110;
	 glbl_wrfence_pop_status <= 0;
      end
      // Pop write fence
      else if (wrfence_rspvalid && (vl0_wrfence_flag|vh0_wrfence_flag|vh1_wrfence_flag) && ~glbl_wrfence_pop_status) begin
	 case (wrfence_rsphdr.vc_used)
	   VC_VA :
	     begin
		if ( (wrfence_rsptid == vl0_wrfence_tid) &&
		     (wrfence_rsptid == vh0_wrfence_tid) &&
		     (wrfence_rsptid == vh1_wrfence_tid) &&
		     vl0_wrfence_flag &&
		     vh0_wrfence_flag &&
		     vh1_wrfence_flag ) begin
		   vl0_wrfence_deassert <= 1;
		   vh0_wrfence_deassert <= 1;
		   vh1_wrfence_deassert <= 1;
		   latbuf_pop_proc_status	<= 3'b100;
		   glbl_wrfence_pop_status <= 1;
		   outfifo.push_back({wrfence_rsptid, {CCIP_DATA_WIDTH{1'b0}}, CCIP_RX_HDR_WIDTH'(wrfence_rsphdr), CCIP_TX_HDR_WIDTH'(wrfence_reqhdr) });
		end
	     end

	   VC_VL0:
	     begin
		if ((wrfence_rsptid == vl0_wrfence_tid) && vl0_wrfence_flag) begin
		   vl0_wrfence_deassert <= 1;
		   vh0_wrfence_deassert <= 0;
		   vh1_wrfence_deassert <= 0;
		   latbuf_pop_proc_status	<= 3'b101;
		   glbl_wrfence_pop_status <= 1;
		   outfifo.push_back({wrfence_rsptid, {CCIP_DATA_WIDTH{1'b0}}, CCIP_RX_HDR_WIDTH'(wrfence_rsphdr), CCIP_TX_HDR_WIDTH'(wrfence_reqhdr) });
		end
	     end

	   VC_VH0:
	     begin
		if ((wrfence_rsptid == vh0_wrfence_tid) && vh0_wrfence_flag ) begin
		   vl0_wrfence_deassert <= 0;
		   vh0_wrfence_deassert <= 1;
		   vh1_wrfence_deassert <= 0;
		   latbuf_pop_proc_status	<= 3'b110;
		   glbl_wrfence_pop_status <= 1;
		   outfifo.push_back({wrfence_rsptid, {CCIP_DATA_WIDTH{1'b0}}, CCIP_RX_HDR_WIDTH'(wrfence_rsphdr), CCIP_TX_HDR_WIDTH'(wrfence_reqhdr) });
		end
	     end

	   VC_VH1:
	     begin
		if ((wrfence_rsptid == vh1_wrfence_tid) && vh1_wrfence_flag ) begin
		   vl0_wrfence_deassert <= 0;
		   vh0_wrfence_deassert <= 0;
		   vh1_wrfence_deassert <= 1;
		   latbuf_pop_proc_status	<= 3'b111;
		   glbl_wrfence_pop_status <= 1;
		   outfifo.push_back({wrfence_rsptid, {CCIP_DATA_WIDTH{1'b0}}, CCIP_RX_HDR_WIDTH'(wrfence_rsphdr), CCIP_TX_HDR_WIDTH'(wrfence_reqhdr) });
		end
	     end
	 endcase
      end
      else begin
	 vl0_wrfence_deassert <= 0;
	 vh0_wrfence_deassert <= 0;
	 vh1_wrfence_deassert <= 0;
	 latbuf_pop_proc_status	<= 3'b000;
	 glbl_wrfence_pop_status <= 0;
      end
      // Book keeping
      for(int ready_i = 0; ready_i < NUM_WAIT_STATIONS ; ready_i = ready_i + 1) begin
	 latbuf_ready[ready_i] <= records[ready_i].record_ready;
	 if ( (records[ready_i].state == LatSc_RecordPopped) ||
	      (records[ready_i].state == LatSc_Disabled) ) begin
	    records[ready_i].record_pop <= 0;
	 end
      end
   end

   // Outfifo Full/Empty
   assign outfifo_almfull  = (outfifo_cnt > VISIBLE_FULL_THRESH) ? 1 : 0;
   assign outfifo_empty    = (outfifo_cnt == 0) ? 1 : 0;
   assign outfifo_almempty = (outfifo_cnt <= 2) ? 1 : 0;

   assign outfifo_read_en = read_en;

   // Module empty (out)
   assign empty = outfifo_empty;

   assign txhdr_out = TxHdr_t'(txhdr_out_vec);
   assign rxhdr_out = RxHdr_t'(rxhdr_out_vec);


   //////////////////////////////////////////////////////////////////////
   // Read guard *FIXME*

   typedef enum {RdPop_Idle, RdPop_Stream, RdPop_Toggle}  rdpop_state;
   rdpop_state rdstate;

   always @(posedge clk) begin : read_guard_proc
      if (rst) begin
   	 rdstate <= RdPop_Idle;
   	 valid_out  <= 0;
      end
      else begin
   	 case (rdstate)
   	   RdPop_Idle   :
   	     begin
   	 	if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b100 ) begin
   		   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   		   valid_out <= 1;
   	 	   rdstate <= RdPop_Stream;
   		end
   		else if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b110 ) begin
   		   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   		   valid_out <= 1;
 		   rdstate <= RdPop_Toggle;
   		end
   		else begin
   		   valid_out <= 0;
   		   rdstate <= RdPop_Idle;
   		end
   	     end

   	   RdPop_Stream :
   	     begin
   	 	if ( {read_en, outfifo_almempty, outfifo_empty } == 3'b100 ) begin
   		   { tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   		   valid_out <= 1;
   	 	   rdstate <= RdPop_Stream;
   	 	end
   	 	else begin
   		   valid_out <= 0;
   	 	   rdstate <= RdPop_Idle;
   	 	end
   	     end

   	   RdPop_Toggle :
   	     begin
   		//{ tid_out, data_out, rxhdr_out_vec, txhdr_out_vec } <= outfifo.pop_front();
   	 	valid_out <= 0;
   	 	rdstate <= RdPop_Idle;
   	     end

   	   default :
   	     begin
   	 	valid_out <= 0;
   	 	rdstate <= RdPop_Idle;
   	     end
   	 endcase
      end
   end

   // Log output pop
`ifdef ASE_DEBUG
   always @(posedge clk) begin
      if (valid_out) begin
	 $fwrite(log_fd, "%d | tid=%x with txhdr=%x ejected from channel\n", $time, tid_out, txhdr_out);
      end
   end
`endif


   /*
    * Transaction IN-OUT checker
    * Sniffs dropped transactions
    */
`ifdef ASE_DEBUG
   stream_checker #(CCIP_TX_HDR_WIDTH, TID_WIDTH, UNROLL_ENABLE)
   checkunit (clk, write_en, hdr_in, tid_in, valid_out, txhdr_out, rxhdr_out,tid_out);
`endif

endmodule // outoforder_wrf_channel
