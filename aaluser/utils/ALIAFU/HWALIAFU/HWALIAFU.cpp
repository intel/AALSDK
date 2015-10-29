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
	   // TODO: IALIBuffer_Client::bufferAllocated doen't take a physical address anymore
	   //       Remove from dispatchable
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
               TransactionID        TranID) :
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
// init.
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
   // Wrap original transaction id and timeout
   ReleaseContext *prc = new ReleaseContext(TranID, timeout);
   btApplicationContext appContext = reinterpret_cast<btApplicationContext>(prc);
   // Release ALI / AFUProxy
   ASSERT(m_pAALService != NULL);
   return m_pAALService->Release(TransactionID(appContext), timeout);
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

   // Get MMIO buffer (UMSG buffer is handled in mmioAllocEventHandler callback)

   // Set transaction IDs so that AFUEvent() can distinguish between MMIO and
   // UMSG events (both are uid_wseventCSRMap events)
   TransactionID mmioTid(GetMMIO);
   TransactionID umsgTid(GetUMSG);

   // Create the Transactions
   GetMMIOBufferTransaction mmioTransaction(mmioTid);
   GetUMSGBufferTransaction umsgTransaction(umsgTid);

   // Check the parameters
   if ( mmioTransaction.IsOK() && umsgTransaction.IsOK()) {
      // Will return to AFUEvent(), below.
      m_pAFUProxy->SendTransaction(&mmioTransaction);
      m_pAFUProxy->SendTransaction(&umsgTransaction);
   } else {
      initFailed(new CExceptionTransactionEvent(NULL,
                                                m_tidSaved,
                                                errAFUWorkSpace,
                                                reasAFUNoMemory,
                                                "GetMMIOBuffer/GetUMSGBuffer transaction validity check failed"));
      return;
   }
}

// Service allocated failed callback
void HWALIAFU::serviceAllocateFailed(const IEvent &rEvent) {
   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errAllocationFailure,
                                              reasUnknown,
                                              "Error: Failed to allocate ALI."));
}

// Service released callback
void HWALIAFU::serviceReleased(TransactionID const &rTranID) {
   ReleaseContext *prc = reinterpret_cast<ReleaseContext *>(rTranID.Context());
   ServiceBase::Release(prc->TranID, prc->timeout);
}

// Service released failed callback
void HWALIAFU::serviceReleaseFailed(const IEvent &rEvent) {
   m_bIsOK = false;
   // TODO EMPTY
}

// Callback for generic events
void HWALIAFU::serviceEvent(const IEvent &rEvent) {
   // TODO: handle unexpected events
   ASSERT(false);
}

// ---------------------------------------------------------------------------
// IALIMMIO interface implementation
// ---------------------------------------------------------------------------

//
// mmioGetAddress. Return address of MMIO space.
//
btVirtAddr HWALIAFU::mmioGetAddress( void )
{
   return m_MMIORmap;
}

//
// mmioGetLength. Return length of MMIO space.
//
btCSROffset HWALIAFU::mmioGetLength( void )
{
   return m_MMIORsize;
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

	*pValue = *( (btUnsigned32bitInt*)(pMMIOBase + Offset) );      // FIXME: might want to use C++ style casts

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

	   // TODO: Create a transaction id that wraps the original from the application,
//	   TransactionID tid(new(std::nothrow) TransactionID(TranID));

	   // Create the Transaction
	   BufferAllocateTransaction transaction(TranID, Length);

	   // Check the parameters
	   if ( transaction.IsOK() ) {
	      // Will return to AFUEvent, below.
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
   AutoLock(this);
   // TODO: Create a transaction id that wraps the original from the application,
//    TransactionID tid(new(std::nothrow) TransactionID(TranID));

   // Find workspace id
   mapWkSpc_t::iterator i = m_mapWkSpc.find(Address);
   if (i == m_mapWkSpc.end()) {  // not found
      AAL_ERR(LM_All, "Tried to free non-existent Buffer");
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasInvalidParameter,
                                                                     "Tried to free non-existent Buffer");
      getRuntime()->schedDispatchable(
         new(std::nothrow) BufferFreeFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, this),
                                                        pExcept)
             );
      return;
   }
   // workspace id is in i->second.wsid

   // Create the Transaction
   BufferFreeTransaction transaction(TranID, i->second.wsid);

   // Check the parameters
   if ( transaction.IsOK() ) {

      // Unmap buffer
      m_pAFUProxy->UnMapWSID(i->second.ptr, i->second.size);

      // Send transaction
      // Will eventually trigger AFUEvent(), below.
      m_pAFUProxy->SendTransaction(&transaction);

      // Forget workspace parameters
      m_mapWkSpc.erase(i);

   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errMemory,
                                                                     reasUnknown,
                                                                     "AFUTran validity check failed");
      getRuntime()->schedDispatchable(
         new(std::nothrow) BufferFreeFailed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client, this),
                                                        pExcept)
             );
   }
}

//
// bufferGetIOVA. Retrieve IO Virtual Address for a virtual address.
//
btPhysAddr HWALIAFU::bufferGetIOVA( btVirtAddr Address)
{
   // TODO Return actual IOVA instead of physptr

   // try to find exact match
   mapWkSpc_t::iterator i = m_mapWkSpc.find(Address);
   if (i != m_mapWkSpc.end()) {
      return i->second.physptr;
   }

   // look through all workspaces to see if Address is in one of them
   // TODO: there might be a more efficient way
   for (mapWkSpc_t::iterator i = m_mapWkSpc.begin(); i != m_mapWkSpc.end(); i++ ) {
      if (Address < i->second.ptr + i->second.size) {
         return i->second.physptr + (Address - i->second.ptr);
      }
   }

   // not found
   return 0;
}

// ---------------------------------------------------------------------------
// IALIUMsg interface implementation
// ---------------------------------------------------------------------------

//
// umsgGetNumber. Return number of UMSGs.
//
btUnsignedInt HWALIAFU::umsgGetNumber( void )
{
   return m_uMSGsize;
}

//
// umsgGetAddress. Get address of specific UMSG.
//
btVirtAddr HWALIAFU::umsgGetAddress( const btUnsignedInt UMsgNumber )
{
   return m_uMSGmap + (UMsgNumber << 9);	// assumes 512 bit (cacheline) UMSGs FIXME incorrect conversion
}

//
// umsgSetAttributes. Set UMSG attributes.
//
// TODO: not implemented
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

// ---------------------------------------------------------------------------
// IAFUProxyClient interface implementation
// ---------------------------------------------------------------------------

// Callback for ALIAFUProxy
void HWALIAFU::AFUEvent(AAL::IEvent const &theEvent)
{
   IUIDriverEvent *puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                           theEvent);
   ASSERT(NULL != puidEvent);

//   std::cerr << "Got AFU event type " << puidEvent->MessageID() << "\n" << std::endl;

   switch(puidEvent->MessageID())
   {
   //===========================
   // WSM response
   // ==========================
   case rspid_WSM_Response:
      {
         // TODO check result code

         // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
         struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(puidEvent->Payload());

         switch(pResult->evtID)
         {
         //------------------------
         // Workspace allocate
         //------------------------
         case uid_wseventAllocate:
            {

            // mmap
            if (!m_pAFUProxy->MapWSID(pResult->wsParms.size, pResult->wsParms.wsid, &pResult->wsParms.ptr)) {
               AAL_ERR( LM_All, "FATAL: MapWSID failed");
            }

            // Remember workspace parameters associated with virtual ptr
            if (m_mapWkSpc.find(pResult->wsParms.ptr) != m_mapWkSpc.end()) {
               AAL_ERR( LM_All, "FATAL: WSID already exists in m_mapWSID");
            } else {
               // store entire aalui_WSParms struct in map
               m_mapWkSpc[pResult->wsParms.ptr] = pResult->wsParms;
            }

            getRuntime()->schedDispatchable(
                        new(std::nothrow) BufferAllocated(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client,
                                                                                         m_pSvcClient),
                                                          puidEvent->msgTranID(),
                                                          pResult->wsParms.ptr,
                                                          pResult->wsParms.physptr,
                                                          pResult->wsParms.size)
                            );
            } break;
         //------------------------
         // Workspace free
         //------------------------
         case uid_wseventFree:
            {
               // TODO: Forget workspace parameters here, not in bufferFree().
               getRuntime()->schedDispatchable(
                            new(std::nothrow) BufferFreed(dynamic_ptr<IALIBuffer_Client>(iidALI_BUFF_Service_Client,
                                                                                             m_pSvcClient),
                                                              puidEvent->msgTranID())
                                );

            } break;
         //-----------------------
         // Initial GetMMIO/GetUMSG transactions
         //-----------------------
         case uid_wseventCSRMap:
            {
               if (puidEvent->msgTranID().m_intID == GetMMIO) {

                  //
                  // GetMMIO
                  //

                  // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
                  struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(puidEvent->Payload());

                  // mmap
                  if (!m_pAFUProxy->MapWSID(pResult->wsParms.size, pResult->wsParms.wsid, &pResult->wsParms.ptr)) {
                     AAL_ERR( LM_All, "FATAL: MapWSID failed");
                  }

                  // Remember workspace parameters associated with virtual ptr (if we ever need it)
                  if (m_mapWkSpc.find(pResult->wsParms.ptr) != m_mapWkSpc.end()) {
                     AAL_ERR( LM_All, "FATAL: WSID already exists in m_mapWSID");
                  } else {
                     // store entire aalui_WSParms struct in map
                     m_mapWkSpc[pResult->wsParms.ptr] = pResult->wsParms;
                  }

                  m_MMIORmap = pResult->wsParms.ptr;
                  m_MMIORsize = pResult->wsParms.size;

               } else if (puidEvent->msgTranID().m_intID == GetUMSG) {

                  //
                  // GetUMSG
                  //

                  // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
                  struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(puidEvent->Payload());

                  // TODO: handle unmap event

                  // mmap
                  if (!m_pAFUProxy->MapWSID(pResult->wsParms.size, pResult->wsParms.wsid, &pResult->wsParms.ptr)) {
                     AAL_ERR( LM_All, "FATAL: MapWSID failed");
                  }

                  // Remember workspace parameters associated with virtual ptr
                  if (m_mapWkSpc.find(pResult->wsParms.ptr) != m_mapWkSpc.end()) {
                     AAL_ERR( LM_All, "FATAL: WSID already exists in m_mapWSID");
                  } else {
                     // store entire aalui_WSParms struct in map
                     m_mapWkSpc[pResult->wsParms.ptr] = pResult->wsParms;
                  }

                  m_uMSGmap = pResult->wsParms.ptr;
                  m_uMSGsize = pResult->wsParms.size;

               } else {
                  ASSERT(false); // unexpected transaction ID
               }

               // init is complete when MMIOmap and uMSGmap are both set
               if (m_MMIORmap != NULL && m_uMSGmap != NULL) {
                  initComplete(m_tidSaved);
               }
            }

         default:
            ASSERT(false); // unexpected WSM_Response evtID
         } // switch evtID

      } break;
   default:
      ASSERT(false); // unexpected event
   }
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

