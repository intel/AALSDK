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
/// @file fpgasane.cpp
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
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 7/21/2014      TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aalclp/aalclp.h>

#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/service/CCIAFUService.h>
#include <aalsdk/service/ICCIClient.h>

#include <aalsdk/utils/SingleAFUApp.h>

#include <aalsdk/kernel/NLBVAFU.h>

#ifdef INFO
# undef INFO
#endif // INFO
#if 0
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif

#ifdef ERR
# undef ERR
#endif // ERR
#if 0
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

/// @addtogroup cciapp
/// @{

////////////////////////////////////////////////////////////////////////////////
// CMyCCIClient

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyCCIClient CMyfpgasaneCCIClient

/// @brief cciapp-specific instantiation of ICCIClient that receives the event notifications
///        sent by the ICCIAFU.
class CMyCCIClient : public ICCIClient,
                     public CAASBase
{
public:
   /// Native Loopback (NLB) requires three workspaces:
   /// <ul>
   ///   <li>a Device Status Memory workspace.</li>
   ///   <li>an Input or source data buffer for the memory copy.</li>
   ///   <li>an Output or destination data buffer for the memory copy.</li>
   /// </ul>
   enum WorkspaceType
   {
      WKSPC_DSM, ///< Device Status Memory
      WKSPC_IN,  ///< Input workspace
      WKSPC_OUT  ///< Output workspace
   };

   CMyCCIClient();

   // <ICCIClient>
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void          OnWorkspaceFreed(TransactionID const &TranID);

   virtual void     OnWorkspaceFreeFailed(const IEvent &Event);
   // </ICCIClient>

   btVirtAddr DSMVirt()    const { return m_DSMVirt;    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_InputVirt;  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_OutputVirt; } ///< Accessor for the Output workspace.

   btPhysAddr DSMPhys()    const { return m_DSMPhys;    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_InputPhys;  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_OutputPhys; } ///< Accessor for the Output workspace.

   btWSSize   DSMSize()    const { return m_DSMSize;    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_InputSize;  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_OutputSize; } ///< Accessor for the Output workspace.

   /// @brief Wait on the client's internal semaphore.
   void ClientWait();

protected:
   /// @brief Signal (one count) to the client's internal semaphore.
   void ClientPost();

   /// @brief Mutator for the DSM workspace.
   void DSM(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_DSMVirt = v;
      m_DSMPhys = p;
      m_DSMSize = s;
   }

   /// @brief Mutator for the Input workspace.
   void Input(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_InputVirt = v;
      m_InputPhys = p;
      m_InputSize = s;
   }

   /// @brief Mutator for the Output workspace.
   void Output(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_OutputVirt = v;
      m_OutputPhys = p;
      m_OutputSize = s;
   }

   btVirtAddr m_DSMVirt;    ///< DSM workspace virtual address.
   btPhysAddr m_DSMPhys;    ///< DSM workspace physical address.
   btWSSize   m_DSMSize;    ///< DSM workspace size in bytes.
   btVirtAddr m_InputVirt;  ///< Input workspace virtual address.
   btPhysAddr m_InputPhys;  ///< Input workspace physical address.
   btWSSize   m_InputSize;  ///< Input workspace size in bytes.
   btVirtAddr m_OutputVirt; ///< Output workspace virtual address.
   btPhysAddr m_OutputPhys; ///< Output workspace physical address.
   btWSSize   m_OutputSize; ///< Output workspace size in bytes.
   btInt      m_Wkspcs;     ///< current number of workspaces allocated.
   CSemaphore m_Sem;        ///< client's internal semaphore.
};

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

////////////////////////////////////////////////////////////////////////////////
// CMyApp
// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyApp CMyfpgasaneApp

/// The default CCIAFU Delegate.
#define DEFAULT_TARGET_AFU CCIAFU_NVS_VAL_TARGET_FPGA

/// @brief cciapp-specific instantiation of ISingleAFUApp that provides access to an ICCIAFU.
class CMyApp : public ISingleAFUApp<ICCIAFU>
{
public:
   CMyApp();

   // <ISingleAFUApp>
   virtual void OnRuntimeStarted(AAL::XL::RT::IRuntime *,
                                 const AAL::NamedValueSet &);
   virtual void OnRuntimeStopped(AAL::XL::RT::IRuntime *);
   virtual void OnRuntimeStartFailed(const AAL::IEvent &);
   virtual void OnRuntimeAllocateServiceFailed(IEvent const &);
   virtual void OnRuntimeAllocateServiceSucceeded(AAL::IBase * ,
                                                  TransactionID const &);
   virtual void OnRuntimeEvent(const AAL::IEvent &);

   virtual void OnServiceAllocated(AAL::IBase *,
                                   AAL::TransactionID const &);
   virtual void OnServiceAllocateFailed(const AAL::IEvent &);
   virtual void OnServiceFreed(AAL::TransactionID const &);
   virtual void OnServiceEvent(const AAL::IEvent &);
   // </ISingleAFUApp>

   /// @brief Mutator for setting the NVS value that selects the AFU Delegate.
   void AFUTarget(const std::string &target) { m_AFUTarget = target; }
   /// @brief Accessor for the NVS value that selects the AFU Delegate.
   std::string AFUTarget() const             { return m_AFUTarget;   }

   /// @brief Wait on the m_CCIClient's internal semaphore.
   void ClientWait()       { m_CCIClient.ClientWait();  }
   /// @brief Determine m_CCIClient's status.
   btBool ClientOK() const { return m_CCIClient.IsOK(); }

   btVirtAddr DSMVirt()    const { return m_CCIClient.DSMVirt();    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_CCIClient.InputVirt();  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_CCIClient.OutputVirt(); } ///< Accessor for the Output workspace.

   btPhysAddr DSMPhys()    const { return m_CCIClient.DSMPhys();    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_CCIClient.InputPhys();  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_CCIClient.OutputPhys(); } ///< Accessor for the Output workspace.

   btWSSize   DSMSize()    const { return m_CCIClient.DSMSize();    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_CCIClient.InputSize();  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_CCIClient.OutputSize(); } ///< Accessor for the Output workspace.

protected:
   std::string  m_AFUTarget; ///< The NVS value used to select the AFU Delegate (FPGA, ASE, or SWSim).
   CMyCCIClient m_CCIClient; ///< The ICCIClient used to communicate with the allocated Service.
};

CMyApp::CMyApp() :
   m_AFUTarget(DEFAULT_TARGET_AFU)
{
   SetInterface(iidCCIClient, dynamic_cast<ICCIClient *>(&m_CCIClient));
}

void CMyApp::OnRuntimeStarted(AAL::XL::RT::IRuntime    *pRT,
                              const AAL::NamedValueSet &Args)
{
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

void CMyApp::OnRuntimeStopped(AAL::XL::RT::IRuntime *pRT)
{
   INFO("Runtime Stopped");
}

void CMyApp::OnRuntimeStartFailed(const AAL::IEvent &e)
{
   m_bIsOK = false;
   INFO("Runtime Start Failed");
}

void CMyApp::OnRuntimeAllocateServiceFailed(IEvent const &e)
{
   m_bIsOK = false;
   ERR("Service Allocate Failed (rt)");
}

void CMyApp::OnRuntimeAllocateServiceSucceeded(AAL::IBase          *pServiceBase,
                                               TransactionID const &tid)
{
   INFO("Service Allocated (rt)");
}

void CMyApp::OnRuntimeEvent(const AAL::IEvent &e)
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

void CMyApp::OnServiceAllocated(AAL::IBase               *pServiceBase,
                                AAL::TransactionID const &tid)
{
   INFO("Service Allocated");
}

void CMyApp::OnServiceAllocateFailed(const AAL::IEvent &e)
{
   m_bIsOK = false;
   ERR("Service Allocate Failed");
}

void CMyApp::OnServiceFreed(AAL::TransactionID const &tid)
{
   INFO("Service Freed");
}

void CMyApp::OnServiceEvent(const AAL::IEvent &e)
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

/// @} group cciapp

////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS

struct CMyCmdLine
{
   btUIntPtr   flags;
#define MY_CMD_FLAG_HELP    0x00000001
#define MY_CMD_FLAG_VERSION 0x00000002

   std::string AFUTarget;
   btInt       LogLevel;
};

struct CMyCmdLine gMyCmdLine =
{
   0,
   std::string(DEFAULT_TARGET_AFU),
   0
};

int my_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int my_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only my_nix_long_option_only = { my_on_nix_long_option_only, };
aalclp_option      my_nix_long_option      = { my_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "fpgasane",
                             PACKAGE_VERSION,
                             "",
                             help_msg_callback,
                             &gMyCmdLine)

int parsecmds(struct CMyCmdLine * , int , char *[] );
int verifycmds(struct CMyCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

#define NANOSEC_PER_MILLI(x)      ((x) * 1000 * 1000)

class INLB
{
public:
   virtual ~INLB() {}
   virtual btInt RunTest(btWSSize wssize) = 0;

   std::string ReadBandwidth()  const { return m_RdBw; }
   std::string WriteBandwidth() const { return m_WrBw; }

protected:
   INLB(CMyApp *pMyApp) :
      m_pMyApp(pMyApp),
      m_pCCIAFU((ICCIAFU *) *pMyApp) // uses type cast operator from ISingleAFUApp.
   {
      ASSERT(NULL != m_pMyApp);
      ASSERT(NULL != m_pCCIAFU);

      btInt i;
      for ( i = 0 ; i < sizeof(m_QLPCounters) / sizeof(m_QLPCounters[0]) ; ++i ) {
         m_QLPCounters[i] = 0;
         m_SavedQLPCounters[i] = 0;
      }
   }

   btInt ResetHandshake();
   btInt CacheCooldown(btVirtAddr CoolVirt, btPhysAddr CoolPhys, btWSSize CoolSize);

   void       ReadQLPCounters();
   void       SaveQLPCounters();
   bt32bitCSR   GetQLPCounter(btUnsignedInt ) const;

   std::string  CalcReadBandwidth();
   std::string CalcWriteBandwidth();

   CMyApp     *m_pMyApp;
   ICCIAFU    *m_pCCIAFU;
   bt32bitCSR  m_QLPCounters[QLP_NUM_COUNTERS];
   bt32bitCSR  m_SavedQLPCounters[QLP_NUM_COUNTERS];
   std::string m_RdBw;
   std::string m_WrBw;
};

class CNLBLpbk1 : public INLB
{
public:
   CNLBLpbk1(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(btWSSize wssize);
};

class CNLBRead : public INLB
{
public:
   CNLBRead(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(btWSSize wssize);
};

class CNLBWrite : public INLB
{
public:
   CNLBWrite(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(btWSSize wssize);
};

class CNLBTrput : public INLB
{
public:
   CNLBTrput(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(btWSSize wssize);
};

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

   if ( argc < 2 ) {
      ;
   } else if ( parsecmds(&gMyCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gMyCmdLine.flags, MY_CMD_FLAG_HELP|MY_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( verifycmds(&gMyCmdLine) ) {
      return 3;
   }

   cout << endl
        << "Intel QuickAssist FPGA Installation Test:" << endl;

#if DBG_HOOK
   cerr << "Waiting for debugger attach.." << endl;
   while ( gWaitForDebuggerAttach ) {
      SleepSec(1);
   }
   // Init the AAL logger.
   pAALLogger()->AddToMask(LM_All, 8); // All subsystems
   pAALLogger()->SetDestination(ILogger::CERR);
#else
   if ( gMyCmdLine.LogLevel > 0 ) {
      pAALLogger()->AddToMask(LM_All, gMyCmdLine.LogLevel);
      //pAALLogger()->SetDestination(ILogger::CERR);
      pAALLogger()->SetLogPID(true);
      pAALLogger()->SetLogTimeStamp(true);
   }
#endif // DBG_HOOK

   CMyApp        myapp;
   NamedValueSet args;
   Runtime       aal;

   myapp.AFUTarget(gMyCmdLine.AFUTarget);

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
   res = nlblpbk1.RunTest(MAX_NLB_LPBK1_WKSPC);
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
   res = nlbread.RunTest(MAX_NLB_READ_WKSPC);
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
   res = nlbwrite.RunTest(MAX_NLB_WRITE_WKSPC);
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
   res = nlbtrput.RunTest(MAX_NLB_TRPUT_WKSPC);
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

std::string INLB::CalcReadBandwidth()
{
   const btUnsigned32bitInt clockfreq = 200 * 1000000;
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

std::string INLB::CalcWriteBandwidth()
{
   const btUnsigned32bitInt clockfreq = 200 * 1000000;
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

// non-continuous mode.
// no cache treatment.
btInt CNLBLpbk1::RunTest(btWSSize wssize)
{
   btInt res = 0;

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   m_pCCIAFU->WorkspaceAllocate(wssize,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceAllocate(wssize,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt = m_pMyApp->InputVirt();

   const    btUnsigned32bitInt  InputData = 0xdecafbad;
   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
                                     (m_pMyApp->InputSize() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) {
      *pInput = InputData;
   }

   volatile btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();

   // zero the output buffer
   ::memset((void *)pOutputUsrVirt, 0, m_pMyApp->OutputSize());

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   m_pCCIAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set output workspace address
   m_pCCIAFU->CSRWrite(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the number of cache lines for the test
   m_pCCIAFU->CSRWrite(CSR_NUM_LINES, wssize / CL(1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_LPBK1);

   // Start the test
   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while ( 0 == pAFUDSM->test_complete ) {
      SleepMicro(100);
   }

   // Stop the device
   m_pCCIAFU->CSRWrite(CSR_CTL, 7);
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Verify the buffers
   if ( ::memcmp((void *)pInputUsrVirt, (void *)pOutputUsrVirt, wssize) != 0 ) {
      ++res;
   }

   // Verify the device
   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }


   // Clean up..

   // Release the Workspaces
   m_pCCIAFU->WorkspaceFree(pInputUsrVirt,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceFree(pOutputUsrVirt,      TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}

// continuous mode for 10 seconds.
// cool off fpga cache.
btInt CNLBRead::RunTest(btWSSize wssize)
{
   btInt res = 0;

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   m_pCCIAFU->WorkspaceAllocate(wssize,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));

   // Overloading the "output" workspace here to be the buffer we use to cool down the cache.
   m_pCCIAFU->WorkspaceAllocate(MAX_NLB_READ_WKSPC, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   const btUnsigned32bitInt ReadBufData = 0xc0cac01a;

   volatile btVirtAddr pInputUsrVirt = m_pMyApp->InputVirt();

   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
                                     (m_pMyApp->InputSize() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) {
      *pInput = ReadBufData;
   }

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   btVirtAddr pCoolOffUsrVirt = m_pMyApp->OutputVirt();

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   if ( 0 != CacheCooldown(pCoolOffUsrVirt, m_pMyApp->OutputPhys(), m_pMyApp->OutputSize()) ) {
      return 1;
   }

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   ReadQLPCounters();
   SaveQLPCounters();

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   m_pCCIAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set the number of cache lines for the test
   m_pCCIAFU->CSRWrite(CSR_NUM_LINES, wssize / CL(1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_READ|NLB_TEST_MODE_CONT);

   btInt TestTimeoutSeconds = 10;

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);

   // Start the test
   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( TestTimeoutSeconds > 0 ) ) {
      SleepSec(1);
      --TestTimeoutSeconds;
   }

   // Stop the device
   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( MaxPoll >= 0 ) ) {
      MaxPoll -= 500;
      SleepNano(500);
   }

   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   ReadQLPCounters();

   // Check the device status
   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for test stop was exceeded." << endl;
      ++res;
   }

   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   m_RdBw = CalcReadBandwidth();

   // Clean up..

   // Release the Workspaces
   m_pCCIAFU->WorkspaceFree(pInputUsrVirt,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceFree(pCoolOffUsrVirt,     TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}

btInt CNLBWrite::RunTest(btWSSize wssize)
{
   btInt res = 0;

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Overloading the "input" workspace here to be the buffer we use to cool down the cache.
   m_pCCIAFU->WorkspaceAllocate(MAX_NLB_WRITE_WKSPC, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));

   m_pCCIAFU->WorkspaceAllocate(wssize, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   btVirtAddr pCoolOffUsrVirt = m_pMyApp->InputVirt();
   btVirtAddr pOutputUsrVirt  = m_pMyApp->OutputVirt();

   // Zero the dest buffer.
   ::memset((void*)pOutputUsrVirt, 0, m_pMyApp->OutputSize());

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   if ( 0 != CacheCooldown(pCoolOffUsrVirt, m_pMyApp->InputPhys(), m_pMyApp->InputSize()) ) {
      return 1;
   }

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   ReadQLPCounters();
   SaveQLPCounters();

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set output workspace address
   m_pCCIAFU->CSRWrite(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the number of cache lines for the test
   m_pCCIAFU->CSRWrite(CSR_NUM_LINES, wssize / CL(1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_WRITE|NLB_TEST_MODE_CONT);

   btInt TestTimeoutSeconds = 10;

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);

   // Start the test
   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( TestTimeoutSeconds > 0 ) ) {
      SleepSec(1);
      --TestTimeoutSeconds;
   }

   // Stop the device
   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( MaxPoll >= 0 ) ) {
      MaxPoll -= 500;
      SleepNano(500);
   }

   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   ReadQLPCounters();

   // Check the device status
   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for test stop was exceeded." << endl;
      ++res;
   }

   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   m_WrBw = CalcWriteBandwidth();

   // Clean up..

   // Release the Workspaces
   m_pCCIAFU->WorkspaceFree(pCoolOffUsrVirt,     TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceFree(pOutputUsrVirt,      TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}

btInt CNLBTrput::RunTest(btWSSize wssize)
{
   btInt res = 0;

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   m_pCCIAFU->WorkspaceAllocate(wssize, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceAllocate(wssize, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   btVirtAddr pInputUsrVirt  = m_pMyApp->InputVirt();
   btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   ReadQLPCounters();
   SaveQLPCounters();

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   m_pCCIAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set output workspace address
   m_pCCIAFU->CSRWrite(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the number of cache lines for the test
   m_pCCIAFU->CSRWrite(CSR_NUM_LINES, wssize / CL(1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_TRPUT|NLB_TEST_MODE_CONT);

   btInt TestTimeoutSeconds = 10;

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);

   // Start the test
   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( TestTimeoutSeconds > 0 ) ) {
      SleepSec(1);
      --TestTimeoutSeconds;
   }

   // Stop the device
   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( MaxPoll >= 0 ) ) {
      MaxPoll -= 500;
      SleepNano(500);
   }

   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   ReadQLPCounters();

   // Check the device status
   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for test stop was exceeded." << endl;
      ++res;
   }

   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   m_RdBw = CalcReadBandwidth();
   m_WrBw = CalcWriteBandwidth();

   // Clean up..

   // Release the Workspaces
   m_pCCIAFU->WorkspaceFree(pInputUsrVirt,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceFree(pOutputUsrVirt,      TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}


#if defined ( __AAL_WINDOWS__ )
# define strcasecmp _stricmp
#endif // __AAL_WINDOWS__


BEGIN_C_DECLS

int my_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
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
}

int parsecmds(struct CMyCmdLine *cl, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   my_nix_long_option_only.user = cl;
   aalclp_add_nix_long_option_only(&clp, &my_nix_long_option_only);

   my_nix_long_option.user = cl;
   aalclp_add_nix_long_option(&clp, &my_nix_long_option);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}

int verifycmds(struct CMyCmdLine *cl)
{
   return 0;
}

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

