// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_SingleApp/ALIReconf.cpp
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
/// @file ALIReconf.cpp
/// @brief Partial reconfiguration of green bit stream service .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL Partial reconfiguration test application
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

#include "../PR_SingleApp/ALIReconf.h"

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
//#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define MSG(x)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


AllocatesReconfService::AllocatesReconfService(const arguments &args) :
   m_pRuntime(NULL),
   m_pFMEService(NULL),
   m_pALIReconfService(NULL),
   m_Result(0),
   m_errNum(0),
   m_args(args)
{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client *>(this));
   m_Sem.Create(0, 1);

   m_bIsOK = true;
}

AllocatesReconfService::~AllocatesReconfService()
{
   m_Sem.Destroy();
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void AllocatesReconfService::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   m_pFMEService = pServiceBase;
   ASSERT(NULL != m_pFMEService);
   if ( NULL == m_pFMEService ) {
      m_bIsOK = false;
      return;
   }

   m_pALIReconfService = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
   ASSERT(NULL != m_pALIReconfService);
   if ( NULL == m_pALIReconfService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void AllocatesReconfService::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
   PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void AllocatesReconfService::serviceReleased(TransactionID const &rTranID)
{
   MSG("Service Released");
   m_Sem.Post(1);
}

 void AllocatesReconfService::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
  {
   MSG("AllocatesReconfService->Service unexpected requested back");
   if ( rEvent.Has(iidReleaseRequestEvent) ) {

      std::cerr << "Description: " << dynamic_ref<IReleaseRequestEvent>(iidReleaseRequestEvent, rEvent).Description() << std::endl;
   }

   if(NULL != pServiceBase){
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
      ASSERT(pIAALService);
      pIAALService->Release(TransactionID());
   }
  }

 void AllocatesReconfService::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void AllocatesReconfService::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   if ( rEvent.Has(iidExEvent) ) {
      std::cerr << "\n Description  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Description() << std::endl;
      std::cerr << "\n ExceptionNumber:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).ExceptionNumber() << std::endl;
      std::cerr << "\n Reason:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Reason() << std::endl;

   }

}
// <end IServiceClient interface>


void AllocatesReconfService::deactivateSucceeded( TransactionID const &rTranID )
{
   MSG("deactivateSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::deactivateFailed( IEvent const &rEvent )
{
   ERR("Failed deactivate");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}

void AllocatesReconfService::configureSucceeded( TransactionID const &rTranID )
{
   MSG("configureSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::configureFailed( IEvent const &rEvent )
{
   ERR("configureFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}
void AllocatesReconfService::activateSucceeded( TransactionID const &rTranID )
{
   MSG("activateSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::activateFailed( IEvent const &rEvent )
{
   ERR("activateFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}

void AllocatesReconfService::PrintReconfExceptionDescription(IEvent const &rEvent)
{
   if ( rEvent.Has(iidExTranEvent) ) {
      std::cerr << "ExceptionNumber: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber() << std::endl;
      m_errNum  = dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber();
   }
}


void AllocatesReconfService::setreconfnvs( const std::string & pbitstream,
                                           btUnsigned64bitInt reconftimeout ,
                                           btUnsigned64bitInt reconfAction ,
                                           btUnsigned64bitInt reactivateDisabled)
{
   m_reconfnvs.Empty();
   m_reconfnvs.Add(AALCONF_MILLI_TIMEOUT,(static_cast<btUnsigned64bitInt>(reconftimeout)) *1000);

   // Reconfigure action
   if(AALCONF_RECONF_ACTION_HONOR_OWNER_ID == reconfAction) {
      m_reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_OWNER_ID);
   }
   else {
      m_reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID);
   }

   // ReActivated state
   if(reactivateDisabled) {
      m_reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,true);
   } else {
      m_reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,false);
   }

   m_reconfnvs.Add(AALCONF_FILENAMEKEY,pbitstream.c_str());

}

btBool AllocatesReconfService::AllocatePRService(Runtime *pRuntime)
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
   ConfigRecord.Add(keyRegAFU_ID,ALI_AFUID_UAFU_CONFIG);
   ConfigRecord.Add(keyRegSubDeviceNumber,0);

   if (m_args.have("bus")){
       ConfigRecord.Add(keyRegBusNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("bus")));
   }

   if (m_args.have("device")){
       ConfigRecord.Add(keyRegDeviceNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("device")));
   }

   if (m_args.have("function")){
       ConfigRecord.Add(keyRegFunctionNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("function")));
   }

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "ALI Conf AFU");

   m_pRuntime = pRuntime;
   m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      return false;
   }
   return true;
}

btBool AllocatesReconfService::FreePRService()
{
   MSG("Release Service");
   (dynamic_ptr<IAALService>(iidService, m_pFMEService))->Release(TransactionID());
   m_Sem.Wait();
   return true;
}

