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
   logic [DEPTH_BASE2:0] 	  count_reg;
   
   logic [DATA_WIDTH-1:0] 	  my_queue[$:FIFO_DEPTH-1];
  
   always @(posedge clk) begin : wr_proc
      if (wr_en) begin
   	 my_queue.push_back(data_in);
      end
   end

   always @(posedge clk) begin : rd_proc
      if (rd_en) begin
   	 data_out <= my_queue.pop_front();
      end
   end
      
   // always @(posedge clk) begin : ctr_proc
   always @(*) begin : ctr_proc
      if (rst) begin
   	 count <= 0;	 
      end
      else begin
   	 case ({wr_en, rd_en})
   	   2'b10   : count <= count_reg + 1;
   	   2'b01   : count <= count_reg - 1;
   	   default : count <= count_reg ;	   
   	 endcase // case ({wr_en, rd_en})
      end
   end 

   int cnt;
   always @(posedge clk)
     cnt <= my_queue.size();
   
   
   always @(posedge clk)
     count_reg <= count;
   
   
   always @(posedge clk) begin : status_proc
      data_out_v <= rd_en && ~empty;
      overflow   <= full && wr_en;
      underflow  <= empty && rd_en;
   end

   always @(*) begin : empty_proc
      if (count <= 0)
   	empty <= 1;
      else
   	empty <= 0;
   end

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

endmodule
