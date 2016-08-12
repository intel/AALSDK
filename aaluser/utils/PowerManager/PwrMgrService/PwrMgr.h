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
/// @file PwrMgr.h
/// @brief Definitions for PwrMgr  Service.
/// @ingroup PowerManger
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
/// 07/25/2016     AR       Initial version.@endverbatim
//****************************************************************************
#ifndef __PWRMGR_SERVICE_H__
#define __PWRMGR_SERVICE_H__
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/PwrMgrService.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/service/IPwrMgr.h>
#include <aalsdk/uaia/IAFUProxy.h>


BEGIN_NAMESPACE(AAL)

/// @addtogroup PwrMgr
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IPwrMgR is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IPwrMgr' used as base for dll-interface class 'AAL::CPwrMgr'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by CPwrMgr to interact with an FPGA-accelerated
///        CCI.
///
class PWRMGR_API CPwrMgr : public ServiceBase,
                           public IServiceClient,
                           public IAFUProxyClient ,
                           public IPwrMgr
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <DeviceServiceBase>
DECLARE_AAL_SERVICE_CONSTRUCTOR(CPwrMgr,ServiceBase),
      m_pAALService(NULL),
      m_pAFUProxy(NULL),
      m_tidSaved(),
      m_pSvcClient(NULL),
      m_pPwrMgrClient(NULL)
   {

      if ( EObjOK != SetInterface(iidAFUProxyClient, dynamic_cast<IAFUProxyClient *>(this)) ){
         m_bIsOK = false;
      }  // for AFUProy

      if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ){
            m_bIsOK = false;
         } // for AIA
   }  // DECLARE_AAL_SERVICE_CONSTRUCTOR()

   virtual btBool init(IBase               *pclientBase,
                       NamedValueSet const &optArgs,
                       TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   // </DeviceServiceBase>

   // </IPwrMgr>
   virtual btBool reconfPowerResponse(TransactionID const &rTranID ,
                                      NamedValueSet const &rInputArgs) ;
   // </IPwrMgr>

   btBool setPwrMgrInterfaces();

   // <IServiceClient>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID = TransactionID());  // FIXME: potential dangling reference
   virtual void serviceAllocateFailed(const IEvent &rEvent);
   virtual void serviceReleased(TransactionID const &rTranID = TransactionID());  // FIXME: potential dangling reference
   virtual void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
   virtual void serviceReleaseFailed(const IEvent &rEvent);
   virtual void serviceEvent(const IEvent &rEvent);
   // </IServiceClient>

   // <IAFUProxyClient>
   virtual void AFUEvent(AAL::IEvent const &theEvent);
   // </IAFUProxyClient>

protected:

   IAALService            *m_pAALService;
   IAFUProxy              *m_pAFUProxy;
   TransactionID           m_tidSaved;
   IBase                  *m_pSvcClient;
   IPwrMgr_Client         *m_pPwrMgrClient;

   struct ReleaseContext {
      const TransactionID   TranID;
      const btTime          timeout;
      ReleaseContext(const TransactionID &rtid, btTime to) :
         TranID(rtid),
         timeout(to)
      {}
   };

};

// Power Request Event Dispatchable event
class reconfPowerRequestEvent : public IDispatchable
{
public:
   reconfPowerRequestEvent( IPwrMgr_Client   *pSvcClient,
                            const IEvent     *pEvent,
                            struct aalui_PwrMgrReconfEvent* pPwrMgrEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent)
   {
      if(NULL != pPwrMgrEvent) {

         m_rTranID = pPwrMgrEvent->m_tranID;

         m_rResult.Add(PWRMGR_SOCKETID,pPwrMgrEvent->m_SocketID);
         m_rResult.Add(PWRMGR_BUSID,pPwrMgrEvent->m_BusID);
         m_rResult.Add(PWRMGR_DEVICEID,pPwrMgrEvent->m_DeviceID);
         m_rResult.Add(PWRMGR_FUNID,pPwrMgrEvent->m_FunctionID);
         m_rResult.Add(PWRMGR_RECONF_PWRREQUIRED,pPwrMgrEvent->m_Reconf_PwrRequired);
      }
   }

   virtual void operator() ()
   {
      m_pSvcClient->reconfPowerRequest(m_rTranID,*m_pEvent,m_rResult);

   }

protected:
   IPwrMgr_Client                *m_pSvcClient;
   const IEvent                  *m_pEvent;
   NamedValueSet                  m_rResult;
   TransactionID                  m_rTranID;

};

/// @}

END_NAMESPACE(AAL)

#endif // __PWRMGR_SERVICE_H__

