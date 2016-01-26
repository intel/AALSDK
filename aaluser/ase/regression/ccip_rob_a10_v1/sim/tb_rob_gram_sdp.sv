module tb_rob_gram_sdp();

   logic clk;
   logic re, we;
   logic [3:0] raddr, waddr;
   logic [31:0] din, dout;
   
   rob_gram_sdp #(4, 32)
   ram (clk, we, waddr, din, re, raddr, dout);

   
   initial begin
      clk = 0;      
      forever begin
	 #5;
	 clk = ~clk;	 
      end
   end

   logic [3:0] wr_i, rd_i;

   logic tb_wren, tb_rden;
   
   
   // Write process
   always @(posedge clk) begin
      if (tb_wren) begin
	 we <= 1;
	 waddr <= wr_i;
	 din   <= 32'hCAFE_0000 + wr_i;
	 wr_i  <= wr_i + 1;	 
      end
      else begin
	 wr_i <= 0;	 
	 we <= 0;	 
      end
   end
   
   // Begin reading
   always @(posedge clk) begin
      if (tb_rden) begin
	 re <= 1;
	 raddr <= rd_i;
	 rd_i <= rd_i + 1;	 
      end
      else begin
	 rd_i <= 0;	 
	 re <= 0;	 
      end
   end
   
   // TB
   initial begin
      tb_wren = 0;
      tb_rden = 0;
      #100;
      @(posedge clk);      
      tb_wren = 1;
      wait (wr_i == 15);
      @(posedge clk);
      tb_wren = 0;
      #100;
      tb_rden = 1;
      #200;
      $finish;      
   end
   
endmodule