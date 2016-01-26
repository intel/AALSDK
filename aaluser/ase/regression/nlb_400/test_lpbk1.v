// ***************************************************************************
//
//        Copyright (C) 2008-2013 Intel Corporation All Rights Reserved.
//
// Engineer:            Pratik Marolia
// Create Date:         Thu Jul 28 20:31:17 PDT 2011
// Module Name:         test_lpbk1.v
// Project:             NLB AFU 
// Description:         memory copy test
//
// ***************************************************************************
// ---------------------------------------------------------------------------------------------------------------------------------------------------
//                                         Loopback 1- memory copy test
//  ------------------------------------------------------------------------------------------------------------------------------------------------
//
// This is a memory copy test. It copies cache lines from source to destination buffer.
//

module test_lpbk1 #(parameter PEND_THRESH=1, ADDR_LMT=20, MDATA=14)
(

//      ---------------------------global signals-------------------------------------------------
       Clk_16UI               ,        // in    std_logic;  -- Core clock
       Resetb                 ,        // in    std_logic;  -- Use SPARINGLY only for control

       l12ab_WrAddr,                   // [ADDR_LMT-1:0]        arb:               write address
       l12ab_WrTID,                    // [ADDR_LMT-1:0]        arb:               meta data
       l12ab_WrDin,                    // [511:0]               arb:               Cache line data
       l12ab_WrEn,                     //                       arb:               write enable
       ab2l1_WrSent,                   //                       arb:               write issued
       ab2l1_WrAlmFull,                //                       arb:               write fifo almost full
       
       l12ab_RdAddr,                   // [ADDR_LMT-1:0]        arb:               Reads may yield to writes
       l12ab_RdTID,                    // [15:0]                arb:               meta data
       l12ab_RdEn,                     //                       arb:               read enable
       ab2l1_RdSent,                   //                       arb:               read issued

       ab2l1_RdRspValid,               //                       arb:               read response valid
       ab2l1_RdRsp,                    // [15:0]                arb:               read response header
       ab2l1_RdRspAddr,                // [ADDR_LMT-1:0]        arb:               read response address
       ab2l1_RdData,                   // [511:0]               arb:               read data
       ab2l1_stallRd,                  //                       arb:               stall read requests FOR LPBK1

       ab2l1_WrRspValid,               //                       arb:               write response valid
       ab2l1_WrRsp,                    // [15:0]                arb:               write response header
       ab2l1_WrRspAddr,                // [ADDR_LMT-1:0]        arb:               write response address
       re2xy_go,                       //                       requestor:         start the test
       re2xy_NumLines,                 // [31:0]                requestor:         number of cache lines
       re2xy_Cont,                     //                       requestor:         continuous mode

       l12ab_TestCmp,                  //                       arb:               Test completion flag
       l12ab_ErrorInfo,                // [255:0]               arb:               error information
       l12ab_ErrorValid,               //                       arb:               test has detected an error
       test_Resetb                     //                       requestor:         rest the app
);
    input                   Clk_16UI;               //                      csi_top:            Clk_16UI
    input                   Resetb;                 //                      csi_top:            system Resetb
    
    output  [ADDR_LMT-1:0]  l12ab_WrAddr;           // [ADDR_LMT-1:0]        arb:               write address
    output  [15:0]          l12ab_WrTID;            // [15:0]                arb:               meta data
    output  [511:0]         l12ab_WrDin;            // [511:0]               arb:               Cache line data
    output                  l12ab_WrEn;             //                       arb:               write enable
    input                   ab2l1_WrSent;           //                       arb:               write issued
    input                   ab2l1_WrAlmFull;        //                       arb:               write fifo almost full
           
    output  [ADDR_LMT-1:0]  l12ab_RdAddr;           // [ADDR_LMT-1:0]        arb:               Reads may yield to writes
    output  [15:0]          l12ab_RdTID;            // [15:0]                arb:               meta data
    output                  l12ab_RdEn;             //                       arb:               read enable
    input                   ab2l1_RdSent;           //                       arb:               read issued
    
    input                   ab2l1_RdRspValid;       //                       arb:               read response valid
    input  [15:0]           ab2l1_RdRsp;            // [15:0]                arb:               read response header
    input  [ADDR_LMT-1:0]   ab2l1_RdRspAddr;        // [ADDR_LMT-1:0]        arb:               read response address
    input  [511:0]          ab2l1_RdData;           // [511:0]               arb:               read data
    input                   ab2l1_stallRd;          //                       arb:               stall read requests FOR LPBK1
    
    input                   ab2l1_WrRspValid;       //                       arb:               write response valid
    input  [15:0]           ab2l1_WrRsp;            // [15:0]                arb:               write response header
    input  [ADDR_LMT-1:0]   ab2l1_WrRspAddr;        // [Addr_LMT-1:0]        arb:               write response address
    
    input                   re2xy_go;               //                       requestor:         start of frame recvd
    input  [31:0]           re2xy_NumLines;         // [31:0]                requestor:         number of cache lines
    input                   re2xy_Cont;             //                       requestor:         continuous mode
    
    output                  l12ab_TestCmp;          //                       arb:               Test completion flag
    output [255:0]          l12ab_ErrorInfo;        // [255:0]               arb:               error information
    output                  l12ab_ErrorValid;       //                       arb:               test has detected an error
    input                   test_Resetb;
    //------------------------------------------------------------------------------------------------------------------------
    
    reg     [ADDR_LMT-1:0]  l12ab_WrAddr;           // [ADDR_LMT-1:0]        arb:               Writes are guaranteed to be accepted
    reg     [15:0]          l12ab_WrTID;            // [15:0]                arb:               meta data
    reg     [511:0]         l12ab_WrDin;            // [511:0]               arb:               Cache line data
    reg                     l12ab_WrEn;             //                       arb:               write enable
    reg     [ADDR_LMT-1:0]  l12ab_RdAddr;           // [ADDR_LMT-1:0]        arb:               Reads may yield to writes
    reg     [15:0]          l12ab_RdTID;            // [15:0]                arb:               meta data
    reg                     l12ab_RdEn;             //                       arb:               read enable
    reg                     l12ab_TestCmp;          //                       arb:               Test completion flag
    reg    [255:0]          l12ab_ErrorInfo;        // [255:0]               arb:               error information
    reg                     l12ab_ErrorValid;       //                       arb:               test has detected an error
    
    reg     [6:0]           rd_mdata, rd_mdata_next; // limit max mdata to 8 bits or 256 requests
    reg     [2**7-1:0]      rd_mdata_pend;          // bitvector to track used mdata values
    reg     [MDATA-1:0]     wr_mdata;
    reg                     rd_mdata_avail;         // is the next rd madata free and available
    reg     [1:0]           read_fsm;
    reg     [1:0]           write_fsm;
    reg     [15:0]          Num_Read_req;
    reg     [15:0]          Num_Write_req;
    reg     [15:0]          Num_Write_rsp;
    reg                     ab2l1_WrSent_x;
    
    
    
    always @(posedge Clk_16UI)
    begin
            //Read FSM
            case(read_fsm)  /* synthesis parallel_case */
            2'h0:   begin                           // Wait for re2xy_go
                            l12ab_RdAddr            <= 0;
                            Num_Read_req            <= 16'h1;
                            if(re2xy_go)
                            if(re2xy_NumLines!=0)
                                    read_fsm        <= 2'h1;
                            else    read_fsm        <= 2'h2;
                    end
            2'h1:   begin                           // Send read requests
                            if(ab2l1_RdSent)        
                            begin   
                                    l12ab_RdAddr    <= l12ab_RdAddr + 1'b1;
                                    Num_Read_req    <= Num_Read_req    + 1'b1;            // final count will be same as re2xy_NumLines
                                   
                                    if(Num_Read_req == re2xy_NumLines)
                                    if(re2xy_Cont)    read_fsm        <= 2'h0;
                                    else              read_fsm        <= 2'h2;
                            end // ab2l1_RdSent
                    end
            default:                read_fsm        <= read_fsm;
            endcase
            
            //Write FSM
            // requestor manages the num rd credits to match the depth of the tx wr fifo.
            // Therefore for LPBK1 test, the reqeustor tx fifo guarantees to accept the write requests generated.
            // Implies assume, ab2l1_WrSent = 1
            ab2l1_WrSent_x <= 1'b1;
            case(write_fsm) /* synthesis parallel_case */
            2'h0:   begin
                           l12ab_WrAddr    <= ab2l1_RdRspAddr;
                           l12ab_WrDin     <= ab2l1_RdData;
                            if(ab2l1_RdRspValid)
                            begin
                                    write_fsm       <= 2'h1;
                            end
                    end
            2'h1:   begin
                            if(ab2l1_WrSent_x)                                        // assuming that this will always be set
                            begin
                                    Num_Write_req      <= Num_Write_req   + 1'b1;            // final count will be same as re2xy_NumLines
    
                                    if(!ab2l1_RdRspValid)
                                            write_fsm       <= 2'h0;
                                    
                                    if(Num_Write_req == re2xy_NumLines)
                                    begin
                                            if(!re2xy_Cont)  write_fsm      <= 2'h2;
                                            else             Num_Write_req  <= 16'h1;
                                    end
    
//                                    if(ab2l1_RdRspValid)
//                                    begin
                                            l12ab_WrAddr    <= ab2l1_RdRspAddr;
                                            l12ab_WrDin     <= ab2l1_RdData;
//                                    end
                            end
                    end
            default:                write_fsm       <= write_fsm;
            endcase
            
    
            if(l12ab_RdEn && ab2l1_RdSent)
            begin
                    rd_mdata_pend[rd_mdata] <= 1'b1;
                    rd_mdata_next           <= rd_mdata + 2'h2;
                    rd_mdata                <= rd_mdata_next;
                    rd_mdata_avail          <= !rd_mdata_pend[rd_mdata_next];
            end
            else
            begin
                    rd_mdata_avail          <= !rd_mdata_pend[rd_mdata];
                    rd_mdata_next           <= rd_mdata + 1'h1;
            end
    
            if(ab2l1_RdRspValid)
            begin
                    rd_mdata_pend[ab2l1_RdRsp] <= 1'b0;
            end
    
            if(l12ab_WrEn && ab2l1_WrSent_x)
                    wr_mdata   <= wr_mdata + 1'b1;
                    
            // Write response count
            if(ab2l1_WrRspValid)    Num_Write_rsp <= Num_Write_rsp + 1'b1;

            if(write_fsm==2'h2
              && Num_Write_rsp==re2xy_NumLines)
                    l12ab_TestCmp <= 1'b1;
    
           // Error logic
           if(l12ab_WrEn && ab2l1_WrSent==0)
           begin
               // WrFSM assumption is broken
               $display ("%m LPBK1 test WrEn asserted, but request Not accepted by requestor");
               l12ab_ErrorValid <= 1'b1;
               l12ab_ErrorInfo  <= 1'b1;
           end
           
            if(!test_Resetb)
            begin
//                    l12ab_WrAddr            <= 0;
//                    l12ab_RdAddr            <= 0;
                    l12ab_TestCmp           <= 0;
                    l12ab_ErrorInfo         <= 0;
                    l12ab_ErrorValid        <= 0;
                    read_fsm                <= 0;
                    write_fsm               <= 0;
                    rd_mdata                <= 0;
                    rd_mdata_avail          <= 1'b1;
                    rd_mdata_pend           <= 0;
                    wr_mdata                <= 0;
                    Num_Read_req            <= 16'h1;
                    Num_Write_req           <= 16'h1;
                    Num_Write_rsp           <= 0;
            end
            
    end
    
    always @(*)
    begin
            l12ab_WrTID = 0;
            l12ab_RdTID = 0;
            l12ab_WrTID[MDATA-1:0] = wr_mdata;
            l12ab_RdTID[MDATA-1:0] = rd_mdata;

            l12ab_RdEn = (read_fsm  ==2'h1) & !ab2l1_stallRd & rd_mdata_avail;
            l12ab_WrEn = (write_fsm ==2'h1);
    end
    
endmodule
