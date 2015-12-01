/* ****************************************************************************
 * Copyright (c) 2011-2014, Intel Corporation
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

   // Include platform.vh if not already
 `ifndef _PLATFORM_VH_
  `include "platform.vh"
 `endif

   // `define GRAM_AUTO "no_rw_check"                         // defaults to auto
   // `define GRAM_STYLE RAM_STYLE
   // `define SYNC_RESET_POLARITY 0

   // Address widths
   // `define PHYSADDR_WIDTH       38
   parameter PHYSCLADDR_WIDTH   =  42;

   /*
    * CCI Transactions
    */
   // Read Request/Response
   parameter CCIP_TX0_RDLINE_S   =  4'h4;
   parameter CCIP_TX0_RDLINE_I   =  4'h6;
   parameter CCIP_TX0_RDLINE_E   =  4'h7;
   parameter CCIP_RX0_RD_RESP    =  4'h4;

   // Write request/response
   parameter CCIP_TX1_WRLINE_I   =  4'h1;
   parameter CCIP_TX1_WRLINE_M   =  4'h2;
   parameter CCIP_TX1_WRFENCE    =  4'h5;
   parameter CCIP_RX0_WR_RESP    =  4'h1;
   parameter CCIP_RX1_WR_RESP    =  4'h1;

   // MSI-X request/response
   parameter CCIP_TX1_INTRVALID  =  4'h8;
   parameter CCIP_RX0_INTR_CMPLT =  4'h8;
   parameter CCIP_RX1_INTR_CMPLT =  4'h8;

   // CSR Write/Rread
   parameter CCIP_MMIO_RD        =  4'h0;
   parameter CCIP_MMIO_WR        =  4'hC;

   // UMsg // TBD
   parameter CCIP_RX0_UMSG       =  4'hF;


   /*
    * CCI specifications
    */
   parameter CCIP_DATA_WIDTH       = 512;
   parameter CCIP_UMSG_BITINDEX    = 12;
   parameter CCIP_CFG_RDDATA_WIDTH = 64;

   /*
    * TX header deconstruction
    */
   // SPL (CCI-extended additions)
   //parameter TX_HDR_NUMCL_BITRANGE      98:93
   //parameter TX_HDR_CLADDR_BITRANGE     92:67
   //parameter TX_HDR_PV_BIT              66
   // CCI only
 `define TX_META_TYPERANGE          67:64
 `define TX_CLADDR_BITRANGE         57:16
 `define TX_MDATA_BITRANGE          15:0

   /*
    * RX header deconstruction
    */
   // RX header (SPL/CCI common response)
 `define RX_META_TYPERANGE          19:16
 `define RX_MDATA_BITRANGE          15:0
 `define RX_CSR_BITRANGE            13:0
 `define RX_CSR_DATARANGE           64:0


   /*
    * FIFO depth bit-width
    * Enter 'n' here, where n = log_2(FIFO_DEPTH) & n is an integer
    */
   parameter ASE_FIFO_DEPTH_NUMBITS = 8;


   /*
    * SIMKILL_ON_UNDEFINED: A switch to kill simulation if on a valid
    * signal, 'X' or 'Z' is not allowed, gracious closedown on same
    */
 `define VLOG_UNDEF                   1'bx
 `define VLOG_HIIMP                   1'bz


   /*
    * Latency Scoreboard generics
    */
   // Number of transactions in latency scoreboard
   parameter LATBUF_NUM_TRANSACTIONS = 32;
   // Radix of latency scoreboard radix
   parameter LATBUF_COUNT_WIDTH      = $clog2(LATBUF_NUM_TRANSACTIONS) + 1;
   // ASE_fifo full threshold inside latency scoreboard
   parameter LATBUF_FULL_THRESHOLD   = LATBUF_NUM_TRANSACTIONS - 5;
   // Radix of ASE_fifo (subcomponent in latency scoreboard)
   parameter LATBUF_DEPTH_BASE2      = $clog2(LATBUF_NUM_TRANSACTIONS);


   /*
    * Print in Color
    */
   // Error in RED color
 `define BEGIN_RED_FONTCOLOR   $display("\033[1;31m");
 `define END_RED_FONTCOLOR     $display("\033[1;m");

   // Info in GREEN color
 `define BEGIN_GREEN_FONTCOLOR $display("\033[32;1m");
 `define END_GREEN_FONTCOLOR   $display("\033[0m");

   // Warnings/ASEDBGDUMP in YELLOW color
 `define BEGIN_YELLOW_FONTCOLOR $display("\033[0;33m");
 `define END_YELLOW_FONTCOLOR   $display("\033[0m");


   /*
    * CCI Transaction packet
    */
   typedef struct {
      longint     meta;
      longint     qword[8];
      int 	  cfgvalid;
      int 	  wrvalid;
      int 	  rdvalid;
      int 	  intrvalid;
      int 	  umsgvalid;
   } cci_pkt;


   /*
    * ASE config structure
    * This will reflect ase.cfg
    */
   typedef struct {
      int         ase_mode;
      int 	  ase_timeout;
      int 	  ase_num_tests;
      int 	  enable_reuse_seed;
      int 	  num_umsg_log2;
      int 	  enable_cl_view;
      int 	  enable_capcm;
      int 	  memmap_sad_setting;
   } ase_cfg_t;
   ase_cfg_t cfg;


   /*
    * UMSG Hint/Data state machine
    */

   // UMSG control states
   typedef enum   {UMsg_Idle, UMsg_ChangeOccured, UMsg_SendHint, UMsg_Waiting, UMsg_SendData}
		  UMsg_StateEnum;

   // UMSG control structure
   typedef struct {
      logic [`UMSG_DELAY_TIMER_LOG2-1:0] hint_timer;
      logic [`UMSG_DELAY_TIMER_LOG2-1:0] data_timer;
      logic [CCIP_DATA_WIDTH-1:0] 	 data;
      logic [CCIP_DATA_WIDTH-1:0] 	 data_q;
      logic 				 change;
      logic 				 hint_enable;
      logic 				 hint_timer_started;
      logic 				 hint_ready;
      logic 				 hint_pop;
      logic 				 data_timer_started;
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
   function automatic void pack_vector_to_ccipkt (input [511:0] vec, ref cci_pkt pkt);
      begin
	 pkt.qword[0] =  vec[  63:00 ];
	 pkt.qword[1] =  vec[ 127:64  ];
	 pkt.qword[2] =  vec[ 191:128 ];
	 pkt.qword[3] =  vec[ 255:192 ];
	 pkt.qword[4] =  vec[ 319:256 ];
	 pkt.qword[5] =  vec[ 383:320 ];
	 pkt.qword[6] =  vec[ 447:384 ];
	 pkt.qword[7] =  vec[ 511:448 ];
      end
   endfunction


   /* ***********************************************************
    * CCI-P headers
    * RxHdr, TxHdr, CCIP Packets
    * ***********************************************************/
   // RxHdr
   typedef struct packed {
      logic [1:0] vc;       // 27:26  // Virtual channel select
      logic       poison;   // 25     // Poison bit
      logic       hitmiss;  // 24     // Hit/miss indicator
      logic       format;   // 23     // Multi-CL enable
      logic       rsvd22;   // 22     // X
      logic [1:0] clnum;    // 21:20  // Cache line number
      logic [3:0] resptype; // 19:16  // Response type
      logic [15:0] mdata;   // 15:0   // Metadata
   } RxHdr_t;
   parameter CCIP_RX_HDR_WIDTH     = $bits(RxHdr_t);

   // TxHdr
   typedef struct packed {
      logic [1:0]  vc;       // 73:72  // Virtual channel select
      logic 	   sop;      // 71     // Start of packet
      logic 	   rsvd70;   // 70     // X
      logic [1:0]  len;      // 69:68  // Length
      logic [3:0]  reqtype;  // 67:64  // Request Type
      logic [5:0]  rsvd63_58;// 63:58  // X
      logic [41:0] addr;     // 57:16  // Address
      logic [15:0] mdata;    // 15:0   // Metadata
   } TxHdr_t;
   parameter CCIP_TX_HDR_WIDTH     = $bits(TxHdr_t);

   // CfgHdr
   typedef struct packed {
      logic [15:0] index;
      logic [1:0]  len;
      logic 	   poison;
      logic [8:0]  tid;  
      } CfgHdr_t;
   parameter CCIP_CFG_HDR_WIDTH    = $bits(CfgHdr_t);

   // MMIO header
   typedef struct packed {
      logic [8:0] tid; 
      } MMIOHdr_t;
   parameter CCIP_MMIO_TID_WIDTH    = $bits(MMIOHdr_t);
   
      
   // Config channel
   parameter CCIP_MMIO_ADDR_WIDTH   = 16;
   parameter CCIP_MMIO_INDEX_WIDTH  = 14;
   parameter CCIP_MMIO_RDDATA_WIDTH = 64;


endpackage
