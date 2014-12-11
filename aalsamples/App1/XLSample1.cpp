// Copyright (c) 2007-2014, Intel Corporation
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
/// @file XLSample1.cpp
/// @brief Basic AFU interaction.
/// @ingroup XLSample1
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Henry Mitchel, Intel Corporation.
///          Tim Whisonant, Intel Corporation.
///          Sadruta Chandrashekar, Intel Corporation.
///
/// This Sample demonstrates the following:
///    - The basic structure of an AAL program using the XL APIs.
///    - The ISampleAFUPing interface of SampleAFU1.
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
/// 08/29/2007     JG       Initial version started based on older sample code
/// 04/09/2008     JG       Added semaphore to protect against premature exit
/// 05/10/2008     HM       Comments & License
/// 06/24/2008     HM       Added <stdlib.h> for gcc 4.3.1 for exit()
/// 01/04/2009     HM       Updated Copyright
/// 07/22/2010     AG       SampleAFU2 now registers with AIA
/// 09/30/2011     JG       Renamed Sample 1. Ported to SDK 3.0
/// 06/18/2014     TSW      Ported to XL.@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
using namespace AAL;
using namespace AAS;

#include <aalsdk/xlRuntime.h>
using namespace XL;
using namespace RT;

#include "SampleAFU1Service.h"      // AFU package specific definitions
#include <aalsdk/AALLoggerExtern.h> // Logger

// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

/// @addtogroup XLSample1
/// @{

class MyRuntimeClient;  ///< Forward reference

/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class MyServiceClient : public CAASBase, public AAL::AAS::IServiceClient, public AAL::ISampleAFUPingClient
{
public:
   MyServiceClient();
   
   void start(MyRuntimeClient *p_Runtime);

   // <begin IServiceClient interface>
   void serviceAllocated(AAL::IBase          *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent        &rEvent);

   void serviceFreed(TransactionID const &rTranID);

   void PingReceived(AAL::TransactionID const &rTranID);
   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

protected:
   AAL::IAALService *m_pAALService;    // The generic AAL Service interface for the AFU.
   ISampleAFUPing   *m_pPingAFU;       // The AFU-specific interface.
   MyRuntimeClient  *m_pRuntime;
};

/// @brief   Define our Runtime client class so that we can receive the runtime started/stopped notifications.
///
/// We implement a Service client within, to handle AAL Service allocation/free.
/// We also implement a Semaphore for synchronization with the AAL runtime.
class MyRuntimeClient : public CAASBase,
                        public AAL::XL::RT::IRuntimeClient
{
public:
   MyRuntimeClient();
   ~MyRuntimeClient();

   void Wait();
   void Post();

   // <begin IRuntimeClient interface>
   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded( AAL::IBase *pClient,
                                         TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);
   
   AAL::XL::RT::IRuntime* getRuntime();
   // <end IRuntimeClient interface>

protected:
   MyServiceClient        m_SvcClient; // To acquire an AFU from the AAL runtime.
   AAL::XL::RT::IRuntime *m_pRuntime;  // Our AAL XL runtime instance.
   CSemaphore             m_Sem;       // For synchronizing with the AAL runtime.
   AAL::XL::RT::Runtime   m_Runtime; 
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyRuntimeClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
MyRuntimeClient::MyRuntimeClient() :
    m_Runtime(),
    m_SvcClient(),
    m_pRuntime(NULL)
{
   NamedValueSet configArgs;

   // Publish our interface
   SetSubClassInterface(iidRuntimeClient, dynamic_cast<AAL::XL::RT::IRuntimeClient*>(this));

#if CONFIGURE_RUNTIME
   NamedValueSet configRecord;
   configRecord.Add(XLRUNTIME_CONFIG_BROKER_SERVICE,"libsamplebroker");
   configArgs.Add(XLRUNTIME_CONFIG_RECORD,configRecord);
#endif
   m_Sem.Create(0, 1);
   m_Runtime.start(this, configArgs);
}

MyRuntimeClient::~MyRuntimeClient()
{
    m_Sem.Destroy();
}

 void MyRuntimeClient::Wait()
 {
    m_Sem.Wait();
 }

 void MyRuntimeClient::Post()
 {
    m_Sem.Post(1);
 }

 void MyRuntimeClient::runtimeStarted(IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    // Save a copy of our runtime interface instance.
    m_pRuntime = pRuntime;
    m_SvcClient.start(this);
 }

 void MyRuntimeClient::runtimeStopped(IRuntime *pRuntime)
 {
    MSG("Runtime stopped");
    Post();
 }

 void MyRuntimeClient::runtimeStartFailed(const IEvent &rEvent)
 {
    MSG("Runtime start failed");
    Post();
 }
 void MyRuntimeClient::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    MSG("Runtime AllocateService failed");
 }

 void MyRuntimeClient::runtimeAllocateServiceSucceeded( AAL::IBase *pClient,
                                                        TransactionID const &rTranID)
 {
    MSG("Runtime Allocate Service Succeeded");
 }

 void MyRuntimeClient::runtimeEvent(const IEvent &rEvent)
 {
    MSG("Generic message handler (runtime)");
 }

 AAL::XL::RT::IRuntime* MyRuntimeClient::getRuntime()
 {
    return m_pRuntime;
 }

///////////////////////////////////////////////////////////////////////////////
///
///  MyServiceClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
MyServiceClient::MyServiceClient():
    m_pRuntime(),
    m_pAALService(NULL),
    m_pPingAFU(NULL)
 {
    SetSubClassInterface(iidServiceClient, dynamic_cast<AAL::AAS::IServiceClient*>(this));
    SetInterface(iidSampleAFUPingClient, dynamic_cast<AAL::ISampleAFUPingClient*>(this));
 }


void MyServiceClient::start(MyRuntimeClient *p_Runtime)
{
   m_pRuntime = p_Runtime;

   // Request our AFU.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.
   NamedValueSet Manifest(strConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AFU 1");

   MSG("Allocating Sample AFU 1");

   #if DBG_HOOK
         cout << Manifest << endl;
   #endif // DBG_HOOK

   m_pRuntime->getRuntime()->allocService(dynamic_cast<AAL::IBase*>(this), Manifest);
}

 // We must implement the IServiceClient interface (IServiceClient.h):

 // <begin IServiceClient interface>
 void MyServiceClient::serviceAllocated(AAL::IBase          *pServiceBase,
                                        TransactionID const &rTranID)
 {
    m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
    ASSERT(NULL != m_pAALService);

    m_pPingAFU = subclass_ptr<ISampleAFUPing>(pServiceBase);
    ASSERT(NULL != m_pPingAFU);
    if ( NULL == m_pPingAFU ) {
       return;
    }

    MSG("Sample AFU 1 Allocated");

    // We have the Service, now to use it.
    m_pPingAFU->Ping("Hello World", TransactionID()); // This function now generates 5 event responses.
 }

 void MyServiceClient::serviceAllocateFailed(const IEvent        &rEvent)
 {
    MSG("Failed to allocate a Sample AFU 1");
    m_pRuntime->getRuntime()->stop();
 }

 void MyServiceClient::serviceFreed(TransactionID const &rTranID)
 {
    MSG("Sample AFU 1 Freed");
    m_pRuntime->getRuntime()->stop();
 }

 void MyServiceClient::PingReceived(AAL::TransactionID const &rTranID)
 {
    static int count = 0;
    // The Ping was received. This example shows how to extract the value of the TransactionID context
    // which was set in the transactionID sent with the ping command.

    MSG("got Ping Received " << count);
    ++count;
    if ( 5 == count ) {
	   MSG("got " << count << " Ping events. Releasing the Ping AFU.");
	   m_pAALService->Release(TransactionID());
    }
 }

 void MyServiceClient::serviceEvent(const IEvent &rEvent)
 {
    ERR("unexpected event 0x" << hex << rEvent.SubClassID());
 }
 // <end IServiceClient interface>

/// @} group XLSample1

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
   cout << "==========================" << endl;
   cout << " AAL Sample 1 Application"  << endl;
   cout << "==========================" << endl << endl;

   MyRuntimeClient RuntimeClient;
   
   RuntimeClient.Wait(); // For the ping events from ISampleAFUPing.
   MSG("Done");
   return 0;
}


/**
@addtogroup XLSample1
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>The basic structure of an AAL program using the serialized callback model.</li>
  <li>The ISampleAFUPing interface of SampleAFU1.</li>
  <li>Raw exposure of the the IRuntimeClient and IServiceClient Interfaces.</li>
  <li>Invoking a method on an AFU using a proprietary interface.</li>
</ul>

This sample is designed to be used with AFU1/SampleAFU1Service, a.k.a ISampleAFUPing.

1 Summary of Operation

XLSample1 instantiates an instance of the XL Runtime object, which then runs all
off its code from its Constructor.

In particular, it starts up the Runtime, which calls back into MyRuntimeClient::runtimeStarted().
This function then starts the IServiceClient in m_SvcClient by calling m_SvcClient.start().

In m_SvcClient.start() we see that the service is allocated, which will return in
MServiceClient::serviceAllocated().

There the ISampleAFUPing Interface is obtained and the Ping method on the Interface
invoked, which causes 5 calls back to MyServiceClient::PingReceived(). This method counts
to 5 and then Release's the Service.

This is a typical C++ way of coding, where an object is instantiated and the code
flows from that object's Constructor.

For a more typical linear flow shown in main() please see XLSyncSample1.

2 Running the application<br>

$ XLSample1

@} group XLSample1
*/
