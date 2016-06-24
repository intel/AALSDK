// INTEL CONFIDENTIAL - For Intel Internal Use Only
#include "vallib.h"

AllocatesAALService::AllocatesAALService() :
   m_pAALService(NULL),
   m_Errors(0)
{
   m_Sem.Create(0, 1);
   SetInterface(iidServiceClient, dynamic_cast<AAL::IServiceClient *>(this));
}

void AllocatesAALService::Free()
{
   if ( NULL != m_pAALService ) {
      MSG("Freeing " << ServiceDescription() << " Service");

      AAL::IAALService *pIAALService = dynamic_ptr<AAL::IAALService>(iidService, m_pAALService);

      ASSERT(NULL != pIAALService);
      pIAALService->Release(AAL::TransactionID());
   } else {
      m_Sem.Post(1);
   }
}

void AllocatesAALService::serviceAllocated(AAL::IBase               *pServiceBase,
                                           AAL::TransactionID const &rTranID)
{
   // Save the IBase for the Service. Through it we can get any other
   //  interface implemented by the Service
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);
   if ( NULL == m_pAALService ) {
      ++m_Errors;
      ERR("returned " << ServiceDescription() << " service was NULL");
      m_Sem.Post(1);
      return;
   }

   MSG(ServiceDescription() << " Service Allocated");
   m_Sem.Post(1);
}

void AllocatesAALService::serviceAllocateFailed(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR("Failed to allocate " << ServiceDescription() << " Service");
   m_Sem.Post(1);
}

void AllocatesAALService::serviceReleased(AAL::TransactionID const &rTranID)
{
   MSG(ServiceDescription() << " Service Released");
   if ( rTranID.Context() != (AAL::btApplicationContext)1 ) {
      // Don't post the semaphore if this happened because of serviceReleaseRequest().
      m_Sem.Post(1);
   }
   m_pAALService = NULL;
}

void AllocatesAALService::serviceReleaseRequest(AAL::IBase        *pServiceBase,
                                                const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR(ServiceDescription() << " didn't expect serviceReleaseRequest()");
   if ( NULL != m_pAALService ) {
      AAL::IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
      ASSERT(NULL != pIAALService);

      // Breadcrumb that this Release resulted from serviceReleaseRequest().
      AAL::TransactionID tid((AAL::btApplicationContext)1);

      pIAALService->Release(tid);
   }
}

void AllocatesAALService::serviceReleaseFailed(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR(ServiceDescription() << " didn't expect serviceReleaseFailed()");
   m_Sem.Post(1);
}

void AllocatesAALService::serviceEvent(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR(ServiceDescription() << " serviceEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}

////////////////////////////////////////////////////////////////////////////////

void AllocatesNLBLpbk1AFU::Allocate(AAL::IRuntime *pRuntime)
{
   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   AAL::NamedValueSet Manifest;
   AAL::NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */

   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID, "D8424DC4-A4A3-C413-F89E-433683F9040B");

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

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "NLBLpbk1");

   MSG("Allocating NLB Lpbk1 Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_Errors appropriately.

   // We are the service client.
   pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
}

////////////////////////////////////////////////////////////////////////////////

void AllocatesFME::Allocate(AAL::IRuntime *pRuntime)
{
   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   AAL::NamedValueSet Manifest;
   AAL::NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */

   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID, "BFAF2AE9-4A52-46E3-82FE-38F0F9E17764");

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

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "FME");

   MSG("Allocating FME Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_Errors appropriately.

   // We are the service client.
   pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
}


void AllocatesPort::Allocate(AAL::IRuntime *pRuntime)
{
   MSG("Allocating PORT Service Enter");
   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   AAL::NamedValueSet Manifest;
   AAL::NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */

   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID, "3AB49893-138D-42EB-9642-B06C6B355B87");

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

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "PORT");

   MSG("Allocating PORT Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_Errors appropriately.

   // We are the service client.
   pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
}
////////////////////////////////////////////////////////////////////////////////

DoesNLBLpbk1::DoesNLBLpbk1(AAL::IBase   *pAALService,
                           AAL::btWSSize TransferSize,
                           AAL::btWSSize DSMSize) :
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

AAL::btInt DoesNLBLpbk1::NLBLpbk1()
{
   AllocateBuffers();

   // Get the Reset interface from the AAL Service.
   AAL::IALIReset *pALIResetIfc = dynamic_ptr<AAL::IALIReset>(iidALI_RSET_Service, m_pAALService);

   ASSERT(NULL != pALIResetIfc);
   if ( NULL == pALIResetIfc ) {
      ++m_Errors;
   }

   // Get the MMIO interface from the AAL Service.
   AAL::IALIMMIO *pALIMMIOIfc = dynamic_ptr<AAL::IALIMMIO>(iidALI_MMIO_Service, m_pAALService);

   ASSERT(NULL != pALIMMIOIfc);
   if ( NULL == pALIMMIOIfc ) {
      ++m_Errors;
   }

   volatile AAL::nlb_vafu_dsm *pDSM;

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
   m_pALIBufferIfc = dynamic_ptr<AAL::IALIBuffer>(iidALI_BUFF_Service, m_pAALService);

   ASSERT(NULL != m_pALIBufferIfc);
   if ( NULL == m_pALIBufferIfc ) {
      ++m_Errors;
      return;
   }


   // Allocate the Device Status Memory.
   if ( AAL::ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_DSMSize, &m_DSMVirt) ) {
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
   if ( AAL::ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_TransferSize, &m_InputVirt) ) {
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
   if ( AAL::ali_errnumOK != m_pALIBufferIfc->bufferAllocate(m_TransferSize, &m_OutputVirt) ) {
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

   AAL::ali_errnum_e res;

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

////////////////////////////////////////////////////////////////////////////////

PerfCounters::PerfCounters(const AAL::NamedValueSet &nvs)
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

AAL::btBool PerfCounters::operator == (const PerfCounters &rhs) const
{
   AAL::btInt          i;
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

#define PC_STREAMER(__x)                              \
if ( pc.Valid(__x) ) {                                \
   os << #__x << " : " << pc.Value(__x) << std::endl; \
}

std::ostream & operator << (std::ostream &os, const PerfCounters &pc)
{
   PC_STREAMER(PerfCounters::VERSION)
   PC_STREAMER(PerfCounters::READ_HIT)
   PC_STREAMER(PerfCounters::WRITE_HIT)
   PC_STREAMER(PerfCounters::READ_MISS)
   PC_STREAMER(PerfCounters::WRITE_MISS)
   PC_STREAMER(PerfCounters::EVICTIONS)
   PC_STREAMER(PerfCounters::PCIE0_READ)
   PC_STREAMER(PerfCounters::PCIE0_WRITE)
   PC_STREAMER(PerfCounters::PCIE1_READ)
   PC_STREAMER(PerfCounters::PCIE1_WRITE)
   PC_STREAMER(PerfCounters::UPI_READ)
   PC_STREAMER(PerfCounters::UPI_WRITE)
   return os;
}

#undef PC_STREAMER

