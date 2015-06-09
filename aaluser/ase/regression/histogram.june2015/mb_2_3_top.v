// ***************************************************************************
//
//        Copyright (C) 2008-2012 Intel Corporation All Rights Reserved.
//
// Engineer:            Narayanan Ravichandran, Pratik Marolia
// Create Date:         Tue Feb 21 17:18:22 PDT 2012
// Edited on:           Wed Apr 09 11:28:01 PDT 2014
// Module Name:         mb_2_3.v
// Project:             mb_2_3 AFU (Compliant with CCI v2.1)
// Description:         top level wrapper for microbenchmark, it instantiates requestor & arbiter
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------
//                                         mb_2_3 - Microbenchmark AFU
//  ------------------------------------------------------------------------------------------------------------------------------------------------
//
// 
//
//   Block Diagram:
//
//   +------------------------------------------------------------------+                       
//   |    +----------+           +---------+      +------------+        |                           
//   |    |          |  Wr       |         |<---->|Test_sw1    |        |                           
//  CCI   |Requestor |<--------->|         |      +------------+        |                            
// <----->|          |  Rd       | Arbiter |      +------------+        |                       
//   |    |          |<--------->|         |<---->|Test_rd     |        |                       
//   |    +----------+           +---------+      +------------+        |                       
//   | mb_2_3_top                                                        |
//   +------------------------------------------------------------------+
//
//  BM Revision and feature tracking
//-------------------------------------------------------------------------------------------
//      Rev     CCI spec        Comments
//-------------------------------------------------------------------------------------------
//      1.0     2.10            BM2 and BM3 for performance characterization 
//
// CSR Address Map -- Change v1.1
//------------------------------------------------------------------------------------------
//      Address[15:0] Attribute         Name                    Comments
//     'h1A00          WO                CSR_AFU_DSM_BASEL       Lower 32-bits of AFU DSM base address. The lower 6-bbits are 4x00 since the address is cache aligned.
//     'h1A04          WO                CSR_AFU_DSM_BASEH       Upper 32-bits of AFU DSM base address.
//
//     'h1A20:         WO                CSR_SRC_ADDR            Start physical address for source buffer. All read requests of rd test are targetted to this region.
//     'h1A28:         WO                CSR_NUM_LINES           Number of cache lines for rd test
//	   'h1A40:		   WO			     CSR_SRC_ADDR_SW       	 Start physical address for source buffer. All read requests of SW test are targetted to this region.			   	       
//	   'h1A44:		   WO				 CSR_DST_ADDR_SW       	 Start physical address for destination buffer. All write requests of SW test are targetted to this region.				           
//	   'h1A48:		   WO				 CSR_NUM_LINES_SW     	 Number of cache lines for SW test				           
//	   'h1A4c:		   WO				 CSR_SW_CTL				 Controls SW test flow. Set the number of instances of SW test and no. of times each instance to be run before test completion.		 
//     'h1A2c:         WO                CSR_CTL                 Controls test flow, start, stop, force completion
//     'h1A34:         WO                CSR_CFG                 Configures test parameters
//     'h1A38:         WO                CSR_INACT_THRESH        --DO NOT USE--
//     'h1A3c          WO                CSR_INTERRUPT0          --DO NOT USE--
//     
//
// DSM Offeset Map -- Change v1.1
//------------------------------------------------------------------------------------------
//      Byte Offset   Width     Attribute         Name                  Comments
//      0x00          32b       RO                DSM_AFU_ID            non-zero value to uniquely identify the AFU
//      0x40          512b      RO                DSM_STATUS            test status and error register
// -- DSM_STATUS definition --
//      0x40          32b       RO                                      test completion flag
//      0x44          32b       RO                                      test error register
//      0x48          64b       RO                                      Number of clocks
//      0x50          32b       RO                                      Number of reads from SW test
//      0x54          32b       RO                                      Number of writes from SW test
//      0x58          32b       RO                                      start overhead
//      0x5c          32b       RO                                      end overhead
//
//
// Let's say the source and dest addresses are 2^(N+6) aligned, then the CSR_NUM_LINES must be less than or equal to 2^N.
//
// CSR_SRC_ADDR and CSR_SRC_ADDR_SW:
// [31:N]      2^(N+6)MB aligned address
// [N-1:0]     'h0
//
// CSR_DST_ADDR and CSR_DST_ADDR_SW:
// [31:N]      2^(N+6)MB aligned address
// [N-1:0]     'h0
//
// CSR_NUM_LINES:
// [31:N]      'h0
// [N-1:0]      No. of cache lines to be read for rd test 
//
// CSR_NUM_LINES_SW:
// [31:N]      'h0
// [N-1:0]      No. of cache lines to be read for SW test 
//
// Let's assume N=14, then CSR_SRC_ADDR and CSR_DST_ADDR will accept a 2^20, i.e. 1MB aligned addresses.
//
// CSR_SRC_ADDR:
// [31:14]      1MB aligned address
// [13:0]      'h0
//
// CSR_DST_ADDR:
// [31:14]      1MB aligned address
// [13:0]      'h0
//
// CSR_NUM_LINES:
// [31:14]      'h0
// [13:0]       No. of cache lines to be read/written to. This threshold may be different for each test AFU. IMPORTANT- Ensure that source and destination buffers 
//              are large enough to accomodate the # cache lines.
//
// CSR_CTL:
// [31:3]       Rsvd
// [2]          Force test completion. Writes test completion flag and other performance counters to csr_stat. It appears to be like a normal test completion.
// [1]          Starts test execution.
// [0]          Active low test Reset. All configuration parameters change to reset defaults.
//
//
// CSR_CFG:
// [31]			If set Disable SW test (all instances)
// [30]			If set Disable Read test
// [29:5]       Rsvd
// [4:2]        cr_mode      - configures test mode
// [1]          cr_cont      - 1 - read test rollsover to start address after it reaches the CSR_NUM_LINES count. Such a test terminates only on an error.
//                             0 - readtest terminates, updated the status csr when CSR_NUM_LINES count is reached.
// [0]          Rsvd
// 
//
// CSR_SW_CTL:
// [16:6]		11 bits		- Number of times to run each instance of SW test before completion. Range : [1 to 2047]      		
// [5:0]		6 bits		- Number of instances of the SW test. Range : [1 to 32]	      
//
// Test flow:
//---------------------------------------------------------------
//o     Test Flow:
// 1.   Reset Device Status Memory (DSM) region.
// 2.   CSR Write to DSM_BASE_H & DSM_BASE_L.
// 3.   SW polls on DSM_AFU_ID.
// 3.   Prepare memory buffer for the test
// 4.   Set cr_ctl[0]. Brings test modules out of Reset.
// 5.   Configure the test parameters, i.e. src/dest address, csr_cfg, num lines etc. These new values can be loaded only when cr_ctl[0]=1 & cr_ctl[1]=0.
// 6.   Set cr_ctl[1]. Start test execution.
// 7.   Deasserting cr_ctl[1] before test completion will kill the test but all test configurations are retained.
// 8.   In order to force complete a test and print out the statistics, set cr_ctl[2].
//                              
// Supported test modes:
//---------------------------------------------------------------
// READ:
// ----
// This is a read only test with NO data checking. It reads CSR_NUM_LINES starting from CSR_SRC_ADDR location. 
// The purpose of this test is to exercise maximum read bandwidth.
// The test can be run in normal mode or cont mode. 
// In normal mode -> No. of reads  = No. of cache lines set for read test (CSR_NUM_LINES)
// In cont mode   -> After reading CSR_NUM_LINES lines the test starts again. Such a test never terminates.
//
// SW1:
// ----
// This test measures the round trip delay for a CPU data move operation. Let N=CSR_NUM_LINES_SW.
//   a. Each instance [1 to 31] initializes N [0 to 2047] cache lines 
//   b. After the instance collects the write completions for N writes, it notifies the CPU. The instance writes predetermined data value to flag address.
//   c. Each CPU thread polls on flag Address then CPU copies N cache lines from CSR_DST_ADDR_SW to CSR_SRC_ADDR_SW buffer.
//   d. Then the CPU thread notifies the FPGA instance that copy operation has completed. CPU thread writes predetermined value to flag address.
//   e. Each FPGA instance is polling on this address (or waiting for an UMsg). When it detects the data pattern, it reads N cache lines the CPU thread wrote to.
//   f. After the instance collects the read completion for N reads, one iteration ends.
//   g. The same process (a to f) is repeated multiple times [1 to 2047] and then the test completes, reports the stats.  
//
// Note:
// -----
// 1. The terms INSTANCE and thread are used interchangeably. 
// 2. When both Read test and SW test are enabled, the overall test terminates upon completion of all instances of SW test.
//
module mb_2_3_top #(parameter TXHDR_WIDTH=61, RXHDR_WIDTH=18, CACHE_WIDTH=512)
(
                
       // ---------------------------global signals-------------------------------------------------
       Clk_32UI,                         //              in    std_logic;  -- Core clock
       Resetb,                           //              in    std_logic;  -- Use SPARINGLY only for control
       // ---------------------------IF signals between CCI and FPL  --------------------------------
       rb2cf_C0RxHdr,                    // [RXHDR_WIDTH-1:0]   cci_intf:           Rx header to CCI channel 0
       rb2cf_C0RxData,                   // [CACHE_WIDTH-1:0]   cci_intf:           Rx data response to CCI | no back pressure
       rb2cf_C0RxWrValid,                //                     cci_intf:           Rx write response enable
       rb2cf_C0RxRdValid,                //                     cci_intf:           Rx read response enable
       rb2cf_C0RxCfgValid,               //                     cci_intf:           Rx config response enable
       rb2cf_C0RxUMsgValid,              //                     cci_intf:           Rx UMsg valid
       rb2cf_C0RxIntrValid,              //                     cci_intf:           Rx interrupt valid
       rb2cf_C1RxHdr,                    // [RXHDR_WIDTH-1:0]   cci_intf:           Rx header to CCI channel 1
       rb2cf_C1RxWrValid,                //                     cci_intf:           Rx write response valid
       rb2cf_C1RxIntrValid,              //                     cci_intf:           Rx interrupt valid

       cf2ci_C0TxHdr,                    // [TXHDR_WIDTH-1:0]   cci_intf:           Tx Header from CCI channel 0
       cf2ci_C0TxRdValid,                //                     cci_intf:           Tx read request enable
       cf2ci_C1TxHdr,                    //                     cci_intf:           Tx Header from CCI channel 1
       cf2ci_C1TxData,                   //                     cci_intf:           Tx data from CCI
       cf2ci_C1TxWrValid,                //                     cci_intf:           Tx write request enable
       cf2ci_C1TxIntrValid,              //                     cci_intf:           Tx interrupt valid
       ci2cf_C0TxAlmFull,                //                     cci_intf:           Tx memory channel 0 almost full
       ci2cf_C1TxAlmFull,                //                     cci_intf:           TX memory channel 1 almost full

       ci2cf_InitDn                      // Link initialization is complete
);


   input                        Clk_32UI;             //              in    std_logic;  -- Core clock
   input                        Resetb;               //              in    std_logic;  -- Use SPARINGLY only for control

   input [RXHDR_WIDTH-1:0]      rb2cf_C0RxHdr;        // [RXHDR_WIDTH-1:0]cci_intf:           Rx header to CCI channel 0
   input [CACHE_WIDTH-1:0]      rb2cf_C0RxData;       // [CACHE_WIDTH-1:0]cci_intf:           data response to CCI | no back pressure
   input                        rb2cf_C0RxWrValid;    //                  cci_intf:           write response enable
   input                        rb2cf_C0RxRdValid;    //                  cci_intf:           read response enable
   input                        rb2cf_C0RxCfgValid;   //                  cci_intf:           config response enable
   input                        rb2cf_C0RxUMsgValid;  //                  cci_intf:           Rx UMsg valid
   input                        rb2cf_C0RxIntrValid;    //                  cci_intf:           interrupt response enable
   input [RXHDR_WIDTH-1:0]      rb2cf_C1RxHdr;        // [RXHDR_WIDTH-1:0]cci_intf:           Rx header to CCI channel 1
   input                        rb2cf_C1RxWrValid;    //                  cci_intf:           write response valid
   input                        rb2cf_C1RxIntrValid;    //                  cci_intf:           interrupt response valid

   output [TXHDR_WIDTH-1:0]     cf2ci_C0TxHdr;        // [TXHDR_WIDTH-1:0]cci_intf:           Tx Header from CCI channel 0
   output                       cf2ci_C0TxRdValid;    //                  cci_intf:           Tx read request enable
   output [TXHDR_WIDTH-1:0]     cf2ci_C1TxHdr;        //                  cci_intf:           Tx Header from CCI channel 1
   output [CACHE_WIDTH-1:0]     cf2ci_C1TxData;       //                  cci_intf:           Tx data from CCI
   output                       cf2ci_C1TxWrValid;    //                  cci_intf:           Tx write request enable
   output                       cf2ci_C1TxIntrValid;  //                  cci_intf:           Tx interrupt valid
   input                        ci2cf_C0TxAlmFull;    //                  cci_intf:           Tx memory channel 0 almost full
   input                        ci2cf_C1TxAlmFull;    //                  cci_intf:           TX memory channel 1 almost full
   
   input                        ci2cf_InitDn;         //                  cci_intf:           Link initialization is complete

   localparam      PEND_THRESH = 7;
   localparam      ADDR_LMT    = 32;
   localparam      MDATA       = 'd11;
   localparam      INSTANCE	   = 31;
   
   wire                         Clk_32UI;
   wire                         Resetb;

   wire [RXHDR_WIDTH-1:0]       rb2cf_C0RxHdr;
   wire [CACHE_WIDTH-1:0]       rb2cf_C0RxData;
   wire                         rb2cf_C0RxWrValid;
   wire                         rb2cf_C0RxRdValid;
   wire                         rb2cf_C0RxCfgValid;
   wire                         rb2cf_C0RxUMsgValid;
   wire [RXHDR_WIDTH-1:0]       rb2cf_C1RxHdr;
   wire                         rb2cf_C1RxWrValid;
   
   wire [TXHDR_WIDTH-1:0]       cf2ci_C0TxHdr;
   wire                         cf2ci_C0TxRdValid;
   wire [TXHDR_WIDTH-1:0]       cf2ci_C1TxHdr;
   wire [CACHE_WIDTH-1:0]       cf2ci_C1TxData;
   wire                         cf2ci_C1TxWrValid;
   wire                         cf2ci_C1TxIntrValid;
   
   wire                         ci2cf_InitDn;
   
   wire [ADDR_LMT-1:0]          ab2re_WrAddr;
   wire [13:0]                  ab2re_WrTID;
   wire [CACHE_WIDTH-1:0]       ab2re_WrDin;
   wire                         ab2re_WrEn;
   wire                         re2ab_WrSent;
   wire                         re2ab_WrAlmFull;
   wire [ADDR_LMT-1:0]          ab2re_RdAddr;
   wire [13:0]                  ab2re_RdTID;
   wire                         ab2re_RdEn;
   wire                         re2ab_RdSent;
   wire                         re2ab_RdRspValid;
   wire                         re2ab_UMsgValid;
   wire [13:0]                  re2ab_RdRsp;
   wire [CACHE_WIDTH-1:0]       re2ab_RdData;
   wire                         re2ab_WrRspValid;
   wire [13:0]                  re2ab_WrRsp;
   wire                         re2xy_go;
   wire [31:0]                  re2xy_src_addr;
   wire [31:0]                  re2xy_src_addr_sw;
   wire [31:0]                  re2xy_dst_addr_sw;
   wire [31:0]                  re2xy_NumLines;
   wire [31:0]                  re2xy_NumLines_sw;
   wire [5:0]					re2xy_NumInst_sw;   			   			   				
   wire	[26:6]	   				re2xy_Numrepeat_sw;              			   
   wire                         re2xy_Cont;
   wire [7:0]                   re2xy_test_cfg;
   wire                         ab2re_TestCmp;
  
   wire                         test_Resetb;
   wire 						re2xy_disable_rd;				
   wire	   						re2xy_disable_sw;	

requestor #(.PEND_THRESH(PEND_THRESH),
            .ADDR_LMT   (ADDR_LMT),
            .TXHDR_WIDTH(TXHDR_WIDTH),
            .RXHDR_WIDTH(RXHDR_WIDTH),
            .CACHE_WIDTH(CACHE_WIDTH)
            )
requestor(


//      ---------------------------global signals-------------------------------------------------
       Clk_32UI               ,        //                       in    std_logic;  -- Core clock
       Resetb                 ,        //                       in    std_logic;  -- Use SPARINGLY only for control
//      ---------------------------CCI IF signals between CCI and requestor  ---------------------
       cf2ci_C0TxHdr,                  //   [TXHDR_WIDTH-1:0]  cci_top:         Tx hdr
       cf2ci_C0TxRdValid,              //                      cci_top:         Tx hdr is valid
       cf2ci_C1TxHdr,                  //   [TXHDR_WIDTH-1:0]  cci_top:         Tx hdr
       cf2ci_C1TxData,                 //                      cci_top:         Tx data
       cf2ci_C1TxWrValid,              //                      cci_top:         Tx hdr is valid
       cf2ci_C1TxIntrValid,            //                      cci_top:         Tx Interrupt valid

       rb2cf_C0RxHdr,                  //  [TXHDR_WIDTH-1:0]   cci_rb:          Rx hdr
       rb2cf_C0RxData,                 //  [CACHE_WIDTH-1:0]   cci_rb:          Rx data
       rb2cf_C0RxWrValid,              //                      cci_rb:          Rx hdr is valid
       rb2cf_C0RxRdValid,              //                      cci_rb:          Rx hdr is valid
       rb2cf_C0RxCfgValid,             //                      cci_rb:          Rx hdr is valid
       rb2cf_C0RxUMsgValid,            //                      cci_intf:        Rx UMsg valid
       rb2cf_C0RxIntrValid,              //                      cci_intf:        Rx interrupt valid
       rb2cf_C1RxHdr,                  //  [TXHDR_WIDTH-1:0]   cci_rb:          Rx hdr
       rb2cf_C1RxWrValid,              //                      cci_rb:          Rx hdr is valid
       rb2cf_C1RxIntrValid,              //                      cci_intf:        Rx interrupt valid

       ci2cf_C0TxAlmFull,              //                      cci_top:         Tx channel is almost full
       ci2cf_C1TxAlmFull,              //                      cci_top:         Tx channel is almost full
       ci2cf_InitDn,                   //                                       Link initialization is complete

       ab2re_WrAddr,                   // [ADDR_LMT-1:0]        arbiter:        Writes are guaranteed to be accepted
       ab2re_WrTID,                    // [13:0]                arbiter:        meta data
       ab2re_WrDin,                    // [CACHE_WIDTH-1:0]     arbiter:        Cache line data
       ab2re_WrFence,                  //                       arbiter:        write fence
       ab2re_WrEn,                     //                       arbiter:        write enable
       re2ab_WrSent,                   //                       arbiter:        write issued
       re2ab_WrAlmFull,                //                       arbiter:        write fifo almost full
       
       ab2re_RdAddr,                   // [ADDR_LMT-1:0]        arbiter:        Reads may yield to writes
       ab2re_RdTID,                    // [13:0]                arbiter:        meta data
       ab2re_RdEn,                     //                       arbiter:        read enable
       re2ab_RdSent,                   //                       arbiter:        read issued

       re2ab_RdRspValid,               //                       arbiter:        read response valid
       re2ab_UMsgValid,                //                       arbiter:        UMsg valid
       re2ab_RdRsp,                    // [ADDR_LMT-1:0]        arbiter:        read response header
       re2ab_RdData,                   // [CACHE_WIDTH-1:0]     arbiter:        read data
       
       re2ab_WrRspValid,               //                       arbiter:        write response valid
       re2ab_WrRsp,                    // [ADDR_LMT-1:0]        arbiter:        write response header
       re2xy_go,                       //                       requestor:      start the test
       re2xy_NumLines,                 // [31:0]                requestor:      number of cache lines for rd test
       re2xy_NumLines_sw,       	   // [31:0]                requestor:      number of cache lines for sw test
	   re2xy_NumInst_sw,   			   // [5:0]			   		requestor:	   	number of instances - SW test			
	   re2xy_Numrepeat_sw,             // [26:6] 			    requestor:	   	number of times to run each instance of SW test before completion.
	   
	   re2xy_Cont,                     //                       requestor:      rd test continuous mode
       re2xy_src_addr,                 // [31:0]                requestor:      src address
       re2xy_src_addr_sw,              // [31:0]                requestor:      src address
       re2xy_dst_addr_sw,              // [31:0]                requestor:      destination address
       re2xy_test_cfg,                 // [7:0]                 requestor:      8-bit test cfg register.
             
       ab2re_TestCmp,                  //                       arbiter:        Test completion flag
       test_Resetb,                    //                       requestor:      rest the app
	   re2xy_disable_rd,			   //									   	   Disable Rd test						
	   re2xy_disable_sw				   //									   	   Disable SW test
);

arbiter #(.PEND_THRESH(PEND_THRESH),
          .ADDR_LMT(ADDR_LMT),
          .MDATA   (MDATA),
		  .INSTANCE(INSTANCE)
          )
arbiter (

//      ---------------------------global signals-------------------------------------------------
       Clk_32UI               ,        //                       in    std_logic;  -- Core clock
       Resetb                 ,        //                       in    std_logic;  -- Use SPARINGLY only for control

       ab2re_WrAddr,                   // [ADDR_LMT-1:0]        arbiter:           write address
       ab2re_WrTID,                    // [13:0]                arbiter:           meta data
       ab2re_WrDin,                    // [CACHE_WIDTH-1:0]     arbiter:           Cache line data
       ab2re_WrFence,                  //                       arbiter:           write fence 
       ab2re_WrEn,                     //                       arbiter:           write enable
       re2ab_WrSent,                   //                       arbiter:           write issued
       re2ab_WrAlmFull,                //                       arbiter:           write fifo almost full
       
       ab2re_RdAddr,                   // [ADDR_LMT-1:0]        arbiter:           Reads may yield to writes
       ab2re_RdTID,                    // [13:0]                arbiter:           meta data
       ab2re_RdEn,                     //                       arbiter:           read enable
       re2ab_RdSent,                   //                       arbiter:           read issued

       re2ab_RdRspValid,               //                       arbiter:           read response valid
       re2ab_UMsgValid,                //                       arbiter:           UMsg valid
       re2ab_RdRsp,                    // [ADDR_LMT-1:0]        arbiter:           read response header
       re2ab_RdData,                   // [CACHE_WIDTH-1:0]     arbiter:           read data

       re2ab_WrRspValid,               //                       arbiter:           write response valid
       re2ab_WrRsp,                    // [ADDR_LMT-1:0]        arbiter:           write response header
       
	   re2xy_go,                       //                       requestor:         start the test
       re2xy_src_addr,                 // [31:0]                requestor:         src address
       re2xy_src_addr_sw,              // [31:0]                requestor:         src address
       re2xy_dst_addr_sw,   		   // [31:0]                requestor:         destination address
       re2xy_NumLines,                 // [31:0]                requestor:         number of cache lines for rd test
       re2xy_NumLines_sw,              // [31:0]                requestor:         number of cache lines for sw test
	   re2xy_NumInst_sw,   			   // [5:0]			   		requestor:	   	   number of instances - SW test			
	   re2xy_Numrepeat_sw,             // [16:6] 			    requestor:	   	   number of times to run each instance of SW test before completion.
	   re2xy_Cont,                     //                       requestor:         rd test continuous mode
       re2xy_test_cfg,                 // [7:0]                 requestor:         8-bit test cfg register.
       ab2re_TestCmp,                  //                       arbiter:           Test completion flag
       test_Resetb,                    //                       requestor:         rest the app
	   re2xy_disable_rd,			   //									   	   Disable Rd test						
	   re2xy_disable_sw				   //									   	   Disable SW test
);


endmodule
