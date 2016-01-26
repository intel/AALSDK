`include "ase_global.vh"

module prng();

   int i;
   int rand_out;
    
   initial begin
      #100;
      for(i = 0; i < 10; i = i + 1) begin
	 rand_out = latency_scoreboard.get_random_delay(4'h4);
	 // rand_out = $urandom_range (3, 15);	 
	 $display(i , rand_out);	 
	 #10;	 
      end
      $finish;      
   end
   
endmodule
