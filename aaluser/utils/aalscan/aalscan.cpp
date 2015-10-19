// Copyright (c) 2014-2015, Intel Corporation
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
/// @file aalscan.cpp
/// @brief Scan for AAL loadable services.
/// @ingroup aalscan
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing
///     commercially-deployable applications.
///
/// AUTHORS: Tim Whisonant, Intel Corporation.@endverbatim
/**
@addtogroup aalscan
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

Scan for installed AALSDK Service Modules and report their versions.

1 Summary of Operation

aalscan searches the canonical installation paths for installed AALSDK Service Modules,
reporting the version information when found. aalscan uses the @ref ServiceModule "low-level AALSDK Service Module API".

2 Running the application

2.0 Explicit Scan

aalscan can search for Service Modules whose module root names are given on the command line.

@verbatim
$ aalscan libOSAL libAAS
3.4.0 libOSAL
3.4.0 libAAS@endverbatim

2.1 AALSDK Internals Scan

When no command parameters are given, aalscan searches for the default Service Modules known
by the current AALSDK installation.

@verbatim
$ aalscan
3.4.0 libAAS
3.4.0 libAASRegistrar
3.4.0 libAASUAIA
0.0.0 libASECCIAFU
0.0.0 libASESPLAFU
0.0.0 libCCIAFU
0.0.0 libHWCCIAFU
0.0.0 libHWSPLAFU
3.4.0 libOSAL
0.0.0 librrmbroker
0.0.0 libsampleafu1
0.0.0 libsampleafu2
0.0.0 libsamplebroker
0.0.0 libSPLAFU
0.0.0 libSWSimCCIAFU
0.0.0 libSWSimSPLAFU
0.0.0 libAALRUNTIME@endverbatim

@}
*/
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <stdio.h>

#include "aalsdk/aalclp/aalclp.h"
#include "aalsdk/osal/OSServiceModule.h"
USING_NAMESPACE(AAL)

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#    include <stdlib.h>
# else
#    error Required system header stdlib.h not found.
# endif // HAVE_STDLIB_H
#endif // STDC_HEADERS


struct Service
{
   Service(const char *rn) :
      m_RootName(rn)
   {}
   std::string     m_RootName;
   OSServiceModule m_Module;
};

struct aalscanCmdLine
{
   aalscanCmdLine() :
      m_flags(0)
   {}

   btUIntPtr m_flags;
#define AALSCAN_CMD_FLAG_HELP    0x00000001
#define AALSCAN_CMD_FLAG_VERSION 0x00000002
   
   typedef std::list<Service>        list_type;
   typedef list_type::iterator       iter;
   typedef list_type::const_iterator const_iter;

   list_type m_Services;
};

aalscanCmdLine gaalscanCmdLine;

void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );
int parsecmds(struct aalscanCmdLine * , int , char *[] );


const char *gCoreAALServiceRootNames[] = {
   "libAAS",
   "libAASRegistrar",
   "libAASUAIA",
   "libASECCIAFU",
   "libASESPLAFU",
   "libCCIAFU",
   "libHWCCIAFU",
   "libHWSPLAFU",
   "libOSAL",
   "librrmbroker",
//   "libsampleafu1",
//   "libsampleafu2",
   "libsamplebroker",
   "libSPLAFU",
   "libSWSimCCIAFU",
   "libSWSimSPLAFU",
   "libAALRUNTIME"
};

OSServiceModule gServiceModules[sizeof(gCoreAALServiceRootNames)/sizeof(gCoreAALServiceRootNames[0])];

void DumpServiceMod(FILE *fp, OSServiceModule *p)
{
   fprintf(fp, "\t  Root name: %s\n", p->root_name);
   fprintf(fp, "\t  Full name: %s\n", p->full_name);
   fprintf(fp, "\tEntry_Point: %s\n", p->entry_point_name);
}

int main(int argc, char *argv[])
{
   int              i;
   int              res;
   AALSvcEntryPoint fn;
   char             ver[AAL_SVC_MOD_VER_STR_MAX];

   if ( argc >= 2 ) {
      if ( parsecmds(&gaalscanCmdLine, argc, argv) ) {
         fprintf(stderr, "Error scanning command line.\n");
         return 1;
      }
   }

   if ( flag_is_set(gaalscanCmdLine.m_flags, AALSCAN_CMD_FLAG_HELP|AALSCAN_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   }

   if ( gaalscanCmdLine.m_Services.empty() ) {
      // No services named on the command line - add the AAL core services.
      for ( i = 0 ; i < sizeof(gCoreAALServiceRootNames) / sizeof(gCoreAALServiceRootNames[0]) ; ++i ) {
         gaalscanCmdLine.m_Services.push_back( Service(gCoreAALServiceRootNames[i]) );
      }
   }

   aalscanCmdLine::iter it = gaalscanCmdLine.m_Services.begin();
   while ( gaalscanCmdLine.m_Services.end() != it ) {

      OSServiceModuleInit(&(*it).m_Module, (*it).m_RootName.c_str());

      //DumpServiceMod(stdout, &(*it).m_Module);

      res = OSServiceModuleOpen(&(*it).m_Module);
      if ( 0 != res ) {
         printf("not_found %s\n", (*it).m_RootName.c_str());
         ++it;
         continue;
      }

      fn = (*it).m_Module.entry_point_fn;

      ver[0] = 0;
      fn(AAL_SVC_CMD_VER_STR, ver);

      printf("%s %s\n", ver, (*it).m_RootName.c_str());

      ++it;
   }

   it = gaalscanCmdLine.m_Services.begin();
   while ( gaalscanCmdLine.m_Services.end() != it ) {
      OSServiceModuleClose(&(*it).m_Module);
      ++it;
   }

   return 0;
}

int aalscan_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct aalscanCmdLine *cl = (struct aalscanCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->m_flags, AALSCAN_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->m_flags, AALSCAN_CMD_FLAG_VERSION);
   }

   return 0;
}
aalclp_option_only aalscan_nix_long_option_only = { aalscan_on_nix_long_option_only, };

int aalscan_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption)
{
   struct aalscanCmdLine *cl = (struct aalscanCmdLine *)user;

   cl->m_Services.push_back( Service(nonoption) );

   return 0;
}
aalclp_non_option aalscan_non_option = { aalscan_on_non_option, };


void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "aalscan : search for AALSDK loadable services and report their versions.\n");
   fprintf(fp, "\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "aalscan",
                             PACKAGE_VERSION,
                             "",
                             help_msg_callback,
                             &gaalscanCmdLine)

int parsecmds(struct aalscanCmdLine *cl, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_init() failed : %d %s\n", res, strerror(res));
      return res;
   }

   aalscan_nix_long_option_only.user = cl;
   aalclp_add_nix_long_option_only(&clp, &aalscan_nix_long_option_only);
   aalscan_non_option.user = cl;
   aalclp_add_non_option(&clp, &aalscan_non_option);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_add_gcs_compliance() failed : %d %s\n", res, strerror(res));
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_scan_argv() failed : %d %s\n", res, strerror(res));
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      fprintf(stderr, "aalclp_destroy() failed : %d %s\n", clean, strerror(clean));
   }

   return res;
}


