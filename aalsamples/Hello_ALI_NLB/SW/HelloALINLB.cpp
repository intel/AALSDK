// Copyright (c) 2007-2015, Intel Corporation
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
/// @file HelloALINLB.cpp
/// @brief Basic ALI AFU interaction.
/// @ingroup HelloALINLB
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// This Sample demonstrates the following:
///    - The basic structure of an AAL program using the AAL APIs.
///    - The ICCI and ICCIClient interfaces of ALI AFU Service.
///    - System initialization and shutdown.
///    - Use of interface IDs (iids).
///    - Accessing object interfaces through the Interface functions.
///
/// This sample is designed to be used with the CCIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/09/2015     JG       Initial version started based on older sample code.@endverbatim
//****************************************************************************
//#include <aalsdk/AAL.h>
#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger
#include <aalsdk/kernel/ccipdriver.h>

/*#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/service/ICCIClient.h>*/
#include <aalsdk/service/IALIAFU.h>

#include <string.h>

//****************************************************************************
// UN-COMMENT appropriate #define in order to enable either Hardware or ASE.
//    DEFAULT is to use Software Simulation.
//****************************************************************************
// #define  HWAFU
#define  ASEAFU

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

#ifndef CL
# define CL(x)                     ((x) * 64)
#endif // CL
#ifndef LOG2_CL
# define LOG2_CL                   6
#endif // LOG2_CL
#ifndef MB
# define MB(x)                     ((x) * 1024 * 1024)
#endif // MB
#define LPBK1_BUFFER_SIZE        CL(1)

#define LPBK1_DSM_SIZE           MB(4)
//#define CSR_AFU_DSM_BASEH        0x1a04
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
//#define CSR_CIPUCTL              0x280  /* should not be used */
#define CSR_NUM_LINES            0x0130
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114
#define NLB_TEST_MODE_PCIE0      0x2000
/// @addtogroup HelloCCINLB
/// @{


/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class HelloALINLBApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

   HelloALINLBApp();
   ~HelloALINLBApp();

   btInt run();    ///< Return 0 if success

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleased(const AAL::TransactionID&);

   void serviceReleaseFailed(const AAL::IEvent&);

   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};

   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeStopFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);

   btBool isOK()  {return m_isOK;}

   // <end IRuntimeClient interface>
protected:
   Runtime        m_Runtime;           ///< AAL Runtime
   IBase         *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
   IALIUMsg      *m_pALIuMSGService;   ///< Pointer to uMSg Service
   CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
   btUnsignedInt  m_wsfreed;           ///< Simple counter used for when we free workspaces
   btInt          m_Result;            ///< Returned result value; 0 if success
   btBool         m_isOK;

   // Workspace info
   btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
   btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
   btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
   btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
   btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
   btWSSize       m_InputSize;      ///< Input workspace size in bytes.
   btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
   btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
   btWSSize       m_OutputSize;     ///< Output workspace size in bytes.
};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////
HelloALINLBApp::HelloALINLBApp() :
   m_Runtime(this),
   m_pAALService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_wsfreed(0),
   m_Result(0),
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_DSMSize(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_InputSize(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0),
   m_OutputSize(0),
   m_isOK(false)

{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
   m_Sem.Create(0, 1);

   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   //  Note that this could also be accomplished by setting the environment variable
   //   AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
#if defined( HWAFU )
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
#endif

   if(!m_Runtime.start(configArgs)){
      m_isOK = false;
      return;
   }
   m_Sem.Wait();
   // m_bIsOK, inherited from CAASBase, will generally be true here.
   // Set it to false if any construction fails.
}

HelloALINLBApp::~HelloALINLBApp()
{
   m_Sem.Destroy();
}

btInt HelloALINLBApp::run()
{
   cout <<"========================"<<endl;
   cout <<"= Hello ALI NLB Sample ="<<endl;
   cout <<"========================"<<endl;

   // Request our AFU.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change. But it does
   //  illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */

   // TODO: backdoor - this should all be figured out be the resource manager
   // the service library name of the requested service
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");
   // the AFUID (to be passed to AIA)
   ConfigRecord.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612");
   // indicate that this service needs to allocate an AIAService, too (to talk to the AFU)
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

#elif defined ( ASEAFU )         /* Use ASE based RTL simulation */
   Manifest.Add(keyRegHandle, 20);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
   MSG("ALI/ASE Simulation NLB");
#else                            /* default is Software Simulator */

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

#endif

   // backdoor
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything should be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");
   MSG("Allocating Service");

   // Allocate the Service and allocate the required workspace.
   //   This happens in the background via callbacks (simple state machine).
   //   When everything is set we do the real work here in the main thread.
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);

   m_Sem.Wait();
   if(!m_isOK){
      ERR("Allocation failed\n");
      m_Runtime.stop();
      m_Sem.Wait();
      return -1;
   }
   // Allocate 3 Workspaces .
   if( uid_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_DSM_SIZE, &m_DSMVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      return -1;
   }
   m_DSMSize = LPBK1_DSM_SIZE;
   m_DSMPhys = m_pALIBufferService->bufferGetIOVA(m_DSMVirt);

   if( uid_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_InputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      return -1;
   }
   m_InputSize = LPBK1_BUFFER_SIZE;
   m_InputPhys = m_pALIBufferService->bufferGetIOVA(m_InputVirt);

   if( uid_errnumOK !=  m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_OutputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      return -1;
   }

   m_OutputSize = LPBK1_BUFFER_SIZE;
   m_OutputPhys = m_pALIBufferService->bufferGetIOVA(m_OutputVirt);

   btUnsignedInt numUmsg = m_pALIuMSGService->umsgGetNumber();
   btVirtAddr uMsg0 = m_pALIuMSGService->umsgGetAddress(0);
  if(NULL != uMsg0){

     NamedValueSet nvs;
     nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)0xdeadbeaf);

     btBool ret = m_pALIuMSGService->umsgSetAttributes(nvs);
  }else{
     ERR("No uMSG support");
  }

   // If all went well run test.
   //   NOTE: If not successful we simply bail.
   //         A better design would do all appropriate clean-up.
   if( IsOK() ){

      //=============================
      // Now we have the NLB Service
      //   now we can use it
      //=============================
      MSG("Running Test");

      // #define SIMULATED

// #if defined (SIMULATED)

//       // Initiate AFU Reset
//       if(IALIReset::e_OK != m_pALIResetService->afuReset()){
//          ERR("Reset Failed... as expected in simulation\n");
//       }

// #else
      /* Setting to 0 turns off actul NLB functionality for debug purposes */
      MSG("Here");

      // Clear the DSM
      ::memset( m_DSMVirt, 0, m_DSMSize);

      // Initialize the source and destination buffers
      ::memset( m_InputVirt,  0, m_InputSize);     // Input initialized to 0
      ::memset( m_OutputVirt, 0, m_OutputSize);    // Output initialized to 0

      struct CacheLine {                           // Operate on cache lines
         btUnsigned32bitInt uint[16];
      };
      struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(m_InputVirt);
      for ( btUnsigned32bitInt i = 0; i < m_InputSize / CL(1) ; ++i ) {
         pCL[i].uint[15] = i;
      };                         // Cache-Line[n] is zero except last uint = n


      // Original code puts DSM Reset prior to AFU Reset, but ccipTest
      //    reverses that. We are following ccipTest here.

      // Initiate AFU Reset
      m_pALIResetService->afuReset();


      // Initiate DSM Reset
      // Set DSM base, high then low
      m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_DSMPhys);

      // Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

      //De-Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

      // If ASE, give it some time to catch up
      // #if defined ( ASEAFU )
      // SleepSec(5);
      // #endif /* ASE AFU */



      // Set input workspace address
      m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_InputPhys));

      // Set output workspace address
      m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_OutputPhys));

      // Set the number of cache lines for the test
      m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

      // Set the test mode
      m_pALIMMIOService->mmioWrite32(CSR_CFG,0);

      volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                         (m_DSMVirt  + DSM_STATUS_TEST_COMPLETE);
      // Start the test
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);


      // Wait for test completion
      while( 0 == ((*StatusAddr)&0x1) ) {
         SleepMicro(100);
      }
      MSG("Done Running Test");

      // Stop the device
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

      // Check that output buffer now contains what was in input buffer, e.g. 0xAF
      if (int err = memcmp( m_OutputVirt, m_InputVirt, m_OutputSize)) {
         ERR("Output does NOT Match input, at offset " << err << "!");
         ++m_Result;
      } else {
         MSG("Output matches Input!");
      }

      // Now clean up Workspaces and Release.
      //  Once again all of this is done in a simple
      //  state machine via callbacks
// #endif

      MSG("Done Running Test");

      // Release the Workspaces and wait for all three then Release the Service
      m_wsfreed = 0;  // Reset the counter
      m_pALIBufferService->bufferFree(m_InputVirt);
      m_pALIBufferService->bufferFree(m_OutputVirt);
      m_pALIBufferService->bufferFree(m_DSMVirt);

      // Freed all three so now Release() the Service through the Services IAALService::Release() method
      (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
      m_Sem.Wait();

      m_Runtime.stop();
      m_Sem.Wait();
   }

   return m_Result;
}

// We must implement the IServiceClient interface (IServiceClient.h):

// <begin IServiceClient interface>
void HelloALINLBApp::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   // Documentation says HWALIAFU Service publishes IAALService as subclass interface
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);
   if ( NULL == m_pAALService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIBuffer as subclass interface
   m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
   ASSERT(NULL != m_pALIBufferService);
   if ( NULL == m_pALIBufferService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIMMIO as subclass interface
   m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
   ASSERT(NULL != m_pALIMMIOService);
   if ( NULL == m_pALIMMIOService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIReset as subclass interface
   m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
   ASSERT(NULL != m_pALIResetService);
   if ( NULL == m_pALIResetService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIReset as subclass interface
   m_pALIuMSGService = dynamic_ptr<IALIUMsg>(iidALI_UMSG_Service, pServiceBase);
   ASSERT(NULL != m_pALIuMSGService);
   if ( NULL == m_pALIuMSGService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}
void HelloALINLBApp::runtimeStarted( IRuntime            *pRuntime,
                                     const NamedValueSet &rConfigParms)
{
   m_isOK = true;
   m_Sem.Post(1);
}

void HelloALINLBApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void HelloALINLBApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void HelloALINLBApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void HelloALINLBApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   // The state machine may or may not stop here. It depends upon what happened.
   // A fatal error implies no more messages and so none of the other Post()
   //    will wake up.
   // OTOH, a notification message will simply print and continue.
}
// <end IServiceClient interface>
 void HelloALINLBApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_isOK = false;
     m_Sem.Post(1);
  }

 void HelloALINLBApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void HelloALINLBApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
 }

 void HelloALINLBApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);

 }

 void HelloALINLBApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void HelloALINLBApp::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }

/// @} group HelloALINLB


//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the objects.
//=============================================================================
int main(int argc, char *argv[])
{
   HelloALINLBApp theApp;
   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

