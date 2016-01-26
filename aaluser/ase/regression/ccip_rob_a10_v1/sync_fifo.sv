module sync_fifo
  #(
    parameter int FIFO_DATA_WIDTH = 512,
    parameter int FIFO_DEPTH_RADIX = 3
    )
   (
    input  logic [FIFO_DATA_WIDTH-1:0]  data,  //  fifo_input.datain
    input  logic                        wrreq, //            .wrreq
    input  logic                        rdreq, //            .rdreq
    input  logic                        clock, //            .clk
    input  logic                        sclr,  //            .sclr
    output logic [FIFO_DATA_WIDTH-1:0]  q,     // fifo_output.dataout
    output logic [FIFO_DEPTH_RADIX-1:0] usedw, //            .usedw
    output logic                        full,  //            .full
    output logic                        empty  //            .empty
    );
   
   // QSYS component
   scfifo  scfifo_component (
			     .clock (clock),
			     .data (data),
			     .rdreq (rdreq),
			     .sclr (sclr),
			     .wrreq (wrreq),
			     .empty (empty),
			     .full (full),
			     .q (q),
			     .usedw (usedw),
			     .aclr (),
			     .almost_empty (),
			     .almost_full (),
			     .eccstatus ()
			     );

   defparam
     scfifo_component.add_ram_output_register  = "OFF",
     scfifo_component.enable_ecc  = "FALSE",
     scfifo_component.intended_device_family  = "Arria 10",
     scfifo_component.lpm_hint  = "DISABLE_DCFIFO_EMBEDDED_TIMING_CONSTRAINT=TRUE",
     scfifo_component.lpm_numwords  = 2**FIFO_DEPTH_RADIX,
     scfifo_component.lpm_showahead  = "OFF",
     scfifo_component.lpm_type  = "scfifo",
     scfifo_component.lpm_width  = FIFO_DATA_WIDTH,
     scfifo_component.lpm_widthu  = FIFO_DEPTH_RADIX,
     scfifo_component.overflow_checking  = "ON",
     scfifo_component.underflow_checking  = "ON",
     scfifo_component.use_eab  = "ON";
   
endmodule // sync_fifo

