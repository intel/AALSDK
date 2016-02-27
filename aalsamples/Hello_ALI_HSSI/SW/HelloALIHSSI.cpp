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
/// @file HelloALINLB.cpp
/// @brief Basic ALI AFU interaction.
/// @ingroup HelloALINLB
/// @verbatim
/// Intel(R) Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/15/2015     JG       Initial version started based on older sample code.@endverbatim
//****************************************************************************
#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>

#include <aalsdk/service/IALIAFU.h>

#include <string.h>

//****************************************************************************
// UN-COMMENT appropriate #define in order to enable either Hardware or ASE.
//    DEFAULT is to use Software Simulation.
//****************************************************************************
#define  HWAFU
//#define  ASEAFU

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

#define CSR_CTL                  0x0138

#define CSR_HSSI_COUNTER_L32     0x0100
#define CSR_HSSI_COUNTER_H32     0x0104
#define CSR_HSSI_STATUS_L32      0x0108
#define CSR_HSSI_STATUS_H32      0x010C

#define EP_DEVICE_ID             0x1572
#define EP_VENDOR_ID             0x8086
#define RP_LINK_WIDTH            0x8
#define RP_SPEED                 0x3
#define RP_LTSSM_STATE           0xF

/// @addtogroup HelloALIHSSI
/// @{


/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class HelloALIHSSIApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

   HelloALIHSSIApp();
   ~HelloALIHSSIApp();

   btInt run(btInt timeout_secs);            ///< Return 0 if success
   btBool getHSSIStatus();  ///< Return true if success
   btUnsigned64bitInt getHSSICounter();

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleased(const AAL::TransactionID&);

   void serviceReleaseFailed(const AAL::IEvent&);

   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};    // Not Used

   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeStopFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);

   btBool isOK()  {return m_bIsOK;}

   // <end IRuntimeClient interface>
protected:
   Runtime        m_Runtime;           ///< AAL Runtime
   IBase         *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
   CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt          m_Result;            ///< Returned result value; 0 if success

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

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
HelloALIHSSIApp::HelloALIHSSIApp() :
   m_Runtime(this),
   m_pAALService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_Result(0),
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_DSMSize(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_InputSize(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0),
   m_OutputSize(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   //  Note that this could also be accomplished by setting the environment variable
   //   AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

#if defined( HWAFU )
   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
#endif

   // Start the Runtime and wait for the callback by sitting on the semaphore.
   //   the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
   if(!m_Runtime.start(configArgs)){
      m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

/// @brief   Destructor
///
HelloALIHSSIApp::~HelloALIHSSIApp()
{
   m_Sem.Destroy();
}

/// @brief   run() is called from main performs the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Allocates the necessary buffers to be used by the NLB AFU algorithm
///             - Executes the NLB algorithm
///             - Cleans up.
///
btInt HelloALIHSSIApp::run(btInt timeout_secs)
{
   cout <<"========================="<<endl;
   cout <<"= Hello ALI HSSI Sample ="<<endl;
   cout <<"========================="<<endl;

   // Request the Servcie we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,"3EAC8EC1-D729-43E8-9E8D-46777DCDBD14");


   // indicate that this service needs to allocate an AIAService, too to talk to the HW
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

   #elif defined ( ASEAFU )         /* Use ASE based RTL simulation */
   Manifest.Add(keyRegHandle, 20);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   #else                            /* default is Software Simulator */
#if 0 // NOT CURRRENTLY SUPPORTED
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
#endif
   return -1;
#endif

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI HSSI");

   MSG("Allocating Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
   //   (Refer to the serviceAllocated() callback to see how the Service's interfaces
   //    are collected.)
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      m_Runtime.stop();
      m_Sem.Wait();
   }

   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================

   btUnsigned64bitInt init_ctr;
   btUnsigned64bitInt compare_ctr;

   struct timespec ts; //TODO get timeout value from Cmd line
   ts.tv_sec = timeout_secs;
   Timer     absolute = Timer() + Timer(&ts);

   MSG("Running Test");
   if(true == m_bIsOK){

      // Initiate AFU Reset
      m_pALIResetService->afuReset();

      // Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

      //De-Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

      init_ctr = getHSSICounter();
      if(0x0 == init_ctr)
      {
         ERR("No Transactions recorded - FAIL.");
         goto done_1;
      }
      if(!getHSSIStatus())
      {
         ERR("Initial HSSI Status - FAIL.");
         goto done_1;
      }

      MSG("Initial HSSI Status - PASS.");

      while(Timer() < absolute){
         if(!getHSSIStatus())
         {
            ERR("HSSI Status - FAIL.");
            break;
         }

         compare_ctr = getHSSICounter();

         if(init_ctr == compare_ctr)
         {
            ERR("No Transactions recorded - FAIL.");
            break;
         }

         init_ctr = compare_ctr;

         SleepMicro(100);
      }


      MSG("Done Running Test");
   }

   // Clean-up and return

done_1:
   // Freed all three so now Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

btUnsigned64bitInt HelloALIHSSIApp::getHSSICounter()
{
   struct CSR_HSSI_COUNTER {

         union {
            btUnsigned64bitInt csr;
            struct {
               btUnsigned32bitInt low;
               btUnsigned32bitInt high;
            };// end struct
         } ; // end union
      }csr_hssi_counter; // end struct CSR_HSSI_COUNTER

      csr_hssi_counter.csr = 0;
      m_pALIMMIOService->mmioRead32(CSR_HSSI_COUNTER_L32, &csr_hssi_counter.low);
      m_pALIMMIOService->mmioRead32(CSR_HSSI_COUNTER_H32, &csr_hssi_counter.high);

      return csr_hssi_counter.csr;
}

btBool HelloALIHSSIApp::getHSSIStatus()
{

   // HSSI Status values
   struct CSR_HSSI_STATUS {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned32bitInt low;
            btUnsigned32bitInt high;
         };
         struct {
            btUnsigned64bitInt rp_ltssm_state :8;
            btUnsigned64bitInt rp_speed :4;
            btUnsigned64bitInt rp_error :12;
            btUnsigned64bitInt rp_link_width :8;
            btUnsigned64bitInt ep_vendor_id :16;
            btUnsigned64bitInt ep_device_id :16;
         }; // end struct
      } ; // end union
   }csr_hssi_status; // end struct CSR_HSSI_STATUS

   csr_hssi_status.csr = 0;
   m_pALIMMIOService->mmioRead32(CSR_HSSI_STATUS_L32, &csr_hssi_status.low);
   m_pALIMMIOService->mmioRead32(CSR_HSSI_STATUS_H32, &csr_hssi_status.high);

   if(EP_DEVICE_ID != csr_hssi_status.ep_device_id)
   {
      cerr << "Bad Device ID. Test failed.\n";
      return false;
   }

   if(EP_VENDOR_ID != csr_hssi_status.ep_vendor_id)
   {
      cerr << "Bad Vendor ID. Test failed.\n";
      return false;
   }

   if(0 != csr_hssi_status.rp_error)
   {
      cerr << "RP Errors detected. Test failed.\n";
      return false;
   }

   if(RP_LINK_WIDTH != csr_hssi_status.rp_link_width)
   {
      cerr << "RP Link Width Mismatch. Test failed.\n";
      return false;
   }

   if(RP_SPEED != csr_hssi_status.rp_speed)
   {
      cerr << "RP not at gen3 speed. Test failed.\n";
      return false;
   }

   if(RP_LTSSM_STATE != csr_hssi_status.rp_ltssm_state)
   {
     cerr << "Link out of L0. Test failed.\n";
     return false;
   }
   return true;
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void HelloALIHSSIApp::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   // Save the IBase for the Service. Through it we can get any other
   //  interface implemented by the Service
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);
   if ( NULL == m_pAALService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIBuffer as subclass interface. Used in Buffer Allocation and Free
   m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
   ASSERT(NULL != m_pALIBufferService);
   if ( NULL == m_pALIBufferService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIMMIO as subclass interface. Used to set/get MMIO Region
   m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
   ASSERT(NULL != m_pALIMMIOService);
   if ( NULL == m_pALIMMIOService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIReset as subclass interface. Used for resetting the AFU
   m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
   ASSERT(NULL != m_pALIResetService);
   if ( NULL == m_pALIResetService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void HelloALIHSSIApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void HelloALIHSSIApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void HelloALIHSSIApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void HelloALIHSSIApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   // The state machine may or may not stop here. It depends upon what happened.
   // A fatal error implies no more messages and so none of the other Post()
   //    will wake up.
   // OTOH, a notification message will simply print and continue.
}
// <end IServiceClient interface>


 //=================
 //  IRuntimeClient
 //=================

  // <begin IRuntimeClient interface>
 // Because this simple example has one object implementing both IRuntieCLient and IServiceClient
 //   some of these interfaces are redundant. We use the IServiceClient in such cases and ignore
 //   the RuntimeClient equivalent e.g.,. runtimeAllocateServiceSucceeded()

 void HelloALIHSSIApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void HelloALIHSSIApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void HelloALIHSSIApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void HelloALIHSSIApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void HelloALIHSSIApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void HelloALIHSSIApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void HelloALIHSSIApp::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }
 // <begin IRuntimeClient interface>

/// @} group HelloALINLB


//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the object theApp.
//=============================================================================
int main(int argc, char *argv[])
{

   if(argc < 2)
   {
      cout << "Usage: --ts=S              where S is timeout in seconds" << endl;
      exit (1);
   }

   HelloALIHSSIApp theApp;
   btInt timeout_secs = 5;

   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   sscanf(argv[1], "--ts=%d", &timeout_secs);

   btInt Result = theApp.run(timeout_secs);

   if (0 == Result){
      MSG("TEST PASS");
   }
   else{
      ERR("TEST FAIL");
   }
   MSG("Done");
   return Result;
}

