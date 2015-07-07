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
/// @file cciapp.cpp
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

using namespace AAL;

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

/// @addtogroup cciapp
/// @{

////////////////////////////////////////////////////////////////////////////////
// CMyCCIClient

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyCCIClient CMycciappCCIClient

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
#define CMyApp CMycciappApp

/// The default CCIAFU Delegate.
#define DEFAULT_TARGET_AFU CCIAFU_NVS_VAL_TARGET_FPGA

/// @brief cciapp-specific instantiation of ISingleAFUApp that provides access to an ICCIAFU.
class CMyApp : public ISingleAFUApp<ICCIAFU>
{
public:
   CMyApp();

   // <ISingleAFUApp>
   virtual void OnRuntimeStarted(IRuntime *,
                                 const NamedValueSet &);
   virtual void OnRuntimeStopped(IRuntime *);
   virtual void OnRuntimeStartFailed(const IEvent &);
   virtual void OnRuntimeAllocateServiceFailed(IEvent const &);
   virtual void OnRuntimeAllocateServiceSucceeded(IBase * ,
                                                  TransactionID const &);
   virtual void OnRuntimeEvent(const IEvent &);

   virtual void OnServiceAllocated(IBase *,
                                   TransactionID const &);
   virtual void OnServiceAllocateFailed(const IEvent &);
   virtual void OnServiceReleased(TransactionID const &);
   virtual void OnServiceReleaseFailed(const IEvent &);
   virtual void OnServiceEvent(const IEvent &);
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

void CMyApp::OnRuntimeStarted(IRuntime            *pRT,
                              const NamedValueSet &Args)
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

void CMyApp::OnRuntimeStopped(IRuntime *pRT)
{
   INFO("Runtime Stopped");
}

void CMyApp::OnRuntimeStartFailed(const IEvent &e)
{
   m_bIsOK = false;
   INFO("Runtime Start Failed");
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
   LOG_INFO
};

int my_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int my_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only my_nix_long_option_only = { my_on_nix_long_option_only, };
aalclp_option      my_nix_long_option      = { my_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "cciapp",
                             "0.0.0",
                             "",
                             help_msg_callback,
                             &gMyCmdLine)

int parsecmds(struct CMyCmdLine * , int , char *[] );
int verifycmds(struct CMyCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

btInt RunTest(CMyApp * );

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
   cerr << " AAL CCI VAFU Sample"  << endl;
   cerr << "=====================" << endl << endl;

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
      pAALLogger()->SetDestination(ILogger::CERR);
      pAALLogger()->SetLogPID(true);
      pAALLogger()->SetLogTimeStamp(true);
   }
#endif // DBG_HOOK

   CMyApp        myapp;
   NamedValueSet args;
   Runtime       aal(&myapp);

   myapp.AFUTarget(gMyCmdLine.AFUTarget);

   if ( (0 == myapp.AFUTarget().compare(CCIAFU_NVS_VAL_TARGET_ASE)) ||
        (0 == myapp.AFUTarget().compare(CCIAFU_NVS_VAL_TARGET_SWSIM)) ) {
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
      INFO("Stopping the AAL Runtime");
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


/// @addtogroup cciapp
/// @{

/// @brief Executes one iteration of a Native Loopback (LPBK1) test on the Service supplied within
///        the given CMyApp.
btInt NLBLpbk1(CMyApp *app)
{
   btInt res = 0;

   ICCIAFU *pAFU = (ICCIAFU *) *app; // uses type cast operator from ISingleAFUApp.
   ASSERT(NULL != pAFU);

#define LPBK1_DSM_SIZE           MB(4)
#define LPBK1_BUFFER_SIZE        CL(1)

#define CSR_CIPUCTL              0x280

#define CSR_AFU_DSM_BASEL        0x1a00
#define CSR_AFU_DSM_BASEH        0x1a04
#define CSR_SRC_ADDR             0x1a20
#define CSR_DST_ADDR             0x1a24
#define CSR_NUM_LINES            0x1a28
#define CSR_CTL                  0x1a2c
#define CSR_CFG                  0x1a34

#define DSM_STATUS_TEST_COMPLETE 0x40
#define DSM_STATUS_TEST_ERROR    0x44
#define DSM_STATUS_MODE_ERROR_0  0x60

#define DSM_STATUS_ERROR_REGS    8

#define CSR_OFFSET(x)            ((x) / sizeof(bt32bitCSR))

   // Use the Service to create Workspaces (buffers).
   // For NLB loopback mode (LPBK1), we need three Workspaces:
   //
   // 1.) A workspace to serve as the DSM buffer for AFU -> Host communication.
   // 2.) A data input buffer.
   // 3.) A data output buffer.

   pAFU->WorkspaceAllocate(LPBK1_DSM_SIZE,    TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   pAFU->WorkspaceAllocate(LPBK1_BUFFER_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   pAFU->WorkspaceAllocate(LPBK1_BUFFER_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   app->ClientWait();

   if ( !app->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt = app->InputVirt();

   const    btUnsigned32bitInt  InputData = 0xdecafbad;
   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
                                     (app->InputSize() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) {
      *pInput = InputData;
   }

   volatile btVirtAddr pOutputUsrVirt = app->OutputVirt();

   // zero the output buffer
   ::memset((void *)pOutputUsrVirt, 0, app->OutputSize());

   volatile btVirtAddr pDSMUsrVirt  = app->DSMVirt();

   // zero the DSM
   ::memset((void *)pDSMUsrVirt, 0, app->DSMSize());

   btCSRValue i;
   btCSRValue csr;

   // Assert CAFU Reset
   csr = 0;
   pAFU->CSRRead(CSR_CIPUCTL, &csr);
   csr |= 0x01000000;
   pAFU->CSRWrite(CSR_CIPUCTL, csr);

   // De-assert CAFU Reset
   csr = 0;
   pAFU->CSRRead(CSR_CIPUCTL, &csr);
   csr &= ~0x01000000;
   pAFU->CSRWrite(CSR_CIPUCTL, csr);

   // Set DSM base, high then low
   pAFU->CSRWrite64(CSR_AFU_DSM_BASEL, app->DSMPhys());

   // Poll for AFU ID
   do
   {
      csr = *(volatile bt32bitCSR *)pDSMUsrVirt;
   }while( 0 == csr );

   // Print the AFU ID
   std::ostringstream oss;

   for ( i = 0 ; i < 4 ; ++i ) {
      oss << std::setw(8) << std::hex << std::setfill('0')
          << *(btUnsigned32bitInt *)(pDSMUsrVirt + (3 - i) * sizeof(btUnsigned32bitInt));
   }

   INFO("AFU ID=" << oss.str());

   // Assert Device Reset
   pAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM
   ::memset((void *)pDSMUsrVirt, 0, app->DSMSize());

   // De-assert Device Reset
   pAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   pAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(app->InputPhys()));

   // Set output workspace address
   pAFU->CSRWrite(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(app->OutputPhys()));

   // Set the number of cache lines for the test
   pAFU->CSRWrite(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

   // Set the test mode
   pAFU->CSRWrite(CSR_CFG, 0);


   volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                    (pDSMUsrVirt  + DSM_STATUS_TEST_COMPLETE);

   // Start the test
   pAFU->CSRWrite(CSR_CTL, 3);


   // Wait for test completion
   while( 0 == *StatusAddr ) {
      SleepMicro(100);
   }


   // Verify the buffers
   if ( ::memcmp((void *)pInputUsrVirt, (void *)pOutputUsrVirt, LPBK1_BUFFER_SIZE) != 0 ) {
      res = 1;

      pInput = (volatile btUnsigned32bitInt *)pInputUsrVirt;
      volatile btUnsigned32bitInt *pOutput = (volatile btUnsigned32bitInt *)pOutputUsrVirt;

      btUnsigned32bitInt Byte;

      oss.str(std::string(""));

      ios::fmtflags f = oss.flags();
      oss.flags(ios_base::right | ios_base::hex);
      oss.fill('0');

      for ( Byte = 0 ;
               pInput < pEndInput ;
                  ++pInput, ++pOutput, Byte += sizeof(btUnsigned32bitInt) ) {

         if ( *pInput != *pOutput ) {

            oss << "Buffer[0x" << setw(2) << Byte << "] Input: 0x"
                     << *pInput << " != Output: 0x" << *pOutput << std::endl;

         }

      }

      oss.fill(' ');
      oss.flags(f);

      ERR(oss.str());
   }


   // Verify the device
   if ( *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE)) != 0 ) {
      res = 1;

      oss.str(std::string(""));

      oss << *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE))
          << " Device Errors" << std::endl;

      ios::fmtflags f = oss.flags();
      oss.flags(ios_base::right);
      oss.fill('0');

      for ( i = 0 ; i < DSM_STATUS_ERROR_REGS ; ++i ) {
         oss << "Error Status[" << std::dec << i << "] = 0x"
             << std::hex << std::setw(8) <<
                *(StatusAddr + CSR_OFFSET(DSM_STATUS_MODE_ERROR_0 - DSM_STATUS_TEST_COMPLETE) + i)
             << std::endl;
      }

      oss.fill(' ');
      oss.flags(f);

      ERR(oss.str());
   }


   INFO((0 == res ? "PASS" : "ERROR"));


   // Clean up..

   // Stop the device
   pAFU->CSRWrite(CSR_CTL, 7);

   // Release the Workspaces
   pAFU->WorkspaceFree(pInputUsrVirt,  TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   pAFU->WorkspaceFree(pOutputUsrVirt, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   pAFU->WorkspaceFree(pDSMUsrVirt,    TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   app->ClientWait();

   if ( !app->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}

/// @} group cciapp


btInt RunTest(CMyApp *p)
{
   ASSERT(NULL != (IAALService *) *p);
   return NLBLpbk1(p);
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
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   cciapp [--target=<TARGET>]\n");
   fprintf(fp, "\n");
   fprintf(fp, "      <TARGET> = one of { fpga ase swsim }\n");
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

