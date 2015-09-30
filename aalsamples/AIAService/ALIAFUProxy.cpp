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
/// @file AFUProxy.cpp
/// @brief Implementation of the ALI AFUProxy Class. The AFUProxy class
///          is an abstraction of the AFU.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// PURPOSE: 
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/15/2015     JG       Initial version@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aalsdk/AALLoggerExtern.h"

#include "ALIAFUProxy.h"
#include "AIA-internal.h"

#include "AIATransactions.h"

//#include "aalsdk/INTCDefs.h"




//#include "aalsdk/uaia/IAFUProxy.h"
//#include "aalsdk/faptrans/FAP10.h"
//#include "aalsdk/kernel/ahmpipdefs.h"

USING_NAMESPACE(AAL);

//=============================================================================
// Name: ALIAFUProxy
// Description: The AFUDev object is the proxy to the physical AFU engine
//              implementation on a device. It abstracts a session between the
//              Host AFU and the device through the AIA.
//
//              The ALIAFUProxy sends messages to the device through
//              ITransaction objects.
// Comments:
//=============================================================================

// Destructor
ALIAFUProxy::~ALIAFUProxy() {}

//=============================================================================
// Name: init
// Description: Initialize the Proxy
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
btBool ALIAFUProxy::init( IBase *pclientBase,
                          NamedValueSet const &optArgs,
                          TransactionID const &rtid)
{
   // Make sure the AIA passed in its interface
   ASSERT( true == optArgs.Has(AIA_SERVICE_BASE_INTERFACE) );
   if(true != optArgs.Has(AIA_SERVICE_BASE_INTERFACE)){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Did not get AIA interface for proxy"));
      return true;
   }


   // Get the AIA pointer
   if( ENamedValuesOK != optArgs.Get(AIA_SERVICE_BASE_INTERFACE, static_cast<btAny*>(static_cast<btAny>(&m_pAIABase)) )){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "FATAL Error: Manifest said AIA interface present but could not be acquired!"));
      return true;
   }
   m_pAIA = dynamic_ptr<AIAService>(iidAIAService, m_pAIABase);

   //
   // Check Client for proper interface

   // TODO


   // Bind the AFU device to complete transfer of ownership
   // Get the device handle if there is one
   if( optArgs.Has(keyRegHandle) ) {
      optArgs.Get(keyRegHandle, &m_devHandle);
   }else {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                rtid,
                                                errBadParameter,
                                                reasNoDevice,
                                                "No device handle in Configuration Record!"));
      return true;
    }

   m_pAIA->SendMessage(m_devHandle, new BindAFUDevice(rtid), dynamic_cast<IAFUProxyClient*>(this) );

   // TODO BIND DEVICE
#if 0
   m_pAIA->AFUProxyAdd(this);
   initComplete(rtid);
#endif

}

//=============================================================================
// Name: SendTransaction
// Description: Send a message to the device
// Inputs: pAFUmessage - Transaction object
//         rtid - Transaction ID
// Outputs: true - success
// Comments:
//=============================================================================
btBool ALIAFUProxy::SendTransaction(IAFUTransaction *pAFUmessage, TransactionID const &rtid)
{
#if 0
   // Wrap the message in a AFUTransaction object and have the AIA
   //  marshal it
   m_pSession->ruAIA().SendMessage( AFUTransaction( m_AIAMarshaller,
                                                    this,
                                                    m_Handle,
                                                    pAFUmessage,
                                                    rtid,
                                                    &m_returnAddress));
#endif
   return true;  /// SendMessage is a void TDO cleanup
}

//=============================================================================
// Name: AFUEvent
// Description: Callback when an upstream messsage is received
// Inputs: theEvent - AIA message event
// Outputs: true - success
// Comments:
//=============================================================================
void ALIAFUProxy::AFUEvent( AAL::IEvent const &theEvent)
{
   IUIDriverEvent * puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                            theEvent);

   ASSERT(NULL != puidEvent);

   switch(puidEvent->MessageID())
   {
      case rspid_UID_Shutdown:
      {
         std::cerr<<"Got rspid_UID_Shutdown event" << std::endl;
      }
      break;
      case rspid_UID_UnbindComplete:
      {
         std::cerr<<"Got rspid_UID_UnbindComplete event" << std::endl;
      }
      break;
      case rspid_UID_BindComplete:
      {
         initComplete(puidEvent->msgTranID());
      }
      break;
   }

}

AAL::btBool ALIAFUProxy::Release(AAL::TransactionID const &rTranID, AAL::btTime timeout)
{
   return ServiceBase::Release(rTranID, timeout);

}

#if 0
//=============================================================================
// Name: Initialize
// Description: Initialize the CAFUdev
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
void ALIAFUProxy::Initialize(struct aalui_extbindargs * extBindParmsp, TransactionID const &rtid )
{
    // Store the extended bin parameters (TODO bind parms need to be deep copied)
    m_extBindParms = *extBindParmsp;

    // If mappable API enabled for this device then map CSRs, MMIO=R and UMSG spaces
    if(m_extBindParms.m_mappableAPI){
       // Need to keep track of number of CSR map transactions to wait for.
       unsigned int *pnumEvents = NULL;

       AAL_VERBOSE(LM_AFU, "ALIAFUProxy::Initialize() : m_mappableAPI " << m_extBindParms.m_mappableAPI << std::endl);

       // Create the Transaction
       // Send both transactions in parallel
       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_CSRREAD ) {
          AAL_VERBOSE(LM_AFU, "ALIAFUProxy::Initialize() : m_mappableAPI Read" << std::endl);

          pnumEvents  = new unsigned;
          *pnumEvents = 1;

          struct ALIAFUProxy::wrapper *preadwrap = new ALIAFUProxy::wrapper(this, WSID_CSRMAP_READAREA, rtid, pnumEvents);
          TransactionID rdtid(static_cast<btApplicationContext>(preadwrap), _CSRMapHandler, true );

          Sig_MapCSRSpace_AFUTransaction MapReadTran( WSID_CSRMAP_READAREA );
          SendTransaction(&MapReadTran, rdtid);
       }

       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_CSRWRITE ) {
          AAL_VERBOSE(LM_AFU, "ALIAFUProxy::Initialize() : m_mappableAPI Write" << std::endl);

          if ( NULL == pnumEvents ) {
             pnumEvents = new unsigned;
             *pnumEvents = 0;
          }

          ++(*pnumEvents);

          struct ALIAFUProxy::wrapper *pwriterwrap = new ALIAFUProxy::wrapper(this,WSID_CSRMAP_WRITEAREA,rtid, pnumEvents);
          TransactionID wrtid(static_cast<btApplicationContext>(pwriterwrap), _CSRMapHandler, true );

          Sig_MapCSRSpace_AFUTransaction MapWriteTran( WSID_CSRMAP_WRITEAREA );
          SendTransaction(&MapWriteTran, wrtid);
       }


       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_MMIOR ) {
          AAL_VERBOSE(LM_AFU, "ALIAFUProxy::Initialize() : m_mappableAPI MMIO-R" << std::endl);

          if ( NULL == pnumEvents ) {
             pnumEvents = new unsigned;
             *pnumEvents = 0;
          }

          ++(*pnumEvents);

          struct ALIAFUProxy::wrapper *pwriterwrap = new ALIAFUProxy::wrapper(this,WSID_MAP_MMIOR,rtid, pnumEvents);
          TransactionID wrtid(static_cast<btApplicationContext>(pwriterwrap), _CSRMapHandler, true );

          Sig_MapMMIO_Space_AFUTransaction MapMMIORTran;
          SendTransaction(&MapMMIORTran, wrtid);
       }

       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_UMSG ) {
          AAL_VERBOSE(LM_AFU, "ALIAFUProxy::Initialize() : m_mappableAPI UMSG" << std::endl);

          if ( NULL == pnumEvents ) {
             pnumEvents = new unsigned;
             *pnumEvents = 0;
          }

          ++(*pnumEvents);

          struct ALIAFUProxy::wrapper *pwriterwrap = new ALIAFUProxy::wrapper(this,WSID_MAP_UMSG,rtid, pnumEvents);
          TransactionID wrtid(static_cast<btApplicationContext>(pwriterwrap), _CSRMapHandler, true );

          Sig_MapUMSGpace_AFUTransaction MapUMSGTran;
          SendTransaction(&MapUMSGTran, wrtid);
       }

    }else{

       // and generate the event
       m_pSession->QueueAASEvent( NULL,
                                  new CTransactionEvent( static_cast<IBase*>(this),
                                                        tranevtInitAFUDevEvent,
                                                        rtid));

    }

}

//=============================================================================
// Name: _CSRMapHandler
// Description: Static Event Handler for CSR Map transactions
// Inputs: none
// Outputs: none
// Comments: calls ALIAFUProxy method
//=============================================================================
void ALIAFUProxy::_CSRMapHandler(IEvent const &theEvent)
{
   // Check for exception
   if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
      PrintExceptionDescription(theEvent);
      return;
   }

   // Get the wrapper from the context of the event's TransactionID
   struct ALIAFUProxy::wrapper *pwrapper = reinterpret_cast< struct ALIAFUProxy::wrapper *>(dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).TranID().Context());

   pwrapper->pafudev->CSRMapHandler(theEvent, pwrapper->id, pwrapper->tid, pwrapper->pnumEvents);
   delete pwrapper;

}

//=============================================================================
// Name: CSRMapHandler
// Description: Event Handler for CSR Map transactions
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
void
ALIAFUProxy::CSRMapHandler(IEvent const        &theEvent,
                       btWSID               id,
                       TransactionID const &rtid,
                       btUnsignedInt       *pnumEvents )
{
   // This Event is a IUIDriverClientEvent. Get the results from the call.
   IUIDriverClientEvent *pevt = subclass_ptr<IUIDriverClientEvent>(theEvent);
   if ( NULL == pevt ) {

      // and generate the event
      m_pSession->QueueAASEvent(m_pSession->OwnerMessageRoute().Handler(),
                                 new CExceptionTransactionEvent(static_cast <IBase*> (this),
                                                                  exttranevtInitAFUDevEvent,
                                                                  rtid,
                                                                  errInternal,
                                                                  reasUnknown,
                                                                  "Unexpected event in CSRMapHandler"));
      return;
   }

   // Make sure the Map event was OK
   if ( uid_errnumOK != pevt->ResultCode() ) {
      std::stringstream s;
      s << "Map failed with error code ";
      s << pevt->ResultCode();
      std::string errstr = s.str();
      // and generate the event
      m_pSession->QueueAASEvent(m_pSession->OwnerMessageRoute().Handler(),
                                 new CExceptionTransactionEvent(static_cast <IBase*> (this),
                                                         exttranevtInitAFUDevEvent,
                                                         rtid,
                                                         errInternal,
                                                         reasUnknown,
                                                         errstr.c_str()));
      return;

   }

   // Payload is a aalui_WSMEvent.
   struct aalui_WSMEvent *pResult =
            reinterpret_cast<struct aalui_WSMEvent *>(pevt->Payload());

   if ( WSID_CSRMAP_WRITEAREA == id ) {

      // Set the correct mutator

      // QPI/PCIe/CCI
      if ((4 == pResult->wsParms.itemsize) && (4 == pResult->wsParms.itemspacing)) {
         _atomicSetCSR = &ALIAFUProxy::atomicSetCSR_32x4B;
         m_csrwritemap = pResult->wsParms.ptr;
      }
      // FSB
      else if ((8 == pResult->wsParms.itemsize) && (128 == pResult->wsParms.itemspacing)) {
         _atomicSetCSR = &ALIAFUProxy::atomicSetCSR_64x128B;
         m_csrwritemap = pResult->wsParms.ptr;
      }
      else {
      // Can't do Atomic CSR (use default values)
      }

      m_csrwritesize       = (btUnsigned32bitInt)pResult->wsParms.size;
      m_csrwrite_item_size = (btUnsigned32bitInt)pResult->wsParms.itemsize;

      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::CSRMapHandler() : WSID_CSRMAP_WRITEAREA initialized" << std::endl);

   }  // if (id == WSID_CSRMAP_WRITEAREA)

   if ( WSID_CSRMAP_READAREA == id ) {

      // CCI
      if ((4 == pResult->wsParms.itemsize) && (4 == pResult->wsParms.itemspacing)) {
         _atomicGetCSR = &ALIAFUProxy::atomicGetCSR_32x4B;
         m_csrreadmap  = pResult->wsParms.ptr;
      }
      // FSB/QPI/PCIe
      else if ((8 == pResult->wsParms.itemsize) && (128 == pResult->wsParms.itemspacing)) {
         _atomicGetCSR = &ALIAFUProxy::atomicGetCSR_64x128B;
         m_csrreadmap  = pResult->wsParms.ptr;
      }
      else {
      // Can't do Atomic CSR (use default values)
      }

      m_csrreadsize       = (btUnsigned32bitInt)pResult->wsParms.size;
      m_csrread_item_size = (btUnsigned32bitInt)pResult->wsParms.itemsize;

      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::CSRMapHandler() : WSID_CSRMAP_READAREA initialized" << std::endl);

   }  // if (id == WSID_CSRMAP_READAREA)

   if ( WSID_MAP_MMIOR == id ) {

      // Save the pointer and size
      m_mmiormap  = pResult->wsParms.ptr;
      m_mmiorsize = (btUnsigned32bitInt)pResult->wsParms.size;

      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::CSRMapHandler() : WSID_MAP_MMIOR initialized" << std::endl);

   }  // if (id == WSID_MAP_MMIOR)

   if ( WSID_MAP_UMSG == id ) {

      // Save the pointer and size
      m_umsgmap  = pResult->wsParms.ptr;
      m_umsgsize = (btUnsigned32bitInt)pResult->wsParms.size;

      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::CSRMapHandler() : WSID_MAP_UMSG initialized" << std::endl);

   }  // if (id == WSID

   // Lock this critical region since multiple transactions are in flight simultaneously
   {
      AutoLock(this);

      // See if both have been set
      --(*pnumEvents);
      if ( 0 == *pnumEvents ) {
         // and generate the event
         m_pSession->QueueAASEvent(m_pSession->OwnerMessageRoute().Handler(),
                                                   new CTransactionEvent(static_cast <IBase*> (this),
                                                                         tranevtInitAFUDevEvent,
                                                                         rtid));
         delete pnumEvents;
      }
   }

   return;
}

//=============================================================================
// Name: Handler
// Description: Return the event handler of the owner
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
btEventHandler ALIAFUProxy::Handler()
{
   return m_returnAddress.Handler();
}
#endif

#if 0
// This provides a Direct Mapping from user space to the AFU's registers - very fast
// NOT YET IMPLEMENTED - PLACEHOLDER, currently returns NULL
IAFUCSRMap* ALIAFUProxy::GetCSRMAP() { return NULL; }

//=============================================================================
// Name: atomicSetCSR_64x128B
// Description: Sets a single CSR value atomically. This implementation
//              assumes 64bit values spaced 128 bytes apart
// Inputs: CSR - number
//         Value - Value to set
// Outputs: true - success
// Comments:  Hardwired CSR size at the moment - Should come from manifest
//=============================================================================
btBool
ALIAFUProxy::atomicSetCSR_64x128B( btUnsignedInt CSR, btUnsigned64bitInt Value )
{
   if ((m_csrwritemap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
      AAL_WARNING(LM_AFU, "ALIAFUProxy::atomicSetCSR_64x128B Abort: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR << ", Value " << Value <<
                  std::showbase << std::hex << " " << Value << std::endl);
      return false;
   }
   else {
      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::atomicSetCSR_64x128B: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<7) <<
                  ", Value " << Value <<
                  std::showbase << std::hex << " " << Value << std::endl);
      *(reinterpret_cast <btUnsigned64bitInt*> (m_csrwritemap + (CSR << 7))) = Value;
      return true;
   }
}

//=============================================================================
// Name: atomicGetCSR_64x128B
// Description: Gets a single CSR value atomically
//              assumes 64bit values spaced 128 bytes apart
// Inputs: CSR - number
//         pValue - pointer to where to return the value
// Outputs: true - success
// Comments: On failure the returned value is undefined
//=============================================================================
btBool
ALIAFUProxy::atomicGetCSR_64x128B( btUnsignedInt CSR, btUnsigned64bitInt *pValue )
{
   if ((m_csrreadmap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
      AAL_WARNING(LM_AFU, "ALIAFUProxy::atomicGetCSR_64x128B Abort: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR << std::endl);
      return false;
   }
   else {
      *pValue = *(reinterpret_cast <btUnsigned64bitInt*> (m_csrreadmap + (CSR << 7)));
      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::atomicGetCSR_64x128B: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<7) <<
                  ", Value " << *pValue <<
                  std::showbase << std::hex << " " << *pValue << std::endl);
      return true;
   }

}

//=============================================================================
// Name: atomicSetCSR_64x4B
// Description: Sets a single CSR value atomically. This implementation
//              assumes 64bit values spaced 4 bytes apart
// Inputs: CSR - number
//         Value - Value to set
// Outputs: true - success
// Comments:  Hardwired CSR size at the moment - Should come from manifest
//=============================================================================
btBool ALIAFUProxy::atomicSetCSR_32x4B(btUnsignedInt CSR, btUnsigned64bitInt Value)
{
//   if((m_csrwritemap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)){
// disabling check for AHMPIP_MAX_AFU_CSR_INDEX for now to enable CCI devices.
   if( m_csrwritemap == NULL ) {
      AAL_WARNING(LM_AFU, "ALIAFUProxy::atomicSetCSR_32x4B Abort: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR << ", Value " << Value <<
                  std::showbase << std::hex << " " << Value << std::endl);
//      cout << "Map "<<m_csrwritemap << "CSR index " <<"CSR" <<endl;
      return false;
   }
   else {
      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::atomicSetCSR_32x4B: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<2) <<
                  ", Value " << Value <<
                  std::showbase << std::hex << " " << Value << std::endl);
      *(reinterpret_cast<btUnsigned32bitInt*>(m_csrwritemap+(CSR<<2))) = (btUnsigned32bitInt)Value;
      return true;
   }
}

//=============================================================================
// Name: atomicGetCSR_32x4B
// Description: Gets a single CSR value atomically
//              assumes 32bit values spaced 4 bytes apart
// Inputs: CSR - number
//         pValue - pointer to where to return the value
// Outputs: true - success
// Comments: On failure the returned value is undefined
//=============================================================================
btBool
ALIAFUProxy::atomicGetCSR_32x4B( btUnsignedInt CSR, btUnsigned64bitInt *pValue )
{
//   if ((m_csrreadmap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
// disabling check for AHMPIP_MAX_AFU_CSR_INDEX for now to enable CCI devices.
   if ( m_csrreadmap == NULL ) {
      AAL_WARNING(LM_AFU, "ALIAFUProxy::atomicGetCSR_32x4B Abort: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR << std::endl);
      return false;
   } else {
      btUnsigned32bitInt tmp;
      tmp     = *( reinterpret_cast<btUnsigned32bitInt*>(m_csrreadmap + (CSR << 2)) );
      *pValue = (btUnsigned64bitInt)tmp;

      AAL_VERBOSE(LM_AFU, "ALIAFUProxy::atomicGetCSR_32x4B: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<2) <<
                  ", Value " << *pValue <<
                  std::showbase << std::hex << " " << *pValue << std::endl);
      return true;
   }

}

//=============================================================================
// Name: MapWSID
// Description: Maps a workspace ID into user space
// Inputs: size - size in bytes
//         wsid - workspace ID
//         pRet - return pointer
// Outputs: true - success
// Comments: On failure pRet is undefined
//=============================================================================
btBool ALIAFUProxy::MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet)
{
   return m_pSession->ruAIA().MapWSID(Size, wsid, pRet);
}

//=============================================================================
// Name: UnMapWSID
// Description: UnMaps a workspace
// Inputs: ptr - workspace
//         wsid - workspace ID
// Outputs: true - success
// Comments:
//=============================================================================
void ALIAFUProxy::UnMapWSID(btVirtAddr ptr, btWSSize Size, btWSID wsid)
{
//   void_proxy_call(m_pSession->pAIAProxy(), UnMapWSID(ptr, Size));
   return m_pSession->ruAIA().UnMapWSID(ptr, Size);
}

//=============================================================================
// Name: WSM
// Description: Accessor to workspace mapper utility object
// Inputs: ptr - workspace
//         wsid - workspace ID
// Outputs: true - success
// Comments:
//=============================================================================
WorkSpaceMapper & ALIAFUProxy::WSM() { return m_WSM; }


//=============================================================================
// Name: Destroy
// Description: Destroy this object async
// Inputs: TranID - Transaction ID
// Outputs: true - success
// Comments:
//=============================================================================
void ALIAFUProxy::Destroy(TransactionID const &TranID){
   // Free all of the allocated workspace memory
   FreeAllWS();

   m_pSession->UnBind(m_Handle, TranID);
   delete this;
}

//=============================================================================
// Name: FreeAllWS
// Description: Frees all allocated workspaces atomically
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
void ALIAFUProxy::FreeAllWS()
{

   // TODO Use the interation function and free Workspace mapper

   // Free CSR Map if mapped
   if ( NULL != m_csrwritemap ) {
      UnMapWSID(m_csrwritemap, m_csrwritesize, WSID_CSRMAP_WRITEAREA);
      m_csrwritemap = NULL;
   }

   if ( NULL != m_csrreadmap ) {
      UnMapWSID(m_csrreadmap, m_csrreadsize, WSID_CSRMAP_READAREA);
      m_csrreadmap = NULL;
   }

   if ( NULL != m_mmiormap ) {
      UnMapWSID(m_mmiormap, m_mmiorsize, WSID_MAP_MMIOR);
      m_mmiormap = NULL;
   }

   if ( NULL != m_umsgmap ) {
      UnMapWSID(m_umsgmap, m_umsgsize, WSID_MAP_UMSG);
      m_umsgmap = NULL;
   }
}

//=============================================================================
// Name: Handle
// Description: Low level handle
// Inputs: None
// Outputs: handle
// Comments:
//=============================================================================
void * ALIAFUProxy::Handle()
{
   return m_Handle;
}

#endif



