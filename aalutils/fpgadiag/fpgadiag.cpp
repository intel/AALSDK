// Copyright (c) 2015, Intel Corporation
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
/// @file fpgadiag.cpp
/// @brief Uses XL and ICCIAFU to interact with CCI.
/// @ingroup cciapp
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// This sample demonstrates the following:
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///			 Sadruta Chandrashekar, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 7/21/2014      TSW      Initial version(fpgasane).
/// 5/28/2015      SC       fapdiag version.@endverbatim
//****************************************************************************
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aalclp/aalclp.h>

#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/service/CCIAFUService.h>
#include <aalsdk/service/ICCIClient.h>

//#include <aalsdk/utils/SingleAFUApp.h>
//#include <aalsdk/utils/Utilities.h>
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>

#include <aalsdk/utils/NLBVAFU.h>
#include "diag-nlb-common.h"
#include "diag-common.h"
#include "diag_defaults.h"

#define AALSDK_COPYRIGHT_STMNT "Copyright (c) 2003-2015, Intel Corporation" //TODO add in appropriate header file

USING_NAMESPACE(AAL)

#ifdef INFO
# undef INFO
#endif // INFO
#if 1
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif

#ifdef ERR
# undef ERR
#endif // ERR
#if 1
# define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define ERR(x)
#endif

// Change DBG_HOOK to 1 if you want an opportunity to attach the debugger.
// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#define DBG_HOOK 0
#if DBG_HOOK
btBool gWaitForDebuggerAttach = true;
#endif // DBG_HOOK

//#define iidICCIAFU __AAL_IID(AAL_sysAAL, 0x0007)

/// @addtogroup cciapp
/// @{

BEGIN_C_DECLS
struct NLBCmdLine gCmdLine =
{
   AALSDK_COPYRIGHT_STMNT,
   NLB_MODE,
   NLB_TITLE,
   NLB_CMD_FLAG_BANDWIDTH,
   0,
   0,
   DEFAULT_BEGINCL,
   DEFAULT_ENDCL,
   DEFAULT_DSMPHYS,
   DEFAULT_SRCPHYS,
   DEFAULT_DSTPHYS,
   200000000ULL, // TODO - Add as DEFAULT_FPGA_CLK_FREQ.Hertz(), - But this is hardcoded as well
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
   { 0, 0 },
   0,
   0,
   0,
   0,
   0,
   0,
#endif // OS
   // defaults
   {
      NLB_MIN_CL,
      NLB_MAX_CL,
      DEFAULT_BEGINCL,
      DEFAULT_ENDCL,
      DEFAULT_DSMPHYS,
      DEFAULT_SRCPHYS,
      DEFAULT_DSTPHYS,
      200000000ULL, // TODO - Add as DEFAULT_FPGA_CLK_FREQ.Hertz(), - But this is hardcoded as well
      DEFAULT_PREFILLHITS,
      DEFAULT_PREFILLMISS,
      DEFAULT_NOBW,
      DEFAULT_TABULAR,
      DEFAULT_SUPPRESSHDR,
      DEFAULT_WT,
      DEFAULT_WB,
      DEFAULT_PWR,
      DEFAULT_CONT,
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
      DEFAULT_TONSEC,
      DEFAULT_TOUSEC,
      DEFAULT_TOMSEC,
      DEFAULT_TOSEC,
      DEFAULT_TOMIN,
      DEFAULT_TOHOUR,
#endif // OS
      DEFAULT_NOGUI,
      DEFAULT_DEMO,
      DEFAULT_NOHIST,
      DEFAULT_HISTDATA
   },
   0,
   {
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } }
   },
   0,
   std::string(DEFAULT_TARGET_AFU),
   0
};

END_C_DECLS


////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS

/*struct CMyCmdLine
{
   btUIntPtr   flags;
#define MY_CMD_FLAG_HELP    0x00000001
#define MY_CMD_FLAG_VERSION 0x00000002

   std::string AFUTarget;
   btInt       LogLevel;
};*/

/*struct CMyCmdLine gMyCmdLine =
{
   0,
   std::string(DEFAULT_TARGET_AFU),
   0
};*/

/*int my_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int my_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only my_nix_long_option_only = { my_on_nix_long_option_only, };
aalclp_option      my_nix_long_option      = { my_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );*/

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "fpgasane",
                             "0.0.0",
                             "",
                             nlb_help_message_callback,
                             &gCmdLine)

int ParseCmds(struct NLBCmdLine * , int , char *[] );
//int verifycmds(struct NLBCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

#define NANOSEC_PER_MILLI(x)      ((x) * 1000 * 1000)

CMyApp::CMyApp() :
   m_AFUTarget(DEFAULT_TARGET_AFU),
   m_pRuntime(NULL),
   m_pAALService(NULL),
   m_pProprietary(NULL)
{
	m_Sem.Create(0, 1);
	SetSubClassInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this)); //TODO check if CCIAFU expects ICCIClient
    SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
	SetInterface(iidCCIClient, dynamic_cast<ICCIClient *>(&m_CCIClient));
}

CMyApp::~CMyApp()
{
   Stop();
   m_Sem.Destroy();
}


void CMyApp::Stop()
{
   if ( NULL != m_pAALService ) {
      m_pAALService->Release(TransactionID(), 0);
      Wait(); // For service freed notification.
      m_pAALService = NULL;
   }

   if ( NULL != m_pRuntime ) {
      m_pRuntime->stop();
      Wait(); // For runtime stopped notification.
      m_pRuntime = NULL;
   }

   Post(); // Wake up main, if waiting
}

void CMyApp::runtimeStarted(IRuntime            *pRT,
							const NamedValueSet &Args)
{
   ASSERT(pRT->IsOK());
   if ( !pRT->IsOK() ) {
      return;
   }

   m_pRuntime = pRT;

   btcString AFUName = "CCIAFU";

     INFO("Allocating " << AFUName << " Service");

     // NOTE: This example is bypassing the Resource Manager's configuration record lookup
     //  mechanism.  This code is workaround code and is subject to change.

     NamedValueSet Manifest(CCIAFU_MANIFEST);

     Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, AFUName);
     Manifest.Add(CCIAFU_NVS_KEY_TARGET, AFUTarget().c_str());

  #if DBG_HOOK
     INFO(Manifest);
  #endif // DBG_HOOK

     pRT->allocService(dynamic_cast<IBase *>(this), Manifest);
}

void CMyApp::runtimeStopped(IRuntime *pRT)
{
   INFO("Runtime Stopped");
   Post();
}

void CMyApp::runtimeStartFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   INFO("Runtime Start Failed");
   Post();
}

void CMyApp::runtimeAllocateServiceFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   ERR("Service Allocate Failed (rt)");
}

void CMyApp::runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
											 TransactionID const &tid)
{
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   ASSERT(NULL != m_pAALService);

   m_pProprietary = subclass_ptr<ICCIAFU>(pServiceBase);
   ASSERT(NULL != m_pProprietary);

   INFO("Service Allocated (rt)");
}

void CMyApp::runtimeEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
        PrintExceptionDescription(e);
        m_bIsOK = false;
        Post();
        return;
     }

     INFO("Unknown event (rt)");
     Post();
}

void CMyApp::serviceAllocated(IBase               *pServiceBase,
							  TransactionID const &tid)
{
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   ASSERT(NULL != m_pAALService);

   m_pProprietary = subclass_ptr<ICCIAFU>(pServiceBase);
   ASSERT(NULL != m_pProprietary);

   INFO("Service Allocated");
   Post();
}

void CMyApp::serviceAllocateFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   ERR("Service Allocate Failed");
   Post();
}

void CMyApp::serviceFreed(TransactionID const &tid)
{
	INFO("Service Freed");
   Post();
}

void CMyApp::serviceEvent(const IEvent &e)
{
	  if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
	      PrintExceptionDescription(e);
	      m_bIsOK = false;
	      Post();
	      return;
	   }

	   INFO("Unknown event");
	   Post();
}
///////////////////////////////////////////////////////////////////////////////////

CMyCCIClient::CMyCCIClient() :
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_DSMSize(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_InputSize(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0),
   m_OutputSize(0),
   m_Wkspcs(0)
{
   m_Sem.Create(0, INT_MAX);
   SetSubClassInterface(iidCCIClient, dynamic_cast<ICCIClient *>(this));
}

void CMyCCIClient::OnWorkspaceAllocated(TransactionID const &TranID,
                                        btVirtAddr           WkspcVirt,
                                        btPhysAddr           WkspcPhys,
                                        btWSSize             WkspcSize)
{
   btBool GotOne = true;
   AutoLock(this);

   switch ( (CMyCCIClient::WorkspaceType)TranID.ID() ) {
      case WKSPC_DSM : {
         DSM(WkspcVirt, WkspcPhys, WkspcSize);
         INFO("Got DSM");
      } break;
      case WKSPC_IN : {
         Input(WkspcVirt, WkspcPhys, WkspcSize);
         INFO("Got Input Workspace");
      } break;
      case WKSPC_OUT : {
         Output(WkspcVirt, WkspcPhys, WkspcSize);
         INFO("Got Output Workspace");
      } break;
      default : {
         GotOne = false;
         ERR("Invalid workspace type: " << TranID.ID());
      } break;
   }

   if ( GotOne ) {
      ++m_Wkspcs;
      if ( 3 == m_Wkspcs ) {
         ClientPost();
      }
   }
}

void CMyCCIClient::OnWorkspaceAllocateFailed(const IEvent & /*unused*/)
{
   AutoLock(this);
   m_bIsOK = false;
   ClientPost();
}

void CMyCCIClient::OnWorkspaceFreed(TransactionID const &TranID)
{
   btBool FreedOne = true;
   AutoLock(this);

   switch ( (CMyCCIClient::WorkspaceType)TranID.ID() ) {
      case WKSPC_DSM : {
         DSM(NULL, 0, 0);
         INFO("Freed DSM");
      } break;
      case WKSPC_IN : {
         Input(NULL, 0, 0);
         INFO("Freed Input Workspace");
      } break;
      case WKSPC_OUT : {
         Output(NULL, 0, 0);
         INFO("Freed Output Workspace");
      } break;
      default : {
         FreedOne = false;
         ERR("Invalid workspace type");
      } break;
   }

   if ( FreedOne ) {
      --m_Wkspcs;
      if ( 0 == m_Wkspcs ) {
         ClientPost();
      }
   }
}

void CMyCCIClient::OnWorkspaceFreeFailed(const IEvent & /*unused*/)
{
   AutoLock(this);
   m_bIsOK = false;
   ClientPost();
}

void CMyCCIClient::ClientWait() { m_Sem.Wait();  }

void CMyCCIClient::ClientPost() { m_Sem.Post(1); }

//////////////////////////////////////////////////////////////////////////////


inline std::ostream & PASS(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '3', '2', 'm', 0 };
      os << Esc;
   }
   return os;
}
inline std::ostream & FAIL(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '3', '1', 'm', 0 };
      os << Esc;
   }
   return os;
}
inline std::ostream & NORMAL(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '0', 'm', 0 };
      os << Esc;
   }
   return os;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   btInt res      = 0;
   btInt totalres = 0;

   // NLBConfig          cfg;
   /*uint_type          i;
   uint_type          NumCacheLines;
   wkspc_size_type    sz;
   std::ostringstream oss;
   Workspace          DSMWkspc;
   Workspace          SrcWkspc(NULLWorkspace);
   Workspace          DestWkspc(NULLWorkspace);*/

   if ( argc < 2 ) {
	   MyNLBShowHelp(stdout, &_aalclp_gcs_data);
	   return 1;
   } else if ( 0 != ParseCmds(&gCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gCmdLine.flags, MY_CMD_FLAG_HELP|MY_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( !NLBVerifyCmdLine(gCmdLine, std::cout) ) {
      return 3;
   }

   cout << endl
        << "FpgaDiag - Intel QuickAssist FPGA Installation Test:" << endl;

#if DBG_HOOK
   cerr << "Waiting for debugger attach.." << endl;
   while ( gWaitForDebuggerAttach ) {
      SleepSec(1);
   }
   // Init the AAL logger.
   pAALLogger()->AddToMask(LM_All, 8); // All subsystems
   pAALLogger()->SetDestination(ILogger::CERR);
#else
   if ( gCmdLine.LogLevel > 0 ) {
      pAALLogger()->AddToMask(LM_All, gCmdLine.LogLevel);
      //pAALLogger()->SetDestination(ILogger::CERR);
      pAALLogger()->SetLogPID(true);
      pAALLogger()->SetLogTimeStamp(true);
   }
#endif // DBG_HOOK

   CMyApp        myapp;
   NamedValueSet args;
   Runtime       aal;

   myapp.AFUTarget(gCmdLine.AFUTarget);

   if ( (0 == myapp.AFUTarget().compare(CCIAFU_NVS_VAL_TARGET_ASE)) ||
        (0 == myapp.AFUTarget().compare(CCIAFU_NVS_VAL_TARGET_SWSIM)) ) {
      args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);
   } else {
      NamedValueSet ConfigRecord;
      ConfigRecord.Add(XLRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
      args.Add(XLRUNTIME_CONFIG_RECORD, ConfigRecord);
   }

   INFO("Starting the AAL Runtime");
   if ( aal.start(&myapp, args) ) {
      myapp.Wait(); // For service allocated notification.
   } else {
      ERR("AAL Runtime start failed");
      return 4;     // Runtime could not start
   }

   if ( !myapp.IsOK() ) {
      // runtime start failed.
      return 5;
   }

   // Run NLB Lpbk1, which performs sw data verification.
   CNLBLpbk1 nlblpbk1(&myapp);

   cout << " * Data Copy " << flush;
   res = nlblpbk1.RunTest(gCmdLine, MAX_NLB_LPBK1_WKSPC);
   totalres += res;
   if ( 0 == res ) {
      cout << PASS << "VERIFIED";
   } else {
      cout << FAIL << "ERROR";
   }
   cout << NORMAL << endl;

   // Run an NLB Read bandwidth measurement..
   // * cold cache (a la, --prefill-misses)
   // * report read bandwidth in GiB/s
   CNLBRead nlbread(&myapp);

   cout << " * Read Bandwidth from Memory " << flush;
   res = nlbread.RunTest(gCmdLine, MAX_NLB_READ_WKSPC);
   totalres += res;
   if ( 0 == res ) {
      cout << PASS << nlbread.ReadBandwidth();
   } else {
      cout << FAIL << "ERROR";
   }
   cout << NORMAL << endl;

   // Run an NLB Write bandwidth measurement..
   // * cold cache (a la, --prefill-misses)
   // * report write bandwidth in GiB/s
   CNLBWrite nlbwrite(&myapp);

   cout << " * Write Bandwidth to Memory " << flush;
   res = nlbwrite.RunTest(gCmdLine, MAX_NLB_WRITE_WKSPC);
   totalres += res;
   if ( 0 == res ) {
      cout << PASS << nlbwrite.WriteBandwidth();
   } else {
      cout << FAIL << "ERROR";
   }
   cout << NORMAL << endl;

   // Run an NLB Trput measurement..
   // * report bandwidth in GiB/s
   CNLBTrput nlbtrput(&myapp);

   cout << " * Simultaneous Read/Write Bandwidth " << flush;
   res = nlbtrput.RunTest(gCmdLine, MAX_NLB_TRPUT_WKSPC);
   totalres += res;
   if ( 0 == res ) {
      cout << PASS << nlbtrput.ReadBandwidth() << " / " << nlbtrput.WriteBandwidth();
   } else {
      cout << FAIL << "ERROR";
   }
   cout << NORMAL << endl
        << endl;

   INFO("Stopping the AAL Runtime");
   myapp.Stop();

   if ( totalres > 0 ) {
      ERR("Test FAILED with " << res << " error" << ((res > 1) ? "s." : "."));
   } else {
      INFO("Test PASSED");
   }

   return totalres;
}

btInt INLB::ResetHandshake()
{
   btInt      res = 0;
   btCSRValue csr;
   btCSRValue tmp;

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   const btInt StatusTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StatusTimeoutMillis);

   // Assert CAFU Reset
   csr = 0;
   m_pCCIAFU->CSRRead(QLP_CSR_CIPUCTL, &csr);
   flag_setf(csr, CIPUCTL_RESET_BIT);
   m_pCCIAFU->CSRWrite(QLP_CSR_CIPUCTL, csr);

   // Poll CAFU Status until ready.
   do
   {
      SleepNano(500);
      MaxPoll -= 500;
      csr = 0;
      m_pCCIAFU->CSRRead(QLP_CSR_CAFU_STATUS, &csr);
   }while( flag_is_clr(csr, CAFU_STATUS_READY_BIT) && (MaxPoll >= 0) );

   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for CAFU_STATUS ready bit was exceeded." << endl;
      return 1;
   }

   // De-assert CAFU Reset
   csr = 0;
   m_pCCIAFU->CSRRead(QLP_CSR_CIPUCTL, &csr);
   flag_clrf(csr, CIPUCTL_RESET_BIT);
   m_pCCIAFU->CSRWrite(QLP_CSR_CIPUCTL, csr);

   tmp = 0;
   m_pCCIAFU->CSRRead(QLP_CSR_CIPUCTL, &tmp);
   ASSERT(csr == tmp);


   const btInt AFUIDTimeoutMillis = 250;
   MaxPoll = NANOSEC_PER_MILLI(AFUIDTimeoutMillis);

   // zero the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // Set DSM base, high then low
   m_pCCIAFU->CSRWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   // Poll for AFU ID
   do
   {
      SleepNano(500);
      MaxPoll -= 500;
   }while( (0 == pAFUDSM->afuid[0]) && (MaxPoll >= 0) );

   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for AFU ID was exceeded." << endl;
      return 1;
   }


   ASSERT(0x84570612 == pAFUDSM->afuid[0]);
   if ( 0x84570612 != pAFUDSM->afuid[0] ) {
      ++res;
   }

   ASSERT(0x9aeffe5f == pAFUDSM->afuid[1]);
   if ( 0x9aeffe5f != pAFUDSM->afuid[1] ) {
      ++res;
   }

   ASSERT(0x0d824272 == pAFUDSM->afuid[2]);
   if ( 0x0d824272 != pAFUDSM->afuid[2] ) {
      ++res;
   }

   ASSERT(0xc000c966 == pAFUDSM->afuid[3]);
   if ( 0xc000c966 != pAFUDSM->afuid[3] ) {
      ++res;
   }

   return res;
}

btInt INLB::CacheCooldown(btVirtAddr CoolVirt, btPhysAddr CoolPhys, btWSSize CoolSize)
{
   btInt res = 0;

   const btUnsigned32bitInt CoolOffData = 0xc001c001;

   volatile btUnsigned32bitInt *pCoolOff    = (volatile btUnsigned32bitInt *)CoolVirt;
   volatile btUnsigned32bitInt *pEndCoolOff = (volatile btUnsigned32bitInt *)pCoolOff +
                                     (CoolSize / sizeof(btUnsigned32bitInt));

   for ( ; pCoolOff < pEndCoolOff ; ++pCoolOff ) {
      *pCoolOff = CoolOffData;
   }

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   m_pCCIAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(CoolPhys));

   // Set the number of cache lines for the test
   m_pCCIAFU->CSRWrite(CSR_NUM_LINES, CoolSize / CL(1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_READ); // non-continuous mode

   // Start the test
   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while ( 0 == pAFUDSM->test_complete ) {
      SleepMicro(100);
   }

   // Stop the device
   m_pCCIAFU->CSRWrite(CSR_CTL, 7);
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Check the device status
   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   return res;
}

void INLB::ReadQLPCounters()
{
   btCSRValue perf[2];
   btCSRValue i;

   for ( i = 0 ; i < sizeof(m_QLPCounters) / sizeof(m_QLPCounters[0]) ; ++i ) {
      switch ( i ) {
         case QLP_PERF_CACHE_RD_HITS : // FALL THROUGH
         case QLP_PERF_CACHE_WR_HITS : // FALL THROUGH
         case QLP_PERF_CACHE_RD_MISS : // FALL THROUGH
         case QLP_PERF_CACHE_WR_MISS : // FALL THROUGH
         case QLP_PERF_EVICTIONS     : {

            perf[0] = 0;
            perf[1] = 0;

            m_pCCIAFU->CSRWrite(QLP_CSR_ADDR_PERF1C, i);
            m_pCCIAFU->CSRRead(QLP_CSR_ADDR_PERF1, &perf[0]);

            m_pCCIAFU->CSRWrite(QLP_CSR_ADDR_PERF1C, (btCSRValue)(1 << 31) | i);
            m_pCCIAFU->CSRRead(QLP_CSR_ADDR_PERF1, &perf[1]);

            m_QLPCounters[i] = perf[0] + perf[1];

         } break;

         default: break;
      }
   }
}

void INLB::SaveQLPCounters()
{
   btCSRValue i;
   for ( i = 0 ; i < sizeof(m_QLPCounters) / sizeof(m_QLPCounters[0]) ; ++i ) {
      m_SavedQLPCounters[i] = m_QLPCounters[i];
   }
}

bt32bitCSR INLB::GetQLPCounter(btUnsignedInt i) const
{
   switch ( i ) {
      case QLP_PERF_CACHE_RD_HITS : // FALL THROUGH
      case QLP_PERF_CACHE_WR_HITS : // FALL THROUGH
      case QLP_PERF_CACHE_RD_MISS : // FALL THROUGH
      case QLP_PERF_CACHE_WR_MISS : // FALL THROUGH
      case QLP_PERF_EVICTIONS     : return m_QLPCounters[i] - m_SavedQLPCounters[i];

      default : return 0;
   }
}

std::string INLB::CalcReadBandwidth(const NLBCmdLine &cmd)
{
   const btUnsigned32bitInt clockfreq = cmd.clkfreq;
   const double             giga      = 1000.0 * 1000.0 * 1000.0;

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;
   bt32bitCSR rds          = pAFUDSM->num_reads;

   //bt64bitCSR ticks = rawticks - (startpenalty + endpenalty);

   // cont mode
   bt64bitCSR ticks = rawticks - startpenalty;

   const double Rds   = (double)rds;
   const double Ticks = (double)ticks;
   const double Hz    = (double)clockfreq;

   double bw = (Rds * (CL(1) * Hz)) / Ticks;

   bw /= giga;

   std::ostringstream oss;

   oss.precision(3);
   oss.setf(std::ios::fixed, std::ios::floatfield);

   oss << bw << " GB/s";

   return oss.str();
}

std::string INLB::CalcWriteBandwidth(const NLBCmdLine &cmd)
{
   const btUnsigned32bitInt clockfreq = cmd.clkfreq;
   const double             giga      = 1000.0 * 1000.0 * 1000.0;

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;
   bt32bitCSR wrs          = pAFUDSM->num_writes;

   //bt64bitCSR ticks = rawticks - (startpenalty + endpenalty);

   // cont mode
   bt64bitCSR ticks = rawticks - startpenalty;

   const double Wrs   = (double)wrs;
   const double Ticks = (double)ticks;
   const double Hz    = (double)clockfreq;

   double bw = (Wrs * (CL(1) * Hz)) / Ticks;

   bw /= giga;

   std::ostringstream oss;

   oss.precision(3);
   oss.setf(std::ios::fixed, std::ios::floatfield);

   oss << bw << " GB/s";

   return oss.str();
}

std::string INLB::Normalized(const NLBCmdLine &cmd ) const throw()
{
	const btUnsigned32bitInt clockfreq = cmd.clkfreq;
	std::ostringstream oss;

   oss << std::dec;

   if ( clockfreq >= GHZ(1) ) {
      oss << (clockfreq / GHZ(1)) << " GHz";
      return oss.str();
   }
   if ( clockfreq >= MHZ(1) ) {
      oss << (clockfreq / MHZ(1)) << " MHz";
      return oss.str();
   }
   if ( clockfreq >= KHZ(1) ) {
      oss << (clockfreq / KHZ(1)) << " KHz";
      return oss.str();
   }
   oss << clockfreq << " Hz";
   return oss.str();

}




#if defined ( __AAL_WINDOWS__ )
# define strcasecmp _stricmp
#endif // __AAL_WINDOWS__


BEGIN_C_DECLS

/*int my_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct CMyCmdLine *cl = (struct CMyCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, MY_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, MY_CMD_FLAG_VERSION);
   }

   return 0;
}

int my_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct CMyCmdLine *cl = (struct CMyCmdLine *)user;

   if ( 0 == strcmp("--target", option) ) {
      if ( 0 == strcasecmp("fpga", value) ) {
         cl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_FPGA);
      } else if ( 0 == strcasecmp("ase", value) ) {
         cl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_ASE);
      } else if ( 0 == strcasecmp("swsim", value) ) {
         cl->AFUTarget = std::string(CCIAFU_NVS_VAL_TARGET_SWSIM);
      } else {
         cout << "Invalid value for --target : " << value << endl;
         return 4;
      }
   } else if ( 0 == strcmp("--log", option) ) {
      char *endptr = NULL;

      cl->LogLevel = (btInt)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->LogLevel = 0;
      } else if ( cl->LogLevel < 0) {
         cl->LogLevel = 0;
      } else if ( cl->LogLevel > 8) {
         cl->LogLevel = 8;
      }
   }
   return 0;
}

void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "fpgasane takes no command arguments.\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}*/

int ParseCmds(struct NLBCmdLine *nlbcl, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

  /* my_nix_long_option_only.user = cl;
   aalclp_add_nix_long_option_only(&clp, &my_nix_long_option_only);

   my_nix_long_option.user = cl;
   aalclp_add_nix_long_option(&clp, &my_nix_long_option);*/

   NLBSetupCmdLineParser(&clp, nlbcl);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR) ) {
         res = 99;
      }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}

/*int verifycmds(struct NLBCmdLine *cl)
{
   return 0;
}*/

END_C_DECLS


/**
@addtogroup cciapp
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>An ICCIClient implementation.</li>
  <li>Use of the ISingleAFUApp class template.</li>
  <li>Runtime selection of AFU targets with CCIAFU.</li>
  <li>Native Loopback with ICCIAFU.</li>
</ul>

This sample is designed to be used with CCIAFU.

1 Summary of Operation

cciapp relies on its instantiation of ISingleAFUApp inherited by CMyApp to
perform the brunt of the XL runtime interaction. An XL Runtime object is declared
in main() to handle the runtime startup and shutdown.

An object instance of CMyApp is declared in main(), and the selection of AFU target
implementation is specified based on the command line parameters to cciapp.

Some AFU-specific Runtime parameter configuration is performed prior to passing the
CMyApp instance to the Runtime's start routine. When the override of ISingleAFUApp::OnRuntimeStarted
is called, an instance of CCIAFU is requested.

The service allocation request is satisfied by ISingleAFUApp::serviceAllocated, which
pulses the internal semaphore, waking the thread blocked in main(). Once awake, some basic
error checking is done on the CMyApp instance and the Native Loopback test is performed.

When the Native Loopback test is complete, ISingleAFUApp::Stop is called to release the AFU and to
wait for the release notification from the runtime.

Finally, the Native Loopback test status is reported, and the application exits.

2 Running the application

2.0 Online Help

@code
$ cciapp --help

Usage:
   cciapp [--target=<TARGET>]

      <TARGET> = one of { fpga ase swsim }@endcode

2.1 CCI FPGA (HWCCIAFU)

Prerequisites for running the sample with an FPGA:
<ul>
  <li>The CCI AAL device drivers must be loaded.</li>
  <li>The AAL Resource Manager must be running.</li>
  <li>The FPGA module connected to the system must be programmed with an appropriate CCI AFU bit stream.</li>
</ul>

@code
$ cciapp --target=fpga@endcode

2.2 CCI AFU Simulation Environment (ASECCIAFU)

Prerequisites for running the sample with ASE:
<ul>
  <li>The ASE simulation-side application must be running on the system.</li>
</ul>

@code
$ cciapp --target=ase@endcode

2.3 CCI Software Simulation (SWSimCCIAFU)

Prerequisites for running the sample with Software Simulation:
<ul>
  <li>(none)
</ul>

@code
$ cciapp --target=swsim@endcode

@} group cciapp
*/

