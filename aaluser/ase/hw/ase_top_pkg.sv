// Date: 10/8/2015
// Compliant with CCI-P spec v0.58
package ase_top_pkg;

   parameter CCIP_CLDATA_WIDTH      = 512;
   parameter CCIP_CFGDATA_WIDTH     = 64;
   parameter CCIP_CFG_ADDR_MASK     = 16'h7fff;        // AFU developers ignore this

   //=====================================================================
   // CCI-P Field paramters
   //=====================================================================
   // Request Type  Encodings
   //----------------------------------------------------------------------
   parameter CCIP_REQ_TYPE_WIDTH          = 4'h4;
   parameter CCIP_REQ_WRLINE_I            = 4'h1;
   parameter CCIP_REQ_WRLINE_M            = 4'h2;
   parameter CCIP_REQ_WRFENCE             = 4'h5;
   parameter CCIP_REQ_RDLINE_S            = 4'h4;
   parameter CCIP_REQ_RDLINE_I            = 4'h6;
   parameter CCIP_REQ_INTR                = 4'h8;


   // Response Type  Encodings
   //----------------------------------------------------------------------
   parameter CCIP_RSP_WRLINE              = 4'h1;
   parameter CCIP_RSP_RDLINE              = 4'h4;
   parameter CCIP_RSP_INTR                = 4'h8;
   parameter CCIP_RSP_UMSG                = 4'hf;
   //
   // Virtual Channel Select
   //----------------------------------------------------------------------
   parameter CCIP_VC_SEL_WIDTH = 2'h2;
   parameter CCIP_VC_VA  = 2'b00;
   parameter CCIP_VC_VL0 = 2'b01;
   parameter CCIP_VC_VH0 = 2'b10;
   parameter CCIP_VC_VH1 = 2'b11;
   //----------------------------------------------------------------------
   typedef struct packed {
      logic [CCIP_VC_SEL_WIDTH-1:0] vc_sel;
      logic 			    sop;
      logic 			    rsvd1;
      logic [1:0] 		    length;
      logic [3:0] 		    req_type;
      logic [5:0] 		    rsvd0;
      logic [41:0] 		    address;
      logic [15:0] 		    mdata;
   } cci_p_RqMemHdr;
   parameter CCIP_TXHDR_WIDTH = $bits(cci_p_RqMemHdr);

   typedef struct 		    packed {
      logic [CCIP_VC_SEL_WIDTH-1:0] vc_used;
      logic 			    poison;
      logic 			    hit_miss;
      logic 			    fmt;
      logic 			    rsvd0;
      logic [1:0] 		    cl_num;
      logic [3:0] 		    resp_type;
      logic [15:0] 		    mdata;
   } cci_p_RspMemHdr;
   parameter CCIP_RXHDR_WIDTH = $bits(cci_p_RspMemHdr);

   typedef struct 		    packed {
      logic [7:0] 		    tid;        // delete me: temporarily used for testing
      logic [15:0] 		    address;    // 4B aligned MMIO address
      logic [1:0] 		    rsvd0;
      logic [1:0] 		    length;
   } cci_p_ReqCfgHdr;
   parameter CCIP_RXCFGHDR_WIDTH = $bits(cci_p_ReqCfgHdr);

   /* PM: Delete me: Temporarily used for testing */
   typedef struct 		    packed {
      logic 			    length;
      logic [7:0] 		    tid;
   } cci_p_RspCfgHdr;
   parameter CCIP_TXCFGHDR_WIDTH = $bits(cci_p_RspCfgHdr);

   typedef struct 		    {
      cci_p_RqMemHdr                C0Hdr;          // Tx hdr
      logic                         C0RdValid;      // Tx hdr is valid

      cci_p_RqMemHdr                C1Hdr;          // Tx hdr
      logic [CCIP_CLDATA_WIDTH-1:0] C1Data;         // Tx data
      logic                         C1WrValid;      // Tx hdr is valid
      logic                         C1IntrValid;    // Tx interrupt valid

      logic                         CfgRdValid;
      logic [CCIP_CFGDATA_WIDTH-1:0] CfgRdData;
      cci_p_RspCfgHdr               CfgHdr;         // Delete this. Temporarily added for testing
   } cci_p_TxData_if;


   typedef struct 		     {
      cci_p_RspMemHdr              C0Hdr;          //  Rx hdr
      logic [CCIP_CLDATA_WIDTH-1:0]  C0Data;         //  Rx data
      logic 			     C0WrValid;      //  Rx hdr carries a write response
      logic 			     C0RdValid;      //  Rx hdr carries a read response
      logic 			     C0CfgValid;     //  Rx hdr carries a cfg write
      logic 			     C0IntrValid;    //  Rx interrupt cmp
      logic 			     C0UMsgValid;    //  Rx UMsg valid

      cci_p_RspMemHdr              C1Hdr;          //  Rx hdr
      logic 			     C1WrValid;      //  Rx hdr carries a write response
      logic 			     C1IntrValid;    //  Rx interrupt cmp
      logic 			     C0TxAlmFull;    //  Tx channel is almost full
      logic 			     C1TxAlmFull;    //  Tx channel is almost full

      logic 			     CfgRdValid;
      logic 			     CfgWrValid;
      cci_p_ReqCfgHdr              CfgHdr;
   } cci_p_RxData_if;

endpackage
