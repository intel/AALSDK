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
import ccip_if_pkg::*;
module ccip_std_afu(
  // CCI-P Clocks and Resets
  input           logic             vl_clk_LPdomain_16ui,                      // CCI interface clock
  input           logic             vl_clk_LPdomain_32ui,                      // 1/2x Frequency of interface clock. Synchronous.
  input           logic             vl_clk_LPdomain_64ui,                      // 1/4x Frequency of interface clock. Synchronous.
  input           logic             ffs_LP16ui_afu_SoftReset_n,                // CCI-P Soft Reset
  input           logic [1:0]       ffs_LP16ui_afu_PwrState,                   // CCI-P AFU Power State
  input           logic             ffs_LP16ui_afu_Error,                      // CCI-P Protocol Error Detected

  // Data ports
  input           t_if_ccip_Rx      ffs_LP16ui_sRxData_afu,                    // CCI-P Rx Port
  output          t_if_ccip_Tx      ffs_LP16ui_sTxData_afu                     // CCI-P Tx Port
);


/* User AFU goes here
*/

// NLB AFU- provides validation, performance characterization modes. It also serves as a reference design
nlb_lpbk nlb_lpbk(
  .Clk_16UI            ( vl_clk_LPdomain_16ui   ) ,
  .SoftReset_n         ( ffs_LP16ui_afu_SoftReset_n) ,
  .SystemReset_n       ( 1'b1 ) ,

  .ci2cf_sRxPort       ( ffs_LP16ui_sRxData_afu ) ,
  .cf2ci_sTxPort       ( ffs_LP16ui_sTxData_afu ) 
);

endmodule
