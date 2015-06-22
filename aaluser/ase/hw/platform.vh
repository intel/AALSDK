`ifndef _PLATFORM_VH_
 `define _PLATFORM_VH_


/*
 * ASE Channel randomization features
 */
`define ASE_RANDOMIZE_TRANSACTIONS 

parameter CCI_AFU_LOW_OFFSET  = 14'h1000 / 4;
parameter AFU_CSR_LO_BOUND   = 16'h1000;


/*
 * Select the platform to test
 * Options: GENERIC | QPI_JKT
 * 
 * ## WARNING ## Select only one of these
 * 
 * GENERIC = Generic platform (non-realistic functional test)
 * QPI_JKT = QPI Jaketown platform
 * 
 */ 
 `define GENERIC


/*
 * Relevant CSRs that control CCI or AFU behaviour
 */
parameter CCI_RESET_CTRL_OFFSET = 14'h280;
parameter CCI_RESET_CTRL_BITLOC = 24;


/*
 * Platform Specific parameters
 * ----------------------------- 
 * INITIAL_SYSTEM_RESET_DURATION = Duration of initial system reset before system is up and running
 * CLK_TIME                      = Clock cycle timescale
 * LP_INITDONE_READINESS_LATENCY = Amount of time LP takes to be ready after reset is released 
 */

 `define UMSG_HINT2DATA_DELAY          40
 `define UMSG_NOHINT_DATADELAY         50
 `define UMSG_DELAY_TIMER_LOG2         8
 `define UMSG_MAX_MSG_LOG2             5
 `define UMSG_MAX_MSG                  2**`UMSG_MAX_MSG_LOG2


/* QPI Ivytown */
 `ifdef QPI_IVT
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5000ps
  `define CLK_16UI_TIME                         2500ps
  `define CLK_8UI_TIME                          1250ps
  `define LP_INITDONE_READINESS_LATENCY         5

/* Generic, non-realistic, functional only simulation */ 
 `elsif GENERIC
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5ns
  `define CLK_16UI_TIME                         2.5ns
  `define CLK_8UI_TIME                          1.25ns
  `define LP_INITDONE_READINESS_LATENCY         5

 `endif


/*
 * TEST: Latency ranges
 * Coded as a Min,Max tuple
 * -------------------------------------------------------
 * CSR_WR_LATRANGE : CSR Write latency range
 * RDLINE_LATRANGE : ReadLine turnaround time
 * WRLINE_LATRANGE : WriteLine turnaround time
 * UMSG_LATRANGE   : UMsg latency
 * INTR_LATRANGE   : Interrupt turnaround time
 * 
 * LAT_UNDEFINED   : Undefined latency
 * 
 */ 
`define CSR_WRITE_LATRANGE         8 // 730 ns
`define RDLINE_LATRANGE 8,16
`define WRLINE_LATRANGE 4,7
`define WRTHRU_LATRANGE 4,7
`define UMSG_START2HINT_LATRANGE   39,41   // 200 ns
`define UMSG_HINT2DATA_LATRANGE    41,45   // 220 ns
`define UMSG_START2DATA_LATRANGE   82,85   // 420 ns
`define INTR_LATRANGE   10,15

`define LAT_UNDEFINED              50

`endif
