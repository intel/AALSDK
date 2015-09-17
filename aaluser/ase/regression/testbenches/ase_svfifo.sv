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
   
   logic [DATA_WIDTH-1:0] 	  data_out_tmp;

   always @(posedge clk) begin
      if (wr_en) begin
   	 my_queue.push_back(data_in);
      end
   end

   // always @(posedge clk) begin
   //    if (rd_en) begin
   // 	 data_out_tmp <= my_queue.pop_front();
   //    end
   // end

   always @(posedge rd_en) begin
      data_out <= my_queue.pop_front(); 
   end
      
   always @(posedge clk) begin
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
   
   always @(*) begin
      // data_out   <= my_queue[0];
      data_out_v <= rd_en && ~empty;
      overflow   <= full && wr_en;
      underflow  <= empty && rd_en;
      // count      <= my_queue.PtrSize();      
   end

   always @(*) begin
      if (count == 0)
   	empty <= 1;
      else
   	empty <= 0;
   end

   always @(*) begin
      if (count == FIFO_DEPTH)
   	full <= 1;
      else
   	full <= 0;
   end

   always @(*) begin
      if (count >= ALMFULL_THRESH)
   	alm_full <= 1;
      else
   	alm_full <= 0;
   end

endmodule
