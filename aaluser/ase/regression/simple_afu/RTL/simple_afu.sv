import ccip_if_pkg::*;

module simple_afu (
   clk,                 // interface clock
   rst,                 // reset (active high)

   cp2af_sRxPort, // CCIP RX port
   af2cp_sTxPort  // CCIP TX port
);

input clk;
input rst;

// Register all input and output signals
input   t_if_ccip_Rx   cp2af_sRxPort;
output  t_if_ccip_Tx   af2cp_sTxPort;

// Define counter CSR
reg [31:0]  rCounter;      // counter (32 bits should be good for a few seconds)

// Cast c0 hdr into ReqMmioHdr
t_ccip_c0_ReqMmioHdr mmioHdr;
assign mmioHdr = t_ccip_c0_ReqMmioHdr'(cp2af_sRxPort.c0.hdr);

// To implement MMIO CSRs, we need to listen for incoming MMIO requests
// on RX Channel 0 and respond to reads on TX Channel 2

// Be lazy and react to ALL MMIO addresses
always @(posedge clk)
begin
   if (rst) begin 
      rCounter                     <= 32'b0;
      af2cp_sTxPort.c2.mmioRdValid <= 0;
      af2cp_sTxPort.c2.hdr.tid     <= 9'b0;
      af2cp_sTxPort.c2.data        <= 64'b0;
   end else begin
      if (cp2af_sRxPort.c0.mmioWrValid == 1) begin               // MMIO Write
         rCounter <= cp2af_sRxPort.c0.data[31:0];
      end else begin
         rCounter <= rCounter - 1'b1;
      end
      if (cp2af_sRxPort.c0.mmioRdValid == 1) begin               // MMIO Read
         af2cp_sTxPort.c2.hdr.tid      <= mmioHdr.tid;           // copy TID
         af2cp_sTxPort.c2.data         <= { 32'b0, rCounter };   // return data
         af2cp_sTxPort.c2.mmioRdValid  <= 1;                     // post response
      end else begin
         af2cp_sTxPort.c2.mmioRdValid  <= 0;
      end
   end
end


// Tie all other Tx channels to 0
//assign cp2af_sTxPort.c0....

endmodule

