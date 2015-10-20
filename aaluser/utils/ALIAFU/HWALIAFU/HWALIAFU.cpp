// Copyright (c) 2015, Intel Corporation
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
/// @file HWALIAFU.cpp
/// @brief Implementation of ALI AFU Hardware Service.
/// @ingroup HWALIAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// This sample demonstrates how to create an AFU Service that uses a host-based AFU engine.
///  This design also applies to AFU Services that use hardware via a
///  Physical Interface Protocol (PIP) module.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "ALIAIATransactions.h"

#include "HWALIAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWALIAFU
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
// Dispatchables for client callbacks
//
// ===========================================================================

//
// Dispatchable for IALIBuffer_Client::bufferAllocated
//
class BufferAllocated : public IDispatchable
{
public:
   BufferAllocated(IALIBuffer_Client *pRecipient,
                   TransactionID     TranID,
                   btVirtAddr        WkspcVirt,
                   btPhysAddr        WkspcPhys,
                   btWSSize          WkspcSize) :
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
	   // FIXME: IALIBuffer_Client::bufferAllocated doen't take a physical address anymore
	   //        Remove from dispatchable
	   // 	  m_pRecipient->bufferAllocated(m_TranID, m_WkspcVirt, m_WkspcPhys, m_WkspcSize);
	   m_pRecipient->bufferAllocated(m_TranID, m_WkspcVirt, m_WkspcSize);
	   delete this;
   }

protected:
   IALIBuffer_Client *m_pRecipient;
   TransactionID     m_TranID;
   btVirtAddr        m_WkspcVirt;
   btPhysAddr        m_WkspcPhys;
   btWSSize          m_WkspcSize;
};

//
// Dispatchable for IALIBuffer_Client::bufferAllocateFailed
//
class BufferAllocateFailed : public IDispatchable
{
public:
   BufferAllocateFailed(IALIBuffer_Client *pRecipient,
                        IEvent            *pExcept) :
      m_pRecipient(pRecipient),
      m_pExcept(pExcept)
   {
      ASSERT(NULL != m_pRecipient);
      ASSERT(NULL != m_pExcept);
   }
   ~BufferAllocateFailed()
   {
      if ( NULL != m_pExcept ) {
         delete m_pExcept;
      }
   }

   virtual void operator() ()
   {
      m_pRecipient->bufferAllocateFailed(*m_pExcept);
      delete this;
   }

protected:
   IALIBuffer_Client *m_pRecipient;
   IEvent     *m_pExcept;
};

//
// Dispatchable for IALIBuffer_Client::bufferFreed
//
class BufferFreed : public IDispatchable
{
public:
   BufferFreed(IALIBuffer_Client   *pRecipient,
                           TransactionID       TranID) :
      m_pRecipient(pRecipient),
      m_TranID(TranID)
   {
      ASSERT(NULL != m_pRecipient);
   }

   virtual void operator() ()
   {
      m_pRecipient->bufferFreed(m_TranID);
      delete this;
   }

protected:
   IALIBuffer_Client   *m_pRecipient;
   TransactionID m_TranID;
};

//
// Dispatchable for IALIBuffer_Client::bufferFreeFailed
//
class BufferFreeFailed : public IDispatchable
{
public:
   BufferFreeFailed(IALIBuffer_Client *pRecipient,
                    IEvent            *pExcept) :
      m_pRecipient(pRecipient),
      m_pExcept(pExcept)
   {
      ASSERT(NULL != m_pRecipient);
      ASSERT(NULL != m_pExcept);
   }
   ~BufferFreeFailed()
   {
      if ( NULL != m_pExcept ) {
         delete m_pExcept;
      }
   }

   virtual void operator() ()
   {
      m_pRecipient->bufferFreeFailed(*m_pExcept);
      delete this;
   }

protected:
   IALIBuffer_Client *m_pRecipient;
   IEvent     *m_pExcept;
};



// ===========================================================================
//
// HWALIAFU implementation
//
// ===========================================================================

//
// init. Does nothing but dispatch ObjectCreatedEvent.
//
// TODO: Add checks for AFUDev capabilities, possibly selective exposure of
//       interfaces based on results
//
btBool HWALIAFU::init(IBase *pclientBase,
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
   //nvsConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
   nvsConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
   nvsManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &nvsConfigRecord);
   nvsManifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AIA");

   // add hardware handle obtained by resource manager
   if( optArgs.Has(keyRegHandle) ) {
      optArgs.Get(keyRegHandle, &devHandle);
   }else {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                TranID,
                                                errBadParameter,
                                                reasNoDevice,
                                                "No device handle in Configuration Record!"));
      return true;
    }
   nvsManifest.Add(keyRegHandle, devHandle);

   m_tidSaved = TranID;
   getRuntime()->allocService(this, nvsManifest, TransactionID());

   // initComplete happens in serviceAllocated()
   return true;
}

//
// Release. Release service
//
btBool HWALIAFU::Release(TransactionID const &TranID, btTime timeout)
{
   return ServiceBase::Release(TranID, timeout);
}


// ---------------------------------------------------------------------------
// IALIMMIO interface implementation
// ---------------------------------------------------------------------------

//
// mmioGetAddress. Return address of MMIO space.
//
btVirtAddr HWALIAFU::mmioGetAddress( void )
{
	// FIXME: might want to cache this
//	return AFUDev().getMMIOR();
   return NULL;
}

//
// mmioGetLength. Return length of MMIO space.
//
btCSROffset HWALIAFU::mmioGetLength( void )
{
//	return AFUDev().getMMIORsize();
   return 0;
}

//
// mmioRead32. Read 32bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioRead32( const btCSROffset Offset, btUnsigned32bitInt * const pValue)
{
	btVirtAddr pMMIOBase = mmioGetAddress();

	if (pMMIOBase == NULL) {
		return false;
	}

	*pValue = *( (btUnsigned32bitInt*)(pMMIOBase + Offset) );

	return true;
}

//
// mmioWrite32. Write 32bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value)
{
	btVirtAddr pMMIOBase = mmioGetAddress();

	if (pMMIOBase == NULL) {
		return false;
	}

	*( (btUnsigned32bitInt*)(pMMIOBase + Offset) ) = Value;

	return true;
}

//
// mmioRead64. Read 64bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioRead64( const btCSROffset Offset, btUnsigned64bitInt * const pValue)
{
	btVirtAddr pMMIOBase = mmioGetAddress();

	if (pMMIOBase == NULL) {
		return false;
	}

	*pValue = *( (btUnsigned64bitInt*)(pMMIOBase + Offset) );

	return true;
}

//
// mmioWrite64. Write 64bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value)
{
	btVirtAddr pMMIOBase = mmioGetAddress();

	if (pMMIOBase == NULL) {
		return false;
	}

	*( (btUnsigned64bitInt*)(pMMIOBase + Offset) ) = Value;

	return true;
}

// ---------------------------------------------------------------------------
// IALIBuffer interface implementation
// ---------------------------------------------------------------------------

//
// bufferAllocate. Allocate a shared buffer (formerly known as workspace).
//
void HWALIAFU::bufferAllocate( btWSSize             Length,
                               TransactionID const &TranID,
                               NamedValueSet       *pOptArgs)
{
	   AutoLock(this);
/*
	   // Create a transaction id that wraps the original from the application,
	   // Otherwise the return transaction will go straight back there
	   TransactionID tid(new(std::nothrow) TransactionID(TranID),
	                     HWALIAFU::AllocateBufferHandler,
	                     true);
*/
	   // Create the request to bundle in the transaction

	   // Create the Transaction
	   BufferAllocate transaction(TranID, Length);

	   // Check the parameters
	   if ( transaction.IsOK() ) {
	      // Will return to AllocateBufferHandler, below.
	      m_pAFUProxy->SendTransaction(&transaction);
	   } else {
	      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(m_pSvcClient,
	                                                                     TranID,
	                                                                     errAFUWorkSpace,
	                                                                     reasAFUNoMemory,
	                                                                     "BufferAllocate transaction validity check failed");
	      getRuntime()->schedDispatchable(
	         new(std::nothrow) BufferAllocateFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, this),
	                                                            pExcept)

	             );
	   }
}

//
// bufferFree. Release previously allocated buffer.
//
void HWALIAFU::bufferFree( btVirtAddr           Address,
                             TransactionID const &TranID)
{
   // FIXME: port to IAFUProxy
/*	   AutoLock(this);

	   // Create a transaction id that wraps the original from the application,
	   //    Otherwise the return transaction will go straight back there
	   // The AFU can use the new transaction id's Context and EventHandler
	   //    to carry information around.
	   TransactionID tid(new(std::nothrow) TransactionID(TranID),
	                     HWALIAFU::FreeBufferHandler,
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
	         new(std::nothrow) BufferFreeFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, ClientBase()),
	                                                        pExcept)
	             );
	   }
	   */
}

//
// bufferGetIOVA. Retrieve IO Virtual Address for a virtual address.
//
btPhysAddr HWALIAFU::bufferGetIOVA( btVirtAddr Address)
{
   // FIXME: port to IAFUProxy
/*   WorkSpaceMapper &wsm = AFUDev().WSM();
	WorkSpaceMapper::pcWkSp_t pWkSp;	// workspace
	WorkSpaceMapper::eWSM_Ret eRet = wsm.GetWkSp(Address, &pWkSp);

	switch(eRet) {
		case WorkSpaceMapper::FOUND_EXACT:
			return pWkSp->m_phys_ptr;			// converted address is beginning of workspace

		case WorkSpaceMapper::FOUND_INCLUDED:
		case WorkSpaceMapper::FOUND:
			return pWkSp->m_phys_ptr + (Address - pWkSp->m_ptr);	// beginning of workspace + offset

		case WorkSpaceMapper::NOT_FOUND:
		default:
			return 0;
	}
*/
   return 0;
}

//
// AllocateBufferHandler. Internal callback on completion of transaction
//                        sent from bufferAllocate().
//
void HWALIAFU::AllocateBufferHandler(IEvent const &theEvent)
{
   // FIXME: port to IAFUProxy
/*
   // The object that generated the event (AIAProxy) has our this as its context
   HWALIAFU *This = static_cast<HWALIAFU *>(theEvent.Object().Context());

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
      This->getRuntime()->schedDispatchable( new(std::nothrow) BufferAllocated(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, This->ClientBase()),
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
   This->getRuntime()->schedDispatchable( new(std::nothrow) BufferAllocateFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, This->ClientBase()),
                                                                                                                     pExcept) );
*/
}

//
// FreeBufferHandler. Internal callback on completion of transaction
//                    sent from bufferFree().
//
void HWALIAFU::FreeBufferHandler(IEvent const &theEvent)
{
   // FIXME: port to IAFUProxy
   /*
   // The object that generated the event (AIAProxy) has our this as its context
   HWALIAFU *This = static_cast<HWALIAFU *>(theEvent.Object().Context());

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
      This->getRuntime()->schedDispatchable( new(std::nothrow) BufferFreed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, This->ClientBase()),
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
   This->getRuntime()->schedDispatchable( new(std::nothrow) BufferFreeFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, This->ClientBase()),
                                                                                         pExcept) );
*/
}

// ---------------------------------------------------------------------------
// IALIUMsg interface implementation
// ---------------------------------------------------------------------------

//
// umsgGetNumber. Return number of UMSGs.
//
btUnsignedInt HWALIAFU::umsgGetNumber( void )
{
   // FIXME: port to IAFUProxy
//	return AFUDev().getUMSGsize();
   return 0;
}

//
// umsgGetAddress. Get address of specific UMSG.
//
btVirtAddr HWALIAFU::umsgGetAddress( const btUnsignedInt UMsgNumber )
{
   // FIXME: port to IAFUProxy
//   return AFUDev().getUMSG() + (UMsgNumber << 9);	// assumes 512 bit (cacheline) UMSGs
   return NULL;
}

//
// umsgSetAttributes. Set UMSG attributes.
//
// FIXME: not implemented
//
bool HWALIAFU::umsgSetAttributes( NamedValueSet const &nvsArgs)
{
	return false;
}


// ---------------------------------------------------------------------------
// IALIReset interface implementation
// ---------------------------------------------------------------------------

IALIReset::e_Reset HWALIAFU::afuQuiesceAndReset( NamedValueSet const *pOptArgs)
{
   return e_OK;
}

IALIReset::e_Reset HWALIAFU::afuReEnable( NamedValueSet const *pOptArgs)
{
   return e_OK;
}

IALIReset::e_Reset HWALIAFU::afuReset( NamedValueSet const *pOptArgs)
{
   return e_OK;
}



/*
 * IServiceClient methods (callbacks from AIA service)
 */

// Service allocated callback
void HWALIAFU::serviceAllocated(IBase               *pServiceBase,
                                TransactionID const &rTranID)
{
   // Store ResMgrService pointer
   m_pAFUProxy = dynamic_ptr<IAFUProxy>(iidAFUProxy, pServiceBase);
   if (!m_pAFUProxy) {
      // TODO: handle error
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
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Error: Missing service base interface."));
      return;
   }

   // AFUProxy acquired, init complete for original (saved) transaction/
   initComplete(m_tidSaved);
   return;
}

// Service allocated failed callback
void HWALIAFU::serviceAllocateFailed(const IEvent &rEvent) {
   m_bIsOK = false;  // FIXME: reusing ServiceBase's m_bIsOK - is that okay?
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errAllocationFailure,
                                              reasUnknown,
                                              "Error: Failed to allocate ALI."));
}

// Service released callback
void HWALIAFU::serviceReleased(TransactionID const &rTranID) {
   // EMPTY
}

// Service released failed callback
void HWALIAFU::serviceReleaseFailed(const IEvent &rEvent) {
   m_bIsOK = false;  // FIXME: reusing ServiceBase's m_bIsOK - is that okay?
   // EMPTY
}

// Callback for generic events
void HWALIAFU::serviceEvent(const IEvent &rEvent) {
   // TODO: handle unexpected events
   ASSERT(false);
}


/// @} group HWALIAFU

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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::HWALIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

HWALIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
HWALIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

