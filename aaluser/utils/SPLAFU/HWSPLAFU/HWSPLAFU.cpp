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
/// @file HWSPLAFU.cpp
/// @brief Implementation of SPL AFU Hardware Service.
/// @ingroup HWSPLAFU
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
/// This sample demonstrates how to create an AFU Service that uses a host-based AFU engine.
///  This design also applies to AFU Services that use hardware via a
///  Physical Interface Protocol (PIP) module.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/18/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/faptrans/FAP20.h>

#include "HWSPLAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWSPLAFU
/// @{

void HWSPLAFU::init(TransactionID const &TranID)
{
   ISPLClient *pClient = dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase());
   ASSERT( NULL != pClient );
   if(NULL == pClient){
      /// ObjectCreatedExceptionEvent Constructor.
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                      Client(),
                                                                      this,
                                                                      TranID,
                                                                      errBadParameter,
                                                                      reasMissingInterface,
                                                                      "Client did not publish ISPLClient Interface"));
      return;
   }

   getRuntime()->schedDispatchable( new(std::nothrow) ObjectCreatedEvent(getRuntimeClient(),
                                                                         Client(),
                                                                         dynamic_cast<IBase *>(this),
                                                                         TranID) );
}

btBool HWSPLAFU::Release(TransactionID const &TranID, btTime timeout)
{
   return DeviceServiceBase::Release(TranID, timeout);
}

btBool HWSPLAFU::Release(btTime timeout)
{
   return DeviceServiceBase::Release(timeout);
}

void HWSPLAFU::WorkspaceAllocate(btWSSize             Length,
                                 TransactionID const &TranID)
{
   AutoLock(this);
   WkspcAlloc(Length, TranID);
}

void HWSPLAFU::WorkspaceFree(btVirtAddr           Address,
                             TransactionID const &TranID)
{
   AutoLock(this);
   WkspcFree(Address, TranID);
}

btBool HWSPLAFU::CSRRead(btCSROffset CSR,
                         btCSRValue *pValue)
{
   return AFUDev().atomicGetCSR(CSR, pValue);
}

btBool HWSPLAFU::CSRWrite(btCSROffset CSR,
                          btCSRValue  Value)
{
   return AFUDev().atomicSetCSR(CSR, Value);
}

btBool HWSPLAFU::CSRWrite64(btCSROffset CSR,
                            bt64bitCSR  Value)
{
   if ( CSRWrite(CSR + 4, Value >> 32) ) {
      return CSRWrite(CSR, Value & 0xffffffff);
   }
   return false;
}

void HWSPLAFU::StartTransactionContext(TransactionID const &TranID,
                                       btVirtAddr           Address,
                                       btTime               Pollrate)
{
   AutoLock(this);

   TransactionID tid = WrapTransactionID(TranID);
   tid.Handler(HWSPLAFU::TransactionHandler);

   SPL2_Start_AFUTransaction AFUTran(&AFUDev(),
                                     TranID,
                                     Address,
                                     Pollrate);

   // Check the parameters
   if ( AFUTran.IsOK() ) {

      AFUDev().SendTransaction(&AFUTran, tid);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasUnknown,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                    pExcept) );
   }
}

void HWSPLAFU::StopTransactionContext(TransactionID const &TranID)
{
   AutoLock(this);

   TransactionID tid = WrapTransactionID(TranID);
   tid.Handler(HWSPLAFU::TransactionHandler);

   SPL2_Stop_AFUTransaction AFUTran(&AFUDev(), TranID);

   // Check the parameters
   if ( AFUTran.IsOK() ) {

      AFUDev().SendTransaction(&AFUTran, tid);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasUnknown,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                    pExcept) );
   }
}

void HWSPLAFU::SetContextWorkspace(TransactionID const &TranID,
                                   btVirtAddr           Address,
                                   btTime               Pollrate)
{
   // Address is required.
   ASSERT(Address != NULL);

   AutoLock(this);

   TransactionID tid = WrapTransactionID(TranID);
   tid.Handler(HWSPLAFU::TransactionHandler);

   SPL2_SetContextWorkspace_AFUTransaction AFUTran(&AFUDev(), TranID, Address, Pollrate);

   // Check the parameters
   if ( AFUTran.IsOK() ) {

      AFUDev().SendTransaction(&AFUTran, tid);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasUnknown,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                    pExcept) );
   }
}

void HWSPLAFU::TransactionHandler(const IEvent &theEvent)
{
   // The object that generated the event (AIAProxy) has our this as its context
   HWSPLAFU *This = static_cast<HWSPLAFU *>(theEvent.Object().Context());

   // Need the event in order to get its payload
   IUIDriverClientEvent &revt = subclass_ref<IUIDriverClientEvent>(theEvent);

   // Since MessageID is rspid_AFU_Response, Payload is struct aalui_AFUResponse, defined in aalui.h
   struct aalui_AFUResponse *pResponse =
            reinterpret_cast<struct aalui_AFUResponse *>(revt.Payload());

   TransactionID tid;
   btcString     descr = NULL;

   if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
      descr = "SubClassID() was exception";
      goto _SEND_ERR;
   }

   // Expect it to be a tranevtUIDriverClientEvent; just error check
   ASSERT(tranevtUIDriverClientEvent == theEvent.SubClassID());
   if ( tranevtUIDriverClientEvent != theEvent.SubClassID() ) {
      AAL_ERR(LM_AFU, __AAL_FUNCSIG__ <<
                       ": expected tranevtUIDriverClientEvent, got " << theEvent.SubClassID() << endl);
      goto _SEND_ERR;
   }

   // Expect it to be an rspid_AFU_Response; just error check
   ASSERT(rspid_AFU_Response == revt.MessageID());
   if ( rspid_AFU_Response != revt.MessageID() ) {
      AAL_ERR(LM_AFU, __AAL_FUNCSIG__ << ": expected rspid_AFU_Response, got " << revt.MessageID() << endl);
      goto _SEND_ERR;
   }

   // Do not expect a NULL payload
   ASSERT(NULL != pResponse);
   if ( NULL == pResponse ) {
      descr = "No payload in response to Transaction20 message";
      AAL_ERR(LM_AFU, __AAL_FUNCSIG__ << ": " << descr << endl);
      goto _SEND_ERR;
   }

   // pResponse is valid

   switch ( pResponse->respID ) {

      case uid_afurespTaskStarted : {
         if ( uid_errnumOK == revt.ResultCode() ) {

            // Get the WSMParms for the AFU DSM from the response payload.
            struct aalui_WSMParms *pWSParms = reinterpret_cast<struct aalui_WSMParms *>( aalui_AFURespPayload(pResponse) );

            // Get the original TransactionID, but don't delete the wrapper copy and don't
            //  remove it from theEvent.TranID().Context()
            TransactionID OrigTid = UnWrapTransactionIDFromEvent(theEvent, false);

            This->getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, This->ClientBase()),
                                                                                                 OrigTid,
                                                                                                 pWSParms->ptr,
                                                                                                 pWSParms->size) );
            } else {
               // Delete the wrapper.
               (void) UnWrapTransactionIDFromEvent(theEvent, true);

               descr = "bad ResultCode()";
               tid = revt.msgTranID();
               goto _SEND_ERR;
            }
      } break;

      case uid_afurespTaskStopped : {
         if ( uid_errnumOK == revt.ResultCode() ) {

            // Get the WSMParms for the AFU DSM from the response payload.
            struct aalui_WSMParms *pWSParms = reinterpret_cast<struct aalui_WSMParms *>( aalui_AFURespPayload(pResponse) );

            // Get the original TransactionID, but don't delete the wrapper copy and don't
            //  remove it from theEvent.TranID().Context()
            TransactionID OrigTid = UnWrapTransactionIDFromEvent(theEvent, false);

            This->getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStopped(dynamic_ptr<ISPLClient>(iidSPLClient, This->ClientBase()),
                                                                                                 OrigTid) );
            } else {
               // Delete the wrapper.
               (void) UnWrapTransactionIDFromEvent(theEvent, true);

               descr = "bad ResultCode()";
               tid = revt.msgTranID();
               goto _SEND_ERR;
            }
      }break;

      case uid_afurespSetContext : {
         if ( uid_errnumOK == revt.ResultCode() ) {

            // Get the original TransactionID, but don't delete the wrapper copy and don't
            //  remove it from theEvent.TranID().Context()
            TransactionID OrigTid = UnWrapTransactionIDFromEvent(theEvent, false);

            This->getRuntime()->schedDispatchable( new(std::nothrow) SPLClientContextWorkspaceSet(dynamic_ptr<ISPLClient>(iidSPLClient, This->ClientBase()),
                                                                                                  OrigTid) );

            } else {
               // Delete the wrapper.
               (void) UnWrapTransactionIDFromEvent(theEvent, true);

               descr = "bad ResultCode()";
               tid = revt.msgTranID();
               goto _SEND_ERR;
            }
      } break;

      case uid_afurespTaskComplete : {
         // Unwrap and delete wrapper.
         TransactionID OrigTranID = UnWrapTransactionIDFromEvent(theEvent, true);

         if ( uid_errnumOK == revt.ResultCode() ) {

            This->getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionComplete(dynamic_ptr<ISPLClient>(iidSPLClient, This->ClientBase()),
                                                                                                  OrigTranID) );
         } else {

            descr = "bad ResultCode()";
            tid = revt.msgTranID();
            goto _SEND_ERR;
         }
      } break;

      default : {
         descr = "respID not in enumeration: LOGIC ERROR.";
         AAL_ERR(LM_AFU, __AAL_FUNCSIG__ << ": " << descr << endl);
         ASSERT(false);
         goto _SEND_ERR;
      } break;
   }


   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast <IBase *>(This),
                                                                  tid,
                                                                  errInternal,
                                                                  reasCauseUnknown,
                                                                  descr);
   This->getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, This->ClientBase()),
                                                                                       pExcept) );
}

/// @} group HWSPLAFU

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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::HWSPLAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

HWSPLAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
HWSPLAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

