// Copyright (c) 2014-2015, Intel Corporation
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
/// @file ICCIClient.h
/// @brief ICCIClient definition.
/// @ingroup ICCIAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/21/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_ICCICLIENT_H__
#define __AALSDK_SERVICE_ICCICLIENT_H__
#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/aas/AALDeviceService.h>
#include <aalsdk/kernel/AALWorkspace.h>         // TTASKMODE
#include <aalsdk/faptrans/FAP10.h>              // WkSp_Single_Allocate_AFUTransaction
#include <aalsdk/utils/AALEventUtilities.h>     // UnWrapTransactionIDFromEvent
#include <aalsdk/uaia/AALuAIA_UIDriverClient.h> // IUIDriverClientEvent


BEGIN_NAMESPACE(AAL)

/// @addtogroup ICCIAFU
/// @{

/// CCI Client interface.
/// @brief Defines the notification interface for clients of CCI AFU's.
class ICCIClient
{
public:
   virtual ~ICCIClient() {}

   /// @brief Notification callback for workspace allocation success.
   ///
   /// Sent in response to a successful workspace allocation request (ICCIAFU::WorkspaceAllocate).
   ///
   /// @param[in]  TranID     The transaction ID provided in the call to ICCIAFU::WorkspaceAllocate.
   /// @param[in]  WkspcVirt  The user virtual address of the newly-allocated workspace.
   /// @param[in]  WkspcPhys  The physical address of the newly-allocated workspace.
   /// @param[in]  WkspcSize  The size in bytes of the allocation.
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize) = 0;

   /// @brief Notification callback for workspace free success.
   ///
   /// Sent in response to a successful free workspace request (ICCIAFU::WorkspaceFree).
   ///
   /// @param[in]  TranID  The transaction ID provided in the call to ICCIAFU::WorkspaceFree.
   virtual void          OnWorkspaceFreed(TransactionID const &TranID)    = 0;

   /// @brief Notification callback for workspace allocation failure.
   ///
   /// Sent in response to a failed workspace allocation request (ICCIAFU::WorkspaceAllocate).
   ///
   /// @param[in]  Event  An IExceptionTransactionEvent describing the failure.
   virtual void OnWorkspaceAllocateFailed(const IEvent &Event)            = 0;

   /// @brief Notification callback for workspace free failure.
   ///
   /// Sent in response to a failed free workspace request (ICCIAFU::WorkspaceFree).
   ///
   /// @param[in]  Event  An IExceptionTransactionEvent describing the failure.
   virtual void     OnWorkspaceFreeFailed(const IEvent &Event)            = 0;
};

#define iidCCIClient __INTC_IID(INTC_sysSampleAFU, 0x0007)

/// Notification functor for workspace allocate success.
class CCIClientWorkspaceAllocated : public IDispatchable
{
public:
   CCIClientWorkspaceAllocated(ICCIClient   *pRecipient,
                               TransactionID TranID,
                               btVirtAddr    WkspcVirt,
                               btPhysAddr    WkspcPhys,
                               btWSSize      WkspcSize) :
      m_pRecipient(pRecipient),
      m_TranID(TranID),
      m_WkspcVirt(WkspcVirt),
      m_WkspcPhys(WkspcPhys),
      m_WkspcSize(WkspcSize)
   {
      ASSERT(NULL != m_pRecipient);
   }

   virtual void operator() ()
   {
      m_pRecipient->OnWorkspaceAllocated(m_TranID, m_WkspcVirt, m_WkspcPhys, m_WkspcSize);
      delete this;
   }

protected:
   ICCIClient   *m_pRecipient;
   TransactionID m_TranID;
   btVirtAddr    m_WkspcVirt;
   btPhysAddr    m_WkspcPhys;
   btWSSize      m_WkspcSize;
};

/// Notification functor for workspace allocate failure.
class CCIClientWorkspaceAllocateFailed : public IDispatchable
{
public:
   CCIClientWorkspaceAllocateFailed(ICCIClient *pRecipient,
                                    IEvent     *pExcept) :
      m_pRecipient(pRecipient),
      m_pExcept(pExcept)
   {
      ASSERT(NULL != m_pRecipient);
      ASSERT(NULL != m_pExcept);
   }
   ~CCIClientWorkspaceAllocateFailed()
   {
      if ( NULL != m_pExcept ) {
         delete m_pExcept;
      }
   }

   virtual void operator() ()
   {
      m_pRecipient->OnWorkspaceAllocateFailed(*m_pExcept);
      delete this;
   }

protected:
   ICCIClient *m_pRecipient;
   IEvent     *m_pExcept;
};

/// Notification functor for workspace free success.
class CCIClientWorkspaceFreed : public IDispatchable
{
public:
   CCIClientWorkspaceFreed(ICCIClient   *pRecipient,
                           TransactionID TranID) :
      m_pRecipient(pRecipient),
      m_TranID(TranID)
   {
      ASSERT(NULL != m_pRecipient);
   }

   virtual void operator() ()
   {
      m_pRecipient->OnWorkspaceFreed(m_TranID);
      delete this;
   }

protected:
   ICCIClient   *m_pRecipient;
   TransactionID m_TranID;
};

/// Notification functor for workspace free failure.
class CCIClientWorkspaceFreeFailed : public IDispatchable
{
public:
   CCIClientWorkspaceFreeFailed(ICCIClient *pRecipient,
                                IEvent     *pExcept) :
      m_pRecipient(pRecipient),
      m_pExcept(pExcept)
   {
      ASSERT(NULL != m_pRecipient);
      ASSERT(NULL != m_pExcept);
   }
   ~CCIClientWorkspaceFreeFailed()
   {
      if ( NULL != m_pExcept ) {
         delete m_pExcept;
      }
   }

   virtual void operator() ()
   {
      m_pRecipient->OnWorkspaceFreeFailed(*m_pExcept);
      delete this;
   }

protected:
   ICCIClient *m_pRecipient;
   IEvent     *m_pExcept;
};

/// @} group ICCIAFU

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

template <TTASK_MODE mode>
class HWAFUWkspcDelegate : public DeviceServiceBase
{
public:
   DECLARE_AAL_SERVICE_CONSTRUCTOR(HWAFUWkspcDelegate, DeviceServiceBase) {}

protected:
   void WkspcAlloc(btWSSize ,
                   TransactionID const & );
   void  WkspcFree(btVirtAddr ,
                   TransactionID const & );
   static void AllocateWorkSpaceHandler(IEvent const & );
   static void FreeWorkSpaceHandler(IEvent const & );
};

template <TTASK_MODE mode>
void HWAFUWkspcDelegate<mode>::WkspcAlloc(btWSSize             Length,
                                          TransactionID const &TranID)
{
   // Create a transaction id that wraps the original from the application,
   // Otherwise the return transaction will go straight back there
   TransactionID tid(new(std::nothrow) TransactionID(TranID),
                     HWAFUWkspcDelegate<mode>::AllocateWorkSpaceHandler,
                     true);

   // Create the Transaction
   WkSp_Single_Allocate_AFUTransaction AFUTran(Length, mode);

   // Check the parameters
   if ( AFUTran.IsOK() ) {
      // Will return to AllocateWorkspaceHandler, below.

      AFUDev().SendTransaction(&AFUTran, tid);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasAFUNoMemory,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable(
         new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                            pExcept)

             );
   }

}

template <TTASK_MODE mode>
void HWAFUWkspcDelegate<mode>::WkspcFree(btVirtAddr           Address,
                                         TransactionID const &TranID)
{
   // Create a transaction id that wraps the original from the application,
   //    Otherwise the return transaction will go straight back there
   // The AFU can use the new transaction id's Context and EventHandler
   //    to carry information around.
   TransactionID tid(new(std::nothrow) TransactionID(TranID),
                     HWAFUWkspcDelegate<mode>::FreeWorkSpaceHandler,
                     true);

   WkSp_Single_Free_AFUTransaction AFUTran(&AFUDev(), Address);

   if ( AFUTran.IsOK() ) {

      AFUDev().SendTransaction(&AFUTran, tid);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasUnknown,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable(
         new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                        pExcept)
             );
   }
}

template <TTASK_MODE mode>
void HWAFUWkspcDelegate<mode>::AllocateWorkSpaceHandler(IEvent const &theEvent)
{
   // The object that generated the event (AIAProxy) has our this as its context
   HWAFUWkspcDelegate<mode> *This = static_cast<HWAFUWkspcDelegate<mode> *>(theEvent.Object().Context());

   // Recover the original wrapped TransactionID
   TransactionID OrigTranID = UnWrapTransactionIDFromEvent(theEvent);

   // Need the event in order to get its payload
   IUIDriverClientEvent &revt = subclass_ref<IUIDriverClientEvent>(theEvent);

   // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
   struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(revt.Payload());

   btcString descr = NULL;

   // Check for exception
   if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
      descr = "SubClassID() was exception";
      goto _SEND_ERR;
   }

   // ALL THE CODE IN THIS BLOCK IS JUST DEBUG CHECK CODE - none of these EVER fire unless
   //    the framework is broken
   {
      // Debug Check; Expect it to be a tranevtUIDriverClientEvent
      if ( tranevtUIDriverClientEvent != theEvent.SubClassID() ) {
         ERR("Expected tranevtUIDriverClientEvent, got " << theEvent.SubClassID());
         goto _SEND_ERR;
      }
      VERBOSE("Got tranevtUIDriverClientEvent");

      // Debug Check; Expect it to be an rspid_WSM_Response
      if ( rspid_WSM_Response != revt.MessageID() ) {
         ERR("Expected rspid_WSM_Response, got " << revt.MessageID());
         goto _SEND_ERR;
      }
      VERBOSE("Got tranevtUIDriverClientEvent->rspid_WSM_Response.");

      // Debug Check, expect a payload
      if ( !pResult ) {
         ERR("No payload, sending Exception Transaction");
         goto _SEND_ERR;
      }

      // Debug Check, pResult->evtID should be uid_wseventAllocate
      if ( uid_wseventAllocate != pResult->evtID ) {
         ERR("LOGIC ERROR: not an uid_wseventAllocate; sending Exception Transaction");
         goto _SEND_ERR;
      }
   }  // End of DEBUG CHECK CODE

   // REAL CODE
   if ( uid_errnumOK == revt.ResultCode() ) {      // Have a valid memory allocation

      // Send the message
      This->getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, This->ClientBase()),
                                                                                           OrigTranID,
                                                                                           pResult->wsParms.ptr,
                                                                                           pResult->wsParms.physptr,
                                                                                           pResult->wsParms.size) );

   } else {    // error during allocate
      // get an error string
      descr = "bad ResultCode()";
      goto _SEND_ERR;
   }

   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(This),
                                                                  OrigTranID,
                                                                  errAFUWorkSpace,
                                                                  reasAFUNoMemory,
                                                                  descr);
   This->getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, This->ClientBase()),
                                                                                                                     pExcept) );
}

template <TTASK_MODE mode>
void HWAFUWkspcDelegate<mode>::FreeWorkSpaceHandler(IEvent const &theEvent)
{
   // The object that generated the event (AIAProxy) has our this as its context
   HWAFUWkspcDelegate<mode> *This = static_cast<HWAFUWkspcDelegate<mode> *>(theEvent.Object().Context());

   // Recover the original wrapped TransactionID
   TransactionID OrigTranID = UnWrapTransactionIDFromEvent(theEvent);

   // Need the event in order to get its payload
   IUIDriverClientEvent &revt = subclass_ref<IUIDriverClientEvent>(theEvent);

   // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
   struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(revt.Payload());

   btcString descr = NULL;

   // Check for exception
   if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
      descr = "SubClassID() was exception";
      goto _SEND_ERR;
   }

   // ALL THE CODE IN THIS BLOCK IS JUST DEBUG CHECK CODE - none of these EVER fire unless
   //    the framework is broken
   {
      // Debug Check; Expect it to be a tranevtUIDriverClientEvent
      if ( tranevtUIDriverClientEvent != theEvent.SubClassID() ) {
         ERR("Expected tranevtUIDriverClientEvent, got " << theEvent.SubClassID());
         goto _SEND_ERR;
      }
      VERBOSE("Got tranevtUIDriverClientEvent");

      // Debug Check; Expect it to be an rspid_AFU_Response
      if ( rspid_WSM_Response != revt.MessageID() ) {
         ERR("Expected rspid_AFU_Response, got " << revt.MessageID());
         goto _SEND_ERR;
      }
      VERBOSE("Got tranevtUIDriverClientEvent->rspid_AFU_Response");

      // Debug Check, expect a payload
      if ( !pResult ) {
         ERR("No payload, sending Exception Transaction");
         goto _SEND_ERR;
      }

      // Debug Check, pResult->evtID should be uid_wseventFree
      if ( uid_wseventFree != pResult->evtID ) {
         ERR("LOGIC ERROR: not an uid_wseventFree; sending Exception Transaction");
         goto _SEND_ERR;
      }
   }  // End of DEBUG CHECK code

   // REAL CODE
   if ( uid_errnumOK == revt.ResultCode() ) {      // Have a valid memory free

      // Send the message
      This->getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, This->ClientBase()),
                                                                                       OrigTranID) );

   } else {    // error during free
      // get an error string
      descr = "bad ResultCode()";
      goto _SEND_ERR;
   }

   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(This),
                                                                  OrigTranID,
                                                                  errAFUWorkSpace,
                                                                  reasAFUNoMemory,
                                                                  descr);
   This->getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, This->ClientBase()),
                                                                                         pExcept) );
}


END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_ICCICLIENT_H__

