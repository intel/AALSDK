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
       test_Resetb,                    //                       requestor:         rest the app
	   
       l12ab_RdLen,
       l12ab_RdSop,
       l12ab_WrLen,
       l12ab_WrSop,
   	   
       ab2l1_RdRspFormat,
       ab2l1_RdRspCLnum,
       ab2l1_WrRspFormat,
       ab2l1_WrRspCLnum,
       re2xy_multiCL_len
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
	
    output [1:0]            l12ab_RdLen;
    output                  l12ab_RdSop;
    output [1:0]            l12ab_WrLen;
    output                  l12ab_WrSop;
	
    input                   ab2l1_RdRspFormat;
    input  [1:0]            ab2l1_RdRspCLnum;
    input                   ab2l1_WrRspFormat;
    input  [1:0]            ab2l1_WrRspCLnum;

    input  [1:0]            re2xy_multiCL_len;
	
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
    reg                     write_fsm;
    reg     [15:0]          Num_Read_req;
    reg     [15:0]          Num_Write_req;
    reg     [15:0]          Num_Write_rsp;
    reg     [1:0]           l12ab_RdLen;
    reg                     l12ab_RdSop;
    reg     [1:0]           l12ab_WrLen;              
    reg                     l12ab_WrSop;
    
	// ------------------------------------------------------
	// RAM to store RdRsp Data and Address
	// ------------------------------------------------------
	logic [533:0] rdrsp_mem_in;
	logic [533:0] wrreq_mem_out;
	logic [8:0]   memwr_addr;
	logic [8:0]   memrd_addr;
	logic         memwr_en;
    
	// 2 cycle Read latency
	// 2 cycle Rd2Wr latency
	// Unknown data returned if same address is read, written 
    nlb_gram_sdp #(.BUS_SIZE_ADDR(9),
                   .BUS_SIZE_DATA(512+20+2),
                   .GRAM_MODE(2'd3)
                  )
    rdrsp_mem 
    (
      .clk  (Clk_16UI),
      .we   (memwr_en),        
      .waddr(memwr_addr),     
      .din  (rdrsp_mem_in),       
      .raddr(memrd_addr),     
      .dout (wrreq_mem_out)
    );    
    // ------------------------------------------------------
	logic         Wr_go;
	logic         ram_rdValid;
	logic         ram_rdValid_q;
	logic         ram_rdValid_qq;
	logic         wrsop;
	logic         wrsop_q;
	logic         wrsop_qq;
	logic [7:0]   i;
	logic [6:0]   WrReq_tid;
	logic [6:0]   WrReq_tid_mCL;
	logic [1:0]   CL_ID;
	logic [1:0]   wrCLnum;
	logic [1:0]   wrCLnum_q;
	logic [1:0]   wrCLnum_qq;
	logic [2:0]   multiCL_num;
	logic         ab2l1_RdRspValid_q;
	logic [6:0]   ab2l1_RdRsp_q;
	logic [2:0]   count_RdRsp;
	logic [2:0]   numRdRsp_vector_bank[0:15] [0:7];
	logic         numRdRsp_vector_bank_q[0:15] [0:7];
	
    always @(posedge Clk_16UI)
    begin
	  memwr_en                                   <= 0; 
	  // Store RdResponses in RAM and update count vector
	  if(ab2l1_RdRspValid)
      begin
        memwr_addr                               <= {ab2l1_RdRsp[6:0],ab2l1_RdRspCLnum[1:0]}; 
		memwr_en                                 <= 1;
		rdrsp_mem_in                             <= {ab2l1_RdData[511:0],ab2l1_RdRspAddr[19:0],ab2l1_RdRspCLnum[1:0]}; 
      end
	  
	  // 2 cycles to update numRdRsp vector
	  begin
		multiCL_num[2:0]                         <= re2xy_multiCL_len[1:0] + 1;
		
		// Register Read Responses
		ab2l1_RdRspValid_q                       <= ab2l1_RdRspValid;
		ab2l1_RdRsp_q                            <= ab2l1_RdRsp;
				
		// Response Received, Read current count from RdRsp vector
		if (ab2l1_RdRspValid && !ab2l1_RdRspValid_q)  
		begin
          count_RdRsp                            <= numRdRsp_vector_bank[ab2l1_RdRsp[6:3]][ab2l1_RdRsp[2:0]][2:0];		  
		end
		
		// Forwarding value to count while updating RdRsp vector 
		else if (ab2l1_RdRspValid && ab2l1_RdRspValid_q && (ab2l1_RdRsp[6:0] == ab2l1_RdRsp_q[6:0]) )
		begin
		  count_RdRsp                            <= numRdRsp_vector_bank[ab2l1_RdRsp[6:3]][ab2l1_RdRsp[2:0]][2:0] + 1'b1;
		end
				
		// Update RdRsp vector
		if (ab2l1_RdRspValid_q)
	    begin
		  numRdRsp_vector_bank[ab2l1_RdRsp_q[6:3]][ab2l1_RdRsp_q[2:0]][2:0] <= count_RdRsp + 1'b1;
		  if ((count_RdRsp + 1'b1) == multiCL_num[2:0]) 
		  begin
		  numRdRsp_vector_bank_q[ab2l1_RdRsp_q[6:3]][ab2l1_RdRsp_q[2:0]]    <= 1;
		  end
		end        		
	  end
	 	  
	  // Compute next Write Request number and update Write go
	  begin
	    Wr_go                                                    <= numRdRsp_vector_bank_q[WrReq_tid[6:3]][WrReq_tid[2:0]];
		if (Wr_go && !write_fsm)
	    begin		
		  // Store TID for mCL requests and update TID
		  WrReq_tid_mCL                                          <= WrReq_tid[6:0]; 
		  WrReq_tid[6:0]                                         <= WrReq_tid[6:0] + 1'h1;
		  // Clear RdRsp vectors and mdata vector
		  rd_mdata_pend[{WrReq_tid[6:3],WrReq_tid[2:0]}]         <= 1'b0;
		  numRdRsp_vector_bank[WrReq_tid[6:3]][WrReq_tid[2:0]]   <= 3'h0;
		  numRdRsp_vector_bank_q[WrReq_tid[6:3]][WrReq_tid[2:0]] <= 1'h0;
	    end
	  end
      	  
	  // FSM to send mem write requests 
	  // FSM should stall if Writes cannot be accepted by requestor
	  case (write_fsm)   /* synthesis parallel_case */
		  1'h0:
		    begin
			  if (Wr_go)
			  begin
			    // Read first CL of 'num_multi_CL' memWrite requests from RAM
			    write_fsm                        <= 1'h1;
			    CL_ID                            <= CL_ID + 1'b1;
			    memrd_addr                       <= {WrReq_tid[6:0], CL_ID};
			    ram_rdValid                      <= 1;
			    wrsop                            <= 1;
			    wrCLnum                          <= re2xy_multiCL_len[1:0];
			  end
		    end
		  
		  1'h1:
		    begin
		      if (|wrCLnum[1:0])
		      begin
                // Read remaining CLs of 're2xy_multiCL_len' memWrite requests from RAM
			    write_fsm                        <= 1'h1;
			    CL_ID                            <= CL_ID + 1'b1;
			    memrd_addr                       <= {WrReq_tid_mCL[6:0], CL_ID};
			    ram_rdValid                      <= 1;
			    wrsop                            <= 0;
			    wrCLnum                          <= wrCLnum - 1'b1;
			  end
			  
              else
			  begin				 
				// Goto next set of multiCL requests 
				// One cycle bubble between each set of multi CL writes. 
				// TODO: optimize 
			    write_fsm                        <= 1'h0;
			    CL_ID                            <= 0;
			    ram_rdValid                      <= 0;
			    wrsop                            <= 1;
			    wrCLnum                          <= re2xy_multiCL_len[1:0];
			  end
            end
			
		  default:
		  begin
		    write_fsm                            <= write_fsm;
		  end
		endcase  

        // Pipeline WrReq parameters till RAM output is valid
	    ram_rdValid_q                            <= ram_rdValid;
	    ram_rdValid_qq                           <= ram_rdValid_q;
	    wrsop_q                                  <= wrsop;
        wrsop_qq                                 <= wrsop_q;
        wrCLnum_q                                <= wrCLnum;
	    wrCLnum_qq                               <= wrCLnum_q;
	    
	    // send Multi CL Write Requests 
	    l12ab_WrEn                               <= (ram_rdValid_qq == 1'b1); 
	    l12ab_WrAddr                             <= wrreq_mem_out[21:2] + wrreq_mem_out[1:0] ;
        l12ab_WrDin                              <= wrreq_mem_out[533:22];
	    l12ab_WrTID[15:0]                        <= wrreq_mem_out[17:2];
        l12ab_WrSop                              <= wrsop_qq;
        l12ab_WrLen                              <= wrCLnum_qq;
        
	    // Track Num Write requests
	    if (l12ab_WrEn)
        begin
	      Num_Write_req                          <= Num_Write_req   + 1'b1;
        end	  
        
        // Track Num Write responses
        if(ab2l1_WrRspValid && ab2l1_WrRspFormat)   // Packed write response
        begin
          Num_Write_rsp                          <= Num_Write_rsp + 1'b1 + ab2l1_WrRspCLnum; 
        end
	    else if (ab2l1_WrRspValid)                  // unpacked write response
	    begin
	      Num_Write_rsp                          <= Num_Write_rsp + 1'b1;   
	    end
	    
	    // Meta data locked when RdSent
	    if(l12ab_RdEn && ab2l1_RdSent)
        begin
          rd_mdata_pend[rd_mdata]                <= 1'b1;
	    end
	  
	    if (!test_Resetb)
	    begin
		  Wr_go                                  <= 0;
		  memwr_en                               <= 0;  
		  
		  for (i=0; i[7]!=1; i=i+1'b1)
		  begin
	      numRdRsp_vector_bank[i[6:3]][i[2:0]]   <= 0;
		  numRdRsp_vector_bank_q[i[6:3]][i[2:0]] <= 0;
		  end
		  
		  count_RdRsp                            <= 0; 
		  multiCL_num                            <= 1;
		  rd_mdata_pend                          <= 0;
		  write_fsm                              <= 1'h0;
		  WrReq_tid                              <= 0;
		  WrReq_tid_mCL                          <= 0;
		  CL_ID                                  <= 0;
		  ram_rdValid                            <= 0;
		  wrsop                                  <= 1;
 	      wrCLnum                                <= 0;
		  l12ab_WrEn                             <= 0;
		  l12ab_WrSop                            <= 1;
		  l12ab_WrLen                            <= 0;
		  Num_Write_req                          <= 16'h1;
		  Num_Write_rsp                          <= 0;
	    end
	end
	
    always @(posedge Clk_16UI)
    begin
            //Read FSM
            case(read_fsm)  /* synthesis parallel_case */
            2'h0:   
			begin  // Wait for re2xy_go
              l12ab_RdAddr                       <= 0;
              l12ab_RdLen                        <= re2xy_multiCL_len; 
              l12ab_RdSop                        <= 1'b1;
              Num_Read_req                       <= 16'h0 + re2xy_multiCL_len + 1'b1;       // Default is 1 req; implies single CL 
              
			  if(re2xy_go)
                if(re2xy_NumLines!=0)
                  read_fsm                       <= 2'h1;
                else    
				  read_fsm                       <= 2'h2;
            end
            
			2'h1:   
			begin  // Send read requests
              if(ab2l1_RdSent)        
              begin   
                l12ab_RdAddr                     <= l12ab_RdAddr + re2xy_multiCL_len + 1'b1; // multiCL_len = {0/1/2/3}
                l12ab_RdLen                      <= l12ab_RdLen;                             // All reqs are uniform. Based on test cfg
                l12ab_RdSop                      <= 1'b1;                                    // All reqs are uniform. Based on test cfg
                Num_Read_req                     <= Num_Read_req + re2xy_multiCL_len + 1'b1; // final count will be same as re2xy_NumLines   
								                                                 
                if(Num_Read_req >= re2xy_NumLines)
                if(re2xy_Cont)    read_fsm       <= 2'h0;
                else              read_fsm       <= 2'h2;
              end // ab2l1_RdSent
            end
            
			default:              read_fsm       <= read_fsm;
            endcase
            
			if(l12ab_RdEn && ab2l1_RdSent)
            begin
              rd_mdata_next                      <= rd_mdata + 2'h2;
              rd_mdata                           <= rd_mdata_next;
              rd_mdata_avail                     <= !rd_mdata_pend[rd_mdata_next];
            end
            else
            begin
              rd_mdata_avail                     <= !rd_mdata_pend[rd_mdata];
              rd_mdata_next                      <= rd_mdata + 1'h1;
            end
			
            // TODO:			
			if(read_fsm==2'h2 && Num_Write_rsp==re2xy_NumLines)
            begin            
              l12ab_TestCmp                      <= 1'b1;
			end
    
            // Error logic
            if(l12ab_WrEn && ab2l1_WrSent==0)
            begin
              // WrFSM assumption is broken
              $display ("%m LPBK1 test WrEn asserted, but request Not accepted by requestor");
              l12ab_ErrorValid                   <= 1'b1;
              l12ab_ErrorInfo                    <= 1'b1;
            end
           
            if(!test_Resetb)
            begin
//            l12ab_WrAddr                       <= 0;
//            l12ab_RdAddr                       <= 0;
              l12ab_TestCmp                      <= 0;
              l12ab_ErrorInfo                    <= 0;
              l12ab_ErrorValid                   <= 0;
              read_fsm                           <= 0;
              rd_mdata                           <= 0;
              rd_mdata_avail                     <= 1'b1;
              Num_Read_req                       <= 16'h1;
              l12ab_RdLen                        <= 0;
              l12ab_RdSop                        <= 1;
            end
    end
    
    always @(*)
    begin
      l12ab_RdTID = 0;
      l12ab_RdTID[MDATA-1:0] = rd_mdata;
      l12ab_RdEn = (read_fsm  ==2'h1) & !ab2l1_stallRd & rd_mdata_avail;
    end
	
	// synthesis translate_off
	logic numCL_error = 0;
	always @(posedge Clk_16UI)
    begin
	if( re2xy_go && ((re2xy_NumLines)%(re2xy_multiCL_len + 1) != 0)  )
    begin
      $display("%m \m ERROR: Total Num Lines should be exactly divisible by multiCL length");
	  $display("\m re2xy_NumLines = %d and re2xy_multiCL_len = %d",re2xy_NumLines,re2xy_multiCL_len);
      numCL_error <= 1'b1;
    end
	
    if(numCL_error)
    $finish();
	end
    // synthesis translate_on
    
endmodule

