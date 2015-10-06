`include "ase_global.vh"
`include "platform.vh"

module inorder_wrf_channel
  #(
    parameter int HDR_WIDTH = 61,
    parameter int DATA_WIDTH = 512,
    parameter int FIFO_FULL_THRESH = 5,
    parameter int FIFO_DEPTH_BASE2 = 3,
    parameter int VISIBLE_DEPTH_BASE2 = 7,
    parameter int VISIBLE_FULL_THRESH = 96
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

   parameter int 		  TID_WIDTH = 32;
   parameter int 		  FIFO_WIDTH = TID_WIDTH + HDR_WIDTH + DATA_WIDTH;

   // Tracking ID
   logic [TID_WIDTH-1:0] 	  tid_in;
   logic [TID_WIDTH-1:0] 	  tid_out;

   // Tagging process
   always @(posedge clk) begin
      if (rst)
	tid_in	<= {TID_WIDTH{1'b0}};
      else if (write_en)
	tid_in	<= tid_in + 1;
   end

   logic [HDR_WIDTH-1:0] 	  inff_meta;
   logic [DATA_WIDTH-1:0] 	  inff_data;
   logic [TID_WIDTH-1:0] 	  inff_tid;
   logic 			  inff_valid;
   logic 			  inff_empty;
   logic 			  inff_pop;
   logic [VISIBLE_DEPTH_BASE2:0]  inff_count;

   logic 			  assert_wrfence;
   logic 			  wrfence_pop;

   logic [FIFO_WIDTH-1:0] 	  outff_din;
   logic 			  outff_wen;
   logic 			  outff_full;
   logic 			  outff_empty;
   logic [FIFO_DEPTH_BASE2:0] 	  outff_count;
   
   /*
    * In-FIFO
    * Visible depth emulation, and hold until logic
    */ 
   // In-FIFO
   ase_fifo
     #(
       .DATA_WIDTH     (FIFO_WIDTH),
       .DEPTH_BASE2    (VISIBLE_DEPTH_BASE2),
       .ALMFULL_THRESH (VISIBLE_FULL_THRESH)
       )
   infifo
     (
      .clk        (clk),
      .rst        (rst),
      .wr_en      (write_en),
      .data_in    ({tid_in, meta_in, data_in}),
      .rd_en      (inff_pop),
      .data_out   ({inff_tid, inff_meta, inff_data}),
      .data_out_v (inff_valid),
      .alm_full   (full),
      .full       (),
      .empty      (inff_empty),
      .count      (inff_count),
      .overflow   (inff_overflow),
      .underflow  (inff_underflow)
      );

   assign inff_pop = ~inff_empty && ((~assert_wrfence && ~outff_full) || (assert_wrfence && wrfence_pop));
   
   always @(posedge clk) begin
      outff_din <= {inff_tid, inff_meta, inff_data};   
      outff_wen <= inff_valid && ~assert_wrfence && ~outff_full;
   end
   
   // Assert WriteFence
   always @(*) begin
      if (~inff_empty && (inff_meta[`TX_META_TYPERANGE]==`ASE_TX1_WRFENCE))
	assert_wrfence	<= 1;
      else
	assert_wrfence	<= 0;
   end   

   /*
    * Wrfence filter & jamming logic
    */ 
   typedef enum {NoWrfencePassThru, WrFenceWaiting, WaitState} WriteFence_checker;
   WriteFence_checker wrf_state;

   // WriteFence passthru/trap FSM
   always @(posedge clk) begin
      if (rst) begin
	 wrf_state		<= NoWrfencePassThru;
	 wrfence_pop		<= 0;	 
      end
      begin
	 case (wrf_state)
	   NoWrfencePassThru:
	     begin
		wrfence_pop	<= 0;		
		if (assert_wrfence) begin
		   wrf_state	<= WrFenceWaiting;		   
		end
		else begin
		   wrf_state	<= NoWrfencePassThru;		   
		end
	     end

	   WrFenceWaiting:
	     begin
		if (outff_empty) begin
		   wrfence_pop	<= 1;
		   wrf_state	<= WaitState;		     
		end
		else begin
		   wrfence_pop	<= 0;	
		   wrf_state	<= WrFenceWaiting;		     
		end
	     end

	   WaitState:
	     begin
		wrfence_pop	<= 0;		  
		wrf_state	<= NoWrfencePassThru;		  
	     end

	   default:
	     begin
		wrfence_pop	<= 0;		  
		wrf_state	<= NoWrfencePassThru;		  
	     end
	 endcase
      end
   end

   /*
    * Out-FIFO
    * Pop out transaction
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
      .wr_en      (outff_wen),
      .data_in    (outff_din),
      .rd_en      (read_en),
      .data_out   ({tid_out, meta_out, data_out}),
      .data_out_v (valid_out),
      .alm_full   (outff_full),
      .full       (),
      .empty      (outff_empty),
      .count      (outff_count),
      .overflow   (outff_overflow),
      .underflow  (outff_underflow)
      );

   
   assign empty = outff_empty;
   assign count = inff_count + outff_count;  
   assign overflow = inff_overflow || outff_overflow ;
   assign underflow = inff_underflow || outff_underflow ;

   /*
    * Transaction IN-OUT checker
    * Sniffs dropped transactions
    */
`ifdef ASE_DEBUG
   stream_checker #(HDR_WIDTH, TID_WIDTH)
   checkunit (clk, write_en, meta_in, tid_in, valid_out, meta_out, tid_out);   
`endif  
   
endmodule   
