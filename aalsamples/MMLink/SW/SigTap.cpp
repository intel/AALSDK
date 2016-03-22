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
/// @file SigTap.cpp
/// @brief Basic AFU interaction.
/// @ingroup SigTap
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Sadruta Chandrashekar, Intel Corporation.
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
/// 04/10/2015     JG       Initial version started based on older sample code.
/// 01/19/2016     SC       SigTap version.@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger
#include <signal.h>
#include <aalsdk/service/IALIAFU.h>

#include "mmlink_server.h"
#include "mm_debug_link_interface.h"

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

#define MAX_FILENAME_SIZE (256)

// doxygen hACK to generate correct class diagrams
#define RuntimeClient SigTapRuntimeClient
/// @addtogroup HelloAAL
/// @{

/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class SigTapApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:
   SigTapApp();
   ~SigTapApp();
   /// @brief Called by the main part of the application,Returns 0 if Success
   ///
   /// Application Requests Service using Runtime Client passing a pointer to self.
   /// Blocks calling thread from [Main} untill application is done. 
   int run(mmlink_server *server, char* filename);

   btBool IsOK()  {return m_isOK;}

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
            TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleaseFailed(const IEvent &rEvent);

   void serviceReleased(TransactionID const &rTranID);
   virtual void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent){};  // Ignored TODO better implementation

   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

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
   IBase            *m_pAALService;    // The generic AAL Service interface for the AFU.
   Runtime           m_Runtime;
   IALIMMIO         *m_pALIMMIOService;
   btBool            m_isOK;
   CSemaphore        m_Sem;            // For synchronizing with the AAL runtime.
   int               m_Result;         // Returned result value; 0 if success
   btVirtAddr        m_mmio;
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyServiceClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
SigTapApp::SigTapApp() :
   m_pAALService(NULL),
   m_Runtime(this),
   m_pALIMMIOService(NULL),
   m_isOK(false),
   m_Result(0),
   m_mmio(NULL)
{
   // Publish our interfaces
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));

    m_Sem.Create(0, 1);

    NamedValueSet configArgs;
    NamedValueSet configRecord;

    configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
    configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

    m_Runtime.start(configArgs);

    m_Sem.Wait();
}

SigTapApp::~SigTapApp()
{
   m_Runtime.stop();
   m_Sem.Destroy();
}

int SigTapApp::run(mmlink_server *server, char* filename)
{

   cout <<"====================="<<endl;
   cout <<"= Signal Tap Sample ="<<endl;
   cout <<"====================="<<endl;

   btVirtAddr stpAddr = NULL;
   // Request our AFU.

   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
   ConfigRecord.Add(keyRegAFU_ID,"3AB49893-138D-42EB-9642-B06C6B355B87"); //PORT0 AFU ID

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Signal Tap");

   MSG("Allocating Service");

   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();

   if(!m_bIsOK){
      ERR("Allocation failed\n");
      ++m_Result;
      goto done_0;
   }

   // So signal tap
   if(m_isOK){
      stpAddr = m_pALIMMIOService->mmioGetAddress();
   }

   if (NULL != stpAddr){
      m_Result = server->run(stpAddr);
   }else{
      ERR("Failed to map STP region");
      m_Result = 1;
   }

    // Clean-up and return
    // Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

// We must implement the IServiceClient interface (IServiceClient.h):

// <begin IServiceClient interface>
void SigTapApp::serviceAllocated(IBase *pServiceBase,
                                 TransactionID const &rTranID)
{
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);

   m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);

   ASSERT(NULL != m_pALIMMIOService);
   if ( NULL == m_pALIMMIOService ) {
      return;
   }
   m_Sem.Post(1);
}

void SigTapApp::serviceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Failed to allocate a Service");
   ERR(pExEvent->Description());
   m_isOK = false;
   m_Sem.Post(1);
}

 void SigTapApp::serviceReleaseFailed(const IEvent        &rEvent)
{
    MSG("Failed to Release a Service");
    m_isOK = false;
    m_Sem.Post(1);
 }

 void SigTapApp::serviceReleased(TransactionID const &rTranID)
 {
    MSG("Service Released");
   m_Sem.Post(1);
}

void SigTapApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}
// <end IServiceClient interface>

void SigTapApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   MSG("Runtime Create or Get Proxy failed");
   m_isOK = false;
   m_Sem.Post(1);
}

void SigTapApp::runtimeStarted( IRuntime *pRuntime,
                                const NamedValueSet &rConfigParms)
{
   m_isOK = true;
   m_Sem.Post(1);
}

void SigTapApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_isOK = false;
   m_Sem.Post(1);
}

void SigTapApp::runtimeStartFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime start failed");
   ERR(pExEvent->Description());
   m_Sem.Post(1);
}

void SigTapApp::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
    m_Sem.Post(1);
}

void SigTapApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime AllocateService failed");
   ERR(pExEvent->Description());
}

void SigTapApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
   TransactionID const * foo = &rTranID;
   MSG("Runtime Allocate Service Succeeded");
}

void SigTapApp::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}


void int_handler(int sig)
{
   cerr<< "SIGINT: stopping the server\n";
}

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

   SigTapApp         theApp;
   int result = 0;
   mmlink_server *server;

   int 	 ip 	  = INADDR_ANY;
   int    port	  = 3333;
   char *sys_file = (char *)malloc (MAX_FILENAME_SIZE);

	signal(SIGINT, int_handler);

	for (int i = 1; i < argc; ++i)
	{
		sscanf(argv[i], "--ip=%d", &ip);
		sscanf(argv[i], "--port=%d", &port);
		sscanf(argv[i], "--sysfs=%s", sys_file);
	}

	struct sockaddr_in sock;
	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);
	sock.sin_addr.s_addr = htonl(ip);

   mm_debug_link_interface *driver = get_mm_debug_link();
   server = new mmlink_server(&sock, driver);

   //int err = server->run(sys_file);

   if(theApp.IsOK()){
      result = theApp.run(server, sys_file);
   }else{
      MSG("App failed to initialize");
   }

   MSG("Done");
   return result;
}


