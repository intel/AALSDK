// INTEL CONFIDENTIAL - For Intel Internal Use Only

// valapps/Read_Perf_Counters/main.cpp

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/utils/NLBVAFU.h>

using namespace std;
using namespace AAL;


#define HWAFU
//#define  ASEAFU


#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


// NLB_DSM_SIZE
// Buffer Size : CL(1)

// CSR_AFU_DSM_BASEL
// CSR_AFU_DSM_BASEH
// CSR_SRC_ADDR
// CSR_DST_ADDR
// CSR_CTL
// CSR_CFG
// CSR_NUM_LINES

// NLB_TEST_MODE_LPBK1

// #define DSM_STATUS_TEST_COMPLETE 0x40

class DoesNLBLpbk1
{
public:
   DoesNLBLpbk1(IBase   *pAALService,
                btWSSize TransferSize=CL(2),    // 2 cache lines by default
                btWSSize DSMSize=NLB_DSM_SIZE); // from NLBVAFU.h

   btInt NLBLpbk1(); // return 0 on success

protected:

   virtual void AllocateBuffers();
   virtual void FreeBuffers();

   IBase      *m_pAALService;    ///< IBase for valid AAL Service to NLB VAFU.
   btInt       m_Errors;         ///< Tracks number of errors encountered.
   btWSSize    m_TransferSize;   ///< Input/Output workspace size in bytes.
   btWSSize    m_DSMSize;        ///< DSM workspace size in bytes.

   IALIBuffer *m_pALIBufferIfc;  ///< Interface for allocating / freeing buffers.

   btVirtAddr  m_DSMVirt;
   btPhysAddr  m_DSMPhys;
   btVirtAddr  m_InputVirt;
   btPhysAddr  m_InputPhys;
   btVirtAddr  m_OutputVirt;
   btPhysAddr  m_OutputPhys;
};

DoesNLBLpbk1::DoesNLBLpbk1(IBase   *pAALService,
                           btWSSize TransferSize,
                           btWSSize DSMSize) :
   m_pAALService(pAALService),
   m_Errors(0),
   m_TransferSize(TransferSize),
   m_DSMSize(DSMSize),
   m_pALIBufferIfc(NULL),
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0)
{
   ASSERT(NULL != m_pAALService);
}

btInt DoesNLBLpbk1::NLBLpbk1()
{
   AllocateBuffers();

   // Get the Reset interface from the AAL Service.
   IALIReset *pALIResetIfc = dynamic_ptr<IALIReset>(iidALI_RSET_Service, m_pAALService);

   ASSERT(NULL != pALIResetIfc);
   if ( NULL == pALIResetIfc ) {
      ++m_Errors;
   }

   // Get the MMIO interface from the AAL Service.
   IALIMMIO *pALIMMIOIfc = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, m_pAALService);

   ASSERT(NULL != pALIMMIOIfc);
   if ( NULL == pALIMMIOIfc ) {
      ++m_Errors;
   }

   volatile nlb_vafu_dsm *pDSM;

   if ( 0 != m_Errors ) {
      goto _FREE;
   }

   // Do an AFU Reset.
   pALIResetIfc->afuReset();

   // Set the DSM base, high then low
   pALIMMIOIfc->mmioWrite64(CSR_AFU_DSM_BASEL, m_DSMPhys);

   // Assert NLB AFU reset
   pALIMMIOIfc->mmioWrite32(CSR_CTL, 0);

   // De-Assert NLB AFU reset
   pALIMMIOIfc->mmioWrite32(CSR_CTL, 1);

   // Set Input workspace address
   pALIMMIOIfc->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_InputPhys));

   // Set Output workspace address
   pALIMMIOIfc->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_OutputPhys));

   // Set the number of cache lines for the transfer
   pALIMMIOIfc->mmioWrite32(CSR_NUM_LINES, m_TransferSize / CL(1));

   // Set the transfer mode
   pALIMMIOIfc->mmioWrite32(CSR_CFG, NLB_TEST_MODE_LPBK1);

   pDSM = (volatile nlb_vafu_dsm *)m_DSMVirt;

   // Start the transfer
   pALIMMIOIfc->mmioWrite32(CSR_CTL, 3);

   MSG("Starting NLB Lpbk1 transfer");

   // Wait for transfer completion
   while ( 0 == pDSM->test_complete ) {
      SleepMicro(100);
   }

   MSG("NLB Lpbk1 transfer complete");

   // Stop the AFU
   pALIMMIOIfc->mmioWrite32(CSR_CTL, 7);


   // Verify the buffer contents. Output must match Input.
   if ( 0 != ::memcmp(m_InputVirt, m_OutputVirt, m_TransferSize) ) {
      ++m_Errors;
      ERR("NLB Lpbk1 buffer mismatch");
   }

_FREE:
   FreeBuffers();

   return m_Errors;
}

void DoesNLBLpbk1::AllocateBuffers()
{
   // Get the IALIBuffer associated with the AAL Service.
   m_pALIBufferIfc = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, m_pAALService);

   ASSERT(NULL != m_pALIBufferIfc);
   if ( NULL == m_pALIBufferIfc ) {
      ++m_Errors;
      return;
   }


   // Allocate the Device Status Memory.
   if ( ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_DSMSize, &m_DSMVirt) ) {
      ++m_Errors;
      return;
   }
   ASSERT(NULL != m_DSMVirt);

   m_DSMPhys = m_pALIBufferIfc->bufferGetIOVA(m_DSMVirt);
   if ( 0 == m_DSMPhys ) {
      ++m_Errors;
      return;
   }
   ASSERT(0 != m_DSMPhys);


   // Allocate the Input (Source) buffer for the transfer.
   if ( ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_TransferSize, &m_InputVirt) ) {
      ++m_Errors;
      return;
   }
   ASSERT(NULL != m_InputVirt);

   m_InputPhys = m_pALIBufferIfc->bufferGetIOVA(m_InputVirt);
   if ( 0 == m_InputPhys ) {
      ++m_Errors;
      return;
   }
   ASSERT(0 != m_InputPhys);


   // Allocate the Output (Destination) buffer for the transfer.
   if ( ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_TransferSize, &m_OutputVirt) ) {
      ++m_Errors;
      return;
   }
   ASSERT(NULL != m_OutputVirt);

   m_OutputPhys = m_pALIBufferIfc->bufferGetIOVA(m_OutputVirt);
   if ( 0 == m_OutputPhys ) {
      ++m_Errors;
      return;
   }
   ASSERT(0 != m_OutputPhys);


   // Initialize the buffer contents.
   ::memset(m_DSMVirt,    0,    m_DSMSize);
   ::memset(m_InputVirt,  0xaf, m_TransferSize);
   ::memset(m_OutputVirt, 0xbe, m_TransferSize);
}

void DoesNLBLpbk1::FreeBuffers()
{
   if ( NULL == m_pALIBufferIfc ) {
      return;
   }

   ali_errnum_e res;

   if ( NULL != m_OutputVirt ) {
      res = m_pALIBufferIfc->bufferFree(m_OutputVirt);
      ASSERT(ali_errnumOK == res);
      m_OutputVirt = NULL;
      m_OutputPhys = 0;
   }

   if ( NULL != m_InputVirt ) {
      res = m_pALIBufferIfc->bufferFree(m_InputVirt);
      ASSERT(ali_errnumOK == res);
      m_InputVirt = NULL;
      m_InputPhys = 0;
   }

   if ( NULL != m_DSMVirt ) {
      res = m_pALIBufferIfc->bufferFree(m_DSMVirt);
      ASSERT(ali_errnumOK == res);
      m_DSMVirt = NULL;
      m_DSMPhys = 0;
   }

   m_pALIBufferIfc = NULL;
}

class PerfCounters
{
public:
   enum Index
   {
      VERSION = 0,
      READ_HIT,
      WRITE_HIT,
      READ_MISS,
      WRITE_MISS,
      EVICTIONS,
      PCIE0_READ,
      PCIE0_WRITE,
      PCIE1_READ,
      PCIE1_WRITE,
      UPI_READ,
      UPI_WRITE
   };

   PerfCounters(const NamedValueSet &nvs)
   {
      if ( nvs.Has(AALPERF_VERSION) ) {
         m_Values[VERSION].Valid = true;
         nvs.Get(AALPERF_VERSION, &m_Values[VERSION].Value);
      } else {
         m_Values[VERSION].Valid = false;
      }

      if ( nvs.Has(AALPERF_READ_HIT) ) {
         m_Values[READ_HIT].Valid = true;
         nvs.Get(AALPERF_READ_HIT, &m_Values[READ_HIT].Value);
      } else {
         m_Values[READ_HIT].Valid = false;
      }

      if ( nvs.Has(AALPERF_WRITE_HIT) ) {
         m_Values[WRITE_HIT].Valid = true;
         nvs.Get(AALPERF_WRITE_HIT, &m_Values[WRITE_HIT].Value);
      } else {
         m_Values[WRITE_HIT].Valid = false;
      }

      if ( nvs.Has(AALPERF_READ_MISS) ) {
         m_Values[READ_MISS].Valid = true;
         nvs.Get(AALPERF_READ_MISS, &m_Values[READ_MISS].Value);
      } else {
         m_Values[READ_MISS].Valid = false;
      }

      if ( nvs.Has(AALPERF_WRITE_MISS) ) {
         m_Values[WRITE_MISS].Valid = true;
         nvs.Get(AALPERF_WRITE_MISS, &m_Values[WRITE_MISS].Value);
      } else {
         m_Values[WRITE_MISS].Valid = false;
      }

      if ( nvs.Has(AALPERF_EVICTIONS) ) {
         m_Values[EVICTIONS].Valid = true;
         nvs.Get(AALPERF_EVICTIONS, &m_Values[EVICTIONS].Value);
      } else {
         m_Values[EVICTIONS].Valid = false;
      }

      if ( nvs.Has(AALPERF_PCIE0_READ) ) {
         m_Values[PCIE0_READ].Valid = true;
         nvs.Get(AALPERF_PCIE0_READ, &m_Values[PCIE0_READ].Value);
      } else {
         m_Values[PCIE0_READ].Valid = false;
      }

      if ( nvs.Has(AALPERF_PCIE0_WRITE) ) {
         m_Values[PCIE0_WRITE].Valid = true;
         nvs.Get(AALPERF_PCIE0_WRITE, &m_Values[PCIE0_WRITE].Value);
      } else {
         m_Values[PCIE0_WRITE].Valid = false;
      }

      if ( nvs.Has(AALPERF_PCIE1_READ) ) {
         m_Values[PCIE1_READ].Valid = true;
         nvs.Get(AALPERF_PCIE1_READ, &m_Values[PCIE1_READ].Value);
      } else {
         m_Values[PCIE1_READ].Valid = false;
      }

      if ( nvs.Has(AALPERF_PCIE1_WRITE) ) {
         m_Values[PCIE1_WRITE].Valid = true;
         nvs.Get(AALPERF_PCIE1_WRITE, &m_Values[PCIE1_WRITE].Value);
      } else {
         m_Values[PCIE1_WRITE].Valid = false;
      }

      if ( nvs.Has(AALPERF_UPI_READ) ) {
         m_Values[UPI_READ].Valid = true;
         nvs.Get(AALPERF_UPI_READ, &m_Values[UPI_READ].Value);
      } else {
         m_Values[UPI_READ].Valid = false;
      }

      if ( nvs.Has(AALPERF_UPI_WRITE) ) {
         m_Values[UPI_WRITE].Valid = true;
         nvs.Get(AALPERF_UPI_WRITE, &m_Values[UPI_WRITE].Value);
      } else {
         m_Values[UPI_WRITE].Valid = false;
      }
   }

   btBool             Valid(PerfCounters::Index i) const { return m_Values[i].Valid; }
   btUnsigned64bitInt Value(PerfCounters::Index i) const { return m_Values[i].Value; }

   btBool operator == (const PerfCounters &rhs) const;
   btBool operator != (const PerfCounters &rhs) const { return ! this->PerfCounters::operator == (rhs); }

protected:
   struct
   {
      btBool             Valid;
      btUnsigned64bitInt Value;
   } m_Values[12];
};

btBool PerfCounters::operator == (const PerfCounters &rhs) const
{
   btInt               i;
   PerfCounters::Index idx;

   for ( i = (btInt)PerfCounters::VERSION ;
            i <= (btInt)PerfCounters::UPI_WRITE ;
               ++i ) {

      idx = (PerfCounters::Index) i;

      if ( (  Valid(idx) && !rhs.Valid(idx) ) ||
           ( !Valid(idx) &&  rhs.Valid(idx) ) ) {
         return false;
      }

      if ( Valid(idx) ) {
         if ( Value(idx) != rhs.Value(idx) ) {
            return false;
         }
      }

   }

   return true;
}



class Read_Perf_CountersApp : public CAASBase,
                              public IRuntimeClient,
                              public IServiceClient
{
public:
   Read_Perf_CountersApp();

   btInt    Run();    ///< Return 0 if success
   btInt Errors() const { return m_Errors; }

   // <IRuntimeClient>
   void   runtimeCreateOrGetProxyFailed(IEvent const        &rEvent);
   void                  runtimeStarted(IRuntime            *pRuntime,
                                        const NamedValueSet &rConfigParms);
   void                  runtimeStopped(IRuntime            *pRuntime);
   void              runtimeStartFailed(const IEvent        &rEvent);
   void               runtimeStopFailed(const IEvent        &rEvent);
   void    runtimeAllocateServiceFailed(IEvent const        &rEvent);
   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);
   void                    runtimeEvent(const IEvent        &rEvent);
   // </IRuntimeClient>

   // <IServiceClient>
   void      serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &rTranID);
   void serviceAllocateFailed(const IEvent        &rEvent);
   void       serviceReleased(const TransactionID &rTranID);
   void serviceReleaseRequest(IBase               *pServiceBase,
                              const IEvent        &rEvent);
   void  serviceReleaseFailed(const IEvent        &rEvent);
   void          serviceEvent(const IEvent        &rEvent);
   // </IServiceClient>

protected:
   IBase * AllocateService();
   void    FreeService();

   void sw_perfc_01();
   void sw_perfc_02();


   Runtime     m_Runtime;       ///< AAL Runtime
   IBase      *m_pAALService;   ///< The generic AAL Service interface for the AFU.
   IALIBuffer *m_pALIBufferIfc; ///< Pointer to Buffer Service
   IALIMMIO   *m_pALIMMIOIfc;   ///< Pointer to MMIO Service
   IALIReset  *m_pALIResetIfc;  ///< Pointer to AFU Reset Service
   btInt       m_Errors;        ///< Returned result value; 0 if success
   CSemaphore  m_Sem;           ///< For synchronizing with the AAL runtime.
};

Read_Perf_CountersApp::Read_Perf_CountersApp() :
   m_Runtime(this),
   m_pAALService(NULL),
   m_pALIBufferIfc(NULL),
   m_pALIMMIOIfc(NULL),
   m_pALIResetIfc(NULL),
   m_Errors(0)
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
   //   the runtimeStarted() or runtimeStartFailed() callbacks should set m_Errors appropriately.
   if ( !m_Runtime.start(configArgs) ) {
      ++m_Errors;
      return;
   }

   m_Sem.Wait();
}

void Read_Perf_CountersApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeCreateOrGetProxyFailed()");
}

void Read_Perf_CountersApp::runtimeStarted(IRuntime            *pRuntime,
                                           const NamedValueSet &rConfigParms)
{
   MSG("Runtime Started");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime Stopped");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::runtimeStartFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStartFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::runtimeStopFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStopFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::runtimeAllocateServiceFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeAllocateServiceFailed()");
   PrintExceptionDescription(rEvent);
}

void Read_Perf_CountersApp::runtimeAllocateServiceSucceeded(IBase               *pClient,
                                                            TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void Read_Perf_CountersApp::runtimeEvent(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("runtimeEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}

IBase * Read_Perf_CountersApp::AllocateService()
{
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
   ConfigRecord.Add(keyRegAFU_ID, "C000C966-0D82-4272-9AEF-FE5F84570612");


   // indicate that this service needs to allocate an AIAService, too to talk to the HW
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

#elif defined ( ASEAFU )         /* Use ASE based RTL simulation */

   Manifest.Add(keyRegHandle, 20);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

#else
# error Must define HWAFU or ASEAFU.
#endif // HWAFU

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");

   MSG("Allocating Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_Errors appropriately.

   // We are the service client.
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);

   m_Sem.Wait();

   return m_pAALService;
}

void Read_Perf_CountersApp::FreeService()
{
   MSG("Freeing Service");

   ASSERT(NULL != m_pAALService);
   IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);

   ASSERT(NULL != pIAALService);
   pIAALService->Release(TransactionID());

   m_Sem.Wait();
}

void Read_Perf_CountersApp::serviceAllocated(IBase               *pServiceBase,
                                             TransactionID const &rTranID)
{
   // Save the IBase for the Service. Through it we can get any other
   //  interface implemented by the Service
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);
   if ( NULL == m_pAALService ) {
      ++m_Errors;
      ERR("returned service was NULL");
      m_Sem.Post(1);
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("Failed to allocate Service");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::serviceReleased(TransactionID const &rTranID)
{
   MSG("Service Released");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::serviceReleaseRequest(IBase        *pServiceBase,
                                                  const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect serviceReleaseRequest()");
   if ( NULL != m_pAALService ) {
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
      ASSERT(NULL != pIAALService);
      pIAALService->Release(TransactionID());
   }
}

void Read_Perf_CountersApp::serviceReleaseFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect serviceReleaseFailed()");
   m_Sem.Post(1);
}

void Read_Perf_CountersApp::serviceEvent(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("serviceEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}

btInt Read_Perf_CountersApp::Run()
{
   // Get a pointer to our NLB AAL Service.
   IBase *pAALService = AllocateService();

   if ( NULL == pAALService ) {
      goto _STOP;
   }

   sw_perfc_01();
   sw_perfc_02();

   FreeService();

_STOP:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Errors;
}

void Read_Perf_CountersApp::sw_perfc_01()
{
   // Null test: Get performance counters. Do nothing. Get them again. They should not change.

   // Get the IALIPerf associated with the AAL Service.
   IALIPerf *pALIPerfIfc = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, m_pAALService);

   ASSERT(NULL != pALIPerfIfc);
   if ( NULL == pALIPerfIfc ) {
      ++m_Errors;
      return;
   }

   NamedValueSet NVS1;
   btBool        res;

   res = pALIPerfIfc->performanceCountersGet(&NVS1);
   if ( !res ) {
      ++m_Errors;
      ERR("1st performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters1(NVS1);

   NamedValueSet NVS2;
   NamedValueSet OptArgs;

   res = pALIPerfIfc->performanceCountersGet(&NVS2, OptArgs);
   if ( !res ) {
      ++m_Errors;
      ERR("2nd performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters2(NVS2);

   if ( Counters1 != Counters2 ) {
      ++m_Errors;
      ERR("SW-PERFC-01 FAIL");
   } else {
      MSG("SW-PERFC-01 PASS");
   }
}

void Read_Perf_CountersApp::sw_perfc_02()
{
   // Active test: Get performance counters. Run any NLB test. Get the performance counters again.
   // If they changed, then the SW is accessing the HW. Test Passes.

   // Perhaps add some sanity checking on returned value (!= 0xFFFF, value_before < value_after, ...)

   //   DoesNLBLpbk1 lpbk1(pAALService);
   //   m_Errors += lpbk1.NLBLpbk1();

}


int main(int argc, char *argv[])
{
   Read_Perf_CountersApp TheApp;

   if ( TheApp.Errors() > 0 ) {
      // something bad happened during Runtime startup.
      return TheApp.Errors();
   }

   return TheApp.Run();
}

