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
 * Module Info: CCI Emulation top-level - SystemVerilog Module
 * Language   : System{Verilog}
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * MAJOR UPGRADES:
 * - RRS: Wed Aug 10 22:17:28 PDT 2011
 *   Completed FIFO'ing all channels in all directions
 * - RRS: Tue Jun 17 16:46:06 PDT 2014
 *   Started cleaning up code to add latency model
 *   Connect up new transactions CCI 1.8
 * - RRS: Tue Dec 23 11:01:28 PST 2014
 *   Optimizing ASE for performance
 *   Added return path FIFOs for marshalling
 */

`include "ase_global.vh"
`include "platform.vh"

// CCI to Memory translator module
module cci_emulator();

   /*
    * DPI import/export functions
    */
   // Scope function
   import "DPI" function void scope_function();      
   // ASE Initialize function
   import "DPI-C" context task ase_init();
   // Indication that ASE is ready
   import "DPI-C" function void ase_ready();
   // Global listener function
   import "DPI-C" context task ase_listener();

   // ASE config data exchange (read from ase.cfg)
   export "DPI-C" task ase_config_dex;

   // CSR Write Dispatch
   export "DPI-C" task csr_write_dispatch;
   // Unordered message dispatch
   export "DPI-C" task umsg_dispatch;

   // CAPCM initilize
   import "DPI-C" context task capcm_init();
   // CAPCM destroy
   // import "DPI-C" context task capcm_deinit();

   // Start simulation structures teardown
   import "DPI-C" context task start_simkill_countdown();
   // Signal to kill simulation
   export "DPI-C" task simkill;
      
   // Data exchange for READ system/CAPCM memory line
   import "DPI-C" function void rd_memline_dex(inout cci_pkt foo, inout int cl_addr, inout int mdata );
   // Data exchange for WRITE system/CAPCM memory line
   import "DPI-C" function void wr_memline_dex(inout cci_pkt foo, inout int cl_addr, inout int mdata, inout bit [511:0] wr_data );
   // Software controlled process - run clocks
   export "DPI-C" task run_clocks;

   // Declare packets for each channel
   cci_pkt rx0_pkt, rx1_pkt;

   // Scope generator
   initial scope_function();
   

   /*
    * FUNCTION: Convert CAPCM_GB_SIZE to NUM_BYTES
    */
   function automatic longint conv_gbsize_to_num_bytes(int gb_size);
      begin
	 return (gb_size*1024*1024*1024);
      end
   endfunction


   /*
    * FUNCTION: Return absolute value
    */
   function automatic int abs_val(int num);
      begin
	 return (num < 0) ? ~num : num;
      end
   endfunction

   /* ***************************************************************************
    * CCI signals declarations
    * ***************************************************************************
    *
    *                          -------------------
    *   tx0_header     ---61-->|                 |---18---> rx0_header
    *   tx0_valid      ------->|                 |---512--> rx0_data
    *   tx0_almostfull <-------|                 |--------> rx0_rdvalid
    *   tx1_header     ---61-->|      ASE        |--------> rx0_wrvalid
    *   tx1_data       --512-->|     BLOCK       |--------> rx0_cfgvalid
    *   tx1_valid      ------->|                 |--------> rx0_umsgvalid (TBD)
    *   tx1_almostfull <-------|                 |--------> rx0_intrvalid (TBD)
    *   tx1_intrvalid  ------->|                 |---18---> rx1_header
    *                          |                 |--------> rx1_intrvalid (TBD)
    *                          |                 |--------> rx1_wrvalid
    *                          |                 |--------> lp_initdone
    *                          |                 |--------> reset
    *                          |                 |--------> clk
    *                          -------------------
    *
    * ***************************************************************************/

   logic                          clk   ;                  // out
   logic 			  resetb ;                 // out
   logic 			  lp_initdone ;            // out
   logic [`CCI_TX_HDR_WIDTH-1:0]  tx_c0_header;            // in
   logic 			  tx_c0_rdvalid;           // in
   logic 			  tx_c0_almostfull;        // out
   logic [`CCI_TX_HDR_WIDTH-1:0]  tx_c1_header;            // in
   logic [`CCI_DATA_WIDTH-1:0] 	  tx_c1_data;              // in
   logic 			  tx_c1_wrvalid;           // in
   logic 			  tx_c1_almostfull;        // out
   logic [`CCI_RX_HDR_WIDTH-1:0]  rx_c0_header;            // out
   logic [`CCI_DATA_WIDTH-1:0] 	  rx_c0_data;              // out
   logic 			  rx_c0_rdvalid;           // out
   logic 			  rx_c0_wrvalid;           // out
   logic 			  rx_c0_cfgvalid;          // out
   logic [`CCI_RX_HDR_WIDTH-1:0]  rx_c1_header;            // out
   logic 			  rx_c1_wrvalid;           // out
   logic 			  rx_c0_umsgvalid;         // out
   logic 			  tx_c1_intrvalid;         // in
   logic 			  rx_c0_intrvalid;         // out
   logic 			  rx_c1_intrvalid;         // out


   // LP initdone & reset registered signals
   logic 			  lp_initdone_q;
   logic 			  resetb_q;
   logic 			  tx_c1_intrvalid_sel;

   // Derived clocks
   logic 			  clk_32ui; // Normal 200 Mhz clock
   logic 			  clk_16ui; // Faster 400 Mhz clock

   /*
    * Overflow/underflow signal checks
    */
   logic 			  tx0_underflow;
   logic 			  tx1_underflow;
   logic 			  tx0_overflow;
   logic 			  tx1_overflow;

   /*
    * State indicators
    */
   typedef enum 		  {RxIdle, RxAFUCSRWrite, RxQLPCSRWrite, RxReadResp, RxWriteResp, RxUmsg, RxUmsgHint, RxUmsgData, RxIntrResp}
				  RxGlue_StateEnum;
   RxGlue_StateEnum rx0_state;
   RxGlue_StateEnum rx1_state;


   /*
    * Clock process: Operates the CAFU clock
    */
   // 200 Mhz clock
   initial begin : clk32ui_proc
      begin
	 clk_32ui = 0;
	 forever begin
	    #`CLK_32UI_TIME;
	    clk_32ui = 1'b0;
	    #`CLK_32UI_TIME;
	    clk_32ui = 1'b1;
	 end
      end
   end

   // ASE clock
   assign clk = clk_32ui;

   // 400 Mhz clock
   initial begin : clk16ui_proc
      begin
	 clk_16ui = 0;
	 forever begin
	    #`CLK_16UI_TIME;
	    clk_16ui = 1'b0;
	    #`CLK_16UI_TIME;
	    clk_16ui = 1'b1;
	 end
      end
   end

   // Reset management
   logic 			  sys_reset_n;
   logic 			  sw_reset_n;
   logic 			  sw_reset_n_q;

   /*
    * AFU reset - software & system resets
    */
   always @(posedge clk) begin
      if ((sys_reset_n == 1'b0) || (sw_reset_n == 1'b0)) begin
   	 resetb <= 1'b0;
      end
      else begin
   	 resetb <= 1'b1;
      end
   end


   /*
    * run_clocks : Run 'n' clocks
    * Software controlled event trigger for watching signals
    *
    */
   task run_clocks (int num_clks);
      int clk_iter;
      begin
	 for (clk_iter = 0; clk_iter < num_clks; clk_iter = clk_iter + 1) begin
	    @(posedge clk);
	 end
      end
   endtask


   /*
    * CSR Write infrastructure
    * csr_write_dispatch: A Single task to dispatch CSR Writes
    */
   parameter int CSR_FIFO_WIDTH = 32 + 32;

   logic [CSR_FIFO_WIDTH-1:0] csrff_din;
   logic [CSR_FIFO_WIDTH-1:0] csrff_dout;
   logic 		      csrff_write;
   logic 		      csrff_pop;
   logic 		      csrff_read;
   logic 		      csrff_valid;
   logic 		      csrff_full;
   logic 		      csrff_empty;
   logic 		      csrff_overflow;
   logic 		      csrff_underflow;

   task csr_write_dispatch(int init, int csr_index, int csr_data);
      begin
	 if (init) begin
	    csrff_write = 0;
	    csrff_din = 0;
	 end
	 else begin
	    csrff_write = 0;
	    while (csrff_full) begin
	       `BEGIN_RED_FONTCOLOR;
	       $display("SIM-SV: WARNING => CSR FIFO is almost full, waiting to clear up");
	       `END_RED_FONTCOLOR;
	       run_clocks(1);
	    end
	    run_clocks(1);
	    csrff_din = {csr_index, csr_data};
	    csrff_write = 1;
	    run_clocks(1);
	    csrff_write = 0;
	 end
      end
   endtask


   // CSR write FIFO
   ase_fifo
     #(
       .DATA_WIDTH     ( CSR_FIFO_WIDTH ),
       .DEPTH_BASE2    ( 10 ),
       .ALMFULL_THRESH ( 960 )
       )
   csrwr_fifo
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( csrff_write ),
      .data_in    ( csrff_din ),
      .rd_en      ( csrff_pop ),
      .data_out   ( csrff_dout ),
      .data_out_v ( csrff_valid ),
      .alm_full   ( csrff_full ),
      .full       (  ),
      .empty      ( csrff_empty ),
      .count      (  ),
      .overflow   ( csrff_overflow ),
      .underflow  ( csrff_underflow )
      );

   assign csrff_pop = ~csrff_empty && csrff_read;

   /*
    * Umsg infrastructure
    * umsg_dispatch: Single push process triggering UMSG machinery
    */
   parameter int UMSG_FIFO_WIDTH = `CCI_RX_HDR_WIDTH + `CCI_DATA_WIDTH;

   logic [UMSG_FIFO_WIDTH-1:0] umsgff_din;
   logic [UMSG_FIFO_WIDTH-1:0] umsgff_dout;
   logic 		       umsgff_write;
   logic 		       umsgff_pop;
   logic 		       umsgff_read;
   logic 		       umsgff_valid;
   logic 		       umsgff_full;
   logic 		       umsgff_empty;
   logic 		       umsgff_overflow;
   logic 		       umsgff_underflow;

   int 			       umas_iter;

   umsg_t umsg_array[`UMSG_MAX_MSG];
   int 			       umsg_data_slot;
   int 			       umsg_hint_slot;

   logic [0:`UMSG_MAX_MSG-1]   umas_rst;
   logic [0:`UMSG_MAX_MSG-1]   umas_en;
   logic [0:`UMSG_MAX_MSG-1]   umas_rdy;
   logic [`CCI_DATA_WIDTH-1:0] umsg_data_array [0:`UMSG_MAX_MSG-1];


   // UMSG dispatch function
   task umsg_dispatch(int init, int umas_en, int hint_en, int umsg_id, bit [`CCI_DATA_WIDTH-1:0] umsg_data_in);
      begin
	 if (init) begin
	    for (umas_iter = 0; umas_iter < `UMSG_MAX_MSG; umas_iter = umas_iter + 1) begin
	       umsg_array[ umas_iter ].umsg_enable = 0;
	       umsg_data_array[ umas_iter ]	<= `CCI_DATA_WIDTH'bx;
	    end
	 end
	 else begin
	    if (umas_en) begin
	       if ( umsg_data_in != umsg_data_array[umsg_id]) begin
		  umsg_data_array[ umsg_id ] = umsg_data_in;
		  if (hint_en) begin
		     umsg_array[ umsg_id ].umsg_data   = umsg_data_in;
		     umsg_array[ umsg_id ].umsg_hint   = 1;
		     umsg_array[ umsg_id ].umsg_enable = 1;
		     run_clocks (1);
		  end
		  else begin
		     umsg_array[ umsg_id ].umsg_data   = umsg_data_in;
		     umsg_array[ umsg_id ].umsg_hint   = 0;
		     umsg_array[ umsg_id ].umsg_enable = 1;
		     run_clocks (1);
		  end
	       end
	    end
	    else begin
	       umsg_array[ umsg_id ].umsg_enable = 0;
	    end
	 end
      end
   endtask

   // UMSG Hint-to-Data time emulator (toaster style)
   // New Umsg hints to same location are ignored
   // If Data is same, hints dont get generated
   genvar umas_i;
   generate
      for ( umas_i = 0; umas_i > `UMSG_MAX_MSG; umas_i = umas_i + 1 ) begin
	 // UMSG state machine
	 always @(posedge clk) begin
	      if (~sys_reset_n) begin
		 umsg_array[umas_i].umsg_state			<= UMsg_Idle;
	      end
	      else begin
		 case (umsg_array[umas_i].umsg_state)
		   UMsg_Idle:
		     begin
			if (umsg_array[umas_i].umsg_enable && umsg_array[umas_i].umsg_hint) begin
			   umsg_array[umas_i].umsg_timer	<= `UMSG_HINT2DATA_DELAY;
			   umsg_array[umas_i].umsg_state	<= UMsg_SendHint;
			end
			else if (umsg_array[umas_i].umsg_enable && ~umsg_array[umas_i].umsg_hint) begin
			   umsg_array[umas_i].umsg_timer	<= `UMSG_NOHINT_DATADELAY;
			   umsg_array[umas_i].umsg_state	<= UMsg_H2D_Wait;
			end
			else begin
			   umsg_array[umas_i].umsg_timer	<= `UMSG_DELAY_TIMER_LOG2'b0;
			   umsg_array[umas_i].umsg_state	<= UMsg_Idle;
			end
		     end

		   UMsg_SendHint:
		     begin
			if (umsg_array[umas_i].umsghint_push) begin
			   umsg_array[umas_i].umsg_state	<= UMsg_H2D_Wait;
			end
			else begin
			   umsg_array[umas_i].umsg_state	<= UMsg_SendHint;
			end
		     end

		   UMsg_H2D_Wait:
		     begin
			umsg_array[umas_i].umsg_timer		<= umsg_array[umas_i].umsg_timer - 1;
			if (umsg_array[umas_i].umsg_timer	<= 1) begin
			   umsg_array[umas_i].umsg_state	<= UMsg_SendData;
			end
			else begin
			   umsg_array[umas_i].umsg_state	<= UMsg_H2D_Wait;
			end
		     end

		   UMsg_SendData:
		     begin
			if (umsg_array[umas_i].umsgdata_push) begin
			   umsg_array[umas_i].umsg_state	<= UMsg_Idle;
			end
			else begin
			   umsg_array[umas_i].umsg_state	<= UMsg_SendData;
			end
		     end

		   default:
		     begin
			umsg_array[umas_i].umsg_state	<= UMsg_Idle;
		     end
		 endcase
	      end
	   end
	end
   endgenerate

   // Find UMSG Hintable slot
   function int find_umsg_hint ();
      int ret_hint_slot;
      int slot;
      begin
	 ret_hint_slot = 255;
	 for (slot = 0; slot < `UMSG_MAX_MSG; slot = slot + 1) begin
	    if (umsg_array[slot].umsg_state == UMsg_SendHint) begin
	       ret_hint_slot = slot;
	       break;
	    end
	 end
	 return ret_hint_slot;
      end
   endfunction

   // Find UMSG Data slot to send
   function int find_umsg_data();
      int ret_data_slot;
      int slot;
      begin
	 ret_data_slot = 255;
	 for (slot = 0; slot < `UMSG_MAX_MSG; slot = slot + 1) begin
	    if (umsg_array[slot].umsg_state == UMsg_SendData) begin
	       ret_data_slot = slot;
	       break;
	    end
	 end
	 return ret_data_slot;
      end
   endfunction

   // Calculate slots for UMSGs
   always @(posedge clk) begin
      umsg_data_slot = find_umsg_data();
      umsg_hint_slot = find_umsg_hint();
   end

   // UMSG latency model to UMSG FIFO glue process
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 umsgff_write	<= 0;
      end
      else begin
	 if ((umsg_data_slot != 255) && ~umsgff_full) begin
	    umsgff_din <= { `ASE_RX0_UMSG, 1'b0,
			  1'b0, 6'b0,
			  1'b0, umsg_data_slot[4:0],
			  umsg_data_array[umsg_data_slot] };
	    umsgff_write <= 1;
	 end
	 else if ((umsg_hint_slot != 255) && ~umsgff_full) begin
	    umsgff_din <= { `ASE_RX0_UMSG, 1'b0,
			  1'b1, 6'b0,
			  1'b0, umsg_hint_slot[4:0],
			  `CCI_DATA_WIDTH'b0 };
	    umsgff_write <= 1;
	 end
	 else begin
	    umsgff_write  <= 0;
	 end
      end
   end

   // Unordered message FIFO
   ase_fifo
     #(
       .DATA_WIDTH     ( UMSG_FIFO_WIDTH ),
       .DEPTH_BASE2    ( 10 ),
       .ALMFULL_THRESH ( 960 )
       )
   umsg_fifo
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( umsgff_write ),
      .data_in    ( umsgff_din ),
      .rd_en      ( umsgff_pop ),
      .data_out   ( umsgff_dout ),
      .data_out_v ( umsgff_valid ),
      .alm_full   ( umsgff_full ),
      .full       (  ),
      .empty      ( umsgff_empty ),
      .count      (  ),
      .overflow   ( umsgff_overflow ),
      .underflow  ( umsgff_underflow )
      );

   assign umsgff_pop = ~umsgff_empty && umsgff_read;


   /*
    * Config data exchange - Supplied by ase.cfg
    */
   task ase_config_dex(ase_cfg_t cfg_in);
      begin
	 cfg.enable_timeout     = cfg_in.enable_timeout   ;
	 cfg.enable_reuse_seed  = cfg_in.enable_reuse_seed;
	 cfg.enable_capcm       = cfg_in.enable_capcm     ;
	 cfg.memmap_sad_setting = cfg_in.memmap_sad_setting    ;
	 cfg.enable_umsg        = cfg_in.enable_umsg      ;
	 cfg.num_umsg_log2      = cfg_in.num_umsg_log2    ;
	 cfg.enable_intr        = cfg_in.enable_intr      ;
	 cfg.enable_ccirules    = cfg_in.enable_ccirules  ;
	 cfg.enable_bufferinfo  = cfg_in.enable_bufferinfo;
	 cfg.enable_cl_view     = cfg_in.enable_cl_view   ;
	 cfg.enable_asedbgdump  = cfg_in.enable_asedbgdump;
      end
   endtask


   /*
    * Count Valid signals
    */
   int ase_rx0_cfgvalid_cnt;
   int ase_rx0_rdvalid_cnt;
   int ase_rx0_wrvalid_cnt;
   int ase_rx0_umsgvalid_cnt;
   int ase_rx0_intrvalid_cnt;
   int ase_rx1_wrvalid_cnt;
   int ase_rx1_intrvalid_cnt;
   int ase_tx0_rdvalid_cnt;
   int ase_tx1_wrvalid_cnt;
   int ase_tx1_intrvalid_cnt;

   // int csr_write_enabled_cnt;

   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 ase_rx0_cfgvalid_cnt = 0;
	 ase_rx0_rdvalid_cnt = 0;
	 ase_rx0_wrvalid_cnt = 0;
	 ase_rx0_umsgvalid_cnt = 0;
	 ase_rx0_intrvalid_cnt = 0;
	 ase_rx1_wrvalid_cnt = 0;
	 ase_tx0_rdvalid_cnt = 0;
	 ase_tx1_wrvalid_cnt = 0;
      end
      else begin
	 // TX channels
	 if (rx_c0_cfgvalid) ase_rx0_cfgvalid_cnt	<= ase_rx0_cfgvalid_cnt + 1;
	 if (rx_c0_rdvalid)  ase_rx0_rdvalid_cnt	<= ase_rx0_rdvalid_cnt + 1;
	 if (rx_c0_wrvalid)  ase_rx0_wrvalid_cnt	<= ase_rx0_wrvalid_cnt + 1;
	 if (rx_c1_wrvalid)  ase_rx1_wrvalid_cnt	<= ase_rx1_wrvalid_cnt + 1;
	 // TX channels
	 if (tx_c0_rdvalid)  ase_tx0_rdvalid_cnt	<= ase_tx0_rdvalid_cnt + 1;
	 if (tx_c1_wrvalid)  ase_tx1_wrvalid_cnt	<= ase_tx1_wrvalid_cnt + 1;
      end
   end


   /*
    * This call is made on ERRORs requiring a shutdown
    * simkill is called from software, and is the final step before
    * graceful closedown
    */
   task simkill();
   // function void simkill();
      begin
	 $display("SIM-SV: Simulation kill command received...");
	 // Print transactions
	 `BEGIN_YELLOW_FONTCOLOR;
	 $display("Transaction counts => ");
	 $display("\tConfigs    = %d", ase_rx0_cfgvalid_cnt );
	 $display("\tRdReq      = %d", ase_tx0_rdvalid_cnt );
	 $display("\tRdResp     = %d", ase_rx0_rdvalid_cnt );
	 $display("\tWrReq      = %d", ase_tx1_wrvalid_cnt );
	 $display("\tWrResp-CH0 = %d", ase_rx0_wrvalid_cnt );
	 $display("\tWrResp-CH1 = %d", ase_rx1_wrvalid_cnt );
	 `END_YELLOW_FONTCOLOR;

	 // Valid Count
	 if (cfg.enable_asedbgdump) begin
	    // Print errors
	    `BEGIN_RED_FONTCOLOR;
	    if (ase_tx0_rdvalid_cnt != ase_rx0_rdvalid_cnt)
	      $display("\tREADs  : Response counts dont match request count !!");
	    if (ase_tx1_wrvalid_cnt != (ase_rx0_wrvalid_cnt + ase_rx1_wrvalid_cnt))
	      $display("\tWRITEs : Response counts dont match request count !!");
	    `END_RED_FONTCOLOR;
`ifdef ASE_DEBUG
 `ifdef ASE_RANDOMIZE_TRANSACTIONS
	    `BEGIN_YELLOW_FONTCOLOR;
	    $display("cf2as_latbuf_ch0 dropped =>");
	    $display(cci_emulator.cf2as_latbuf_ch0.checkunit.check_array);
	    $display("cf2as_latbuf_ch1 dropped =>");
	    $display(cci_emulator.cf2as_latbuf_ch1.checkunit.check_array);
	    `END_YELLOW_FONTCOLOR;
 `endif
`endif
	 end
	 $fclose(log_fd);
	 $finish;
      end
      //   endfunction
   endtask

   
   /*
    * Unified message watcher daemon
    */
   always @(posedge clk) begin : daemon_proc
      if (lp_initdone) begin
	 ase_listener();
      end
   end


   /* *******************************************************************
    * Staging incoming requests for TX0 and TX1 channels
    * - cf2as_latbuf_ch0
    * - cf2as_latbuf_ch1
    *
    * *******************************************************************/
   // CAFU->ASE CH0
   logic [`CCI_TX_HDR_WIDTH-1:0] cf2as_latbuf_ch0_header;
   logic 			 cf2as_latbuf_ch0_pop;
   logic 			 cf2as_latbuf_ch0_read;
   logic 			 cf2as_latbuf_ch0_empty;
   logic 			 cf2as_latbuf_ch0_empty_q;
   logic 			 cf2as_latbuf_ch0_valid;
   logic [31:0] 		 cf2as_latbuf_ch0_claddr;
   logic [13:0] 		 cf2as_latbuf_ch0_meta;

   // CAFU->ASE CH0
   logic [`CCI_TX_HDR_WIDTH-1:0] cf2as_latbuf_ch1_header;
   logic [`CCI_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data;
   logic [`CCI_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data_0;
   logic [`CCI_DATA_WIDTH-1:0] 	 cf2as_latbuf_ch1_data_1;
   logic 			 cf2as_latbuf_ch1_pop;
   logic 			 cf2as_latbuf_ch1_read_0;
   logic 			 cf2as_latbuf_ch1_read_1;
   logic 			 cf2as_latbuf_ch1_read;
   logic 			 cf2as_latbuf_ch1_empty;
   logic 			 cf2as_latbuf_ch1_empty_q;
   logic 			 cf2as_latbuf_ch1_valid;
   logic [31:0] 		 cf2as_latbuf_ch1_claddr;
   logic [31:0] 		 cf2as_latbuf_ch1_claddr_0;
   logic [31:0] 		 cf2as_latbuf_ch1_claddr_1;
   logic [13:0] 		 cf2as_latbuf_ch1_meta;
   logic [13:0] 		 cf2as_latbuf_ch1_meta_0;
   logic [13:0] 		 cf2as_latbuf_ch1_meta_1;

   
   // Both streams write back to CH0 at same time
   always @(posedge clk) begin
      if (cf2as_latbuf_ch1_read_0 && cf2as_latbuf_ch1_read_1) begin
	 `BEGIN_RED_FONTCOLOR;	 
	 $display ("*** ERROR: Both streams popped at the same time --- data may be lost");	 
	 start_simkill_countdown();
	 `END_RED_FONTCOLOR;	 
      end
   end   


   // CAFU->ASE CH0 (TX0)
   // Composed as {header, data}
`ifdef ASE_RANDOMIZE_TRANSACTIONS
   // Latency scoreboard (for latency modeling and shuffling)
   latency_scoreboard
     #(
       .NUM_TRANSACTIONS    (`LATBUF_NUM_TRANSACTIONS),
       .HDR_WIDTH           (`CCI_TX_HDR_WIDTH),
       .DATA_WIDTH          (`CCI_DATA_WIDTH),
       .COUNT_WIDTH         (`LATBUF_COUNT_WIDTH),
       .FIFO_FULL_THRESH    (`LATBUF_FULL_THRESHOLD),
       .FIFO_DEPTH_BASE2    (`LATBUF_DEPTH_BASE2)
       )
   cf2as_latbuf_ch0
     (
      .clk		( clk ),
      .rst		( ~sys_reset_n ),
      .meta_in		( tx_c0_header ),
      .data_in		( {`CCI_DATA_WIDTH{1'b0}} ),
      .write_en		( tx_c0_rdvalid ),
      .meta_out		( cf2as_latbuf_ch0_header ),
      .data_out		(  ),
      .valid_out	( cf2as_latbuf_ch0_valid ),
      .read_en		( cf2as_latbuf_ch0_pop ),
      .empty		( cf2as_latbuf_ch0_empty ),
      .full             ( tx_c0_almostfull ),
      .overflow         ( tx0_overflow ),
      .underflow        ( tx0_underflow ),
      .count            ( )
      );
`else // !`ifdef ASE_RANDOMIZE_TRANSACTIONS
   // FIFO (no randomization)
   ase_fifo
     #(
       .DATA_WIDTH     ( `CCI_TX_HDR_WIDTH ),
       .DEPTH_BASE2    ( `LATBUF_DEPTH_BASE2 ),
       .ALMFULL_THRESH ( `LATBUF_FULL_THRESHOLD )
       )
   cf2as_latbuf_ch0
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( tx_c0_rdvalid ),
      .data_in    ( tx_c0_header ),
      .rd_en      ( cf2as_latbuf_ch0_pop ),
      .data_out   ( cf2as_latbuf_ch0_header ),
      .data_out_v ( cf2as_latbuf_ch0_valid ),
      .alm_full   ( tx_c0_almostfull ),
      .full       ( ),
      .empty      ( cf2as_latbuf_ch0_empty ),
      .count      ( ),
      .overflow   ( tx0_overflow ),
      .underflow  ( tx0_underflow )
      );
`endif

   // POP CH0 staging
   assign cf2as_latbuf_ch0_pop = ~cf2as_latbuf_ch0_empty && cf2as_latbuf_ch0_read;

   always @(posedge clk)
     cf2as_latbuf_ch0_empty_q	<= cf2as_latbuf_ch0_empty;

   // Duplicate signals
   always @(*) begin
      cf2as_latbuf_ch0_claddr	<= cf2as_latbuf_ch0_header[`TX_CLADDR_BITRANGE];
      cf2as_latbuf_ch0_meta	<= cf2as_latbuf_ch0_header[`TX_MDATA_BITRANGE];
   end


   // CAFU->ASE CH1 (TX1)
`ifdef ASE_RANDOMIZE_TRANSACTIONS
   // Latency scoreboard (latency modeling and shuffling)
   latency_scoreboard
     #(
       .NUM_TRANSACTIONS (`LATBUF_NUM_TRANSACTIONS),
       .HDR_WIDTH        (`CCI_TX_HDR_WIDTH),
       .DATA_WIDTH       (`CCI_DATA_WIDTH),
       .COUNT_WIDTH      (`LATBUF_COUNT_WIDTH),
       .FIFO_FULL_THRESH (`LATBUF_FULL_THRESHOLD),
       .FIFO_DEPTH_BASE2 (`LATBUF_DEPTH_BASE2)
       )
   cf2as_latbuf_ch1
     (
      .clk		( clk ),
      .rst		( ~sys_reset_n ),
      .meta_in		( tx_c1_header ),
      .data_in		( tx_c1_data ),
      .write_en		( tx_c1_wrvalid ),
      .meta_out		( cf2as_latbuf_ch1_header ),
      .data_out		( cf2as_latbuf_ch1_data ),
      .valid_out	( cf2as_latbuf_ch1_valid ),
      .read_en		( cf2as_latbuf_ch1_pop ),
      .empty		( cf2as_latbuf_ch1_empty ),
      .full             ( tx_c1_almostfull ),
      .overflow         ( tx1_overflow ),
      .underflow        ( tx1_underflow ),
      .count            ( )
      );
`else // !`ifdef ASE_RANDOMIZE_TRANSACTIONS
   // FIFO (no shuffling, simple forwarding)
   ase_fifo
     #(
       .DATA_WIDTH     ( `CCI_TX_HDR_WIDTH + `CCI_DATA_WIDTH ),
       .DEPTH_BASE2    ( `LATBUF_DEPTH_BASE2 ),
       .ALMFULL_THRESH ( `LATBUF_FULL_THRESHOLD )
       )
   cf2as_latbuf_ch1
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( tx_c1_wrvalid ),
      .data_in    ( {tx_c1_header,tx_c1_data} ),
      .rd_en      ( cf2as_latbuf_ch1_pop ),
      .data_out   ( {cf2as_latbuf_ch1_header,cf2as_latbuf_ch1_data} ),
      .data_out_v ( cf2as_latbuf_ch1_valid ),
      .alm_full   ( tx_c1_almostfull ),
      .full       ( ),
      .empty      ( cf2as_latbuf_ch1_empty ),
      .count      ( ),
      .overflow   ( tx1_overflow ),
      .underflow  ( tx1_underflow )
      );
`endif

   // POP CH1 staging
   assign cf2as_latbuf_ch1_read = cf2as_latbuf_ch1_read_0 ^ cf2as_latbuf_ch1_read_1;
   assign cf2as_latbuf_ch1_pop = ~cf2as_latbuf_ch1_empty && cf2as_latbuf_ch1_read;

   always @(posedge clk)
     cf2as_latbuf_ch1_empty_q	<= cf2as_latbuf_ch1_empty;

   // Duplicating signals (DPI seems to cause errors in DEX function) --- P2 debug priority
   always @(*) begin
      cf2as_latbuf_ch1_claddr_1 <= cf2as_latbuf_ch1_header[`TX_CLADDR_BITRANGE];
      cf2as_latbuf_ch1_meta_1	<= cf2as_latbuf_ch1_header[`TX_MDATA_BITRANGE];
      cf2as_latbuf_ch1_data_1	<= cf2as_latbuf_ch1_data;
      cf2as_latbuf_ch1_claddr_0 <= cf2as_latbuf_ch1_header[`TX_CLADDR_BITRANGE];
      cf2as_latbuf_ch1_meta_0	<= cf2as_latbuf_ch1_header[`TX_MDATA_BITRANGE];
      cf2as_latbuf_ch1_data_0	<= cf2as_latbuf_ch1_data;
      cf2as_latbuf_ch1_claddr	<= cf2as_latbuf_ch1_header[`TX_CLADDR_BITRANGE];
      cf2as_latbuf_ch1_meta	<= cf2as_latbuf_ch1_header[`TX_MDATA_BITRANGE];
   end


   /*
    * Return response channel
    * PROBLEM: MUXing between channels 0 and 1 causes dropped transactions
    *          Replacing with FIFO doesnt seem to change occurance of problem
    *          Restricting write responses to TX1 seems to be a temporary solution
    *
    * DIVE:
    * - Problem seems to be when ch0_write gets dropped, conditions unknown
    */
   int 	 tx_to_rx_channel;

   // TX-CH1 must select RX-CH0 or RX-CH1 channels for fulfillment
   // Since requests on TX1 can return either via RX0 or RX1, this is needed
   // always @(posedge clk) begin
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 tx_to_rx_channel	<= 1;
      end
      // else if (~cf2as_latbuf_ch1_empty) begin
      else if (cf2as_latbuf_ch1_valid) begin
	 // tx_to_rx_channel	<= abs_val($random) % 2;
	 tx_to_rx_channel	<= 1;
	 // tx_to_rx_channel	<= 0;
      end
   end


   /* *******************************************************************
    * Response path management
    * - as2cf_fifo_ch0
    * - as2cf_fifo_ch1
    *
    * *******************************************************************/
   parameter int 		 ASE_RX0_PATHWIDTH = 5 + `CCI_RX_HDR_WIDTH + `CCI_DATA_WIDTH;
   parameter int 		 ASE_RX1_PATHWIDTH = 2 + `CCI_RX_HDR_WIDTH;

   logic [ASE_RX0_PATHWIDTH-1:0] as2cf_fifo_ch0_din;
   logic [ASE_RX0_PATHWIDTH-1:0] as2cf_fifo_ch0_dout;
   logic 			 as2cf_fifo_ch0_write;
   logic 			 as2cf_fifo_ch0_read;
   logic 			 as2cf_fifo_ch0_full;
   logic 			 as2cf_fifo_ch0_empty;
   logic 			 as2cf_fifo_ch0_overflow;
   logic 			 as2cf_fifo_ch0_underflow;
   logic 			 as2cf_fifo_ch0_valid;

   logic [ASE_RX1_PATHWIDTH-1:0] as2cf_fifo_ch1_din;
   logic [ASE_RX1_PATHWIDTH-1:0] as2cf_fifo_ch1_dout;
   logic 			 as2cf_fifo_ch1_write;
   logic 			 as2cf_fifo_ch1_read;
   logic 			 as2cf_fifo_ch1_full;
   logic 			 as2cf_fifo_ch1_empty;
   logic 			 as2cf_fifo_ch1_overflow;
   logic 			 as2cf_fifo_ch1_underflow;
   logic 			 as2cf_fifo_ch1_valid;


   // CH0 coded as {intrvalid, umsgvalid, wrvalid, rdvalid, cfgvalid, hdr, data}
   ase_fifo
     #(
       .DATA_WIDTH (ASE_RX0_PATHWIDTH)
       )
   as2cf_fifo_ch0
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( as2cf_fifo_ch0_write ),
      .data_in    ( as2cf_fifo_ch0_din ),
      .rd_en      ( as2cf_fifo_ch0_read ),
      .data_out   ( as2cf_fifo_ch0_dout ),
      .data_out_v ( as2cf_fifo_ch0_valid ),
      .alm_full   ( as2cf_fifo_ch0_full ),
      .full       ( ),
      .empty      ( as2cf_fifo_ch0_empty ),
      .count      ( ),
      .overflow   ( as2cf_fifo_ch0_overflow ),
      .underflow  ( as2cf_fifo_ch0_underflow )
      );

   // CH1 coded as {intrvalid, wrvalid, hdr}
   ase_fifo
     #(
       .DATA_WIDTH (ASE_RX1_PATHWIDTH)
       )
   as2cf_fifo_ch1
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( as2cf_fifo_ch1_write ),
      .data_in    ( as2cf_fifo_ch1_din ),
      .rd_en      ( as2cf_fifo_ch1_read ),
      .data_out   ( as2cf_fifo_ch1_dout ),
      .data_out_v ( as2cf_fifo_ch1_valid ),
      .alm_full   ( as2cf_fifo_ch1_full ),
      .full       ( ),
      .empty      ( as2cf_fifo_ch1_empty ),
      .count      ( ),
      .overflow   ( as2cf_fifo_ch1_overflow ),
      .underflow  ( as2cf_fifo_ch1_underflow )
      );

   // read control (no flow control on RX0 channels (pop when available)
   assign as2cf_fifo_ch0_read = ~as2cf_fifo_ch0_empty;
   assign as2cf_fifo_ch1_read = ~as2cf_fifo_ch1_empty;

   // RX0 channel
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 rx_c0_data		<= `CCI_DATA_WIDTH'b0;
	 rx_c0_header		<= `CCI_RX_HDR_WIDTH'b0;
	 rx_c0_cfgvalid		<= 0;
	 rx_c0_wrvalid		<= 0;
	 rx_c0_rdvalid		<= 0;
	 rx_c0_intrvalid	<= 0;
	 rx_c0_umsgvalid	<= 0;
      end
      else if (as2cf_fifo_ch0_valid) begin
	 rx_c0_data		<= as2cf_fifo_ch0_dout[`CCI_DATA_WIDTH-1:0];
	 rx_c0_header		<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH-1):`CCI_DATA_WIDTH];
	 rx_c0_cfgvalid		<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH)];
	 rx_c0_rdvalid		<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH+1)];
	 rx_c0_wrvalid		<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH+2)];
	 rx_c0_umsgvalid	<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH+3)];
	 rx_c0_intrvalid	<= as2cf_fifo_ch0_dout[(`CCI_DATA_WIDTH+`CCI_RX_HDR_WIDTH+4)];
      end
      else begin
	 rx_c0_data		<= 0;
	 rx_c0_header		<= 0;
	 rx_c0_cfgvalid		<= 0;
	 rx_c0_wrvalid		<= 0;
	 rx_c0_rdvalid		<= 0;
	 rx_c0_intrvalid	<= 0;
	 rx_c0_umsgvalid	<= 0;
      end
   end

   // RX1 channel
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 rx_c1_header		<= `CCI_RX_HDR_WIDTH'b0;
	 rx_c1_wrvalid		<= 1'b0;
	 rx_c1_intrvalid	<= 1'b0;
      end
      else if (as2cf_fifo_ch1_valid) begin
	 rx_c1_header		<= as2cf_fifo_ch1_dout[`CCI_RX_HDR_WIDTH-1:0];
	 rx_c1_wrvalid		<= as2cf_fifo_ch1_dout[`CCI_RX_HDR_WIDTH];
	 rx_c1_intrvalid	<= as2cf_fifo_ch1_dout[`CCI_RX_HDR_WIDTH+1];
      end
      else begin
	 rx_c1_header		<= 0; // as2cf_fifo_ch1_dout[`CCI_RX_HDR_WIDTH-1:0];
	 rx_c1_wrvalid		<= 1'b0;
	 rx_c1_intrvalid	<= 1'b0;
      end
   end


   /*
    * RX0 channel management
    */
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 csrff_read					<= 0;
	 as2cf_fifo_ch0_write				<= 0;
	 cf2as_latbuf_ch0_read				<= 0;
	 cf2as_latbuf_ch1_read_0			<= 0;
	 rx0_state					<= RxIdle;
      end
      else begin
	 csrff_read					<= 0;
	 umsgff_read <= 0;
	 as2cf_fifo_ch0_write				<= 0;
	 cf2as_latbuf_ch0_read				<= 0;
	 cf2as_latbuf_ch1_read_0			<= 0;
	 // CSR Write in AFU space
	 if (~csrff_empty && (csrff_dout[45:32] > CCI_AFU_LOW_OFFSET)) begin
	    as2cf_fifo_ch0_din				<= { 5'b00001, {`ASE_RX0_CSR_WRITE, csrff_dout[47:34]}, {480'b0, csrff_dout[31:0]}};
	    as2cf_fifo_ch0_write			<= csrff_valid;
	    csrff_read					<= 1;
	    rx0_state					<= RxAFUCSRWrite;
	    cf2as_latbuf_ch1_read_0			<= 0;
	    cf2as_latbuf_ch0_read			<= 0;
	 end
	 // CSR Write in QLP region
	 else if (~csrff_empty && (csrff_dout[45:32]	<= CCI_AFU_LOW_OFFSET)) begin
	    as2cf_fifo_ch0_write			<= 0;
	    if (csrff_dout[45:32]			<= CCI_AFU_LOW_OFFSET) begin
	       sw_reset_n				<= ~csrff_dout[CCI_RESET_CTRL_BITLOC];
	    end
	    csrff_read					<= 1;
	    rx0_state					<= RxQLPCSRWrite;
	    cf2as_latbuf_ch1_read_0			<= 0;
	    cf2as_latbuf_ch0_read			<= 0;
	 end
	 // UMSG Hint/Data
	 else if (~umsgff_empty) begin
	    as2cf_fifo_ch0_din <= { 5'b01000, umsgff_dout };
	    as2cf_fifo_ch0_write			<= umsgff_valid;
	    umsgff_read                    <= 1;
	    rx0_state                      <= RxUmsg;
	    cf2as_latbuf_ch1_read_0			<= 0;
	    cf2as_latbuf_ch0_read			<= 0;
	 end
	 // Read request
	 else if (~cf2as_latbuf_ch0_empty) begin
	    rd_memline_dex (rx0_pkt, cf2as_latbuf_ch0_claddr, cf2as_latbuf_ch0_meta );
	    as2cf_fifo_ch0_din				<= {5'b00010,
	    			   rx0_pkt.meta[`CCI_RX_HDR_WIDTH-1:0],
	    			   unpack_ccipkt_to_vector(rx0_pkt)};
	    csrff_read					<= 0;
	    as2cf_fifo_ch0_write			<= cf2as_latbuf_ch0_valid;
	    cf2as_latbuf_ch0_read			<= 1;
	    cf2as_latbuf_ch1_read_0			<= 0;
	    rx0_state					<= RxReadResp;
	 end
	 // Write request & RX0 is selected
	 else if (~cf2as_latbuf_ch1_empty && (tx_to_rx_channel == 0)) begin
	    wr_memline_dex(rx0_pkt,
	    		   cf2as_latbuf_ch1_claddr_0,
	    		   cf2as_latbuf_ch1_meta_0,
	    		   cf2as_latbuf_ch1_data_0 );
	    csrff_read					<= 0;
	    as2cf_fifo_ch0_din				<= {5'b00100, rx0_pkt.meta[`CCI_RX_HDR_WIDTH-1:0], 512'b0};
	    as2cf_fifo_ch0_write			<= cf2as_latbuf_ch1_valid;
	    cf2as_latbuf_ch1_read_0			<= 1;
	    cf2as_latbuf_ch0_read			<= 0;
	    rx0_state					<= RxWriteResp;
	 end
	 // Else
	 else begin
	    sw_reset_n					<= sw_reset_n_q;
	    csrff_read					<= 0;
	    as2cf_fifo_ch0_write			<= 0;
	    cf2as_latbuf_ch0_read			<= 0;
	    cf2as_latbuf_ch1_read_0			<= 0;
	    rx0_state					<= RxIdle;
	 end
      end
   end


   /*
    * RX1 channel management
    */
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 as2cf_fifo_ch1_write		<= 0;
	 cf2as_latbuf_ch1_read_1	<= 0;
	 rx1_state			<= RxIdle;
      end
      else begin
	 cf2as_latbuf_ch1_read_1	<= 0;
	 as2cf_fifo_ch1_write		<= 0;
	 // If Write Request was received & RX1 is selected
	 if (~cf2as_latbuf_ch1_empty && (tx_to_rx_channel == 1)) begin
	    wr_memline_dex(rx1_pkt,
	    		   cf2as_latbuf_ch1_claddr_1,
	    		   cf2as_latbuf_ch1_meta_1,
	    		   cf2as_latbuf_ch1_data_1 );
	    as2cf_fifo_ch1_din		<= { 2'b01, rx1_pkt.meta[`CCI_RX_HDR_WIDTH-1:0]};
	    as2cf_fifo_ch1_write	<= cf2as_latbuf_ch1_valid;
	    cf2as_latbuf_ch1_read_1	<= 1;
	    rx1_state			<= RxWriteResp;
	 end
	 else begin
	    cf2as_latbuf_ch1_read_1	<= 0;
	    as2cf_fifo_ch1_write	<= 0;
	    rx1_state			<= RxIdle;
	 end
      end
   end


   /* *******************************************************************
    * Inactivity management block
    *
    * DESCRIPTION: Running ASE simulations for too long can cause
    *              large dump-files to be formed. To prevent this, the
    *              inactivity counter will close down the simulation
    *              when CCI transactions are not seen for a long
    *              duration of time.
    *
    * This feature can be disabled, if desired.
    *
    * *******************************************************************/
   logic 	    first_transaction_seen = 0;
   logic [31:0]     inactivity_counter;
   logic 	    any_valid;
   logic 	    inactivity_found;


   // Inactivity management - Sense first transaction
   assign any_valid =    rx_c0_umsgvalid
			 || tx_c1_intrvalid_sel
			 || rx_c0_intrvalid
			 || rx_c1_intrvalid
			 || rx_c0_wrvalid
			 || rx_c0_rdvalid
			 || rx_c0_cfgvalid
			 || rx_c1_wrvalid
			 || tx_c0_rdvalid
			 || tx_c1_wrvalid ;


   // Check for first transaction
   always @(posedge clk, any_valid)
     begin
	if(any_valid) begin
	   first_transaction_seen	<= 1'b1;
	end
     end

   // Inactivity management - killswitch
   always @(posedge clk) begin
      if((inactivity_found==1'b1) && (cfg.enable_timeout != 0)) begin
	 $display("SIM-SV: Inactivity timeout reached !!\n");
	 start_simkill_countdown();
      end
   end

   // Inactivity management - counter
   counter
     #(
       .COUNT_WIDTH (32)
       )
   inact_ctr
     (
      .clk          (clk),
      .rst          ( first_transaction_seen && any_valid ),
      .cnt_en       (1'b1),
      .load_cnt     (32'b0),
      .max_cnt      (cfg.enable_timeout),
      .count_out    (inactivity_counter),
      .terminal_cnt (inactivity_found)
      );


   /* ****************************************************************
    * Initialising the CAFU here.
    * If SPL2 is enabled, SPL top is mapped
    * If CCI is enabled, cci_std_afu.sv is mapped
    *
    * ****************************************************************
    *
    *              ASE   |             |   CAFU or (SPL + AFU)
    *                  TX|------------>|RX
    *                    |             |
    *                  RX|<------------|TX
    *                    |             |
    *
    * ***************************************************************/
   cci_std_afu cci_std_afu (
			    /* Link/Protocol (LP) clocks and reset */
			    .vl_clk_LPdomain_32ui             ( clk_32ui ),
			    .vl_clk_LPdomain_16ui             ( clk_16ui ),
			    .ffs_vl_LP32ui_lp2sy_InitDnForSys ( lp_initdone ),
			    .ffs_vl_LP32ui_lp2sy_SystemReset_n( sys_reset_n ),
			    .ffs_vl_LP32ui_lp2sy_SoftReset_n  ( sw_reset_n ),
			    /* Channel 0 can receive READ, WRITE, WRITE CSR responses.*/
			    .ffs_vl18_LP32ui_lp2sy_C0RxHdr    ( rx_c0_header ),
			    .ffs_vl512_LP32ui_lp2sy_C0RxData  ( rx_c0_data ),
			    .ffs_vl_LP32ui_lp2sy_C0RxWrValid  ( rx_c0_wrvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxRdValid  ( rx_c0_rdvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxCgValid  ( rx_c0_cfgvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxUgValid  ( rx_c0_umsgvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxIrValid  ( rx_c0_intrvalid ),
			    /* Channel 1 reserved for WRITE RESPONSE ONLY */
			    .ffs_vl18_LP32ui_lp2sy_C1RxHdr    ( rx_c1_header ),
			    .ffs_vl_LP32ui_lp2sy_C1RxWrValid  ( rx_c1_wrvalid ),
			    .ffs_vl_LP32ui_lp2sy_C1RxIrValid  ( rx_c1_intrvalid ),
			    /*Channel 0 reserved for READ REQUESTS ONLY */
			    .ffs_vl61_LP32ui_sy2lp_C0TxHdr    ( tx_c0_header ),
			    .ffs_vl_LP32ui_sy2lp_C0TxRdValid  ( tx_c0_rdvalid ),
			    /*Channel 1 reserved for WRITE REQUESTS ONLY */
			    .ffs_vl61_LP32ui_sy2lp_C1TxHdr    ( tx_c1_header ),
			    .ffs_vl512_LP32ui_sy2lp_C1TxData  ( tx_c1_data ),
			    .ffs_vl_LP32ui_sy2lp_C1TxWrValid  ( tx_c1_wrvalid ),
			    .ffs_vl_LP32ui_sy2lp_C1TxIrValid  ( tx_c1_intrvalid ),
			    /* Tx push flow control */
			    .ffs_vl_LP32ui_lp2sy_C0TxAlmFull  ( tx_c0_almostfull ),
			    .ffs_vl_LP32ui_lp2sy_C1TxAlmFull  ( tx_c1_almostfull )
			    );



   /*
    * Initialization procedure
    *
    * DESCRIPTION: This procedural block is called when ./simv is
    *              kicked off, helps put the simulation in a known
    *              state.
    *
    * STEPS:
    * - Print startup info
    * - Send initial system reset, cleaning up state machines
    * - Initialize ASE (ase_init executes in SW)
    *   - Set up message queues for IPC (done in SW)
    *   - Set up memory management structure (called in SW)
    * - If ENABLED, start the CA-private memory region (emulated with
    *   software
    * - Then set up the QLP InitDone signal to go indicate readiness
    * - SIMULATION is ready to begin
    *
    */
   initial begin : ase_entry_point
      $display("SIM-SV: Simulator started...");
      // Initialize data-structures
      csr_write_dispatch(1, 0, 0);
      umsg_dispatch(1, 0, 0, 0, 0);
      ase_init();

      // Initial signal values *FIXME*
      $display("SIM-SV: Sending initial reset...");
      sys_reset_n = 0;
      #100ns;
      sys_reset_n = 1;
      #100ns;

      // Setting up CA-private memory
      if (cfg.enable_capcm) begin
	 $display("SIM-SV: Enabling structures for CA Private Memory... ");
	 capcm_init();
      end

      // Link layer ready signal
      wait (lp_initdone == 1'b1);
      $display("SIM-SV: CCI InitDone is HIGH...");

      // Indicate to APP that ASE is ready
      ase_ready();

   end


   /*
    * Latency pipe : For LP_InitDone delay
    * This block simulates the latency between a generic reset and QLP
    * InitDone
    */
   latency_pipe
     #(
       .NUM_DELAY (`LP_INITDONE_READINESS_LATENCY),
       .PIPE_WIDTH (1)
       )
   lp_initdone_lat
     (
      .clk (clk),
      .rst (~sys_reset_n),
      .pipe_in (sys_reset_n),
      .pipe_out (lp_initdone)
      );


   /*
    * CCI rule-checker function
    * This block of code exists for checking incoming signals for 'X' & 'Z'
    * Warning messages will be flashed, and simulation exited, when enabled
    */
   // Used for rule-checking meta-only transactions
   logic [`CCI_DATA_WIDTH-1:0] 		  zero_data = `CCI_DATA_WIDTH'b0;
   logic 				  tx0_rc_error;
   logic 				  tx1_rc_error;
   logic 				  rx0_rc_error;
   logic 				  rx1_rc_error;
   int 					  tx0_rc_time;
   int 					  tx1_rc_time;
   int 					  rx0_rc_time;
   int 					  rx1_rc_time;


   // Initial message
   initial begin
      if (cfg.enable_ccirules) begin
	 $display("SIM-SV: CCI Signal rule-checker is watching for 'X' and 'Z'");
      end
   end

   // CCI Rules Checker: Checking CCI for 'X' and 'Z' endorsed by valid signal
   cci_rule_checker
     #(
       .TX_HDR_WIDTH (`CCI_TX_HDR_WIDTH),
       .RX_HDR_WIDTH (`CCI_RX_HDR_WIDTH),
       .DATA_WIDTH   (`CCI_DATA_WIDTH)
       )
   cci_rule_checker
     (
      // Enable
      .enable          (cfg.enable_ccirules[0]),
      // CCI signals
      .clk             (clk),
      .resetb          (sys_reset_n),
      .lp_initdone     (lp_initdone),
      .tx_c0_header    (tx_c0_header),
      .tx_c0_rdvalid   (tx_c0_rdvalid),
      .tx_c1_header    (tx_c1_header),
      .tx_c1_data      (tx_c1_data),
      .tx_c1_wrvalid   (tx_c1_wrvalid),
      .tx_c1_intrvalid (tx_c1_intrvalid_sel ),
      .rx_c0_header    (rx_c0_header),
      .rx_c0_data      (rx_c0_data),
      .rx_c0_rdvalid   (rx_c0_rdvalid),
      .rx_c0_wrvalid   (rx_c0_wrvalid),
      .rx_c0_cfgvalid  (rx_c0_cfgvalid),
      .rx_c1_header    (rx_c1_header),
      .rx_c1_wrvalid   (rx_c1_wrvalid),
      // Error signals
      .tx_ch0_error    (tx0_rc_error),
      .tx_ch1_error    (tx1_rc_error),
      .rx_ch0_error    (rx0_rc_error),
      .rx_ch1_error    (rx1_rc_error),
      .tx_ch0_time     (tx0_rc_time),
      .tx_ch1_time     (tx1_rc_time),
      .rx_ch0_time     (rx0_rc_time),
      .rx_ch1_time     (rx1_rc_time)
      );

   // Interrupt select (enables
   assign tx_c1_intrvalid_sel = cfg.enable_intr ? tx_c1_intrvalid : 1'b0 ;


   // Call simkill on bad outcome of checker process
   task xz_simkill(int sim_time) ;
      begin
   	 `BEGIN_RED_FONTCOLOR;
   	 $display("SIM-SV: ASE has detected 'Z' or 'X' were qualified by a valid signal.");
   	 $display("SIM-SV: Check simulation around time, t = %d", sim_time);
   	 // $display("SIM-SV: Simulation will end now");
   	 // $display("SIM-SV: If 'X' or 'Z' are intentional, set ENABLE_CCI_RULES to '0' in ase.cfg file");
   	 `END_RED_FONTCOLOR;
   	 // start_simkill_countdown();
      end
   endtask

   // Flow simkill
   task flowerror_simkill(int sim_time, int channel) ;
      begin
	 `BEGIN_RED_FONTCOLOR;
	 $display("SIM-SV: ASE has detected a possible OVERFLOW or UNDERFLOW error.");
	 $display("SIM-SV: Check simulation around time, t = %d in Channel %d", sim_time, channel);
   	 $display("SIM-SV: Simulation will end now");
	 `END_RED_FONTCOLOR;
      end
   endtask

   // Almostfull disobedience warning
   // always @(posedge clk) begin
   //    if ( tx_c0_almostfull && tx_c0_rdvalid ) begin
   // 	 `BEGIN_YELLOW_FONTCOLOR;
   // 	 $display ("SIM-SV: t = ", $time, " => *** TX-CH0 almostfull was HIGH and READ request was seen !! ***");
   // 	 `END_YELLOW_FONTCOLOR;
   //    end
   //    if ( tx_c1_almostfull && tx_c1_wrvalid ) begin
   // 	 `BEGIN_YELLOW_FONTCOLOR;
   // 	 $display ("SIM-SV: t = ", $time, " => *** TX-CH1 almostfull was HIGH and WRITE request was seen !! ***");
   // 	 `END_YELLOW_FONTCOLOR;
   //    end
   // end

   // Flow error messages
   always @(posedge clk) begin
      if (tx0_overflow) begin
	 flowerror_simkill($time, 0);
      end
      if (tx0_underflow) begin
	 flowerror_simkill($time, 0);
      end
      if (tx1_overflow) begin
	 flowerror_simkill($time, 1);
      end
      if (tx1_underflow) begin
	 flowerror_simkill($time, 1);
      end
   end

   // Watch checker signal
   always @(posedge clk) begin
      if (tx0_rc_error) begin
   	 xz_simkill(tx0_rc_time);
      end
      else if (tx1_rc_error) begin
   	 xz_simkill(tx1_rc_time);
      end
      else if (rx0_rc_error) begin
   	 xz_simkill(rx0_rc_time);
      end
      else if (rx1_rc_error) begin
   	 xz_simkill(rx1_rc_time);
      end
   end


   /*
    * ASE Hardware Interface (CCI) logger
    * - Logs CCI transaction into a transactions.tsv file
    * - Watch for "*valid", and write transaction to log name
    */
   // Log file descriptor
   int log_fd;

   // Registers for comparing previous states
   always @(posedge clk) begin
      lp_initdone_q	<= lp_initdone;
      resetb_q		<= resetb;
      sw_reset_n_q	<= sw_reset_n;
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
	 if (resetb_q != resetb) begin
	    $fwrite(log_fd, "%d\tResetb toggled from %b to %b\n", $time, resetb_q, resetb);
	 end
	 // Watch CCI for valid transactions
	 if (lp_initdone) begin
	    ////////////////////////////// RX0 cfgvalid /////////////////////////////////
	    if (rx_c0_cfgvalid) begin
	       $fwrite(log_fd, "%d\tCSRWrite\t\t\t%x\t%x\n", $time, rx_c0_header[`RX_CSR_BITRANGE], rx_c0_data[31:0]);
	       if (cfg.enable_cl_view) $display("%d\tCSRWrite\t\t\t%x\t%x", $time, rx_c0_header[`RX_CSR_BITRANGE], rx_c0_data[31:0]);
	    end
	    /////////////////////////////// RX0 wrvalid /////////////////////////////////
	    if (rx_c0_wrvalid) begin
	       $fwrite(log_fd, "%d\tWrResp\t\t0\t%x\tNA\tNA\n", $time, rx_c0_header[`RX_MDATA_BITRANGE] );
	       if (cfg.enable_cl_view) $display("%d\tWrResp\t\t0\t%x\tNA\tNA", $time, rx_c0_header[`RX_MDATA_BITRANGE] );
	    end
	    /////////////////////////////// RX0 rdvalid /////////////////////////////////
	    if (rx_c0_rdvalid) begin
	       $fwrite(log_fd, "%d\tRdResp\t\t0\t%x\tNA\t%x\n", $time, rx_c0_header[`RX_MDATA_BITRANGE], rx_c0_data );
	       if (cfg.enable_cl_view) $display("%d\tRdResp\t\t0\t%x\tNA\t%x", $time, rx_c0_header[`RX_MDATA_BITRANGE], rx_c0_data );
	    end
	    ////////////////////////////// RX0 umsgvalid ////////////////////////////////
	    if (rx_c0_umsgvalid) begin
	       if (rx_c0_header[`CCI_UMSG_BITINDEX]) begin              // Umsg Hint
		  $fwrite(log_fd, "%d\tUmsgHint\t0\n", $time );
		  if (cfg.enable_cl_view) $display("%d\tUmsgHint\t\t0\n", $time );
	       end
	       else begin                                               // Umsg with data
		  $fwrite(log_fd, "%d\tUmsg    \t0\t%x\n", $time, rx_c0_data );
		  if (cfg.enable_cl_view) $display("%d\tUmsgHint\t\t0\t%x\n", $time, rx_c0_data );
	       end
	    end
	    /////////////////////////////// RX1 wrvalid /////////////////////////////////
	    if (rx_c1_wrvalid) begin
	       $fwrite(log_fd, "%d\tWrResp\t\t1\t%x\tNA\tNA\n", $time, rx_c1_header[`RX_MDATA_BITRANGE] );
	       if (cfg.enable_cl_view) $display("%d\tWrResp\t\t1\t%x\tNA\tNA", $time, rx_c1_header[`RX_MDATA_BITRANGE] );
	    end
	    /////////////////////////////// TX0 rdvalid /////////////////////////////////
	    if (tx_c0_rdvalid) begin
	       if ((tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_S) || (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE)) begin
		  $fwrite(log_fd, "%d\tRdLineReq_S\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (cfg.enable_cl_view) $display("%d\tRdLineReq_S\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else if (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_I) begin
		  $fwrite(log_fd, "%d\tRdLineReq_I\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (cfg.enable_cl_view) $display("%d\tRdLineReq_I\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else if (tx_c0_header[`TX_META_TYPERANGE] == `ASE_TX0_RDLINE_O) begin
		  $fwrite(log_fd, "%d\tRdLineReq_O\t0\t%x\t%x\tNA\n", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
		  if (cfg.enable_cl_view) $display("%d\tRdLineReq_O\t0\t%x\t%x\tNA", $time, tx_c0_header[`TX_MDATA_BITRANGE], tx_c0_header[45:14]);
	       end
	       else begin
		  $fwrite(log_fd, "ReadValid on TX-CH0 validated an UNKNOWN Request type at t = %d \n", $time);
	       end
	    end
	    /////////////////////////////// TX1 wrvalid /////////////////////////////////
	    if (tx_c1_wrvalid) begin
	       if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRTHRU) begin
		  $fwrite(log_fd, "%d\tWrThruReq\t1\t%x\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
		  if (cfg.enable_cl_view) $display("%d\tWrThruReq\t1\t%x\t%x\t%x", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
	       end
	       else if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRLINE) begin
		  $fwrite(log_fd, "%d\tWrLineReq\t1\t%x\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
		  if (cfg.enable_cl_view) $display("%d\tWrLineReq\t1\t%x\t%x\t%x", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14], tx_c1_data);
	       end
	       else if (tx_c1_header[`TX_META_TYPERANGE] == `ASE_TX1_WRFENCE) begin
		  $fwrite(log_fd, "%d\tWrFence\t1\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14]);
		  if (cfg.enable_cl_view) $display("%d\tWrFence\t1\t%x\t%x\n", $time, tx_c1_header[`TX_MDATA_BITRANGE], tx_c1_header[45:14]);
	       end
	       else begin
		  $fwrite(log_fd, "WriteValid on TX-CH1 validated an UNKNOWN Request type at t = %d \n", $time);
		  if (cfg.enable_cl_view) $display("WriteValid on TX-CH1 validated an UNKNOWN Request type at t = %d \n", $time);
	       end
	    end
	 end
	 // Wait till next clock
	 @(posedge clk);
      end
   end


endmodule // cci_emulator
