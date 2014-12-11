// ***************************************************************************
//
//        Copyright (C) 2014 Intel Corporation All Rights Reserved.
//
//
// Engineer:            Rahul R Sharma
// Create Date:         Mon Aug 11 13:25:05 PDT 2014
// Module Name:         cci_std_afu.sv
// Project:             CNN Acclerator POC 1
// Description:
//
//                    |-----------|    |------------|    
//  CCI_unordered --->| CCI FIFO  |--->| NLB_lpbk   |
//                    |-----------|    |------------|    
//
// ***************************************************************************

module cci_std_afu(
		   // Link/Protocol (LP) clocks and reset
		   input  /*var*/ logic vl_clk_LPdomain_32ui, // 32ui link/protocol clock domain
		   input  /*var*/ logic vl_clk_LPdomain_16ui, // 2x CCI interface clock. Synchronous.16ui link/protocol clock domain.
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_SystemReset_n, // System Reset
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_SoftReset_n, // CCI-S soft reset

		   // Native CCI Interface (cache line interface for back end)
		   /* Channel 0 can receive READ, WRITE, WRITE CSR responses.*/
		   input  /*var*/ logic [17:0] ffs_vl18_LP32ui_lp2sy_C0RxHdr, // System to LP header
		   input  /*var*/ logic [511:0] ffs_vl512_LP32ui_lp2sy_C0RxData, // System to LP data
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0RxWrValid, // RxWrHdr valid signal
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0RxRdValid, // RxRdHdr valid signal
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0RxCgValid, // RxCgHdr valid signal
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0RxUgValid, // Rx Umsg Valid signal
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0RxIrValid, // Rx Interrupt valid signal
		   /* Channel 1 reserved for WRITE RESPONSE ONLY */
		   input  /*var*/ logic [17:0] ffs_vl18_LP32ui_lp2sy_C1RxHdr, // System to LP header (Channel 1)
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C1RxWrValid, // RxData valid signal (Channel 1)
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C1RxIrValid, // Rx Interrupt valid signal (Channel 1)

		   /*Channel 0 reserved for READ REQUESTS ONLY */
		   output /*var*/ logic [60:0] ffs_vl61_LP32ui_sy2lp_C0TxHdr, // System to LP header
		   output /*var*/ logic ffs_vl_LP32ui_sy2lp_C0TxRdValid, // TxRdHdr valid signals
		   /*Channel 1 reserved for WRITE REQUESTS ONLY */
		   output /*var*/ logic [60:0] ffs_vl61_LP32ui_sy2lp_C1TxHdr, // System to LP header
		   output /*var*/ logic [511:0] ffs_vl512_LP32ui_sy2lp_C1TxData, // System to LP data
		   output /*var*/ logic ffs_vl_LP32ui_sy2lp_C1TxWrValid, // TxWrHdr valid signal
		   output /*var*/ logic ffs_vl_LP32ui_sy2lp_C1TxIrValid, // Tx Interrupt valid signal
		   /* Tx push flow control */
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C0TxAlmFull, // Channel 0 almost full
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_C1TxAlmFull, // Channel 1 almost full

		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_InitDnForSys           // System layer is aok to run
		   );


   logic 		  ffs_vl_LP32ui_lp2sy_Reset_n_rob;
   logic [60:0] 	  ffs_vl61_LP32ui_sy2lp_C0TxHdr_rob;
   logic 		  ffs_vl_LP32ui_sy2lp_C0TxRdValid_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C0TxAlmFull_rob;
   logic [60:0] 	  ffs_vl61_LP32ui_sy2lp_C1TxHdr_rob;
   logic [511:0] 	  ffs_vl512_LP32ui_sy2lp_C1TxData_rob;
   logic 		  ffs_vl_LP32ui_sy2lp_C1TxWrValid_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C1TxAlmFull_rob;
   logic [17:0] 	  ffs_vl18_LP32ui_lp2sy_C0RxHdr_rob;
   logic [511:0] 	  ffs_vl512_LP32ui_lp2sy_C0RxData_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxRdValid_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxWrValid_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxCgValid_rob;
   logic [17:0] 	  ffs_vl18_LP32ui_lp2sy_C1RxHdr_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_C1RxWrValid_rob;
   logic 		  ffs_vl_LP32ui_lp2sy_InitDnForSys_rob;


   logic 		  ffs_vl_LP32ui_lp2sy_Reset_n_async;
   logic [60:0] 	  ffs_vl61_LP32ui_sy2lp_C0TxHdr_async;
   logic 		  ffs_vl_LP32ui_sy2lp_C0TxRdValid_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C0TxAlmFull_async;
   logic [60:0] 	  ffs_vl61_LP32ui_sy2lp_C1TxHdr_async;
   logic [511:0] 	  ffs_vl512_LP32ui_sy2lp_C1TxData_async;
   logic 		  ffs_vl_LP32ui_sy2lp_C1TxWrValid_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C1TxAlmFull_async;
   logic [17:0] 	  ffs_vl18_LP32ui_lp2sy_C0RxHdr_async;
   logic [511:0] 	  ffs_vl512_LP32ui_lp2sy_C0RxData_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxRdValid_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxWrValid_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C0RxCgValid_async;
   logic [17:0] 	  ffs_vl18_LP32ui_lp2sy_C1RxHdr_async;
   logic 		  ffs_vl_LP32ui_lp2sy_C1RxWrValid_async;
   logic 		  ffs_vl_LP32ui_lp2sy_InitDnForSys_async;


   // Slower clock generation
   logic [2:0] 		  afu_clk_cnt = 3'b0;
   logic 		  afu_clk;

   always @(posedge vl_clk_LPdomain_32ui) begin
      afu_clk_cnt <= afu_clk_cnt + 1;
   end
   
   assign afu_clk = afu_clk_cnt[1];


   // CCI Asynchronous interface
   cci_async_shim cci_async_shim(
				 // -------- QLP <-> SHIM ---------- //
				 .qlp_clk                (vl_clk_LPdomain_32ui),
				 .qlp_resetb             (ffs_vl_LP32ui_lp2sy_SystemReset_n && ffs_vl_LP32ui_lp2sy_SoftReset_n),
				 .qlp_lp_initdone        (ffs_vl_LP32ui_lp2sy_InitDnForSys),
				 .qlp_tx_c0_header       (ffs_vl61_LP32ui_sy2lp_C0TxHdr ),
				 .qlp_tx_c0_rdvalid      (ffs_vl_LP32ui_sy2lp_C0TxRdValid),
				 .qlp_tx_c0_almostfull   (ffs_vl_LP32ui_lp2sy_C0TxAlmFull),
				 .qlp_tx_c1_header       (ffs_vl61_LP32ui_sy2lp_C1TxHdr ),
				 .qlp_tx_c1_data         (ffs_vl512_LP32ui_sy2lp_C1TxData),
				 .qlp_tx_c1_wrvalid      (ffs_vl_LP32ui_sy2lp_C1TxWrValid),
				 .qlp_tx_c1_almostfull   (ffs_vl_LP32ui_lp2sy_C1TxAlmFull),
				 .qlp_rx_c0_header       (ffs_vl18_LP32ui_lp2sy_C0RxHdr ),
				 .qlp_rx_c0_data         (ffs_vl512_LP32ui_lp2sy_C0RxData),
				 .qlp_rx_c0_rdvalid      (ffs_vl_LP32ui_lp2sy_C0RxRdValid),
				 .qlp_rx_c0_wrvalid      (ffs_vl_LP32ui_lp2sy_C0RxWrValid),
				 .qlp_rx_c0_cfgvalid     (ffs_vl_LP32ui_lp2sy_C0RxCgValid),
				 .qlp_rx_c1_header       (ffs_vl18_LP32ui_lp2sy_C1RxHdr),
				 .qlp_rx_c1_wrvalid      (ffs_vl_LP32ui_lp2sy_C1RxWrValid),
				 // -------- SHIM <-> CAFU ---------- //
				 .afu_clk                (afu_clk),
				 .afu_resetb             (ffs_vl_LP32ui_lp2sy_Reset_n_async),
				 .afu_lp_initdone        (ffs_vl_LP32ui_lp2sy_InitDnForSys_async),
				 .afu_tx_c0_header       (ffs_vl61_LP32ui_sy2lp_C0TxHdr_async),
				 .afu_tx_c0_rdvalid      (ffs_vl_LP32ui_sy2lp_C0TxRdValid_async),
				 .afu_tx_c0_almostfull   (ffs_vl_LP32ui_lp2sy_C0TxAlmFull_async),
				 .afu_tx_c1_header       (ffs_vl61_LP32ui_sy2lp_C1TxHdr_async),
				 .afu_tx_c1_data         (ffs_vl512_LP32ui_sy2lp_C1TxData_async),
				 .afu_tx_c1_wrvalid      (ffs_vl_LP32ui_sy2lp_C1TxWrValid_async),
				 .afu_tx_c1_almostfull   (ffs_vl_LP32ui_lp2sy_C1TxAlmFull_async),
				 .afu_rx_c0_header       (ffs_vl18_LP32ui_lp2sy_C0RxHdr_async),
				 .afu_rx_c0_data         (ffs_vl512_LP32ui_lp2sy_C0RxData_async),
				 .afu_rx_c0_rdvalid      (ffs_vl_LP32ui_lp2sy_C0RxRdValid_async),
				 .afu_rx_c0_wrvalid      (ffs_vl_LP32ui_lp2sy_C0RxWrValid_async),
				 .afu_rx_c0_cfgvalid     (ffs_vl_LP32ui_lp2sy_C0RxCgValid_async),
				 .afu_rx_c1_header       (ffs_vl18_LP32ui_lp2sy_C1RxHdr_async),
				 .afu_rx_c1_wrvalid      (ffs_vl_LP32ui_lp2sy_C1RxWrValid_async)
				 );


   // CCI Reordering shim
   cci_reorder_shim rob_interface(
				  .clk                             (afu_clk),
				  // -----------Unordered Interface (QLP <-> ROB) -------------- //
				  .qlp_resetb                    (ffs_vl_LP32ui_lp2sy_Reset_n_async),
				  .qlp_lp_initdone               (ffs_vl_LP32ui_lp2sy_InitDnForSys_async),
				  .qlp_tx_c0_header              (ffs_vl61_LP32ui_sy2lp_C0TxHdr_async),
				  .qlp_tx_c0_rdvalid             (ffs_vl_LP32ui_sy2lp_C0TxRdValid_async),
				  .qlp_tx_c0_almostfull          (ffs_vl_LP32ui_lp2sy_C0TxAlmFull_async),
				  .qlp_tx_c1_header              (ffs_vl61_LP32ui_sy2lp_C1TxHdr_async),
				  .qlp_tx_c1_data                (ffs_vl512_LP32ui_sy2lp_C1TxData_async),
				  .qlp_tx_c1_wrvalid             (ffs_vl_LP32ui_sy2lp_C1TxWrValid_async),
				  .qlp_tx_c1_almostfull          (ffs_vl_LP32ui_lp2sy_C1TxAlmFull_async),
				  .qlp_rx_c0_header              (ffs_vl18_LP32ui_lp2sy_C0RxHdr_async),
				  .qlp_rx_c0_data                (ffs_vl512_LP32ui_lp2sy_C0RxData_async),
				  .qlp_rx_c0_rdvalid             (ffs_vl_LP32ui_lp2sy_C0RxRdValid_async),
				  .qlp_rx_c0_wrvalid             (ffs_vl_LP32ui_lp2sy_C0RxWrValid_async),
				  .qlp_rx_c0_cfgvalid            (ffs_vl_LP32ui_lp2sy_C0RxCgValid_async),
				  .qlp_rx_c1_header              (ffs_vl18_LP32ui_lp2sy_C1RxHdr_async),
				  .qlp_rx_c1_wrvalid             (ffs_vl_LP32ui_lp2sy_C1RxWrValid_async),
				  // ------------- Ordered Interface (ROB <-> AFU) ------------ //
				  .afu_resetb                    (ffs_vl_LP32ui_lp2sy_Reset_n_rob),
				  .afu_lp_initdone               (ffs_vl_LP32ui_lp2sy_InitDnForSys_rob),
				  .afu_tx_c0_header              (ffs_vl61_LP32ui_sy2lp_C0TxHdr_rob),
				  .afu_tx_c0_rdvalid             (ffs_vl_LP32ui_sy2lp_C0TxRdValid_rob),
				  .afu_tx_c0_almostfull          (ffs_vl_LP32ui_lp2sy_C0TxAlmFull_rob),
				  .afu_tx_c1_header              (ffs_vl61_LP32ui_sy2lp_C1TxHdr_rob),
				  .afu_tx_c1_data                (ffs_vl512_LP32ui_sy2lp_C1TxData_rob),
				  .afu_tx_c1_wrvalid             (ffs_vl_LP32ui_sy2lp_C1TxWrValid_rob),
				  .afu_tx_c1_almostfull          (ffs_vl_LP32ui_lp2sy_C1TxAlmFull_rob),
				  .afu_rx_c0_header              (ffs_vl18_LP32ui_lp2sy_C0RxHdr_rob),
				  .afu_rx_c0_data                (ffs_vl512_LP32ui_lp2sy_C0RxData_rob),
				  .afu_rx_c0_rdvalid             (ffs_vl_LP32ui_lp2sy_C0RxRdValid_rob),
				  .afu_rx_c0_wrvalid             (ffs_vl_LP32ui_lp2sy_C0RxWrValid_rob),
				  .afu_rx_c0_cfgvalid            (ffs_vl_LP32ui_lp2sy_C0RxCgValid_rob),
				  .afu_rx_c1_header              (ffs_vl18_LP32ui_lp2sy_C1RxHdr_rob),
				  .afu_rx_c1_wrvalid             (ffs_vl_LP32ui_lp2sy_C1RxWrValid_rob)
				  );


   // Loopback module with 512 bit CCI Native interface
   nlb_lpbk nlb_lpbk(
                     .Clk_32UI                           (afu_clk),
                     .Resetb                             (ffs_vl_LP32ui_lp2sy_Reset_n_rob),
                     .ci2cf_InitDn                       (ffs_vl_LP32ui_lp2sy_InitDnForSys_rob),
                     .rb2cf_C0RxHdr                      (ffs_vl18_LP32ui_lp2sy_C0RxHdr_rob),
                     .rb2cf_C0RxData                     (ffs_vl512_LP32ui_lp2sy_C0RxData_rob),
                     .rb2cf_C0RxWrValid                  (ffs_vl_LP32ui_lp2sy_C0RxWrValid_rob),
                     .rb2cf_C0RxRdValid                  (ffs_vl_LP32ui_lp2sy_C0RxRdValid_rob),
                     .rb2cf_C0RxCfgValid                 (ffs_vl_LP32ui_lp2sy_C0RxCgValid_rob),
                     .rb2cf_C0RxUMsgValid                (1'b0),
                     .rb2cf_C0RxIntrValid                (1'b0),
                     .rb2cf_C1RxHdr                      (ffs_vl18_LP32ui_lp2sy_C1RxHdr_rob),
                     .rb2cf_C1RxWrValid                  (ffs_vl_LP32ui_lp2sy_C1RxWrValid_rob),
                     .rb2cf_C1RxIntrValid                (1'b0),
                     .cf2ci_C0TxHdr                      (ffs_vl61_LP32ui_sy2lp_C0TxHdr_rob),
		     .cf2ci_C0TxRdValid                  (ffs_vl_LP32ui_sy2lp_C0TxRdValid_rob),
                     .cf2ci_C1TxHdr                      (ffs_vl61_LP32ui_sy2lp_C1TxHdr_rob),
                     .cf2ci_C1TxData                     (ffs_vl512_LP32ui_sy2lp_C1TxData_rob),
                     .cf2ci_C1TxWrValid                  (ffs_vl_LP32ui_sy2lp_C1TxWrValid_rob),
		     .cf2ci_C1TxIntrValid                (),
                     .ci2cf_C0TxAlmFull                  (ffs_vl_LP32ui_lp2sy_C0TxAlmFull_rob),
                     .ci2cf_C1TxAlmFull                  (ffs_vl_LP32ui_lp2sy_C1TxAlmFull_rob)
                     );

endmodule
