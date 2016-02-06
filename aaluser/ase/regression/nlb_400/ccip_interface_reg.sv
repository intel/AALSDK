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
module ccip_interface_reg(
  // CCI-P Clocks and Resets
  input           logic             pClk,                    // 400MHz - CC-P clock domain. Primary Clock
  input           logic             pClkDiv2,                // 200MHz - CC-P clock domain
  input           logic             pClkDiv4,                // 100MHz - CC-P clock domain
  input           logic             uClk_usr,                // User clock domain. Refer to clock programming guide  ** Currently provides fixed 300MHz clock **
  input           logic             uClk_usrDiv2,            // User clock domain. Half the programmed frequency  ** Currently provides fixed 150MHz clock **
  input           logic             pck_cp2af_softReset,      // CCI-P ACTIVE HIGH Soft Reset
  input           logic [1:0]       pck_cp2af_pwrState,       // CCI-P AFU Power State
  input           logic             pck_cp2af_error,          // CCI-P Protocol Error Detected

  // Interface structures
  input           t_if_ccip_Rx      pck_cp2af_sRx,        // CCI-P Rx Port
  output          t_if_ccip_Tx      pck_af2cp_sTx         // CCI-P Tx Port
);

logic  [1:0]   pck_cp2af_pwrState_T1;
logic          pck_cp2af_softReset_T1;
logic          pck_cp2af_error_T1;
t_if_ccip_Rx   pck_cp2af_sRx_T1;
t_if_ccip_Tx   pck_af2cp_sTx_T1;

always@(posedge pClk)
begin
    pck_cp2af_sRx_T1           <= pck_cp2af_sRx;
    pck_af2cp_sTx              <= pck_af2cp_sTx_T1;
    pck_cp2af_softReset_T1     <= pck_cp2af_softReset;
    pck_cp2af_pwrState_T1      <= pck_cp2af_pwrState;
    pck_cp2af_error_T1         <= pck_cp2af_error;
end

// NLB AFU- provides validation, performance characterization modes. It also serves as a reference design
ccip_std_afu 
inst_ccip_std_afu (
        .pClk                ( pClk),
        .pClkDiv2            ( pClkDiv2),
        .pClkDiv4            ( pClkDiv4),
        .uClk_usr            ( uClk_usr),
        .uClk_usrDiv2        ( uClk_usrDiv2),
        .pck_cp2af_softReset ( pck_cp2af_softReset_T1),
        .pck_cp2af_pwrState  ( pck_cp2af_pwrState_T1),
        .pck_cp2af_error     ( pck_cp2af_error_T1),
        
        .pck_af2cp_sTx       ( pck_af2cp_sTx_T1),                // CCI-P Tx Port
        .pck_cp2af_sRx       ( pck_cp2af_sRx_T1)                 // CCI-P Rx Port
);

endmodule
