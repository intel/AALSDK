import ccip_if_pkg::*;

module simple_afu (
   clk,                 // interface clock
   rst,                 // reset (active high)

   cp2af_sRxPort, // CCIP RX port
   af2cp_sTxPort  // CCIP TX port
);

// Specify input and output signals
input                 clk;
input                 rst;
input  t_if_ccip_Rx   cp2af_sRxPort;
output t_if_ccip_Tx   af2cp_sTxPort;

// Define CSRs
reg [1:0]   rStatus;       // status register
reg [31:0]  rCounter;      // counter (32 bits should be good for a few seconds)

// Cast c0 hdr into ReqMmioHdr
t_ccip_c0_ReqMmioHdr mmioHdr;
assign mmioHdr = t_ccip_c0_ReqMmioHdr'(cp2af_sRxPort.c0.hdr);

// To implement MMIO CSRs, we need to listen for incoming MMIO requests
// on RX Channel 0 and respond to reads on TX Channel 2

// Implement two MMIO registers:
//    CTL (0x00)  Control register (RW). Bits 1:0 control counter: 
//                                              "00": don't count, 
//                                              "01" count up,
//                                              "10" count down
//    CNT (0x04)  Counter register (RW). Actual counter value.
always @(posedge clk)
begin
   if (rst) begin 
      rStatus          <= '0;
      rCounter         <= '0;
      af2cp_sTxPort.c2 <= '0;
   end else begin

      // complete previous transaction, if necessary
      af2cp_sTxPort.c2.mmioRdValid  <= 0;

      // incremet/decrement counter
      case (rStatus)
         2'b01 : rCounter <= rCounter + 1'b1;
         2'b10 : rCounter <= rCounter - 1'b1;
      endcase

      // on MMIO Write Request, set registers
      if (cp2af_sRxPort.c0.mmioWrValid == 1) begin               // MMIO Write
         case (mmioHdr.address)
            16'h0000 : rStatus  <= cp2af_sRxPort.c0.data[1:0];
            16'h0001 : rCounter <= cp2af_sRxPort.c0.data[31:0];
         endcase
      end

      // on MMIO Read Request, return current counter
      if (cp2af_sRxPort.c0.mmioRdValid == 1) begin               // MMIO Read
         af2cp_sTxPort.c2.hdr.tid      <= mmioHdr.tid;           // copy TID
         case (mmioHdr.address)
            16'h0000 : af2cp_sTxPort.c2.data <= { 62'b0, rStatus[1:0] };    // return status -OR-
            16'h0001 : af2cp_sTxPort.c2.data <= { 32'b0, rCounter[31:0] };  // return counter
         endcase
         af2cp_sTxPort.c2.mmioRdValid  <= 1;                     // post response
      end

   end
end

// Tie all other Tx channels to 0
assign af2cp_sTxPort.c0 = '0;
assign af2cp_sTxPort.c1 = '0;

endmodule

