module tb_async_channel();

   logic wrclk, rdclk;

   initial begin
      wrclk = 0;      
      forever begin
	 #5;
	 wrclk = ~wrclk;	 
      end
   end

   initial begin
      rdclk = 0;
      forever begin
	 #10;
	 rdclk = ~rdclk;	 
      end
   end

   logic rst;
   logic [31:0] data_in;
   logic 	write_in;
   logic [31:0] data_out;
   logic 	full;
      
   afifo_channel #(32, 5)
   afifo_channel (rst, wrclk, rdclk, data_in, write_in, data_out, full, , );
   
   logic 	start;
   int 		wr_i;

   parameter MAX_ITEMS = 32;

   always @(posedge wrclk) begin
      if (rst) begin
	 write_in <= 0;
	 wr_i     <= 0;	 
      end
      else if (start && (wr_i < MAX_ITEMS) & ~full) begin
	 data_in  <= 32'hCAFE_0000 + wr_i; 
	 write_in <= 1;
	 wr_i     <= wr_i + 1;	 
      end
      else begin
	 write_in <= 0;	 
      end
   end
     
   
   initial begin
      rst = 1;
      start = 0;      
      #100;
      rst = 0;
      #100;
      start = 1;
      #2000;
      $finish;      
   end

   
endmodule
