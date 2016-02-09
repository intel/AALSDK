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

#define MAX_FILENAME_SIZE (256)

#define TMP_RDSENSOR 0x1010
#define PM_RDVR      0x2010
#define PM_MAXVR     0x2018


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
   btInt run(btBool bClear); //Return 0 if success

   void   getTemp();
   btBool getPower(btBool bClear);

   btBool isOK()  {return m_bIsOK;}

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleaseFailed(const IEvent &rEvent);

   void serviceReleased(TransactionID const &rTranID);

   void serviceReleaseRequest(const IEvent &rEvent);

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
   // Temperature Sensor Read values
   struct CCIP_TEMP_RDSSENSOR_FMT1 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned32bitInt low;
            btUnsigned32bitInt high;
         };
         struct {
            btUnsigned64bitInt tmp_reading :7; // Reads out FPGA temperature in celsius.
            btUnsigned64bitInt rsvd2 :1;
            btUnsigned64bitInt tmp_reading_seq_num :16; // Temperature reading sequence number
            btUnsigned64bitInt tmp_reading_valid :1; // Temperature reading is valid
            btUnsigned64bitInt rsvd1 :7;
            btUnsigned64bitInt dbg_mode :8; //Debug mode
            btUnsigned64bitInt rsvd :24;
         }; // end struct
      } ; // end union
   }ccip_tmp_rdssensor_fm1; // end struct CCIP_TMP_RDSSENSOR_FMT1

   ccip_tmp_rdssensor_fm1.csr = 0;
   m_pALIMMIOService->mmioRead32(TMP_RDSENSOR, &ccip_tmp_rdssensor_fm1.low);

   cout << "--Temperature Sequence Number = " <<
      ccip_tmp_rdssensor_fm1.tmp_reading_seq_num << " \n";

   cout << "Temperature = " <<
      ccip_tmp_rdssensor_fm1.tmp_reading << " Degrees Celcius.\n";

}

btBool TempPowMonApp::getPower(btBool bClear)
{
   const btFloat      CORE_AMP_UNITS=0.09765625;
   const btFloat      CORE_VOLTAGE=0.95;
   btInt              AmpsValue;
   btFloat            AmpsAdjusted;
   btFloat            Power;
   btBool             bReturn = true;
   btUnsigned64bitInt SequenceNumber = 0;

   struct CCIP_PM_RDVR {
      // #define PM_RDVR      0x2010
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt clock_buffer_supply_i_valid :1; // clock buffer supply current valid
            btUnsigned64bitInt core_supply_i_valid :1;         // core supply current valid
            btUnsigned64bitInt trans_supply_i_valid :1;        // transceiver supply current valid
            btUnsigned64bitInt fpga_supply_i_valid :1;         // fpga 1.8v supply current valid
            btUnsigned64bitInt volt_regulator_readmods :1;     // Voltage regulator read modes
            btUnsigned64bitInt rsvd :3;
            btUnsigned64bitInt clock_buffer_supply_i_value :8; // clock buffer supply current value
            btUnsigned64bitInt core_supply_i_value :16;        // core supply current value
            btUnsigned64bitInt trans_supply_i_value :8;        // transceiver supply current value
            btUnsigned64bitInt fpga_supply_i_value :8;         // fpga supply current value
            btUnsigned64bitInt sequence_number :16;            // read sample sequence number
         }; // end struct
      }; // end union
   } ccip_pm_rdvr; // end struct CCIP_PM_RDVR

   // Now deal with Max current values
   struct CCIP_PM_MAXVR {
      // #define PM_MAXVR     0x2018
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt hw_set_field :1; //Hardware set field
            btUnsigned64bitInt rsvd :7;
            btUnsigned64bitInt max_clock_supply_i_rec :8;  // Maximum clock buffer supply current recorded
            btUnsigned64bitInt max_core_supply_i_rec  :16; // Maximum core  supply current recorded
            btUnsigned64bitInt max_trans_supply_i_rec :8;  // Maximum Transceiver  supply current recorded
            btUnsigned64bitInt max_fpga_supply_i_rec  :8;  // Maximum FPGA  supply current recorded
            btUnsigned64bitInt rsvd1 :16;
         }; // end struct
      }; // end union
   }ccip_pm_mrdvr; // end struct CCIP_PM_MAXVR

   // get current sequence number
   m_pALIMMIOService->mmioRead64(PM_RDVR, &ccip_pm_rdvr.csr);
   SequenceNumber = ccip_pm_rdvr.sequence_number;

   // Clear the max values?
   if( bClear ) {
      // set hw reset to 1 to put into reset
      m_pALIMMIOService->mmioRead64(PM_MAXVR, &ccip_pm_mrdvr.csr);
      ccip_pm_mrdvr.hw_set_field = 1;
      m_pALIMMIOService->mmioWrite64(PM_MAXVR, ccip_pm_mrdvr.csr);

      // wait a bit and re-read sequence number
      SleepMilli(1); // insurance, should not be needed, remove for faster execution
      m_pALIMMIOService->mmioRead64(PM_RDVR, &ccip_pm_rdvr.csr);
      SequenceNumber = ccip_pm_rdvr.sequence_number;

      // set hw reset to 0 to re-enable
      m_pALIMMIOService->mmioRead64(PM_MAXVR, &ccip_pm_mrdvr.csr);
      ccip_pm_mrdvr.hw_set_field = 0;
      m_pALIMMIOService->mmioWrite64(PM_MAXVR, ccip_pm_mrdvr.csr);

      // Wait for sequence number to change, or error out
      btUnsigned64bitInt NewSequenceNumber = SequenceNumber;
      btUnsigned64bitInt count = 0;
      do {
         SleepMilli(1); // insurance, remove to speed up processing
         // read new Sequence number
         m_pALIMMIOService->mmioRead64(PM_RDVR, &ccip_pm_rdvr.csr);
         NewSequenceNumber = ccip_pm_rdvr.sequence_number;
         ++count;
      } while ( count < 10 && SequenceNumber == NewSequenceNumber);

      // Did Sequence number increment? If so, that is good
      bReturn = (SequenceNumber != NewSequenceNumber);
      SequenceNumber = NewSequenceNumber;

      cout << "Clear of Max values requested: result is " << bReturn << endl;
   } // bClear

   ///////////////////////////////////////////////////////////
   // Read regular values
   m_pALIMMIOService->mmioRead64(PM_RDVR, &ccip_pm_rdvr.csr);
   ccip_pm_rdvr.volt_regulator_readmods = 0; // turn on all 4 read channels
   m_pALIMMIOService->mmioWrite64(PM_RDVR, ccip_pm_rdvr.csr);
   SleepMilli(1);   // Wait a bit

   m_pALIMMIOService->mmioRead64(PM_RDVR, &ccip_pm_rdvr.csr);

   // Print Sequence #
   cout << "--Power Sequence Number = " <<
      ccip_pm_rdvr.sequence_number << endl;

   // Print Core Amps and power
   if (ccip_pm_rdvr.core_supply_i_valid) {
      AmpsValue = static_cast< btInt >(ccip_pm_rdvr.core_supply_i_value);
      AmpsAdjusted = AmpsValue * CORE_AMP_UNITS ;
      Power = AmpsAdjusted * CORE_VOLTAGE;
      cout << "Core Reading Is Valid: " << AmpsAdjusted << " Amps. " <<
         Power << " Estimated Watts at nominal " <<
         CORE_VOLTAGE << " Volts." <<
         endl;

   } else {
      cout << "Core Reading Not Valid" << endl;
   }

   // Print transceiver amps
   if (ccip_pm_rdvr.trans_supply_i_valid) {
      cout << "Xcvr Reading Is Valid: " <<
         ccip_pm_rdvr.trans_supply_i_value << " Unknown Units." <<
         endl;

   } else {
      cout << "Xcvr Reading Not Valid" << endl;
   }

   // Print 1.8V amps
   if (ccip_pm_rdvr.fpga_supply_i_valid) {
      cout << "1.8V Reading Is Valid: " <<
         ccip_pm_rdvr.fpga_supply_i_value << " Unknown Units." <<
         endl;

   } else {
      cout << "1.8V Reading Not Valid" << endl;
   }

   ///////////////////////////////////////////////////////////////

   m_pALIMMIOService->mmioRead64(PM_MAXVR, &ccip_pm_mrdvr.csr);

   // Print Max Core Amps and power
   AmpsValue = static_cast< btInt >(ccip_pm_mrdvr.max_core_supply_i_rec);
   AmpsAdjusted = AmpsValue * CORE_AMP_UNITS ;
   Power = AmpsAdjusted * CORE_VOLTAGE;
   cout << "Core Maximum  Reading: " << AmpsAdjusted << " Amps. " <<
      Power << " Estimated Watts at nominal " <<
      CORE_VOLTAGE << " Volts." <<
      endl;
   cout << "Xcvr Maximum  Reading: " <<
      ccip_pm_mrdvr.max_trans_supply_i_rec << " Unknown Units." <<
      endl;
   cout << "1.8V Maximum  Reading: " <<
      ccip_pm_mrdvr.max_fpga_supply_i_rec << " Unknown Units." <<
      endl;

} // TempPowMonApp::getPower


btInt TempPowMonApp::run(btBool bClear)
{

   btBool bPowerReturnValue;

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
    bPowerReturnValue = getPower( bClear );
    if (!bPowerReturnValue) ++m_Result;   // record error

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

 void TempPowMonApp::serviceReleaseRequest(const IEvent &rEvent)
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
   btBool bClear = false;

   if( argc>1 ) { // process command line arg of "-c"
      if (0 == strcmp (argv[1], "-c")) bClear = true;
   }

   if(theApp.IsOK()){
      result = theApp.run( bClear );
   }else{
      MSG("App failed to initialize");
   }

   MSG("Done");
   return result;
}


