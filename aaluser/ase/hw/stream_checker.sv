import ase_pkg::*;

module stream_checker
  #(
    parameter int HDR_WIDTH = CCIP_TX_HDR_WIDTH,
    parameter int TID_WIDTH = 32
    )
   (
    input logic 		clk,
    input logic 		valid_in,
    input logic [HDR_WIDTH-1:0] meta_in,
    input logic [TID_WIDTH-1:0] tid_in,
    input logic 		valid_out,
    input logic [HDR_WIDTH-1:0] meta_out,
    input logic [TID_WIDTH-1:0] tid_out    
    );

   int 			  check_array[*];
   
   always @(posedge clk) begin
      if (valid_in && (meta_in[`TX_META_TYPERANGE] != CCIP_TX1_WRFENCE)) begin
	 check_array[tid_in] = meta_in;	 
      end
      if (valid_out) begin
	 if (check_array.exists(tid_out)) begin
	    check_array.delete(tid_out);
	 end
	 else begin
	    `BEGIN_RED_FONTCOLOR;
	    $display("%m (%d) => tid = %x, meta = %x was not found in checker memory !!", $time, tid_out, meta_out);
	    `END_RED_FONTCOLOR;
	 end
      end      
   end
   
endmodule
