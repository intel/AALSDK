// Copyright (c) 2013-2015, Intel Corporation
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
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
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
#include <aalsdk/service/ICCIAFU.h>

/* All fn's return non-zero on error, unless otherwise noted. */

BEGIN_C_DECLS

static int
nlb_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_VERSION);
   } else if ( 0 == strcmp("--tabular", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TABULAR);
   } else if ((0 == strcmp("--suppress-hdr", option)) || (0 == strcmp("--sh", option))) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_SUPPRESSHDR);
   } else if ( 0 == strcmp("--no-bw", option) ) {
      flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_BANDWIDTH);
   } else if ( 0 == strcmp("--wt", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WT);
   } else if ( 0 == strcmp("--wb", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WB);
   } else if ( 0 == strcmp("--pwr", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_PWR);
   } else if ( 0 == strcmp("--cont", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CONT);
   } else if ( 0 == strcmp("--csrs", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CSRS);
   } else if ((0 == strcmp("--warm-fpga-cache", option)) || (0 == strcmp("--wfc", option))) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_WARM_FPGA_CACHE);
   } else if ((0 == strcmp("--cool-fpga-cache", option)) || (0 == strcmp("--cfc", option))) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_FPGA_CACHE);
   } else if ((0 == strcmp("--cool-cpu-cache", option))  || (0 == strcmp("--ccc", option))) {
	  flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_CPU_CACHE);
   } else if ( 0 == strcmp("--rds", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_RDS);
   } else if ( 0 == strcmp("--rdi", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_RDI);
   } else if ( 0 == strcmp("--rdo", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_RDO);
   } else if ((0 == strcmp("--poll", option) ) || (0 == strcmp("--p", option))) {
	  flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_POLL);
   } else if ((0 == strcmp("--csr-write", option)) || (0 == strcmp("--cw", option))) {
	  flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CSR_WRITE);
   } else if ((0 == strcmp("--umsg-data", option) ) || (0 == strcmp("--ud", option))) {
	  flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_DATA);
   } else if ((0 == strcmp("--umsg-hint", option)) || (0 == strcmp("--uh", option))) {
	  flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_HINT);
   } else if ( 0 == strcmp("--0", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_FEATURE0);
   } else if ( 0 == strcmp("--1", option) ) {
      flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_FEATURE1);
   } else {
      printf("Invalid option: %s\n", option);
      flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   }

   return 0;
}

static int
nlb_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   struct NLBCmdLine *nlbcl  = (struct NLBCmdLine *)user;
   char              *endptr = NULL;

#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
   timespec_type tsval;
#endif // OS

   	   if ( (0 == strcmp("--target", option)) || (0 == strcmp("--t", option))) {
         if ( 0 == strcasecmp("fpga", value) ) {
        	 nlbcl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_FPGA);
         } else if ( 0 == strcasecmp("ase", value) ) {
        	 nlbcl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_ASE);
         } else if ( 0 == strcasecmp("swsim", value) ) {
        	 nlbcl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_SWSIM);
         } else {
            cout << "Invalid value for --target : " << value << endl;
            return 4;
         }
      }else if ( (0 == strcmp("--mode", option)) || (0 == strcmp("--m", option)) ) {
          if ( 0 == strcasecmp("lpbk1", value) ) {
         	 nlbcl->TestMode = std::string(NLB_TESTMODE_LPBK1);
          } else if ( 0 == strcasecmp("read", value) ) {
         	 nlbcl->TestMode = std::string(NLB_TESTMODE_READ);
          } else if ( 0 == strcasecmp("write", value) ) {
         	 nlbcl->TestMode = std::string(NLB_TESTMODE_WRITE);
          } else if ( 0 == strcasecmp("trput", value) ) {
			 nlbcl->TestMode = std::string(NLB_TESTMODE_TRPUT);
          } else if ( 0 == strcasecmp("sw", value) ) {
			 nlbcl->TestMode = std::string(NLB_TESTMODE_SW);
          } else {
             cout << "Invalid value for --mode : " << value << endl;
             return 4;
          }
       }else if ( 0 == strcmp("--log", option) ) {
         char *endptr = NULL;

         nlbcl->LogLevel = (AAL::btInt)strtol(value, &endptr, 0);
         if ( endptr != value + strlen(value) ) {
        	 nlbcl->LogLevel = 0;
         } else if ( nlbcl->LogLevel < 0) {
        	 nlbcl->LogLevel = 0;
         } else if ( nlbcl->LogLevel > 8) {
        	 nlbcl->LogLevel = 8;
         }
      }else if ( (0 == strcmp("--begin", option)) || (0 == strcmp("--b", option)) ) {

      nlbcl->begincls = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->begincls = nlbcl->defaults.begincls;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_BEGINCL);
         printf("Invalid value for --begin : %s. Defaulting to %llu.\n", value, nlbcl->begincls);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_BEGINCL);
      }

   } else if ( (0 == strcmp("--end", option)) || (0 == strcmp("--e", option)) ) {

      nlbcl->endcls = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->endcls = nlbcl->defaults.endcls;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_ENDCL);
         printf("Invalid value for --end : %s. Defaulting to %llu.\n", value, nlbcl->endcls);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_ENDCL);
      }

   } else if ( 0 == strcmp("--dsm-phys", option) ) {

      nlbcl->dsmphys = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->dsmphys = nlbcl->defaults.dsmphys;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_DSM_PHYS);
         printf("Invalid value for --dsm-phys : %s. Defaulting to %" PRIuPHYS_ADDR ".\n", value, nlbcl->dsmphys);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_DSM_PHYS);
      }

   } else if ( 0 == strcmp("--src-phys", option) ) {

      nlbcl->srcphys = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->srcphys = nlbcl->defaults.srcphys;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_SRC_PHYS);
         printf("Invalid value for --src-phys : %s. Defaulting to %" PRIuPHYS_ADDR ".\n", value, nlbcl->srcphys);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_SRC_PHYS);
      }

   } else if ( 0 == strcmp("--src-capcm", option) ) {

      nlbcl->srcphys = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->srcphys = nlbcl->defaults.srcphys;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_SRC_CAPCM);
         printf("Invalid value for --src-capcm : %s. Defaulting to %" PRIuPHYS_ADDR ".\n", value, nlbcl->srcphys);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_SRC_CAPCM);
      }

   } else if ( 0 == strcmp("--dest-phys", option) ) {

      nlbcl->dstphys = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->dstphys = nlbcl->defaults.dstphys;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_DST_PHYS);
         printf("Invalid value for --dest-phys : %s. Defaulting to %" PRIuPHYS_ADDR ".\n", value, nlbcl->dstphys);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_DST_PHYS);
      }

   } else if ( 0 == strcmp("--dest-capcm", option) ) {

      nlbcl->dstphys = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->dstphys = nlbcl->defaults.dstphys;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_DST_CAPCM);
         printf("Invalid value for --dest-capcm : %s. Defaulting to %" PRIuPHYS_ADDR ".\n", value, nlbcl->dstphys);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_DST_CAPCM);
      }

   } else if ( (0 == strcmp("--clock-freq", option)) ||  (0 == strcmp("--f", option))) {

      nlbcl->clkfreq = strtoul(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
         nlbcl->clkfreq = nlbcl->defaults.clkfreq;
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_CLKFREQ);
         printf("Invalid value for --clock-freq : %s. Defaulting to %llu.\n", value, nlbcl->clkfreq);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_CLKFREQ);
      }

   } else if ( (0 == strcmp("--timeout-nsec", option)) || (0 == strcmp("--tn", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_nsec = nlbcl->defaults.to_nsec;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TONSEC);
         printf("Invalid nsec value: %s. Defaulting to %ld\n", value, nlbcl->to_nsec);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TONSEC);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_nsec += tsval;
#endif // OS
      }

   } else if ( (0 == strcmp("--timeout-usec", option)) || (0 == strcmp("--tu", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_usec = nlbcl->defaults.to_usec;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TOUSEC);
         printf("Invalid usec value: %s. Defaulting to %ld\n", value, nlbcl->to_usec);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOUSEC);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_usec += tsval;
#endif // OS
      }

   } else if ( (0 == strcmp("--timeout-msec", option)) || (0 == strcmp("--tms", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_msec = nlbcl->defaults.to_msec;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMSEC);
         printf("Invalid msec value: %s. Defaulting to %ld\n", value, nlbcl->to_msec);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMSEC);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_msec += tsval;
#endif // OS
      }

   } else if ( (0 == strcmp("--timeout-sec", option)) || (0 == strcmp("--ts", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_sec = nlbcl->defaults.to_sec;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TOSEC);
         printf("Invalid seconds value: %s. Defaulting to %ld\n", value, nlbcl->to_sec);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOSEC);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_sec += tsval;
#endif // OS
      }

   } else if ( (0 == strcmp("--timeout-min", option)) || (0 == strcmp("--tmi", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_min = nlbcl->defaults.to_min;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMIN);
         printf("Invalid minutes value: %s. Defaulting to %ld\n", value, nlbcl->to_min);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOMIN);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_min += tsval;
#endif // OS
      }

   } else if ( (0 == strcmp("--timeout-hour", option)) || (0 == strcmp("--th", option)) ) {

      tsval = strtotimespec(value, &endptr, 0);
      if ( value + strlen(value) != endptr ) {
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_hour = nlbcl->defaults.to_hour;
#endif // OS
         flag_clrf(nlbcl->cmdflags, NLB_CMD_FLAG_TOHOUR);
         printf("Invalid hours value: %s. Defaulting to %ld\n", value, nlbcl->to_hour);
      } else {
         flag_setf(nlbcl->cmdflags, NLB_CMD_FLAG_TOHOUR);
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
         nlbcl->to_hour += tsval;
#endif // OS
      }

   } else {
      printf("Invalid option: %s\n", option);
      flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   }

   return 0;
}

static int
nlb_on_nix_short_option_only(AALCLP_USER_DEFINED user, const char *option) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s\n", option);
   return 0;
}

static int
nlb_on_nix_short_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s %s\n", option, value);
   return 0;
}

static int
nlb_on_dash_only(AALCLP_USER_DEFINED user) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

static int
nlb_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

static int
nlb_on_win_long_option_only(AALCLP_USER_DEFINED user, const char *option) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s\n", option);
   return 0;
}

static int
nlb_on_win_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s %s\n", option, value);
   return 0;
}

static int
nlb_on_win_short_option_only(AALCLP_USER_DEFINED user, const char *option) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s\n", option);
   return 0;
}

static int
nlb_on_win_short_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid option: %s %s\n", option, value);
   return 0;
}

static int
nlb_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

static int
nlb_on_invalid(AALCLP_USER_DEFINED user, const char *invalid, size_t len) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)user;
   flag_setf(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR);

   size_t i;

   for ( i = 0 ; i < len ; ++i ) {
      printf("Invalid: '%c'\n", invalid[i]);
   }

   return 0;
}

static aalclp_option_only nlb_nix_long_option_only  = { nlb_on_nix_long_option_only,  };
static aalclp_option      nlb_nix_long_option       = { nlb_on_nix_long_option,       };
static aalclp_option_only nlb_nix_short_option_only = { nlb_on_nix_short_option_only, };
static aalclp_option      nlb_nix_short_option      = { nlb_on_nix_short_option,      };
static aalclp_dash_only   nlb_dash_only             = { nlb_on_dash_only,             };
static aalclp_dash_only   nlb_dash_dash_only        = { nlb_on_dash_dash_only,        };
static aalclp_option_only nlb_win_long_option_only  = { nlb_on_win_long_option_only,  };
static aalclp_option      nlb_win_long_option       = { nlb_on_win_long_option,       };
static aalclp_option_only nlb_win_short_option_only = { nlb_on_win_short_option_only, };
static aalclp_option      nlb_win_short_option      = { nlb_on_win_short_option,      };
static aalclp_non_option  nlb_non_option            = { nlb_on_non_option,            };
static aalclp_invalid     nlb_invalid               = { nlb_on_invalid,               };

void NLBSetupCmdLineParser(aalclp *clp, struct NLBCmdLine *nlbcl) {
   nlb_nix_long_option_only.user  = nlbcl;
   nlb_nix_long_option.user       = nlbcl;
   nlb_nix_short_option_only.user = nlbcl;
   nlb_nix_short_option.user      = nlbcl;
   nlb_dash_only.user             = nlbcl;
   nlb_dash_dash_only.user        = nlbcl;
   nlb_win_long_option_only.user  = nlbcl;
   nlb_win_long_option.user       = nlbcl;
   nlb_win_short_option_only.user = nlbcl;
   nlb_win_short_option.user      = nlbcl;
   nlb_non_option.user            = nlbcl;
   nlb_invalid.user               = nlbcl;

   aalclp_add_nix_long_option_only(clp,  &nlb_nix_long_option_only);
   aalclp_add_nix_long_option(clp,       &nlb_nix_long_option);
   aalclp_add_nix_short_option_only(clp, &nlb_nix_short_option_only);
   aalclp_add_nix_short_option(clp,      &nlb_nix_short_option);
   aalclp_add_dash_only(clp,             &nlb_dash_only);
   aalclp_add_dash_dash_only(clp,        &nlb_dash_dash_only);
   aalclp_add_win_long_option_only(clp,  &nlb_win_long_option_only);
   aalclp_add_win_long_option(clp,       &nlb_win_long_option);
   aalclp_add_win_short_option_only(clp, &nlb_win_short_option_only);
   aalclp_add_win_short_option(clp,      &nlb_win_short_option);
   aalclp_add_non_option(clp,            &nlb_non_option);
   aalclp_add_invalid(clp,               &nlb_invalid);
}

void nlb_help_message_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs) {
   struct NLBCmdLine *nlbcl = (struct NLBCmdLine *)gcs->user;

	string test;
	cout << "Enter test name: [LPBK1] [READ] [WRITE] [TRPUT] [SW]" << endl;
	cin >> test;
   fprintf(fp, "Usage:\n");

   if ( 0 == strcmp(test.c_str(), "LPBK1") ) {
      fprintf(fp, "   --mode=lpbk1 <TARGET> [<BEGIN>] [<END>] [<WRITES>] [<CONT>] [<FREQ>] [<RDSEL>] [<OUTPUT>]");
   } else if ( 0 == strcmp(test.c_str(), "READ") ) {
      fprintf(fp, "   --mode=read <TARGET> [<BEGIN>] [<END>] [<PREFILL>] [<CPU-CACHE>] [<BANDWIDTH>] [<CONT> <TIMEOUT>] [<FREQ>] [<RDSEL>] [<OUTPUT>]");
   } else if ( 0 == strcmp(test.c_str(), "WRITE") ) {
      fprintf(fp, "   --mode=write <TARGET> [<BEGIN>] [<END>] [<PREFILL>] [<CPU-CACHE>] [<BANDWIDTH>] [<WRITES>] [<CONT> <TIMEOUT>] [<FREQ>] [<OUTPUT>]");
   } else if ( 0 == strcmp(test.c_str(), "TRPUT") ) {
      fprintf(fp, "   --mode=trput <TARGET> [<BEGIN>] [<END>] [<BANDWIDTH>] [<WRITES>] [<CONT> <TIMEOUT>] [<FREQ>] [<RDSEL>] [<OUTPUT>]");
   } else if ( 0 == strcmp(test.c_str(), "SW") ) {
      fprintf(fp, "   --mode=sw <TARGET> [<BEGIN>] [<END>] [<WRITES>] [<CONT>] [<FREQ>] [<RDSEL>] [<OUTPUT>] [<NOTICE>]");
   }

   fprintf(fp, "\n\n");

   fprintf(fp, "      <TARGET>    = --target=one of { fpga ase swsim } OR --t=one of { fpga ase swsim }\n");
   if ( 0 != strcmp(test.c_str(), "LPBK1") &&
		0 != strcmp(test.c_str(), "SW")) {
      fprintf(fp, "      <BANDWIDTH> = --no-bw,                      suppress bandwidth calculations,                ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.nobw);
      } else {
         fprintf(fp, "yes\n");
      }
   }

   fprintf(fp, "      <BEGIN>     = --begin=B         OR --b=B,   where %llu <= B <= %5llu,                          ",
              nlbcl->defaults.mincls, nlbcl->defaults.maxcls);
   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_BEGINCL) ) {
      fprintf(fp, "%llu\n", nlbcl->begincls);
   } else {
      fprintf(fp, "Default=%llu\n", nlbcl->defaults.begincls);
   }

   fprintf(fp, "      <END>       = --end=E           OR --e=E,   where %llu <= E <= %5llu,                          ",
              nlbcl->defaults.mincls, nlbcl->defaults.maxcls);
   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_ENDCL) ) {
      fprintf(fp, "%llu\n", nlbcl->endcls);
   } else {
      fprintf(fp, "Default=B\n"/*, nlbcl->defaults.endcls*/);
   }

   if ( 0 == strcmp(test.c_str(), "READ") ||
        0 == strcmp(test.c_str(), "WRITE") ) {
      fprintf(fp, "      <FPGA-CACHE>= --warm-fpga-cache OR --wfc,   attempt to prime the cache with hits,           ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_WARM_FPGA_CACHE) ) {
         fprintf(fp, "yes\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.warmfpgacache);
      }

      fprintf(fp, "                    --cool-fpga-cache OR --cfc,   attempt to prime the cache with misses,         ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_FPGA_CACHE) ) {
         fprintf(fp, "yes\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.coolfpgacache);
      }

      fprintf(fp, "      <CPU-CACHE> = --cool-cpu-cache  OR --ccc,   attempt to prime the cpu cache with misses,     ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_COOL_CPU_CACHE) ) {
	   fprintf(fp, "yes\n");
      } else {
	   fprintf(fp, "Default=%s\n", nlbcl->defaults.coolcpucache);
      }
   }

   if ( 0 == strcmp(test.c_str(), "LPBK1") ||
        0 == strcmp(test.c_str(), "WRITE") ||
        0 == strcmp(test.c_str(), "TRPUT") ||
        0 == strcmp(test.c_str(), "SW") ) {
      fprintf(fp, "      <WRITES>    = --wt,                         write-through cache behavior,                   ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_WT) ) {
         fprintf(fp, "on\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.wt);
      }

      fprintf(fp, "                    --wb,                         write-back cache behavior,                      ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_WB) ) {
         fprintf(fp, "on\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.wb);
      }
   }

   if ( 0 == strcmp(test.c_str(), "LPBK1") ) {
	  fprintf(fp, "      <CONT>                   (LPBK1 is a non-continuous mode-only test)                         off\n");
   } else if ( 0 == strcmp(test.c_str(), "TRPUT") ) {
      fprintf(fp, "      <CONT>                   (TRPUT is a continuous mode-only test)                             on\n");
   } else if ( 0 == strcmp(test.c_str(), "SW") ) {
	  fprintf(fp, "      <CONT>                   (SW is a non-continuous mode-only test)                            off\n");
   } else {
      fprintf(fp, "      <CONT>      = --cont,                       continuous mode,                                ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_CONT) ) {
         fprintf(fp, "on\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.cont);
      }
   }

   fprintf(fp, "      <FREQ>      = --clock-freq=T    OR --f=T,   Clock frequency in Hz,                          Default=200 MHz\n");

   if ( 0 == strcmp(test.c_str(), "READ") ||
	    0 == strcmp(test.c_str(), "WRITE") ||
	    0 == strcmp(test.c_str(), "TRPUT") ) {

	   fprintf(fp, "      <TIMEOUT>   = --timeout-nsec=T  OR --tn=T,  timeout for --cont mode (nanoseconds portion),  ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TONSEC) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_nsec);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_nsec);
	   }

	   fprintf(fp, "                    --timeout-usec=T  OR --tu=T,  timeout for --cont mode (microseconds portion), ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TOUSEC) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_usec);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_usec);
	   }

	   fprintf(fp, "                    --timeout-msec=T  OR --tms=T, timeout for --cont mode (milliseconds portion), ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TOMSEC) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_msec);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_msec);
	   }

	   fprintf(fp, "                    --timeout-sec=T   OR --ts=T,  timeout for --cont mode (seconds portion),      ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TOSEC) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_sec);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_sec);
	   }

	   fprintf(fp, "                    --timeout-min=T   OR --tmi=T, timeout for --cont mode (minutes portion),      ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TOMIN) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_min);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_min);
	   }

	   fprintf(fp, "                    --timeout-hour=T  OR --th=T,  timeout for --cont mode (hours portion),        ");
	   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_TOHOUR) ) {
		  fprintf(fp, "%ld\n", nlbcl->to_hour);
	   } else {
		  fprintf(fp, "Default=%ld\n", nlbcl->defaults.to_hour);
	   }
   }

   fprintf(fp, "      <OUTPUT>    = --suppress-hdr    OR --sh,    suppress column headers for text output,        ");
   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
	  fprintf(fp, "yes\n");
   } else {
	  fprintf(fp, "Default=%s\n", nlbcl->defaults.suppresshdr);
   }

   if ( 0 == strcmp(test.c_str(), "LPBK1") ||
        0 == strcmp(test.c_str(), "READ")  ||
        0 == strcmp(test.c_str(), "TRPUT") ||
        0 == strcmp(test.c_str(), "SW") ) {
      fprintf(fp, "      <RDSEL>     = --rds,                        readline-shared,                                ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_RDS) ) {
         fprintf(fp, "yes\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.rds);
      }

     fprintf(fp, "                  = --rdi,                        readline-invalidate,                            ");
	 if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_RDI) ) {
	   fprintf(fp, "yes\n");
	 } else {
	   fprintf(fp, "Default=%s\n", nlbcl->defaults.rdi);
	 }

	 fprintf(fp, "                  = --rdo,                        readline-ownership,                             ");
	 if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_RDO) ) {
	   fprintf(fp, "yes\n");
	 } else {
	   fprintf(fp, "Default=%s\n", nlbcl->defaults.rdo);
	 }
   }

   if ( 0 == strcmp(test.c_str(), "SW") ) {
      fprintf(fp, "      <NOTICE>    = --poll      OR --p,           Polling-method,                                 ");
      if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_POLL) ) {
         fprintf(fp, "yes\n");
      } else {
         fprintf(fp, "Default=%s\n", nlbcl->defaults.poll);
      }

      fprintf(fp, "                  = --csr-write OR --cw,          CSR Write,                                      ");
	  if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_CSR_WRITE) ) {
	     fprintf(fp, "yes\n");
	  } else {
	     fprintf(fp, "Default=%s\n", nlbcl->defaults.csr_write);
	  }

	  fprintf(fp, "                  = --umsg-data OR --ud,          UMsg with data,                                 ");
	  if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_DATA) ) {
		 fprintf(fp, "yes\n");
	  } else {
		 fprintf(fp, "Default=%s\n", nlbcl->defaults.umsg_data);
	  }

	  fprintf(fp, "                  = --umsg-hint OR --uh,          UMsg Hint without data,                         ");
	  if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_FLAG_UMSG_HINT) ) {
		 fprintf(fp, "yes\n");
	  } else {
		 fprintf(fp, "Default=%s\n", nlbcl->defaults.umsg_hint);
	  }
   }
   fprintf(fp, "\n");
}

void MyNLBShowHelp(FILE *fp, aalclp_gcs_compliance_data *gcs) {
   nlb_help_message_callback(fp, gcs);
}

END_C_DECLS

// false indicates error.
bool NLBVerifyCmdLine(NLBCmdLine &cmd, std::ostream &os) throw()
{
   //cfg.SetAsynchronous(false);

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

   // --src-phys and --src-capcm
   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_SRC_PHYS|NLB_CMD_FLAG_SRC_CAPCM) ) {
      os << "--src-phys and --src-capcm are mutually exclusive." << endl;
      return false;
   }

   // --dst-phys and --dst-capcm
   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_DST_PHYS|NLB_CMD_FLAG_DST_CAPCM) ) {
      os << "--dst-phys and --dst-capcm are mutually exclusive." << endl;
      return false;
   }

   // --rdi, --rds, --rdo

   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_RDI|NLB_CMD_FLAG_RDS) ||
	  flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_RDI|NLB_CMD_FLAG_RDO) ||
	  flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_RDS|NLB_CMD_FLAG_RDO) ) {
	  os << "--rdi --rds and --rdo are mutually exclusive." << endl;
	  return false;
   }

   // --rdi, --rds, --rdo

   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_CSR_WRITE) ||
   flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_UMSG_DATA) ||
   flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_POLL|NLB_CMD_FLAG_UMSG_HINT) ||
   flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE|NLB_CMD_FLAG_UMSG_DATA) ||
   flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE|NLB_CMD_FLAG_UMSG_HINT) ||
   flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA|NLB_CMD_FLAG_UMSG_HINT)
   ) {
   os << "--poll --csr-write --umsg-data and --umsg-hint are mutually exclusive." << endl;
   return false;
   }


   // --wt, --wb, --pwr

   if ( flags_are_set(cmd.cmdflags, NLB_CMD_FLAG_WB|NLB_CMD_FLAG_WT) ) {
      os << "--wb and --wt are mutually exclusive." << endl;
      return false;
   }

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WB|NLB_CMD_FLAG_PWR) ||
        flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_WT) ) {
      // force --wb on; force --wt off.

      flag_setf(cmd.cmdflags, NLB_CMD_FLAG_WB);
      flag_clrf(cmd.cmdflags, NLB_CMD_FLAG_WT);
   } else {
      // force --wb off; force --wt on.

      flag_clrf(cmd.cmdflags, NLB_CMD_FLAG_WB);
      flag_setf(cmd.cmdflags, NLB_CMD_FLAG_WT);
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
         cmd.to_nsec = cmd.defaults.to_nsec;
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
   (x).tv_nsec += (timespec_type)cmd.to_nsec;                 \
   NORM_TS(x);                                                \
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

      //TODO cfg.SetContinuous(true);
      //TODO cfg.SetContModeTimeout(Timer(&ts));

# undef CALC_TIME
# undef NORM_TS
#endif // OS
   } else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAGS_TO) ) {
      os << "--timeout-* are meaningful only when --cont is also given." << endl;
      return false;
   }

   // --clock-freq

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CLKFREQ) ) {
	   //TODO cfg.SetFPGAClkFreqHz(cmd.clkfreq);
   }

   //cmd.dispflags = NLBDisplayFlags(cmd);

   return true;
}

