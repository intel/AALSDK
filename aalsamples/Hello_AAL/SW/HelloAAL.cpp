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
/// @file HelloAAL.cpp
/// @brief Basic AFU interaction.
/// @ingroup HelloAAL
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
///    - The IHelloAAL and IHelloAALClient interfaces of HelloAALService.
///    - System initialization and shutdown.
///    - Use of interface IDs (iids).
///    - Creating a AFU class that exposes a proprietary interface.
///    - Invoking a method on an AFU using a proprietary interface.
///    - Accessing object interfaces through the Interface functions.
///
/// This sample is designed to be used with SampleAFU1.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/10/2015     JG       Initial version started based on older sample code.@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger

using namespace std;
using namespace AAL;

#include "HelloAALService.h"      // Service API

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
// doxygen hACK to generate correct class diagrams
#define RuntimeClient HelloAALRuntimeClient
/// @addtogroup HelloAAL
/// @{



/// @brief   Define our Runtime client class so that we can receive the runtime started/stopped notifications.
///
/// We implement a Service client within, to handle AAL Service allocation/free.
/// We also implement a Semaphore for synchronization with the AAL runtime.
class RuntimeClient : public CAASBase,
                      public IRuntimeClient
{
public:
   RuntimeClient();
   ~RuntimeClient();

   /// @brief Synchronous wrapper for stopping the Runtime.
   void end();
   /// @brief Accessor for pointer to IRuntime stored in Runtime Client
   ///
   /// This pointer is used to allocate Service. 
   IRuntime* getRuntime();

   /// @brief Checks that the object is in an internally consistent state
   ///
   /// The general paradigm in AAL is for an object to track its internal state for subsequent query,
   /// as opposed to throwing exceptions or having to constantly check return codes.
   /// We implement this to check if the status of the service allocated.
   /// In this case, isOK can be false for many reasons, but those reasons will already have been indicated by logging output.
   btBool isOK();

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent);

   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeStopFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);
 // <end IRuntimeClient interface>

protected:
   IRuntime        *m_pRuntime;  // Pointer to AAL runtime instance.
   Runtime          m_Runtime;   // AAL Runtime
   btBool           m_isOK;      // Status
   CSemaphore       m_Sem;       // For synchronizing with the AAL runtime.
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyRuntimeClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
RuntimeClient::RuntimeClient() :
   m_Runtime(this),        // Instantiate the AAL Runtime
   m_pRuntime(NULL),
   m_isOK(false)
{
   NamedValueSet configArgs;     // Bot used

   // Publish our interface
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   m_Sem.Create(0, 1);
   m_Runtime.start(configArgs);
   m_Sem.Wait();
}

RuntimeClient::~RuntimeClient()
{
   m_Sem.Destroy();
}

btBool RuntimeClient::isOK()
{
   return m_isOK;
}

void RuntimeClient::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   MSG("Runtime Create or Get Proxy failed");
   m_isOK = false;
   m_Sem.Post(1);
}

void RuntimeClient::runtimeStarted(IRuntime *pRuntime,
                                   const NamedValueSet &rConfigParms)
{
   // Save a copy of our runtime interface instance.
   m_pRuntime = pRuntime;
   m_isOK = true;
   m_Sem.Post(1);
}

void RuntimeClient::end()
{
   m_Runtime.stop();
   m_Sem.Wait();
}

void RuntimeClient::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_isOK = false;
   m_Sem.Post(1);
}

void RuntimeClient::runtimeStartFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime start failed");
   ERR(pExEvent->Description());
}

void RuntimeClient::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
}

void RuntimeClient::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime AllocateService failed");
   ERR(pExEvent->Description());
}

void RuntimeClient::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
   TransactionID const * foo = &rTranID;
   MSG("Runtime Allocate Service Succeeded");
}

void RuntimeClient::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

IRuntime * RuntimeClient::getRuntime()
{
   return m_pRuntime;
}


/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class HelloAALApp: public CAASBase, public IServiceClient, public IHelloAALClient
{
public:
   HelloAALApp(RuntimeClient * rtc);
   ~HelloAALApp();
   /// @brief Called by the main part of the application,Returns 0 if Success
   ///
   /// Application Requests Service using Runtime Client passing a pointer to self.
   /// Blocks calling thread from [Main} untill application is done. 
   int run();

   // <begin IHelloAALClient>
   void HelloApp(TransactionID const &rTranID);
   // <end IHelloAALClient>

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
            TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

    void serviceReleaseFailed(const IEvent &rEvent);

    void serviceReleased(TransactionID const &rTranID);

   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

protected:
   IBase            *m_pAALService;    // The generic AAL Service interface for the AFU.
   RuntimeClient    *m_runtimClient;
   CSemaphore        m_Sem;            // For synchronizing with the AAL runtime.
   int               m_Result;         // Returned result value; 0 if success
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyServiceClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
HelloAALApp::HelloAALApp(RuntimeClient *rtc) :
   m_pAALService(NULL),
   m_runtimClient(rtc),
   m_Result(0)
{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidSampleHelloAALClient, dynamic_cast<IHelloAALClient *>(this));
   m_Sem.Create(0, 1);
}

HelloAALApp::~HelloAALApp()
{
   m_Sem.Destroy();
}

int HelloAALApp::run()
{

   cout <<"===================="<<endl;
   cout <<"= Hello AAL Sample ="<<endl;
   cout <<"===================="<<endl;

   // Request our AFU.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libhelloaalservice");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello AAL");

   MSG("Allocating Service");

   #if DBG_HOOK
         cout << Manifest << endl;
   #endif // DBG_HOOK

   m_runtimClient->getRuntime()->allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();

   m_runtimClient->end();
   return m_Result;
}

// We must implement the IServiceClient interface (IServiceClient.h):

// <begin IServiceClient interface>
void HelloAALApp::serviceAllocated(IBase *pServiceBase,
                                   TransactionID const &rTranID)
{
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);

   IHelloAALService *ptheService = dynamic_ptr<IHelloAALService>(iidSampleHelloAAL, pServiceBase);

   ASSERT(NULL != ptheService);
   if ( NULL == ptheService ) {
      return;
   }

   MSG("Service Allocated");

   // We have the Service, now to use it.
   ptheService->Hello("the Application", TransactionID());
}

void HelloAALApp::serviceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Failed to allocate a Service");
   ERR(pExEvent->Description());
   ++m_Result;
   m_Sem.Post(1);
}

 void HelloAALApp::serviceReleaseFailed(const IEvent        &rEvent)
{
    MSG("Failed to Release a Service");
    m_Sem.Post(1);
 }

 void HelloAALApp::serviceReleased(TransactionID const &rTranID)
 {
    MSG("Service Released");
   m_Sem.Post(1);
}

void HelloAALApp::HelloApp(TransactionID const &rTranID)
{

   MSG("AAL says Hello Back ");
   IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
   ASSERT(pIAALService);
   pIAALService->Release(TransactionID());
}

void HelloAALApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}
// <end IServiceClient interface>

/// @}



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

   RuntimeClient     runtimeClient;
   HelloAALApp       theApp(&runtimeClient);

   if(!runtimeClient.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   int result = theApp.run();

   MSG("Done");
   return result;
}


