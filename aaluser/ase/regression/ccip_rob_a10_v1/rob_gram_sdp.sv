/*
 * ROB GRAM SDP
 * - Input address is registered, dout is async - account for 1 clk latency
 */

 
module rob_gram_sdp
  #(
    parameter ADDR_WIDTH = 4,
    parameter DATA_WIDTH = 32    
    )
   (
    input logic clk,
    input logic write_en,
    input logic [ADDR_WIDTH-1:0] waddr,
    input logic [DATA_WIDTH-1:0] din,
    input logic read_en,
    input logic [ADDR_WIDTH-1:0] raddr,
    output logic [DATA_WIDTH-1:0] dout
    );

    altera_syncram  altera_syncram_component (
                .address_a (waddr),
                .address_b (raddr),
                .clock0 (clk),
                .data_a (din),
                .rden_b (read_en),
                .wren_a (write_en),
                .q_b (dout),
                .aclr0 (1'b0),
                .aclr1 (1'b0),
                .addressstall_a (1'b0),
                .addressstall_b (1'b0),
                .byteena_a (1'b1),
                .byteena_b (1'b1),
                .clock1 (1'b1),
                .clocken0 (1'b1),
                .clocken1 (1'b1),
                .clocken2 (1'b1),
                .clocken3 (1'b1),
                .data_b ({DATA_WIDTH{1'b1}}),
                .eccencbypass (1'b0),
                .eccencparity (8'b0),
                .eccstatus (),
                .q_a (),
                .rden_a (1'b1),
                .sclr (1'b0),
                .wren_b (1'b0));
   defparam
     altera_syncram_component.address_aclr_b  = "NONE",
     altera_syncram_component.address_reg_b  = "CLOCK0",
     altera_syncram_component.clock_enable_input_a  = "BYPASS",
     altera_syncram_component.clock_enable_input_b  = "BYPASS",
     altera_syncram_component.clock_enable_output_b  = "BYPASS",
     altera_syncram_component.intended_device_family  = "Arria 10",
     altera_syncram_component.lpm_type  = "altera_syncram",
     altera_syncram_component.numwords_a  = 2**ADDR_WIDTH,     
     altera_syncram_component.numwords_b  = 2**ADDR_WIDTH,
     altera_syncram_component.operation_mode  = "DUAL_PORT",
     altera_syncram_component.outdata_aclr_b  = "NONE",
     altera_syncram_component.outdata_sclr_b  = "NONE",
     altera_syncram_component.outdata_reg_b  = "UNREGISTERED",
     altera_syncram_component.power_up_uninitialized  = "FALSE",
     altera_syncram_component.rdcontrol_reg_b  = "CLOCK0",
     altera_syncram_component.read_during_write_mode_mixed_ports  = "DONT_CARE",
     altera_syncram_component.widthad_a  = ADDR_WIDTH,
     altera_syncram_component.widthad_b  = ADDR_WIDTH,
     altera_syncram_component.width_a  = DATA_WIDTH,
     altera_syncram_component.width_b  = DATA_WIDTH,
     altera_syncram_component.width_byteena_a  = 1;
   

   
endmodule
