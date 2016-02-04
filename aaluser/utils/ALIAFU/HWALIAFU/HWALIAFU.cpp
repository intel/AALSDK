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
//           Enno Luebbers, Intel Corporation
///          
///
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
#include "aalsdk/aas/Dispatchables.h"

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
class AFUDeactivated : public IDispatchable
{
public:
   AFUDeactivated(  IALIReconfigure_Client   *pSvcClient,
                    TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->deactivateSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};

class AFUDeactivateFailed : public IDispatchable
{
public:
   AFUDeactivateFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->deactivateFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
};

class AFUActivated : public IDispatchable
{
public:
   AFUActivated(  IALIReconfigure_Client   *pSvcClient,
                    TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->activateSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};

class AFUActivateFailed : public IDispatchable
{
public:
   AFUActivateFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->activateFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
};

class AFUReconfigured : public IDispatchable
{
public:
   AFUReconfigured(  IALIReconfigure_Client   *pSvcClient,
                     TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->configureSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};


class AFUReconfigureFailed : public IDispatchable
{
public:
   AFUReconfigureFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->configureFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
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
btBool HWALIAFU::init(IBase               *pclientBase,
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


//
//  Configure the Service for the specific AFU we received
btBool HWALIAFU::configureForAFU()
{
   // Get the AFU ID from the optargs
   INamedValueSet const *pConfigRecord;
   if(!OptArgs().Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)){
      AAL_ERR( LM_All, "No Config Record");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                   m_tidSaved,
                                                   errAllocationFailure,
                                                   reasInvalidParameter,
                                                   "Error: AFU Configuration information invalid. No Config Record."));
      return false;
   }

   OptArgs().Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &pConfigRecord );

   // Service Library to use
   btcString pAFUID;
   if(!pConfigRecord->Has(keyRegAFU_ID)){
      AAL_ERR( LM_All, "No AFU ID in Config Record");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errAllocationFailure,
                                                 reasInvalidParameter,
                                                 "Error: AFU Configuration information invalid. No AFU ID in Config Record."));
      return false;

   }
   pConfigRecord->Get(keyRegAFU_ID, &pAFUID);


   // TODO USE BIND PARMS WHEN AVAILABLE AS WELL AS AFU ID
   if( 0 == strcmp(pAFUID, ALI_AFUID_UAFU_CONFIG)){
      // Configure as PR

      m_pReconClient = dynamic_ptr<IALIReconfigure_Client>(iidALI_CONF_Service_Client, m_pSvcClient);
      ASSERT( NULL != m_pReconClient ); //QUEUE object failed
      if(NULL == m_pReconClient){
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errBadParameter,
                                                    reasMissingInterface,
                                                    "Client did not publish IALIReconfigure_Client Interface"));
         return true;
      }



      if ( EObjOK != SetInterface(iidALI_CONF_Service, dynamic_cast<IALIReconfigure *>(this)) ){
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errCreationFailure,
                                                    reasUnknown,
                                                    "Error: Could not register interface."));
         return false;
      }
      return true;
   }else{
      // TODO KEEP GOING

      // Create the Transactions
      GetMMIOBufferTransaction mmioTransaction;
   /*
      // Set transaction IDs so that AFUEvent() can distinguish between MMIO and
      // UMSG events (both are ali_wseventCSRMap events)
      TransactionID umsgTid(GetUMSG);
      GetUMSGBufferTransaction umsgTransaction(umsgTid);
   */
      // Check the parameters
      if ( mmioTransaction.IsOK()/* && umsgTransaction.IsOK()*/) {
         // Will return to AFUEvent(), below.
         m_pAFUProxy->SendTransaction(&mmioTransaction);
         if(uid_errnumOK == mmioTransaction.getErrno() ){
            struct AAL::aalui_WSMEvent wsevt = mmioTransaction.getWSIDEvent();

            // mmap
            if (!m_pAFUProxy->MapWSID(wsevt.wsParms.size, wsevt.wsParms.wsid, &wsevt.wsParms.ptr)) {
               AAL_ERR( LM_All, "FATAL: MapWSID failed");
            }

            // Remember workspace parameters associated with virtual ptr (if we ever need it)
            if (m_mapWkSpc.find(wsevt.wsParms.ptr) != m_mapWkSpc.end()) {
               AAL_ERR( LM_All, "FATAL: WSID already exists in m_mapWSID");
            } else {
               // store entire aalui_WSParms struct in map
               m_mapWkSpc[wsevt.wsParms.ptr] = wsevt.wsParms;
            }

            m_MMIORmap = wsevt.wsParms.ptr;
            m_MMIORsize = wsevt.wsParms.size;
            return true;
         }else {
            initFailed(new CExceptionTransactionEvent(NULL,
                                                      m_tidSaved,
                                                      errAFUWorkSpace,
                                                      mmioTransaction.getErrno(),
                                                      "GetMMIOBuffer/GetUMSGBuffer transaction validity check failed"));
            return false;
         }
         //      m_pAFUProxy->SendTransaction(&umsgTransaction);
      } else {
         initFailed(new CExceptionTransactionEvent(NULL,
                                                   m_tidSaved,
                                                   errAFUWorkSpace,
                                                   reasAFUNoMemory,
                                                   "GetMMIOBuffer/GetUMSGBuffer transaction validity check failed"));
         return false;
      }
   }

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

   // Configure the Service for the specific AFU capabilities we have
   //   Proper event will be generated if failure.
   if(false == configureForAFU()){
      return;
   }

   initComplete(m_tidSaved);

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
btBool HWALIAFU::mmioRead32(const btCSROffset Offset, btUnsigned32bitInt * const pValue)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   *pValue = *( reinterpret_cast<btUnsigned32bitInt *>(m_MMIORmap + Offset) );

   return true;
}

//
// mmioWrite32. Write 32bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioWrite32(const btCSROffset Offset, const btUnsigned32bitInt Value)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   *( reinterpret_cast<btUnsigned32bitInt *>(m_MMIORmap + Offset) ) = Value;

   return true;
}

//
// mmioRead64. Read 64bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioRead64(const btCSROffset Offset, btUnsigned64bitInt * const pValue)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   *pValue = *( reinterpret_cast<btUnsigned64bitInt *>(m_MMIORmap + Offset) );

   return true;
}

//
// mmioWrite64. Write 64bit CSR. Offset given in bytes.
//
btBool HWALIAFU::mmioWrite64(const btCSROffset Offset, const btUnsigned64bitInt Value)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   *( reinterpret_cast<btUnsigned64bitInt *>(m_MMIORmap + Offset) ) = Value;

   return true;
}

// ---------------------------------------------------------------------------
// IALIBuffer interface implementation
// ---------------------------------------------------------------------------

//
// bufferAllocate. Allocate a shared buffer (formerly known as workspace).
//
AAL::ali_errnum_e HWALIAFU::bufferAllocate( btWSSize             Length,
                                                    btVirtAddr          *pBufferptr,
                                                    NamedValueSet const &rInputArgs,
                                                    NamedValueSet       &rOutputArgs )
{
   AutoLock(this);
   *pBufferptr = NULL;

   // Create the Transaction
   BufferAllocateTransaction transaction(Length);

   // Check the parameters
   if ( transaction.IsOK() ) {
      // Will return to AFUEvent, below.
      m_pAFUProxy->SendTransaction(&transaction);
   } else{
      return ali_errnumSystem;
   }

   if(uid_errnumOK != transaction.getErrno() ){
      AAL_ERR( LM_All, "FATAL:buffer allocate error = " << transaction.getErrno());
      return ali_errnumSystem;

   }
   struct AAL::aalui_WSMEvent wsevt = transaction.getWSIDEvent();

   // mmap
   if (!m_pAFUProxy->MapWSID(wsevt.wsParms.size, wsevt.wsParms.wsid, &wsevt.wsParms.ptr)) {
      AAL_ERR( LM_All, "FATAL: MapWSID failed");
      return ali_errnumSystem;
   }
   // store entire aalui_WSParms struct in map
   m_mapWkSpc[wsevt.wsParms.ptr] = wsevt.wsParms;

   *pBufferptr = wsevt.wsParms.ptr;
   return ali_errnumOK;

}

//
// bufferFree. Release previously allocated buffer.
//
AAL::ali_errnum_e HWALIAFU::bufferFree( btVirtAddr           Address)
{
   AutoLock(this);
   // TODO: Create a transaction id that wraps the original from the application,
//    TransactionID tid(new(std::nothrow) TransactionID(TranID));

   // Find workspace id
   mapWkSpc_t::iterator i = m_mapWkSpc.find(Address);
   if (i == m_mapWkSpc.end()) {  // not found
      AAL_ERR(LM_All, "Tried to free non-existent Buffer");
      return ali_errnumBadParameter;
   }
   // workspace id is in i->second.wsid

   // Create the Transaction
   BufferFreeTransaction transaction(i->second.wsid);

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
      return ali_errnumSystem;
   }
   return ali_errnumOK;
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
   // TODO: this loop works only if map keeps keys in increasing order -- does it?
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
   UmsgGetNumber transaction;
   m_pAFUProxy->SendTransaction(&transaction);
   btUnsignedInt temp = transaction.getNumber();
   return temp;
}

//
// umsgGetAddress. Get address of specific UMSG.
//
btVirtAddr HWALIAFU::umsgGetAddress( const btUnsignedInt UMsgNumber )
{
   // If we've never gotten the map getit now
   if(NULL == m_uMSGmap){
      UmsgGetBaseAddress transaction;

      m_pAFUProxy->SendTransaction(&transaction);
      if(uid_errnumOK != transaction.getErrno()){
         return NULL;
      }
      struct AAL::aalui_WSMEvent wsevt = transaction.getWSIDEvent();

      // mmap
      if (!m_pAFUProxy->MapWSID(wsevt.wsParms.size, wsevt.wsParms.wsid, &wsevt.wsParms.ptr)) {
         AAL_ERR( LM_All,"FATAL: MapWSID failed");
         return NULL;
      }
      else
      {
     	  m_uMSGsize = wsevt.wsParms.size;
		  m_uMSGmap = wsevt.wsParms.ptr;
      }
   }
   // Umsgs are separated by 1 Page + 1 CL
   // Malicious call could overflow and cause wrap to invalid address.
   // TODO: Check if there is any problem with using a different address
   //       in the UMAS range
   btUnsigned32bitInt offset = UMsgNumber * (4096 + 64) ;

   if ( offset >=  m_uMSGsize) {
      return NULL;
   } else {
      // m_uMSGmap is btVirtAddr is char* so math is in bytes
      return m_uMSGmap + offset;
   }
}

void HWALIAFU::umsgTrigger64( const btVirtAddr pUMsg,
                              const btUnsigned64bitInt Value )
{
   *reinterpret_cast<btUnsigned64bitInt*>(pUMsg) = Value;
}  // umsgTrigger64

//
// umsgSetAttributes. Set UMSG attributes.
//
// TODO: not implemented
//
bool HWALIAFU::umsgSetAttributes( NamedValueSet const &nvsArgs)
{

   if( true != nvsArgs.Has(UMSG_HINT_MASK_KEY)){
      AAL_ERR( LM_All,"Missing Parameter or Key");
      return false;
   }
   eBasicTypes nvsType;
   if(ENamedValuesOK !=  nvsArgs.Type(UMSG_HINT_MASK_KEY, &nvsType)){
      AAL_ERR( LM_All,"Unable to get key value type.");
      return false;
   }

   if(btUnsigned64bitInt_t !=  nvsType){
      AAL_ERR( LM_All,"Bad value type.");
      return false;
   }

   UmsgSetAttributes transaction(nvsArgs);

   m_pAFUProxy->SendTransaction(&transaction);
   if(uid_errnumOK != transaction.getErrno()){
      return false;
   }

   return true;
}

// ---------------------------------------------------------------------------
// IALIReset interface implementation
// ---------------------------------------------------------------------------

IALIReset::e_Reset HWALIAFU::afuQuiesceAndHalt( NamedValueSet const &rInputArgs )
{
   // Create the Transaction
   AFUQuiesceAndHalt transaction;

   // Should never fail
   if ( !transaction.IsOK() ) {
      return e_Internal;
   }

   // Send transaction
   // Will eventually trigger AFUEvent(), below.
   m_pAFUProxy->SendTransaction(&transaction);

   if(transaction.getErrno() != uid_errnumOK){
      return e_Error_Quiesce_Timeout;
   }

   return e_OK;
}

IALIReset::e_Reset HWALIAFU::afuEnable( NamedValueSet const &rInputArgs)
{
   // Create the Transaction
   AFUEnable transaction;

   // Should never fail
   if ( !transaction.IsOK() ) {
      return e_Internal;
   }

   // Send transaction
   // Will eventually trigger AFUEvent(), below.
   m_pAFUProxy->SendTransaction(&transaction);

   if(transaction.getErrno() != uid_errnumOK){
      return e_Error_Quiesce_Timeout;
   }

   return e_OK;

}

IALIReset::e_Reset HWALIAFU::afuReset( NamedValueSet const &rInputArgs )
{
   IALIReset::e_Reset ret = afuQuiesceAndHalt();

   if(ret != e_OK){
      afuEnable();
   }else{
      ret = afuEnable();
   }

   return ret;
}

btBool HWALIAFU::performanceCountersGet ( INamedValueSet * const  pResult,
                                          NamedValueSet    const &pOptArgs )
{

   btWSSize size;
   size = sizeof(struct  CCIP_PERF_COUNTERS);
   struct  CCIP_PERF_COUNTERS *pPref =NULL;

   if( NULL == pResult)
      return  false;

   // Create the Transaction
   PerfCounterGet transaction(size);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   // Will eventually trigger AFUEvent(), below.
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK){
         return false;
      }

   if(NULL == transaction.getBuffer() )  {
     return false;
   }

   pPref = (struct  CCIP_PERF_COUNTERS *)transaction.getBuffer();

   pResult->Add(pPref->version.name,pPref->version.value);
   pResult->Add(pPref->num_counters.name,pPref->num_counters.value);

   pResult->Add(pPref->read_hit.name,pPref->read_hit.value);
   pResult->Add(pPref->write_hit.name,pPref->write_hit.value);
   pResult->Add(pPref->read_miss.name,pPref->read_miss.value);
   pResult->Add(pPref->write_miss.name,pPref->write_miss.value);
   pResult->Add(pPref->evictions.name,pPref->evictions.value);

   pResult->Add(pPref->pcie0_read.name,pPref->pcie0_read.value);
   pResult->Add(pPref->pcie0_write.name,pPref->pcie0_write.value);
   pResult->Add(pPref->pcie1_read.name,pPref->pcie1_read.value);
   pResult->Add(pPref->pcie1_write.name,pPref->pcie1_write.value);
   pResult->Add(pPref->upi_read.name,pPref->upi_read.value);
   pResult->Add(pPref->upi_write.name,pPref->upi_write.value);


   return true;
}


// ---------------------------------------------------------------------------
// IALIReconfigure interface implementation
// ---------------------------------------------------------------------------

/// @brief Deactivate an AFU in preparation for it being reconfigured.
///
/// Basically, if there is an AFU currently instantiated and connected to an
///    application, then this will send an exception to the application indicating
///    that it should release the AFU. There can be a timeout option that specifies
///    that if the application does not Release within a particular time, then
///    the AFU will be yanked. Then, a CleanIt!(tm) AFU will be loaded to clear
///    out all the gates and clear the FPGA memory banks.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments if needed. Defaults to NULL.
/// @return     void. Callback in IALIReconfigureClient.
///
void HWALIAFU::reconfDeactivate( TransactionID const &rTranID,
                                 NamedValueSet const &rInputArgs)
{
   AFUDeactivateTransaction deactivatetrans(rTranID,rInputArgs);
   // Send transaction
   m_pAFUProxy->SendTransaction(&deactivatetrans);
   if(deactivatetrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Deactivate failed");
      return;
   }

}

/// @brief Configure an AFU.
///
/// Download the defined bitstream to the PR region. Initially, the bitstream
///    is a file name. Later, it might be a goal record, and that is why the
///    parameter is an NVS. It is also possible in the NVS to specify a PR number
///    if that is relevant, e.g. for the PF driver.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments. Initially need a bitstream.
/// @return     void. Callback in IALIReconfigureClient.
///
void HWALIAFU::reconfConfigure( TransactionID const &rTranID,
                                NamedValueSet const &rInputArgs)
{
   btByte        *bufptr      = NULL;
   std::streampos    filesize    = 0;

   if(rInputArgs.Has(AALCONF_FILENAMEKEY)){
      btcString filename;
      rInputArgs.Get(AALCONF_FILENAMEKEY, &filename);
      std::ifstream bitfile(filename, std::ios::binary );

      bitfile.seekg( 0, std::ios::end );
      filesize = bitfile.tellg();
      bitfile.seekg( 0, std::ios::beg );
      bufptr = reinterpret_cast<btByte*>(malloc(filesize));
      bitfile.read(reinterpret_cast<char *>(bufptr), filesize);
   }else{

      getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                               rTranID,
                                                                                                               errBadParameter,
                                                                                                               reasMissingParameter,
                                                                                                               "Error: No bitfile source.")));
      return;

#if 0 // Test code
      bufptr = reinterpret_cast<btByte*>(malloc(100));
      *bufptr = '<';
      memset(bufptr+1,'*',97);
      *(bufptr+98) = '>';
      *(bufptr+99) = 0;
      filesize = 100;
#endif
   }

   AFUConfigureTransaction configuretrans(reinterpret_cast<btVirtAddr>(bufptr), filesize, rTranID,rInputArgs);
   // Send transaction
   m_pAFUProxy->SendTransaction(&configuretrans);
   if(configuretrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Reconfigure failed");
      getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                               rTranID,
                                                                                                               errCauseUnknown,
                                                                                                               reasUnknown,
                                                                                                               "Error: Failed transaction")));

      return;
   }
   free(bufptr);
}

/// @brief Activate an AFU after it has been reconfigured.
///
/// Once the AFU has been reconfigured there needs to be a "probe" to load
///    the AFU configuration information, e.g. AFU_ID, so that the associated
///    service can be loaded and the whole shebang returned to the application.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments if needed. Defaults to NULL.
/// @return     void. Callback in IALIReconfigureClient.
///
void HWALIAFU::reconfActivate( TransactionID const &rTranID,
                               NamedValueSet const &rInputArgs)
{
   AFUActivateTransaction activatetrans(rTranID);
   // Send transaction
   m_pAFUProxy->SendTransaction(&activatetrans);
   if(activatetrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Activate failed");
      return;
   }


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

   std::cerr << "Got AFU event type " << puidEvent->MessageID() << "\n" << std::endl;

   switch(puidEvent->MessageID())
   {
   //===========================
   // WSM response
   // ==========================
   case rspid_AFU_PR_Honar_Owner_Event:

   {
      std::cout << "HWALIAFU::AFUEvent rspid_AFU_PR_Honar_Owner_Event \n" << std::endl;
      std::cout << "puidEvent->ResultCode()\n" << puidEvent->ResultCode()<< std::endl;


      getRuntime()->schedDispatchable(new ServiceEvent(getServiceClient(),new CExceptionTransactionEvent(NULL,
                puidEvent->msgTranID(),
                puidEvent->ResultCode(),
                reasUnknown,
                "Release AFU rspid_AFU_PR_Honar_Owner_Event ")));
   }

   break;

   case rspid_AFU_PR_Revoked_Event:

   {
      std::cout << "HWALIAFU::AFUEvent rspid_AFU_PR_Revoked_Event \n" << std::endl;
      std::cout << "puidEvent->ResultCode()\n" << puidEvent->ResultCode()<< std::endl;

      getRuntime()->schedDispatchable( new ServiceRevoke(dynamic_ptr<IServiceRevoke>(iidServiceRevoke,this)) );

   }
   break;

   case rspid_AFU_PR_Honor_Request_Event:

   {
      std::cout << "HWALIAFU::AFUEvent rspid_AFU_PR_Honor_Request_Event \n" << std::endl;
      std::cout << "puidEvent->ResultCode()\n" << puidEvent->ResultCode()<< std::endl;


      getRuntime()->schedDispatchable(new ServiceEvent(getServiceClient(),new CExceptionTransactionEvent(NULL,
                    puidEvent->msgTranID(),
                    puidEvent->ResultCode(),
                    reasUnknown,
                    "Release AFU rspid_AFU_PR_Honor_Request_Event ")));

   }
   break ;
   case rspid_WSM_Response:
      {
         // TODO check result code

         // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
         struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(puidEvent->Payload());

         switch(pResult->evtID)
         {

         default:
            ASSERT(false); // unexpected WSM_Response evtID
         } // switch evtID
      } break;
   case rspid_AFU_Response:
   {
      struct aalui_AFUResponse* presp = reinterpret_cast<struct aalui_AFUResponse *>(puidEvent->Payload());
      switch(presp->respID)
      {
      case uid_afurespDeactivateComplete:
         {
            if( uid_errnumOK != puidEvent->ResultCode()){
               getRuntime()->schedDispatchable(new AFUDeactivateFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                     puidEvent->msgTranID(),
                                                                                                                     puidEvent->ResultCode(),
                                                                                                                     reasUnknown,
                                                                                                                     "Error: Deactivate failed. Check Exception number against uid_errnum_e codes")));
            }else{
               getRuntime()->schedDispatchable(new AFUDeactivated(m_pReconClient, TransactionID(puidEvent->msgTranID())));
            }
            return;
         }
      case uid_afurespActivateComplete:
         {
            if( uid_errnumOK != puidEvent->ResultCode()){
                getRuntime()->schedDispatchable(new AFUActivateFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                      puidEvent->msgTranID(),
                                                                                                                      puidEvent->ResultCode(),
                                                                                                                      reasUnknown,
                                                                                                                      "Error: Activate failed. Check Exception number against uid_errnum_e codes")));
             }else{
                getRuntime()->schedDispatchable(new AFUActivated(m_pReconClient, TransactionID(puidEvent->msgTranID())));
             }
            return;
         }
      case uid_afurespConfigureComplete:
         {
            if( uid_errnumOK !=puidEvent->ResultCode()){
                getRuntime()->schedDispatchable(new AFUReconfigureFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                      puidEvent->msgTranID(),
                                                                                                                      puidEvent->ResultCode(),
                                                                                                                      reasUnknown,
                                                                                                                      "Error: Configure failed. Check Exception number against uid_errnum_e codes")));
             }else{
                getRuntime()->schedDispatchable(new AFUReconfigured(m_pReconClient, TransactionID(puidEvent->msgTranID())));
             }
            return;
         }

      default:
         break;
      }
   }
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

