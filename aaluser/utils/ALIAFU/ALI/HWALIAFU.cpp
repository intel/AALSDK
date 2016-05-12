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
/// @brief Definitions for ALI Hardware AFU Service.
/// @ingroup ALI
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/11/2016     HM       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H


#include <aalsdk/utils/ResMgrUtilities.h>

#include "ALIAIATransactions.h"
#include "aalsdk/aas/Dispatchables.h"
#include "HWALIAFU.h"


BEGIN_NAMESPACE(AAL)

/// @addtogroup HWALIAFU
/// @{


//
// ctor,HWALIAFU constructor.
//
CHWALIAFU::CHWALIAFU( IBase *pSvcClient,
                      IServiceBase *pServiceBase,
                      TransactionID transID,
                      IAFUProxy *pAFUProxy): CHWALIBase(pSvcClient,pServiceBase,transID,pAFUProxy),
                      m_uMSGmap(NULL),
                      m_uMSGsize(0)
{

}


// ---------------------------------------------------------------------------
// IALIBuffer interface implementation
// ---------------------------------------------------------------------------

//
// bufferAllocate. Allocate a shared buffer (formerly known as workspace).
//
AAL::ali_errnum_e CHWALIAFU::bufferAllocate( btWSSize             Length,
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
   if (!m_pAFUProxy->MapWSID(wsevt.wsParms.size, wsevt.wsParms.wsid, &wsevt.wsParms.ptr, rInputArgs)) {
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
AAL::ali_errnum_e CHWALIAFU::bufferFree( btVirtAddr           Address)
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
btPhysAddr CHWALIAFU::bufferGetIOVA( btVirtAddr Address)
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
btUnsignedInt CHWALIAFU::umsgGetNumber( void )
{
   UmsgGetNumber transaction;
   m_pAFUProxy->SendTransaction(&transaction);
   btUnsignedInt temp = transaction.getNumber();
   return temp;
}

//
// umsgGetAddress. Get address of specific UMSG.
//
btVirtAddr CHWALIAFU::umsgGetAddress( const btUnsignedInt UMsgNumber )
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

void CHWALIAFU::umsgTrigger64( const btVirtAddr pUMsg,
                              const btUnsigned64bitInt Value )
{
   *reinterpret_cast<btUnsigned64bitInt*>(pUMsg) = Value;
}  // umsgTrigger64

//
// umsgSetAttributes. Set UMSG attributes.
//
// TODO: not implemented
//
bool CHWALIAFU::umsgSetAttributes( NamedValueSet const &nvsArgs)
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

IALIReset::e_Reset CHWALIAFU::afuQuiesceAndHalt( NamedValueSet const &rInputArgs )
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

IALIReset::e_Reset CHWALIAFU::afuEnable( NamedValueSet const &rInputArgs)
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

IALIReset::e_Reset CHWALIAFU::afuReset( NamedValueSet const &rInputArgs )
{
   IALIReset::e_Reset ret = afuQuiesceAndHalt();

   if(ret != e_OK){
      afuEnable();
   }else{
      ret = afuEnable();
   }

   return ret;
}

//
// AFUEvent,AFU Event Handler.
//
void CHWALIAFU::AFUEvent(AAL::IEvent const &theEvent)
{
   CHWALIBase::AFUEvent(theEvent);
}
/// @} group HWALIAFU

END_NAMESPACE(AAL)
