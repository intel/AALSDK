package cvl_pkg;
// `include "sys_cfg_pkg.svh"
`include "vendor_defines.vh"

`ifdef NUM_AFUS
    parameter   NUM_AFUS    = `NUM_AFUS;
`else
    ** Define NUM_AFUS in sys_cfg_pkg.svh **
`endif
parameter   USER_MDATA_WIDTH = 16;
parameter   BLK_SIZE      = 4;         // max # of cachelines in a block transfer




parameter DATA_WIDTH    = 512;
parameter CL_WIDTH      = DATA_WIDTH;
parameter L_MAX_QPI_RDS = 7;         // LOG # out outstanding reads
parameter L_MAX_QPI_WRS = 7;         // LOG # out outstanding reads
parameter NUM_UMSG      = 32;        // max value = 32
parameter MAX_QPI_RDS   = 2**L_MAX_QPI_RDS;
parameter MAX_QPI_WRS   = 2**L_MAX_QPI_WRS;
parameter LNUM_UMSG     = $clog2(NUM_UMSG);
parameter L_BLK_SIZE    = $clog2(BLK_SIZE);
parameter LNUM_AFUS     = NUM_AFUS>1?$clog2(NUM_AFUS):1'h1;


//Delete this, Date- 07282015
// CCI-S paramters - Delete this
//----------------------------------------------------------------------
parameter CCIS_TXHDR_WIDTH     = 61;
parameter CCIS_TXHDR_MDATA_LSB = 0;
parameter CCIS_TXHDR_MDATA_MSB = 13;
parameter CCIS_TXHDR_ADDR_LSB  = CCIS_TXHDR_MDATA_MSB+1;
parameter CCIS_TXHDR_ADDR_MSB  = CCIS_TXHDR_MDATA_MSB+32;
parameter CCIS_TXHDR_REQ_LSB   = CCIS_TXHDR_ADDR_MSB+7;
parameter CCIS_TXHDR_REQ_MSB   = CCIS_TXHDR_ADDR_MSB+6+4;

parameter CCIS_RXHDR_WIDTH     = 24;
parameter CCIS_RXHDR_MDATA_LSB = 0;
parameter CCIS_RXHDR_MDATA_MSB = 13;
parameter CCIS_RXHDR_REQ_LSB   = CCIS_RXHDR_MDATA_MSB+1;
parameter CCIS_RXHDR_REQ_MSB   = CCIS_RXHDR_MDATA_MSB+4;

// CCI-U paramters
//----------------------------------------------------------------------
parameter CCIU_TXHDR_WIDTH          = 75;
parameter CCIU_RXHDR_WIDTH          = 32;

parameter CCIU_REQ_WRCSR            = 4'b0000;
parameter CCIU_REQ_RDCSR            = 4'b1100;
parameter CCIU_REQ_WR_FENCE         = 4'b0101;

parameter CCIU_RSP_WR               = 4'b0001;
parameter CCIU_RSP_RD               = 4'b0100;

// PCIe specific
//----------------------------------------------------------------------
parameter PCIE_TAG_WIDTH            = 5;
parameter PCIE_MAX_NUM_TAGS         = 2**PCIE_TAG_WIDTH;
parameter PCIE_MAX_NUM_FUNS         = 2;

parameter PCIE_FMTTYPE_MEM_READ32   = 7'b000_0000;
parameter PCIE_FMTTYPE_MEM_READ64   = 7'b010_0000;
parameter PCIE_FMTTYPE_MEM_WRITE32  = 7'b100_0000;
parameter PCIE_FMTTYPE_MEM_WRITE64  = 7'b110_0000;
parameter PCIE_FMTTYPE_CFG_WRITE    = 7'b100_0100;
parameter PCIE_FMTTYPE_CPL          = 7'b000_1010;
parameter PCIE_FMTTYPE_CPLD         = 7'b100_1010;

/*
// CCI-E paramters - Delete this
//----------------------------------------------------------------------
parameter CCIE_TXHDR_MDATA_LSB   = 0;
parameter CCIE_TXHDR_MDATA_MSB   = USER_MDATA_WIDTH-1;
parameter CCIE_TXHDR_ADDR_LSB    = CCIE_TXHDR_MDATA_MSB+1;
parameter CCIE_TXHDR_ADDR_MSB    = CCIE_TXHDR_MDATA_MSB+32;
parameter CCIE_TXHDR_REQ_LSB     = CCIE_TXHDR_ADDR_MSB+7;
parameter CCIE_TXHDR_REQ_MSB     = CCIE_TXHDR_ADDR_MSB+6+4;
parameter CCIE_TXHDR_HADDR_LSB   = CCIE_TXHDR_REQ_MSB+12;
parameter CCIE_TXHDR_HADDR_MSB   = CCIE_TXHDR_REQ_MSB+11+26;
parameter CCIE_TXHDR_BLKSIZE_LSB = CCIE_TXHDR_HADDR_MSB+1;
parameter CCIE_TXHDR_BLKSIZE_MSB = CCIE_TXHDR_HADDR_MSB+BLK_SIZE;
parameter CCIE_TXHDR_PORTSEL_LSB = CCIE_TXHDR_BLKSIZE_MSB+1;
parameter CCIE_TXHDR_PORTSEL_MSB = CCIE_TXHDR_BLKSIZE_MSB+2;

parameter CCIE_TXHDR_WIDTH       = CCIE_TXHDR_PORTSEL_MSB+1;

parameter CCIE_RXHDR_MDATA_LSB   = 0;
parameter CCIE_RXHDR_MDATA_MSB   = USER_MDATA_WIDTH-1;
parameter CCIE_RXHDR_REQ_LSB     = CCIE_RXHDR_MDATA_MSB+1;
parameter CCIE_RXHDR_REQ_MSB     = CCIE_RXHDR_MDATA_MSB+4;
parameter CCIE_RXHDR_CLNUM_LSB   = CCIE_RXHDR_REQ_MSB+1;
parameter CCIE_RXHDR_CLNUM_MSB   = CCIE_RXHDR_REQ_MSB+3;

parameter CCIE_RXHDR_WIDTH       = CCIE_RXHDR_CLNUM_MSB+1;
*/

// CCI-P paramters
//----------------------------------------------------------------------
parameter CCIP_TXHDR_MDATA_LSB   = 0;
parameter CCIP_TXHDR_MDATA_MSB   = USER_MDATA_WIDTH-1;
parameter CCIP_TXHDR_ADDR_LSB    = CCIP_TXHDR_MDATA_MSB   + 1;
parameter CCIP_TXHDR_ADDR_MSB    = CCIP_TXHDR_MDATA_MSB   + 42;
parameter CCIP_TXHDR_REQ_LSB     = CCIP_TXHDR_ADDR_MSB    + 6 + 1;
parameter CCIP_TXHDR_REQ_MSB     = CCIP_TXHDR_ADDR_MSB    + 6 + 4;
parameter CCIP_TXHDR_LENGTH_LSB  = CCIP_TXHDR_REQ_MSB     + 1;
parameter CCIP_TXHDR_LENGTH_MSB  = CCIP_TXHDR_REQ_MSB     + L_BLK_SIZE;
parameter CCIP_TXHDR_SOP_LSB     = CCIP_TXHDR_LENGTH_MSB  + 1 + 1;
parameter CCIP_TXHDR_SOP_MSB     = CCIP_TXHDR_LENGTH_MSB  + 1 + 1;
parameter CCIP_TXHDR_VCSEL_LSB   = CCIP_TXHDR_SOP_MSB     + 1;
parameter CCIP_TXHDR_VCSEL_MSB   = CCIP_TXHDR_SOP_MSB     + 2;

parameter CCIP_TXHDR_WIDTH       = CCIP_TXHDR_VCSEL_MSB   + 1;

parameter CCIP_RXHDR_MDATA_LSB   = 0;
parameter CCIP_RXHDR_MDATA_MSB   = USER_MDATA_WIDTH-1;
parameter CCIP_RXHDR_REQ_LSB     = CCIP_RXHDR_MDATA_MSB   + 1;
parameter CCIP_RXHDR_REQ_MSB     = CCIP_RXHDR_MDATA_MSB   + 4;
parameter CCIP_RXHDR_CLNUM_LSB   = CCIP_RXHDR_REQ_MSB     + 1;
parameter CCIP_RXHDR_CLNUM_MSB   = CCIP_RXHDR_REQ_MSB     + 2;
parameter CCIP_RXHDR_FMT_LSB     = CCIP_RXHDR_CLNUM_MSB   + 1 + 1;
parameter CCIP_RXHDR_FMT_MSB     = CCIP_RXHDR_CLNUM_MSB   + 1 + 1;
parameter CCIP_RXHDR_HIT_LSB     = CCIP_RXHDR_FMT_MSB     + 1;
parameter CCIP_RXHDR_HIT_MSB     = CCIP_RXHDR_FMT_MSB     + 1;
parameter CCIP_RXHDR_POISON_LSB  = CCIP_RXHDR_HIT_MSB     + 1;
parameter CCIP_RXHDR_POISON_MSB  = CCIP_RXHDR_HIT_MSB     + 1;
parameter CCIP_RXHDR_VCUSED_LSB  = CCIP_RXHDR_POISON_MSB  + 1;
parameter CCIP_RXHDR_VCUSED_MSB  = CCIP_RXHDR_POISON_MSB  + 2;

parameter CCIP_RXHDR_WIDTH       = CCIP_RXHDR_VCUSED_MSB  + 1;

parameter CCIP_CFGHDR_ADDR_LSB   = 0;
parameter CCIP_CFGHDR_ADDR_MSB   = 15;
parameter CCIP_CFGHDR_LENGTH_LSB = CCIP_CFGHDR_ADDR_MSB   + 1;
parameter CCIP_CFGHDR_LENGTH_MSB = CCIP_CFGHDR_ADDR_MSB   + 2;

parameter CCIP_CFGHDR_WIDTH      = CCIP_CFGHDR_LENGTH_MSB + 1;
parameter CCIP_CFGDATA_WIDTH     = 64;
parameter CCIP_CFG_ADDR_MASK     = 16'h7fff;

// CCI-P Request/Response Encodings
//----------------------------------------------------------------------
parameter WrLine_I            = 4'h1;
parameter WrLine_M            = 4'h2;
parameter WrFence             = 4'h5;
parameter RdLine_S            = 4'h4;
parameter RdLine_I            = 4'h6;
parameter RdLine_E            = 4'h7;
parameter Intr                = 4'h8;
parameter UMsg                = 4'hf;

// A2C Response Encodings
//----------------------------------------------------------------------
parameter MMIO_Rd             = 4'h0;
parameter MMIO_Wr             = 4'hc;

//----------------------------------------------------------------------

// User Port Select
//----------------------------------------------------------------------
parameter USER_SEL_AUTO = 2'b00;
parameter USER_SEL_QPI  = 2'b01;
parameter USER_SEL_PCIE0= 2'b10;
parameter USER_SEL_PCIE1= 2'b11;
//----------------------------------------------------------------------
/*
Delete this - date 07282015
typedef struct {
    logic [CCIS_TXHDR_WIDTH-1:0]  C0Hdr;          // Tx hdr
    logic                         C0RdValid;      // Tx hdr is valid
    logic                         C0Valid;
    logic [CCIS_TXHDR_WIDTH-1:0]  C1Hdr;          // Tx hdr
    logic [DATA_WIDTH-1:0]        C1Data;         // Tx data
    logic                         C1WrValid;      // Tx hdr is valid
    logic                         C1IntrValid;    // Tx interrupt valid
    logic                         C1Valid;
    logic [LNUM_AFUS-1:0]         C0PortId;       // Port number
    logic [LNUM_AFUS-1:0]         C1PortId;       // Port number
} cci_s_TxData_if;

typedef struct {
    logic [CCIS_RXHDR_WIDTH-1:0] C0Hdr;          //  Rx hdr
    logic [DATA_WIDTH-1:0]       C0Data;         //  Rx data
    logic                        C0WrValid;      //  Rx hdr carries a write response
    logic                        C0RdValid;      //  Rx hdr carries a read response
    logic                        C0CfgValid;     //  Rx hdr carries a cfg write
    logic                        C0IntrValid;    //  Rx interrupt cmp
    logic                        C0UMsgValid;    //  Rx UMsg valid
    logic                        C0Valid;
    logic [CCIS_RXHDR_WIDTH-1:0] C1Hdr;          //  Rx hdr
    logic                        C1WrValid;      //  Rx hdr carries a write response
    logic                        C1IntrValid;    //  Rx interrupt cmp
    logic                        C1Valid;
    logic                        C0TxAlmFull;    //  Tx channel is almost full
    logic                        C1TxAlmFull;    //  Tx channel is almost full
    logic                        InitDn;         //  Initialization complete
    logic [LNUM_AFUS-1:0]        C0PortId;       // Port number
    logic [LNUM_AFUS-1:0]        C1PortId;       // Port number
} cci_s_RxData_if;
*/
typedef struct {
    logic [CCIP_TXHDR_WIDTH-1:0]  C0Hdr;          // Tx hdr
    logic                         C0RdValid;      // Tx hdr is valid
    logic                         C0Valid;
    logic [LNUM_AFUS-1:0]         C0PortId;       // Port number

    logic [CCIP_TXHDR_WIDTH-1:0]  C1Hdr;          // Tx hdr
    logic [DATA_WIDTH-1:0]        C1Data;         // Tx data
    logic                         C1WrValid;      // Tx hdr is valid
    logic                         C1IntrValid;    // Tx interrupt valid
    logic                         C1Valid;
    logic [LNUM_AFUS-1:0]         C1PortId;       // Port number

    logic                         CfgRdValid;
    logic [CCIP_CFGDATA_WIDTH-1:0]CfgRdData;
    logic [LNUM_AFUS-1:0]         CfgPortId;
} cci_p_TxData_if;

typedef struct {
    logic [CCIP_RXHDR_WIDTH-1:0] C0Hdr;          //  Rx hdr
    logic [DATA_WIDTH-1:0]       C0Data;         //  Rx data
    logic                        C0WrValid;      //  Rx hdr carries a write response
    logic                        C0RdValid;      //  Rx hdr carries a read response
    logic                        C0CfgValid;     //  Rx hdr carries a cfg write
    logic                        C0IntrValid;    //  Rx interrupt cmp
    logic                        C0UMsgValid;    //  Rx UMsg valid
    logic                        C0Valid;
    logic [LNUM_AFUS-1:0]        C0PortId;       // Port number

    logic [CCIP_RXHDR_WIDTH-1:0] C1Hdr;          //  Rx hdr
    logic                        C1WrValid;      //  Rx hdr carries a write response
    logic                        C1IntrValid;    //  Rx interrupt cmp
    logic                        C1Valid;
    logic                        C0TxAlmFull;    //  Tx channel is almost full
    logic                        C1TxAlmFull;    //  Tx channel is almost full
    logic                        InitDn;         //  Initialization complete
    logic [LNUM_AFUS-1:0]        C1PortId;       // Port number

    logic                        CfgValid;
    logic                        CfgRdValid;
    logic                        CfgWrValid;
    logic [CCIP_CFGHDR_WIDTH-1:0]CfgHeader;
    logic [LNUM_AFUS-1:0]        CfgPortId;
} cci_p_RxData_if;


//`define KEEP_WIRE syn_keep=1
//`define GRAM_AUTO "no_rw_check"                         // defaults to auto
//`define GRAM_BLCK "no_rw_check, M20K"
//`define GRAM_DIST "no_rw_check, MLAB"

endpackage
