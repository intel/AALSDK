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
 * Module Info: ASE top-level
 *              (hides ASE machinery, makes finding cci_std_afu easy)
 * Language   : System{Verilog}
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * **************************************************************************/

import ccip_if_pkg::*;

`timescale 1ns/1ns

module ase_top();


   logic vl_clk_LPdomain_64ui;
   logic vl_clk_LPdomain_32ui;
   logic vl_clk_LPdomain_16ui;
   logic ffs_LP16ui_afu_SoftReset_n;
   t_if_ccip_Tx ffs_LP16ui_sTxData_afu;
   t_if_ccip_Rx ffs_LP16ui_sRxData_afu;

   logic [1:0] ffs_LP16ui_afu_PwrState;   // CCI-P AFU Power State
   logic       ffs_LP16ui_afu_Error;      // CCI-P Protocol Error Detected
   
   // CCI-P emulator
   ccip_emulator ccip_emulator
     (
      .vl_clk_LPdomain_64ui               (vl_clk_LPdomain_64ui         ),
      .vl_clk_LPdomain_32ui               (vl_clk_LPdomain_32ui         ),
      .vl_clk_LPdomain_16ui               (vl_clk_LPdomain_16ui         ),
      .ffs_LP16ui_afu_SoftReset_n         (ffs_LP16ui_afu_SoftReset_n   ),
      .ffs_LP16ui_afu_PwrState            (ffs_LP16ui_afu_PwrState      ),
      .ffs_LP16ui_afu_Error               (ffs_LP16ui_afu_Error         ),
      .ffs_LP16ui_sTxData_afu		  (ffs_LP16ui_sTxData_afu       ),
      .ffs_LP16ui_sRxData_afu             (ffs_LP16ui_sRxData_afu       )
      );


   // CCIP AFU
   ccip_std_afu ccip_std_afu
     (
      .vl_clk_LPdomain_64ui               (vl_clk_LPdomain_64ui         ),
      .vl_clk_LPdomain_32ui               (vl_clk_LPdomain_32ui         ),
      .vl_clk_LPdomain_16ui		  (vl_clk_LPdomain_16ui         ),
      .ffs_LP16ui_afu_SoftReset_n         (ffs_LP16ui_afu_SoftReset_n   ),
      .ffs_LP16ui_afu_PwrState            (ffs_LP16ui_afu_PwrState      ),
      .ffs_LP16ui_afu_Error               (ffs_LP16ui_afu_Error         ),
      .ffs_LP16ui_sTxData_afu		  (ffs_LP16ui_sTxData_afu       ),
      .ffs_LP16ui_sRxData_afu		  (ffs_LP16ui_sRxData_afu       )
      );

endmodule
