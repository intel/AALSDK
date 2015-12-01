/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Description:
//  	Single dual port memory
// 
// Author:  Yaping Liu
// Date:    2/10/2011
// 
// $Id: sim_sp_mem.v 13882 2014-07-14 21:25:52Z yliu9 $
/////////////////////////////////////////////////////////////////////////////////////////////////////////


module sim_sp_mem #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 8
) (
    input  wire                     clk,
    input  wire                     we,     
    input  wire [ADDR_WIDTH-1:0]    addr,
    input  wire [DATA_WIDTH-1:0]    din,
    output reg  [DATA_WIDTH-1:0]    dout
);


//  (* RAM_STYLE="{AUTO | BLOCK |  BLOCK_POWER1 | BLOCK_POWER2}" *)
`ifdef VENDOR_XILINX
    (* ram_extract = "yes", ram_style = "block" *)
    reg  [DATA_WIDTH-1:0]         mem[2**ADDR_WIDTH-1:0];
`else
    reg  [DATA_WIDTH-1:0]         mem[2**ADDR_WIDTH-1:0];
`endif

    always @(posedge clk) begin
        if (we)
            mem[addr] <= din;                    
        dout <= mem[addr];
    end
			
endmodule

