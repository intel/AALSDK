import ccip_if_pkg::*;

module mmio_stress_afu
  (
   // CCI-P Clocks and Resets
   input logic 	     pClk,
   input logic 	     pClkDiv2,
   input logic 	     pClkDiv4,
   input logic 	     uClk_usr,
   input logic 	     uClk_usrDiv2,
   input logic 	     pck_cp2af_softReset,
   input logic [1:0] pck_cp2af_pwrState,
   input logic 	     pck_cp2af_error,
   input 	     t_if_ccip_Rx pck_cp2af_sRx,
   output 	     t_if_ccip_Tx pck_af2cp_sTx
   );


   logic [63:0]      afu_dfh [0:2];

   assign afu_dfh[0] = 64'h100000000028100A;
   assign afu_dfh[1] = 64'h96bf6f5fc4038fac;
   assign afu_dfh[2] = 64'h10c1bff188d14dfb;

   // Temp singal
   logic [14:0]      ram_addr;
   logic [7:0] 	     ram_byte_en;
   logic [63:0]      ram_data_in;
   logic [63:0]      ram_data_out;
   logic 	     ram_write_en;
   logic 	     ram_read_en;
   logic 	     ram_data_valid;

   logic [8:0] 	     ram_tid_out;
   logic [1:0] 	     ram_read_type;
   logic 	     ram_mask_loc;
   
   logic [63:0]      ram_data_masked;
   logic 	     ram_data_masked_valid;
         
   logic 	     clk;
   assign clk = pClk;

   // Instantiate RAM_1R1W
   ram_1r1w ram_1r1w
     (
      .address (ram_addr),
      .byteena (ram_byte_en),
      .clock   (clk),
      .data    (ram_data_in),
      .wren    (ram_write_en),
      .q       (ram_data_out)
      );

   // CCIP memory header & mapping
   t_ccip_c0_ReqMmioHdr CfgHdr;
   assign CfgHdr = t_ccip_c0_ReqMmioHdr'(pck_cp2af_sRx.c0.hdr);

   // logic [8:0] 	     mmio_rtid;
   
   
   // logic [63:0]      mmio_rdata_0;
   // logic [8:0] 	     mmio_rtid_0;
   // logic 	     mmio_rvalid_0;

   // logic [63:0]      mmio_rdata_1;
   // logic [8:0] 	     mmio_rtid_1;
   // logic 	     mmio_rvalid_1;


   /*
    * Soft reset, starts off a 4-cycle long internal reset
    */
   // Reset status
   // logic 	     softreset_q;
   logic 	     softreset;
   logic 	     internal_rst;
   // logic 	     rst_trigger;
   int 		     rst_counter;

   typedef enum {
		 ResetIdle,
		 ResetLine1,
		 ResetLine2,
		 ResetLine3,
		 ResetDone
		 } ResetStateEnum;
   ResetStateEnum rst_state = ResetIdle;

   assign softreset = pck_cp2af_softReset;

   // always @(posedge clk) begin
   //    softreset_q <= softreset;
   // end

   // always @(posedge clk) begin
   //    if (softreset & ~softreset_q) begin
   // 	 rst_trigger <= 1;
   //    end
   //    else begin
   // 	 rst_trigger <= 0;
   //    end
   // end

   /*
    * Main process =>
    * Reset initializer FSM
    * Read/Write process
    */
   always @(posedge clk) begin
      // -------------------------------------------- //
      // Reset Initializer
      // -------------------------------------------- //
      if (softreset) begin
	 case (rst_state)
   	   ResetIdle:
   	     begin
   		ram_write_en <= 0;
   		if (softreset) begin
   		   internal_rst <= 1;
   		   rst_state <= ResetLine1;
   		end
   		else begin
   		   internal_rst <= 0;
   		   rst_state <= ResetIdle;
   		end
   	     end

   	   ResetLine1:
   	     begin
   		internal_rst <= 1;
   		rst_state <= ResetLine2;
   		ram_write_en <= 1;
   		ram_data_in  <= afu_dfh[0];
		ram_byte_en  <= 8'hFF;
   		ram_addr     <= 14'h000;
   	     end

   	   ResetLine2:
   	     begin
   		internal_rst <= 1;
   		rst_state <= ResetLine3;
   		ram_write_en <= 1;
   		ram_data_in  <= afu_dfh[1];
		ram_byte_en  <= 8'hFF;
   		ram_addr     <= 14'h001;
   	     end

   	   ResetLine3:
   	     begin
   		internal_rst <= 1;
   		rst_state <= ResetDone;
   		ram_write_en <= 1;
   		ram_data_in  <= afu_dfh[2];
		ram_byte_en  <= 8'hFF;
   		ram_addr     <= 14'h002;
   	     end

   	   ResetDone:
   	     begin
   		internal_rst <= 1;
   		rst_state <= ResetIdle;
   		ram_write_en <= 0;
		ram_byte_en  <= 8'h00;
   		ram_addr     <= 14'h000;
   		ram_data_in  <= 64'b0;
   	     end

   	   default:
   	     begin
   		internal_rst <= 0;
   		rst_state <= ResetIdle;
   		ram_write_en <= 0;
   	     end
	 endcase
      end
      else begin
	 // ------------------------------------------------ //
	 // MMIO Write process (start to end)
	 // ------------------------------------------------ //
	 ram_write_en  <= 0;
	 ram_read_en   <= 0;	 
	 if (pck_cp2af_sRx.c0.mmioWrValid) begin
	    ram_write_en <= 1;
	    // 32-bit write
	    if (CfgHdr.length == 2'b00) begin
	       ram_addr    <= CfgHdr.address[15:1];
	       if (CfgHdr.address[0] == 0) begin
		  ram_byte_en <= 8'h0F;
		  ram_data_in <= {32'b0, pck_cp2af_sRx.c0.data[31:0]};
	       end
	       else if (CfgHdr.address[0] == 1) begin
		  ram_byte_en <= 8'hF0;
		  ram_data_in <= {pck_cp2af_sRx.c0.data[31:0], 32'b0};
	       end
	    end
	    // 64-bit write
	    else if (CfgHdr.length == 2'b01) begin
	       ram_addr    <= CfgHdr.address[15:1];
	       ram_byte_en <= 8'hFF;
	       ram_data_in <= pck_cp2af_sRx.c0.data[63:0];
	    end
	 end
	 // ------------------------------------------------ //
	 // MMIO Read process (start)
	 // ------------------------------------------------ //
	 else if (pck_cp2af_sRx.c0.mmioRdValid) begin
	    ram_write_en <= 0;
	    ram_byte_en  <= 8'hFF;
	    ram_addr     <= CfgHdr.address[15:1];
	    ram_tid_out  <= CfgHdr.tid;
	    ram_read_type <= CfgHdr.length;
	    ram_mask_loc  <= CfgHdr.address[0];
	    ram_read_en  <= 1;		  
	 end	 
      end
   end

   // Read Valid indicator
   always @(posedge clk) begin
      ram_data_valid <= ram_read_en;      
   end

   // Masked data
   always @(posedge clk) begin
      if (ram_data_valid) begin
	 ram_data_masked_valid <= 1;
	 // 32 vs 64
	 if (ram_read_type == 2'b00) begin
	    if (ram_mask_loc == 0) begin
	       ram_data_masked <= {32'b0, ram_data_out[31:0]};
	    end
	    else if (ram_mask_loc == 1) begin
	       ram_data_masked <= {32'b0, ram_data_out[63:32]};
	    end
	 end
	 else if (ram_read_type == 2'b01) begin
	    ram_data_masked <= ram_data_out;
	 end
      end
      else begin
	 ram_data_masked_valid <= 0;	 
      end
   end
  
   
   // MMIO Read Response (end)
   always @(posedge clk) begin
      if (softreset) begin
   	 pck_af2cp_sTx.c2.mmioRdValid <= 0;
      end
      else begin
   	 pck_af2cp_sTx.c2.hdr.tid     <= ram_tid_out;	 
   	 pck_af2cp_sTx.c2.data        <= ram_data_masked;	 
   	 pck_af2cp_sTx.c2.mmioRdValid <= ram_data_masked_valid;	 
      end
   end


   // TX channel 0 out
   assign pck_af2cp_sTx.c0 = 0;
   assign pck_af2cp_sTx.c1 = 0;


endmodule // mmio_stress_afu
