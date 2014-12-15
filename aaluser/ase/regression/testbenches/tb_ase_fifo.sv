module tb_ase_fifo();

   parameter DATA_WIDTH = 64;
   parameter MAX_COUNT = 10;

   logic clk, rst, full, empty, valid_in, valid_out, read_en;
   logic [DATA_WIDTH-1:0] data_in, data_out;
   logic [3:0] 		  count;
   logic 		  start_reading;

   int 			  wr_iter;
   int 			  rd_iter;

    ase_fifo inst_fifo (clk, rst, valid_in, data_in, read_en, data_out, valid_out, full, , empty, count, , );
<<<<<<< HEAD
=======
//   sbv_gfifo #(64, 3, 0, 5) inst_fifo (~rst, clk, data_in, valid_in, , data_out, valid_out, empty, , , full );
  
   // initial $vcdpluson;
>>>>>>> 05c4bb7fac7b761fadb4b03b29677ab78d274e90

   //clk
   initial begin
      clk = 1;
      forever begin
	 #5;
	 clk = ~clk;
      end
   end

   // Reset
   initial begin
      rst = 1;
      #100;
      rst = 0;
   end

   // simkill
   initial begin
      wait (wr_iter >= MAX_COUNT);
      #400;
      $finish;
   end

   // Enable reads
   initial begin
      start_reading = 0;
      #400;
      start_reading = 1;
   end

<<<<<<< HEAD
   // Write process
=======
   
   int ii;
   int jj;
   
>>>>>>> 05c4bb7fac7b761fadb4b03b29677ab78d274e90
   always @(posedge clk) begin
      if (rst) begin
	 valid_in <= 0;
	 wr_iter <= 0;
      end
      else begin
	 if ((~full) && (wr_iter <= MAX_COUNT))  begin
	    wr_iter <= wr_iter + 1;
	    valid_in <= 1;
	    data_in <= 64'hCAFEBABE_00000000 + wr_iter;
	 end
	 else begin
	    valid_in <= 0;
	 end
      end
   end

   // Print process
   always @(posedge clk) begin
      if (valid_in)  $display ("WRITE => %d | %x", wr_iter, data_in);
      if (valid_out) $display ("READ  => %d | %x", rd_iter, data_out );
   end

   // Read process
   logic read_trig;
   
   always @(posedge clk) begin
      if (rst) begin
   	 read_trig <= 0;
   	 rd_iter <= 0;
      end
      else begin
   	 if (~empty && start_reading) begin
   	    read_trig <= 1;
   	    rd_iter <= rd_iter + 1;
   	 end
   	 else begin
   	    read_trig <= 0;
   	 end
      end
   end
<<<<<<< HEAD
   
   assign read_en = ~empty && read_trig;


   initial begin
      #4000;
      $finish;
   end

=======
  
   // Push
   // initial begin
   //    data_in = 64'h00000000_00000000;
   //    valid_in = 0;      
   //    wait (rst != 1);
   //    #100;
   //    @(posedge clk);
   //    data_in = 64'hCAFEBABE_DECAFBAD;
   //    valid_in = 1;
   //    @(posedge clk);
   //    valid_in = 0;
   //    #200;
   // end // initial begin

   // // Pop
   // initial begin
   //    read_en = 0;      
   //    wait (empty != 1);
   //    @(posedge clk);
   //    // read_en = 1;
   //    $display("Ready to pop ?", $time);
   // end
   
>>>>>>> 05c4bb7fac7b761fadb4b03b29677ab78d274e90
endmodule
