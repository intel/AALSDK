// ***************************************************************************
//
//        Copyright (C) 2008-2015 Intel Corporation All Rights Reserved.
//
// Engineer :           Pratik Marolia
// Creation Date :      20-05-2015
// Last Modified :      Wed 20 May 2015 03:03:09 PM PDT
// Module Name :        ccip_std_afu
// Project :        ccip afu top (work in progress)
// Description :    This module instantiates CCI-P compliant AFU

// ***************************************************************************

`include "cci_mpf_if.vh"

module ccip_std_afu(
  // CCI-P Clocks and Resets
  input           logic             vl_clk_LPdomain_16ui,                      // CCI interface clock
  input           logic             vl_clk_LPdomain_64ui,                      // 1/4x Frequency of interface clock. Synchronous.
  input           logic             vl_clk_LPdomain_32ui,                      // 1/2x Frequency of interface clock. Synchronous.
  input           logic             ffs_LP16ui_afu_SoftReset_n,                // CCI-P Soft Reset
  input           logic [1:0]       ffs_LP16ui_afu_PwrState,                   // CCI-P AFU Power State
  input           logic             ffs_LP16ui_afu_Error,                      // CCI-P Protocol Error Detected

  // Data ports
  output          t_if_ccip_Tx      ffs_LP16ui_sTxData_afu,                    // CCI-P Tx Port
  input           t_if_ccip_Rx      ffs_LP16ui_sRxData_afu                     // CCI-P Rx Port
);

localparam MPF_DFH_MMIO_ADDR = 'h1000;

logic clk;
assign clk = vl_clk_LPdomain_16ui;

//
// Expose FIU as an MPF interface
//
cci_mpf_if fiu(.clk);

ccip_wires_to_mpf
  #(
    .REGISTER_INPUTS(0),
    .REGISTER_OUTPUTS(1)
    )
  map_ifc(.*);

//
// Put MPF between AFU and FIU.
//
cci_mpf_if afu(.clk);

cci_mpf
  #(
    .SORT_READ_RESPONSES(1),
    .PRESERVE_WRITE_MDATA(1),

    // Don't enforce write/write or write/read ordering within a cache line.
    // (Default CCI behavior.)
    .ENFORCE_WR_ORDER(0),

    // Address of the MPF feature header
    .DFH_MMIO_BASE_ADDR(MPF_DFH_MMIO_ADDR)
    )
  mpf
   (
    .clk,
    .fiu,
    .afu
    );

t_if_ccip_Rx afu_rx;
t_if_ccip_Tx afu_tx;

always_comb
begin
    afu_rx.c0 = afu.c0Rx;
    afu_rx.c1 = afu.c1Rx;

    afu_rx.c0TxAlmFull = afu.c0TxAlmFull;
    afu_rx.c1TxAlmFull = afu.c1TxAlmFull;

    afu.c0Tx = cci_mpf_cvtC0TxFromBase(afu_tx.c0);
    // Treat all addresses as virtual
    afu.c0Tx.hdr.ext.addrIsVirtual = 1'b1;
    afu.c0Tx.hdr.ext.checkLoadStoreOrder = 1'b1;

    afu.c1Tx = cci_mpf_cvtC1TxFromBase(afu_tx.c1);
    afu.c1Tx.hdr.ext.addrIsVirtual = 1'b1;
    afu.c1Tx.hdr.ext.checkLoadStoreOrder = 1'b1;

    afu.c2Tx = afu_tx.c2;
end

/* User AFU goes here
*/

// NLB AFU- provides validation, performance characterization modes. It also serves as a reference design
nlb_lpbk#(.MPF_DFH_MMIO_ADDR(MPF_DFH_MMIO_ADDR))
nlb_lpbk(
  .Clk_16UI            ( clk ) ,
  .SoftReset_n         ( ffs_LP16ui_afu_SoftReset_n) ,
  .SystemReset_n       ( 1'b1 ) ,

  .ci2cf_sRxPort       ( afu_rx ) ,
  .cf2ci_sTxPort       ( afu_tx ) 
);

endmodule
