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
/// @file ErrorMonitor.cpp
/// @brief Basic AFU interaction.
/// @ingroup ErrorMonitor
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Ananda Ravuri, Intel Corporation.
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
/// 03/09/2016     SC       Initial version started based on older sample code.@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger
#include <signal.h>
#include <aalsdk/service/IALIAFU.h>
#include <getopt.h>

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.

#ifdef MSG
# undef MSG
#endif // MSG
#if 0
   #define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#else
   #define MSG(x)
#endif
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


// command line parsing
#define GETOPT_STRING ":hcCB:D:F"

struct option longopts[] = {
      {"help",                no_argument,       NULL, 'h'},
      {"clear",               no_argument,       NULL, 'c'},
      {"Clear",               no_argument,       NULL, 'C'},
      {"bus-number",          required_argument, NULL, 'B'},
      {"device-number",       required_argument, NULL, 'D'},
      {"function-number",     required_argument, NULL, 'F'},
      {0,0,0,0}
};

// FME GUID
#define CCIP_FME_AFUID              "BFAF2AE9-4A52-46E3-82FE-38F0F9E17764"
// PORT GUID
#define CCIP_PORT_AFUID             "3AB49893-138D-42EB-9642-B06C6B355B87"


// doxygen HACK to generate correct class diagrams
#define RuntimeClient ErrorMonRuntimeClient
/// @addtogroup HelloAAL
/// @{

/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class ErrorMonApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:
   ErrorMonApp();
   ~ErrorMonApp();
   /// @brief Called by the main part of the application,Returns 0 if Success
   ///
   /// Application Requests Service using Runtime Client passing a pointer to self.
   /// Blocks calling thread from [Main} untill application is done.
   btInt run(int busnum, int devnum, int funnum,btBool bClear);    ///< Return 0 if success

   // Get FME Errors
   btBool getFMEError();

   // Get Port errors
   btBool getPortError();

   // Get FME Error masks
   btBool getFMEErrorMask();

   // Get PORT Error masks
   btBool getPortErrorMask();

   // Clears FME Errors
   btBool clearFMEErrors();

   // Clears Port Errors
   btBool clearPortErrors();

   // Prints All FME  Errors
   btBool printAllFMEErrors();

   // Prints All PORT Errors
   btBool printAllPortErrors();

   btBool isOK()  {return m_bIsOK;}

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleaseFailed(const IEvent &rEvent);

   void serviceReleased(TransactionID const &rTranID);

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

   void serviceReleaseRequest(IBase *pServiceBase,
                              const IEvent &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);
   // <end IRuntimeClient interface>

protected:
   enum {
      FME,
      PORT
   };
   IBase            *m_pFMEService;      // The generic AAL Service interface for the FME.
   IBase            *m_pPortService;     // The generic AAL Service interface for the Port.
   Runtime           m_Runtime;
   CSemaphore        m_Sem;              // For synchronizing with the AAL runtime.
   btInt             m_Result;           // Returned result value; 0 if success
   IALIFMEError     *m_pALIFMEError;     // FME ALI Error Interface
   IALIPortError    *m_pALIPortError;    // PORT ALI Error Interface
};

///////////////////////////////////////////////////////////////////////////////
///
///  MyServiceClient Implementation
///
///////////////////////////////////////////////////////////////////////////////
ErrorMonApp::ErrorMonApp() :
   m_pFMEService(NULL),
   m_pPortService(NULL),
   m_Runtime(this),
   m_Result(0),
   m_pALIFMEError(NULL),
   m_pALIPortError(NULL)
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

ErrorMonApp::~ErrorMonApp()
{
   m_Runtime.stop();
   m_Sem.Destroy();
}

btBool ErrorMonApp::clearFMEErrors()
{
   cout << endl<< "-----Clears All FME Error ----- "<< endl;

   // Clear All FME Errors
   if(!m_pALIFMEError->errorClearAll()) {
      return false;
   }
   return true ;
}

btBool ErrorMonApp::clearPortErrors()
{
   cout << endl<< "-----Clears All PORT Error ----- "<< endl;
   // Clear All PORT Errors
   if(!m_pALIPortError->errorClearAll()) {
      return false;
   }
   return true ;
}


btBool ErrorMonApp::getPortErrorMask()
{
   NamedValueSet Error;
   btUnsignedInt count = 0;

   // Get PORT Errors Masks
   if(!m_pALIPortError->errorGetMask(Error)) {
      return false;
   }

   cout << endl<< "-----PORT Error Mask ----- "<< endl;
   // Print PORT Error masks
   Error.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      Error.GetName(i,&type);
      std::cout << "Port Error Mask: " << type <<"  Set"<< std::endl;
   }
   return true ;
}

btBool ErrorMonApp::getFMEErrorMask()
{
   NamedValueSet Error;
   btUnsignedInt count = 0;

   // Get FME Errors Masks
   if(!m_pALIFMEError->errorGetMask(Error)) {
      return false;
   }

   cout << endl<< "-----FME Error Mask ----- "<< endl;
   // Print FME Error masks
   Error.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      Error.GetName(i,&type);
      std::cout << "FME Error Mask: " << type <<"  Set"<< std::endl;
   }
   return true ;
}

btBool ErrorMonApp::getFMEError()
{
   NamedValueSet fmeError;
   btUnsignedInt count   = 0;

   // Get FME Errors
   if(!m_pALIFMEError->errorGet(fmeError)) {
      return false;
   }

   cout <<endl << "-----FME Error ----- "<< endl;

   // Print FME Errors
   fmeError.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeError.GetName(i,&type);
      std::cout << "FME Error : " << type <<"  Set"<< std::endl;
   }

   fmeError.Empty();

   cout << endl<< "-----FME First& Next Error ----- "<< endl;
   // Get FME First and Next Errors
   if(!m_pALIFMEError->errorGetOrder(fmeError)) {
      return false;
   }

   // Print FME First and Next Errors
   fmeError.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeError.GetName(i,&type);
      std::cout << "Error : " << type <<"  Set"<< std::endl;
   }

   return true ;
}

btBool ErrorMonApp::getPortError()
{
   NamedValueSet portError;
   btUnsignedInt count       = 0;
   btUnsigned64bitInt value  = 0;

   // Get PORT Errors
   if(!m_pALIPortError->errorGet(portError)) {
      return false;
   }

   cout << endl<< "-----PORT Error ----- "<< endl;
   // Print PORT Errors
   portError.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      portError.GetName(i,&type);
      std::cout << "Port Error : " << type <<"  Set"<< std::endl;
   }

   portError.Empty();

   cout << endl<< "-----PORT First Error ----- "<< endl;
   // Get PORT First  Errors
   if(!m_pALIPortError->errorGetOrder(portError)) {
      return false;
   }

   // Prints PORT First  Errors
   portError.GetNumNames(&count);
   for(int i=0;i<count ;i++) {
      btStringKey type;
      portError.GetName(i,&type);
      std::cout << "Port First Error : " << type <<"  Set"<< std::endl;
   }

   portError.Empty();
   // Get PORT Errors
   if(!m_pALIPortError->errorGet(portError)) {
      return false;
   }
   portError.GetNumNames(&count);

   // Get Port Malformed Request if PORT Error
   if(count >0) {
      portError.Empty();

      if(!m_pALIPortError->errorGetPortMalformedReq(portError)) {
       return false;
      }

      cout << endl<< "-----PORT MalFormed Request----- "<< endl;

      if (portError.Has(AAL_ERR_PORT_MALFORMED_REQ_0)) {
         portError.Get( AAL_ERR_PORT_MALFORMED_REQ_0, &value);
         printf("PORT malformed request0 %llu \n",value);
      }

      if (portError.Has(AAL_ERR_PORT_MALFORMED_REQ_1)) {
         portError.Get( AAL_ERR_PORT_MALFORMED_REQ_1, &value);
         printf("PORT malformed request1 %llu \n",value);
      }
   }

   return true;
}

btBool ErrorMonApp::printAllFMEErrors()
{
   cout <<endl<<"-------Print ALL FME Errors ------"<<endl;
   if(!m_pALIFMEError->printAllErrors()) {
      return false;
   }

   return true;
}

btBool ErrorMonApp::printAllPortErrors()
{
   cout <<endl<<"-------Print ALL PORT Errors ------"<<endl;
   if(!m_pALIPortError->printAllErrors()) {
      return false;
   }

   return true;
}

btInt ErrorMonApp::run(int busnum, int devnum, int funnum,btBool bClear)
{

   cout <<"===================================="<<endl;
   cout <<"= Error Monitor Sample ="<<endl;
   cout <<"===================================="<<endl;

   // Request our AFU.

   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");

   // Modify the manifest for the PORT
   ConfigRecord.Add(keyRegAFU_ID,CCIP_PORT_AFUID);

   if (busnum >= 0) {
      cout << "Using PCIe bus 0x" << std::hex << busnum << std::dec << endl;
      ConfigRecord.Add(keyRegBusNumber, btUnsigned32bitInt(busnum));
   }
   if (devnum >= 0) {
      cout << "Using PCIe device 0x" << std::hex << devnum << std::dec << endl;
      ConfigRecord.Add(keyRegDeviceNumber, btUnsigned32bitInt(devnum));
   }
   if (funnum >= 0) {
      cout << "Using PCIe function 0x" << std::hex << funnum << std::dec << endl;
      ConfigRecord.Add(keyRegFunctionNumber, btUnsigned32bitInt(funnum));
   }

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Error Monitor");

   MSG("Allocating PORT Service");

   // Allocate the Service and wait for it to complete by sitting on the
   // semaphore. The serviceAllocated() callback will be called if successful.
   // If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
   // (Refer to the serviceAllocated() callback to see how the Service's interfaces
   // are collected.)
   {
      // Allocate the PORT Resource
      TransactionID port_tid(ErrorMonApp::PORT);
      m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, port_tid);
      m_Sem.Wait();
      if(!m_bIsOK){
         ERR("Allocation failed\n");
         goto done_0;
      }
   }

   // clears PORT Errors
   if(bClear) {
      if(!clearPortErrors()) {
            ++m_Result;   // record error
      }
   }

   // get PORT Error mask
   if(!getPortErrorMask()) {
       ++m_Result;   // record error
   }

   // Get PORT Errors
   if(!getPortError()) {
       ++m_Result;   // record error
   }

   //Prints all PORT errors
   if(!printAllPortErrors()) {
       ++m_Result;   // record error
   }


   // Modify the manifest for the FME
   Manifest.Delete(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED);
   ConfigRecord.Delete(keyRegAFU_ID);

   ConfigRecord.Add(keyRegAFU_ID,CCIP_FME_AFUID);
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   MSG("Allocating FME Service");

   // Allocate the Service and wait for it to complete by sitting on the
   // semaphore. The serviceAllocated() callback will be called if successful.
   // If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
   // (Refer to the serviceAllocated() callback to see how the Service's interfaces
   // are collected.)
   {
      // Allocate the FME Resource
      TransactionID fme_tid(ErrorMonApp::FME);
      m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, fme_tid);
      m_Sem.Wait();
      if(!m_bIsOK){
         ERR("Allocation failed\n");
         goto done_0;
      }
   }

   // clears FME Errors
   if(bClear) {
      if(!clearFMEErrors()) {
         ++m_Result;   // record error
      }
   }

   // Get FME Errors mask
   if(!getFMEErrorMask()) {
       ++m_Result;   // record error
   }

   // Get FME Errors
   if(!getFMEError()) {
      ++m_Result;   // record error
   }

   // Prints all FME Errors
   if(!printAllFMEErrors()) {
       ++m_Result;   // record error
   }

   // Clean-up and return
   // Release() the Service through the Services IAALService::Release() method
done_0:

   // Release PORT Resource
   if(m_pPortService) {
      (dynamic_ptr<IAALService>(iidService, m_pPortService))->Release(TransactionID());
      m_Sem.Wait();
   }

   // Release FME Resource
   if(m_pFMEService) {
      (dynamic_ptr<IAALService>(iidService, m_pFMEService))->Release(TransactionID());
      m_Sem.Wait();
   }

   // Stop Runtime
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

// We must implement the IServiceClient interface (IServiceClient.h):

// <begin IServiceClient interface>
void ErrorMonApp::serviceAllocated(IBase *pServiceBase,
                                   TransactionID const &rTranID)
{

   if(rTranID.ID() == ErrorMonApp::FME) {
      // FME Resource  Allocation
      m_pFMEService = pServiceBase;
      ASSERT(NULL != m_pFMEService);
      if ( NULL == m_pFMEService ) {
         m_bIsOK = false;
         return;
      }

      m_pALIFMEError = dynamic_ptr<IALIFMEError>(iidALI_FMEERR_Service, pServiceBase);
      ASSERT(NULL != m_pALIFMEError);
      if ( NULL == m_pALIFMEError ) {
         m_bIsOK = false;
         return;
      }


   } else if(rTranID.ID() == ErrorMonApp::PORT)  {
      //PORT Resource  Allocation
      m_pPortService = pServiceBase;
      ASSERT(NULL != m_pPortService);
      if ( NULL == m_pPortService ) {
         m_bIsOK = false;
         return;
      }

      m_pALIPortError = dynamic_ptr<IALIPortError>(iidALI_PORTERR_Service, pServiceBase);
      ASSERT(NULL != m_pALIPortError);
      if ( NULL == m_pALIPortError ) {
         m_bIsOK = false;
         return;
      }

   } else {
      // Wrong Transaction ID
      ERR("Failed to allocate Service");
      m_bIsOK = false;
      return;
   }

   m_Sem.Post(1);
}

void ErrorMonApp::serviceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Failed to allocate a Service");
   ERR(pExEvent->Description());
   m_bIsOK = false;
   m_Sem.Post(1);
}

 void ErrorMonApp::serviceReleaseFailed(const IEvent        &rEvent)
{
    MSG("Failed to Release a Service");
    m_bIsOK = false;
    m_Sem.Post(1);
 }

 void ErrorMonApp::serviceReleased(TransactionID const &rTranID)
 {
    MSG("Service Released");
    m_Sem.Post(1);
}

void ErrorMonApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}
// <end IServiceClient interface>

void ErrorMonApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   MSG("Runtime Create or Get Proxy failed");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ErrorMonApp::runtimeStarted( IRuntime *pRuntime,
                                const NamedValueSet &rConfigParms)
{
   m_bIsOK = true;
   m_Sem.Post(1);
}

void ErrorMonApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ErrorMonApp::runtimeStartFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime start failed");
   ERR(pExEvent->Description());
   m_Sem.Post(1);
}

void ErrorMonApp::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
    m_Sem.Post(1);
}

void ErrorMonApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime AllocateService failed");
   ERR(pExEvent->Description());
}

void ErrorMonApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                  TransactionID const &rTranID)
{
   TransactionID const * foo = &rTranID;
   MSG("Runtime Allocate Service Succeeded");
}

void ErrorMonApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
   MSG("Service unexpected requested back");
   if(NULL != pServiceBase){
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
      ASSERT(pIAALService);
      pIAALService->Release(TransactionID());
   }
}

void ErrorMonApp::runtimeEvent(const IEvent &rEvent)
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

   int result       = 0;
   int getopt_ret;
   int option_index;
   char *endptr     = NULL;
   btBool bClear    = false;
   int busnum       = -1;
   int devnum       = -1;
   int funnum       = -1;

   while( -1 != ( getopt_ret = getopt_long(argc, argv, GETOPT_STRING, longopts, &option_index))){
      const char *tmp_optarg = optarg;

      if((optarg) &&
         ('=' == *tmp_optarg)){
         ++tmp_optarg;
      }

      if((!optarg) &&
         (NULL != argv[optind]) &&
         ('-' != argv[optind][0]) ) {
         tmp_optarg = argv[optind++];
      }

      switch(getopt_ret){
         case 'h':
            printf("Usage:\n\t%s [-B <bus>] [-D <device>] [-F <function>] [ -c < Clear All Errors]\n\n",
                  argv[0]);
            return -2;
            break;

         case 'B':
            endptr = NULL;
            busnum = strtol(tmp_optarg, &endptr, 0);
            break;

         case 'D':
            endptr = NULL;
            devnum = strtol(tmp_optarg, &endptr, 0);
            break;

         case 'F':
            endptr = NULL;
            funnum = strtol(tmp_optarg, &endptr, 0);
            break;

         case 'c':
         case 'C':
            bClear = true ;
            break;

         case ':':   /* missing option argument */
            cout << "Missing option argument.\n";
            return -1;

         case '?':
         default:    /* invalid option */
            cout << "Invalid cmdline options.\n";
            return -1;
      }
   }

   ErrorMonApp      theApp;
   if(theApp.IsOK()){
      result = theApp.run( busnum, devnum, funnum, bClear );
   }else{
      MSG("App failed to initialize");
   }

   MSG("Done");
   return result;
}


