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
/// @file splapp2.cpp
/// @brief Uses XL and ISPLAFU to interact with SPL.
/// @ingroup splapp2
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
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:         WHO:  WHAT:
/// 10/12/2014    HM    Initial version copied from splapp.
/// 10/14/2014    HM    Added new SPL protocol for StopTransaction@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <cstring>                        // memcmp
#include <aalsdk/utils/Utilities.h>       // Brings in CL, MB, GB, etc.
#include <aalsdk/utils/CSyncClient.h>
#include <aalsdk/service/ISPLAFU.h>       // Service Interface
#include <aalsdk/service/ISPLClient.h>    // Service Client Interface
#include <aalsdk/kernel/vafu2defs.h>      // AFU structure definitions (brings in spl2defs.h)
#include <aalsdk/AALLoggerExtern.h>       // Logger, used by INFO and ERR macros
#include <aalsdk/aalclp/aalclp.h>         // Command-line processor
#include <aalsdk/service/SPLAFUService.h> // Service Manifest and #defines

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

/// @addtogroup splapp2
/// @{

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyApp CMysplapp2_App

////////////////////////////////////////////////////////////////////////////////
// CMyApp
#define DEFAULT_TARGET_AFU SPLAFU_NVS_VAL_TARGET_FPGA

/// @brief splapp2-specific instantiation of ISPLClient that receives the event notifications
///        sent by the ISPLAFU.
class CMyApp : public CSyncClient,        // Inherit interface and implementation of IRunTimeClien and IServiceClient
               public ISPLClient          // SPL Client Interface
{
public:
   CMyApp() :
         m_pISPLAFU(NULL),
         m_pServiceBase(NULL),
         m_OneLargeWorkspaceVirt(NULL),
         m_OneLargeWorkspaceSize(0),
         m_AFUDSMVirt(NULL),
         m_AFUDSMSize(0),
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
      SetInterface(iidSPLClient, dynamic_cast<ISPLClient *>(this));
      m_bIsOK = true;
   }
   ~CMyApp(){}
   btBool IsOK() const { return m_bIsOK && CSyncClient::IsOK(); }

   ///////////////////////////////////////////////////////////////////////////
   // <Extend CSyncClient>
   // First call to CMyApp. Start everything up and then return status.
   // If returns false, need to shut down the runtime and exit.
   bool start( const NamedValueSet &RunTimeArgs, const NamedValueSet &Manifest)
   {
      if ( !syncStart( RunTimeArgs ) ) { // CSyncClient synchronous runtime start
         ERR("Could not start Runtime.");
         return false;
      }
      m_pServiceBase = syncAllocService( Manifest );  // CSyncClient synchronous get pointer
      if ( !m_pServiceBase ) {                        //    to Service Object
         ERR("Could not allocate Service.");          // Error return possible if it cannot
         return false;                                //    be obtained
      }
      // Get pointer to SPL AFU
      m_pISPLAFU = dynamic_ptr<ISPLAFU>( iidSPLAFU, m_pServiceBase);
      ASSERT( m_pISPLAFU );
      if ( !m_pISPLAFU ) {                         // this would represent an internal logic error
         ERR( "Could not access SPL Service.");
         return false;
      }
      return m_bIsOK;
   }
   // Shutdown the RunTime Client, and therefore the RunTime itself
   void stop()
   {
      syncStop();    // use CSyncClient's synchronouse stop
   }
   // </Extend CSyncClient>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // <ISPLClient>

   /// CMyApp Client implementation of ISPLClient::OnWorkspaceAllocated
   virtual void OnWorkspaceAllocated( TransactionID const &TranID,
                                      btVirtAddr           WkspcVirt,
                                      btPhysAddr           WkspcPhys,
                                      btWSSize             WkspcSize)
   {
      OneLarge(WkspcVirt, WkspcPhys, WkspcSize);
      INFO("Got Workspace");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnWorkspaceAllocateFailed
   virtual void OnWorkspaceAllocateFailed( const IEvent &Event)
   {
      m_bIsOK = false;
      OneLarge( NULL, 0, 0);
      ERR("Workspace Allocate Failed");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnWorkspaceFreed
   virtual void OnWorkspaceFreed( TransactionID const &TranID)
   {
      OneLarge( NULL, 0, 0);
      INFO("Freed Workspace");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnWorkspaceFreeFailed
   virtual void OnWorkspaceFreeFailed( const IEvent &Event)
   {
      m_bIsOK = false;
      OneLarge( NULL, 0, 0);
      ERR("Workspace Free Failed");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnTransactionStarted
   virtual void OnTransactionStarted( TransactionID const &TranID,
                                      btVirtAddr           AFUDSMVirt,
                                      btWSSize             AFUDSMSize)
   {
      INFO("Transaction Started");
      AFUDSM(AFUDSMVirt, AFUDSMSize);
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnContextWorkspaceSet
   virtual void OnContextWorkspaceSet( TransactionID const &TranID)
   {
      INFO("Context Set");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnTransactionFailed
   virtual void OnTransactionFailed( const IEvent &Event)
   {
      m_bIsOK = false;
      AFUDSM( NULL, 0);
      ERR("Transaction Failed");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnTransactionComplete
   virtual void OnTransactionComplete( TransactionID const &TranID)
   {
      AFUDSM( NULL, 0);
      INFO("Transaction Complete");
      Post();
   }
   /// CMyApp Client implementation of ISPLClient::OnTransactionStopped
   virtual void OnTransactionStopped( TransactionID const &TranID)
   {
      AFUDSM( NULL, 0);
      INFO("Transaction Stopped");
      Post();
   }
   // </ISPLClient>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // <Synchronous versions of ISPLAFU (which includes ICCIAFU>

   /// CMyApp Client synchronous implementation of ISPLAFU::StartTransactionContext
   btBool syncStartTransactionContext(TransactionID const &TranID,
                                  btVirtAddr           Address=NULL,
                                  btTime               Pollrate=0)
   {
      m_pISPLAFU->StartTransactionContext( TranID, Address, Pollrate);
      Wait();                    // Posted in OnTransactionStarted()
      return m_bIsOK;
   }
   /// CMyApp Client synchronous implementation of ISPLAFU::StopTransactionContext
   btBool syncStopTransactionContext(TransactionID const &TranID)
   {
      m_pISPLAFU->StopTransactionContext( TranID);
      Wait();
      return m_bIsOK;
   }
   /// CMyApp Client synchronous implementation of ISPLAFU::SetContextWorkspace
   btBool syncSetContextWorkspace(TransactionID const &TranID,
                                  btVirtAddr           Address,
                                  btTime               Pollrate=0)
   {
      m_pISPLAFU->SetContextWorkspace( TranID, Address, Pollrate);
      Wait();
      return m_bIsOK;
   }
   /// CMyApp Client synchronous implementation of ISPLAFU::WorkspaceAllocate
   btBool syncWorkspaceAllocate(btWSSize             Length,
                                TransactionID const &rTranID)
   {
      m_pISPLAFU->WorkspaceAllocate( Length, rTranID);
      Wait();
      return m_bIsOK;
   }
   /// CMyApp Client synchronous implementation of ISPLAFU::WorkspaceFree
   btBool syncWorkspaceFree(btVirtAddr           Address,
                            TransactionID const &rTranID)
   {
      m_pISPLAFU->WorkspaceFree( Address, rTranID);
      Wait();
      return m_bIsOK;
   }

   // These are already synchronous, but this object is not derived from
   //    ICCIAFU, so must delegate

   /// CMyApp Client delegation of ICCIAFU::CSRRead
   btBool CSRRead(btCSROffset CSR, btCSRValue *pValue)
   {
      return m_pISPLAFU->CSRRead( CSR, pValue);
   }
   /// CMyApp Client delegation of ICCIAFU::CSRWrite
   btBool CSRWrite(btCSROffset CSR, btCSRValue Value)
   {
      return m_pISPLAFU->CSRWrite( CSR, Value);
   }
   /// CMyApp Client delegation of ICCIAFU::CSRWrite64
   btBool CSRWrite64(btCSROffset CSR, bt64bitCSR Value)
   {
      return m_pISPLAFU->CSRWrite64( CSR, Value);
   }
   // </Synchronous versions of ISPLAFU (which includes ICCIAFU>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // Accessors and Mutators

   btVirtAddr OneLargeVirt() const { return m_OneLargeWorkspaceVirt; } ///< Accessor for the AFU Context workspace.
   btWSSize   OneLargeSize() const { return m_OneLargeWorkspaceSize; } ///< Accessor for the AFU Context workspace.

   btVirtAddr AFUDSMVirt()   const { return m_AFUDSMVirt; } ///< Accessor for the AFU DSM workspace.
   btWSSize   AFUDSMSize()   const { return m_AFUDSMSize; } ///< Accessor for the AFU DSM workspace.

   /// Mutator for setting the NVS value that selects the AFU Delegate.
   void AFUTarget(const std::string &target) { m_AFUTarget = target;  }
   /// Accessor for the NVS value that selects the AFU Delegate.
   std::string AFUTarget() const             { return m_AFUTarget;    }

   /// Mutator for setting the AFU Context workspace size.
   void WSRequestLen(btWSSize len)           { m_WSRequestLen = len;  }
   /// Accessor for the AFU Context workspace size.
   btWSSize WSRequestLen() const             { return m_WSRequestLen; }

   enum BufLayout
   {  // VAFU2 Context is always at the start of the workspace.
      CNTXT_SRC_DEST   = 0,               ///< source buffer appears before the destination buffer.
      BUF_LAYOUT_FIRST = CNTXT_SRC_DEST,
      CNTXT_DEST_SRC,                     ///< destination buffer appears before the source buffer.
      BUF_LAYOUT_LAST
   };

   /// Mutator for setting the AFU Context buffer layout.
   void BufferLayout(BufLayout layout)       { m_BufferLayout = layout; }
   /// Accessor for the AFU Context buffer layout.
   BufLayout BufferLayout() const            { return m_BufferLayout;   }

   enum BufInit
   {
      FIXED          = 0,        ///< src = 0xAFAF..., dest = 0xBEBE...
      BUF_INIT_FIRST = FIXED,
      ID,                        ///< src = 0xA..dword_location_in_CL .. CL#
      RAND,                      ///< src = rand(), dest = 0
      BUF_INIT_LAST
   };

   /// Mutator for setting the source buffer initialization pattern.
   void BufferInit(BufInit pattern)              { m_BufferInit = pattern; }
   /// Accessor for the source buffer initialization pattern.
   BufInit BufferInit() const                    { return m_BufferInit;    }
   /// Mutator for setting the random seed value when initializing with a random pattern.
   void BufferInitSeed(btUnsigned32bitInt s)     { m_InitSeed = s;         }
   /// Accessor for the random seed value.
   btUnsigned32bitInt BufferInitSeed() const     { return m_InitSeed;      }
   /// Mutator for saving a stashed copy of the random seed value.
   void SaveBufferInitSeed(btUnsigned32bitInt s) { m_SaveInitSeed = s;     }
   /// Accessor for the last stashed copy of the random seed value.
   btUnsigned32bitInt SaveBufferInitSeed() const { return m_SaveInitSeed;  }

   /// Mutator for setting the SPL driver poll rate.
   /// @deprecated
   void SPLPollRate(btTime pollrate)         { m_SPLPollRate = pollrate; }
   /// Accessor for the SPL driver poll rate.
   /// @deprecated
   btTime SPLPollRate() const                { return m_SPLPollRate;     }

   /// Select modes of interacting with ISPLAFU.
   enum TskMode
   {
      ONE_SHOT        = 0, ///< Provide the workspace pointer in the call to StartTransactionContextFAP20(). This will start the transaction.
      TASK_MODE_FIRST = ONE_SHOT,
      SPLIT,               ///< Call StartTransactionContextFAP20() w/o workspace pointer, then call SetContextWorkspaceFAP20() to start the transaction.
      TASK_MODE_LAST
   };

   /// Mutator for setting the SPL interaction mode.
   void TaskMode(TskMode mode)               { m_TaskMode = mode;   }
   /// Accessor for the SPL interaction mode.
   TskMode TaskMode() const                  { return m_TaskMode;   }

   /// Mutator for choosing between a rudimentary test configuration (false) and an exhaustive test configuration (true).
   void DeepScrub(btBool b)                  { m_bDeepScrub = b;    }
   /// Accessor for rudimentary vs. exhaustive test configurations.
   btBool DeepScrub() const                  { return m_bDeepScrub; }

   btUnsigned32bitInt m_RandScratchpad;

protected:
   /// Store information about the Virtual Workspace into CMyApp
   void OneLarge(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_OneLargeWorkspaceVirt = v;
      m_OneLargeWorkspaceSize = s;
   }
   /// Store information about the DSM (Device Status Memory) into CMyApp
   void AFUDSM(btVirtAddr v, btWSSize s)
   {
      m_AFUDSMVirt = v;
      m_AFUDSMSize = s;
   }

   // Member variables
   ISPLAFU             *m_pISPLAFU;       ///< Points to the actual AFU, stored here for convenience
   IBase               *m_pServiceBase;   ///< Pointer to Service containing SPL AFU

   btVirtAddr           m_OneLargeWorkspaceVirt; ///< Points to Virtual workspace
   btWSSize             m_OneLargeWorkspaceSize; ///< Length in bytes of Virtual workspace
   btVirtAddr           m_AFUDSMVirt;            ///< Points to DSM
   btWSSize             m_AFUDSMSize;            ///< Length in bytes of DSM

   std::string          m_AFUTarget;      ///< The NVS value used to select the AFU Delegate (FPGA, ASE, or SWSim).
   btWSSize             m_WSRequestLen;   ///< Requested size of the AFU Context workspace in bytes.
   BufLayout            m_BufferLayout;   ///< The arrangement of buffers within the AFU Context workspace.
   BufInit              m_BufferInit;     ///< The type of buffer initialization requested.
   btUnsigned32bitInt   m_InitSeed;       ///< Used only for RAND buffer init pattern.
   btUnsigned32bitInt   m_SaveInitSeed;   ///< Space to save a stashed copy of the random seed value.
   btTime               m_SPLPollRate;    ///< @deprecated Poll rate for SPL transactions.
   TskMode              m_TaskMode;       ///< Whether ISPLAFU::StartTransactionContext receives a non-NULL AFU Context.
   btBool               m_bDeepScrub;     ///< Whether to run permutation testing or basic testing.
}; //CMyApp

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
                             "splapp2",
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
// Other utility functions
struct WordInBuf {            // 32 bits for BufInit type ID = Identifiable
   btUnsigned32bitInt CLine       :24; // up to 1GB of CLs
   btUnsigned32bitInt DWordPos    : 4; // 0 to 0xF
   btUnsigned32bitInt BeforeAfter : 4; // OXB(efore) or 0xA(after)
};
CASSERT( sizeof(struct WordInBuf) == sizeof(btUnsigned32bitInt) );


////////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:          main
// Description:   Entry point to the application
// Inputs:        arc, argv
// Outputs:       returns number of errors
// Comments:      Main initializes the system and tracks state as it runs
//                through the protocol.
//=============================================================================
int main(int argc, char *argv[])
{
   ////////////////////////////////////////////////////////////////////////////
   // Get the arguments

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

   cerr << "======================" << endl;
   cerr << " AAL SPL VAFU Sample 2"  << endl;
   cerr << "======================" << endl << endl;

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

   bool              bStarted;      // Tracks if the runtime and service are initialized
   CMyApp            myapp;         // SPL AFU Client. Also a CSyncClient.

   ////////////////////////////////////////////////////////////////////////////
   // Store the command line options into the myapp object for use later
   myapp.AFUTarget(gMyCmdLine.AFUTarget);
   myapp.DeepScrub(flag_is_set(gMyCmdLine.flags, MY_CMD_FLAG_DEEPSCRUB) ? true : false);
   myapp.BufferInitSeed(gMyCmdLine.Seed);

   ////////////////////////////////////////////////////////////////////////////
   // Define the startup parameters of the Runtime.
   // These are, in general, a dependent on what kinds of Services you will be
   //    loading and accessing.
   // For example, software-only services can be loaded via the default broker,
   //    but hardware-based services typically take something more, e.g. a
   //    broker that understands the underlying hardware resources.
   // THIS CODE IS SUBJECT TO CHANGE.

   NamedValueSet RunTimeArgs;          // Used to initialize the Runtim
   if ( (0 == myapp.AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_ASE)) ||
        (0 == myapp.AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_SWSIM)) ) {
      RunTimeArgs.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);
   } else {
      NamedValueSet ConfigRecord;
      ConfigRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
      RunTimeArgs.Add(AALRUNTIME_CONFIG_RECORD, &ConfigRecord);
   }

   ////////////////////////////////////////////////////////////////////////////
   // Define the Manifest, which selects which Service is to be obtained.
   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.

   btcString AFUName = "SPLAFU";
   NamedValueSet Manifest(SPLAFU_MANIFEST);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, AFUName);
   Manifest.Add(SPLAFU_NVS_KEY_TARGET, myapp.AFUTarget().c_str());

   ////////////////////////////////////////////////////////////////////////////
   // Start up the runtime and get the service.
#if DBG_HOOK
   INFO("RunTimeArgs: " << RunTimeArgs);
   INFO("Manifest: " << Manifest);
#endif // DBG_HOOK

   bStarted = myapp.start( RunTimeArgs, Manifest);
   if (!bStarted) {
      myapp.syncStop();
      return 4;
   }

   // Run the test
   res += RunTest(&myapp);

   INFO("Releasing the SPL Service");
   myapp.syncRelease(TransactionID());

   INFO("Stopping the AAL Runtime");
   myapp.stop();

   if ( res > 0 ) {
      ERR("Test FAILED with " << res << " error" << ((res > 1) ? "s." : "."));
   } else {
      INFO("Test PASSED");
   }

   return res;
}


btInt RunTest(CMyApp *p)
{

   if ( p->DeepScrub() ) {
      p->m_RandScratchpad = p->BufferInitSeed();
      return HardwareDeepScrub(p);
   } else {
      return BasicHardwareCheckout(p);
   }
}


btInt BasicHardwareCheckout(CMyApp *p)
{
   p->WSRequestLen(MB(4));
   p->BufferLayout(CMyApp::CNTXT_SRC_DEST);
   p->BufferInit(CMyApp::ID);
   p->SPLPollRate(50); // milliseconds
   p->TaskMode(CMyApp::ONE_SHOT);
   return SPLTest(p);
}

///////////////////////////////////////////////////////////////////////////////
// Utility functions for Identifiable cache-line copy tests

/// @addtogroup splapp2
/// @{

/// Set an Identifiable cache-line worker's worker.
/// Length of target is explicitly ONE Cache Line
void __SetCLID( struct WordInBuf *pwib, // Pointer to cache-line
                struct WordInBuf *pWIB) // WordInBuf to use for initialization
{
   struct WordInBuf  WIB = *pWIB;      // Make a copy
   for( btUnsigned32bitInt DWordPos = 0; DWordPos < ( CL(1) / sizeof(WIB) ); ++DWordPos) {
      WIB.DWordPos = DWordPos;
      *pwib++ = WIB;
   }
}  // _SetCLID

/// Set an Identifiable cache line worker
void _SetCLID( void *pBuf,        // Address of source or destination cache line
               int   clIndex,     // Cache line index into buffer
               bool  bIsSource)   // True if Source buffer
{
   struct WordInBuf   WIB;
   WIB.BeforeAfter = bIsSource ? 0xA : 0xB;
   WIB.CLine = clIndex;

   struct WordInBuf *pwib = reinterpret_cast<struct WordInBuf *>( (char*)pBuf );
   __SetCLID( pwib, &WIB);
}  // _SetCLID

/// Set an Identifiable cacheline
void SetCLID( void *pBuf,        // Address of source or destination buffer
              int   clIndex,     // Cache line index into buffer
              bool  bIsSource)   // True if Source buffer
{
   pBuf = static_cast<char*>(pBuf) + CL(1)*clIndex;
   _SetCLID( pBuf, clIndex, bIsSource);
}  // SetCLID

/// See if this is an Identifiable cacheline worker. Always will be source, if correct.
bool _QueryCLID( struct WordInBuf const *pwib,  // Pointer to cache-line
                 struct WordInBuf const *pWIB)  // WordInBuf to use for initialization
{
   struct WordInBuf  WIB = *pWIB;      // Make a copy
   for( btUnsigned32bitInt DWordPos = 0; DWordPos < 16; ++DWordPos) {
      WIB.DWordPos = DWordPos;
      if( memcmp( pwib++, &WIB, sizeof(struct WordInBuf)) ) return false;
   }
   return true;
}  // _QueryCLID

/// See if this is an Identifiable cacheline. Always will be source, if correct.
bool QueryCLID( void *pBuf,        // Address of source or destination buffer
                int   clIndex)     // Cache line index into buffer
{
   struct WordInBuf   WIB;
   WIB.BeforeAfter = 0xA;  // If source, then source. If dest, then also source.
   WIB.CLine = clIndex;

   struct WordInBuf  *pwib = reinterpret_cast<struct WordInBuf *>( (char*)pBuf + 64*clIndex );
   return _QueryCLID( pwib, &WIB);
}  // QueryCLID

/// Print a cache-line worker.
void _DumpCL( void         *pCL,  // pointer to cache-line to print
             ostringstream &oss)  // add it to this ostringstream
{
   oss << std::hex << std::setfill('0') << std::uppercase;
   btUnsigned32bitInt *pu32 = reinterpret_cast<btUnsigned32bitInt*>(pCL);
   for( int i = 0; i < ( CL(1) / sizeof(btUnsigned32bitInt)); ++i ) {
      oss << "0x" << std::setw(8) << *pu32 << " ";
      ++pu32;
   }
   oss << std::nouppercase;
}  // _DumpCL

/// Print a cache-line.
void Show2CLs( void          *pCLExpected, // pointer to cache-line expected
               void          *pCLFound,    // pointer to found cache line
               ostringstream &oss)         // add it to this ostringstream
{
   oss << "Expected: ";
   _DumpCL( pCLExpected, oss);
   oss << "\n";
   oss << "Found:    ";
   _DumpCL( pCLFound, oss);
//   oss << "\n";    /* no terminating linefeed, macro at end will add it. */
}  // _DumpCL

/// @}

// End Utility functions
///////////////////////////////////////////////////////////////////////////////

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
                  int tres = SPLTest(p);
                  if ( tres > 0 ) {
                     res += tres;
                     ERR("\n\nBufferLayout=" << bl <<
                         " BufferInit="         << bi <<
                         " Seed="               << p->BufferInitSeed() <<
                         " TaskMode="           << tm <<
                         " Poll Rate="          << PollRates[pr] <<
                         " Workspace Size="     << WSSizes[wss] <<
                         "\n\n");
                     break;
                  }
               }
            }
         }
      }
   }

   return res;
}

/// @addtogroup splapp2
/// @{

/// Executes one iteration of an SPL VAFU test on the Service supplied within
///   the given CMyApp.
/// @param[inout] p is pointer to CMyApp instance
/// @return       number of errors found
btInt SPLTest(CMyApp *p)
{
   ////////////////////////////////////////////////////////////////////////////
   // Get a big buffer that will contain 3 items:
   // 1) VAFU_CNTXT is a command parameter block instructing AFU what to do, and
   //    providing a place for AFU to respond when it is finished doing it.
   // 2) Source buffer
   // 3) Destination buffer

   // Allocate a single contiguous workspace for the VAFU2 Context and src/dest buffers.
   p->syncWorkspaceAllocate(p->WSRequestLen(), TransactionID());

   btVirtAddr         pWSUsrVirt = p->OneLargeVirt();
   const btWSSize     WSLen      = p->OneLargeSize();
   btUnsigned32bitInt a_num_cl   = (btUnsigned32bitInt) (((WSLen - sizeof(VAFU2_CNTXT)) / 2) / CL(1));
   void              *pSource    = NULL;        // temporary for pVAFU2_cntxt->pSource
   void              *pDest      = NULL;        // temporary for pVAFU2_cntxt->pDest
   struct OneCL {                               // Make a cache-line sized structure
      btUnsigned32bitInt dw[16];                //    for array arithmetic
   };
   struct OneCL      *pSourceCL;                // Indexing this pointer will do CL array math
   struct OneCL      *pDestCL;                  // Indexing this pointer will do CL array math

   INFO("Allocated " << WSLen << "-byte Workspace at virtual address "
                     << std::hex << (void *)pWSUsrVirt);

   // Note: the usage of the VAFU2_CNTXT structure here is specific to the underlying bitstream
   // implementation. The bitstream targeted for use with this sample application must implement
   // the Validation AFU 2 interface and abide by the contract that a VAFU2_CNTXT structure will
   // appear at byte offset 0 within the supplied AFU Context workspace.

   // Initialize the command buffer
   VAFU2_CNTXT *pVAFU2_cntxt = reinterpret_cast<VAFU2_CNTXT *>(pWSUsrVirt);
   ::memset(pVAFU2_cntxt, 0, sizeof(VAFU2_CNTXT));

   switch ( p->BufferLayout() ) {

      case CMyApp::CNTXT_SRC_DEST : {
         // The source buffer starts after the VAFU_CNTXT.
         pVAFU2_cntxt->pSource = pWSUsrVirt + sizeof(VAFU2_CNTXT);
         // The destination buffer starts after the source buffer.
         pVAFU2_cntxt->pDest   = pWSUsrVirt + sizeof(VAFU2_CNTXT)
                                 + ((WSLen - sizeof(VAFU2_CNTXT)) / 2);

         INFO("VAFU2 Context=" << std::hex << (void *)pVAFU2_cntxt <<
              " Src="  << std::hex << (void *)pVAFU2_cntxt->pSource <<
              " Dest=" << std::hex << (void *)pVAFU2_cntxt->pDest << std::dec);
      } break;

      case CMyApp::CNTXT_DEST_SRC : {
         // The dest buffer starts after the VAFU_CNTXT.
         pVAFU2_cntxt->pDest   = pWSUsrVirt + sizeof(VAFU2_CNTXT);
         // The source starts after the destination buffer.
         pVAFU2_cntxt->pSource = pWSUsrVirt + sizeof(VAFU2_CNTXT)
                                 + ((WSLen - sizeof(VAFU2_CNTXT)) / 2);

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

   // Keep temporaries around in case VAFU context gets corrupted somehow
   pVAFU2_cntxt->num_cl = a_num_cl;
   pSource              = pVAFU2_cntxt->pSource;
   pSourceCL            = static_cast<struct OneCL *>(pVAFU2_cntxt->pSource);
   pDest                = pVAFU2_cntxt->pDest;
   pDestCL              = static_cast<struct OneCL *>(pVAFU2_cntxt->pDest);

   INFO(" Cache lines=" << std::dec << pVAFU2_cntxt->num_cl <<
        " (bytes="       << std::dec << pVAFU2_cntxt->num_cl * CL(1) <<
        " 0x"            << std::hex << pVAFU2_cntxt->num_cl * CL(1) << std::dec << ")");

   btUnsigned32bitIntArray    pu32;
   btUnsigned32bitIntArray pEndu32;

   // Init the src/dest buffers, based on the desired sequence (either fixed or random).
   if ( CMyApp::FIXED == p->BufferInit() ) {
      INFO("Initializing buffers with fixed data pattern. (src=0xafafafaf dest=0xbebebebe)");

      ::memset( pSource, 0xAF, a_num_cl * CL(1));
      ::memset( pDest,   0xBE, a_num_cl * CL(1));

   } else if ( CMyApp::RAND == p->BufferInit() ) {
      p->SaveBufferInitSeed(p->m_RandScratchpad);

      INFO("Initializing src buffer with random data pattern (seed=0x" <<
           std::hex << p->m_RandScratchpad << std::dec << "), and dest buffer with 0.");

      pu32    = reinterpret_cast<btUnsigned32bitIntArray>(pSource);
      pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pSource) + (a_num_cl * CL(1)));

      // Source is random based on seed
      while ( pu32 < pEndu32 ) {
         *pu32 = GetRand(&p->m_RandScratchpad);
         ++pu32;
      }

      // Destination is zero
      ::memset( pDest, 0, a_num_cl * CL(1));

   } else if ( CMyApp::ID == p->BufferInit() ) {
      ostringstream        oss("");

      INFO("Initializing src/dest buffers with Identifiable data pattern.");

      for( btUnsigned32bitInt CLine = 0; CLine < a_num_cl; ++CLine) {
         SetCLID( pSource, CLine, true);   // source
         SetCLID( pDest  , CLine, false);  // destination
      }

   } else {
      ERR("Invalid buffer init: " << p->BufferInit());
      ASSERT(false);
      return 1;
   } // Switching on buffer initialization

   // Buffers have been initialized
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // Get the AFU and start talking to it

   volatile VAFU2_DSM *pAFUDSM;

   // Start the SPL task, or init the session, then start the task.
   if ( CMyApp::ONE_SHOT == p->TaskMode() ) {
      INFO("Starting SPL Transaction with Workspace");

      // Provide a workspace and so also start the transaction.
      // The VAFU2 Context is assumed to be at the start of the workspace.
      p->syncStartTransactionContext(TransactionID(), pWSUsrVirt, p->SPLPollRate());

      // Examine the AFU ID from the AFU DSM pointer returned in the response event.
      pAFUDSM = (volatile VAFU2_DSM *)p->AFUDSMVirt();
      ASSERT(pAFUDSM != NULL);

      INFO("AFU ID = 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[1] <<
           " 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[0] << std::dec);

   } else if ( CMyApp::SPLIT == p->TaskMode() ) {
      INFO("Starting SPL Transaction without Workspace");

      // No workspace, so transaction not started.
      p->syncStartTransactionContext(TransactionID());

      // Examine the AFU ID from the AFU DSM pointer returned in the response event.
      pAFUDSM = (volatile VAFU2_DSM *)p->AFUDSMVirt();
      ASSERT(pAFUDSM != NULL);

      INFO("AFU ID = 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[1] <<
           " 0x" << std::hex << pAFUDSM->vafu2.AFU_ID[0] << std::dec);

      // [OPTIONAL]
      // Verify that the CSR write region was mapped properly.
      // To do so, we..
      // 1) clear AFU_DSM_SCRATCH to zero.
      // 2) write some non-zero value to AFU_CSR_SCRATCH.
      // 3) verify that the value is reflected at AFU_DSM_SCRATCH.

      INFO("(before clearing) AFU_DSM_SCRATCH is " <<
           std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);

      pAFUDSM->AFU_DSM_SCRATCH = 0;
      INFO("(after clearing) AFU_DSM_SCRATCH is " << std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);

      btBool bRes = p->CSRWrite(byte_offset_AFU_CSR_SCRATCH / 4, 0xdecafbad);
      ASSERT(bRes);
      INFO("CSRWrite() returned " << bRes);

      INFO("(when checking) AFU_DSM_SCRATCH is " << std::hex << pAFUDSM->AFU_DSM_SCRATCH << std::dec);
      ASSERT(0xdecafbad == pAFUDSM->AFU_DSM_SCRATCH);

      INFO("Setting SPL Context Workspace");
      p->syncSetContextWorkspace(TransactionID(), pWSUsrVirt, p->SPLPollRate());
      INFO("Workspace set, SPL Transaction started");

   } else {
      ERR("Invalid task mode: " << p->TaskMode());
      ASSERT(false);
      return 1;
   } // End of action based on task mode

   // Check the status.
   if ( !p->IsOK() ) {
      ERR("IsOK check failed");
      return 1;
   }
   // The AFU is running
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // Wait for the AFU to be done. This is AFU-specific, we have chosen to poll ...

   // Set timeout increment based on hardware, software, or simulation
   bt32bitInt count(500);  // 5 seconds with 10 millisecond sleep
   bt32bitInt delay(10);   // 10 milliseconds is the default
   if ( 0 == p->AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_ASE) ) {
      delay = 1000;        // 1 second polling loop for RTL simulation
      count = 7200;        // two hour timeout
   }

   // Wait for SPL VAFU to finish code
   volatile bt32bitInt done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
   while (!done && --count) {
      SleepMilli( delay );
      done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
   }
   if ( !done ) {
      // must have dropped out of loop due to count -- never saw update
      ERR("AFU never signaled it was done. Timing out anyway. Results may be strange.\n");
   }
   // The AFU is done
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // OPTIONAL: Check performance counters on hardware

   // only do the performance counters for the real FPGA. Not defined in other modes.
   if ( done && (0 == p->AFUTarget().compare(SPLAFU_NVS_VAL_TARGET_FPGA))) {
      // Print performance counters
      INFO("FPGA Clocks to Load 1 CL   = 0x" << hex << pAFUDSM->AFU_DSM_LATENCY <<
           " "                               << dec << pAFUDSM->AFU_DSM_LATENCY);
      INFO("FPGA Clocks to finish Task = 0x" << hex << pAFUDSM->AFU_DSM_PERFORMANCE <<
           " "                               << dec << pAFUDSM->AFU_DSM_PERFORMANCE);
      INFO("");
      INFO("FPGA Clock is 200 MHz, or takes 5 ns.");
      INFO("Latency is Clocks to Load * 5 ns = " << 5*pAFUDSM->AFU_DSM_LATENCY << " ns.");
      INFO("");
      double num_bytes = (double)a_num_cl * CL(1);
      INFO("Bandwidth = #bytes moved/time.\n" <<
            "\t#bytes moved is " << num_bytes << " twice, once" <<
            " in and once out, for total " << num_bytes*2);
      num_bytes *= 2;
      double nano_secs = (double)pAFUDSM->AFU_DSM_PERFORMANCE * 5.0;
      const double billion = 1000.0 * 1000.0 * 1000.0 ;
      INFO("\ttime in nanoseconds is " << nano_secs << " or in seconds " << (nano_secs/billion));
      INFO("\tBandwith = " << (num_bytes/nano_secs) << " Gigabytes/second (where Giga is 10^9)");
   } // performance counters for FPGA

   // Done with Performance Counters
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // Stop the AFU

   // Issue Stop Transaction and wait for OnTransactionStopped
   INFO("Stopping SPL Transaction");
   p->syncStopTransactionContext( TransactionID() );
   INFO("SPL Transaction complete");

   ////////////////////////////////////////////////////////////////////////////
   // Check the buffers to make sure they copied okay

   btUnsignedInt        cl;
   btBool               bCLOK;
   btInt                res = 0;
   ostringstream        oss("");
   btUnsigned32bitInt   tCacheLine[16];   // Temporary cacheline for various purposes
   CASSERT( sizeof(tCacheLine) == CL(1) );

   INFO("Verifying buffers");

   // Verify 1) that the source buffer was not corrupted and
   //        2) that the dest buffer contains the source buffer contents.
   if ( CMyApp::FIXED == p->BufferInit() ) {

      int tres(0);                                       // dump only 4 CL's at a time
      ::memset( tCacheLine, 0xAF, sizeof(tCacheLine) );  // expected for both source and dest buffers

      for ( cl = 0 ; cl < a_num_cl && tres < 4; ++cl ) { // check for error in source
         if( ::memcmp( tCacheLine, &pSourceCL[cl], CL(1) ) ) {
            Show2CLs( tCacheLine, &pSourceCL[cl], oss);
            ERR("Source cache line " << cl << " @" << (void*)&pSourceCL[cl] <<
                  " has been corrupted.\n" << oss.str() );
            oss.str(std::string(""));
            ++res;
            ++tres;
         }
      }

      tres = 0;                                          // dump only 4 CL's at a time

      for ( cl = 0 ; cl < a_num_cl && tres < 4; ++cl ) { // check for error in dest
         if( ::memcmp( tCacheLine, &pDestCL[cl], CL(1) ) ) {
            Show2CLs( tCacheLine, &pDestCL[cl], oss);
            ERR("Destination cache line " << cl << " @" << (void*)&pDestCL[cl] <<
                  " is not what was expected.\n" << oss.str() );
            oss.str(std::string(""));
            ++res;
            ++tres;
         }
      }

   } else if ( CMyApp::RAND == p->BufferInit() ) {
      p->m_RandScratchpad = p->SaveBufferInitSeed();

      btUnsigned32bitInt CLSeed;
      btUnsigned32bitInt RandNum;

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CL(1)));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CL(1));

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

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pSource) + (cl * CL(1)));
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
            break;
         }
      }

      p->m_RandScratchpad = p->SaveBufferInitSeed();

      for ( cl = 0 ; cl < pVAFU2_cntxt->num_cl ; ++cl ) {

         pu32    = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CL(1)));
         pEndu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pu32) + CL(1));

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

            pu32 = reinterpret_cast<btUnsigned32bitIntArray>(((btByteArray)pVAFU2_cntxt->pDest) + (cl * CL(1)));
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
            break;
         }
      } // end of RAND

   } else if ( CMyApp::ID == p->BufferInit() ) {
      int tres(0);                                       // dump only 4 CL's at a time

      for ( cl = 0 ; cl < a_num_cl && tres < 4; ++cl ) { // check for error in source
         _SetCLID( tCacheLine, cl, true);                // source
         if( ::memcmp( tCacheLine, &pSourceCL[cl], CL(1) ) ) {
            Show2CLs( tCacheLine, &pSourceCL[cl], oss);
            ERR("Source cache line " << cl << " @" << (void*)&pSourceCL[cl] <<
                  " has been corrupted.\n" << oss.str() );
            oss.str(std::string(""));
            ++res;
            ++tres;
         }
      }

      tres = 0;                                          // dump only 4 CL's at a time

      for ( cl = 0 ; cl < a_num_cl && tres < 4; ++cl ) { // check for error in dest
         _SetCLID( tCacheLine, cl, true);                // source -- because source was to be copied to dest
         if( ::memcmp( tCacheLine, &pDestCL[cl], CL(1) ) ) {
            Show2CLs( tCacheLine, &pDestCL[cl], oss);
            ERR("Destination cache line " << cl << " @" << (void*)&pDestCL[cl] <<
                  " is not what was expected.\n" << oss.str() );
            oss.str(std::string(""));
            ++res;
            ++tres;
         }
      }

   } else {
      ERR("Invalid buffer init: " << p->BufferInit());
      ASSERT(false);
      return 1;
   } // Switching on buffer initialization

   // Done Checking the buffers
   ////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////
   // Clean up and exit

   INFO("Workspace verification complete, freeing workspace.");
   p->syncWorkspaceFree( pWSUsrVirt, TransactionID());

   return res;
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
@addtogroup splapp2
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>An ISPLClient implementation similar to that in splapp.</li>
  <li>Use of the CSyncClient super-class to inherit implementation.</li>
  <li>Runtime selection of AFU targets with SPLAFU.</li>
  <li>SPL transactions with ISPLAFU.</li>
</ul>

This sample is designed to be used with SPLAFU.

1 Summary of Operation

splapp2 relies on its instantiation of CSyncClient inherited by CMyApp to
perform the brunt of the XL runtime interaction. CSyncClient instantiates
a instance of the XL Runtime object, and provides default operations for the
IRuntime and IServiceClient interfaces. CMyApp inherits this implementation
and extends it to add the SPL-specific ISPLClient implementation and a synchronous
version of the ISPLAFU Service interface.

The CMyApp object declared in main() handles all of these functions, hopefully leaving
the bulk of the interesting processing to be exposed cleanly in main().

The command line parameters are parsed and then stored in the CMyApp instance, where they
are used to select the AFU target implementation, whether to run permutation testing,
and to save the requested random seed value, if any, for random source buffer initialization.

Some AFU-specific Runtime parameter configuration is performed prior to calling
CMyApp's start() function, which in turn calls the Runtime.start() function, handles the
response in runtimeStarted(), and then calls synAllocService to instantiate the
SPL Service, which is again serviced in CSyncClient::serviceAllocated(). After all of this
CMyAPP.start() returns with an error code set if anything went wrong.

The SPL test parameters are set up and the test(s) is/are performed.

When the SPL test(s) is(are) complete, syncRelease() and finally stop() are called to shut down
all the machinery. Everything is synchronous behind the scenes. The CMyApp class could be
re-used, although probably the simpler form in splapp3 should be made canonical and then
extended for the additional functionality provided in splapp2.

Finally, the SPL test status is reported, and the application exits.

2 Running the application

2.0 Online Help

@code
$ splapp2 --help
Usage:
   splapp2 [--target=<TARGET>] [--seed=<SEED>] [--deep-scrub] [--log=<LOG_LEVEL>]

      <TARGET>     = one of { fpga ase swsim }
      <SEED>       = 32 bit seed for RNG / fixed pattern
      --deep-scrub = run thorough permutation testing (default is basic checkout)
      <LOG_LEVEL>  = 0 to 8, with 0 being unwise and 8 being excruciatingly verbose.
                     Default is 5.
@endcode

2.1 SPL FPGA (HWSPLAFU)

Prerequisites for running the sample with an FPGA:
<ul>
  <li>The SPL AAL device drivers must be loaded.</li>
  <li>The AAL Resource Manager must be running.</li>
  <li>The FPGA module connected to the system must be programmed with an appropriate SPL AFU bit stream.</li>
</ul>

@code
$ splapp2 --target=fpga@endcode

2.2 SPL AFU Simulation Environment (ASESPLAFU)

Prerequisites for running the sample with ASE:
<ul>
  <li>The ASE simulation-side application must be running on the system.</li>
</ul>

@code
$ splapp2 --target=ase@endcode

2.3 SPL Software Simulation (SWSimSPLAFU)

Prerequisites for running the sample with Software Simulation:
<ul>
  <li>(none)
</ul>

@code
$ splapp2 --target=swsim@endcode

@}
*/


