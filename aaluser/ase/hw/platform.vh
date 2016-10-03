/* ****************************************************************************
 * Copyright(c) 2011-2016, Intel Corporation
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
 * Module Info:
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 * ASE platform generics
 * Control specific ASE behavior such as:
 * - Latency range
 * - Error/Warning colors 
 *
 */

`ifndef _PLATFORM_VH_
 `define _PLATFORM_VH_

   /*
    * SIMKILL_ON_UNDEFINED: A switch to kill simulation if on a valid
    * signal, 'X' or 'Z' is not allowed, gracious closedown on same
    */
 `define VLOG_UNDEF                   1'bx
 `define VLOG_HIIMP                   1'bz

   /*
    * Print in Color
    */
   // Error in RED color
 `define BEGIN_RED_FONTCOLOR   $display("\033[1;31m");
 `define END_RED_FONTCOLOR     $display("\033[1;m");

   // Info in GREEN color
 `define BEGIN_GREEN_FONTCOLOR $display("\033[32;1m");
 `define END_GREEN_FONTCOLOR   $display("\033[0m");

   // Warnings/ASEDBGDUMP in YELLOW color
 `define BEGIN_YELLOW_FONTCOLOR $display("\033[0;33m");
 `define END_YELLOW_FONTCOLOR   $display("\033[0m");


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
 `define BDX_FPGA
 `ifdef BDX_FPGA
  `define DEFEATURE_ATOMICS
 `endif


/*
 * Platform Specific parameters
 * ----------------------------- 
 * INITIAL_SYSTEM_RESET_DURATION = Duration of initial system reset before system is up and running
 * CLK_TIME                      = Clock cycle timescale
 * LP_INITDONE_READINESS_LATENCY = Amount of time LP takes to be ready after reset is released 
 */

 `define UMSG_DELAY_TIMER_LOG2         8

 `define SOFT_RESET_DURATION           16
 `define RESET_TIMEOUT_DURATION        1024

/* OME5 */
 `ifdef BDX_FPGA
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_64UI_TIME                         10000
  `define CLK_32UI_TIME                         5000
  `define CLK_16UI_TIME                         2500
  `define CLK_8UI_TIME                          1250
  `define LP_INITDONE_READINESS_LATENCY         5
  `define NUM_VL_LINKS                          1
  `define NUM_VH_LINKS                          2

/* QPI Ivytown */
 `elsif OME2
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5000ps
  `define CLK_16UI_TIME                         2500ps
  `define CLK_8UI_TIME                          1250ps
  `define LP_INITDONE_READINESS_LATENCY         5
  `define NUM_VL_LINKS                          1
  `define NUM_VH_LINKS                          0

/* Generic, non-realistic, functional only simulation */ 
 `elsif GENERIC
  `define INITIAL_SYSTEM_RESET_DURATION         20
  `define CLK_32UI_TIME                         5ns
  `define CLK_16UI_TIME                         2.5ns
  `define CLK_8UI_TIME                          1.25ns
  `define LP_INITDONE_READINESS_LATENCY         5
  `define NUM_VL_LINKS                          1
  `define NUM_VH_LINKS                          2

 `endif

/*
 * MMIO Specifications
 */ 
`define MMIO_RESPONSE_TIMEOUT        512
`define MMIO_RESPONSE_TIMEOUT_RADIX  $clog2(`MMIO_RESPONSE_TIMEOUT) + 1
`define MMIO_MAX_OUTSTANDING         64


/*
 * Latency model 
 * Coded as a Min,Max tuple
 * -------------------------------------------------------
 * RDLINE_LATRANGE : ReadLine turnaround time
 * WRLINE_LATRANGE : WriteLine turnaround time
 * UMSG_LATRANGE   : UMsg latency
 * INTR_LATRANGE   : Interrupt turnaround time
 * 
 * LAT_UNDEFINED   : Undefined latency
 * 
 */ 
`define MMIO_LATENCY                15
`define RDLINE_S_LATRANGE          8,16
`define RDLINE_I_LATRANGE          8,16
`define WRLINE_M_LATRANGE          4,7
`define WRLINE_I_LATRANGE          4,7
`define UMSG_START2HINT_LATRANGE   39,41   // 200 ns
`define UMSG_HINT2DATA_LATRANGE    41,45   // 220 ns
`define UMSG_START2DATA_LATRANGE   82,85   // 420 ns
`define INTR_LATRANGE              10,15

`define LAT_UNDEFINED              50

`endif
