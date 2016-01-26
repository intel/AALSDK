// ***************************************************************************
// Rahul R Sharma : A test AFU that tries to blow up ASE in different ways
//
// ***************************************************************************


module cci_std_afu (
		   // Link/Protocol (LP) clocks and reset
		   input  /*var*/ logic vl_clk_LPdomain_32ui, // 32ui link/protocol clock domain
		   input  /*var*/ logic ffs_vl_LP32ui_lp2sy_Reset_n, // AFU soft reset

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

   // Rule checker test
   initial begin
      $display ("test started");
      
      ffs_vl61_LP32ui_sy2lp_C0TxHdr = 61'b0;
      ffs_vl_LP32ui_sy2lp_C0TxRdValid = 1'b0;
      ffs_vl61_LP32ui_sy2lp_C1TxHdr = 61'b0;
      ffs_vl512_LP32ui_sy2lp_C1TxData = 512'b0;
      ffs_vl_LP32ui_sy2lp_C1TxWrValid = 1'b0;
      ffs_vl_LP32ui_sy2lp_C1TxIrValid = 1'b0;
      
      wait (ffs_vl_LP32ui_lp2sy_Reset_n == 0);
      wait (ffs_vl_LP32ui_lp2sy_InitDnForSys == 1);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      @(posedge vl_clk_LPdomain_32ui);
      wait (ffs_vl_LP32ui_lp2sy_C0TxAlmFull == 0);
      $display ("Almfull");      
      @(posedge vl_clk_LPdomain_32ui);      
      ffs_vl_LP32ui_sy2lp_C0TxRdValid = 1;      
      ffs_vl61_LP32ui_sy2lp_C0TxHdr = 61'hxZxx_xxxx_xxxx_xxxx;      
      $display ("Validated xxxxx");      
      @(posedge vl_clk_LPdomain_32ui);
      ffs_vl_LP32ui_sy2lp_C0TxRdValid = 0;      
      @(posedge vl_clk_LPdomain_32ui);
   end

endmodule
