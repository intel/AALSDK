/* ****************************************************************************
 * Copyright (c) 2011-2014, Intel Corporation
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
 
`include "ase_global.vh"
`include "platform.vh"

`timescale 1ns/1ns

module ase_top();

   // Connecting signals
   logic                          clk_32ui;                // out
   logic                          clk_16ui;                // out
   logic 			  sys_reset_n;             // out
   logic 			  sw_reset_n;              // out
   logic 			  lp_initdone ;            // out
   logic [`CCI_TX_HDR_WIDTH-1:0]  tx_c0_header;            // in
   logic 			  tx_c0_rdvalid;           // in
   logic 			  tx_c0_almostfull;        // out
   logic [`CCI_TX_HDR_WIDTH-1:0]  tx_c1_header;            // in
   logic [`CCI_DATA_WIDTH-1:0] 	  tx_c1_data;              // in
   logic 			  tx_c1_wrvalid;           // in
   logic 			  tx_c1_almostfull;        // out
   logic [`ASE_CCI_RX_HDR_WIDTH-1:0]  rx_c0_header;            // out
   logic [`CCI_DATA_WIDTH-1:0] 	  rx_c0_data;              // out
   logic 			  rx_c0_rdvalid;           // out
   logic 			  rx_c0_wrvalid;           // out
   logic 			  rx_c0_cfgvalid;          // out
   logic [`ASE_CCI_RX_HDR_WIDTH-1:0]  rx_c1_header;            // out
   logic 			  rx_c1_wrvalid;           // out
   logic 			  rx_c0_umsgvalid;         // out
   logic 			  tx_c1_intrvalid;         // in
   logic 			  rx_c0_intrvalid;         // out
   logic 			  rx_c1_intrvalid;         // out

   
   /*
    * CCI Emulator: ASE generator block
    */ 
   cci_emulator cci_emulator (
			      .clk_32ui         (clk_32ui       ),
			      .clk_16ui         (clk_16ui       ),
			      .sys_reset_n      (sys_reset_n    ),
			      .sw_reset_n       (sw_reset_n     ),
			      .lp_initdone      (lp_initdone    ), 
			      .tx_c0_header     (tx_c0_header   ),
			      .tx_c0_rdvalid    (tx_c0_rdvalid  ),
			      .tx_c0_almostfull (tx_c0_almostfull),
			      .tx_c1_header     (tx_c1_header   ),
			      .tx_c1_data       (tx_c1_data     ),
			      .tx_c1_wrvalid    (tx_c1_wrvalid  ),
			      .tx_c1_almostfull (tx_c1_almostfull),
			      .rx_c0_header     (rx_c0_header   ),
			      .rx_c0_data       (rx_c0_data     ),
			      .rx_c0_rdvalid    (rx_c0_rdvalid   ),
			      .rx_c0_wrvalid    (rx_c0_wrvalid   ),
			      .rx_c0_cfgvalid   (rx_c0_cfgvalid  ),
			      .rx_c1_header     (rx_c1_header    ),
			      .rx_c1_wrvalid    (rx_c1_wrvalid   ),
			      .rx_c0_umsgvalid  (rx_c0_umsgvalid ),
			      .tx_c1_intrvalid  (tx_c1_intrvalid ),
			      .rx_c0_intrvalid  (rx_c0_intrvalid ),
			      .rx_c1_intrvalid  (rx_c1_intrvalid )       
			      );
      
   
   /* ****************************************************************
    * CAFU = cci_std_afu instantiated
    *              ASE   |             |   CAFU or (SPL + AFU)
    *                  TX|------------>|RX
    *                    |             |
    *                  RX|<------------|TX
    *                    |             |
    * ***************************************************************/
   cci_std_afu cci_std_afu (
			    /* Link/Protocol (LP) clocks and reset */
			    .vl_clk_LPdomain_32ui             ( clk_32ui ),
			    .vl_clk_LPdomain_16ui             ( clk_16ui ),
			    .ffs_vl_LP32ui_lp2sy_InitDnForSys ( lp_initdone ),
			    .ffs_vl_LP32ui_lp2sy_SystemReset_n( sys_reset_n ),
			    .ffs_vl_LP32ui_lp2sy_SoftReset_n  ( sw_reset_n ),
			    /* Channel 0 can receive READ, WRITE, WRITE CSR responses.*/
			    .ffs_vl18_LP32ui_lp2sy_C0RxHdr    ( rx_c0_header ),
			    .ffs_vl512_LP32ui_lp2sy_C0RxData  ( rx_c0_data ),
			    .ffs_vl_LP32ui_lp2sy_C0RxWrValid  ( rx_c0_wrvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxRdValid  ( rx_c0_rdvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxCgValid  ( rx_c0_cfgvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxUgValid  ( rx_c0_umsgvalid ),
			    .ffs_vl_LP32ui_lp2sy_C0RxIrValid  ( rx_c0_intrvalid ),
			    /* Channel 1 reserved for WRITE RESPONSE ONLY */
			    .ffs_vl18_LP32ui_lp2sy_C1RxHdr    ( rx_c1_header ),
			    .ffs_vl_LP32ui_lp2sy_C1RxWrValid  ( rx_c1_wrvalid ),
			    .ffs_vl_LP32ui_lp2sy_C1RxIrValid  ( rx_c1_intrvalid ),
			    /*Channel 0 reserved for READ REQUESTS ONLY */
			    .ffs_vl61_LP32ui_sy2lp_C0TxHdr    ( tx_c0_header ),
			    .ffs_vl_LP32ui_sy2lp_C0TxRdValid  ( tx_c0_rdvalid ),
			    /*Channel 1 reserved for WRITE REQUESTS ONLY */
			    .ffs_vl61_LP32ui_sy2lp_C1TxHdr    ( tx_c1_header ),
			    .ffs_vl512_LP32ui_sy2lp_C1TxData  ( tx_c1_data ),
			    .ffs_vl_LP32ui_sy2lp_C1TxWrValid  ( tx_c1_wrvalid ),
			    .ffs_vl_LP32ui_sy2lp_C1TxIrValid  ( tx_c1_intrvalid ),
			    /* Tx push flow control */
			    .ffs_vl_LP32ui_lp2sy_C0TxAlmFull  ( tx_c0_almostfull ),
			    .ffs_vl_LP32ui_lp2sy_C1TxAlmFull  ( tx_c1_almostfull )
			    ); 
   
endmodule