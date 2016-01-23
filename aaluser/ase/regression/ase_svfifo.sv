/*
 * Test: SV FIFO using queue
 */


/*
 * Systemverilog registered FIFO
 */ 
module svfifo
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
      data_out_v <= rd_en && ~empty;
      overflow   <= full && wr_en;
      underflow  <= empty && rd_en;
   end

   always @(*) begin : empty_proc
      if (count == 0)
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

/*
 * SvFIFO + show-ahead
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

   logic [DATA_WIDTH-1:0] 	  data_out_tmp;
   logic 			  data_out_v_tmp;
   logic 			  empty_tmp;
   logic 			  rd_en_tmp;
     
   svfifo #(DATA_WIDTH, DEPTH_BASE2, ALMFULL_THRESH)
   svfifo (clk, rst, wr_en, data_in, 
	   rd_en_tmp, data_out_tmp, data_out_v_tmp, 
	   alm_full, full, empty_tmp, count, overflow, underflow);
  
  
   assign rd_en_tmp = ~empty_tmp && (~data_out_v_tmp || rd_en);
   assign empty = ~data_out_v;

   always @(posedge clk) begin
      if (rst)
	data_out_v <= 0;
      else begin
	 if (rd_en_tmp)
	   data_out_v <= 1;
	 else if (rd_en)
	   data_out_v <= 0;	 
      end
   end
   
endmodule
