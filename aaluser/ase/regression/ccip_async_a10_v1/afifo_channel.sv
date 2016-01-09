module afifo_channel
  #(
    parameter DATA_WIDTH = 32,
    parameter DEPTH_RADIX = 3
    )
   (
    // Clock & reset
    input logic 		  rst,
    input logic 		  wrclk,
    input logic 		  rdclk,
    // Write interface
    input logic [DATA_WIDTH-1:0]  data_in,
    input logic 		  write_en,
    // Read interface
    output logic [DATA_WIDTH-1:0] data_out,
    output logic 		  data_vld,
    // Fill status signal
    output logic 		  wrfull,
    // Error signals
    output logic 		  overflow,
    output logic 		  underflow
    );

   logic 			  aclr;
   
   always @(posedge wrclk) begin
      aclr <= rst;      
   end
   
   logic 			   read_en;
   logic 			   rdempty;
   logic 			   rdfull;
   logic [DEPTH_RADIX-1:0] 	   rdcount;
   logic 			   wrempty;
   logic [DEPTH_RADIX-1:0] 	   wrcount;

   logic [DATA_WIDTH-1:0] 	   data_out_tmp;
      
   // Error signals
   assign overflow = wrfull & write_en;
   assign underflow = rdempty & read_en;

   // Read/pop control   
   assign read_en = ~rdempty;
   
   always @(posedge rdclk) begin
      data_vld <= read_en & ~rdempty;      
   end
   
   always @(posedge rdclk) begin
      if (read_en) begin
	 data_out <= data_out_tmp;	 
      end
      else begin
	 data_out <= {DATA_WIDTH{1'b0}};	 
      end
   end  
   
   /*
    * dcfifo component
    */ 
   dcfifo dcfifo_component (
			    .aclr        (aclr),
			    .data        (data_in),
			    .rdclk       (rdclk),
			    .rdreq       (read_en),
			    .wrclk       (wrclk),
			    .wrreq       (write_en),
			    .q           (data_out_tmp),
			    .rdempty     (rdempty),
			    .rdfull      (rdfull),
			    .rdusedw     (rdcount),
			    .wrempty     (wrempty),
			    .wrfull      (wrfull),
			    .wrusedw     (wrcount),
			    .eccstatus   ()
			    );
   
   defparam
     dcfifo_component.enable_ecc              = "FALSE",
     dcfifo_component.intended_device_family  = "Arria 10",
     dcfifo_component.lpm_hint                = "DISABLE_DCFIFO_EMBEDDED_TIMING_CONSTRAINT=TRUE",
     dcfifo_component.lpm_numwords            = 2**DEPTH_RADIX,
     dcfifo_component.lpm_showahead           = "ON",
     dcfifo_component.lpm_type                = "dcfifo",
     dcfifo_component.lpm_width               = DATA_WIDTH,
     dcfifo_component.lpm_widthu              = DEPTH_RADIX,
     dcfifo_component.overflow_checking       = "ON",
     dcfifo_component.rdsync_delaypipe        = 5,
     dcfifo_component.underflow_checking      = "ON",
     dcfifo_component.use_eab                 = "ON",
     dcfifo_component.write_aclr_synch        = "ON",
     dcfifo_component.read_aclr_synch         = "OFF",
     dcfifo_component.wrsync_delaypipe        = 5;
   
endmodule
