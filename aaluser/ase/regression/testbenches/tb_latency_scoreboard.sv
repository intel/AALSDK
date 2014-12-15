module tb_latency_scoreboard();

   parameter DATA_WIDTH = 64;
   parameter HDR_WIDTH = 64;
      
   logic clk, rst, full, empty, valid_in, valid_out, read_en;     
   logic [DATA_WIDTH-1:0] data_in, data_out;
   logic [HDR_WIDTH-1:0]  meta_in, meta_out;
   int 			  ii;
   logic 		  overflow, underflow;
   
      
   latency_scoreboard #(4, HDR_WIDTH, DATA_WIDTH, 8, 5, 3) 
   buffer (clk, rst, meta_in, data_in, valid_in, meta_out, data_out, valid_out, read_en, empty, full, overflow, underflow);

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
	 data_in <= 0;	 
	 meta_in <= 64'h22222222_22222222;	 
      end
      else begin
	 if ((~full) && (wr_iter < 5)) begin
	    wr_iter <= wr_iter + 1;
	    meta_in <= meta_in + 64'h11111111_11111111;
	    valid_in <= 1;	    
	 end
	 else begin
	    valid_in <= 0;	    
	 end 
      end
   end

   assign read_en = 0;   

   initial begin
      #2000;
      $finish;
      
   end
   
   // testbench
   // initial begin
   //    valid_in = 1'b0;
   //    meta_in = 64'h0000000000000000;
   //    data_in = 64'h0000000000000000;      
   //    rst = 0;
   //    read_en = 0;
      
   //    #20;
   //    rst = 1;
   //    #100;
   //    rst = 0;
   //    #40;
   //    wait (full == 0);
   //    @(posedge clk);
   //    @(posedge clk);
   //    @(posedge clk);
   //    @(posedge clk);
      
   //    // {data_in, meta_in} = {64'hcafebabecafeba00, 64'hcafebabecafeba00};
   //    // valid_in = 0;
   //    // @(posedge clk);
   //    // @(posedge clk);
   //    // @(posedge clk);
   //    // @(posedge clk);
   //    // {data_in, meta_in} = {64'hcafebabecafeba01, 64'hcafebabecafeba01};
   //    // valid_in = 1;
   //    // @(posedge clk);
   //    // {data_in, meta_in} = {64'h5555555555555555, 64'h5555555555555555};
   //    // valid_in = 1;
   //    // @(posedge clk);
   //    // {data_in, meta_in} = {64'hcafebabecafeba02, 64'hcafebabecafeba02};
   //    // valid_in = 1;
   //    // @(posedge clk);
   //    // {data_in, meta_in} = {64'hcafebabecafeba03, 64'hcafebabecafeba03};
   //    // valid_in = 1;
   //    // @(posedge clk);
   //    // {data_in, meta_in} = {64'h0000000000000000, 64'h0000000000000000};
   //    // valid_in = 0;
      
      
   //    meta_in = 64'h3333333333333333;
   //    data_in = 64'h3333333333333333;
   //    for(ii = 0; ii < 5;) begin
   //    	 if (full)	   
   //    	   valid_in = 0;
   //    	 else begin
   //    	    valid_in = 1;
   //    	    meta_in = meta_in + 64'h1111111111111111 ;
   //    	    data_in = data_in + 64'h1111111111111111 ;	    
   //    	    ii = ii + 1;
   //    	    $display("Push word");	 
   //    	 end	 
   //    	 @(posedge clk);	 
   //    end
   //     valid_in = 1'b0;
   //     meta_in = 64'h0000000000000000;
   //     data_in = 64'h0000000000000000;
   //    @(posedge clk);
      
   //    #1000;      
   //    $finish;      
   // end
   
      
endmodule
