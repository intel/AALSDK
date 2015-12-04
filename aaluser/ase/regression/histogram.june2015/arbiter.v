// ***************************************************************************
//
//        Copyright (C) 2008-2013 Intel Corporation All Rights Reserved.
//
// Engineer:            Narayanan Ravichandran, Pratik Marolia
// Create Date:         Thu Nov 20 11:28:01 PDT 2014
// Module Name:         arbiter.v
// Project:             mb_2_3 AFU 
//
// ***************************************************************************
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------
//                                         Arbiter
//  ------------------------------------------------------------------------------------------------------------------------------------------------
//
// This module instantiates different test AFUs, and connect them up to the arbiter.

module arbiter #(parameter PEND_THRESH=1, ADDR_LMT=32, MDATA=14, INSTANCE=2)
(
       // ---------------------------global signals-------------------------------------------------
       Clk_32UI               ,        // in    std_logic;  -- Core clock
       Resetb                 ,        // in    std_logic;  -- Use SPARINGLY only for control

       ab2re_WrAddr,                   // [ADDR_LMT-1:0]        app_cnt:           write address
       ab2re_WrTID,                    // [13:0]                app_cnt:           meta data
       ab2re_WrDin,                    // [511:0]               app_cnt:           Cache line data
       ab2re_WrFence,                  //                       app_cnt:           write fence
       ab2re_WrEn,                     //                       app_cnt:           write enable
       re2ab_WrSent,                   //                       app_cnt:           write issued
       re2ab_WrAlmFull,                //                       app_cnt:           write fifo almost full
       
       ab2re_RdAddr,                   // [ADDR_LMT-1:0]        app_cnt:           Reads may yield to writes
       ab2re_RdTID,                    // [13:0]                app_cnt:           meta data
       ab2re_RdEn,                     //                       app_cnt:           read enable
       re2ab_RdSent,                   //                       app_cnt:           read issued

       re2ab_RdRspValid,               //                       app_cnt:           read response valid
       re2ab_UMsgValid,                //                       arbiter:           UMsg valid
       re2ab_RdRsp,                    // [13:0]                app_cnt:           read response header
       re2ab_RdData,                   // [511:0]               app_cnt:           read data
       
       re2ab_WrRspValid,               //                       app_cnt:           write response valid
       re2ab_WrRsp,                    // [ADDR_LMT-1:0]        app_cnt:           write response header
    
	   re2xy_go,                       //                       requestor:         start the test
       re2xy_src_addr,                 // [31:0]                requestor:         src address
       re2xy_src_addr_sw,              // [31:0]                requestor:         src address
       re2xy_dst_addr_sw,              // [31:0]                requestor:         destination address
       re2xy_NumLines,                 // [31:0]                requestor:         number of cache lines for rd test
       re2xy_NumLines_sw,              // [31:0]                requestor:         number of cache lines for sw test
	   re2xy_NumInst_sw,   			   // [5:0]			   		requestor:	   	   number of instances - SW test			
	   re2xy_Numrepeat_sw,             // [16:6] 			    requestor:	   	   number of times to run each instance of SW test before completion.
	   re2xy_Cont,                     //                       requestor:         read test continuous mode
	   re2xy_test_cfg,                 // [7:0]                 requestor:         8-bit test cfg register.
       ab2re_TestCmp,                  //                       arbiter:           Test completion flag
       test_Resetb,                    //                       requestor:         rest the app
	   re2xy_disable_rd,			   //									   	   Disable Rd test						
	   re2xy_disable_sw			   	   //									   	   Disable SW test	
);
   
   input                   Clk_32UI;               //                      csi_top:            Clk_32UI
   input                   Resetb;                 //                      csi_top:            system Resetb
   
   output [ADDR_LMT-1:0]   ab2re_WrAddr;           // [ADDR_LMT-1:0]        app_cnt:           Writes are guaranteed to be accepted
   output [13:0]           ab2re_WrTID;            // [13:0]                app_cnt:           meta data
   output [511:0]          ab2re_WrDin;            // [511:0]               app_cnt:           Cache line data
   output                  ab2re_WrFence;          //                       app_cnt:           write fence.
   output                  ab2re_WrEn;             //                       app_cnt:           write enable
   input                   re2ab_WrSent;           //                       app_cnt:           write issued
   input                   re2ab_WrAlmFull;        //                       app_cnt:           write fifo almost full
   
   output [ADDR_LMT-1:0]   ab2re_RdAddr;           // [ADDR_LMT-1:0]        app_cnt:           Reads may yield to writes
   output [13:0]           ab2re_RdTID;            // [13:0]                app_cnt:           meta data
   output                  ab2re_RdEn;             //                       app_cnt:           read enable
   input                   re2ab_RdSent;           //                       app_cnt:           read issued
   
   input                   re2ab_RdRspValid;       //                       app_cnt:           read response valid
   input                   re2ab_UMsgValid;        //                       arbiter:           UMsg valid
   input [13:0]            re2ab_RdRsp;            // [13:0]                app_cnt:           read response header
   input [511:0]           re2ab_RdData;           // [511:0]               app_cnt:           read data
   
   input                   re2ab_WrRspValid;       //                       app_cnt:           write response valid
   input [13:0]            re2ab_WrRsp;            // [13:0]                app_cnt:           write response header
   
   input                   re2xy_go;               //                       requestor:         start of frame recvd
   input [31:0]            re2xy_src_addr;         // [31:0]                requestor:         src address
   input [31:0]            re2xy_src_addr_sw;      // [31:0]                requestor:         src address
   input [31:0]            re2xy_dst_addr_sw;      // [31:0]                requestor:         destination address
   input [31:0]            re2xy_NumLines;         // [31:0]                requestor:         number of cache lines for rd test
   input [31:0]            re2xy_NumLines_sw;      // [31:0]                requestor:         number of cache lines for sw test
   input [5:0]			   re2xy_NumInst_sw;   	   // [5:0]				    requestor:		   number of instances - SW test
   input [20:0]			   re2xy_Numrepeat_sw;     // [16:6] 				requestor:		   number of times to run each instance of SW test before completion.
   input                   re2xy_Cont;             //                       requestor:         rd test continuous mode
   input [7:0]             re2xy_test_cfg;         // [7:0]                 requestor:         8-bit test cfg register.
   output                  ab2re_TestCmp;          //                       arbiter:           Test completion flag 
   input                   test_Resetb;
   input 				   re2xy_disable_rd;	   //									   	   Disable Rd test						
   input			       re2xy_disable_sw;       //									   	   Disable SW test
   
   //------------------------------------------------------------------------------------------------------------------------
   wire                    Clk_32UI;               //                      csi_top:            Clk_32UI
   wire                    Resetb;                 //                      csi_top:            system Resetb
   
   reg [ADDR_LMT-1:0]      ab2re_WrAddr;           // [ADDR_LMT-1:0]        app_cnt:           Writes are guaranteed to be accepted
   reg [13:0]              ab2re_WrTID;            // [13:0]                app_cnt:           meta data
   reg [511:0]             ab2re_WrDin;            // [511:0]               app_cnt:           Cache line data
   reg                     ab2re_WrEn;             //                       app_cnt:           write enable
   reg                     ab2re_WrFence;          //
   wire                    re2ab_WrSent;           //                       app_cnt:           write issued
   wire                    re2ab_WrAlmFull;        //                       app_cnt:           write fifo almost full
   
   reg [ADDR_LMT-1:0]      ab2re_RdAddr;           // [ADDR_LMT-1:0]        app_cnt:           Reads may yield to writes
   reg [13:0]              ab2re_RdTID;            // [13:0]                app_cnt:           meta data
   reg                     ab2re_RdEn;             //                       app_cnt:           read enable
   wire                    re2ab_RdSent;           //                       app_cnt:           read issued
   
   wire                    re2ab_RdRspValid;       //                       app_cnt:           read response valid
   wire                    re2ab_UMsgValid;        //                       app_cnt:           UMsg valid
   wire [13:0]             re2ab_RdRsp;            // [13:0]                app_cnt:           read response header
   wire [511:0]            re2ab_RdData;           // [511:0]               app_cnt:           read data
     
   wire                    re2ab_WrRspValid;       //                       app_cnt:           write response valid
   wire [13:0]             re2ab_WrRsp;            // [13:0]                app_cnt:           write response header
   
   wire                    re2xy_go;               //                       requestor:         start of frame recvd
   wire [31:0]             re2xy_NumLines;         // [31:0]                requestor:         number of cache lines for rd test
   wire [31:0]             re2xy_NumLines_sw;      // [31:0]                requestor:         number of cache lines for sw test
   wire                    re2xy_Cont;             //                       requestor:         rd test continuous mode
   reg                     ab2re_TestCmp;          //                       arbiter:           Test completion flag   
   wire                    test_Resetb;
   wire 				   re2xy_disable_rd;	   //									   	   Disable Rd test						
   wire				       re2xy_disable_sw;	   //									   	   Disable SW test

   //------------------------------------------------------------------------------------------------------------------------
   //      test_rdwr signal declarations
   //------------------------------------------------------------------------------------------------------------------------
   
   wire [ADDR_LMT-1:0]     rw2ab_RdAddr;           // [ADDR_LMT-1:0]        app_cnt:           Reads may yield to writes
   wire [13:0]             rw2ab_RdTID;            // [13:0]                app_cnt:           meta data
   wire                    rw2ab_RdEn;             //                       app_cnt:           read enable
   reg                     ab2rw_RdSent;           //                       app_cnt:           read issued
   
   reg                     ab2rw_RdRspValid;       //                       app_cnt:           read response valid
   reg [13:0]              ab2rw_RdRsp;            // [13:0]                app_cnt:           read response header
   reg [ADDR_LMT-1:0]      ab2rw_RdRspAddr;        // [ADDR_LMT-1:0]        app_cnt:           read response address
   reg [511:0]             ab2rw_RdData;           // [511:0]               app_cnt:           read data  
   wire                    rw2ab_TestCmp;          //                       arbiter:           Test completion flag
   
   //------------------------------------------------------------------------------------------------------------------------
   //      test_sw1 signal declarations
   //------------------------------------------------------------------------------------------------------------------------  
   wire [ADDR_LMT-1:0]     s12ab_WrAddr[0:INSTANCE-1];           // [ADDR_LMT-1:0]        app_cnt:           write address
   wire [13:0]             s12ab_WrTID[0:INSTANCE-1];            // [13:0]                app_cnt:           meta data
   wire [511:0]            s12ab_WrDin[0:INSTANCE-1];            // [511:0]               app_cnt:           Cache line data
   wire [INSTANCE-1:0]     s12ab_WrEn;             				 //                       app_cnt:           write enable
   wire [INSTANCE-1:0]     s12ab_WrFence;          				 //                       app_cnt:           write fence 
   reg  				   ab2s1_WrSent[0:INSTANCE-1];			 //                       app_cnt:           write issued
   reg                     ab2s1_WrAlmFull;        			   	 //                       app_cnt:           write fifo almost full
 
   wire [ADDR_LMT-1:0]     s12ab_RdAddr[0:INSTANCE-1];           // [ADDR_LMT-1:0]        app_cnt:           Reads may yield to writes
   wire [13:0]             s12ab_RdTID[0:INSTANCE-1];            // [13:0]                app_cnt:           meta data
   wire [INSTANCE-1:0]     s12ab_RdEn;             				 //                       app_cnt:           read enable
   reg       			   ab2s1_RdSent[0:INSTANCE-1];           //                       app_cnt:           read issued
			
   reg       			   ab2s1_RdRspValid[0:INSTANCE-1];       //                       app_cnt:           read response valid
   reg                     ab2s1_UMsgValid;        			     //                       app_cnt:           UMsg valid
   
   reg  [13:0]             ab2s1_RdRsp[0:INSTANCE-1];            // [13:0]                app_cnt:           read response header
   reg  [ADDR_LMT-1:0]     ab2s1_RdRspAddr[0:INSTANCE-1];        // [ADDR_LMT-1:0]        app_cnt:           read response address
   reg  [511:0]            ab2s1_RdData[0:INSTANCE-1];           // [511:0]               app_cnt:           read data
   
   reg       			   ab2s1_WrRspValid[0:INSTANCE-1];       //                       app_cnt:           write response valid
   reg  [13:0]             ab2s1_WrRsp[0:INSTANCE-1];            // [13:0]                app_cnt:           write response header
   reg  [ADDR_LMT-1:0]     ab2s1_WrRspAddr[0:INSTANCE-1];        // [Addr_LMT-1:0]        app_cnt:           write response address
   
   wire      			   s12ab_TestCmp[0:INSTANCE-1];          //                       arbiter:           Test completion flag
   reg 					   sw_test_enable[0:INSTANCE-1]; 		 //      	                 
   wire [INSTANCE-1:0] 	   s12ab_Hist_Ctrl;                      //                       app_cnt: 			 Control signal from SW to tell Arbitrar when
																 // 										 to write into Histogram Workspace
   // local variables
   reg  [INSTANCE-1:0]     test_comp; 		     				 //    
   reg 					   re2xy_go_reg1;
   reg 					   re2xy_go_reg2;
   reg  [5:0] 		 	   re2xy_NumInst_sw_reg;
   reg 					   sw_test_enable_reg[0:INSTANCE-1];
   wire 				   sw_test_comp;				
   wire 				   rd_test_enable = re2xy_Cont ? (!sw_test_comp & re2xy_go_reg1 & !re2xy_disable_rd):(re2xy_go_reg1 & !re2xy_disable_rd);
   wire [INSTANCE:0]	   read_in_valid;
   wire [INSTANCE-1:0]     write_in_valid;		
  
   reg                     re2ab_RdRspValid_q;
   reg                     re2ab_WrRspValid_q;
   reg                     re2ab_UMsgValid_q;
   reg [13:0]              re2ab_RdRsp_q;
   reg [13:0]              re2ab_WrRsp_q;
   reg [511:0]             re2ab_RdData_q;

   localparam NUM_INPUTS_READ  		= INSTANCE+1;	   
   localparam LOG2_NUM_READ  		= $clog2(NUM_INPUTS_READ);
   localparam NUM_INPUTS_WRITE 		= INSTANCE;	   
   localparam LOG2_NUM_WRITE   		= $clog2(NUM_INPUTS_WRITE);
   
   wire [LOG2_NUM_READ-1:0]  read_fair_arb_out;
   wire [LOG2_NUM_WRITE-1:0] write_fair_arb_out;
   wire					     write_out_valid;
   wire					     read_out_valid;
   

//------------------------------------------------------------------------------------------------------------------------
// Arbitrataion Memory instantiation
//------------------------------------------------------------------------------------------------------------------------
   wire [ADDR_LMT-1:0]     arbmem_rd_dout;
   wire [ADDR_LMT-1:0]     arbmem_wr_dout;
   	
   nlb_gram_sdp #(.BUS_SIZE_ADDR(MDATA),
              .BUS_SIZE_DATA(ADDR_LMT),
              .GRAM_MODE(2'd1)
              )arb_rd_mem 
            (
                .clk  (Clk_32UI),
                .we   (ab2re_RdEn),        
                .waddr(ab2re_RdTID[MDATA-1:0]),     
                .din  (ab2re_RdAddr),       
                .raddr(re2ab_RdRsp[MDATA-1:0]),     
                .dout (arbmem_rd_dout )
            );     
   
   nlb_gram_sdp #(.BUS_SIZE_ADDR(MDATA),
              .BUS_SIZE_DATA(ADDR_LMT),
              .GRAM_MODE(2'd1)
             )arb_wr_mem 
            (
                .clk  (Clk_32UI),
                .we   (ab2re_WrEn),        
                .waddr(ab2re_WrTID[MDATA-1:0]),     
                .din  (ab2re_WrAddr),       
                .raddr(re2ab_WrRsp[MDATA-1:0]),     
                .dout (arbmem_wr_dout )
            );     
   
//------------------------------------------------------------------------------------------------------------------------
    
	always @(posedge Clk_32UI)
     begin
        if(~test_Resetb)
          begin
             re2ab_RdRspValid_q      <= 0;
             re2ab_UMsgValid_q       <= 0;
             re2ab_RdRsp_q           <= 0;
             re2ab_RdData_q          <= 0;
             re2ab_WrRspValid_q      <= 0;
             re2ab_WrRsp_q           <= 0;
          end
        else
          begin
             re2ab_RdRspValid_q      <= re2ab_RdRspValid;
             re2ab_UMsgValid_q       <= re2ab_UMsgValid;
             re2ab_RdRsp_q           <= re2ab_RdRsp;
             re2ab_RdData_q          <= re2ab_RdData;
             re2ab_WrRspValid_q      <= re2ab_WrRspValid;
             re2ab_WrRsp_q           <= re2ab_WrRsp;
          end
    end
	
    reg [5:0] k; 	
	always@(posedge Clk_32UI) 
	begin
		if(~test_Resetb) 
		begin
			re2xy_NumInst_sw_reg <= 0;
			re2xy_go_reg1 		 <= 1'b0;
			re2xy_go_reg2 		 <= 1'b0;
			
			for (k=0; k<INSTANCE; k=k+1) 
			begin
			sw_test_enable_reg[k]<= 1'b0; 
			end
		end
			
		else
		begin
			re2xy_NumInst_sw_reg <= re2xy_NumInst_sw;
			re2xy_go_reg1 	     <= re2xy_go;
			re2xy_go_reg2 	     <= re2xy_go_reg1;
			
			for (k=0; k<INSTANCE; k=k+1)  
			begin
			sw_test_enable_reg[k]<= sw_test_enable[k]; 
			end
		end
	end
	
	// Enable Instances of SW test
	reg [5:0] i;
	always@(*)
	begin
		for (i=0; i<INSTANCE; i=i+1) 
		begin
			if(re2xy_go_reg1 && !re2xy_disable_sw && ((i+1)<=re2xy_NumInst_sw_reg[5:0])) 
			begin
				sw_test_enable[i]  = 1'b1; 
			end
		
			else
			begin
				sw_test_enable[i] = 1'b0;
			end
		end
	end
 
	// SW test completion
	always@(*)
	begin
		for (i=0; i<INSTANCE; i=i+1) 
		begin	
			case ({sw_test_enable_reg[i],s12ab_TestCmp[i]})	 /* synthesis parallel_case */
				2'b10  : test_comp[i] = 1'b0;
				2'b11  : test_comp[i] = 1'b1;
				2'b01  : test_comp[i] = 1'b1;
				2'b00  : test_comp[i] = 1'b1;
			endcase	
		end
	end
	assign sw_test_comp = re2xy_go_reg2 ? &test_comp : 0;
 
//------------------------------------------------------------------------------------------------------------------------
// Fair Arbiter instantiation
//------------------------------------------------------------------------------------------------------------------------    
    assign read_in_valid  = {rw2ab_RdEn,s12ab_RdEn[INSTANCE-1:0]};   
    assign write_in_valid = {s12ab_WrEn[INSTANCE-1:0]}; 
   
	fair_arbiter #(.NUM_INPUTS(NUM_INPUTS_READ), 
				   .LNUM_INPUTS(LOG2_NUM_READ)
				  )
	read_fair_arbiter(
		Clk_32UI,
		test_Resetb,
		read_in_valid,
		read_fair_arb_out,
		read_out_valid
	);

	fair_arbiter #(.NUM_INPUTS(NUM_INPUTS_WRITE), 
				   .LNUM_INPUTS(LOG2_NUM_WRITE)
				  )
	write_fair_arbiter(
		Clk_32UI,
		test_Resetb,
		write_in_valid,
		write_fair_arb_out,
		write_out_valid
	);
//------------------------------------------------------------------------------------------------------------------------												 
		
		always@(*)
		begin  
			// Ouputs from arbiter to requestor 
			ab2re_RdAddr    = 0;
			ab2re_RdTID     = 0;
			ab2re_WrAddr    = 0;
			ab2re_WrTID     = 0;
			ab2re_WrDin     = 0;
			ab2re_WrFence   = 0;			
			ab2re_TestCmp   = 0;
			ab2re_RdEn      = read_out_valid; 
			ab2re_WrEn      = write_out_valid;	
			
			// Compute overall Test completion
			case ({re2xy_disable_rd,re2xy_disable_sw})	 /* synthesis parallel_case */
				2'b10  : ab2re_TestCmp = sw_test_comp;
				2'b01  : ab2re_TestCmp = rw2ab_TestCmp;
				2'b00  : ab2re_TestCmp = (rw2ab_TestCmp | re2xy_Cont) & sw_test_comp;
				default: ab2re_TestCmp = 1'b1;
			endcase			
			
			// Inputs from arbiter to the Read test
			ab2rw_RdSent    = 0;
			ab2rw_RdRspValid= 0;
			ab2rw_RdRsp     = 0;
			ab2rw_RdRspAddr = 0;
			ab2rw_RdData    = 'hx;
		
			// Read Responses to Read test
			if(re2ab_RdRsp_q[13]==1'b1) 
			begin 
				ab2rw_RdRspValid   = re2ab_RdRspValid_q;
				ab2rw_RdRsp        = re2ab_RdRsp_q;
				ab2rw_RdRspAddr    = arbmem_rd_dout;
				ab2rw_RdData       = re2ab_RdData_q;
			end
	
			// Writes routed to sw test		
			ab2s1_UMsgValid    = re2ab_UMsgValid_q; 	
			ab2s1_WrAlmFull    = re2ab_WrAlmFull;	
					
			for (i=0; i<INSTANCE; i=i+1) 
			begin		
			
					// Outputs from read test to arbiter: Read Requests (No Write Requests possible)
					if(read_fair_arb_out==i+1) 
					begin
						ab2re_RdAddr       = rw2ab_RdAddr + re2xy_src_addr;		
						ab2re_RdTID        = rw2ab_RdTID; 
						ab2rw_RdSent	   = re2ab_RdSent;
					end	
					
					// Outputs from Arbiter to sw Tests : Responses
					ab2s1_WrSent[i]    = 0;
					ab2s1_RdSent[i]    = 0;
					ab2s1_RdRspValid[i]= 0;
					ab2s1_RdRsp[i]     = 0;
					ab2s1_RdRspAddr[i] = 0;
					ab2s1_RdData[i]    = 'hx;
					ab2s1_WrRspValid[i]= 0;
					ab2s1_WrRsp[i]     = 0;
					ab2s1_WrRspAddr[i] = 0;
													
					// Outputs from sw test to arbiter : Read Requests  		
					if(read_fair_arb_out==i)	
					begin
						ab2re_RdAddr       = s12ab_RdAddr[i] + re2xy_src_addr_sw;  
						ab2re_RdTID    	   = s12ab_RdTID[i];
						ab2s1_RdSent[i]    = re2ab_RdSent;
					end	
					
					// Write Requests					
					if(write_fair_arb_out==i)
					begin
						//ab2re_WrAddr 	   = s12ab_RdAddr[i] + re2xy_dst_addr_sw;
						ab2re_WrAddr       = s12ab_Hist_Ctrl[i]? s12ab_WrAddr[i] + re2xy_src_addr_sw : s12ab_WrAddr[i] + re2xy_dst_addr_sw;
						ab2re_WrTID        = s12ab_WrTID[i];
						ab2re_WrDin        = s12ab_WrDin[i];
						ab2s1_WrSent[i]    = re2ab_WrSent;
					end
					
					// Read Responses to sw test 
					if(re2ab_RdRsp_q[13:8]==i || ab2s1_UMsgValid)  
					begin
						ab2s1_RdRspValid[i]   = re2ab_RdRspValid_q;
						ab2s1_RdRsp[i]        = re2ab_RdRsp_q;
						ab2s1_RdRspAddr[i]    = arbmem_rd_dout;
						ab2s1_RdData[i]       = re2ab_RdData_q;
					end	
					
					// Write Responses to sw test
					if(re2ab_WrRsp_q[13:8]==i)
					begin
						ab2s1_WrRspValid[i]   = re2ab_WrRspValid_q;
						ab2s1_WrRsp[i]        = re2ab_WrRsp_q;
						ab2s1_WrRspAddr[i]    = arbmem_wr_dout;
					end	
			end		
		end 	
		
//--------------------------------------------------------------------------------------------------------------
// SW Test Instantiation 
//-------------------------------------------------------------------------------------------------------------- 
genvar j;
generate for (j=0; j<INSTANCE; j=j+1) 
	begin									:test_sw1_gen
	test_sw1  #(.PEND_THRESH(PEND_THRESH),
				.ADDR_LMT   (ADDR_LMT),
				.MDATA      (MDATA),
				.INSTANCE   (j)
				)
		
	my_test_sw1(
			   Clk_32UI               ,        // in    std_logic;  -- Core clock
			   Resetb                 ,        // in    std_logic;  -- Use SPARINGLY only for control
		
			   s12ab_WrAddr[j],                   // [ADDR_LMT-1:0]        arb:               write address
			   s12ab_WrTID[j],                    // [ADDR_LMT-1:0]        arb:               meta data
			   s12ab_WrDin[j],                    // [511:0]               arb:               Cache line data
			   s12ab_WrFence[j],                  //                       arb:               write fence 
			   s12ab_WrEn[j],                     //                       arb:               write enable
			   ab2s1_WrSent[j],                   //                       arb:               write issued
			   ab2s1_WrAlmFull,                   //                       arb:               write fifo almost full
			   s12ab_Hist_Ctrl[j],                //                       arb:               Control signal for write to Histogram workspace
			   
			   s12ab_RdAddr[j],                   // [ADDR_LMT-1:0]        arb:               Reads may yield to writes
			   s12ab_RdTID[j],                    // [13:0]                arb:               meta data
			   s12ab_RdEn[j],                     //                       arb:               read enable
			   ab2s1_RdSent[j],                   //                       arb:               read issued
		       
			   
			   ab2s1_RdRspValid[j],               //                       arb:               read response valid
			   ab2s1_UMsgValid,	                  //                       arb:               UMsg valid
			   ab2s1_RdRsp[j],                    // [13:0]                arb:               read response header
			   ab2s1_RdRspAddr[j],                // [ADDR_LMT-1:0]        arb:               read response address
			   ab2s1_RdData[j],                   // [511:0]               arb:               read data
		
			   ab2s1_WrRspValid[j],               //                       arb:               write response valid
			   ab2s1_WrRsp[j],                    // [13:0]                arb:               write response header
			   ab2s1_WrRspAddr[j],                // [ADDR_LMT-1:0]        arb:               write response address
			   
			   sw_test_enable[j],     		  	  //                       requestor:         start the test
			   re2xy_NumLines_sw,                 // [31:0]                requestor:         number of cache lines 
			   re2xy_test_cfg,                    // [7:0]                 requestor:         8-bit test cfg register.
			   re2xy_Numrepeat_sw,				  // [10:0]				   requestor:		  No. of times to repeat the sw test.
		 
			   s12ab_TestCmp[j],                  //                       arb:               Test completion flag
			   test_Resetb,                       //                       requestor:         rest the app
			   (2047*j),					  	  // input to sw [15:0]	   2047*INSTANCE_ID
			   ((2047*32)+j)				      // input to sw [15:0]	   (2047*32)+INSTANCE_ID
		);   
	end 
endgenerate

//--------------------------------------------------------------------------------------------------------------
// Read Streaming Test Instantiation 
//--------------------------------------------------------------------------------------------------------------    
	test_rdwr #(.PEND_THRESH(PEND_THRESH),
					.ADDR_LMT   (ADDR_LMT),
					.MDATA      (MDATA)
				)
	test_rdwr(
			   Clk_32UI,        			   // in    std_logic;  -- Core clock
			   Resetb,        				   // in    std_logic;  -- Use SPARINGLY only for control
						 
			   rw2ab_RdAddr,                   // [ADDR_LMT-1:0]        arb:               Reads may yield to writes
			   rw2ab_RdTID,                    // [13:0]                arb:               meta data
			   rw2ab_RdEn,                     //                       arb:               read enable
			   ab2rw_RdSent,                   //                       arb:               read issued
		
			   ab2rw_RdRspValid,               //                       arb:               read response valid
			   ab2rw_RdRsp,                    // [13:0]                arb:               read response header
			   ab2rw_RdRspAddr,                // [ADDR_LMT-1:0]        arb:               read response address
			   ab2rw_RdData,                   // [511:0]               arb:               read data
		
			   rd_test_enable,  			   //                       requestor:         start the test
			   re2xy_NumLines,                 // [31:0]                requestor:         number of cache lines
			   re2xy_Cont,                     //                       requestor:         continuous mode
		
			   rw2ab_TestCmp,                  //                       arb:               Test completion flag
			   test_Resetb                     //                       requestor:         rest the app
		);
endmodule
 

//--------------------------------------------------------------------------------------------------------------
// Fair Arbiter Module 
//-------------------------------------------------------------------------------------------------------------- 
module fair_arbiter #(parameter NUM_INPUTS=2, LNUM_INPUTS=$clog2(NUM_INPUTS))
(
    clk,
    reset_n,
    in_valid,
    out_select,
    out_valid
);

	input                       clk;
    input                       reset_n;
    input   [NUM_INPUTS-1:0]    in_valid;
    output  [LNUM_INPUTS-1:0]   out_select;
    output  					out_valid;
	
	reg 				  out_valid;
	reg [LNUM_INPUTS-1:0] out_select;
    reg [LNUM_INPUTS-1:0] lsb_select, msb_select;
    reg [NUM_INPUTS-1:0]  lsb_mask;                       // bits [out_select-1:0]='0
    reg [NUM_INPUTS-1:0]  msb_mask;                       // bits [NUM_INPUTS-1:out_select]='0
    reg                   msb_in_notEmpty;
    integer i;
    
	always @(posedge clk)
    begin
        if(out_valid)
        begin
            msb_mask    <= ~({{NUM_INPUTS-1{1'b1}}, 1'b0}<<out_select); 
            lsb_mask    <=   {{NUM_INPUTS-1{1'b1}}, 1'b0}<<out_select;
        end

        if(!reset_n)
        begin
            msb_mask <= {NUM_INPUTS{1'b1}};
            lsb_mask <= {NUM_INPUTS{1'b0}};
        end
    end

    wire    [NUM_INPUTS-1:0]    msb_in = in_valid & lsb_mask;
    wire    [NUM_INPUTS-1:0]    lsb_in = in_valid & msb_mask;
    
    always@(*)
    begin
        msb_in_notEmpty = |msb_in;
        out_valid       = |in_valid;
        lsb_select = 0;
        msb_select = 0;

        for(i=NUM_INPUTS-1; i>=0; i=i-1)
        begin
            if(lsb_in[i])
                lsb_select = i;
            if(msb_in[i])
                msb_select = i;
        end
        out_select = msb_in_notEmpty ? msb_select : lsb_select;
    end
endmodule
