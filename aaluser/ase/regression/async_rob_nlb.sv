import ccip_if_pkg::*;

module ccip_std_afu
(
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

   logic 	  reset_pass_n1, reset_pass_n2;
   logic 	  afu_clk;
   
   t_if_ccip_Tx   afu2rob_tx, rob2async_tx, async2bb_tx;
   t_if_ccip_Rx   bb2async_rx, async2rob_rx, rob2afu_rx;

   assign afu_clk = vl_clk_LPdomain_16ui;
      
   ccip_async_shim ccip_async_shim
     (
      .bb_softreset_n (ffs_LP16ui_afu_SoftReset_n),
      .bb_clk         (vl_clk_LPdomain_16ui),
      .bb_tx          (ffs_LP16ui_sTxData_afu),
      .bb_rx          (ffs_LP16ui_sRxData_afu),
      .afu_softreset_n (reset_pass_n1),
      .afu_clk         (afu_clk),
      .afu_tx          (rob2async_tx),
      .afu_rx          (async2rob_rx),
      .overflow        (),
      .underflow       ()
      );

   ccip_reorder_buffer ccip_reorder_buffer
     (
      .bb_softreset_n  (reset_pass_n1),
      .clk             (afu_clk),
      .bb_tx           (rob2async_tx),
      .bb_rx           (async2rob_rx),
      .afu_softreset_n (reset_pass_n2),
      .afu_tx          (afu2rob_tx),
      .afu_rx          (rob2afu_rx)
      );
 
   
   // NLB AFU- provides validation, performance characterization modes. It also serves as a reference design
   nlb_lpbk nlb_lpbk(
		     .Clk_16UI            ( afu_clk ),
		     .SoftReset_n         ( reset_pass_n2 ),
		     .SystemReset_n       ( 1'b1 ) ,
		     .cf2ci_sTxPort       ( afu2rob_tx ),
		     .ci2cf_sRxPort       ( rob2afu_rx )
		     );


endmodule // ccip_std_afu
