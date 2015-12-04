// ***************************************************************************
//
//        Copyright (C) 2008-2013 Intel Corporation All Rights Reserved.
//
// Engineer:            Narayanan Ravichandran, Pratik Marolia
// Create Date:         Thu Jul 28 20:31:17 PDT 2011
// Edited on:           Thu Nov 20 11:28:01 PDT 2014
// Module Name:         requestor.v
// Project:             mb_2_3 AFU Compliant with CCI v2.1
// Description:         accepts requests from arbiter and formats it per ccispec. It also implements the flow control.
// ***************************************************************************
//
// The requestor accepts the address from the arbiter and sends out the request to the CCI module. 
// It arbitrates between the read and the write requests, peforms the flow control,
// implements all the CSRs for source address, destination address, status address, wrthru enable, start and stop the test.
//
//
//
//----------------------------------------------------------------------------------------------------------------------
/*  Table: Map CSR Address for BM AFU
//                            	ADDR
//      CSR_AFU_DSM_BASEL      	12'h1a00;  		// WO - Lower 32-bits of AFU DSM base address. The lower 6-bbits are 4x00 since the address is cache aligned.
//      CSR_AFU_DSM_BASEH      	12'h1a04;       // WO - Upper 32-bits of AFU DSM base address.
//      CSR_GBCSR              	X               // WO   
//      CSR_CTL                	12'h1a2c        // WO   Control CSR to start n stop the test
//      CSR_CFG                	12'h1a34        // WO   Configures test mode, wrthru, cont and delay mode
//
//      CSR_SRC_ADDR           	12'h1a20        // WO   Reads from rd test are targetted to this region 
//      CSR_NUM_LINES          	12'h1a28        // WO   Numbers of cache lines for rd test to be read/write
//
//		CSR_SRC_ADDR_SW       	12'h1a40;   	// WO   Reads from sw test are targetted to this region 
//		CSR_DST_ADDR_SW       	12'h1a44;       // WO   Writes from sw test are targetted to this region
//		CSR_NUM_LINES_SW     	12'h1a48;       // WO   Numbers of cache lines for sw test to be read/write 
//		CSR_SW_CTL				12'h1A4c;		// WO	Controls SW test flow. Set the number of instances of SW test and no. of times each instance to be run before test completion.		 
//
//      DSM_STATUS             	DSM 32'h40      // R    test status and error info
//      DSM_AFU_ID              DSM 32'h0       // R    non-zero value to uniquely identify the AFU	
*/	
//

module requestor #(parameter PEND_THRESH=1, ADDR_LMT=31, TXHDR_WIDTH=61, RXHDR_WIDTH=18, CACHE_WIDTH=512)
(

//      ---------------------------global signals-------------------------------------------------
       Clk_32UI, 				// in    std_logic;  -- Core clock
       Resetb, 					// in    std_logic;  -- Use SPARINGLY only for control
       // ---------------------------CCI IF signals between CCI and requestor  -------------------
       cf2ci_C0TxHdr,           // [TXHDR_WIDTH-1:0]   cci_top:         Tx hdr
       cf2ci_C0TxRdValid,       //                     cci_top:         Tx hdr is valid
       cf2ci_C1TxHdr,           // [TXHDR_WIDTH-1:0]   cci_top:         Tx hdr
       cf2ci_C1TxData,          //                     cci_top:         Tx data
       cf2ci_C1TxWrValid,       //                     cci_top:         Tx hdr is valid
       cf2ci_C1TxIntrValid,     //                     cci_top:         Tx Interrupt valid

       rb2cf_C0RxHdr,           // [TXHDR_WIDTH-1:0]   cci_rb:          Rx hdr
       rb2cf_C0RxData,          // [CACHE_WIDTH-1:0]   cci_rb:          Rx data
       rb2cf_C0RxWrValid,       //                     cci_rb:          Rx hdr is valid
       rb2cf_C0RxRdValid,       //                     cci_rb:          Rx hdr is valid
       rb2cf_C0RxCfgValid,      //                     cci_rb:          Rx hdr is valid
       rb2cf_C0RxUMsgValid,     //                     cci_intf:        Rx UMsg valid
       rb2cf_C0RxIntrValid,     //                     cci_intf:        Rx interrupt valid
       rb2cf_C1RxHdr,           // [TXHDR_WIDTH-1:0]   cci_rb:          Rx hdr
       rb2cf_C1RxWrValid,       //                     cci_rb:          Rx hdr is valid
       rb2cf_C1RxIntrValid,     //                     cci_intf:        Rx interrupt valid

       ci2cf_C0TxAlmFull,       //                     cci_top:         Tx channel is almost full
       ci2cf_C1TxAlmFull,       //                     cci_top:         Tx channel is almost full
       ci2cf_InitDn,            //                     Link initialization is complete

       ab2re_WrAddr,            // [ADDR_LMT-1:0]      arbiter:        Writes are guaranteed to be accepted
       ab2re_WrTID,             // [13:0]              arbiter:        meta data
       ab2re_WrDin,             // [511:0]             arbiter:        Cache line data
       ab2re_WrFence,           //                     arbiter:        write fence.
       ab2re_WrEn,              //                     arbiter:        write enable
       re2ab_WrSent,            //                     arbiter:        can accept writes. Qualify with write enable
       re2ab_WrAlmFull,         //                     arbiter:        write fifo almost full

       ab2re_RdAddr,            // [ADDR_LMT-1:0]      arbiter:        Reads may yield to writes
       ab2re_RdTID,             // [13:0]              arbiter:        meta data
       ab2re_RdEn,              //                     arbiter:        read enable
       re2ab_RdSent,            //                     arbiter:        read issued

       re2ab_RdRspValid,        //                     arbiter:        read response valid
       re2ab_UMsgValid,         //                     arbiter:        UMsg valid
       re2ab_RdRsp,             // [ADDR_LMT-1:0]      arbiter:        read response header
       re2ab_RdData,            // [511:0]             arbiter:        read data

       re2ab_WrRspValid,        //                     arbiter:        write response valid
       re2ab_WrRsp,             // [ADDR_LMT-1:0]      arbiter:        write response header
       re2xy_go,                //                     requestor:      start the test
       re2xy_NumLines,          // [31:0]              requestor:      number of cache lines for rd test
       re2xy_NumLines_sw,       // [31:0]              requestor:      number of cache lines for sw test
       re2xy_NumInst_sw,   		// [5:0]			   requestor:	   number of instances - SW test			
	   re2xy_Numrepeat_sw,      // [20:0] 			   requestor:	   number of times to run each instance of SW test before completion.
	   
	   re2xy_Cont,              //                     requestor:      read test continuous mode
       re2xy_src_addr,          // [31:0]              requestor:      src address
	   re2xy_src_addr_sw,       // [31:0]              requestor:      src address
       re2xy_dst_addr_sw,       // [31:0]              requestor:      destination address
	   re2xy_test_cfg,          // [7:0]               requestor:      8-bit test cfg register.

       ab2re_TestCmp,           //                     arbiter:        Test completion flag
       test_Resetb,             //                     requestor:      rest the app
	   re2xy_disable_rd,		//									   Disable Rd test						
	   re2xy_disable_sw			//									   Disable SW test
       );
//--------------------------------------------------------------------------------------------------------------

    input[RXHDR_WIDTH-1:0]  rb2cf_C0RxHdr;          //  [RXHDR_WIDTH-1:0]   cci_rb:         Rx hdr
    input[CACHE_WIDTH-1:0]  rb2cf_C0RxData;         //  [CACHE_WIDTH-1:0]   cci_rb:         Rx data
    input                   rb2cf_C0RxWrValid;      //                      cci_rb:         Rx hdr carries a write response
    input                   rb2cf_C0RxRdValid;      //                      cci_rb:         Rx hdr carries a read response
    input                   rb2cf_C0RxCfgValid;     //                      cci_rb:         Rx hdr carries a cfg write
    input                   rb2cf_C0RxUMsgValid;    //                      cci_intf:       Rx UMsg valid
    input                   rb2cf_C0RxIntrValid;    //                      cci_intf:       interrupt response enable
    input[RXHDR_WIDTH-1:0]  rb2cf_C1RxHdr;          //  [RXHDR_WIDTH-1:0]   cci_rb:         Rx hdr
    input                   rb2cf_C1RxWrValid;      //                      cci_rb:         Rx hdr carries a write response
    input                   rb2cf_C1RxIntrValid;    //                      cci_intf:       interrupt response enable
    input                   ci2cf_C0TxAlmFull;      //                      cci_top:        Tx channel is almost full
    input                   ci2cf_C1TxAlmFull;      //                      cci_top:        Tx channel is almost full
    input                   ci2cf_InitDn;
    input                   Clk_32UI;               //                      csi_top:        Clk_32UI
    input                   Resetb;                 //                      csi_top:        system Resetb
    
    output[TXHDR_WIDTH-1:0] cf2ci_C0TxHdr;          //   [TXHDR_WIDTH-1:0]  cci_top:        Tx hdr
    output                  cf2ci_C0TxRdValid;      //                      cci_top:        Tx hdr is valid
    output[TXHDR_WIDTH-1:0] cf2ci_C1TxHdr;          //   [TXHDR_WIDTH-1:0]  cci_top:        Tx hdr
    output[CACHE_WIDTH-1:0] cf2ci_C1TxData;         //                      cci_top:        Tx data
    output                  cf2ci_C1TxWrValid;      //                      cci_top:        Tx hdr is valid
    output                  cf2ci_C1TxIntrValid;    //                      cci_top:        Tx Interrupt valid
    
    input  [ADDR_LMT-1:0]   ab2re_WrAddr;           // [ADDR_LMT-1:0]        arbiter:       Writes are guaranteed to be accepted
    input  [13:0]           ab2re_WrTID;            // [13:0]                arbiter:       meta data
    input  [511:0]          ab2re_WrDin;            // [511:0]               arbiter:       Cache line data
    input                   ab2re_WrFence;          //                       arbiter:       write fence 
    input                   ab2re_WrEn;             //                       arbiter:       write enable
    output                  re2ab_WrSent;           //                       arbiter:       write issued
    output                  re2ab_WrAlmFull;        //                       arbiter:       write fifo almost full
           
    input  [ADDR_LMT-1:0]	ab2re_RdAddr;           // [ADDR_LMT-1:0]        arbiter:       Reads may yield to writes
    input  [13:0]           ab2re_RdTID;            // [13:0]                arbiter:       meta data
    input                   ab2re_RdEn;             //                       arbiter:       read enable
    output                  re2ab_RdSent;           //                       arbiter:       read issued
    
    output                  re2ab_RdRspValid;       //                       arbiter:       read response valid
    output                  re2ab_UMsgValid;        //                       arbiter:       UMsg valid
    output [13:0]           re2ab_RdRsp;            // [13:0]                arbiter:       read response header
    output [511:0]          re2ab_RdData;           // [511:0]               arbiter:       read data
    
    output                  re2ab_WrRspValid;       //                       arbiter:       write response valid
    output [13:0]           re2ab_WrRsp;            // [13:0]                arbiter:       write response header
    
    output                  re2xy_go;               //                       requestor:     start of frame recvd
    output [31:0]           re2xy_NumLines;         // [31:0]                requestor:     number of cache lines for rd test
    output [31:0]           re2xy_NumLines_sw;      // [31:0]                requestor:     number of cache lines for sw test
    output [5:0]			re2xy_NumInst_sw;   	// [5:0]				 requestor:		number of instances - SW test
	output [20:0]			re2xy_Numrepeat_sw;     // [10:0] 				 requestor:		number of times to run each instance of SW test before completion.
	output                  re2xy_Cont;             //                       requestor:     read test in continuous mode
    output [31:0]           re2xy_src_addr;         // [31:0]                requestor:     src address
	output [31:0]           re2xy_src_addr_sw;      // [31:0]                requestor:     src address
    output [31:0]           re2xy_dst_addr_sw;      // [31:0]                requestor:     destination address
    output [7:0]            re2xy_test_cfg;         // [7:0]                 requestor:     8-bit test cfg register.
    input                   ab2re_TestCmp;          //                       arbiter:       Test completion flag
    
    output                  test_Resetb;
	output				    re2xy_disable_rd;		//									   Disable Rd test						
	output				    re2xy_disable_sw;		//									   Disable SW test
	

    //----------------------------------------------------------------------------------------------------------------------
    // BM v1.0 AFU ID
    localparam      BM_V1_0             = 128'hC000_C966_0D82_4272_9AEF_FE5F_8457_0612;
    localparam      CONST_FWDRANGE_BEG   = 16'h1B00;                 //      fwd all CSR writes above this address to test modules
    
	//---------------------------------------------------------
    // CCI-S Request Encodings  ***** DO NOT MODIFY ******
    //---------------------------------------------------------
    localparam       WrThru              = 4'h1;
    localparam       WrLine              = 4'h2;
    localparam       RdLine_S            = 4'h4;
    localparam       WrFence             = 4'h5;
    localparam       RdLine_I            = 4'h6;
    localparam       RdLine_O            = 4'h7;
    localparam       Intr                = 4'h8;    // FPGA to CPU interrupt
    
    // Response types  ***** DO NOT MODIFY ******
    //--------------------------------------------------------
    localparam      RSP_READ             = 4'h0;
    localparam      RSP_CSR              = 4'h1;
    localparam      RSP_WRITE            = 4'h2;
    
    //---------------------------------------------------------
    // Default Values ****** Can be MODIFIED ******* 
    //---------------------------------------------------------
    localparam      DEF_SRC_ADDR         = 32'h0400_0000;           // Read data starting from here. Cache aligned Address
    localparam      DEF_DST_ADDR         = 32'h0500_0000;           // Copy data to here. Cache aligned Address
    localparam      DEF_DSM_BASE         = 32'h04ff_ffff;           // default status address
    
    //---------------------------------------------------------
    // Write CSR Address Map ***** DO NOT MODIFY *****
    //---------------------------------------------------------
                                                                    // CSR Attribute - Comment
    localparam      CSR_AFU_DSM_BASEL    = 16'h1a00;                 // WO - Lower 32-bits of AFU DSM base address. The lower 6-bbits are 4x00 since the address is cache aligned.
    localparam      CSR_AFU_DSM_BASEH    = 16'h1a04;                 // WO - Upper 32-bits of AFU DSM base address.
	localparam      CSR_CTL              = 16'h1a2c;                 // WO   Control CSR to start n stop the test   
	
    localparam      CSR_SRC_ADDR      	 = 16'h1a20;                // WO   Reads from read test are targetted to this region 
    localparam      CSR_DST_ADDR      	 = 16'h1a24;                // WO   Writes from read test are targetted to this region 
	localparam      CSR_NUM_LINES     	 = 16'h1a28;                // WO   Numbers of cache lines to be read/write for read test
	localparam      CSR_CFG          	 = 16'h1a34;                // WO   Configures test mode, wrthru, cont and delay mode
	
	localparam      CSR_SRC_ADDR_SW      = 16'h1a40;                 // WO   Reads from sw test are targetted to this region 
    localparam      CSR_DST_ADDR_SW      = 16'h1a44;                 // WO   Writes from sw test are targetted to this region
	localparam      CSR_NUM_LINES_SW     = 16'h1a48;                 // WO   Numbers of cache lines to be read/write for sw test
	localparam 		CSR_SW_CTL			 = 16'h1a4c;					// WO	SW test control  
	
	localparam      CSR_INACT_THRESH     = 16'h1a38;                 // WO   set the threshold limit for inactivity trigger
    
    //----------------------------------------------------------------------------------
    // Device Status Memory (DSM) Address Map ***** DO NOT MODIFY *****
    // Physical address = value at CSR_AFU_DSM_BASE + Byte offset
    //----------------------------------------------------------------------------------
    //                                     Byte Offset                 Attribute    Width   Comments
    localparam      DSM_AFU_ID           = 32'h0;                   // RO           32b     non-zero value to uniquely identify the AFU
    localparam      DSM_STATUS           = 32'h40;                  // RO           512b    test status and error info
    
    //----------------------------------------------------------------------------------------------------------------------
    
    reg    [511:0]          cf2ci_C1TxData;
    reg    [TXHDR_WIDTH-1:0]cf2ci_C1TxHdr;
    reg                     cf2ci_C1TxWrValid;
    reg    [TXHDR_WIDTH-1:0]cf2ci_C0TxHdr;
    reg                     cf2ci_C0TxRdValid;
    wire                    cf2ci_C1TxIntrValid= 0;
    
    reg  [31:0]             ErrorVector;
    reg  [31:0]             Num_Reads;                              // Number of reads performed
    reg  [31:0]             Num_Reads_rdtest;                       
    reg  [31:0]             Num_Reads_swtest;                             
    reg  [31:0]             Num_Writes;                             // Number of writes performed
    reg  [63:0]             Num_ticks;                              // number of clock ticks
    reg  [PEND_THRESH-1:0]  Num_Pend;                               // Number of pending requests
    reg  [31:0]             Num_C0stall;                            // Number of clocks for which Channel0 was throttled
    reg  [31:0]             Num_C1stall;                            // Number of clocks for which channel1 was throttled
    reg                     RdHdr_valid;
    reg                     WrHdr_valid;
    
	reg                     ci2sl_InitDn_delayed;
    reg                     ci2sl_InitDn_delayed_reg1;
    reg                     ci2sl_InitDn_delayed_reg2;
    reg                     ci2sl_InitDn_delayed_reg3;
    reg                     ci2sl_InitDn_delayed_reg4;
    reg                     ci2sl_InitDn_delayed_reg5;
	   
	reg  [31:0]             wrfifo_addr;
    reg  [511:0]            wrfifo_data;
    reg                     fifo_RdEn;
    reg  [511:0]            rb2cf_C0RxData_q;
    reg  [RXHDR_WIDTH-1:0]  rb2cf_C0RxHdr_q;
    reg                     rb2cf_C0RxWrValid_q;
    reg                     rb2cf_C0RxRdValid_q;
    reg                     rb2cf_C0RxUMsgValid_q;
    reg  [8:0]              ctr;
    reg                     re2ab_RdSent;
    reg                     status_write;
    
    reg   [31:0]            inact_cnt;
    reg                     inact_timeout;
    reg   [5:0]             delay_lfsr;
    reg   [31:0]            cr_inact_thresh;
    reg                     rxfifo_RdEn_q;
    reg                     penalty_start_f;
    reg   [7:0]             penalty_start;
    reg   [7:0]             penalty_end;
    reg                     dsm_base_valid;
    reg                     dsm_base_valid_q;
    reg                     afuid_updtd;
	reg   [3:0]             rdreq_type;
    reg   [3:0]             rnd_rdreq_type;
    reg   [1:0]             rnd_rdreq_sel; 
	
	
    integer                 i;
    reg     [63:0]          cr_dsm_base;                            // a00h, a04h - DSM base address
    reg     [31:0]          cr_ctl = 0;                             // a2ch - control register to start and stop the test
	
	reg     [31:0]          cr_src_address;                         // a20h - source buffer address - rd test
    reg     [31:0]          cr_num_lines;                           // a28h - Number of cache lines - rd test 
    reg                     cr_wrthru_en;                           // a34h - [0]    : test configuration- wrthru_en - rd test
    reg                     cr_cont;                                // a34h - [1]    : repeats the read test sequence, NO end condition 
	reg                     cr_delay_en;                            // a34h - [8]    : use start delay - rd test
    reg     [1:0]           cr_rdsel, cr_rdsel_q;                   // a34h - [10:9] : read request type
	reg     [7:0]           cr_start_delay;                         // a34h - [19:12]: delay count - rd test
    reg     [7:0]           cr_test_cfg;                            // a34h - [27:0] : configuration within a selected test mode - rd test
    reg     [31:0]          cr_src_address_sw;                      // a40h - source buffer address - sw test
    reg     [31:0]          cr_dst_address_sw;                      // a44h - destn buffer address  - sw test
    reg     [31:0]          cr_num_lines_sw;                        // a48h - Number of cache lines - sw test 
	
	reg     [31:0]          cr_num_lines_sw_reg1;                   
	reg     [31:0]          cr_num_lines_sw_reg2;                   
	reg     [31:0]          cr_num_lines_sw_reg3; 
	reg     [31:0]          cr_num_lines_sw_reg4; 
	reg     [31:0]          cr_num_lines_sw_reg5; 
	
	reg     [31:0]          cr_num_inst_sw_reg1;                         
	reg     [31:0]          cr_num_inst_sw_reg2;                         
	reg     [31:0]          cr_num_inst_sw_reg3; 
	reg     [31:0]          cr_num_inst_sw_reg4; 
	reg     [31:0]          cr_num_inst_sw_reg5; 
	
	reg     [5:0]			cr_num_inst_sw;							
	reg     [20:0]			cr_num_repeat_sw;  						
	
	reg     [31:0]          ds_stat_address;                        // 040h - test status is written to this address
    reg     [31:0]          ds_afuid_address;                       // 040h - test status is written to this address
	
	reg						cr_disable_rd;							// 1 - Disable Read test				
	reg						cr_disable_sw;				   			// 1 - Disable SW test
	reg						cr_disable_sw_reg1;				   			
	reg						cr_disable_sw_reg2;				   			
	reg						cr_disable_sw_reg3;				   			
	reg						cr_disable_sw_reg4;				   			
	reg						cr_disable_sw_reg5;				   			
	
    
	wire    [31:0]          re2xy_src_addr     = cr_src_address;
	wire    [31:0]          re2xy_src_addr_sw  = cr_src_address_sw;
	wire    [31:0]          re2xy_dst_addr_sw  = cr_dst_address_sw;
    
    
    // ctl[0]       reset the test
    // ctl[1]       go
    wire                    fifo_Full;
    wire                    fifo_Empty;
    wire                    fifo_almfull;
    wire [13:0]             rxfifo_Din      = rb2cf_C1RxHdr[13:0];
    wire                    rxfifo_WrEn     = rb2cf_C1RxWrValid;
    wire [PEND_THRESH-1:0]  rxfifo_count;
    wire                    rxfifo_Full;
    wire                    rxfifo_almfull;
    wire                    rxfifo_Empty;
    
    wire [13:0]             rxfifo_Dout;
    wire                    rxfifo_RdEn;
    wire                    test_Resetb = cr_ctl[0];                		// Clears all the states. Either is one then test is out of Reset.
    wire                    test_go     = cr_ctl[1];                		// When 0, it pauses the test and allows reconfiguration of test parameters.
																			// All states are preserved
    wire                    re2ab_WrSent   = !fifo_Full && !status_write;   // stop accepting new requests, after status write=1
    wire                    fifo_WrEn      = (ab2re_WrEn| ab2re_WrFence) && ~fifo_Full;
    wire [1+512+ADDR_LMT+13:0]fifo_Din     = {ab2re_WrFence, ab2re_WrDin,ab2re_WrAddr, ab2re_WrTID};
    wire [1+512+ADDR_LMT+13:0]fifo_Dout;
    
    // Format Read Header
    wire [31:0]             RdAddr;
	assign RdAddr =			ab2re_RdAddr;
	    
	wire [13:0]             RdReqId = 14'h0000 | ab2re_RdTID;
    wire [TXHDR_WIDTH-1:0]  RdHdr   = {
                                            5'h00,                          // [60:56]      Byte Enable
                                            rdreq_type,                     // [55:52]      Rquest Type
                                            6'h00,                          // [51:46]      Rsvd
                                            RdAddr,                         // [45:14]      Address
                                            RdReqId                         // [13:0]       Meta data to track the SPL requests
                                      };
    
    // Format Write Header
    wire [31:0]             WrAddr;	
	assign WrAddr = 		fifo_Dout[ADDR_LMT-1+14:0+14];  
    
	wire [13:0]             WrReqId = 14'h0000 | fifo_Dout[13:0];
    wire                    ReqFence = fifo_Dout[1+512+ADDR_LMT+13];
    wire [TXHDR_WIDTH-1:0]  WrHdr   = {
                                            5'h00,                          // [60:56]      Byte Enable
                                            ReqFence     ?WrFence           // [55:52]      Request Type
                                            :cr_wrthru_en?WrThru
                                                         :WrLine, 
                                            6'h00,                          // [51:46]      Rsvd
                                            WrAddr,                         // [45:14]      Address
                                            WrReqId                         // [13:0]       Meta data to track the SPL requests
                                      };
    
    
    wire                    re2ab_RdRspValid   = rb2cf_C0RxRdValid_q;
    wire                    re2ab_UMsgValid    = rb2cf_C0RxUMsgValid_q;
    wire   [13:0]           re2ab_RdRsp        = rb2cf_C0RxHdr_q[13:0];
    wire   [511:0]          re2ab_RdData       = rb2cf_C0RxData_q;
    wire                    re2ab_WrRspValid   = rxfifo_RdEn_q | rb2cf_C0RxWrValid_q;
    wire   [13:0]           re2ab_WrRsp        = rxfifo_RdEn_q ? rxfifo_Dout : rb2cf_C0RxHdr_q[13:0];
    wire                    re2xy_go           = ci2sl_InitDn_delayed_reg5;
    wire   [31:0]           re2xy_NumLines     = cr_num_lines;
    wire   [31:0]           re2xy_NumLines_sw  = cr_num_lines_sw_reg5;
    wire   [5:0]			re2xy_NumInst_sw   = cr_num_inst_sw_reg5;		
	wire   [20:0]			re2xy_Numrepeat_sw = cr_num_repeat_sw;
		
	wire                    re2xy_Cont       = cr_cont;
    wire                    re2ab_WrAlmFull  = fifo_almfull;
    wire   [7:0]            re2xy_test_cfg   = cr_test_cfg;
    
	wire					re2xy_disable_rd = cr_disable_rd;		    
	wire					re2xy_disable_sw = cr_disable_sw_reg5;	

	
	reg	ci2cf_C1TxAlmFull_reg1;
	reg ci2cf_C1TxAlmFull_reg2;
	reg ci2cf_C0TxAlmFull_reg1;
	reg ci2cf_C0TxAlmFull_reg2;
	
	always@(posedge Clk_32UI)
	begin
	ci2cf_C1TxAlmFull_reg1 <= ci2cf_C1TxAlmFull;
	ci2cf_C1TxAlmFull_reg2 <= ci2cf_C1TxAlmFull_reg1;
	
	ci2cf_C0TxAlmFull_reg1 <= ci2cf_C0TxAlmFull;
	ci2cf_C0TxAlmFull_reg2 <= ci2cf_C0TxAlmFull_reg1;
	end
	
	always@(posedge Clk_32UI)
	begin
		if(!Resetb | !test_Resetb)
		begin
				Num_Reads_rdtest <= 0;
				Num_Reads_swtest <= 0;
				Num_Reads        <= 0;
				Num_Writes       <= 0;
		end
		
		else
		begin
			if(cf2ci_C0TxRdValid && cf2ci_C0TxHdr[13]==1'b1)
			begin
				Num_Reads_rdtest <= Num_Reads_rdtest+1;
				Num_Reads_swtest <= Num_Reads_swtest;
				Num_Reads        <= Num_Reads+1'b1;
			end
			
			else if (cf2ci_C0TxRdValid)
			begin
				Num_Reads_rdtest <= Num_Reads_rdtest;
				Num_Reads_swtest <= Num_Reads_swtest+1;
				Num_Reads        <= Num_Reads+1'b1;
			end	
			
			else
			begin
				Num_Reads_rdtest <= Num_Reads_rdtest;
				Num_Reads_swtest <= Num_Reads_swtest;
			end	
			
			if(cf2ci_C1TxWrValid)
				Num_Writes       <= Num_Writes + 1'b1;
            else
				Num_Writes       <= Num_Writes;
		end
	end
	
	
	always @(posedge Clk_32UI)                                              // - Update Test Configuration
    begin    
		if(!Resetb)
		begin
		cr_num_lines_sw_reg1 <=  8; cr_num_inst_sw_reg1 <=1; ci2sl_InitDn_delayed_reg1 <=0; cr_disable_sw_reg1 <=0;                     
		cr_num_lines_sw_reg2 <=  8; cr_num_inst_sw_reg2 <=1; ci2sl_InitDn_delayed_reg2 <=0; cr_disable_sw_reg2 <=0;
		cr_num_lines_sw_reg3 <=  8; cr_num_inst_sw_reg3 <=1; ci2sl_InitDn_delayed_reg3 <=0; cr_disable_sw_reg3 <=0;
		cr_num_lines_sw_reg4 <=  8; cr_num_inst_sw_reg4 <=1; ci2sl_InitDn_delayed_reg4 <=0; cr_disable_sw_reg4 <=0;
		cr_num_lines_sw_reg5 <=  8; cr_num_inst_sw_reg5 <=1; ci2sl_InitDn_delayed_reg5 <=0; cr_disable_sw_reg5 <=0;
		end
		
		else
		begin
		cr_num_lines_sw_reg1 <=  cr_num_lines_sw;                               
		cr_num_lines_sw_reg2 <=  cr_num_lines_sw_reg1;   
		cr_num_lines_sw_reg3 <=  cr_num_lines_sw_reg2;
		cr_num_lines_sw_reg4 <=  cr_num_lines_sw_reg3;
		cr_num_lines_sw_reg5 <=  cr_num_lines_sw_reg4;

		cr_num_inst_sw_reg1 <=  cr_num_inst_sw;                               
		cr_num_inst_sw_reg2 <=  cr_num_inst_sw_reg1;   
		cr_num_inst_sw_reg3 <=  cr_num_inst_sw_reg2;
		cr_num_inst_sw_reg4 <=  cr_num_inst_sw_reg3;
		cr_num_inst_sw_reg5 <=  cr_num_inst_sw_reg4;
		
		ci2sl_InitDn_delayed_reg1 <= ci2sl_InitDn_delayed;
		ci2sl_InitDn_delayed_reg2 <= ci2sl_InitDn_delayed_reg1;
		ci2sl_InitDn_delayed_reg3 <= ci2sl_InitDn_delayed_reg2;
		ci2sl_InitDn_delayed_reg4 <= ci2sl_InitDn_delayed_reg3;
		ci2sl_InitDn_delayed_reg5 <= ci2sl_InitDn_delayed_reg4;

		cr_disable_sw_reg1 <= cr_disable_sw;
		cr_disable_sw_reg2 <= cr_disable_sw_reg1;
		cr_disable_sw_reg3 <= cr_disable_sw_reg2;
		cr_disable_sw_reg4 <= cr_disable_sw_reg3;
		cr_disable_sw_reg5 <= cr_disable_sw_reg4;
		end		
	end
	
    
    always @(posedge Clk_32UI)                                              // - Update Test Configuration
    begin                                                                   //-----------------------------
            if(!Resetb)
            begin
                    cr_src_address  	<= DEF_SRC_ADDR; 
                    cr_num_lines    	<= 8;
					cr_wrthru_en    	<= 0;
                    cr_cont         	<= 0;
                    cr_delay_en     	<= 0;
                    cr_start_delay  	<= 1;
                    cr_test_cfg     	<= 0;
					cr_disable_rd		<= 0;
				    cr_disable_sw		<= 0;
					
					
					cr_src_address_sw  	<= DEF_SRC_ADDR;                   
					cr_dst_address_sw  	<= DEF_DST_ADDR;
					cr_num_lines_sw 	<= 8;
					cr_num_inst_sw		<= 1;
					cr_num_repeat_sw	<= 1;
					                   
					cr_ctl          	<= 0;
					cr_inact_thresh 	<= 32'hffff_ffff;
                    cr_dsm_base     	<= DEF_DSM_BASE;
                    dsm_base_valid  	<= 0;
            end
            else
            begin                  
                    if(rb2cf_C0RxCfgValid)
                        case({rb2cf_C0RxHdr[13:0],2'b00})         /* synthesis parallel_case */
                                CSR_CTL          :   cr_ctl             <= rb2cf_C0RxData[31:0];
                                CSR_AFU_DSM_BASEH:   cr_dsm_base[63:32] <= rb2cf_C0RxData[31:0];
                                CSR_AFU_DSM_BASEL:begin
                                                     cr_dsm_base[31:0]  <= rb2cf_C0RxData[31:0];
                                                     dsm_base_valid     <= 1;
                                                  end
                        endcase
                      
                    if(test_Resetb && ~test_go)                   // Test out of Reset
                    begin
                        if(rb2cf_C0RxCfgValid)
                            case({rb2cf_C0RxHdr[13:0],2'b00})     /* synthesis parallel_case */
                                    CSR_SRC_ADDR:        cr_src_address     <= rb2cf_C0RxData[31:0];
									CSR_SRC_ADDR_SW:	 cr_src_address_sw  <= rb2cf_C0RxData[31:0];
									
                                    CSR_DST_ADDR_SW:     cr_dst_address_sw  <= rb2cf_C0RxData[31:0];
									
                                    CSR_NUM_LINES:       cr_num_lines       <= rb2cf_C0RxData[31:0];
                                    CSR_NUM_LINES_SW:    cr_num_lines_sw    <= rb2cf_C0RxData[31:0];
									
									CSR_SW_CTL:	begin	 
														 cr_num_inst_sw		<= rb2cf_C0RxData[5:0];
														 cr_num_repeat_sw	<= rb2cf_C0RxData[26:6];	
												end
												
                                    CSR_CFG:   begin
                                                         cr_wrthru_en       <= rb2cf_C0RxData[0];
                                                         cr_cont            <= rb2cf_C0RxData[1];
                                                         cr_delay_en        <= rb2cf_C0RxData[8];
                                                         cr_rdsel           <= rb2cf_C0RxData[10:9];
														 cr_start_delay     <= rb2cf_C0RxData[19:12];
                                                         cr_test_cfg        <= rb2cf_C0RxData[27:20];
														 cr_disable_rd		<= rb2cf_C0RxData[30];
														 cr_disable_sw		<= rb2cf_C0RxData[31];
                                               end
									CSR_INACT_THRESH:    cr_inact_thresh    <= rb2cf_C0RxData[31:0];
							endcase
                    end
            end
    end
    
    always @(posedge Clk_32UI)
    begin
        ds_stat_address <= dsm_offset2addr(DSM_STATUS,cr_dsm_base);
        ds_afuid_address<= dsm_offset2addr(DSM_AFU_ID,cr_dsm_base);
        dsm_base_valid_q <= dsm_base_valid;
    
		cr_rdsel_q <= cr_rdsel;
        delay_lfsr <= (delay_lfsr<<1) || {3'h0, (delay_lfsr[5] ^ delay_lfsr[4])};
        
		case(cr_rdsel_q)
            2'h0:   rdreq_type <= RdLine_S;
            2'h1:   rdreq_type <= RdLine_I;
            2'h2:   rdreq_type <= RdLine_O;
            2'h3:   rdreq_type <= rnd_rdreq_type;
        endcase
        rnd_rdreq_sel  <= delay_lfsr%3;
        case(rnd_rdreq_sel)
            2'h1:   rnd_rdreq_type <= RdLine_I;
            2'h2:   rnd_rdreq_type <= RdLine_O;
            default:rnd_rdreq_type <= RdLine_S;
        endcase
		
		
        if(ci2cf_InitDn                                                         // CCI initilaization done
        && test_go)                                                             // Test start
                ctr <= ctr+1'b1;
        else    
                ctr <= 0;
    
        if(~test_go)                                                            // Used for debug- start test after #clks > cr_start_delay
        begin
                ci2sl_InitDn_delayed    <= 0;
                status_write            <= 0;
        end
        else        
        begin
            if( ~ci2sl_InitDn_delayed
              && ctr > cr_start_delay
              && afuid_updtd
              )
                ci2sl_InitDn_delayed    <= 1;
        end
    
        if(ci2sl_InitDn_delayed)                                                // Count #clks after test start
                Num_ticks       <= Num_ticks + 1;
        
        if(  dsm_base_valid_q                                                   // - Transmit Path
          & !afuid_updtd
          & !ci2cf_C1TxAlmFull_reg2
          )
        begin
                     cf2ci_C1TxHdr           <= {
                                                     5'h0,                      // [60:56]      Byte Enable
                                                     WrLine,                    // [55:52]      Request Type
                                                     6'h00,                     // [51:46]      Rsvd
                                                     ds_afuid_address,          // [44:14]      Address
                                                     14'h3ffe                   // [13:0]       Meta data to track the SPL requests
                                                };                
                     cf2ci_C1TxWrValid       <= 1;
                     cf2ci_C0TxRdValid       <= 0;
                     cf2ci_C1TxData          <= {    368'h0,                    // [512:144]    Zeros
                                                     16'h0001,                  // [143:128]    Version #
                                                     BM_V1_0                   // [127:0]      AFU ID
                                                };
                     afuid_updtd             <= 1;
        end
        else if
          ( ci2sl_InitDn_delayed
          & test_Resetb
          & !status_write
          )
        begin
            // Tx Writes
            if( (  ab2re_TestCmp                                               // Update Status upon test completion
                 ||ErrorVector!=0                                              // Error detected 
                 ||cr_ctl[2]                                                   // SW forced test termination
                )                       
               && !ci2cf_C1TxAlmFull_reg2                                           
              )                                                                //
            begin                                                              //-----------------------------------
                    cf2ci_C1TxHdr      <= {
                                               5'h0,                           // [60:56]      Byte Enable
                                               cr_wrthru_en? WrThru            // [55:52]      Req Type
                                                           : WrLine,           //
                                               6'h00,                          // [51:46]      Rsvd
                                               ds_stat_address,                // [44:14]      Address
                                               14'h3fff                        // [13:0]       Meta data to track the SPL requests
                                          };
                    cf2ci_C1TxWrValid  <= 1;
                    cf2ci_C1TxData     <= {    
												Num_Reads,			   		   // [319:288] Total Number of Reads 			9
												Num_Reads_rdtest,			   // [287:256] Number of Reads from rd test    8
												24'h00_0000,penalty_end,       // [255:224] test end overhead in # clks		7
                                                24'h00_0000,penalty_start,     // [223:192] test start overhead in # clks	6
                                                Num_Writes,                    // [191:160] Total number of Writes sent		5
                                                Num_Reads_swtest,              // [159:128] Number of Reads from sw test  	4
                                                Num_ticks,                     // [127:64]  number of clks					2,3
                                                ErrorVector,                   // [63:32]   errors detected            		1
                                                32'h0000_0001};                // [31:0]    test completion flag			0
                    status_write       <= 1;
            end 
            else if(   WrHdr_valid
                   && !ci2cf_C1TxAlmFull_reg2
                   )
            begin                                                                   // Write to Destination Workspace
                                                                                    //-----------------------------------        
                    cf2ci_C1TxHdr     <= WrHdr;
                    cf2ci_C1TxWrValid <= 1;
                    cf2ci_C1TxData    <= fifo_Dout[511+ADDR_LMT+14:ADDR_LMT+14];
                    WrHdr_valid       <= fifo_RdEn;
            end
            else
            begin
                    cf2ci_C1TxWrValid <= 0;
                    if(ci2cf_C1TxAlmFull_reg2)
                           Num_C1stall     <= Num_C1stall + 1;
            end
    
            // Tx Reads
            if(RdHdr_valid && !ci2cf_C0TxAlmFull_reg2 )                                  // Read from Source Workspace
              begin                                                                 //----------------------------------
                    cf2ci_C0TxHdr      <= RdHdr;
                    cf2ci_C0TxRdValid  <= 1;
               end
            else 
            begin
                    cf2ci_C0TxRdValid  <= 0;
                    if(ci2cf_C0TxAlmFull_reg2)
                           Num_C0stall     <= Num_C0stall + 1;
            end
    
        end // ci2sl_InitDn_delayed
        else
        begin
            cf2ci_C1TxData          <= 0;
            cf2ci_C1TxHdr           <= 0;
            cf2ci_C1TxWrValid       <= 0;
            cf2ci_C0TxHdr           <= 0;
            cf2ci_C0TxRdValid       <= 0;
        end
    
        
        case({cf2ci_C1TxWrValid, cf2ci_C0TxRdValid, rb2cf_C1RxWrValid,(rb2cf_C0RxRdValid|rb2cf_C0RxWrValid)})
            4'b0001:    Num_Pend    <= Num_Pend - 2'h1;
            4'b0010:    Num_Pend    <= Num_Pend - 2'h1;
            4'b0011:    Num_Pend    <= Num_Pend - 2'h2;
            4'b0100:    Num_Pend    <= Num_Pend + 2'h1;
            //4'b0101:    
            //4'b0110:
            4'b0111:    Num_Pend    <= Num_Pend - 2'h1;
            4'b1000:    Num_Pend    <= Num_Pend + 2'h1;
            //4'b1001:    
            //4'b1010:
            4'b1011:    Num_Pend    <= Num_Pend - 2'h1;
            4'b1100:    Num_Pend    <= Num_Pend + 2'h2;
            4'b1101:    Num_Pend    <= Num_Pend + 2'h1;
            4'b1110:    Num_Pend    <= Num_Pend + 2'h1;
            //4'b1111:
        endcase                 
                                                                                     // - Response Path
                                                                                     //------------------------------------
        if(!WrHdr_valid)
            WrHdr_valid   <= fifo_RdEn;
    
        rb2cf_C0RxData_q       <= rb2cf_C0RxData;
        rb2cf_C0RxHdr_q        <= rb2cf_C0RxHdr;
        rb2cf_C0RxWrValid_q    <= rb2cf_C0RxWrValid;
        rb2cf_C0RxRdValid_q    <= rb2cf_C0RxRdValid;
        rb2cf_C0RxUMsgValid_q  <= rb2cf_C0RxUMsgValid;
        rxfifo_RdEn_q          <= rxfifo_RdEn;
    
                                                                                     // Error Detection Logic
                                                                                     //--------------------------
        if(Num_Pend<0)
        begin
              ErrorVector[0]  <= 1;
              /*synthesis translate_off */
              $display("bm_top: Error: unexpected Rx response");
              /*synthesis translate_on */
        end
    
        if(rxfifo_Full & rxfifo_WrEn)
        begin
              ErrorVector[1]  <= 1;
              /*synthesis translate_off */
              $display("bm_top: Error: WrRx fifo overflow");
              /*synthesis translate_on */
        end
              
        if(fifo_Full & fifo_WrEn)
        begin
              ErrorVector[2]  <= 1;
              /*synthesis translate_off */
              $display("bm_top: Error: wr fifo overflow");
              /*synthesis translate_on */
        end
                 
        /* synthesis translate_off */
        if(cf2ci_C1TxWrValid)
              $display("*Req Type: %x \t Addr: %x \n Data: %x", cf2ci_C1TxHdr[55:52], cf2ci_C1TxHdr[45:14], cf2ci_C1TxData);
    
        if(cf2ci_C0TxRdValid)
              $display("*Req Type: %x \t Addr: %x", cf2ci_C0TxHdr[55:52], cf2ci_C0TxHdr[45:14]);
    
        /* synthesis translate_on */
              
              
        // Use for Debug- if no transactions going across the CCI interface # clks > inactivity threshold 
        // than set the flag.
        if(cf2ci_C1TxWrValid || cf2ci_C0TxRdValid)
            inact_cnt  <= 0;
        else if(ci2sl_InitDn_delayed && !status_write)
            inact_cnt  <= inact_cnt + 1;
    
        if(inact_timeout==0)
        begin
            if(inact_cnt>=cr_inact_thresh)
                inact_timeout   <= 1;
        end
        else if(cf2ci_C1TxWrValid || cf2ci_C0TxRdValid)
        begin
                inact_timeout   <= 0;
        end
    
        if(!test_Resetb)
        begin
                Num_Pend                <= 0;
                Num_ticks               <= 0;
                WrHdr_valid             <= 0;
                ci2sl_InitDn_delayed    <= 0;
                ctr                     <= 0;
                rb2cf_C0RxData_q        <= 0;
                rb2cf_C0RxHdr_q         <= 0;
                rb2cf_C0RxWrValid_q     <= 0;
                rb2cf_C0RxRdValid_q     <= 0;
                rb2cf_C0RxUMsgValid_q   <= 0;
                ErrorVector             <= 0;
                status_write            <= 0;
                inact_cnt               <= 0;
                inact_timeout           <= 0;
                delay_lfsr              <= 1;
                rxfifo_RdEn_q           <= 0;
                Num_C0stall             <= 0;
                Num_C1stall             <= 0;
        end
        if(!Resetb)
        begin
                afuid_updtd             <= 0;
                dsm_base_valid_q        <= 0;
        end
    end
    
    always @(posedge Clk_32UI)                                                      // Computes BM start and end overheads
    begin                                                                           //-------------------------------------
        if(!test_go)
        begin
            penalty_start   <= 0;
            penalty_start_f <= 0;
            penalty_end     <= 2;
        end
        else
        begin
            if(!penalty_start_f & (cf2ci_C0TxRdValid | cf2ci_C1TxWrValid | cf2ci_C1TxIntrValid))
            begin
                penalty_start_f   <= 1;
                penalty_start     <= Num_ticks[7:0];                    /* synthesis translate_off */
                $display ("BM_INFO : start penalty = %d ", Num_ticks); /* synthesis translate_on */
            end
            
            penalty_end <= penalty_end + 1'b1;
            if(rb2cf_C0RxWrValid | rb2cf_C0RxRdValid | rb2cf_C0RxCfgValid | rb2cf_C0RxUMsgValid | rb2cf_C0RxIntrValid 
             | rb2cf_C1RxWrValid | rb2cf_C1RxIntrValid )
            begin
                penalty_end     <= 2;
            end
                                                                            
            if(ab2re_TestCmp
               && !ci2cf_C1TxAlmFull_reg2
               && !status_write)
            begin                                                       /* synthesis translate_off */
                $display ("BM_INFO : end penalty = %d ", penalty_end); /* synthesis translate_on */
            end
                                                                                
        end
    end
    
    always @(*)
    begin
        RdHdr_valid = ci2sl_InitDn_delayed
                      && !status_write
                      && !ci2cf_C0TxAlmFull_reg2
                      && ab2re_RdEn;

        re2ab_RdSent= RdHdr_valid;
    
        if(WrHdr_valid)
        begin
            fifo_RdEn   = !fifo_Empty
                        &&!status_write
                        &&!ci2cf_C1TxAlmFull_reg2;
        end
        else
        begin
            fifo_RdEn   = !fifo_Empty
                        &&!status_write;
        end
    end
    
    //----------------------------------------------------------------------------------------------------------------------------------------------
    //                                                              Instances
    //----------------------------------------------------------------------------------------------------------------------------------------------
    
    
    wire [PEND_THRESH-1:0] fifo_count;
    
    nlb_sb_gfifo  #(.DATA_WIDTH  (1+512+ADDR_LMT+14),
                    .DEPTH_BASE2 (PEND_THRESH),
                    .FULL_THRESH (2**PEND_THRESH-3)  
                   )nlb_writeTx_fifo      // Write request fifo. This is required to break the dependency between Tx and Rx paths
                    (                 //--------------------- Input  ------------------
                        test_Resetb            ,
                        Clk_32UI               ,    
                        fifo_Din               ,          
                        fifo_WrEn              ,      
                        fifo_RdEn              ,         
                                           //--------------------- Output  ------------------
                        fifo_Dout            ,        
                        fifo_Empty           ,
                        fifo_Full            ,
                        fifo_count,
                        fifo_almfull
                    ); 
    
    assign  rxfifo_RdEn     = ~rb2cf_C0RxWrValid && ~rxfifo_Empty;
     
    nlb_sb_gfifo  #(.DATA_WIDTH  ('d14),
                    .DEPTH_BASE2 (PEND_THRESH)
                   )nlb_writeRx_fifo  // We could receive two write responses per clock. This fifo will store the second write response
                    (                 //--------------------- Input  ------------------
                        test_Resetb              ,
                        Clk_32UI                 ,    
                        rxfifo_Din               ,          
                        rxfifo_WrEn              ,      
                        rxfifo_RdEn              ,         
                                           //--------------------- Output  ------------------
                        rxfifo_Dout            ,        
                        rxfifo_Empty           ,
                        rxfifo_Full            ,
                        rxfifo_count,
                        rxfifo_almfull
                    ); 
    
    
    // Function: Returns physical address for a DSM register
    function automatic [31:0] dsm_offset2addr;
    input    [9:0]  offset_b;
    input    [63:0] base_b;
    begin
            dsm_offset2addr = base_b[37:6] + offset_b[9:6];
    end
    endfunction    
endmodule
