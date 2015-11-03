/*
 * Config Write
 * ------------
 * => Config writes originate from SW to AFU
 *
 * Config Read
 * -----------
 * => Config read request originates from SW to AFU
 *    => this block attaches a tag Id to it and passes it to AFU
 *    => Tag must be returned back with a RdData and MMIO response
 * => This block will generate the tag Id
 *
 */

import ase_pkg::*;

`include "platform.vh"

module mmio_block
   (
    input logic 			     clk,
    input logic 			     rst,
    // MMIO Read/Write request
    output 				     CfgHdr_t mmio_hdr,
    output logic 			     mmio_wrvalid,
    output logic 			     mmio_rdvalid,
    output logic [CCIP_DATA_WIDTH-1:0] 	     mmio_data,
    // MMIO read response
    input logic 			     mmio_rspvalid,
    input logic [CCIP_MMIO_TID_WIDTH-1:0]    mmio_rsptid,
    input logic [CCIP_MMIO_RDDATA_WIDTH-1:0] mmio_rspdata,
    // Pop control (MMIO req)
    input logic 			     cfg_pop
    );


   /*
    *
    */
   // MMIO read tid counter
   logic [CCIP_MMIO_TID_WIDTH-1:0] 	     tid_counter;

   // TID:Address tuple storage
   int 					     unsigned tid_array[*];

   /*
    * CSR Read/Write infrastructure
    * csr_write_dispatch: A Single task to dispatch CSR Writes
    * Storage format = <wrvalid, rdvalid, hdr_width, data_width>
    *
    */
   parameter int MMIOREQ_FIFO_WIDTH = 2 + CCIP_CFG_HDR_WIDTH + CCIP_DATA_WIDTH;

   logic [MMIOREQ_FIFO_WIDTH-1:0] mmioreq_din;
   logic [MMIOREQ_FIFO_WIDTH-1:0] mmioreq_dout;
   logic 		      mmioreq_write;
   logic 		      mmioreq_pop;
   logic 		      mmioreq_read;
   logic 		      mmioreq_valid;
   logic 		      mmioreq_full;
   logic 		      mmioreq_empty;

   logic [CCIP_MMIO_INDEX_WIDTH-1:0] cwlp_header;
   logic [CCIP_DATA_WIDTH-1:0] 	     cwlp_data;
   logic 			     cwlp_wrvalid;
   logic 			     cwlp_rdvalid;

   // MMIO dispatch unit
   task mmio_dispatch(int initialize ,           // Init channel
		      int     mmio_wren_in , // write = 1, read = 0
		      int     mmio_addr_in , // MMIO address
		      longint mmio_data_in , // WrOnly data
		      int     mmio_dwidth_in     // WrOnly dwidth
		      );
      CfgHdr_t hdr;
      begin
	 if (initialize) begin
	    cwlp_wrvalid = 0;
	    cwlp_rdvalid = 0;
	    cwlp_header  = 0;
	    cwlp_data    = 0;
	    tid_counter  = 0;
	 end
	 else begin
	    if (mmio_wren_in == 1) begin
	       hdr.index  = {2'b0, mmio_addr_in[15:2]};
	       hdr.poison = 1'b0;
	       hdr.tid    = 9'b0;
	       if (mmio_dwidth_in == 32) begin
		  hdr.len = 2'b0;
		  cwlp_header = CCIP_CFG_HDR_WIDTH'(hdr);
		  cwlp_data = {480'b0, mmio_data_in[31:0]};
	       end
	       else if (mmio_dwidth_in == 64) begin
		  hdr.len = 2'b01;
		  cwlp_header = CCIP_CFG_HDR_WIDTH'(hdr);
		  cwlp_data = {480'b0, mmio_data_in[63:0]};
	       end
	       cwlp_wrvalid = 1;
	       cwlp_rdvalid = 0;
	    end
	    else if (mmio_wren_in == 0) begin
	       cwlp_data    = 0;
	       hdr.index    = {2'b0, mmio_addr_in[15:2]};
	       hdr.len      = 2'b01;
	       hdr.poison   = 1'b0;
	       hdr.tid      = tid_counter;
	       cwlp_header  = CCIP_CFG_HDR_WIDTH'(hdr);
	       cwlp_wrvalid = 0;
	       cwlp_rdvalid = 1;
	       tid_array[ tid_counter ] = hdr.index;
    	       tid_counter  = tid_counter + 1;
	    end
	    @(posedge clk);
	    cwlp_wrvalid = 0;
	    cwlp_rdvalid = 0;
	 end
      end
   endtask

   // CSR readreq/write FIFO data
   assign mmioreq_din = {cwlp_wrvalid, cwlp_rdvalid, cwlp_header, cwlp_data};

   // Request staging
   ase_fifo
     #(
       .DATA_WIDTH     ( MMIOREQ_FIFO_WIDTH ),
       .DEPTH_BASE2    ( 10 ),
       .ALMFULL_THRESH ( 960 )
       )
   mmioreq_fifo
     (
      .clk        ( clk ),
      .rst        ( ~sys_reset_n ),
      .wr_en      ( mmioreq_write ),
      .data_in    ( mmioreq_din ),
      .rd_en      ( mmioreq_pop ),
      .data_out   ( mmioreq_dout ),
      .data_out_v ( mmioreq_valid ),
      .alm_full   ( mmioreq_full ),
      .full       (  ),
      .empty      ( mmioreq_empty ),
      .count      (  ),
      .overflow   (  ),
      .underflow  (  )
      );

   assign mmioreq_pop  = ~mmioreq_empty && cfg_pop;
   assign mmio_wrvalid = ~mmioreq_empty;
   assign mmio_data    = mmioreq_dout[CCIP_DATA_WIDTH-1:0];
   assign mmio_hdr     = CfgHdr_t'( mmioreq_dout[(CCIP_DATA_WIDTH+CCIP_CFG_HDR_WIDTH-1):CCIP_DATA_WIDTH] );


   /*
    * MMIO Read response
    */
   parameter int MMIORESP_FIFO_WIDTH = CCIP_MMIO_TID_WIDTH + CCIP_MMIO_RDDATA_WIDTH;

   logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_din;
   logic [MMIORESP_FIFO_WIDTH-1:0] mmioresp_dout;
   logic 			   mmioresp_write;
   logic 			   mmioresp_pop;
   logic 			   mmioresp_read;
   logic 			   mmioresp_valid;
   logic 			   mmioresp_full;
   logic 			   mmioresp_empty;

   import "DPI-C" function void mmio_update(inout int mmioaddr64, inout bit [63:0] mmiodata );

   // Response staging FIFO
   ase_fifo
     #(
     .DATA_WIDTH     ( MMIORESP_FIFO_WIDTH ),
     .DEPTH_BASE2    ( 4 ),
     .ALMFULL_THRESH ( 10 )
       )
   mmioresp_fifo
     (
     .clk        ( clk ),
     .rst        ( ~sys_reset_n ),
     .wr_en      ( mmioresp_write ),
     .data_in    ( mmioresp_din ),
     .rd_en      ( mmioresp_pop ),
     .data_out   ( mmioresp_dout ),
     .data_out_v ( mmioresp_valid ),
     .alm_full   ( mmioresp_full ),
     .full       (  ),
     .empty      ( mmioresp_empty ),
     .count      (  ),
     .overflow   (  ),
     .underflow  (  )
       );

   assign mmioresp_din = { mmio_rsptid, mmio_rspdata };
   assign mmioresp_write = mmio_rspvalid;

   // FIFO writes to memory
   always @(posedge clk) begin
      if (~sys_reset_n) begin
	 mmioresp_read <= 0;
      end
      else begin
	 if (~mmioresp_empty) begin
	    mmioresp_read <= 1;
	    mmio_update( {tid_array[ mmioresp_dout[MMIORESP_FIFO_WIDTH-1:CCIP_MMIO_RDDATA_WIDTH] ], 2'b0} , mmioresp_dout[CCIP_MMIO_RDDATA_WIDTH-1:0]);
	 end
	 else begin
	    mmioresp_read <= 0;
	 end
      end
   end

endmodule
