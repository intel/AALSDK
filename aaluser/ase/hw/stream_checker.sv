import ase_pkg::*;

`include "platform.vh"

module stream_checker
  #(
    parameter int HDR_WIDTH     = CCIP_TX_HDR_WIDTH,
    parameter int TID_WIDTH     = 32
    )
   (
    input logic 		clk,
    input logic 		valid_in,
    input TxHdr_t               hdr_in,
    input logic [TID_WIDTH-1:0] tid_in,
    input logic 		valid_out,
    input TxHdr_t               txhdr_out,
    input RxHdr_t               rxhdr_out,
    input logic [TID_WIDTH-1:0] tid_out    
    );

   longint 			check_array[*];
   
   always @(posedge clk) begin
      if (valid_in) begin
	 if ( (hdr_in.reqtype == ASE_RDLINE_I)||(hdr_in.reqtype == ASE_RDLINE_S) ) begin
	    for(int ii = 0; ii <= hdr_in.len; ii = ii + 1) begin
	       check_array[ {tid_in, ii[1:0]} ] = hdr_in;	    
	    end
	 end
	 // else if (hdr_in.reqtype != ASE_WRFENCE) begin
	 else begin
	    check_array[{tid_in, hdr_in.mdata}] = hdr_in;
	 end
      end
      if (valid_out) begin
	 if (check_array.exists({tid_out, rxhdr_out.clnum}) ) begin
	    check_array.delete({tid_out, rxhdr_out.clnum});
	 end
	 else begin
	    `BEGIN_RED_FONTCOLOR;
	    $display("%m (%d) => tid = %x, meta = %x was not found in checker memory !!", $time, tid_out, txhdr_out);
	    `END_RED_FONTCOLOR;
	 end
      end      
   end
   
endmodule

