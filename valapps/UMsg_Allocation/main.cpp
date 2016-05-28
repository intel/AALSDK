// INTEL CONFIDENTIAL - For Intel Internal Use Only

// valapps/UMsg_Allocation/main.cpp

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>

#include <aalsdk/service/IALIAFU.h>

#include <string.h>

#define MAX_NLB_WKSPC_SIZE        CL(65536)
#define HIGH                      0xffffffff
#define LOW                       0x00000000

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


class UMsg_Client : public CAASBase,
                    public IRuntimeClient,
                    public IServiceClient
{
public:

   UMsg_Client();
   ~UMsg_Client();

   btInt run();                 ///< Return 0 if success

   btInt setUmsgHint();         ///< Return 0 if success

   btInt unsetUmsgHint();       ///< Return 0 if success

   btInt countUmsgs();          ///< Return 0 if success

   btInt VerifyUmsgWrites();    ///< Return 0 if success

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleased(const AAL::TransactionID&);

   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &);

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
   enum {
         AFU,
         PORT
      };

   Runtime               m_Runtime;           ///< AAL Runtime
   IBase                *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IBase                *m_pPORTService;      ///< The generic AAL Service interface for the PORT region.
   IALIBuffer           *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO             *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIUMsg             *m_pALIuMSGService;   ///< Pointer to uMSg Service
   CSemaphore            m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt                 m_Result;            ///< Returned result v; 0 if success

   // Workspace info
   btVirtAddr            m_UMsgVirt;          ///< UMsg workspace virtual address.
   btPhysAddr            m_UMsgPhys;          ///< UMsg workspace physical address.
   btWSSize              m_UMsgSize;          ///< UMsg workspace size in bytes.
};

UMsg_Client::UMsg_Client() :
   m_Runtime(this),
   m_pAALService(NULL),
   m_pPORTService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIuMSGService(NULL),
   m_Result(0),
   m_UMsgVirt(NULL),
   m_UMsgPhys(0),
   m_UMsgSize(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   // SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);


   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

   if(!m_Runtime.start(configArgs)){
      m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

UMsg_Client::~UMsg_Client()
{
   m_Sem.Destroy();
}


// <begin IServiceClient interface>
void UMsg_Client::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   if(rTranID.ID() == UMsg_Client::AFU){

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
      //    IALIUMsg as subclass interface
      m_pALIuMSGService = dynamic_ptr<IALIUMsg>(iidALI_UMSG_Service, pServiceBase);
      ASSERT(NULL != m_pALIuMSGService);
      if ( NULL == m_pALIuMSGService ) {
         m_bIsOK = false;
         return;
      }
   }
   else  if(rTranID.ID() == UMsg_Client::PORT){
      m_pPORTService = pServiceBase;
      ASSERT(NULL != m_pPORTService);
      if ( NULL == m_pPORTService ) {
         m_bIsOK = false;
         return;
      }
//       Documentation says HWALIAFU Service publishes
//          IALIMMIO as subclass interface. Used to set/get PORT Region
      m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
      ASSERT(NULL != m_pALIMMIOService);
      if ( NULL == m_pALIMMIOService ) {
         m_bIsOK = false;
         return;
      }
   }
   m_Sem.Post(1);
}

void UMsg_Client::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR( "Failed to allocate Service \n" );
   PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

void UMsg_Client::serviceReleased(TransactionID const &rTranID)
{
   MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

void UMsg_Client::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
   MSG( "Service Release requested \n" );
   if(NULL != pServiceBase){
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
      ASSERT(pIAALService);
      pIAALService->Release(TransactionID());
   }
}

void UMsg_Client::serviceReleaseFailed(const IEvent        &rEvent)
{
   ERR( "Failed to release a Service\n" );
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_Sem.Post(1);
}


void UMsg_Client::serviceEvent(const IEvent &rEvent)
{
   ERR( "unexpected event 0x" << hex << rEvent.SubClassID() );
}
// <end IServiceClient interface>

void UMsg_Client::runtimeStarted( IRuntime            *pRuntime,
                                    const NamedValueSet &rConfigParms)
{
   MSG("Runtime started");
   m_bIsOK = true;
   m_Sem.Post(1);
}

void UMsg_Client::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void UMsg_Client::runtimeStartFailed(const IEvent &rEvent)
{
   ERR( "Runtime start failed\n" );
   PrintExceptionDescription(rEvent);
}

void UMsg_Client::runtimeStopFailed(const IEvent &rEvent)
{
   ERR( "Runtime stop failed\n" );
   m_bIsOK = false;
   m_Sem.Post(1);
}

void UMsg_Client::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   ERR( "Runtime AllocateService failed\n" );
   PrintExceptionDescription(rEvent);
}

void UMsg_Client::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void UMsg_Client::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

btInt UMsg_Client::run()
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID, "A944F6E7-15D3-4D95-9452-15DBD47C76BD");

   ConfigRecord.Add(keyRegSubDeviceNumber,0);

   // indicate that this service needs to allocate an AIAService, too to talk to the HW
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "gt ALI UMsg");

   MSG("Allocating Service(s)");

   TransactionID afu_tid(UMsg_Client::AFU);
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, afu_tid);

   m_Sem.Wait();

   if(!m_bIsOK){
      ERR( "AFU Allocation failed \n" );
      m_Runtime.stop();
      m_Sem.Wait();
      return m_Result;
   }

    // Modify the manifest for the NLB AFU
    Manifest.Delete(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED);
    ConfigRecord.Delete(keyRegAFU_ID);

    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");
    ConfigRecord.Add(keyRegAFU_ID, "3AB49893-138D-42EB-9642-B06C6B355B87");
    Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // Allocate the AFU
   TransactionID port_tid(UMsg_Client::PORT);
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, port_tid);

   m_Sem.Wait();

   if(!m_bIsOK){
      ERR( "PORT service Allocation failed \n" );
      m_Runtime.stop();
      m_Sem.Wait();
   }

   MSG("Running Test\n");

   if(true == m_bIsOK){

      //   Verify that setting the hint bit in Umsgs turn on hint for that Umsg.

      if ( 0 == setUmsgHint())
      {
         MSG("Setting UMsg Hint was successful");
      }
      if ( 0 == unsetUmsgHint())
      {
         MSG("Unsetting UMsg Hint was successful");
      }
      if ( 0 == countUmsgs())
      {
         MSG("Number of Umsgs is the number supported by hardware");
      }
      if ( 0 == VerifyUmsgWrites())
      {
         MSG("UMsg Writes outside of the defined CL but within the 4KiB block doesn't SegFault");
      }
   }

   cout << endl;
   MSG("Done Running Test");

   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

   (dynamic_ptr<IAALService>(iidService, m_pPORTService))->Release(TransactionID());
   m_Sem.Wait();

   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;

 }

btInt UMsg_Client :: setUmsgHint()
{
   btUnsigned32bitInt umsg_hint;
   NamedValueSet nvs;

   nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)LOW); // Unset hint bit for all Umsgs.
   btBool ret = m_pALIuMSGService->umsgSetAttributes(nvs);

   m_pALIMMIOService->mmioRead32(0x2018, &umsg_hint);

   if (0x00000000 != umsg_hint){ //verify that Umsg hint is off

      ERR ("UMsg Hint already Set\n");
      return 1;
   }
   nvs.Delete(UMSG_HINT_MASK_KEY);

   nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)HIGH); // set hint bit for all Umsgs.
   ret = m_pALIuMSGService->umsgSetAttributes(nvs);

   m_pALIMMIOService->mmioRead32(0x2018, &umsg_hint);
   if (0xFFFFFFFF != umsg_hint){ //Verify that hint is set for all Umsgs.

      ERR("Failed to set Umsg Hint.\n");
      return 2;
   }

   return 0;
}


btInt UMsg_Client :: unsetUmsgHint()
{
   btUnsigned32bitInt umsg_hint;
   NamedValueSet nvs;

   nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)HIGH); // set hint bit for all Umsgs.
   btBool ret = m_pALIuMSGService->umsgSetAttributes(nvs);

   m_pALIMMIOService->mmioRead32(0x2018, &umsg_hint);

   if (0xFFFFFFFF != umsg_hint){ //verify that Umsg hint is on

      ERR ("UMsg Hint already off\n");
      return 1;
   }
   nvs.Delete(UMSG_HINT_MASK_KEY);

   nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)LOW); // unset hint bit for all Umsgs.
   ret = m_pALIuMSGService->umsgSetAttributes(nvs);

   m_pALIMMIOService->mmioRead32(0x2018, &umsg_hint);
   if (0x00000000 != umsg_hint){ //Verify that hint is unset for all Umsgs.

      ERR("Failed to unset Umsg Hint.\n");
      return 2;
   }

   return 0;
}

btInt UMsg_Client :: countUmsgs()
{
   //   Based on HW configuration, verify that the number of UMsgs returned
   //   is the number supported by the HW.

   btUnsignedInt numUmsg = m_pALIuMSGService->umsgGetNumber();

   if (8 != numUmsg){ // Number of supported UMsgs is 8.

      ERR ("number of UMsgs returned is NOT the number supported by the HW.\n ");
      return 1;
   }

   return 0;
}

btInt UMsg_Client :: VerifyUmsgWrites()
{
   //   Verify that writing UMsgs outside of defined CL but within the 4KiB block
   //   does nothing (no segfault, no HW interaction except a write to allocated
   //   memory).

   btUnsignedInt numUmsgs = m_pALIuMSGService->umsgGetNumber();

   m_UMsgVirt = m_pALIuMSGService->umsgGetAddress(0);
   if(NULL == m_UMsgVirt){
     ERR("No uMSG support");
     return 1;
   }

   btUnsignedInt numCLs = (numUmsgs * KB(4))/CL(1);
   btUnsignedInt i;
   for (i = 0; i < numCLs; i++ ){
      *(btUnsigned32bitInt *)(m_UMsgVirt + CL(i)) = HIGH;
   }

   return 0;
}


int main(int argc, char *argv[])
{

   cout <<"============================"<<endl;
   cout <<"= Verify UMsg Capabilities ="<<endl;
   cout <<"============================"<<endl;

   UMsg_Client theApp;
   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

