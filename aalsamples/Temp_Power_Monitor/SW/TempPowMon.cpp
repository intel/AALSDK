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
/// @file TempPowMon.cpp
/// @brief Basic AFU interaction.
/// @ingroup TempPowMon
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Sadruta Chandrashekar, Intel Corporation.
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
/// 01/27/2016     SC       Initial version started based on older sample code.@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger
#include <signal.h>
#include <aalsdk/service/IALIAFU.h>

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

#define TMP_RDSENSOR 0x1010
#define PM_RDVR      0x2010

#define tempBitMask   0x00000000000000FFULL
#define seqBitMask    0x0000000000FFFF00ULL

#define powerBitMask  0x00000000FFFF0000ULL
#define powerUnits    0.09765625
#define voltage       1 //placeholder for voltage value to convert amps to wattage

// doxygen hACK to generate correct class diagrams
#define RuntimeClient TempPowMonRuntimeClient
/// @addtogroup HelloAAL
/// @{

/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class TempPowMonApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:
   TempPowMonApp();
   ~TempPowMonApp();
   /// @brief Called by the main part of the application,Returns 0 if Success
   ///
   /// Application Requests Service using Runtime Client passing a pointer to self.
   /// Blocks calling thread from [Main} untill application is done.
   btInt run(); //Return 0 if success

   void getTemp();
   void getPower();

   btBool isOK()  {return m_bIsOK;}

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleaseFailed(const IEvent &rEvent);

   void serviceReleased(TransactionID const &rTranID);

   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);

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
   CSemaphore        m_Sem;            // For synchronizing with the AAL runtime.
   btInt             m_Result;         // Returned result value; 0 if success
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyServiceClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
TempPowMonApp::TempPowMonApp() :
   m_pAALService(NULL),
   m_Runtime(this),
   m_pALIMMIOService(NULL),
   m_Result(0)
{
    // Publish our interfaces
    SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
    SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));

    m_Sem.Create(0, 1);

    NamedValueSet configArgs;
    NamedValueSet configRecord;

    configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
    configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

    if(!m_Runtime.start(configArgs)){
       m_bIsOK = false;
       return;
    }

    m_Sem.Wait();
    m_bIsOK = true;
}

TempPowMonApp::~TempPowMonApp()
{
   m_Runtime.stop();
   m_Sem.Destroy();
}

void TempPowMonApp::getTemp()
{
   btUnsigned32bitInt tempRdSensor;
   m_pALIMMIOService->mmioRead32(TMP_RDSENSOR, &tempRdSensor);

   btUnsigned32bitInt tempValue = tempRdSensor & tempBitMask;
   cout << "Temperature = " << tempValue << " Deg Celcius.\n";

   btUnsigned32bitInt seqNum = (tempRdSensor & seqBitMask) >> 8;
   cout << "Temp Sequence Number = " << seqNum << " \n";

}

void TempPowMonApp::getPower()
{
   btUnsigned64bitInt powerRdVr;
   m_pALIMMIOService->mmioRead64(PM_RDVR, &powerRdVr);

   btInt AmpsValue = (powerRdVr & powerBitMask) >> 16;
   btFloat powerValue = AmpsValue * powerUnits * voltage;

   cout << "Power = " << powerValue << " Amps.\n";

}

btInt TempPowMonApp::run()
{

   cout <<"===================================="<<endl;
   cout <<"= Temperature Power Monitor Sample ="<<endl;
   cout <<"===================================="<<endl;

   // Request our AFU.

   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
   ConfigRecord.Add(keyRegAFU_ID,"BFAF2AE9-4A52-46E3-82FE-38F0F9E17764"); //FME AFU ID

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Temp Power Monitor");

   MSG("Allocating Service");

   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();

   if(!m_bIsOK){
      ERR("Allocation failed\n");
      ++m_Result;
      goto done_0;
   }

    getTemp();
    getPower();

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
void TempPowMonApp::serviceAllocated(IBase *pServiceBase,
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

void TempPowMonApp::serviceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Failed to allocate a Service");
   ERR(pExEvent->Description());
   m_bIsOK = false;
   m_Sem.Post(1);
}

 void TempPowMonApp::serviceReleaseFailed(const IEvent        &rEvent)
{
    MSG("Failed to Release a Service");
    m_bIsOK = false;
    m_Sem.Post(1);
 }

 void TempPowMonApp::serviceReleased(TransactionID const &rTranID)
 {
    MSG("Service Released");
    m_Sem.Post(1);
}

 void TempPowMonApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
  {
     MSG("Service unexpected requested back");
     if(NULL != m_pAALService){
        IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
        ASSERT(pIAALService);
        pIAALService->Release(TransactionID());
     }
  }

void TempPowMonApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}
// <end IServiceClient interface>

void TempPowMonApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   MSG("Runtime Create or Get Proxy failed");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void TempPowMonApp::runtimeStarted( IRuntime *pRuntime,
                                const NamedValueSet &rConfigParms)
{
   m_bIsOK = true;
   m_Sem.Post(1);
}

void TempPowMonApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void TempPowMonApp::runtimeStartFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime start failed");
   ERR(pExEvent->Description());
   m_Sem.Post(1);
}

void TempPowMonApp::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
    m_Sem.Post(1);
}

void TempPowMonApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime AllocateService failed");
   ERR(pExEvent->Description());
}

void TempPowMonApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
   TransactionID const * foo = &rTranID;
   MSG("Runtime Allocate Service Succeeded");
}

void TempPowMonApp::runtimeEvent(const IEvent &rEvent)
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

   TempPowMonApp         theApp;
   int result = 0;

   if(theApp.IsOK()){
      result = theApp.run();
   }else{
      MSG("App failed to initialize");
   }

   MSG("Done");
   return result;
}


