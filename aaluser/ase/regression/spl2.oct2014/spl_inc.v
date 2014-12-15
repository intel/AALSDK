/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Description:
//  	SPL incrementer
// 
// Author:  Yaping Liu
// Date:    9/18/2011
// 
// $Id: spl_inc.v 8844 2011-10-11 17:48:23Z yliu9 $
/////////////////////////////////////////////////////////////////////////////////////////////////////////


module spl_inc #(
    parameter WIDTH = 64
) (
    input  wire                 clk,        
    input  wire                 reset_n,       
    input  wire                 en,
    input  wire                 load,
    input  wire [WIDTH - 1:0]   din,
    output reg  [WIDTH - 1:0]   dout
);

    reg                         cnt;
    reg  [WIDTH/2:0]            sum1;
    wire [WIDTH - 1:0]          sum;
    
        
    assign sum[WIDTH/2 - 1:0] = sum1[WIDTH/2 - 1:0];
    assign sum[WIDTH - 1:WIDTH/2] = dout[WIDTH - 1:WIDTH/2] + sum1[WIDTH/2];
           
    always @(posedge clk) begin
        if (load) begin
            dout <= din;
        end            
        else begin
            if (en) dout <= sum;
        end
        
        sum1[WIDTH/2:0] <= dout[WIDTH/2 - 1:0] + 1'b1;   
    end

endmodule

