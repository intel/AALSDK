// Copyright(c) 2015-2016, Intel Corporation
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
/// @file PwrMgr.cpp
/// @brief Implementation of FPGA Power Manager Hardware Service.
/// @ingroup PwrMgr
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Ananda Ravuri, Intel Corporation
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/utils/ResMgrUtilities.h>
#include "PwrMgrTransactions.h"
#include "PwrMgr.h"
#include "aalsdk/aas/Dispatchables.h"
#include <math.h>
#include <sched.h>

BEGIN_NAMESPACE(AAL)


/// @addtogroup PwrMgr
/// @{

#ifdef INFO
# undef INFO
#endif // INFO
#define INFO(x) AAL_INFO(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) AAL_ERR(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef VERBOSE
# undef VERBOSE
#endif // VERBOSE
#define VERBOSE(x) AAL_VERBOSE(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)


// ===========================================================================
//
// PwrMgr implementation
//
// ===========================================================================

//
// init.
//
// TODO: Add checks for PwrMgr capabilities, possibly selective exposure of
//       interfaces based on results
//
btBool CPwrMgr::init(IBase               *pclientBase,
                     NamedValueSet const &optArgs,
                     TransactionID const &TranID)
{
   btHANDLE devHandle;

   m_pSvcClient = pclientBase;
   ASSERT( NULL != m_pSvcClient );

   //
   // Allocate AIA service. Init is completed in serviceAllocated callback.
   //
   NamedValueSet nvsManifest;
   NamedValueSet nvsConfigRecord;

   nvsConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libaia");
   nvsConfigRecord.Add(keyRegAFU_ID,CCIP_PWR_GUID);

   nvsManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &nvsConfigRecord);
   nvsManifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AIA");

   m_tidSaved = TranID;
   getRuntime()->allocService(this, nvsManifest, TransactionID());

   // initComplete happens in serviceAllocated()
   return true;
}

//
// Release. Release service
//
btBool CPwrMgr::Release(TransactionID const &TranID, btTime timeout)
{
   // Release ALI / AFUProxy
   ASSERT(m_pAALService != NULL);
   if ( m_pAALService != NULL ) {
      // Wrap original transaction id and timeout
      ReleaseContext      *prc        = new ReleaseContext(TranID, timeout);
      btApplicationContext appContext = reinterpret_cast<btApplicationContext>(prc);

      return m_pAALService->Release(TransactionID(appContext), timeout);
   } else {
       return false;
   }
}

/*
 * IServiceClient methods (callbacks from AIA service)
 */

// Service allocated callback
void CPwrMgr::serviceAllocated(IBase               *pServiceBase,
                                TransactionID const &rTranID)
{
   // Store ResMgrService pointer
   m_pAFUProxy = dynamic_ptr<IAFUProxy>(iidAFUProxy, pServiceBase);
   if (!m_pAFUProxy) {
      // TODO: handle error
      AAL_ERR( LM_All," Missing AFUProxy interface ");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Error: Missing AFUProxy interface."));
      return;
   }

   // Store AAL service pointer
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   if (!m_pAALService) {
      // TODO: handle error
      AAL_ERR( LM_All," Missing service base interface ");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Error: Missing service base interface."));
      return;
   }

   setPwrMgrInterfaces();
   initComplete(m_tidSaved);
}

//
// setPwrMgrInterfaces. Sets power Manager interfaces
//
bool CPwrMgr::setPwrMgrInterfaces()
{
   if( EObjOK != SetInterface(iid_PWRMGR_Service, dynamic_cast<IPwrMgr *>(this)) ){
      goto FAIL;
   }

   m_pPwrMgrClient = dynamic_ptr<IPwrMgr_Client>(iid_PWRMGR_Service_Client, m_pSvcClient);

   ASSERT( NULL != m_pPwrMgrClient );
   if(NULL == m_pPwrMgrClient) {
      AAL_ERR( LM_All,"Missing IPwrMgr_Client interface ");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Client did not publish IPwrMgr_Client Interface"));
      return false;
   }
   return true;

FAIL:
   m_bIsOK = false;
   AAL_ERR( LM_All,"IPwrMgr SetInterface failed");
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errCreationFailure,
                                              reasUnknown,
                                             "Error: Could not register interface."));
   return false;
}

//
// reconfPowerResponse. sends PR power response as transaction.
//
btBool CPwrMgr::reconfPowerResponse(TransactionID const &rTranID ,
                                    NamedValueSet const &rInputArgs)
{
   btInt pr_PwrMgmtStatus  = -1;

   if(rInputArgs.Has(PWRMGMT_STATUS)) {
      rInputArgs.Get( PWRMGMT_STATUS, &pr_PwrMgmtStatus);
      //std::cout << "pr_pwrStatus "<< pr_PwrMgmtStatus <<std:: endl;
   }

   std::cout << "rTranID "<< rTranID.ID() <<std:: endl;
   // Create the Transaction
   PwrMgrResponse transaction(rTranID,ccipdrv_PwrMgrResponse,pr_PwrMgmtStatus);

   // Should never fail
   if ( !transaction.IsOK() ) {
      AAL_ERR( LM_All,"Reconf Power Response Transaction failed");
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      AAL_ERR( LM_All,"Reconf Power Response Transaction failed");
      return false;
   }

   return true;
}

//
// serviceReleaseRequest. Releases Service
//
void CPwrMgr::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
   AAL_ERR( LM_All,"Recieved unhandled serviceReleaseRequest() from AFU PRoxy");
}

// Service allocated failed callback
void CPwrMgr::serviceAllocateFailed(const IEvent &rEvent) {

   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errAllocationFailure,
                                              reasUnknown,
                                              "Error: Failed to allocate IPwrMgr."));

}

// Service released callback
void CPwrMgr::serviceReleased(TransactionID const &rTranID) {
   ReleaseContext *prc = reinterpret_cast<ReleaseContext *>(rTranID.Context());
   ServiceBase::Release(prc->TranID, prc->timeout);
}

// Service released failed callback
void CPwrMgr::serviceReleaseFailed(const IEvent &rEvent) {
   m_bIsOK = false;

   // TODO EMPTY
}

// Callback for generic events
void CPwrMgr::serviceEvent(const IEvent &rEvent) {
   // TODO: handle unexpected events
   ASSERT(false);
   AAL_ERR( LM_All,"Recieved unexpected events");
}
// ---------------------------------------------------------------------------
// IAFUProxyClient interface implementation
// ---------------------------------------------------------------------------

// Callback for ALIAFUProxy
void CPwrMgr::AFUEvent(AAL::IEvent const &theEvent)
{
   IUIDriverEvent *puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                           theEvent);

   //  if puidEvent is NULL return
   ASSERT(NULL != puidEvent);
   if(NULL == puidEvent){
      AAL_ERR( LM_All," Invalid ALIAFUProxy event");
      return;
   }

   //std::cerr << "Got AFU event type " << puidEvent->MessageID() << "\n" << std::endl;

   switch(puidEvent->MessageID())
   {
      // Revokes AFu from Owner
      case rspid_AFU_PR_Revoke_Event:
      {
         getRuntime()->schedDispatchable( new ServiceRevoke(dynamic_ptr<IServiceRevoke>(iidServiceRevoke,this)) );

      }
      break;

      // Releases AFu from Owner
      case rspid_AFU_PR_Release_Request_Event:
      {
         struct aalui_PREvent *pResult = reinterpret_cast<struct aalui_PREvent *>(puidEvent->Payload());
         ASSERT(NULL != pResult);

         if( NULL != pResult) {
            getRuntime()->schedDispatchable( new ReleaseServiceRequest(m_pSvcClient, new CReleaseRequestEvent(NULL,
                                                                                                              pResult->reconfTimeout,
                                                                                                              IReleaseRequestEvent::resource_revokeing,
                                                                                                              "AFU Release Request")) );

         } else {
            AAL_ERR( LM_All," Invalid AFU Release Request");
            getRuntime()->schedDispatchable( new ReleaseServiceRequest(m_pSvcClient, new CReleaseRequestEvent(NULL,
                                                                                                              0,
                                                                                                              IReleaseRequestEvent::resource_revokeing,
                                                                                                              "Invalid AFU Release Request")) );
         }

      }
      break;

      // PR Power Request Event
      case rspid_PR_Power_Request_Event:
      {
         struct aalui_PwrMgrReconfEvent *pResult = reinterpret_cast<struct aalui_PwrMgrReconfEvent *>(puidEvent->Payload());
         ASSERT(NULL != pResult);

         if( NULL != pResult) {

            //std::cout << "CPwrMgr::AFUEvent rspid_AFU_PR_Power_Event \n"<< pResult->Reconf_PwrRequired << std::endl;
            getRuntime()->schedDispatchable( new reconfPowerRequestEvent(m_pPwrMgrClient, new CExceptionEvent(NULL,
                                                                                                              errOK,
                                                                                                              errOK,
                                                                                                              "PR Power Request"),
                                                                                                               pResult) );

         } else {
            AAL_ERR( LM_All," Invalid PR Power Request");
            getRuntime()->schedDispatchable( new reconfPowerRequestEvent(m_pPwrMgrClient, new CExceptionEvent(NULL,
                                                                                                              errBadParameter,
                                                                                                              reasInvalidParameter,
                                                                                                              "Invalid PR Power Request"),
                                                                                                               pResult) );

         }

      }
      break;

      // default
      default:
         ASSERT(false); // unexpected event
   }

}

/// @} group ALIPwrMgr

END_NAMESPACE(AAL)


#if defined( __AAL_WINDOWS__ )

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
         break;
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::CPwrMgr >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

PWRMGR_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
PWRMGR_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
