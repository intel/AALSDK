import ase_pkg::*;

`include "platform.vh"

module tb_channel();

   logic rst, clk, wr_en, rd_en, empty;
   TxHdr_t hdr_in;

   TxHdr_t txhdr_out;
   RxHdr_t rxhdr_out;
   logic valid_out;
   logic [CCIP_DATA_WIDTH-1:0] data_out;

   localparam MAX_ITEMS = 256;
      
   outoforder_wrf_channel
     #(
       .UNROLL_ENABLE (1)
       )
   outoforder_wrf_channel
     (clk, rst, hdr_in, 512'b0, wr_en, txhdr_out, rxhdr_out, data_out, valid_out, rd_en, empty, full );

   initial begin
      clk = 0;
      forever begin
	 clk = ~clk;
	 #10;
      end
   end

   int wr_i;
   int x;
      
   initial begin
      rst <= 1;
      #200;
      rst <= 0;
      wait (wr_i == MAX_ITEMS);
      #50000;
      `ifdef ASE_DEBUG
      $display("------- Dropped Transactions -------");
      //      $display(tb_channel.outoforder_wrf_channel.checkunit.check_array);
      if (tb_channel.outoforder_wrf_channel.checkunit.check_array.first(x)) begin
	 do begin	    
	    $display("%x : %x", (x >> 2), tb_channel.outoforder_wrf_channel.checkunit.check_array[x]); 
	 end while (tb_channel.outoforder_wrf_channel.checkunit.check_array.next(x));
      end
      $display("Mismatch Count =%d", tb_channel.outoforder_wrf_channel.checkunit.check_array.num() );      
      
      `endif
      $finish;
   end

   // channel select
   function logic [1:0] sel_rand_len ();
      logic [1:0] rand_ch;      
      begin
	 do begin	    
	    rand_ch = $random % 4;
	 end while (rand_ch == 2'b10); // UNMATCHED !!
	 return rand_ch;	 
      end
   endfunction

   // Push process
   always @(posedge clk) begin
      if (rst) begin
	 wr_en <= 0;
	 wr_i  <= 0;
      end
      // else if (~full && ($time % 7 == 0) && (wr_i < MAX_ITEMS)) begin
      // 	 hdr_in.vc <= VC_VA; // ccip_vc_t'($random) % 4;
      // 	 hdr_in.sop <= 1;
      // 	 hdr_in.len <= ASE_1CL;
      // 	 hdr_in.reqtype <= ASE_WRFENCE;
      // 	 hdr_in.addr <= 0;	 
      // 	 hdr_in.mdata <= wr_i;
      // 	 wr_en <= 1;
      // 	 hdr_in.rsvd70 <= 0;
      // 	 hdr_in.rsvd63_58 <= 0;
      // 	 $display("Wrfence asserted, mdata = %x", wr_i);
      // 	 wr_i <= wr_i + 1;	 
      // end 
      else if (~full && (wr_i < MAX_ITEMS)) begin
	 hdr_in.vc <= ccip_vc_t'($random) % 4;
	 hdr_in.sop <= 1;
	 hdr_in.len <= ASE_1CL; // sel_rand_len();	 
	 hdr_in.reqtype <= ASE_RDLINE_I; // ASE_WRLINE_I;
	 hdr_in.addr <= 32'h8400_0000 + wr_i;
	 hdr_in.mdata <= wr_i;
	 wr_i      <= wr_i + 1;
	 wr_en <= 1;
	 hdr_in.rsvd70 <= 0;
	 hdr_in.rsvd63_58 <= 0;
	 $display("Hdr Push %d", wr_i);	 
      end
      else begin
	 wr_en <= 0;
      end
   end

   int rd_i;
      
   always @(posedge clk) begin
      if (rst)
	rd_en <= 0;
      else if (~empty) 
      	rd_en <= 1;
      else
	rd_en <= 0;      
   end
   
   always @(posedge clk) begin
      if (rst) begin
	 rd_i <= 0;	 
      end
      else if (valid_out) begin
	 rd_i <= rd_i + 1;	 
      end
   end
   

endmodule
