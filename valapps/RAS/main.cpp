// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/RAS/main.cpp
//
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
/// @file main.cpp
/// @brief Verifys Kernel error logs .
/// @ingroup RAS
/// @verbatim
/// AAL RAS test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test PR functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2016     RP       Initial version started based on older sample
//****************************************************************************

// valapps/RAS/main.cpp

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include "vallib.h"

// FME Error CSR Offset
#define FME_ERR0          0x4010
#define FME_ERR1          0x4020
#define FME_ERR2          0x4030
#define FME_FIRST_ERR     0x4038
#define FME_NEXT_ERR      0x4040

// PORT Error CSR Offset
#define PORT_ERR_MASK     0x1008
#define PORT_ERR          0x1010
#define PORT_FIRST_ERR    0x1018
#define PORT_MALFORM0     0x1020
#define PORT_MALFORM1     0x1028

// FME Thermal CSR Offset
#define FME_THERMAL      0x1008

// Set error csr
#define ERROR_CSR        0xFFFFFFFFF

//Sleep time
#define SLEEP_TIME       1


class RASApp : public CAASBase,
               public IRuntimeClient
{
public:
   RASApp();

   btInt    Run();    ///< Return 0 if success
   btInt Errors() const { return m_Errors; }
   btInt serach_kerneltraces(string search_str);

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
   void sw_fme_01(IBase *pFMEService);
   void sw_port_02(IBase *pPortService);
   void sw_fme_ap_03(IBase *pfmeService);
   void sw_pr_04();
   void sw_bad_vkey_05();

   void sw_simulate_errors(IBase *pfmeService,IBase *pPortService);

   Runtime              m_Runtime;       ///< AAL Runtime
   btInt                m_Errors;        ///< Returned result value; 0 if success
   CSemaphore           m_Sem;           ///< For synchronizing with the AAL runtime.
   AllocatesFME         m_FMEAllocator;
   AllocatesPort        m_PortAllocator;
};

RASApp::RASApp() :
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

void RASApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeCreateOrGetProxyFailed()");
}

void RASApp::runtimeStarted(IRuntime            *pRuntime,
                                           const NamedValueSet &rConfigParms)
{
   MSG("Runtime Started");
   m_Sem.Post(1);
}

void RASApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime Stopped");
   m_Sem.Post(1);
}

void RASApp::runtimeStartFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStartFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void RASApp::runtimeStopFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStopFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void RASApp::runtimeAllocateServiceFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeAllocateServiceFailed()");
   PrintExceptionDescription(rEvent);
}

void RASApp::runtimeAllocateServiceSucceeded(IBase               *pClient,
                                                            TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void RASApp::runtimeEvent(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("runtimeEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}

btInt RASApp::Run()
{
   MSG("RASApp::Run");

   // Get a pointer to an NLB Lpbk1 AAL Service.
   m_PortAllocator.Allocate(&m_Runtime);
   m_PortAllocator.Wait();

   IBase *pPortService = m_PortAllocator.Service();
   IBase *pFMEService;

   MSG("RASApp::Run 1");

   if ( ( NULL == pPortService ) || ( m_PortAllocator.Errors() > 0 ) ) {
      goto _STOP;
   }

   m_FMEAllocator.Allocate(&m_Runtime);
   m_FMEAllocator.Wait();
   MSG("RASApp::Run 2");

   pFMEService = m_FMEAllocator.Service();

   if ( ( NULL == pFMEService ) || ( m_FMEAllocator.Errors() > 0 ) ) {
      goto _LPBK1;
   }

   sw_simulate_errors(pFMEService,pPortService);
   sw_fme_01(pFMEService);
   sw_port_02(pPortService);
   sw_fme_ap_03(pFMEService);
   sw_pr_04();
   //sw_bad_vkey_05();

   m_FMEAllocator.Free();
   m_FMEAllocator.Wait();

_LPBK1:
   m_PortAllocator.Free();
   m_PortAllocator.Wait();

_STOP:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Errors + m_PortAllocator.Errors() + m_FMEAllocator.Errors();
}

void RASApp::sw_simulate_errors(IBase *pfmeService,IBase *pPortService)
{
   // Get the pALIFMEMMIO associated with the FME AAL Service.
   IALIMMIO *pALIFMEMMIO = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pfmeService);

   ASSERT(NULL != pALIFMEMMIO);
   if ( NULL == pALIFMEMMIO ) {
      ++m_Errors;
      return;
   }
   // Get the pALIPortMMIO associated with the Port AAL Service.
   IALIMMIO *pALIPortMMIO = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pPortService);

   ASSERT(NULL != pALIPortMMIO);
   if ( NULL == pALIPortMMIO ) {
      ++m_Errors;
      return;
   }

   // Reset FME and Port Errors
   pALIPortMMIO->mmioWrite64(PORT_ERR, 0x0);
   pALIPortMMIO->mmioWrite64(PORT_FIRST_ERR, 0x0);
   pALIPortMMIO->mmioWrite64(PORT_MALFORM0, 0x0);
   pALIPortMMIO->mmioWrite64(PORT_MALFORM1, 0x0);

   pALIFMEMMIO->mmioWrite64(FME_ERR0, 0x0);
   pALIFMEMMIO->mmioWrite64(FME_ERR1, 0x0);
   pALIFMEMMIO->mmioWrite64(FME_ERR2, 0x0);
   pALIFMEMMIO->mmioWrite64(FME_FIRST_ERR, 0x0);
   pALIFMEMMIO->mmioWrite64(FME_NEXT_ERR, 0x0);
   SleepSec(SLEEP_TIME);

   // Set FME and Port errors
   pALIFMEMMIO->mmioWrite64(FME_ERR0, ERROR_CSR);
   pALIFMEMMIO->mmioWrite64(FME_ERR1, ERROR_CSR);
   pALIFMEMMIO->mmioWrite64(FME_ERR2, ERROR_CSR);
   pALIFMEMMIO->mmioWrite64(FME_FIRST_ERR, ERROR_CSR);
   pALIFMEMMIO->mmioWrite64(FME_NEXT_ERR, ERROR_CSR);


   pALIPortMMIO->mmioWrite64(PORT_ERR, ERROR_CSR);
   pALIPortMMIO->mmioWrite64(PORT_FIRST_ERR, ERROR_CSR);
   pALIPortMMIO->mmioWrite64(PORT_MALFORM0, ERROR_CSR);


   // Temperature Threshold
   struct CCIP_TEMP_THRESHOLD {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt tmp_thshold1 :7;                // temperature Threshold 1
            btUnsigned64bitInt tmp_thshold1_status :1;         // temperature Threshold 1 enable /disable
            btUnsigned64bitInt tmp_thshold2 :7;                // temperature Threshold 2
            btUnsigned64bitInt tmp_thshold2_status :1;         // temperature Threshold 2 enable /disable
            btUnsigned64bitInt rsvd4 :8;
            btUnsigned64bitInt therm_trip_thshold :7;          // Thermeal Trip Threshold
            btUnsigned64bitInt rsvd3 :1;
            btUnsigned64bitInt thshold1_status :1;             // Threshold 1 Status
            btUnsigned64bitInt thshold2_status :1;             // Threshold 2 Status
            btUnsigned64bitInt rsvd5 :1;
            btUnsigned64bitInt therm_trip_thshold_status :1;   // Thermeal Trip Threshold status
            btUnsigned64bitInt rsvd2 :8;
            btUnsigned64bitInt thshold_policy :1;              // threshold policy
            btUnsigned64bitInt rsvd :19;

        }; //end struct
      }; // end union

   }; // end struct CCIP_TMP_THRESHOLD

   struct CCIP_TEMP_THRESHOLD          ccip_tmp_threshold ={0};
   pALIFMEMMIO->mmioWrite64(FME_THERMAL, ccip_tmp_threshold.csr);
   ccip_tmp_threshold.thshold_policy  = 0x1;
   ccip_tmp_threshold.thshold1_status = 0x1;
   ccip_tmp_threshold.thshold2_status = 0x1;

   pALIFMEMMIO->mmioWrite64(FME_THERMAL, ccip_tmp_threshold.csr);

   SleepSec(SLEEP_TIME);

}
void RASApp::sw_fme_01(IBase *pFMEService)
{
   // sw_fme_01   Verify FME kernel error logs
   MSG("Begin SW-FME-01");
   btInt errors = 0;

   if( 0 != serach_kerneltraces("FME Error0")) {
      ++errors;
      ERR("No FME Error0");
   }

   if( 0 != serach_kerneltraces("FME Error1")) {
      ++errors;
      ERR("No FME Error1");
   }

   if( 0 != serach_kerneltraces("FME Error2")) {
      ++errors;
      ERR("No  FME Error2");
   }

   if( 0 != serach_kerneltraces("FME First Error")) {
      ++errors;
      ERR("No FME First Error");
   }

   if( 0 != serach_kerneltraces("FME Next Error")) {
      ++errors;
      ERR("No FME Next Error");
   }

   if ( errors > 0  ) {
      ERR("SW-FME-01 FAIL");
   } else {
      MSG("SW-FME-01 PASS");
   }

   m_Errors += errors;
}


void RASApp::sw_port_02(IBase *pPortService)
{
   // sw_port_02   Verify port kernel error logs
   MSG("Begin SW-PORT-02");
   btInt errors = 0;

   if( 0 != serach_kerneltraces("PORT Error")) {
      ++errors;
      ERR("No PORT Error");
   }

   if( 0 != serach_kerneltraces("PORT First Error")) {
      ++errors;
      ERR("No PORT First Error");
   }

   if( 0 != serach_kerneltraces("PORT Malfromed")) {
      ++errors;
      ERR("No PORT Malfromed Request");
   }

   if ( errors > 0   ) {
      ERR("SW-PORT-02 FAIL");
   } else {
      MSG("SW-PORT-01 PASS");
   }
   m_Errors += errors;
}

void RASApp::sw_fme_ap_03(IBase *pfmeService)
{
   // sw_fme_ap_03   Verify ap state kernel error logs
   MSG("Begin SW-FME-AP-03");
   btInt errors = 0;

   if( 0 != serach_kerneltraces("Trigger AP1")) {
      ++errors;
      ERR("Not Trigger AP1");
   }

   if( 0 != serach_kerneltraces("Trigger AP2")) {
      ++errors;
      ERR("NOT Trigger AP2");
   }

   if( 0 != serach_kerneltraces("Trigger AP6")) {
      ++errors;
      ERR("NOT  Trigger AP6");
   }

   if ( (errors > 0 ) && (errors <=3) ) {
      ERR("SW-FME-AP-03 PASS");
   } else {
      MSG("SW-FME-AP-03 FAIL");
   }
   m_Errors += errors;
}

void RASApp::sw_pr_04()
{
   // sw_pr_04  Verify PR error kernel error logs
   MSG("Begin SW-PR-04");


   if( 0 != serach_kerneltraces("PR Host Status")) {
      ERR("No PR Hot status Error");
   }

   if( 0 != serach_kerneltraces("PR Controller Block")) {
      ERR("NO PR Controller Block Error");
   }


}
void RASApp::sw_bad_vkey_05()
{
   // sw_bad_vkey_05  Verify Bad V-key kernel error logs
   MSG("Begin sw_bad_vkey_05");


}
btInt RASApp::serach_kerneltraces(string search_str)
{
   btInt res   =  -1;
   FILE *pfile  = NULL;
   pfile = popen("dmesg","r");

   if(NULL != pfile) {

      while(true) {
         char* line ;
         char buf[2000];
         line = fgets(buf,sizeof(buf),pfile);

         if(NULL == line) break;

         std::string str(buf);
         std::size_t found = str.find(search_str);

         if(found != std::string::npos)  {
            std::cout << "------Traces:: START -----" << std::endl;
            std::cout << buf << std::endl ;
            std::cout << "------Traces:: END --------"<< std::endl;

            res = 0;
         }
      }
   }

   pclose(pfile);

   return res;
}

int main(int argc, char *argv[])
{

   RASApp TheApp;

   if ( TheApp.Errors() > 0 ) {
      // something bad happened during Runtime startup.
      return TheApp.Errors();
   }

   return TheApp.Run();

}

