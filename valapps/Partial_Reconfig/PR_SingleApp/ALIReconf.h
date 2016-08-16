// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_SingleApp/ALIReconf.h
//
// Copyright(c) 2007-2016, Intel Corporation
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
/// @file ALIReconf.h
/// @brief Partial reconfiguration of green bit stream service .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL Partial reconfiguration test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test PR functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2016     RP       Initial version started based on older sample
//****************************************************************************

#ifndef __RECONF_SERVICE__
#define __RECONF_SERVICE__

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aalclp/aalclp.h>
#include <string.h>

#define BitStreamWrongFile  "/home/aravuri/kernelpr/pr_script/bitstream/100.rbf"

using namespace std;
using namespace AAL;

BEGIN_C_DECLS

int aliconigafu_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int aliconigafu_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );
int aliconigafu_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption);
int aliconigafu_on_dash_only(AALCLP_USER_DEFINED user);
int aliconigafu_on_dash_dash_only(AALCLP_USER_DEFINED user);

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );
int  ParseCmds(struct ALIConfigCommandLine *pconfigcmd, int argc, char *argv[]);
void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs);
void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs);
int  verifycmds(struct ALIConfigCommandLine *cl);


// Command Line Parameters
struct  ALIConfigCommandLine
{
   btUIntPtr          flags;
   #define ALICONIFG_CMD_FLAG_HELP      0x00000001
   #define ALICONIFG_CMD_FLAG_VERSION   0x00000002
   #define ALICONIFG_CMD_PARSE_ERROR    0x00000003
   char    bitstream_file1[200];
   char    bitstream_file2[200];
   int     testcasenum;
   int     reconftimeout;
   int     reconfAction;
   bool    reactivateDisabled;
   char    reconfInterface[20];
};

END_C_DECLS

/// @brief   Since this is a simple application, our App class implements IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class AllocatesReconfService: public CAASBase, public IServiceClient, IALIReconfigure_Client
{
public:

   AllocatesReconfService();
   ~AllocatesReconfService();

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase, TransactionID const &rTranID);
   void serviceAllocateFailed(const IEvent &rEvent);
   void serviceReleased(const AAL::TransactionID&);
   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &);
   void serviceReleaseFailed(const AAL::IEvent&);
   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   // <IALIReconfigure_Client interface>
   virtual void deactivateSucceeded( TransactionID const &rTranID );
   virtual void deactivateFailed( IEvent const &rEvent );

   virtual void configureSucceeded( TransactionID const &rTranID );
   virtual void configureFailed( IEvent const &rEvent );

   virtual void activateSucceeded( TransactionID const &rTranID );
   virtual void activateFailed( IEvent const &rEvent );
   // <end IALIReconfigure_Client interface>

   void PrintReconfExceptionDescription(IEvent const &theEvent);

   btBool FreePRService();
   btBool AllocatePRService(Runtime *pRuntime);
   void setreconfnvs( char* pbitstream,
                      btUnsigned64bitInt reconftimeout ,
                      btUnsigned64bitInt reconfAction ,
                      btUnsigned64bitInt reactivateDisabled);

   btID getErrnum() {  return m_errNum; }
   void setIsOk(btBool status) { m_bIsOK =status; }
   IALIReconfigure* getReconfService() { return m_pALIReconfService;}

   void reconfConfigure()  {
      m_pALIReconfService->reconfConfigure(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

   void reconfDeactivate() {
      m_pALIReconfService->reconfDeactivate(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

   void reconfActivate() {
      m_pALIReconfService->reconfActivate(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

protected:
   Runtime              *m_pRuntime;           ///< AAL Runtime
   IBase                *m_pFMEService;        ///< The generic AAL Service interface for the AFU.
   IALIReconfigure      *m_pALIReconfService;  ///< Pointer to Buffer Service
   CSemaphore            m_Sem;                ///< For synchronizing with the AAL runtime.
   btInt                 m_Result;             ///< Returned result v; 0 if success
   NamedValueSet         m_reconfnvs;          ///< Reconfigure input parameters
   btID                  m_errNum;             ///< AAL Error code
};
#endif  //__RECONF_SERVICE__
