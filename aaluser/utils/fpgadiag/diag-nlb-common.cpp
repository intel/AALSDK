// Copyright(c) 2015-2016, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
// @file diag-nlb-common.cpp
// @brief Functionality common to all NLB utils.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 06/09/2013     TSW      Initial version.
// 01/07/2015	  SC	   fpgadiag version.@endverbatim
//****************************************************************************
#include "diag-nlb-common.h"
#include <aalsdk/kernel/ccipdriver.h>
#include <aalsdk/service/IALIAFU.h>

/* All fn's return non-zero on error, unless otherwise noted. */

BEGIN_C_DECLS
#define GETOPT_STRING ":ht:m:b:e:u:LO:Q:X:Y:Z:p:i:HMCr:w:f:a:lN:B:D:F:d:T:SV"

struct option longopts[] = {
      {"help",                no_argument,       NULL, 'h'},
      {"target",              required_argument, NULL, 't'}, //one of { fpga ase }
      {"mode",                required_argument, NULL, 'm'}, //one of { lpbk1 read write trput sw }
      {"begin",               required_argument, NULL, 'b'},
      {"end",                 required_argument, NULL, 'e'},
      {"multi-cl",            required_argument, NULL, 'u'},
      {"cont",                no_argument,       NULL, 'L'}, //continuous mode (Loop)
      {"timeout-usec",        required_argument, NULL, 'O'}, //usec
      {"timeout-msec",        required_argument, NULL, 'Q'}, //millisec
      {"timeout-sec",         required_argument, NULL, 'X'}, //sec
      {"timeout-min",         required_argument, NULL, 'Y'}, //min
      {"timeout-hour",        required_argument, NULL, 'Z'}, //hour
      {"cache-policy",        required_argument, NULL, 'p'}, //one of { wrline-I wli wrline-M wlm wrpush-I wpi}
      {"cache-hint",          required_argument, NULL, 'i'}, //one of {rdline-I, rdline-S}
      {"warm-fpga-cache",     no_argument,       NULL, 'H'}, //attempt to prime the cache with hits
      {"cool-fpga-cache",     no_argument,       NULL, 'M'}, //attempt to prime the cache with misses
      {"cool-cpu-cache",      no_argument,       NULL, 'C'}, //attempt to prime the cpu cache with misses
      {"read-vc",             required_argument, NULL, 'r'}, //one of { auto, vl0, vh0, vh1, random }
      {"write-vc",            required_argument, NULL, 'w'}, //one of { auto, vl0, vh0, vh1, random }
      {"wrfence-vc",          required_argument, NULL, 'f'}, //one of { auto, vl0, vh0, vh1 }
      {"strided-access",      required_argument, NULL, 'a'}, //strided-access
      {"alt-wr-pattern",      no_argument,       NULL, 'l'}, //alternate write pattern
      {"notice",              required_argument, NULL, 'N'}, //one of { poll p csr-write cw umsg-data ud umsg-hint uh}
      {"bus-number",          required_argument, NULL, 'B'},
      {"device-number",       required_argument, NULL, 'D'},
      {"function-number",     required_argument, NULL, 'F'},
      {"sub-device",          required_argument, NULL, 'd'},
      {"clock-freq",          required_argument, NULL, 'T'}, //Timing
      {"suppress-hdr",        no_argument,       NULL, 'S'},
      {"csv",                 no_argument,       NULL, 'V'},
      {0, 0, 0, 0}
};

int ParseCmds(struct NLBCmdLine *nlbcl, int argc, char *argv[])
{
   int res = 0;
   int getopt_ret;
   int option_index;
   char *endptr = NULL;
   timespec_type tsval;

   while( -1 != ( getopt_ret = getopt_long(argc, argv, GETOPT_STRING, longopts, &option_index))){
      const char *tmp_optarg = optarg;

      if((optarg) &&
         ('=' == *tmp_optarg)){
         ++tmp_optarg;
      }

      if((!optarg) &&
         (NULL != argv[optind]) &&
         ('-' != argv[optind][0]) ) {
         tmp_optarg = argv[optind++];
      }

      switch(getopt_ret){
         case 'h':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_HELP);
            NLBShowHelp(nlbcl);
            break;

         case 't':
            if ( 0 == strcasecmp("fpga", tmp_optarg) ) {
               nlbcl->AFUTarget = std::string(ALIAFU_NVS_VAL_TARGET_FPGA);
            } else if ( 0 == strcasecmp("ase", tmp_optarg) ) {
               nlbcl->AFUTarget = std::string(ALIAFU_NVS_VAL_TARGET_ASE);
            } else {
               cout << "Invalid value for --target : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'm':
            if ( 0 == strcasecmp("lpbk1", tmp_optarg) ) {
               nlbcl->TestMode = std::string(NLB_TESTMODE_LPBK1);
            } else if ( 0 == strcasecmp("read", tmp_optarg) ) {
               nlbcl->TestMode = std::string(NLB_TESTMODE_READ);
            } else if ( 0 == strcasecmp("write", tmp_optarg) ) {
               nlbcl->TestMode = std::string(NLB_TESTMODE_WRITE);
            } else if ( 0 == strcasecmp("trput", tmp_optarg) ) {
               nlbcl->TestMode = std::string(NLB_TESTMODE_TRPUT);
            } else if ( 0 == strcasecmp("sw", tmp_optarg) ) {
               nlbcl->TestMode = std::string(NLB_TESTMODE_SW);
            } else {
               cout << "Invalid value for --mode : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'b':
            endptr = NULL;
            nlbcl->begincls = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_BEGINCL);
            break;

         case 'e':
            endptr = NULL;
            nlbcl->endcls = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_ENDCL);
            break;

         case 'u':
            endptr = NULL;
            nlbcl->multicls = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_MULTICL);
            break;

         case 'L':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CONT);
            break;

         case 'O':
            endptr = NULL;
            tsval = strtotimespec(tmp_optarg, &endptr, 0);
            nlbcl->to_usec += tsval;
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOUSEC);
            break;

         case 'Q':
            endptr = NULL;
            tsval = strtotimespec(tmp_optarg, &endptr, 0);
            nlbcl->to_msec += tsval;
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMSEC);
            break;

         case 'X':
            endptr = NULL;
            tsval = strtotimespec(tmp_optarg, &endptr, 0);
            nlbcl->to_sec += tsval;
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOSEC);
            break;

         case 'Y':
            endptr = NULL;
            tsval = strtotimespec(tmp_optarg, &endptr, 0);
            nlbcl->to_min += tsval;
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMIN);
            break;

         case 'Z':
            endptr = NULL;
            tsval = strtotimespec(tmp_optarg, &endptr, 0);
            nlbcl->to_hour += tsval;
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOHOUR);
            break;

         case 'p':
            if ( 0 == strcasecmp("wrline-I", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRLINE_I);
            } else if ( 0 == strcasecmp("wrline-M", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRLINE_M);
            } else if ( 0 == strcasecmp("wrpush-I", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRPUSH_I);
            }else {
               cout << "Invalid value for --cache-policy : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'i':
            if ( 0 == strcasecmp("rdline-I", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_RDI);
            } else if ( 0 == strcasecmp("rdline-S", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_RDS);
            }else {
               cout << "Invalid value for --cache-hint : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'H':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WARM_FPGA_CACHE);
            break;

         case 'M':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_FPGA_CACHE);
            break;

         case 'C':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_CPU_CACHE);
            break;

         case 'r':
            if ( 0 == strcasecmp("auto", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_READ_VA);
            } else if ( 0 == strcasecmp("vl0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_READ_VL0);
            } else if ( 0 == strcasecmp("vh0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_READ_VH0);
            } else if ( 0 == strcasecmp("vh1", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_READ_VH1);
            } else if ( 0 == strcasecmp("random", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_READ_VR);
            }else {
               cout << "Invalid value for --read : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'w':
            if ( 0 == strcasecmp("auto", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRITE_VA);
            } else if ( 0 == strcasecmp("vl0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRITE_VL0);
            } else if ( 0 == strcasecmp("vh0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRITE_VH0);
            } else if ( 0 == strcasecmp("vh1", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRITE_VH1);
            } else if ( 0 == strcasecmp("random", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRITE_VR);
            }else {
               cout << "Invalid value for --write : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'f':
            if ( 0 == strcasecmp("auto", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRFENCE_VA);
            } else if ( 0 == strcasecmp("vl0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRFENCE_VL0);
            } else if ( 0 == strcasecmp("vh0", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRFENCE_VH0);
            } else if ( 0 == strcasecmp("vh1", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WRFENCE_VH1);
            }else {
               cout << "Invalid value for --wrfence : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'a':
            endptr = NULL;
            nlbcl->strided_acs = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_STRIDED_ACS);
            break;

         case 'l':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_ALT_WR_PRN);
            break;

         case 'N':
            if ( 0 == strcasecmp("poll", tmp_optarg)) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_POLL);
            } else if ( 0 == strcasecmp("csr-write", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CSR_WRITE);
            } else if ( 0 == strcasecmp("umsg-data", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_DATA);
            } else if ( 0 == strcasecmp("umsg-hint", tmp_optarg) ) {
               flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_HINT);
            }else {
               cout << "Invalid value for --notice : " << tmp_optarg << endl;
               return CMD_PARSE_ERR;
            }
            break;

         case 'B':
            endptr = NULL;
            nlbcl->busnum = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_BUS_NUMBER);
            break;

         case 'D':
            endptr = NULL;
            nlbcl->devnum = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_DEVICE_NUMBER);
            break;

         case 'F':
            endptr = NULL;
            nlbcl->funnum = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_FUNCTION_NUMBER);
            break;

         case 'd':
            endptr = NULL;
            //TODO change "DevTarget" to "subDev"
            //TODO set subDev flag (reuse --no-bw)
            nlbcl->DevTarget = strtoul(tmp_optarg, &endptr, 0);
            break;

         case 'T':
            endptr = NULL;
            nlbcl->clkfreq = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CLKFREQ);
            break;

         case 'S':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_SUPPRESSHDR);
            break;

         case 'V':
            flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CSV);
            break;

         case ':':   /* missing option argument */
            cout << "Missing option argument.\n";
            return CMD_PARSE_ERR;

         case '?':
         default:    /* invalid option */
            cout << "Invalid cmdline options.\n";
            return CMD_PARSE_ERR;
      }
   }
   return res;
}

void NLBShowHelp( struct NLBCmdLine *nlbcl ) {
   string test;

//   if(0 == strcasecmp(nlbcl->TestMode.c_str(),NLB_TESTMODE_LPBK1)){
//      test="lpbk1";
//   }else if(0 == strcasecmp(nlbcl->TestMode.c_str(),NLB_TESTMODE_READ)){
//      test="read";
//   }else if(0 == strcasecmp(nlbcl->TestMode.c_str(),NLB_TESTMODE_WRITE)){
//      test="write";
//   }else if(0 == strcasecmp(nlbcl->TestMode.c_str(),NLB_TESTMODE_TRPUT)){
//      test="trput";
//   }else if(0 == strcasecmp(nlbcl->TestMode.c_str(),NLB_TESTMODE_SW)){
//      test="sw";
//   }else {
      cout << "Enter test name: [LPBK1] [READ] [WRITE] [TRPUT] [SW]" << endl;
      cin >> test;
//   }
   cout << "Usage:\n";

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ) {
      cout << "   --mode=lpbk1 [<TARGET>] [<BEGIN>] [<END>] [<MULTI-CL>] [<CONT> <TIMEOUT>] [CACHE-POLICY] [CACHE-HINT] [<READ-VC>] [<WRITE-VC>] [<WRFENCE-VC>] [<BUS>] [<DEVICE>] [<FUNCTION>] [SUB-DEVICE] [<FREQ>] [<OUTPUT>]";
   } else if ( 0 == strcasecmp(test.c_str(), "READ") ) {
      cout <<  "   --mode=read [<TARGET>] [<BEGIN>] [<END>] [<MULTI-CL>] [<STRIDES>] [<CONT> <TIMEOUT>] [CACHE-HINT] [<FPGA-CACHE>] [<CPU-CACHE>] [<READ-VC>] [<BUS>] [<DEVICE>] [<FUNCTION>] [SUB-DEVICE] [<FREQ>] [<OUTPUT>]";
   } else if ( 0 == strcasecmp(test.c_str(), "WRITE") ) {
      cout <<  "   --mode=write [<TARGET>] [<BEGIN>] [<END>] [<MULTI-CL>] [<STRIDES>] [<CONT> <TIMEOUT>] [CACHE-POLICY] [CACHE-HINT] [<FPGA-CACHE>] [<CPU-CACHE>] [<WRITE-VC>] [<WRFENCE-VC>] [<WR-PATTERN>] [<BUS>] [<DEVICE>] [<FUNCTION>] [SUB-DEVICE] [<FREQ>] [<OUTPUT>]";
   } else if ( 0 == strcasecmp(test.c_str(), "TRPUT") ) {
      cout <<  "   --mode=trput [<TARGET>] [<BEGIN>] [<END>] [<MULTI-CL>] [<STRIDES>] [<CONT> <TIMEOUT>] [CACHE-POLICY] [CACHE-HINT] [<READ-VC>] [<WRITE-VC>] [<WRFENCE-VC>] [<BUS>] [<DEVICE>] [<FUNCTION>] [SUB-DEVICE] [<FREQ>] [<OUTPUT>]";
   } else if ( 0 == strcasecmp(test.c_str(), "SW") ) {
      cout <<  "   --mode=sw [<TARGET>] [<BEGIN>] [<END>] [<CONT>] [CACHE-POLICY] [CACHE-HINT] [<READ-VC>] [<WRITE-VC>] [<WRFENCE-VC>] [<NOTICE>] [<BUS>] [<DEVICE>] [<FUNCTION>] [SUB-DEVICE] [<FREQ>] [<OUTPUT>]";
   }else {
	   cout << "Invalid test mode." << endl;
	   return;
   }

   cout << endl << endl;

   cout << "      <TARGET>        = --target=one of { fpga ase }  OR  -t=one of { fpga ase },                                  ";
   cout << "Default=fpga\n";

   cout << "      <BEGIN>         = --begin=B              OR  -b=B,    ";
   cout << "Where "<< nlbcl->defaults.mincls <<" <= B <= " << nlbcl->defaults.maxcls << ",                                 ";
   cout << "Default=" << nlbcl->defaults.begincls << endl;

   cout << "      <END>           = --end=E                OR  -e=E,    ";
   cout << "Where " << nlbcl->defaults.mincls <<" <= E <= " << nlbcl->defaults.maxcls <<",                                 ";
   cout << "Default=B\n";

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "READ")  ||
        0 == strcasecmp(test.c_str(), "WRITE") ||
        0 == strcasecmp(test.c_str(), "TRPUT")) {

      cout << "      <MULTI-CL>      = --multi-cl=M           OR  -u=M,    Where M =one of { 1 2 4 },                             ";
      cout << "Default=" << nlbcl->defaults.multicls << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "READ")  ||
        0 == strcasecmp(test.c_str(), "WRITE") ||
        0 == strcasecmp(test.c_str(), "TRPUT")) {

      cout << "      <STRIDES>       = --strided-access=S     OR  -a=S,    ";
      cout << nlbcl->defaults.min_strides << "<= S <= " << nlbcl->defaults.max_strides <<",                                           ";
      cout << "Default=" << nlbcl->defaults.min_strides << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "SW") ) {
      cout << "      <CONT>                   (SW is a non-continuous mode-only test)                                             off\n";

   } else {
      cout << "      <CONT>          = --cont                 OR  -L,      Continuous mode,                                       ";
      cout << "Default=" << nlbcl->defaults.cont << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "READ")  ||
        0 == strcasecmp(test.c_str(), "WRITE") ||
        0 == strcasecmp(test.c_str(), "TRPUT")) {

      cout << "      <TIMEOUT>       = --timeout-usec=T,                   Timeout for --cont mode (microseconds portion),        ";
      cout << "Default=" << nlbcl->defaults.to_usec << endl;

      cout << "                        --timeout-msec=T,                   Timeout for --cont mode (milliseconds portion),        ";
      cout << "Default=" << nlbcl->defaults.to_msec << endl;

      cout << "                        --timeout-sec=T,                    Timeout for --cont mode (seconds portion),             ";
      cout << "Default=" << nlbcl->defaults.to_sec << endl;

      cout << "                        --timeout-min=T,                    Timeout for --cont mode (minutes portion),             ";
      cout << "Default=" << nlbcl->defaults.to_min << endl;

      cout << "                        --timeout-hour=T,                   Timeout for --cont mode (hours portion),               ";
      cout << "Default=" <<  nlbcl->defaults.to_hour << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "WRITE") ||
        0 == strcasecmp(test.c_str(), "TRPUT") ||
        0 == strcasecmp(test.c_str(), "SW") ) {

      cout << "      <CACHE-POLICY>  = --cache-policy=P       OR  -p=P,    Where P =one of { wrline-I wrline-M wrpush-I }         ";
      cout << "Default=" << nlbcl->defaults.cachepolicy << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "READ") ||
        0 == strcasecmp(test.c_str(), "TRPUT") ||
        0 == strcasecmp(test.c_str(), "SW") ) {

      cout << "      <CACHE-HINT>    = --cache-hint=I         OR  -i=I,    Where I =one of { rdline-I rdline-S }                  ";
      cout << "Default=" << nlbcl->defaults.cachehint << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "READ") ||
        0 == strcasecmp(test.c_str(), "WRITE")) {

      cout << "      <FPGA-CACHE>    = --warm-fpga-cache      OR  -H,      Attempt to prime the cache with hits,                  ";
      cout << "Default=" << nlbcl->defaults.warmfpgacache << endl;

      cout << "                        --cool-fpga-cache      OR  -M,      Attempt to prime the cache with misses,                ";
      cout << "Default=" << nlbcl->defaults.coolfpgacache << endl;

      cout << "      <CPU-CACHE>     = --cool-cpu-cache       OR  -C,      Attempt to prime the cpu cache with misses,            ";
      cout << "Default=" << nlbcl->defaults.coolcpucache << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "READ")  ||
        0 == strcasecmp(test.c_str(), "TRPUT") ||
        0 == strcasecmp(test.c_str(), "SW"))   {

      cout << "      <READ-VC>       = --read-vc=R,           OR  -r=R,    Where R =one of { auto vl0 vh0 vh1 random }            ";
      cout << "Default=" << nlbcl->defaults.readvc << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "LPBK1") ||
        0 == strcasecmp(test.c_str(), "WRITE") ||
        0 == strcasecmp(test.c_str(), "TRPUT") ||
        0 == strcasecmp(test.c_str(), "SW"))   {

      cout << "      <WRITE-VC>      = --write-vc=W,          OR  -w=W,    Where R =one of { auto vl0 vh0 vh1 random }            ";
      cout << "Default=" << nlbcl->defaults.writevc << endl;

      cout << "      <WRFENCE-VC>    = --wrfence-vc=F,        OR  -f=F,    Where R =one of { auto vl0 vh0 vh1 }                   ";
      cout << "Default=" << nlbcl->defaults.wrfencevc << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "WRITE")){

      cout << "      <WR-PATTERN>    = --alt-wr-pattern       OR  -l,      Alternate Write Pattern,                               ";
      cout << "Default=" << nlbcl->defaults.awp << endl;
   }

   if ( 0 == strcasecmp(test.c_str(), "SW")) {

      cout << "      <NOTICE>        = --notice=O             OR  -N=O,    Where O =one of { poll csr-write umsg-data umsg-hint } ";
      cout << "Default=" << nlbcl->defaults.notice << endl;
   }

   cout << "      <BUS>           = --bus-number=0xN       OR  -B=0xN,  Bus number of the PCIe device,                         ";
   cout << "Default=" << nlbcl->defaults.busnum << endl;

   cout << "      <DEVICE>        = --device-number=0xN    OR  -D=0xN,  Device number of the PCIe device,                      ";
   cout << "Default=" << nlbcl->defaults.devnum << endl;

   cout << "      <FUNCTION>      = --function-number=0xN  OR  -F=0xN,  Function number of the PCIe device,                    ";
   cout << "Default=" << nlbcl->defaults.funnum << endl;

   cout << "      <SUB-DEVICE>    = --sub-device=D         OR  -d=D,    Where D is the Sub-device Number,                      ";
   cout << "Default is not set\n";

   cout << "      <FREQ>          = --clock-freq=T         OR  -f=T,    Clock frequency in Hz,                                 Default=400 MHz\n";

   cout << "      <OUTPUT>        = --suppress-hdr         OR  -S,      Suppress column headers for text output,               ";
   cout << "Default=" << nlbcl->defaults.suppresshdr << endl;

   cout << "                      = --csv                  OR  -V,      Comma separated value format,                          ";
   cout << "Default=" << nlbcl->defaults.csv << endl;

   cout << endl;
}

END_C_DECLS

// false indicates error.
bool NLBVerifyCmdLine(NLBCmdLine &cmd, std::ostream &os) throw()
{
   // --begin=X
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BEGINCL) ) {
      if ( cmd.begincls < cmd.defaults.mincls ) {
         os << cmd.TestMode << " requires at least " << cmd.defaults.mincls << " cache line";
         if ( cmd.defaults.mincls > 1 ) {
            os << 's';
         }
         os << " for --begin." << endl;
         return false;
      }

      if ( 0 == strcmp(cmd.TestMode.c_str(), "TestMode_sw") ) {
    	 if ( cmd.begincls > cmd.defaults.maxcls-1) {
			  os << cmd.TestMode << " allows at most " << cmd.defaults.maxcls-1 << " cache line";
			  if ( cmd.defaults.maxcls > 1 ) {
				 os << 's';
			  }
			  os << " for --begin." << endl;
			  return false;
			}
      }

      if ( cmd.begincls > cmd.defaults.maxcls ) {
         os << cmd.TestMode << " allows at most " << cmd.defaults.maxcls << " cache line";
         if ( cmd.defaults.maxcls > 1 ) {
            os << 's';
         }
         os << " for --begin." << endl;
         return false;
      }

      if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_ENDCL) ) {
         cmd.endcls = cmd.begincls;
      }
   }

   // --end=X
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_ENDCL) ) {
      if ( cmd.endcls < cmd.defaults.mincls ) {
         os << cmd.TestMode << " requires at least " << cmd.defaults.mincls << " cache line";
         if ( cmd.defaults.mincls > 1 ) {
            os << 's';
         }
         os << " for --end." << endl;
         return false;
      }

      if ( 0 == strcmp(cmd.TestMode.c_str(), "TestMode_sw") ) {
    	 if ( cmd.endcls > cmd.defaults.maxcls-1 ) {
    		os << cmd.TestMode << " allows at most " << cmd.defaults.maxcls-1 << " cache line";
			if ( cmd.defaults.maxcls > 1 ) {
			   os << 's';
		    }
			os << " for --end." << endl;
			return false;
		 }
      }
      if ( cmd.endcls > cmd.defaults.maxcls ) {
         os << cmd.TestMode << " allows at most " << cmd.defaults.maxcls << " cache line";
         if ( cmd.defaults.maxcls > 1 ) {
            os << 's';
         }
         os << " for --end." << endl;
         return false;
      }

      if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BEGINCL) &&
         ( cmd.endcls < cmd.begincls ) ) {
         std::swap(cmd.begincls, cmd.endcls);
         os << "--begin value was less than --end value, so I swapped them." << endl;
      }
   }

   // --multi-cl=X
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_MULTICL) ) {
      if ( 0 != (cmd.begincls % cmd.multicls) ) {
         os << cmd.TestMode << " requires --begin to be a multiple of --multi-cl. " << endl;
         return false;
      }

      if ( 0 != (cmd.endcls % cmd.multicls) ) {
         os << cmd.TestMode << " requires --end to be a multiple of --multi-cl. " << endl;
         return false;
      }

      if ( (1 != (cmd.multicls)) &&
           (2 != (cmd.multicls)) &&
           (4 != (cmd.multicls))) {
         os << cmd.TestMode << " requires --multi-cl to be one of 1, 2 or 4. " << endl;
         return false;
      }
   }

   // --strided-access=X
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_STRIDED_ACS) ) {
      if ( cmd.strided_acs < cmd.defaults.min_strides ) {
         os << cmd.TestMode << " requires at least " << cmd.defaults.min_strides << " stride";
         os << " for --strided-access." << endl;
         return false;
      }

      if ( cmd.strided_acs > cmd.defaults.max_strides ) {
         os << cmd.TestMode << " allows at most " << cmd.defaults.max_strides << " strides";
         os << " for --strided-access." << endl;
         return false;
      }

      if ((cmd.strided_acs * cmd.endcls) >=  cmd.defaults.maxcls) {
         os << cmd.TestMode << " requires last address access to be less than the buffer size. " << endl;
         return false;
      }
   }

   // --rdi, --rds

   /********************** FOR SKX POWER-ON ONLY **************************/

   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_RDS)) {
	   os << "--rds is NOT supported in this release." << endl;
	   return false;
   }

   if ( flags_are_clr(cmd.cmdflags, NLB_CMD_FLAG_RDI)) {
   	   os << "WARNING: No option selected for <CACHE-HINT>. Defaulting to --rdline-I." << endl;
   	   flag_setf(cmd.cmdflags, NLB_CMD_FLAG_RDI);
   }
   /**********************************************************************/

   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_RDI|NLB_CMD_FLAG_RDS)) {
	   	os << "--rdi and --rds are mutually exclusive." << endl;
	   	return false;
   }

   // --poll, --csr-write, --umsg-data, --umsg-hint
   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_CSR_WRITE)      ||
        flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_UMSG_DATA)      ||
        flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_UMSG_HINT)      ||
        flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE|NLB_CMD_FLAG_UMSG_DATA) ||
        flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE|NLB_CMD_FLAG_UMSG_HINT) ||
        flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA|NLB_CMD_FLAG_UMSG_HINT)) {
	    os << "--poll --csr-write --umsg-data and --umsg-hint are mutually exclusive." << endl;
	    return false;
   }

   // --va, --vl0, --vh0, --vh1, --vr

    if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VA|NLB_CMD_FLAG_READ_VL0) ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VA|NLB_CMD_FLAG_READ_VH0) ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VA|NLB_CMD_FLAG_READ_VH1) ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VL0|NLB_CMD_FLAG_READ_VH0)||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VL0|NLB_CMD_FLAG_READ_VH1)||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH0|NLB_CMD_FLAG_READ_VH1)||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VA|NLB_CMD_FLAG_READ_VR)  ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VL0|NLB_CMD_FLAG_READ_VR) ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH0|NLB_CMD_FLAG_READ_VR) ||
         flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH1|NLB_CMD_FLAG_READ_VR) ) {
    	 os << "--read-va, --read-vl0, --read-vh0, --read-vh1 and --read-vr are mutually exclusive." << endl;
    	 return false;
    }

     if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VA|NLB_CMD_FLAG_WRITE_VL0)  ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VA|NLB_CMD_FLAG_WRITE_VH0)  ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VA|NLB_CMD_FLAG_WRITE_VH1)  ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VL0|NLB_CMD_FLAG_WRITE_VH0) ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VL0|NLB_CMD_FLAG_WRITE_VH1) ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH0|NLB_CMD_FLAG_WRITE_VH1) ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VA|NLB_CMD_FLAG_WRITE_VR)   ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VL0|NLB_CMD_FLAG_WRITE_VR)  ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH0|NLB_CMD_FLAG_WRITE_VR)  ||
          flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH1|NLB_CMD_FLAG_WRITE_VR) ) {
        os << "--write-va, --write-vl0, --write-vh0, --write-vh1 and --write-vr are mutually exclusive." << endl;
        return false;
     }

   // --wrline-I, --wrline-M and --wrpush-I
   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRLINE_M|NLB_CMD_FLAG_WRLINE_I) ||
	     flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRLINE_M|NLB_CMD_FLAG_WRPUSH_I) ||
	     flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WRPUSH_I|NLB_CMD_FLAG_WRLINE_I) ) {
      os << "--wrline-I, --wrline-M and --wrpush-I are mutually exclusive." << endl;
      return false;
   }

   // --warm-fpga-cache and --cool-fpga-cache
   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WARM_FPGA_CACHE|NLB_CMD_FLAG_COOL_FPGA_CACHE) ) {
      os << "--warm-fpga-cache and --cool-fpga-cache are mutually exclusive." << endl;
      return false;
   }
   // --cont and timeout

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT) ) {
#if   defined( __AAL_WINDOWS__ )

#elif defined( __AAL_LINUX__ )
      if ( flags_are_clr(cmd.cmdflags, NLB_CMD_FLAGS_TO) ) {
         // no timeout given - use defaults
         cmd.to_usec = cmd.defaults.to_usec;
         cmd.to_msec = cmd.defaults.to_msec;
         cmd.to_sec  = cmd.defaults.to_sec;
         cmd.to_min  = cmd.defaults.to_min;
         cmd.to_hour = cmd.defaults.to_hour;
      }
# define NORM_TS(x)                                   \
do                                                    \
{                                                     \
   (x).tv_sec  += (x).tv_nsec / (1000 * 1000 * 1000); \
   (x).tv_nsec %= 1000 * 1000 * 1000;                 \
}while(0)

# define CALC_TIME(x)                                         \
do                                                            \
{                                                             \
   (x).tv_nsec += (timespec_type)(cmd.to_usec * 1000);        \
   NORM_TS(x);                                                \
   (x).tv_nsec += (timespec_type)(cmd.to_msec * 1000 * 1000); \
   NORM_TS(x);                                                \
   (x).tv_sec += (timespec_type)cmd.to_sec;                   \
   (x).tv_sec += (timespec_type)(cmd.to_min * 60);            \
   (x).tv_sec += (timespec_type)(cmd.to_hour * 60 * 60);      \
}while(0)

      struct timespec ts = { 0, 0 };
      CALC_TIME(ts);
      cmd.timeout = ts;

# undef CALC_TIME
# undef NORM_TS
#endif // OS
   } else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAGS_TO) ) {
      os << "--timeout-* is meaningful only when --cont is also given." << endl;
      return false;
   }
   return true;
}

