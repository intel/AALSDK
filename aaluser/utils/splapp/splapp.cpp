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
/// @file splapp.cpp
/// @brief Uses XL and ISPLAFU to interact with SPL.
/// @ingroup splapp
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

#include <aalsdk/utils/SingleAFUApp.h>
#include <aalsdk/service/ISPLAFU.h>
#include <aalsdk/service/ISPLClient.h>
#include <aalsdk/service/SPLAFUService.h>

#include <aalsdk/kernel/vafu2defs.h>   // AFU structure definitions (brings in spl2defs.h)

USING_NAMESPACE(std)
USING_NAMESPACE(AAL)

#ifdef INFO
# undef INFO
#endif // INFO
#define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)

// Change DBG_HOOK to 1 if you want an opportunity to attach the debugger.
// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#define DBG_HOOK 0
#if DBG_HOOK
btBool gWaitForDebuggerAttach = true;
#endif // DBG_HOOK

/// @addtogroup splapp
/// @{

////////////////////////////////////////////////////////////////////////////////
// CMySPLClient

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMySPLClient CMysplappSPLClient

/// @brief splapp-specific instantiation of ISPLClient that receives the event notifications
///        sent by the ISPLAFU.
class CMySPLClient : public ISPLClient,
                     public CAASBase
{
public:
   CMySPLClient();

   // <ISPLClient>
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void          OnWorkspaceFreed(TransactionID const &TranID);

   virtual void     OnWorkspaceFreeFailed(const IEvent &Event);

   virtual void  OnTransactionStarted(TransactionID const &TranID,
                                      btVirtAddr           AFUDSMVirt,
                                      btWSSize             AFUDSMSize);

   virtual void OnContextWorkspaceSet(TransactionID const &TranID);

   virtual void   OnTransactionFailed(const IEvent &Event);

   virtual void OnTransactionComplete(TransactionID const &TranID);

   virtual void OnTransactionStopped(TransactionID const &TranID);
   // </ISPLClient>

   btVirtAddr OneLargeVirt() const { return m_OneLargeWorkspaceVirt; } ///< Accessor for the AFU Context workspace.
   btPhysAddr OneLargePhys() const { return m_OneLargeWorkspacePhys; } ///< Accessor for the AFU Context workspace.
   btWSSize   OneLargeSize() const { return m_OneLargeWorkspaceSize; } ///< Accessor for the AFU Context workspace.

   btVirtAddr AFUDSMVirt()   const { return m_AFUDSMVirt; } ///< Accessor for the AFU DSM workspace.
   btWSSize   AFUDSMSize()   const { return m_AFUDSMSize; } ///< Accessor for the AFU DSM workspace.

   /// @brief Wait on the client's internal semaphore.
   void Wait();

protected:
   /// @brief Signal (one count) to the client's internal semaphore.
   void Post();

   /// @brief Mutator for the AFU Context workspace.
   void OneLarge(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_OneLargeWorkspaceVirt = v;
      m_OneLargeWorkspacePhys = p;
      m_OneLargeWorkspaceSize = s;
   }

   /// @brief Mutator for the AFU DSM workspace.
   void AFUDSM(btVirtAddr v, btWSSize s)
   {
      m_AFUDSMVirt = v;
      m_AFUDSMSize = s;
   }

   btVirtAddr m_OneLargeWorkspaceVirt; ///< AFU Context workspace virtual address.
   btPhysAddr m_OneLargeWorkspacePhys; ///< AFU Context workspace physical address.
   btWSSize   m_OneLargeWorkspaceSize; ///< AFU Context workspace size in bytes.
   btVirtAddr m_AFUDSMVirt;            ///< AFU DSM workspace virtual address.
   btWSSize   m_AFUDSMSize;            ///< AFU DSM workspace size in bytes.
   CSemaphore m_Sem;                   ///< client's internal semaphore.
};

CMySPLClient::CMySPLClient() :
   m_OneLargeWorkspaceVirt(NULL),
   m_OneLargeWorkspacePhys(0),
   m_OneLargeWorkspaceSize(0),
   m_AFUDSMVirt(NULL),
   m_AFUDSMSize(0)
{
   m_Sem.Create(0, INT_MAX);
   SetSubClassInterface(iidSPLClient, dynamic_cast<ISPLClient *>(this));
}

void CMySPLClient::OnWorkspaceAllocated(TransactionID const &TranID,
                                        btVirtAddr           WkspcVirt,
                                        btPhysAddr           WkspcPhys,
                                        btWSSize             WkspcSize)
{
   OneLarge(WkspcVirt, WkspcPhys, WkspcSize);
   INFO("Got Workspace");
   Post();
}

void CMySPLClient::OnWorkspaceAllocateFailed(const IEvent & )
{
   m_bIsOK = false;
   ERR("Workspace Allocate Failed");
   Post();
}

void CMySPLClient::OnWorkspaceFreed(TransactionID const &TranID)
{
   INFO("Freed Workspace");
   Post();
}

void CMySPLClient::OnWorkspaceFreeFailed(const IEvent & )
{
   m_bIsOK = false;
   ERR("Workspace Free Failed");
   Post();
}

void CMySPLClient::OnTransactionStarted(TransactionID const &TranID,
                                        btVirtAddr           AFUDSMVirt,
                                        btWSSize             AFUDSMSize)
{
   INFO("Transaction Started");
   AFUDSM(AFUDSMVirt, AFUDSMSize);
   Post();
}

void CMySPLClient::OnContextWorkspaceSet(TransactionID const &TranID)
{
   INFO("Context Set");
   Post();
}

void CMySPLClient::OnTransactionFailed(const IEvent & )
{
   m_bIsOK = false;
   ERR("Transaction Failed");
   Post();
}

void CMySPLClient::OnTransactionComplete(TransactionID const &TranID)
{
   INFO("Transaction Complete");
   Post();
}

void CMySPLClient::OnTransactionStopped(TransactionID const &TranID)
{
   INFO("Transaction Stopped");
   Post();
}

void CMySPLClient::Wait() { m_Sem.Wait();  }

void CMySPLClient::Post() { m_Sem.Post(1); }

////////////////////////////////////////////////////////////////////////////////
// CMyApp
#define DEFAULT_TARGET_AFU SPLAFU_NVS_VAL_TARGET_FPGA

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyApp CMysplappApp

/// @brief splapp-specific instantiation of ISingleAFUApp that provides access to an ISPLAFU.
class CMyApp : public ISingleAFUApp<ISPLAFU>
{
public:
   CMyApp();

   /// @brief Checks the status of the app object and the ISPLClient.
   btBool IsOK() const { return m_bIsOK && m_SPLClient.IsOK(); }

   // <ISingleAFUApp>
   virtual void OnRuntimeStarted(IRuntime *,
                                 const NamedValueSet &);
   virtual void OnRuntimeStopped(IRuntime *);
   virtual void OnRuntimeStartFailed(const IEvent &);
   virtual void OnRuntimeAllocateServiceFailed(IEvent const &);
   virtual void OnRuntimeAllocateServiceSucceeded(IBase * ,
                                                  TransactionID const & );
   virtual void OnRuntimeEvent(const IEvent &);

   virtual void OnServiceAllocated(IBase *,
                                   TransactionID const &);
   virtual void OnServiceAllocateFailed(const IEvent &);
   virtual void OnServiceReleased(TransactionID const &);
   virtual void OnServiceReleaseFailed(const IEvent &e);
   virtual void OnServiceEvent(const IEvent &);
   // </ISingleAFUApp>

   /// @brief Mutator for setting the NVS value that selects the AFU Delegate.
   void AFUTarget(const std::string &target) { m_AFUTarget = target;  }
   /// @brief Accessor for the NVS value that selects the AFU Delegate.
   std::string AFUTarget() const             { return m_AFUTarget;    }

   /// @brief Mutator for setting the AFU Context workspace size.
   void WSRequestLen(btWSSize len)           { m_WSRequestLen = len;  }
   /// @brief Accessor for the AFU Context workspace size.
   btWSSize WSRequestLen() const             { return m_WSRequestLen; }

   /// Describes the arrangement of source and destination buffers in the AFU Context.
   enum BufLayout
   {  // VAFU2 Context is always at the start of the workspace.
      CNTXT_SRC_DEST   = 0,              ///< source buffer appears before the destination buffer.
      BUF_LAYOUT_FIRST = CNTXT_SRC_DEST,
      CNTXT_DEST_SRC,                    ///< destination buffer appears before the source buffer.
      BUF_LAYOUT_LAST
   };

   /// @brief Mutator for setting the AFU Context buffer layout.
   void BufferLayout(BufLayout layout)       { m_BufferLayout = layout; }
   /// @brief Accessor for the AFU Context buffer layout.
   BufLayout BufferLayout() const            { return m_BufferLayout;   }

   /// Selects the type of buffer initialization pattern for the source buffer.
   enum BufInit
   {
      FIXED          = 0,     ///< source buffer initialized with a fixed data pattern. (one 32-bit constant)
      BUF_INIT_FIRST = FIXED,
      RAND,                   ///< source buffer initialized with a random data pattern.
      BUF_INIT_LAST
   };

   /// @brief Mutator for setting the source buffer initialization pattern.
   void BufferInit(BufInit pattern)              { m_BufferInit = pattern; }
   /// @brief Accessor for the source buffer initialization pattern.
   BufInit BufferInit() const                    { return m_BufferInit;    }
   /// @brief Mutator for setting the random seed value when initializing with a random pattern.
   void BufferInitSeed(btUnsigned32bitInt s)     { m_InitSeed = s;         }
   /// @brief Accessor for the random seed value.
   btUnsigned32bitInt BufferInitSeed() const     { return m_InitSeed;      }
   /// @brief Mutator for saving a stashed copy of the random seed value.
   void SaveBufferInitSeed(btUnsigned32bitInt s) { m_SaveInitSeed = s;     }
   /// @brief Accessor for the last stashed copy of the random seed value.
   btUnsigned32bitInt SaveBufferInitSeed() const { return m_SaveInitSeed;  }

   /// @brief Mutator for setting the SPL driver poll rate.
   /// @deprecated
   void SPLPollRate(btTime pollrate)         { m_SPLPollRate = pollrate; }
   /// @brief Accessor for the SPL driver poll rate.
   /// @deprecated
   btTime SPLPollRate() const                { return m_SPLPollRate;     }

   /// Select modes of interacting with ISPLAFU.
   enum TskMode
   {
      ONE_SHOT        = 0, ///< Provide the AFU Context in the call to ISPLAFU::StartTransactionContext. This will start the transaction.
      TASK_MODE_FIRST = ONE_SHOT,
      SPLIT,               ///< Call ISPLAFU::StartTransactionContext with NULL, then call ISPLAFU::SetContextWorkspace to start the transaction.
      TASK_MODE_LAST
   };

   /// @brief Mutator for setting the SPL interaction mode.
   void TaskMode(TskMode mode)     { m_TaskMode = mode;   }
   /// @brief Accessor for the SPL interaction mode.
   TskMode TaskMode() const        { return m_TaskMode;   }

   /// @brief Mutator for choosing between a rudimentary test configuration (false) and an exhaustive test configuration (true).
   void DeepScrub(btBool b)        { m_bDeepScrub = b;    }
   /// @brief Accessor for rudimentary vs. exhaustive test configurations.
   btBool DeepScrub() const        { return m_bDeepScrub; }

   btVirtAddr OneLargeVirt() const { return m_SPLClient.OneLargeVirt(); } ///< Accessor for the AFU Context workspace.
   btPhysAddr OneLargePhys() const { return m_SPLClient.OneLargePhys(); } ///< Accessor for the AFU Context workspace.
   btWSSize   OneLargeSize() const { return m_SPLClient.OneLargeSize(); } ///< Accessor for the AFU Context workspace.

   btVirtAddr AFUDSMVirt()   const { return m_SPLClient.AFUDSMVirt(); } ///< Accessor for the AFU DSM workspace.
   btWSSize   AFUDSMSize()   const { return m_SPLClient.AFUDSMSize(); } ///< Accessor for the AFU DSM workspace.

   /// @brief Wait on m_SPLClient's internal semaphore.
   void SPLClientWait()            { m_SPLClient.Wait(); }

   /// @brief Storage for the thread-safe random number generator.
   btUnsigned32bitInt m_RandScratchpad;

protected:
   std::string        m_AFUTarget;    ///< The NVS value used to select the AFU Delegate (FPGA, ASE, or SWSim).
   btWSSize           m_WSRequestLen; ///< Requested size of the AFU Context workspace in bytes.
   BufLayout          m_BufferLayout; ///< The arrangement of buffers within the AFU Context workspace.
   BufInit            m_BufferInit;   ///< The type of buffer initialization requested.
   btUnsigned32bitInt m_InitSeed;     ///< Used only for RAND buffer init pattern.
   btUnsigned32bitInt m_SaveInitSeed; ///< Space to save a stashed copy of the random seed value.
   btTime             m_SPLPollRate;  ///< @deprecated Poll rate for SPL transactions.
   TskMode            m_TaskMode;     ///< Whether ISPLAFU::StartTransactionContext receives a non-NULL AFU Context.
   btBool             m_bDeepScrub;   ///< Whether to run permutation testing or basic testing.
   CMySPLClient       m_SPLClient;    ///< The ISPLClient used to communicate with the allocated Service.
};

CMyApp::CMyApp() :
   m_RandScratchpad(0),
   m_AFUTarget(DEFAULT_TARGET_AFU),
   m_WSRequestLen(0),
   m_BufferLayout(CNTXT_SRC_DEST),
   m_BufferInit(FIXED),
   m_InitSeed(0),
   m_SaveInitSeed(0),
   m_SPLPollRate(0),
   m_TaskMode(ONE_SHOT),
   m_bDeepScrub(false)
{
   SetInterface(iidSPLClient, dynamic_cast<ISPLClient *>(&m_SPLClient));
}

void CMyApp::OnRuntimeStarted(IRuntime            *pRT,
                              const NamedValueSet &Args)
{
   btcString AFUName = "SPLAFU";

   INFO("Allocating " << AFUName << " Service");

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.

   NamedValueSet Manifest(SPLAFU_MANIFEST);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, AFUName);
   Manifest.Add(SPLAFU_NVS_KEY_TARGET, AFUTarget().c_str());

#if DBG_HOOK
   INFO(Manifest);
#endif // DBG_HOOK

   pRT->allocService(dynamic_cast<IBase *>(this), Manifest);
}

void CMyApp::OnRuntimeStopped(IRuntime *pRT)
{
   INFO("Runtime Stopped");
}

void CMyApp::OnRuntimeStartFailed(const IEvent &e)
{
   m_bIsOK = false;
   ERR("Runtime Start Failed");
}

void CMyApp::OnRuntimeAllocateServiceFailed(IEvent const &e)
{
   m_bIsOK = false;
   ERR("Service Allocate Failed (rt)");
   Post();
}

void CMyApp::OnRuntimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                               TransactionID const &tid)
{
   INFO("Service Allocated (rt)");
}

void CMyApp::OnRuntimeEvent(const IEvent &e)
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

void CMyApp::OnServiceAllocated(IBase               *pServiceBase,
                                TransactionID const &tid)
{
   INFO("Service Allocated");
}

void CMyApp::OnServiceAllocateFailed(const IEvent &e)
{
   m_bIsOK = false;
   ERR("Service Allocate Failed");
   Post();
}

void CMyApp::OnServiceReleased(TransactionID const &tid)
{
   INFO("Service Freed");
}

void CMyApp::OnServiceReleaseFailed(const IEvent &e)
{
   m_bIsOK = false;
   INFO("Service Release Start Failed");
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
       PrintExceptionDescription(e);
       m_bIsOK = false;
       Post();
       return;
    }
}
void CMyApp::OnServiceEvent(const IEvent &e)
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

/// @}

////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS

struct CMyCmdLine
{
   btUIntPtr          flags;
#define MY_CMD_FLAG_HELP      0x00000001
#define MY_CMD_FLAG_VERSION   0x00000002
#define MY_CMD_FLAG_DEEPSCRUB 0x00000004
#define MY_CMD_FLAG_SEED      0x00000008

   std::string        AFUTarget;
   btInt              LogLevel;
   btUnsigned32bitInt Seed;
};

struct CMyCmdLine gMyCmdLine =
{
   0,
   std::string(DEFAULT_TARGET_AFU),
   LOG_INFO,
   0
};

int my_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int my_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only my_nix_long_option_only = { my_on_nix_long_option_only, };
aalclp_option      my_nix_long_option      = { my_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "splapp",
                             "0.0.0",
                             "",
                             help_msg_callback,
                             &gMyCmdLine)

int parsecmds(struct CMyCmdLine * , int , char *[] );
int verifycmds(struct CMyCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

btInt RunTest(CMyApp * );
btInt BasicHardwareCheckout(CMyApp * );
btInt HardwareDeepScrub(CMyApp * );
btInt SPLTest(CMyApp * );

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   btInt res = 0;

   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( parsecmds(&gMyCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gMyCmdLine.flags, MY_CMD_FLAG_HELP|MY_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( verifycmds(&gMyCmdLine) ) {
      return 3;
   }

   cerr << "=====================" << endl;
   cerr << " AAL SPL VAFU Sample"  << endl;
   cerr << "=====================" << endl << endl;

#if DBG_HOOK
   cout << "Waiting for debugger attach.." << endl;
   while ( gWaitForDebuggerAttach ) {
      SleepSec(1);
   }
   // Init the AAL logger.
   pAALLogger()->AddToMask(LM_All, 8); // All subsystems
   pAALLogger()->SetDestination(ILogger::CERR);
#else
   if ( gMyCmdLine.LogLevel > 0 ) {
      pAALLogger()->AddToMask(LM_All, gMyCmdLine.LogLevel);
      pAALLogger()->SetDestination(ILogger::CERR);
   }
#endif // DBG_HOOK

   CMyApp        myapp;
   NamedValueSet args;
   Runtime       aal(&myapp);

   myapp.AFUTarget(gMyCmdLine.AFUTarget);
   myapp.DeepScrub(flag_is_set(gMyCmdLine.flags, MY_CMD_FLAG_DEEPSCRUB) ? true : false);
   myapp.BufferInitSeed(gMyCmdLine.Seed);

   if ( (0 == myapp.AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_ASE)) ||
        (0 == myapp.AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_SWSIM)) ) {
      args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);
   } else {
      NamedValueSet ConfigRecord;
      ConfigRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
      args.Add(AALRUNTIME_CONFIG_RECORD, &ConfigRecord);
   }

   INFO("Starting the AAL Runtime");
   if ( aal.start(args) ) {
      myapp.Wait(); // For service allocated notification.
   } else {
      ERR("AAL Runtime start failed");
      return 4;     // Runtime could not start
   }

   if ( !myapp.IsOK() ) {
      // runtime start failed.
      ERR("SPL Service could not be allocated. Aborting.");
      myapp.Stop();
      return 5;
   }

   res += RunTest(&myapp);

   INFO("Stopping the AAL Runtime");
   myapp.Stop();

   if ( res > 0 ) {
      ERR("Test FAILED with " << res << " error" << ((res > 1) ? "s." : "."));
   } else {
      INFO("Test PASSED");
   }

   return res;
}


btInt RunTest(CMyApp *p)
{
   ASSERT(NULL != (IAALService *) *p);

   p->m_RandScratchpad = p->BufferInitSeed();

   if ( p->DeepScrub() ) {
      return HardwareDeepScrub(p);
   } else {
      return BasicHardwareCheckout(p);
   }
}


btInt BasicHardwareCheckout(CMyApp *p)
{
   p->WSRequestLen(CL(4));
   p->BufferLayout(CMyApp::CNTXT_SRC_DEST);
   p->BufferInit(CMyApp::FIXED);
   p->SPLPollRate(50); // milliseconds
   p->TaskMode(CMyApp::ONE_SHOT);
   return SPLTest(p);
}


btInt HardwareDeepScrub(CMyApp *p)
{
   // Permutation testing
   btInt             res = 0;

   CMyApp::BufLayout bl;
   CMyApp::BufInit   bi;
   CMyApp::TskMode   tm;
   btWSSize          WSSizes[] =
   {
      CL(4),
      CL(6),
      CL(8),
      KB(1),   KB(1)   + CL(2),
      KB(2),   KB(2)   + CL(2),
      KB(4),   KB(4)   + CL(2),
      KB(8),   KB(8)   + CL(2),
      KB(16),  KB(16)  + CL(2),
      KB(32),  KB(32)  + CL(2),
      KB(64),  KB(64)  + CL(2),
      KB(128), KB(128) + CL(2),
      KB(256), KB(256) + CL(2),
      KB(512), KB(512) + CL(2),
      MB(1),   MB(1)   + CL(2),
      MB(2),   MB(2)   + CL(2),
      MB(4),   MB(4)   + CL(2),
      MB(8),   MB(8)   + CL(2),
      MB(16),  MB(16)  + CL(2),
      MB(32),  MB(32)  + CL(2),
      MB(64),  MB(64)  + CL(2),
      MB(128), MB(128) + CL(2),
      MB(256), MB(256) + CL(2),
      MB(512), MB(512) + CL(2),
      GB(1),   GB(1)   + CL(2),
      GB(2),
/*
      GB(4),
      GB(8),
      GB(16),
      GB(32),
      GB(64),
      GB(128),
      GB(256),
      GB(512)
*/
   };
   btUnsignedInt wss;
   btTime PollRates[] =
   {  // All are in milliseconds
      25,
      50,
      100,
      250
   };
   btUnsignedInt pr;

   for ( bl = CMyApp::BUF_LAYOUT_FIRST ;
            bl < CMyApp::BUF_LAYOUT_LAST ;
               bl = (CMyApp::BufLayout)((int)bl + 1) ) {

      for ( bi = CMyApp::BUF_INIT_FIRST ;
               bi < CMyApp::BUF_INIT_LAST ;
                  bi = (CMyApp::BufInit)((int)bi + 1) ) {

         for ( tm = CMyApp::TASK_MODE_FIRST ;
                  tm < CMyApp::TASK_MODE_LAST ;
                     tm = (CMyApp::TskMode)((int)tm + 1) ) {

            for ( pr = 0 ;
                     pr < sizeof(PollRates) / sizeof(PollRates[0]) ;
                        ++pr ) {

               for ( wss = 0 ;
                        wss < sizeof(WSSizes) / sizeof(WSSizes[0]) ;
                           ++wss ) {

                  p->WSRequestLen(WSSizes[wss]);
                  p->BufferLayout(bl);
                  p->BufferInit(bi);
                  p->SPLPollRate(PollRates[pr]);
                  p->TaskMode(tm);

                  res += SPLTest(p);
                  if ( res > 0 ) {
                     ERR("Error when.. BufferLayout=" << bl <<
                         " BufferInit="               << bi <<
                         " Seed="                     << p->BufferInitSeed() <<
                         " TaskMode="                 << tm <<
                         " Poll Rate="                << PollRates[pr] <<
                         " Workspace Size="           << WSSizes[wss]);
                     break;
                  }

               }
            }
         }
      }
   }

   return res;
}

/// @addtogroup splapp
/// @{

/// @brief Executes one iteration of an SPL VAFU test on the Service supplied within
///        the given CMyApp.
btInt SPLTest(CMyApp *p)
{
   ISPLAFU *pAFU = (ISPLAFU *) *p; // uses type cast operator from ISingleAFUApp.
   ASSERT(NULL != pAFU);

   // Allocate a single contiguous workspace for the VAFU2 Context and src/dest buffers.
   pAFU->WorkspaceAllocate(p->WSRequestLen(), TransactionID());

   // Synchronize with the workspace allocation event notification.
   p->SPLClientWait();

   // Check the status.
   if ( !p->IsOK() ) {
      ERR("IsOK[0] check failed");
      return 1;
   }

   btVirtAddr     pWSUsrVirt = p->OneLargeVirt();
   btPhysAddr     WSPhys     = p->OneLargePhys();
   const btWSSize WSLen      = p->OneLargeSize();

   INFO("Allocated " << WSLen << "-byte Workspace.");
   INFO("   user virt: "   << std::hex << (void *)pWSUsrVirt);
   INFO("        phys: 0x" << std::hex << WSPhys << std::dec);

   // Note: the usage of the VAFU2_CNTXT structure here is specific to the underlying bitstream
   // implementation. The bitstream targeted for use with this sample application must implement
   // the Validation AFU 2 interface and abide by the contract that a VAFU2_CNTXT structure will
   // appear at byte offset 0 within the supplied AFU Context workspace.
   VAFU2_CNTXT *pVAFU2_cntxt = reinterpret_cast<VAFU2_CNTXT *>(pWSUsrVirt);
   ::memset(pVAFU2_cntxt, 0, sizeof(VAFU2_CNTXT));

   switch ( p->BufferLayout() ) {

      case CMyApp::CNTXT_SRC_DEST : {
         // The source buffer is at Cacheline 2 (from 0).
         pVAFU2_cntxt->pSource = pWSUsrVirt + CL(2);
         // The source and destination buffer are the same size.
         pVAFU2_cntxt->pDest   = pWSUsrVirt + CL(2) + ((WSLen - CL(2)) / 2);

         INFO("VAFU2 Context=" << std::hex << (void *)pVAFU2_cntxt <<
              " Src="  << std::hex << (void *)pVAFU2_cntxt->pSource <<
              " Dest=" << std::hex << (void *)pVAFU2_cntxt->pDest << std::dec);
      } break;

      case CMyApp::CNTXT_DEST_SRC : {
         // The dest buffer is at Cacheline 2 (from 0).
         pVAFU2_cntxt->pDest   = pWSUsrVirt + CL(2);
         // The source and destination buffer are the same size.
         pVAFU2_cntxt->pSource = pWSUsrVirt + CL(2) + ((WSLen - CL(2)) / 2);

         INFO("VAFU2 Context=" << std::hex << (void *)pVAFU2_cntxt <<
              " Dest=" << std::hex << (void *)pVAFU2_cntxt->pDest <<
              " Src="  << std::hex << (void *)pVAFU2_cntxt->pSource << std::dec);
      } break;

      default : {
         ERR("Invalid buffer layout: " << p->BufferLayout());
         ASSERT(false);
         return 1;
      }
   }

   pVAFU2_cntxt->num_cl = (btUnsigned32bitInt) (((WSLen - CL(2)) / 2) / CL(1));
   INFO(" Cache lines=" << std::dec << pVAFU2_cntxt->num_cl <<
        " (bytes="      << std::dec << pVAFU2_cntxt->num_cl * CL(1) <<
        " 0x"           << std::hex << pVAFU2_cntxt->num_cl * CL(1) << std::dec << ")");

   btUnsigned32bitIntArray    pu32;
   btUnsigned32bitIntArray pEndu32;

   // Init the src/dest buffers, based on the desired sequence (either fixed or random).
   if ( CMyApp::FIXED == p->BufferInit() ) {
      INFO("Initializing buffers with fixed data pattern. (src=0xafafafaf dest=0xbebebebe)");

      pu32    = reinterpret_cast<btUnsigned32bitIntArray>(pVAFU2_cntxt->pSource);
      pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (pVAFU2_cntxt->num_cl * CACHELINE_BYTES));

      while ( pu32 < pEndu32 ) {
         *pu32 = 0xafafafaf;
         ++pu32;
      }

      pu32    = reinterpret_cast<btUnsigned32bitIntArray>(pVAFU2_cntxt->pDest);
      pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (pVAFU2_cntxt->num_cl * CACHELINE_BYTES));

      while ( pu32 < pEndu32 ) {
         *pu32 = 0xbebebebe;
         ++pu32;
      }

   } else if ( CMyApp::RAND == p->BufferInit() ) {
      p->SaveBufferInitSeed(p->m_RandScratchpad);

      INFO("Initializing src buffer with random data pattern (seed=0x" <<
           std::hex << p->m_RandScratchpad << std::dec << "), and dest buffer with 0.");

      pu32    = reinterpret_cast<btUnsigned32bitIntArray>(pVAFU2_cntxt->pSource);
      pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (pVAFU2_cntxt->num_cl * CACHELINE_BYTES));

      while ( pu32 < pEndu32 ) {
         *pu32 = GetRand(&p->m_RandScratchpad);
         ++pu32;
      }

      ::memset((void *)pVAFU2_cntxt->pDest, 0, pVAFU2_cntxt->num_cl * CACHELINE_BYTES);

   } else {
      ERR("Invalid buffer init: " << p->BufferInit());
      ASSERT(false);
      return 1;
   }

   volatile VAFU2_DSM *pAFUDSM;

   // Start the SPL task, or init the session, then start the task.
   if ( CMyApp::ONE_SHOT == p->TaskMode() ) {
      INFO("Starting SPL Transaction with Workspace");

      // Provide a workspace and so also start the transaction.
      // The VAFU2 Context is assumed to be at the start of the workspace.
      pAFU->StartTransactionContext(TransactionID(), pWSUsrVirt, p->SPLPollRate());
      // Wait for the StartTransactionContext response.
      p->SPLClientWait();

      // Check the status.
      if ( !p->IsOK() ) {
         ERR("IsOK[1] check failed");
         return 1;
      }

      // Examine the AFU ID from the AFU DSM pointer returned in the response event.
      pAFUDSM = (volatile VAFU2_DSM *)p->AFUDSMVirt();
      ASSERT(pAFUDSM != NULL);

      INFO("AFU ID = 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[1] <<
           " 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[0] << std::dec);

   } else if ( CMyApp::SPLIT == p->TaskMode() ) {
      INFO("Starting SPL Transaction without Workspace");

      // No workspace, so transaction not started.
      pAFU->StartTransactionContext(TransactionID());

      // Wait for the StartTransactionContext response.
      p->SPLClientWait();

      // Check the status.
      if ( !p->IsOK() ) {
         ERR("IsOK[2] check failed");
         return 1;
      }

      // Examine the AFU ID from the AFU DSM pointer returned in the response event.
      pAFUDSM = (volatile VAFU2_DSM *)p->AFUDSMVirt();
      ASSERT(pAFUDSM != NULL);

      INFO("AFU ID = 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[1] <<
           " 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[0] << std::dec);

      // Verify that the CSR write region was mapped properly.
      // To do so, we..
      // 1) clear AFU_DSM_SCRATCH to zero.
      // 2) write some non-zero value to AFU_CSR_SCRATCH.
      // 3) verify that the value is reflected at AFU_DSM_SCRATCH.

      INFO("(before clearing) AFU_DSM_SCRATCH is 0x" <<
           std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);

      pAFUDSM->AFU_DSM_SCRATCH = 0;
      INFO("(after clearing) AFU_DSM_SCRATCH is 0x" << std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);

      btBool bRes = pAFU->CSRWrite(byte_offset_AFU_CSR_SCRATCH / 4, 0xdecafbad);
      ASSERT(bRes);
      INFO("CSRWrite() returned " << bRes);

      INFO("(when checking) AFU_DSM_SCRATCH is 0x" << std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);
      ASSERT(0xdecafbad == pAFUDSM->AFU_DSM_SCRATCH);


      INFO("Setting SPL Context Workspace");
      pAFU->SetContextWorkspace(TransactionID(), pWSUsrVirt, p->SPLPollRate());

      // Wait for the SetContextWorkspace response.
      p->SPLClientWait();

      // Check the status.
      if ( !p->IsOK() ) {
         ERR("IsOK[3] check failed");
         return 1;
      }

      INFO("Workspace set, SPL Transaction started");

   } else {
      ERR("Invalid task mode: " << p->TaskMode());
      ASSERT(false);
      return 1;
   }

#ifdef OLD_SPL_PROTOCOL
   // Wait for the TransactionContextComplete response.
   p->SPLClientWait();
#else
   ////////////////////////////////////////////////////////////////////////////
   // New SPL Protocol

   // Set timeout increment based on hardware, software, or simulation
   bt32bitInt count(200);  // 20 seconds with 100 millisecond sleep
   bt32bitInt delay(100);  // 100 milliseconds is the default
   if ( 0 == p->AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_ASE) ) delay = 5000; // milliseconds

   // Wait for SPL VAFU to finish code
   volatile bt32bitInt done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
   while (!done && --count) {
      SleepMilli( delay );
      done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
   }

   if ( !done ) {
      // must have dropped out of loop due to count -- never saw update
      ERR("Timed out waiting for SPL task done.");
      return 1;
   }

   // Print performance counters
   INFO("AFU_DSM_LATENCY     = 0x" <<
           std::hex << std::setw(8) << std::setfill('0') <<
           (btUnsigned32bitInt) (pAFUDSM->AFU_DSM_LATENCY >> 32) <<
           " 0x" << std::hex << std::setw(8) << std::setfill('0') <<
           (btUnsigned32bitInt) (pAFUDSM->AFU_DSM_LATENCY) <<
           std::dec << std::setfill(' '));

   INFO("AFU_DSM_PERFORMANCE = 0x" <<
           std::hex << std::setw(8) << std::setfill('0') <<
           (btUnsigned32bitInt) (pAFUDSM->AFU_DSM_PERFORMANCE >> 32) <<
           " 0x" << std::hex << std::setw(8) << std::setfill('0') <<
           (btUnsigned32bitInt) (pAFUDSM->AFU_DSM_PERFORMANCE) <<
           std::dec << std::setfill(' '));

   // Issue Stop Transaction and wait for OnTransactionStopped
   pAFU->StopTransactionContext(TransactionID());
   p->SPLClientWait();
#endif // OLD_SPL_PROTOCOL

   // Check the status.
   if ( !p->IsOK() ) {
      ERR("IsOK[4] check failed");
      return 1;
   }

   INFO("SPL Transaction complete");

   btUnsignedInt      cl;
   btBool             bCLOK;
   btInt              res = 0;
   std::ostringstream oss;

   INFO("Verifying buffers");

   // Verify 1) that the source buffer was not corrupted and
   //        2) that the dest buffer contains the source buffer contents.
   if ( CMyApp::FIXED == p->BufferInit() ) {

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CACHELINE_BYTES));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CACHELINE_BYTES);

         bCLOK = true;

         while ( pu32 < pEndu32 ) {
            if ( 0xafafafaf != *pu32 ) {
               bCLOK = false;
            }
            ++pu32;
         }

         if ( !bCLOK ) {
            ++res;
            ERR("Source cache line " << cl << " has been corrupted.");

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CACHELINE_BYTES));
            while ( pu32 < pEndu32 ) {
               oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << *pu32;
               if ( 0xafafafaf != *pu32 ) {
                  oss << '*';
               } else {
                  oss << ' ';
               }
               oss << ' ';
               ++pu32;
            }

            ERR(oss.str());
            oss.str(std::string(""));
         }
      }

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CACHELINE_BYTES));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CACHELINE_BYTES);

         bCLOK = true;

         while ( pu32 < pEndu32 ) {
            if ( 0xafafafaf != *pu32 ) {
               bCLOK = false;
            }
            ++pu32;
         }

         if ( !bCLOK ) {
            ++res;
            ERR("Dest cache line " << cl << " is incorrect.");

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CACHELINE_BYTES));
            while ( pu32 < pEndu32 ) {
               oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << *pu32;
               if ( 0xafafafaf != *pu32 ) {
                  oss << '*';
               } else {
                  oss << ' ';
               }
               oss << ' ';
               ++pu32;
            }

            ERR(oss.str());
            oss.str(std::string(""));
         }
      }

   } else if ( CMyApp::RAND == p->BufferInit() ) {
      p->m_RandScratchpad = p->SaveBufferInitSeed();

      btUnsigned32bitInt CLSeed;
      btUnsigned32bitInt RandNum;

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CACHELINE_BYTES));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CACHELINE_BYTES);

         bCLOK  = true;
         CLSeed = p->m_RandScratchpad;

         while ( pu32 < pEndu32 ) {
            if ( GetRand(&p->m_RandScratchpad) != *pu32 ) {
               bCLOK = false;
            }
            ++pu32;
         }

         if ( !bCLOK ) {
            ++res;
            ERR("Source cache line " << cl << " has been corrupted.");

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CACHELINE_BYTES));
            while ( pu32 < pEndu32 ) {
               oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << *pu32;

               RandNum = GetRand(&CLSeed);

               if ( RandNum != *pu32 ) {
                  oss << "/0x" << std::hex << std::setw(8) << std::setfill('0') << RandNum;
               } else {
                  oss << ' ';
               }
               oss << ' ';
               ++pu32;
            }

            ERR(oss.str());
            oss.str(std::string(""));
         }
      }

      p->m_RandScratchpad = p->SaveBufferInitSeed();

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CACHELINE_BYTES));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CACHELINE_BYTES);

         bCLOK  = true;
         CLSeed = p->m_RandScratchpad;

         while ( pu32 < pEndu32 ) {
            if ( GetRand(&p->m_RandScratchpad) != *pu32 ) {
               bCLOK = false;
            }
            ++pu32;
         }

         if ( !bCLOK ) {
            ++res;
            ERR("Dest cache line " << cl << " is incorrect.");

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CACHELINE_BYTES));
            while ( pu32 < pEndu32 ) {
               oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << *pu32;

               RandNum = GetRand(&CLSeed);

               if ( RandNum != *pu32 ) {
                  oss << "/0x" << std::hex << std::setw(8) << std::setfill('0') << RandNum;
               } else {
                  oss << ' ';
               }
               oss << ' ';
               ++pu32;
            }

            ERR(oss.str());
            oss.str(std::string(""));
         }
      }
   }

   INFO("Buffer verification complete.");

   pAFU->WorkspaceFree(pWSUsrVirt, TransactionID());
   p->SPLClientWait(); // for workspace freed notification.

   // Check the status.
   if ( !p->IsOK() ) {
      ERR("IsOK[5] check failed");
      return 1;
   }

   return 0;
}

/// @}

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
   } else if ( 0 == strcmp("--deep-scrub", option) ) {
      flag_setf(cl->flags, MY_CMD_FLAG_DEEPSCRUB);
   }

   return 0;
}

int my_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct CMyCmdLine *cl     = (struct CMyCmdLine *)user;
   char              *endptr = NULL;

   if ( 0 == strcmp("--target", option) ) {
      if ( 0 == strcasecmp("fpga", value) ) {
         cl->AFUTarget = std::string(SPLAFU_NVS_VAL_TARGET_FPGA);
      } else if ( 0 == strcasecmp("ase", value) ) {
         cl->AFUTarget = std::string(SPLAFU_NVS_VAL_TARGET_ASE);
      } else if ( 0 == strcasecmp("swsim", value) ) {
         cl->AFUTarget = std::string(SPLAFU_NVS_VAL_TARGET_SWSIM);
      } else {
         cout << "Invalid value for --target : " << value << endl;
         return 4;
      }
   } else if ( 0 == strcmp("--log", option) ) {
      cl->LogLevel = (btInt)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->LogLevel = 0;
      } else if ( cl->LogLevel < 0) {
         cl->LogLevel = 0;
      } else if ( cl->LogLevel > 8) {
         cl->LogLevel = 8;
      }
   } else if ( 0 == strcmp("--seed", option) ) {
      cl->Seed = (btUnsigned32bitInt)strtoul(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cout << "Invalid value for --seed : " << value << endl;
         return 5;
      }
   }
   return 0;
}

void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   splapp [--target=<TARGET>] [--seed=<SEED>] [--deep-scrub]\n");
   fprintf(fp, "\n");
   fprintf(fp, "      <TARGET>     = one of { fpga ase swsim }\n");
   fprintf(fp, "      <SEED>       = 32 bit seed for RNG / fixed pattern\n");
   fprintf(fp, "      --deep-scrub = run thorough permutation testing (default is basic checkout)\n");
   fprintf(fp, "\n");
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
@addtogroup splapp
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>An ISPLClient implementation.</li>
  <li>Use of the ISingleAFUApp class template.</li>
  <li>Runtime selection of AFU targets with SPLAFU.</li>
  <li>SPL transactions with ISPLAFU.</li>
</ul>

This sample is designed to be used with SPLAFU.

1 Summary of Operation

splapp relies on its instantiation of ISingleAFUApp inherited by CMyApp to
perform the brunt of the XL runtime interaction. An XL Runtime object is declared
in main() to handle the runtime startup and shutdown.

An object instance of CMyApp is declared in main(), and the command line parameters
are used to select the AFU target implementation, whether to run permutation testing,
and to save the requested random seed value, if any, for random source buffer initialization.

Some AFU-specific Runtime parameter configuration is performed prior to passing the
CMyApp instance to the Runtime's start routine. When the override of ISingleAFUApp::OnRuntimeStarted
is called, an instance of SPLAFU is requested.

The service allocation request is satisfied by ISingleAFUApp::serviceAllocated, which
pulses the internal semaphore, waking the thread blocked in main(). Once awake, some basic
error checking is done on the CMyApp instance and the SPL test is performed.

When the SPL test is complete, ISingleAFUApp::Stop is called to release the AFU and to
wait for the release notification from the runtime.

Finally, the SPL test status is reported, and the application exits.

2 Running the application

2.0 Online Help

@code
$ splapp --help
Usage:
   splapp [--target=<TARGET>] [--seed=<SEED>] [--deep-scrub]

      <TARGET>     = one of { fpga ase swsim }
      <SEED>       = 32 bit seed for RNG / fixed pattern
      --deep-scrub = run thorough permutation testing (default is basic checkout)@endcode

2.1 SPL FPGA (HWSPLAFU)

Prerequisites for running the sample with an FPGA:
<ul>
  <li>The SPL AAL device drivers must be loaded.</li>
  <li>The AAL Resource Manager must be running.</li>
  <li>The FPGA module connected to the system must be programmed with an appropriate SPL AFU bit stream.</li>
</ul>

@code
$ splapp --target=fpga@endcode

2.2 SPL AFU Simulation Environment (ASESPLAFU)

Prerequisites for running the sample with ASE:
<ul>
  <li>The ASE simulation-side application must be running on the system.</li>
</ul>

@code
$ splapp --target=ase@endcode

2.3 SPL Software Simulation (SWSimSPLAFU)

Prerequisites for running the sample with Software Simulation:
<ul>
  <li>(none)
</ul>

@code
$ splapp --target=swsim@endcode

@}
*/

