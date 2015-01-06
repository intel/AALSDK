`ifndef _PLATFORM_VH_
 `define _PLATFORM_VH_

/*
 * Simulation Timescale 
 */ 
 `timescale 10ns/10ns


/*
 * ASE Channel randomization features
 */
`define ASE_RANDOMIZE_TRANSACTIONS 

parameter CCI_AFU_LOW_OFFSET  = 14'h1000 / 4;


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
parameter CCI_RESET_CTRL_OFFSET = 12'h280 / 4;
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
 `define UMSG_DELAY_TIMER_LOG2         6
 `define UMSG_MAX_MSG_LOG2             5
 `define UMSG_MAX_MSG                  2**`UMSG_MAX_MSG_LOG2


/* QPI Ivytown */
 `ifdef QPI_IVT
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5ns
  `define CLK_16UI_TIME                         2.5ns
  `define LP_INITDONE_READINESS_LATENCY         5

/* Generic, non-realistic, functional only simulation */ 
 `elsif GENERIC
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5ns
  `define CLK_16UI_TIME                         2.5ns
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
`define CSR_WR_LATRANGE 5,10
`define RDLINE_LATRANGE 8,16
`define WRLINE_LATRANGE 4,7
`define WRTHRU_LATRANGE 4,7
`define UMSG_LATRANGE   6,12
`define INTR_LATRANGE   10,15

`define LAT_UNDEFINED   5

`endif
