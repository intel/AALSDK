// ***************************************************************************
//
//        Copyright (C) 2008-2015 Intel Corporation All Rights Reserved.
//
// Engineer :           Pratik Marolia
// Creation Date :	20-05-2015
// Last Modified :	Wed 20 May 2015 03:03:09 PM PDT
// Module Name :	ccip_std_afu
// Project :        ccip afu top (work in progress)
// Description :    This module instantiates CCI-P compliant AFU

// ***************************************************************************
import cvl_pkg::*;
module cciu_std_afu(
  // CCI-P Clocks and Resets
  input  /*var*/  logic             vl_clk_LPdomain_32ui,                      // 1/2x Frequency of interface clock. Synchronous.
  input  /*var*/  logic             vl_clk_LPdomain_16ui,                      // CCI interface clock
  input  /*var*/  logic             ffs_vl_LP32ui_lp2sy_SystemReset_n,         // System Reset
  input  /*var*/  logic             ffs_vl_LP32ui_lp2sy_SoftReset_n,           // CCI-P soft reset. All AFU logic must be reset.

  output          cci_u_TxData_if   ffs_LP16ui_sTxData_afu,                    // CCI-P Tx Port
  input           cci_u_RxData_if   ffs_LP16ui_sRxData_afu                     // CCI-P Rx Port
);



// Wire declarations                                 
logic [17:0]    ffs_vl18_LP32ui_lp2sy_C0RxHdr;
logic [511:0]   ffs_vl512_LP32ui_lp2sy_C0RxData;
logic           ffs_vl_LP32ui_lp2sy_C0RxWrValid;
logic           ffs_vl_LP32ui_lp2sy_C0RxRdValid;
logic           ffs_vl_LP32ui_lp2sy_C0RxCgValid;
logic           ffs_vl_LP32ui_lp2sy_C0RxUgValid;
logic           ffs_vl_LP32ui_lp2sy_C0RxIrValid;
logic [17:0]    ffs_vl18_LP32ui_lp2sy_C1RxHdr;
logic           ffs_vl_LP32ui_lp2sy_C1RxWrValid;
logic           ffs_vl_LP32ui_lp2sy_C1RxIrValid;

logic [60:0]    ffs_vl61_LP32ui_sy2lp_C0TxHdr;
logic           ffs_vl_LP32ui_sy2lp_C0TxRdValid;
logic [60:0]    ffs_vl61_LP32ui_sy2lp_C1TxHdr;
logic [511:0]   ffs_vl512_LP32ui_sy2lp_C1TxData;
logic           ffs_vl_LP32ui_sy2lp_C1TxWrValid;
logic           ffs_vl_LP32ui_sy2lp_C1TxIrValid;

logic           ffs_vl_LP32ui_lp2sy_C0TxAlmFull;
logic           ffs_vl_LP32ui_lp2sy_C1TxAlmFull;
logic           ffs_vl_LP32ui_lp2sy_InitDnForSys;

logic [17:0] 	afu_CfgHeader;
logic				afu_CfgValid;
logic				afu_CfgRdValid;
logic				afu_CfgWrValid;
logic				afu_CfgPortId;


logic 			afu_reset_avlclk;
logic 			afu_reset;


initial
begin
	afu_reset_avlclk 	<= 0;
	afu_reset			<= 0;
end

always@(posedge vl_clk_LPdomain_16ui)
begin
	afu_reset_avlclk 	<= ffs_vl_LP32ui_lp2sy_SoftReset_n;
	afu_reset			<= afu_reset_avlclk;
end

always_comb
begin
	// Cfg Rx Path
	afu_CfgValid   = ffs_LP16ui_sRxData_afu.C0Valid & ffs_LP16ui_sRxData_afu.C0CfgValid;				
	afu_CfgHeader[CCIU_CFGHDR_LENGTH_MSB:CCIU_CFGHDR_LENGTH_LSB]  = ffs_LP16ui_sRxData_afu.C0Hdr[21:20];
	afu_CfgHeader[CCIU_CFGHDR_ADDR_MSB:CCIU_CFGHDR_ADDR_LSB]      = ffs_LP16ui_sRxData_afu.C0Hdr[15:0];
	afu_CfgRdValid = ffs_LP16ui_sRxData_afu.C0Hdr[19:16]==4'hC && afu_CfgValid;
	afu_CfgWrValid = ffs_LP16ui_sRxData_afu.C0Hdr[19:16]==4'h0 && afu_CfgValid;
	
	afu_CfgPortId  = ffs_LP16ui_sRxData_afu.C0PortId;																	// Needed ?
end

always_comb
begin
	// Tx Path Valid
	ffs_LP16ui_sTxData_afu.C0Valid = ffs_vl_LP32ui_sy2lp_C0TxRdValid;
	ffs_LP16ui_sTxData_afu.C1Valid = ffs_vl_LP32ui_sy2lp_C1TxWrValid || ffs_vl_LP32ui_sy2lp_C1TxIrValid;
end


always_comb
begin
    ffs_LP16ui_sTxData_afu.C0PortId = 0;
    ffs_LP16ui_sTxData_afu.C1PortId = 0;

    ffs_LP16ui_sTxData_afu.C0Hdr = '0;
    ffs_LP16ui_sTxData_afu.C1Hdr = '0;
    // FIXME: QPI Only
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_VCSEL_MSB:CCIU_TXHDR_VCSEL_LSB] = ffs_vl61_LP32ui_sy2lp_C0TxHdr[60:59];//USER_SEL_PCIE0;
    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_VCSEL_MSB:CCIU_TXHDR_VCSEL_LSB] = ffs_vl61_LP32ui_sy2lp_C1TxHdr[60:59];//USER_SEL_PCIE0;
    // FIXME: 1 CL only
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_SOP_MSB:CCIU_TXHDR_SOP_LSB] = 1'b1;
    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_SOP_MSB:CCIU_TXHDR_SOP_LSB] = 1'b1;
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_LENGTH_MSB:CCIU_TXHDR_LENGTH_LSB] = 0;
    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_LENGTH_MSB:CCIU_TXHDR_LENGTH_LSB] = 0;

    // Tx signals from AFU
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_ADDR_LSB+:32]                   = ffs_vl61_LP32ui_sy2lp_C0TxHdr[CCIS_TXHDR_ADDR_MSB:CCIS_TXHDR_ADDR_LSB];
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_MDATA_MSB:CCIU_TXHDR_MDATA_LSB] = ffs_vl61_LP32ui_sy2lp_C0TxHdr[CCIS_TXHDR_MDATA_MSB:CCIS_TXHDR_MDATA_LSB];
    ffs_LP16ui_sTxData_afu.C0Hdr[CCIU_TXHDR_REQ_MSB:CCIU_TXHDR_REQ_LSB]     = ffs_vl61_LP32ui_sy2lp_C0TxHdr[CCIS_TXHDR_REQ_MSB:CCIS_TXHDR_REQ_LSB];
    ffs_LP16ui_sTxData_afu.C0RdValid                                        = ffs_vl_LP32ui_sy2lp_C0TxRdValid;

    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_ADDR_LSB+:32]                   = ffs_vl61_LP32ui_sy2lp_C1TxHdr[CCIS_TXHDR_ADDR_MSB:CCIS_TXHDR_ADDR_LSB];
    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_MDATA_MSB:CCIU_TXHDR_MDATA_LSB] = ffs_vl61_LP32ui_sy2lp_C1TxHdr[CCIS_TXHDR_MDATA_MSB:CCIS_TXHDR_MDATA_LSB];
    ffs_LP16ui_sTxData_afu.C1Hdr[CCIU_TXHDR_REQ_MSB:CCIU_TXHDR_REQ_LSB]     = ffs_vl61_LP32ui_sy2lp_C1TxHdr[CCIS_TXHDR_REQ_MSB:CCIS_TXHDR_REQ_LSB];
    ffs_LP16ui_sTxData_afu.C1Data                                           = ffs_vl512_LP32ui_sy2lp_C1TxData;
    ffs_LP16ui_sTxData_afu.C1WrValid                                        = ffs_vl_LP32ui_sy2lp_C1TxWrValid;
    ffs_LP16ui_sTxData_afu.C1IntrValid                                      = ffs_vl_LP32ui_sy2lp_C1TxIrValid;

    // Rx signals to AFU
    ffs_vl18_LP32ui_lp2sy_C0RxHdr   = {ffs_LP16ui_sRxData_afu.C0Hdr[CCIU_RXHDR_REQ_MSB:CCIU_RXHDR_REQ_LSB],
                                       ffs_LP16ui_sRxData_afu.C0Hdr[CCIU_RXHDR_MDATA_LSB+:14]
                                     };
    ffs_vl512_LP32ui_lp2sy_C0RxData = ffs_LP16ui_sRxData_afu.C0Data;
    ffs_vl_LP32ui_lp2sy_C0RxWrValid = ffs_LP16ui_sRxData_afu.C0WrValid;
    ffs_vl_LP32ui_lp2sy_C0RxRdValid = ffs_LP16ui_sRxData_afu.C0RdValid;
    ffs_vl_LP32ui_lp2sy_C0RxCgValid = 0;
    ffs_vl_LP32ui_lp2sy_C0RxUgValid = ffs_LP16ui_sRxData_afu.C0UMsgValid;
    ffs_vl_LP32ui_lp2sy_C0RxIrValid = ffs_LP16ui_sRxData_afu.C0IntrValid;
    ffs_vl18_LP32ui_lp2sy_C1RxHdr   = {ffs_LP16ui_sRxData_afu.C1Hdr[CCIU_RXHDR_REQ_MSB:CCIU_RXHDR_REQ_LSB],
                                       ffs_LP16ui_sRxData_afu.C1Hdr[CCIU_RXHDR_MDATA_LSB+:14]
                                      };
    ffs_vl_LP32ui_lp2sy_C1RxWrValid = ffs_LP16ui_sRxData_afu.C1WrValid;
    ffs_vl_LP32ui_lp2sy_C1RxIrValid = ffs_LP16ui_sRxData_afu.C1IntrValid;

    ffs_vl_LP32ui_lp2sy_C0TxAlmFull = ffs_LP16ui_sRxData_afu.C0TxAlmFull;
    ffs_vl_LP32ui_lp2sy_C1TxAlmFull = ffs_LP16ui_sRxData_afu.C1TxAlmFull;
    ffs_vl_LP32ui_lp2sy_InitDnForSys= ffs_LP16ui_sRxData_afu.InitDn;
end
/* User AFU goes here
*/

// NLB AFU- provides validation, performance characterization modes. It also serves as a reference design
nlb_lpbk nlb_lpbk(
  .Clk_32UI            ( vl_clk_LPdomain_16ui                                  ) ,           // FIXME: Change AFU clock port name
  .SoftReset_n         ( afu_reset                       							 ) ,
  .SystemReset_n       ( afu_reset     								                ) ,

  .rb2cf_C0RxHdr       ( ffs_vl18_LP32ui_lp2sy_C0RxHdr                         ) ,
  .rb2cf_C0RxData      ( ffs_vl512_LP32ui_lp2sy_C0RxData                       ) ,
  .rb2cf_C0RxWrValid   ( ffs_vl_LP32ui_lp2sy_C0RxWrValid                       ) ,
  .rb2cf_C0RxRdValid   ( ffs_vl_LP32ui_lp2sy_C0RxRdValid                       ) ,
  .rb2cf_C0RxCfgValid  ( ffs_vl_LP32ui_lp2sy_C0RxCgValid                       ) ,
  .rb2cf_C0RxUMsgValid ( ffs_vl_LP32ui_lp2sy_C0RxUgValid                       ) ,
  .rb2cf_C0RxIntrValid ( ffs_vl_LP32ui_lp2sy_C0RxIrValid                       ) ,
  .rb2cf_C1RxHdr       ( ffs_vl18_LP32ui_lp2sy_C1RxHdr                         ) ,
  .rb2cf_C1RxWrValid   ( ffs_vl_LP32ui_lp2sy_C1RxWrValid                       ) ,
  .rb2cf_C1RxIntrValid ( ffs_vl_LP32ui_lp2sy_C1RxIrValid                       ) ,

  .cf2ci_C0TxHdr       ( ffs_vl61_LP32ui_sy2lp_C0TxHdr                         ) ,
  .cf2ci_C0TxRdValid   ( ffs_vl_LP32ui_sy2lp_C0TxRdValid                       ) ,
  .cf2ci_C1TxHdr       ( ffs_vl61_LP32ui_sy2lp_C1TxHdr                         ) ,
  .cf2ci_C1TxData      ( ffs_vl512_LP32ui_sy2lp_C1TxData                       ) ,
  .cf2ci_C1TxWrValid   ( ffs_vl_LP32ui_sy2lp_C1TxWrValid                       ) ,
  .cf2ci_C1TxIntrValid ( ffs_vl_LP32ui_sy2lp_C1TxIrValid                       ) ,
  .ci2cf_C0TxAlmFull   ( ffs_vl_LP32ui_lp2sy_C0TxAlmFull                       ) ,
  .ci2cf_C1TxAlmFull   ( ffs_vl_LP32ui_lp2sy_C1TxAlmFull                       ) ,

  .ci2cf_InitDn        ( ffs_vl_LP32ui_lp2sy_InitDnForSys                      ) ,

  .cr2cf_CfgHeader     ( afu_CfgHeader                      						 ) ,
  .cr2cf_CfgWrEn       ( afu_CfgWrValid                     						 ) ,
  .cr2cf_CfgRdEn       ( afu_CfgRdValid      						                ) ,
  .cr2cf_CfgDin        ( ffs_LP16ui_sRxData_afu.C0Data[CCIU_CFGDATA_WIDTH-1:0] ) ,
  .cf2cr_CfgDout       ( ffs_LP16ui_sTxData_afu.CfgRdData                      ) ,
  .cf2cr_CfgDoutValid  ( ffs_LP16ui_sTxData_afu.CfgRdValid                     ) 
);

endmodule
