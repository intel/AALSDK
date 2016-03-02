#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>

#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/service/IVTP.h>

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
//#define LPBK1_BUFFER_SIZE        (MB(8)-64)
#define LPBK1_BUFFER_SIZE        (MB(64)-CL(1))
#define LPBK1_BUFFER_OFFSET      (0)

#define LPBK1_DSM_SIZE           MB(4)
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define CSR_NUM_LINES            0x0130
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114
#	define NLB_TEST_MODE_PCIE0		0x2000

/// @addtogroup HelloALIVTPNLB
/// @{


/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class HelloALIVTPNLBApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

   HelloALIVTPNLBApp();
   ~HelloALIVTPNLBApp();

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
   Runtime        m_Runtime;                ///< AAL Runtime
   IBase         *m_pALIAFU_AALService;     ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService;      ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;        ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;       ///< Pointer to AFU Reset Service
   CSemaphore     m_Sem;                    ///< For synchronizing with the AAL runtime.
   btInt          m_Result;                 ///< Returned result value; 0 if success
   TransactionID  m_ALIAFUTranID;           ///< TransactionID used for service allocation

   // VTP service-related information
   IBase         *m_pVTP_AALService;        ///< The generic AAL Service interface for the VTP.
   IVTP          *m_pVTPService;            ///< Pointer to VTP buffer service
   btCSROffset    m_VTPDFHOffset;           ///< VTP DFH offset
   TransactionID  m_VTPTranID;              ///< TransactionID used for service allocation

};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
HelloALIVTPNLBApp::HelloALIVTPNLBApp() :
   m_Runtime(this),
   m_pALIAFU_AALService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_pVTP_AALService(NULL),
   m_pVTPService(NULL),
   m_VTPDFHOffset(-1),
   m_Result(0),
   m_ALIAFUTranID(),
   m_VTPTranID()
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
HelloALIVTPNLBApp::~HelloALIVTPNLBApp()
{
   m_Sem.Destroy();
}

btInt HelloALIVTPNLBApp::run()
{
   cout <<"========================"<<endl;
   cout <<"= Hello ALI NLB Sample ="<<endl;
   cout <<"========================"<<endl;

   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   btUnsigned32bitInt x;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612");


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
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");

   MSG("Allocating ALIAFU Service");

   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, m_ALIAFUTranID);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("ALIAFU allocation failed\n");
      goto done_0;
   }

   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

      m_pALIResetService->afuReset();

      m_pALIMMIOService->mmioWrite32(4, (btUnsigned32bitInt)0xDEADBEEF);

      SleepMicro(1000);

      m_pALIMMIOService->mmioWrite32(0, (btUnsigned32bitInt)1);

      SleepMicro(3000);

      x = 0;

      m_pALIMMIOService->mmioRead32(4, &x);
      m_pALIMMIOService->mmioWrite32(0, (btUnsigned32bitInt)2);
      printf("Read x = 0x%0x\n", x);

      SleepMicro(1000);

      m_pALIMMIOService->mmioRead32(4, &x);
      m_pALIMMIOService->mmioWrite32(0, (btUnsigned32bitInt)0);
      printf("Read x = 0x%0x\n", x);

      SleepMicro(3000);

      m_pALIMMIOService->mmioRead32(4, &x);
      printf("Read x = 0x%0x\n", x);

      SleepMicro(500000);

      MSG("Done Running Test");
   }

done_1:
   (dynamic_ptr<IAALService>(iidService, m_pALIAFU_AALService))->Release(TransactionID());
   m_Sem.Wait();

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void HelloALIVTPNLBApp::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   // This application will allocate two different services (HWALIAFU and
   //  VTPService). We can tell them apart here by looking at the TransactionID.
   if (rTranID ==  m_ALIAFUTranID) {

      // Save the IBase for the Service. Through it we can get any other
      //  interface implemented by the Service
      m_pALIAFU_AALService = pServiceBase;
      ASSERT(NULL != m_pALIAFU_AALService);
      if ( NULL == m_pALIAFU_AALService ) {
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
   }
   else if (rTranID == m_VTPTranID) {

      // Save the IBase for the VTP Service.
       m_pVTP_AALService = pServiceBase;
       ASSERT(NULL != m_pVTP_AALService);
       if ( NULL == m_pVTP_AALService ) {
          m_bIsOK = false;
          return;
       }

       // Documentation says VTP Service publishes
       //    IVTP as subclass interface. Used for allocating shared
       //    buffers that support virtual addresses from AFU
       m_pVTPService = dynamic_ptr<IVTP>(iidVTPService, pServiceBase);
       ASSERT(NULL != m_pVTPService);
       if ( NULL == m_pVTPService ) {
          m_bIsOK = false;
          return;
       }
   }
   else
   {
      ERR("Unknown transaction ID encountered on serviceAllocated().");
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void HelloALIVTPNLBApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void HelloALIVTPNLBApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void HelloALIVTPNLBApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void HelloALIVTPNLBApp::serviceEvent(const IEvent &rEvent)
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

 void HelloALIVTPNLBApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void HelloALIVTPNLBApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void HelloALIVTPNLBApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void HelloALIVTPNLBApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void HelloALIVTPNLBApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void HelloALIVTPNLBApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void HelloALIVTPNLBApp::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }
 // <begin IRuntimeClient interface>

/// @} group HelloALIVTPNLB


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
   pAALLogger()->AddToMask(LM_All, LOG_INFO);
   HelloALIVTPNLBApp theApp;
   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

