/* ****************************************************************************
 * Copyright(c) 2011-2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither the name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * **************************************************************************
 *
 * Module Info:
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * ASE generics (SystemVerilog header file)
 *
 * Description:
 * This file contains definitions and parameters for the DPI
 * module. The intent of this file is that the user should not modify
 * the DPI source files. **Only** this header file must be modified if
 * any DPI parameters need to be changed.
 *
 */

package ase_pkg;

   // ASE modes
   parameter ASE_MODE_DAEMON          = 1;
   parameter ASE_MODE_TIMEOUT_SIMKILL = 2;
   parameter ASE_MODE_SW_SIMKILL      = 3;
   parameter ASE_MODE_REGRESSION      = 4;
   
   
   // Include platform.vh if not already
 `ifndef _PLATFORM_VH_
  `include "platform.vh"
 `endif

   // Address widths
   parameter PHYSCLADDR_WIDTH   =  42;


   /*
    * CCI specifications
    */
   parameter CCIP_DATA_WIDTH       = 512;
   parameter CCIP_CFG_RDDATA_WIDTH = 64;

   /*
    * Sub-structures
    * Request type, response types, VC types
    */ 
   // Request types {channel_id, ccip_if_pkg::req_type}
   typedef enum logic [3:0] {
			     ASE_RDLINE_S   = 4'h1,
			     ASE_RDLINE_I   = 4'h2,			     
			     ASE_WRLINE_I   = 4'h3,
			     ASE_WRLINE_M   = 4'h4,
			     ASE_WRPUSH     = 4'h5,
			     ASE_WRFENCE    = 4'h6,
			     ASE_INTR_REQ   = 4'h7, 
			     ASE_ATOMIC_REQ = 4'h8 
			     } ccip_reqtype_t;  		

   // Response types
   typedef enum logic [3:0] {
			     ASE_RD_RSP      = 4'h1,			     
			     ASE_WR_RSP      = 4'h2,
			     ASE_INTR_RSP    = 4'h3,
			     ASE_WRFENCE_RSP = 4'h4,
			     ASE_ATOMIC_RSP  = 4'h5,
			     ASE_UMSG        = 4'h6
			     } ccip_resptype_t;
   
   // Virtual channel type
   typedef enum logic [1:0] {
			     VC_VA  = 2'b00,
			     VC_VL0 = 2'b01,
			     VC_VH0 = 2'b10,
			     VC_VH1 = 2'b11
			     } ccip_vc_t;
   
   // Length type
   typedef enum logic [1:0] {
			     ASE_1CL = 2'b00,
			     ASE_2CL = 2'b01,
			     ASE_3CL = 2'b10,
			     ASE_4CL = 2'b11
			     } ccip_len_t;  

   
   /* ***********************************************************
    * CCI-P headers
    * RxHdr, TxHdr, CCIP Packets
    * ***********************************************************/
   // RxHdr
   typedef struct packed {
      //--------- CCIP standard header --------- //      
      ccip_vc_t       vc_used;  // 27:26  // Virtual channel select
      logic           rsvd25;   // 25     // Poison bit // Reserved in BDX-P
      logic           hitmiss;  // 24     // Hit/miss indicator
      logic           format;   // 23     // Multi-CL enable (write packing only)
      logic           rsvd22;   // 22     // X
      ccip_len_t      clnum;    // 21:20  // Cache line number
      ccip_resptype_t resptype; // 19:16  // Response type
      logic [15:0]    mdata;    // 15:0   // Metadata
   } RxHdr_t;
   parameter CCIP_RX_HDR_WIDTH     = $bits(RxHdr_t);
   
   // TxHdr
   typedef struct packed {
      //--------- CCIP standard header --------- //
      logic [79:77]   qword_idx; // 79:77  // Qword start (no end, sets a cmp QW index)
      logic [76:74]   rsvd76_74; // 76:74  // X
      ccip_vc_t       vc;        // 73:72  // Virtual channel select
      logic 	      sop;       // 71     // Start of packet
      logic 	      rsvd70;    // 70     // X
      ccip_len_t      len;       // 69:68  // Length
      ccip_reqtype_t  reqtype;   // 67:64  // Request Type
      logic [5:0]     rsvd63_58; // 63:58  // X
      logic [41:0]    addr;      // 57:16  // Address
      logic [15:0]    mdata;     // 15:0   // Metadata
   } TxHdr_t;
   parameter CCIP_TX_HDR_WIDTH     = $bits(TxHdr_t);

   /* 
    * Config MMIO Header
    */ 			
   // MMIO Header specifics
   parameter int      CCIP_CFGHDR_INDEX_WIDTH = 16;
   parameter int      CCIP_CFGHDR_TID_WIDTH = 9;    
   
   // CfgHdr   
   typedef struct packed {
      logic [15:0] index;  // 27:12
      logic [1:0]  len;    // 11:10
      logic 	   rsvd9;  // 9
      logic [8:0]  tid;    // 8:0
      } CfgHdr_t;
   parameter CCIP_CFG_HDR_WIDTH    = $bits(CfgHdr_t);

   // MMIO header
   typedef struct packed {
      logic [8:0] tid;
      } MMIOHdr_t;
   parameter CCIP_MMIO_TID_WIDTH    = $bits(MMIOHdr_t);

   // Umsg header (received when UMsg is received)
   typedef struct packed {
      logic [1:0] rsvd_27_26;  // 27:26 // Reserved
      logic 	  rsvd25;      // 25    // Poison bit
      logic [4:0] rsvd_24_20;  // 24:20 // Reserved
      logic [3:0] resp_type;   // 19:16 // Response type
      logic       umsg_type;   // 15    // Umsg type
      logic [8:0] rsvd_14_6;   // 14:6  // Reserved
      logic [5:0] umsg_id;     // 5:0   // Umsg Id
   } UMsgHdr_t;
   parameter ASE_UMSG_HDR_WIDTH    = $bits(UMsgHdr_t);

   // CmpXchg header (received from a Compare-Exchange operation)
   typedef struct packed {
      ccip_vc_t       vc_used;    // 27:26
      logic 	      rsvd25;     // 25
      logic 	      hitmiss;    // 24
      logic 	      success;    // 23
      logic [2:0]     rsvd_22_20; // 22:20
      ccip_resptype_t resptype;   // 19:16
      logic [15:0]    mdata;      // 15:0
   } Atomics_t;
   parameter CCIP_CMPXCHG_HDR_WIDTH = $bits(Atomics_t);

   // Config channel
   parameter CCIP_MMIO_ADDR_WIDTH   = 16;
   parameter CCIP_MMIO_INDEX_WIDTH  = 14;
   parameter CCIP_MMIO_RDDATA_WIDTH = 64;

   // Interrupt request header
   typedef struct     packed {
      logic [11:0]    rsvd_79_68; // 79:68 // Reserved
      ccip_reqtype_t  req_type;   // 67:64 // Type
      logic [60:0]    rsvd_63_3;  // 63:3  // reserved
      logic [2:0]     id;         // 2:0   // Intr vector 
   } IntrReq_t;
         
   // Interrupt response header
   typedef struct     packed {
      logic [7:0]     rsvd1;      // 27:20 // reserved, don't care
      ccip_resptype_t resp_type;  // 19:16 // Response type
      logic [12:0]    rsvd_15_3;  // 15:3  // reserved, don't care
      logic [2:0]     id;         // 2:0   // Vector    
   } IntrRsp_t;
   
   
   /* **********************************************************
    * Wrapped headers with channel Id
    * ASE's internal datatype used for bookkeeping
    * *********************************************************/  
   // Wrap TxHdr_t with channel_id
   typedef struct packed {
      logic 	  channel_id;
      TxHdr_t     txhdr;
   } ASETxHdr_t;
   parameter ASE_TX_HDR_WIDTH     = $bits(ASETxHdr_t);
      
   // Wrap RxHdr_t with channel_id
   typedef struct packed {
      logic 	  channel_id;
      RxHdr_t     rxhdr;
   } ASERxHdr_t;
   parameter ASE_RX_HDR_WIDTH     = $bits(ASERxHdr_t);
   

   /*
    * FIFO depth bit-width
    * Enter 'n' here, where n = log_2(FIFO_DEPTH) & n is an integer
    */
   parameter ASE_FIFO_DEPTH_NUMBITS = 8;


   /*
    * Latency Scoreboard generics
    */
   // Number of transactions in latency scoreboard
   parameter LATBUF_NUM_TRANSACTIONS = 8;
   // Radix of latency scoreboard radix
   parameter LATBUF_COUNT_WIDTH      = $clog2(LATBUF_NUM_TRANSACTIONS) + 1;
   // ASE_fifo full threshold inside latency scoreboard
   parameter LATBUF_FULL_THRESHOLD   = 5;
   // Radix of ASE_fifo (subcomponent in latency scoreboard)
   parameter LATBUF_DEPTH_BASE2      = $clog2(LATBUF_NUM_TRANSACTIONS);
   // Maximum transactions per MCL request
   // parameter LATBUF_MCL_MAXLEN       = 4;
   // Wait station timer width
   parameter TIMER_WIDTH             = 9;
   

   /*
    * CCI Transaction packet
    */
   typedef struct {
      int 	  mode;
      int 	  qw_start;
      int 	  mdata;
      longint 	  cl_addr;
      longint     qword[8];
      int 	  resp_channel;
      int 	  success;
   } cci_pkt;

   parameter CCIPKT_WRITE_MODE   = 32'h1000;   
   parameter CCIPKT_READ_MODE    = 32'h2000;   
   parameter CCIPKT_WRFENCE_MODE = 32'hFFFF;   
   parameter CCIPKT_ATOMIC_MODE  = 32'h8000;
      

   /*
    * ASE config structure
    * This will reflect ase.cfg
    */
   typedef struct {
      int         ase_mode;
      int 	  ase_timeout;
      int 	  ase_num_tests;
      int 	  enable_reuse_seed;
      int 	  enable_cl_view;
      int 	  phys_memory_available_gb;
   } ase_cfg_t;
   ase_cfg_t cfg;

   /*
    * MMIO packet
    */
   typedef struct {
      int 	  tid;
      int 	  write_en;
      int 	  width;
      int 	  addr;
      longint 	  qword[8];
      int 	  resp_en;
      } mmio_t;  
   
   // Request types
   parameter int  MMIO_WRITE_REQ    = 32'hAA88;
   parameter int  MMIO_READ_REQ     = 32'hBB88;

   // Length
   parameter int  MMIO_WIDTH_32 = 32;
   parameter int  MMIO_WIDTH_64 = 64;

   
   /*
    * UMSG Hint/Data state machine
    */
   // Number of UMSGs per AFU
   parameter int  NUM_UMSG_PER_AFU = 8;
   
   // Umsg command packet
   typedef struct {
      int 	  id;
      int 	  hint;
      longint 	  qword[8] 		;	  
   } umsgcmd_t;
   

   // UMSG control states
   typedef enum   {UMsgIdle, UMsgHintWait, UMsgSendHint, UMsgDataWait, UMsgSendData}
		  UMsg_StateEnum;

   // UMSG control structure
   typedef struct {
      logic [`UMSG_DELAY_TIMER_LOG2-1:0] hint_timer;
      logic [`UMSG_DELAY_TIMER_LOG2-1:0] data_timer;
      logic 				 line_accessed;
      logic 				 hint_enable;
      logic 				 hint_ready;
      logic 				 hint_pop;
      logic 				 data_ready;
      logic 				 data_pop;
      UMsg_StateEnum                     state;
   } umsg_t;


   /*
    * FUNCTION: Unpack qwords[0:7]       to data vector
    */
   function logic [CCIP_DATA_WIDTH-1:0] unpack_ccipkt_to_vector (input cci_pkt pkt);
      logic [CCIP_DATA_WIDTH-1:0] 	 ret;
      int 				 i;
      begin
   	 ret[  63:00  ] = pkt.qword[0] ;
   	 ret[ 127:64  ] = pkt.qword[1] ;
   	 ret[ 191:128 ] = pkt.qword[2] ;
   	 ret[ 255:192 ] = pkt.qword[3] ;
   	 ret[ 319:256 ] = pkt.qword[4] ;
   	 ret[ 383:320 ] = pkt.qword[5] ;
   	 ret[ 447:384 ] = pkt.qword[6] ;
   	 ret[ 511:448 ] = pkt.qword[7] ;
   	 return ret;
      end
   endfunction

   /*
    * FUNCTION: Pack data vector into qwords[0:7]
    */
   // function automatic void pack_vector_to_ccipkt (input [511:0] vec,
   // 						  ref cci_pkt pkt);
   //    begin
   // 	 pkt.qword[0] =  vec[  63:00 ];
   // 	 pkt.qword[1] =  vec[ 127:64  ];
   // 	 pkt.qword[2] =  vec[ 191:128 ];
   // 	 pkt.qword[3] =  vec[ 255:192 ];
   // 	 pkt.qword[4] =  vec[ 319:256 ];
   // 	 pkt.qword[5] =  vec[ 383:320 ];
   // 	 pkt.qword[6] =  vec[ 447:384 ];
   // 	 pkt.qword[7] =  vec[ 511:448 ];
   //    end
   // endfunction


   /*
    * FUNCTION: conv_gbsize_to_num_bytes
    * Converts GB size to num_bytes
    */
   function automatic longint conv_gbsize_to_num_bytes(int gb_size);
      begin
	 return (gb_size*1024*1024*1024);
      end
   endfunction


   /*
    * FUNCTION: Return absolute value
    */
   function automatic int abs_val(int num);
      begin
	 return (num < 0) ? ~num : num;
      end
   endfunction


   /*
    * Easy check functions
    */ 
   // isReadRequest
   function automatic logic isReadRequest(TxHdr_t hdr);
      begin
	 if ((hdr.reqtype == ASE_RDLINE_I)||(hdr.reqtype == ASE_RDLINE_S)) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isReadResponse
   function automatic logic isReadResponse(RxHdr_t hdr);
      begin
	 if (hdr.resptype == ASE_RD_RSP ) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isWriteRequest
   function automatic logic isWriteRequest(TxHdr_t hdr);
      begin
	 if ((hdr.reqtype == ASE_WRLINE_I)||(hdr.reqtype == ASE_WRLINE_M)) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isWriteResponse
   function automatic logic isWriteResponse(RxHdr_t hdr);
      begin
	 if (hdr.resptype == ASE_WR_RSP ) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction
   
   // isWrFenceRequest
   function automatic logic isWrFenceRequest(TxHdr_t hdr);
      begin
	 if (hdr.reqtype == ASE_WRFENCE) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isWrFenceResponse
   function automatic logic isWrFenceResponse(RxHdr_t hdr);
      begin
	 if (hdr.resptype == ASE_WRFENCE_RSP) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isVL0Request
   function automatic logic isVL0Request(TxHdr_t hdr);
      begin
	 if (hdr.vc == VC_VL0) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isVHxRequest
   function automatic logic isVHxRequest(TxHdr_t hdr);
      begin
	 if ((hdr.vc == VC_VH0)||(hdr.vc == VC_VH1)) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isVARequest
   function automatic logic isVARequest(TxHdr_t hdr);
      begin
	 if (hdr.vc == VC_VA) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction
   
   // isVL0Response
   function automatic logic isVL0Response(RxHdr_t hdr);
      begin
	 if (hdr.vc_used == VC_VL0) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   // isVHxResponse
   function automatic logic isVHxResponse(RxHdr_t hdr);
      begin
	 if ((hdr.vc_used == VC_VH0)||(hdr.vc_used == VC_VH1)) begin
	    return 1;
	 end
	 else begin
	    return 0;	    
	 end	      
      end
   endfunction

   
endpackage
