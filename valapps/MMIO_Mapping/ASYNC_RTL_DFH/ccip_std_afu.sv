// ***************************************************************************
//
//        Copyright (C) 2008-2015 Intel Corporation All Rights Reserved.
//
// Engineer :           Enno Luebbers
// Creation Date :      02-09-2016
// Last Modified :      02-09-2016
// Module Name :        ccip_std_afu
// Project :        ccip afu top (work in progress)
// Description :    This module instantiates CCI-P compliant AFU

// ***************************************************************************

import ccip_if_pkg::*;

module ccip_std_afu(
  // CCI-P Clocks and Resets
  input           logic             pClk,              // 400MHz - CCI-P clock domain. Primary interface clock
  input           logic             pClkDiv2,          // 200MHz - CCI-P clock domain.
  input           logic             pClkDiv4,          // 100MHz - CCI-P clock domain.
  input           logic             uClk_usr,          // User clock domain. Refer to clock programming guide  ** Currently provides fixed 300MHz clock **
  input           logic             uClk_usrDiv2,      // User clock domain. Half the programmed frequency  ** Currently provides fixed 150MHz clock **
  input           logic             pck_cp2af_softReset,      // CCI-P ACTIVE HIGH Soft Reset
  input           logic [1:0]       pck_cp2af_pwrState,       // CCI-P AFU Power State
  input           logic             pck_cp2af_error,          // CCI-P Protocol Error Detected

  // Interface structures
  input           t_if_ccip_Rx      pck_cp2af_sRx,        // CCI-P Rx Port
  output          t_if_ccip_Tx      pck_af2cp_sTx         // CCI-P Tx Port
);


   logic 	  reset_pass;   
   logic 	  afu_clk;   

   t_if_ccip_Tx afu_tx;
   t_if_ccip_Rx afu_rx;
   
   assign afu_clk =  uClk_usr;

   
   // AFU fmax @ 350 Mhz, using Async shim instead
   ccip_async_shim ccip_async_shim (
				    .bb_softreset    (pck_cp2af_softReset),
				    .bb_clk          (pClk),
				    .bb_tx           (pck_af2cp_sTx),
				    .bb_rx           (pck_cp2af_sRx),
				    .afu_softreset   (reset_pass),
				    .afu_clk         (afu_clk),
				    .afu_tx          (afu_tx),
				    .afu_rx          (afu_rx)
				    );

   
//===============================================================================================
// User AFU goes here
//===============================================================================================
mmio_stress_afu mmio_stress_afu (
				 .pClk                 ( afu_clk ) ,
				 .pck_cp2af_softReset  ( reset_pass ) ,
				 .pck_cp2af_sRx       ( afu_rx ) ,
				 .pck_af2cp_sTx       ( afu_tx )
				 );

endmodule
