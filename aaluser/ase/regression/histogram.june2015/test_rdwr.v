// ***************************************************************************
//
//        Copyright (C) 2008-2013 Intel Corporation All Rights Reserved.
//
// Engineer:            Narayanan Ravichandran, Pratik Marolia
// Create Date:         Thu Nov 20 11:28:01 PDT 2014
// Module Name:         test_rdwr.v
// Project:             mb_2_3 AFU 
// Description:         Read Streaming
//
// ***************************************************************************
// ---------------------------------------------------------------------------------------------------------------------------------------------------
//                                         Read Streaming test 
//  ------------------------------------------------------------------------------------------------------------------------------------------------
// 

module test_rdwr #(parameter PEND_THRESH=1, ADDR_LMT=20, MDATA=14)
(
   //---------------------------global signals-------------------------------------------------
   Clk_32UI,                // input -- Core clock
   Resetb,                  // input -- Use SPARINGLY only for control
                   
   rw2ab_RdAddr,            // output   [ADDR_LMT-1:0]
   rw2ab_RdTID,             // output   [13:0]
   rw2ab_RdEn,              // output 
   ab2rw_RdSent,            // input
   
   ab2rw_RdRspValid,        // input                    
   ab2rw_RdRsp,             // input    [13:0]          
   ab2rw_RdRspAddr,         // input    [ADDR_LMT-1:0]  
   ab2rw_RdData,            // input    [511:0]         
   
   re2xy_go,                // input                 
   re2xy_NumLines,          // input    [31:0]            
   re2xy_Cont,              // input             
    
   rw2ab_TestCmp,           // output           
   test_Resetb              // input            
);

   input                      Clk_32UI;               // csi_top:    Clk_32UI
   input                      Resetb;                 // csi_top:    system Resetb
   
   output   [ADDR_LMT-1:0]    rw2ab_RdAddr;           // arb:        Reads may yield to writes
   output   [13:0]            rw2ab_RdTID;            // arb:        meta data
   output                     rw2ab_RdEn;             // arb:        read enable
   input                      ab2rw_RdSent;           // arb:        read issued
   
   input                      ab2rw_RdRspValid;       // arb:        read response valid
   input    [13:0]            ab2rw_RdRsp;            // arb:        read response header
   input    [ADDR_LMT-1:0]    ab2rw_RdRspAddr;        // arb:        read response address
   input    [511:0]           ab2rw_RdData;           // arb:        read data
   
   input                      re2xy_go;               // requestor:  start of frame recvd
   input    [31:0]            re2xy_NumLines;         // requestor:  number of cache lines
   input                      re2xy_Cont;             // requestor:  continuous mode
   
   output                     rw2ab_TestCmp;          // arb:        Test completion flag
   input                      test_Resetb;
   
   reg      [ADDR_LMT-1:0]    rw2ab_RdAddr;           // arb:        Reads may yield to writes
   wire     [13:0]            rw2ab_RdTID;            // arb:        meta data
   wire                       rw2ab_RdEn;             // arb:        read enable
   reg                        rw2ab_TestCmp;          // arb:        Test completion flag
   
   //Local Variables
   reg      [31:0]            Num_RdReqs;			  // No. of Rds issued			
   reg      [31:0]            Num_RdPend;			  // No. of Rds waiting for response 
   reg      [1:0]             RdFSM;				  // Read FSM states
   reg      [MDATA-4:0]       Rdmdata;				  // Read Meta-data 
   
   assign rw2ab_RdTID = {6'b100000,Rdmdata};		  // To track response rw2ab_RdTID[13] set to 1
   assign rw2ab_RdEn  = (RdFSM == 2'h1) && re2xy_go;    

   always @(posedge Clk_32UI)
   begin
     if (!test_Resetb)
       begin
         rw2ab_TestCmp    <= 0;
       end
     else
       begin   
         rw2ab_TestCmp    <= (RdFSM == 2'h2 && Num_RdPend == 0);
       end
   end
   
   always @(posedge Clk_32UI)
   begin
         case(RdFSM)       /* synthesis parallel_case */
         2'h0:
         begin
           rw2ab_RdAddr   <= 0;
           Num_RdReqs     <= 0;

           if(re2xy_go)								 
             begin
               if (re2xy_NumLines!=0)
                 RdFSM   <= 2'h1;
               else
                 RdFSM   <= 2'h2;						// Done State (default)
             end
         end
                     
         2'h1:
         begin
           if(ab2rw_RdSent)
             begin
               rw2ab_RdAddr        <= rw2ab_RdAddr + 1'b1;
               Num_RdReqs          <= Num_RdReqs + 1'b1;        
               
               if(Num_RdReqs >= re2xy_NumLines-1)
                 if(re2xy_Cont)							// cont mode - Goto beginning 
                   RdFSM     <= 2'h0;			
                 else									// Goto End	
                   RdFSM     <= 2'h2;
             end
         end
         
         default:
         begin
           RdFSM     <= RdFSM;							// RdFSM = Done
         end
         endcase         

         if ((rw2ab_RdEn && ab2rw_RdSent))
           Rdmdata           <= Rdmdata + 1'b1;			// update m-data for each read 

         if  ((rw2ab_RdEn && ab2rw_RdSent) && !ab2rw_RdRspValid)
           Num_RdPend        <= Num_RdPend + 1'b1;
         else if(!(rw2ab_RdEn && ab2rw_RdSent) &&  ab2rw_RdRspValid && ((rw2ab_TestCmp == 1'b0)))
           Num_RdPend        <= Num_RdPend - 1'b1;
           
       if (!test_Resetb)								// Reset condition
       begin
         rw2ab_RdAddr   <= 0;
         Rdmdata        <= 0;
         Num_RdReqs     <= 0;
         Num_RdPend     <= 0;
         RdFSM          <= 0;
       end   
   end
endmodule


