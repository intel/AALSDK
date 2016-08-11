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
/// @file PwrMgrApp.cpp
/// @brief Basic PwrMgr interaction.
/// @ingroup PwrMgerApp
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Ananda Ravuri, Intel Corporation.
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/2016     SC       Initial version started based on older sample code.@endverbatim
//****************************************************************************
/// @}
#include "PwrMgrApp.h"
#include <math.h>
#include <sched.h>


#define  PWR_MSR610                      "/usr/sbin/rdmsr -c0 -p %d 0x610"
#define  PWR_MSR606                      "/usr/sbin/rdmsr -c0 -p %d 0x606"
#define  SKX_CPU_SPLIT_POINT             48

void PwrMgrApp::serviceAllocated(AAL::IBase               *pServiceBase,
                                 AAL::TransactionID const &rTranID)
{
   // Save the IBase for the Service. Through it we can get any other
   //  interface implemented by the Service
   m_pPwrMgrService = pServiceBase;
   ASSERT(NULL != m_pPwrMgrService);
   if ( NULL == m_pPwrMgrService ) {
      ++m_Errors;
      ERR("returned  service was NULL");
      m_Sem.Post(1);
      return;
   }

   MSG(" Service Allocated");
   m_Sem.Post(1);
}

void PwrMgrApp::serviceAllocateFailed(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR("Failed to allocate  Service");
   m_Sem.Post(1);
}

void PwrMgrApp::serviceReleased(AAL::TransactionID const &rTranID)
{
   MSG(" Service Released");
   if ( rTranID.Context() != (AAL::btApplicationContext)1 ) {
      // Don't post the semaphore if this happened because of serviceReleaseRequest().
      m_Sem.Post(1);
   }
   m_pPwrMgrService = NULL;
}

void PwrMgrApp::serviceReleaseRequest(AAL::IBase        *pServiceBase,
                                      const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR(" didn't expect serviceReleaseRequest()");
   if ( NULL != m_pPwrMgrService ) {
      AAL::IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pPwrMgrService);
      ASSERT(NULL != pIAALService);

      // Breadcrumb that this Release resulted from serviceReleaseRequest().
      AAL::TransactionID tid((AAL::btApplicationContext)1);

      pIAALService->Release(tid);
   }
}

void PwrMgrApp::serviceReleaseFailed(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR( " didn't expect serviceReleaseFailed()");
   m_Sem.Post(1);
}

void PwrMgrApp::serviceEvent(const AAL::IEvent &rEvent)
{
   ++m_Errors;
   ERR( " serviceEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}


btInt PwrMgrApp::AllocateService()
{

   SetInterface(iidServiceClient, dynamic_cast<AAL::IServiceClient *>(this));
   SetInterface(iid_PWRMGR_Service_Client, dynamic_cast<IPwrMgr_Client *>(this));

   AAL::NamedValueSet Manifest;
   AAL::NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libPwrMgr");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "PWR MGR");

   MSG("Allocating Power Manager Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_Errors appropriately.

   // We are the service client.
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();

}


PwrMgrApp::PwrMgrApp() :
              m_Runtime(this),
              m_Errors(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

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

void PwrMgrApp::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeCreateOrGetProxyFailed()");
}

void PwrMgrApp::runtimeStarted(IRuntime            *pRuntime,
                                           const NamedValueSet &rConfigParms)
{
   MSG("Runtime Started");
   m_Sem.Post(1);
}

void PwrMgrApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime Stopped");
   m_Sem.Post(1);
}

void PwrMgrApp::runtimeStartFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStartFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void PwrMgrApp::runtimeStopFailed(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeStopFailed()");
   PrintExceptionDescription(rEvent);
   m_Sem.Post(1);
}

void PwrMgrApp::runtimeAllocateServiceFailed(IEvent const &rEvent)
{
   ++m_Errors;
   ERR("didn't expect runtimeAllocateServiceFailed()");
   PrintExceptionDescription(rEvent);
}

void PwrMgrApp::runtimeAllocateServiceSucceeded(IBase               *pClient,
                                                TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void PwrMgrApp::runtimeEvent(const IEvent &rEvent)
{
   ++m_Errors;
   ERR("runtimeEvent(): received unexpected event 0x" << std::hex << rEvent.SubClassID() << std::dec);
}

btInt PwrMgrApp::FreeService()
{
   if ( NULL != m_pPwrMgrService ) {
      MSG("Freeing  Service");

      AAL::IAALService *pIAALService = dynamic_ptr<AAL::IAALService>(iidService, m_pPwrMgrService);
      ASSERT(NULL != pIAALService);
      pIAALService->Release(AAL::TransactionID());
   } else {
      m_Sem.Post(1);
   }
}

void PwrMgrApp::reconfPowerRequest( TransactionID &rTranID,IEvent const &rEvent ,INamedValueSet &rInputArgs)
{
   MSG("reconfPowerEvent Triggered");

   AAL::NamedValueSet    pwrMgrNVS;
   btInt                 busID        = 0;
   btInt                 deviceID     = 0;
   btInt                 funcID       = 0;
   btInt                 socketID     = 0;
   btInt                 res          = 0;
   btInt                 Wattsvalue   = 0;;


   if (rInputArgs.Has(PWRMGR_SOCKETID)) {
      rInputArgs.Get( PWRMGR_SOCKETID, &socketID);
      printf("PWRMGR_SCOKETID %d \n",socketID);
   }

   if (rInputArgs.Has(PWRMGR_BUSID)) {
      rInputArgs.Get( PWRMGR_BUSID, &busID);
      printf("PWRMGR_BUSID %d \n",busID);
   }

   if (rInputArgs.Has(PWRMGR_DEVICEID)) {
      rInputArgs.Get( PWRMGR_DEVICEID, &deviceID);
      printf("PWRMGR_DEVICEID %d \n",deviceID);
   }

   if (rInputArgs.Has(PWRMGR_FUNID)) {
      rInputArgs.Get( PWRMGR_FUNID, &funcID);
      printf("PWRMGR_FUNID %d \n",funcID);
   }

   if (rInputArgs.Has(PWRMGR_RECONF_PWRREQUIRED)) {
      rInputArgs.Get( PWRMGR_RECONF_PWRREQUIRED, &Wattsvalue);
      printf("PWRMGR_PRPWRRQUIRED %d \n",Wattsvalue);
   }

   // res = CoreIdler( Wattsvalue,socketID);
   //SleepSec(50);

   printf("res= %d \n",res);

   IPwrMgr *pALIPwrMgr = dynamic_ptr<IPwrMgr>(iid_PWRMGR_Service, m_pPwrMgrService);

   ASSERT(NULL != pALIPwrMgr);
   if ( NULL == pALIPwrMgr ) {
      return ;
   }

   pwrMgrNVS.Add(PWRMGMT_STATUS,res);

   pALIPwrMgr->reconfPowerResponse(rTranID,pwrMgrNVS);

}


btInt PwrMgrApp::CoreIdler(btInt &FPIWatts, btInt &socket)
{

   FILE *fp                    = NULL;

   int64_t PackPwrLimit1       = 0;
   int64_t PowerUnitValue      = 0;
   int64_t PackagePowerUnit    = 0;
   int64_t CoreCount           = 0;
   int64_t MaxThreadVal        = 0;

   int ret_val                 = 0;
   int split_point             = 0;
   int max_pid_index           = 0;
   int pid                     = 0;
   int i                       = 0;

   long double TotalWatts      = 0;
   long double AvailableWatts  = 0;
   long double FpgaWatts       = 0;

   char MaxThreadValStr[20]    = {0};
   char data[1024]             = {0};
   char data1[1024]            = {0};
   char command610[40]         = {0};
   char command606[40]         = {0};
   char *endptr                = NULL;

   cpu_set_t idle_set, current_set, full_mask_set;

   // Fail if socket not equal to 0 or 1
   if (socket != 0 ) {
      if (socket != 1)
         ERR("Bad Socket ID");
         return ali_errnumBadSocket;
   }

   // zero array before building commands.
   for (i = 0; i < 40; i++) {
     command610[i] = 0;
     command606[i] = 0;
   } 

   split_point = SKX_CPU_SPLIT_POINT; // force split temporarily, BUGBUG

   // set msr commands based on socket and split_point
   if (socket == 0) {
      sprintf(command610, PWR_MSR610,0);
      sprintf(command606, PWR_MSR606,0);
   } else {
      sprintf(command610, PWR_MSR610, split_point);
      sprintf(command606, PWR_MSR606, split_point);
   }

    FpgaWatts = (double) FPIWatts;
  //
   // Begin MSR retrieval.
   //
   //  fp = popen("/usr/sbin/rdmsr -c0 -p 0 0x610", "r");
   fp = popen(command610, "r");
   if (NULL == fp) {
      ERR("Failed to open MSR 610 ");
      return(ali_errnumRdMsrCmdFail);
   }

   while (fgets(data, sizeof(data)-1, fp) != NULL) {
      printf("%s", data);
   }

   PackPwrLimit1 = strtoll(&data[2], &endptr, 16);
   printf("Power Limit converted: %lx \n", PackPwrLimit1);
   ret_val = pclose(fp);

   PackPwrLimit1 = PackPwrLimit1 & 0x07fff;

    //  fp = popen("/usr/sbin/rdmsr -c0 -p 0 0x606", "r");
   fp = popen(command606, "r");
   if (NULL == fp) {
      printf("Failed to run command\n");
      ERR("Failed to open MSR 606 ");
      return(ali_errnumRdMsrCmdFail);
   }
   while (fgets(data, sizeof(data)-1, fp) != NULL) {
      printf("%s", data);
   }

   PackagePowerUnit = strtoll(&data[2], &endptr, 16);
   printf("Package Power Unit Value converted: %lx \n", PackagePowerUnit);
   ret_val = pclose(fp);

   //
   // MSR retrvial complete
   // Calculate power budget.
   //

   PowerUnitValue = PackagePowerUnit & 0x0f;
   PowerUnitValue = pow(2, PowerUnitValue);
   printf("Divisor of Raw Limit1:%lx\n", PowerUnitValue);
   TotalWatts = ((double)PackPwrLimit1)/((double)PowerUnitValue);
   printf("Total Watts: %Lf \n", TotalWatts);

   //
   // Check that at least one core will be present.
   //
   if ((TotalWatts - 5 ) <= FpgaWatts)  {
      ERR("Invalid PR Power Value");
      return (ali_errnumFPGAPowerRequestTooLarge);
   }
   AvailableWatts = TotalWatts - (double)FpgaWatts;
   printf("Available Watts: %Lf\n", AvailableWatts);
   CoreCount = (int64_t) AvailableWatts / (int64_t) 5;
   printf("Core Count: %ld\n", CoreCount);
   MaxThreadVal = CoreCount * 2;
   // ---  //MaxThreadVal = MaxThreadVal - 1;
   snprintf(MaxThreadValStr, 20, "%ld", MaxThreadVal);
   printf("Max Thread String Value: %s \n",MaxThreadValStr);

   CPU_ZERO(&idle_set);

   if (socket == 1) {
      for (i = 0; i < MaxThreadVal; i++) {
            CPU_SET(i + split_point, &idle_set);
      }
   } else {
      if (socket == 0) {
            for (i = 0; i < MaxThreadVal; i++) {
               CPU_SET(i, &idle_set);
            }
   } else {
         printf("Socket input must be 0 or 1\n");
      }
   }

   i = CPU_COUNT_S(sizeof(cpu_set_t), &idle_set);
   printf("Cpu Count: %d \n",i);

   if (sched_getaffinity(1, sizeof(current_set), &current_set) == -1){
      printf("sched_getaffinity failure for pid: 1\n");
   }

   for (i = 0; i < split_point; i++) {
      if (socket == 0) {
            CPU_CLR(i, &current_set);
   } else {
            CPU_CLR(i + split_point, &current_set);
      }
   }

   CPU_OR(&full_mask_set, &current_set, &idle_set);

   if (sched_setaffinity(1, sizeof(full_mask_set), &full_mask_set) == -1){
      printf("sched_setaffnity failure for pid: 1\n");
   }

   if (sched_getaffinity(2, sizeof(current_set), &current_set) == -1){
      printf("sched_getaffinity failure for pid: 2\n");
   }

   for (i = 0; i < split_point; i++) {
      if (socket == 0) {
         CPU_CLR(i, &current_set);
      } else {
         CPU_CLR(i + split_point, &current_set);
      }
   }

   CPU_OR(&full_mask_set, &current_set, &idle_set);

   if (sched_setaffinity(2, sizeof(full_mask_set), &full_mask_set) == -1){
      printf("sched_setaffnity failure for pid: 2\n");
   }

   //--//
   //--// "User Mode Code"
   //--// Find max pid number
   fp = fopen("/proc/sys/kernel/pid_max", "r");
   for (i = 0; i < 20; i++) {
      data1[i] = fgetc(fp);
      if  (feof(fp)) {
         data1[i] = 0;
         break;
      }
   }
   max_pid_index = strtol(&data1[0], &endptr, 10);
   ret_val = fclose(fp);

   //--//
   //--//  Set affinity for all possible pids to mask in cpuset.
   //--//
   for (pid = 3; pid < max_pid_index; pid++) {

      if (sched_getaffinity(pid, sizeof(current_set), &current_set) == -1){
         //printf("sched_getaffinity failure for pid: 1\n");
         continue;
      }

      for (i = 0; i < split_point; i++) {
         if (socket == 0) {
            CPU_CLR(i, &current_set);
         } else {
            CPU_CLR(i + split_point, &current_set);
         }
      }

      CPU_OR(&full_mask_set, &current_set, &idle_set);

   if (sched_setaffinity(pid, sizeof(full_mask_set), &full_mask_set) == -1){
      //--//      printf("schedaffnity failure pid: %d\r", i);
      }
   }
   return 0;

}
