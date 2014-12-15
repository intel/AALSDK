module tb_shuffle_ram();

   logic [31:0] din, dout;
   logic [3:0] 	waddr, raddr;
   logic 	clk, we;

   parameter int MAX = 2^4;
      
   shuffle_ram
     #(
       .BUS_SIZE_ADDR (4),
       .BUS_SIZE_DATA (32)       
       )
   ram_inst     
     (
      .clk   (clk),
      .we    (we),
      .waddr (waddr),
      .din   (din),
      .raddr (raddr),
      .dout  (dout),
      );
   
   
   
   
endmodule
