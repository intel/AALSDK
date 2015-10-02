/*
 * Test: SV FIFO using queue
 */

module ase_svfifo
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

   localparam FIFO_DEPTH = 2**DEPTH_BASE2;

   logic [DATA_WIDTH-1:0] 	  my_queue[$:FIFO_DEPTH-1];
   logic 			  internal_rden;
   logic 			  internal_rden_reg;
   logic [DATA_WIDTH-1:0] 	  data_pipeout;
   logic 			  data_pipeout_v;
   logic 			  fifo_empty;


   always @(posedge clk) begin : wr_proc
      if (wr_en) begin
   	 my_queue.push_back(data_in);
      end
   end

   // always @(posedge clk) begin : rd_proc
   //    if (rd_en) begin
   // 	 data_out <= my_queue.pop_front();
   //    end
   // end

   always @(posedge clk) begin : int_rd_proc
      if (internal_rden) begin
	 data_pipeout <= my_queue.pop_front();
      end
   end

   // Pipeout_valid
   always @(posedge clk) begin
      if (rst) begin
	 data_pipeout_v <= 0;	 
      end
      else begin
	 if (internal_rden)
	   data_pipeout_v <= 1;
	 else
	   data_pipeout_v <= 0;
      end
   end

   always @(posedge clk) begin : ctr_proc
      if (rst) begin
   	 count <= 0;
      end
      else begin
   	 case ({wr_en, rd_en})
   	   2'b10   : count <= count + 1;
   	   2'b01   : count <= count - 1;
   	   default : count <= count ;
   	 endcase // case ({wr_en, rd_en})
      end
   end

   always @(posedge clk) begin : status_proc
      internal_rden_reg <= internal_rden;
      // data_out_v <= rd_en && ~empty;
      overflow   <= full && wr_en;
      underflow  <= empty && rd_en;
   end

   always @(*) begin : fifoempty_proc
      if (count == 0) begin
   	 fifo_empty <= 1;
      end
      else begin
   	 fifo_empty <= 0;
      end
   end

   assign empty = fifo_empty && ~data_pipeout_v;

   always @(*) begin : full_proc
      if (count == FIFO_DEPTH)
   	full <= 1;
      else
   	full <= 0;
   end

   always @(*) begin : almfull_proc
      if (count >= ALMFULL_THRESH)
   	alm_full <= 1;
      else
   	alm_full <= 0;
   end

   assign data_out = data_pipeout;
   assign data_out_v = data_pipeout_v && rd_en;

   always @(posedge clk) begin
      if (rst) begin
	 internal_rden <= 0;	 
      end
      else begin
	 case ({rd_en, empty})
	   2'b00: internal_rden <= 0;
	   2'b01: internal_rden <= 1;
	   2'b10: internal_rden <= 1;
	   2'b11: internal_rden <= 0;
	   default: internal_rden <= 0;
	 endcase // case ({rd_en, empty})
      end
   end

endmodule
