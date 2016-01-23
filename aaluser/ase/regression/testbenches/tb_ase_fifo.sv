module tb_ase_fifo();

   parameter DATA_WIDTH = 65536;
   parameter MAX_COUNT = 256;

   logic clk, rst, full, empty, valid_in, valid_out, read_en;
   logic [DATA_WIDTH-1:0] data_in, data_out;
   logic [3:0] 		  count;
   logic 		  start_reading;

   int 			  wr_iter;
   int 			  rd_iter;

   ase_svfifo inst_fifo (clk, rst, valid_in, data_in, read_en, data_out, valid_out, full, , empty, count, , );

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
   // initial begin
   //    wait (wr_iter >= MAX_COUNT);
   //    #400;
   //    $finish;
   // end

   // Enable reads
   initial begin
      start_reading = 0;
      #400;
      start_reading = 1;
   end

   int ii;
   int jj;

   always @(posedge clk) begin
      if (rst) begin
	 valid_in <= 0;
	 wr_iter <= 0;
      end
      else begin
	 if ((~full) && (wr_iter < MAX_COUNT) )  begin
	    // wr_iter <= wr_iter + 1;
	    valid_in <= 1;
	    data_in <= 64'hCAFEBABE_00000000 + wr_iter;
	    wr_iter <= wr_iter + 1;      
	 end
	 else begin
	    valid_in <= 0;
	 end
      end
   end

   // always @(posedge clk) begin
   //    if (rst)
   // 	wr_iter <= 0;
   //    else if (valid_in)
   // 	wr_iter <= wr_iter + 1;      
   // end
   
   // print process
   always @(posedge clk) begin
      if (valid_in)  $display ("WRITE => %d | %x", wr_iter, data_in);
      if (valid_out) $display ("READ  => %d | %x", rd_iter, data_out );
   end

   // Read process
   logic read_trig;

   // always @(posedge clk) begin
   //    if (rst) begin
   // 	 // read_trig <= 0;
   // 	 read_en <= 0;
   // 	 // rd_iter <= 0;
   //    end
   //    else begin
   // 	 if (~empty && start_reading) begin
   // 	    read_en <= 1;
   // 	 end
   // 	 else begin
   // 	    read_en <= 0;
   // 	 end
   //    end      
   // end

   always @(posedge clk) begin
     if (rst)
       rd_iter <= 0;
     else if (valid_out)
       rd_iter <= rd_iter + 1;
   end
   
   always @(posedge clk)
     read_en <= ~empty;
// && read_trig;


   initial begin
      // #1000_00;
      wait until (rd_iter == MAX_COUNT);
      #200;		  
      $display(check_array);
      $display(rd_iter);      
      $finish;
   end

   // checker array
   int unsigned check_array[*];
   always @(posedge clk) begin : check_proc
      if (valid_in) begin
	 if (check_array.exists(wr_iter)) begin
	    $display("ERROR: wr_iter = %d found in check_array\n", wr_iter);
	 end
	 else begin
	    check_array[wr_iter] = data_in;
	 end
      end
      if (valid_out) begin
	 if (check_array.exists(rd_iter)) begin
	    check_array.delete(rd_iter);
	 end
	 else begin
	    $display("ERROR: rd_iter = %d not found\n", rd_iter);
	 end
      end
   end

endmodule
