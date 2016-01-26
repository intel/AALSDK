import ase_pkg::*;

module tb_latency_scoreboard();

   parameter DATA_WIDTH = 64;
   parameter HDR_WIDTH = 80;
      
   logic clk, rst, full, empty, valid_in, valid_out, read_en;     
   logic [DATA_WIDTH-1:0] data_in, data_out;
   logic [HDR_WIDTH-1:0]  meta_in, meta_out;
   int 			  ii;
   logic 		  overflow, underflow;
   
   // Buffer
   outoforder_wrf_channel
     #(
       .NUM_TRANSACTIONS (4),
       .HDR_WIDTH  (HDR_WIDTH), 
       .DATA_WIDTH (DATA_WIDTH)
       ) 
   buffer (clk, rst, meta_in, data_in, valid_in, meta_out, data_out, valid_out, read_en, empty, full, overflow, underflow, );

   //clk
   initial begin
      clk = 0;      
      forever begin
	 #5;
	 clk = ~clk;	 
      end
   end

   logic start_reading;
   initial begin
      rst = 1;
      start_reading = 0;      
      #400;
      rst = 0;
      #600;
      start_reading = 0;      
   end

   int wr_iter;
   
   always @(posedge clk) begin
      if (rst) begin
	 valid_in <= 0;
	 wr_iter <= 0;
	 data_in <= 64'hCAFEBABE_BEBAFECA;	 
	 meta_in <= 72'h00_00000000_00000000;	 
      end
      else begin
	 if ((~full) && (wr_iter < 256)) begin
	    $display(wr_iter);	    
	    wr_iter <= wr_iter + 1;
	    meta_in <= meta_in + 72'h01_00000000_00000000;
	    valid_in <= 1;	    
	 end
	 else begin
	    valid_in <= 0;	    
	 end 
      end
   end

   assign read_en = ~empty;   

   
   // int checker_meta[*];
   
   initial begin
      #5000;
// `ifdef ASE_DEBUG
//      $display (buffer.checkunit.check_array);
// `endif
      $finish;     
   end

      
endmodule
