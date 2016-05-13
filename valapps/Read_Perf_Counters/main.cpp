// INTEL CONFIDENTIAL - For Intel Internal Use Only

// valapps/Read_Perf_Counters/main.cpp

#include "vallib.h"


class Read_Perf_CountersApp : public CAASBase,
                              public IRuntimeClient
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

protected:
   void sw_perfc_01(IBase *pAFUService, IBase *pFMEService);
   void sw_perfc_02(IBase *pAFUService, IBase *pFMEService);
   void sw_perfc_03(IBase *pAFUService, IBase *pFMEService);

   Runtime              m_Runtime;       ///< AAL Runtime
   btInt                m_Errors;        ///< Returned result value; 0 if success
   CSemaphore           m_Sem;           ///< For synchronizing with the AAL runtime.
   AllocatesNLBLpbk1AFU m_Lpbk1Allocator;
   AllocatesFME         m_FMEAllocator;
};

Read_Perf_CountersApp::Read_Perf_CountersApp() :
   m_Runtime(this),
   m_Errors(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
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

btInt Read_Perf_CountersApp::Run()
{
   // Get a pointer to an NLB Lpbk1 AAL Service.
   m_Lpbk1Allocator.Allocate(&m_Runtime);
   m_Lpbk1Allocator.Wait();

   IBase *pAFUService = m_Lpbk1Allocator.Service();
   IBase *pFMEService;

   if ( ( NULL == pAFUService ) || ( m_Lpbk1Allocator.Errors() > 0 ) ) {
      goto _STOP;
   }

   m_FMEAllocator.Allocate(&m_Runtime);
   m_FMEAllocator.Wait();

   pFMEService = m_FMEAllocator.Service();

   if ( ( NULL == pFMEService ) || ( m_FMEAllocator.Errors() > 0 ) ) {
      goto _LPBK1;
   }

   sw_perfc_01(pAFUService, pFMEService);
   sw_perfc_02(pAFUService, pFMEService);
   sw_perfc_03(pAFUService, pFMEService);

   m_FMEAllocator.Free();
   m_FMEAllocator.Wait();

_LPBK1:
   m_Lpbk1Allocator.Free();
   m_Lpbk1Allocator.Wait();

_STOP:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Errors + m_Lpbk1Allocator.Errors() + m_FMEAllocator.Errors();
}

void Read_Perf_CountersApp::sw_perfc_01(IBase *pAFUService, IBase *pFMEService)
{
   // Null test: Get performance counters. Do nothing. Get them again. They should not change.

   MSG("Begin SW-PERFC-01");

   // Get the IALIPerf associated with the FME AAL Service.
   IALIPerf *pALIPerfIfc = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, pFMEService);

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
      ERR("SW-PERFC-01: 1st performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters1(NVS1);

   NamedValueSet NVS2;
   NamedValueSet OptArgs;

   res = pALIPerfIfc->performanceCountersGet(&NVS2, OptArgs);
   if ( !res ) {
      ++m_Errors;
      ERR("SW-PERFC-01: 2nd performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters2(NVS2);

   if ( Counters1 != Counters2 ) {
      ERR("SW-PERFC-01: Performance Counters mismatch:\n" << Counters1 << "\n\nversus\n\n" << Counters2);
      ++m_Errors;
      ERR("SW-PERFC-01 FAIL");
   } else {
      MSG("SW-PERFC-01 PASS");
   }
}

void Read_Perf_CountersApp::sw_perfc_02(IBase *pAFUService, IBase *pFMEService)
{
   // Active test: Get performance counters. Run any NLB test. Get the performance counters again.
   // If they changed, then the SW is accessing the HW. Test Passes.

   // Perhaps add some sanity checking on returned value (!= 0xFFFF, value_before < value_after, ...)

   MSG("Begin SW-PERFC-02");

   // Get the IALIPerf associated with the FME AAL Service.
   IALIPerf *pALIPerfIfc = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, pFMEService);

   ASSERT(NULL != pALIPerfIfc);
   if ( NULL == pALIPerfIfc ) {
      ++m_Errors;
      return;
   }

   NamedValueSet NVS1;
   NamedValueSet OptArgs;
   btBool        res;

   res = pALIPerfIfc->performanceCountersGet(&NVS1, OptArgs);
   if ( !res ) {
      ++m_Errors;
      ERR("SW-PERFC-02: 1st performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters1(NVS1);

   DoesNLBLpbk1 lpbk1(pAFUService);
   btInt NLBErrors = lpbk1.NLBLpbk1();

   if ( NLBErrors > 0 ) {
      m_Errors += NLBErrors;
      ERR("SW-PERFC-02: Errors encountered during NLB Lpbk1");
      return;
   }

   NamedValueSet NVS2;

   res = pALIPerfIfc->performanceCountersGet(&NVS2);
   if ( !res ) {
      ++m_Errors;
      ERR("SW-PERFC-02: 2nd performanceCountersGet() failed");
      return;
   }

   PerfCounters Counters2(NVS2);

   if ( Counters1 == Counters2 ) {
      ERR("SW-PERFC-02: Performance Counters should have changed:\n" << Counters1 << "\n\nversus\n\n" << Counters2);
      ++m_Errors;
      ERR("SW-PERFC-02 FAIL");
   } else {
      MSG("SW-PERFC-02 PASS");
   }
}

void Read_Perf_CountersApp::sw_perfc_03(IBase *pAFUService, IBase *pFMEService)
{
   // Randomly select between test 22 and test 23.
   // Repeat this sequence of random executions 1,000 times.

   MSG("Begin SW-PERFC-03");

   btUnsigned32bitInt i;
   btUnsigned32bitInt r = 0;
   for ( i = 0 ; i < 1000 ; ++i ) {

      if ( GetRand(&r) % 2 ) {
         sw_perfc_01(pAFUService, pFMEService);
      } else {
         sw_perfc_02(pAFUService, pFMEService);
      }

      if ( m_Errors > 0 ) {
         ERR("SW-PERFC-03 FAIL");
         return;
      }

   }

   MSG("SW-PERFC-03 PASS");
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

