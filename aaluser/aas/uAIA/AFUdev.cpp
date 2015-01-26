// Copyright (c) 2007-2015, Intel Corporation
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
/// @file AFUdev.cpp
/// @brief Implementation of the FAP PIP AFU Device Class. The AFUDev class
///          is an abstraction of the AFU engine. For the most part it is a
///          Proxy to the AFU engine.  It abstracts the transport and provides
///          the marshaling of C++ objects.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// PURPOSE: 
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/28/2009     JG       moved from FAPPIP_AFUdev.h
/// 02/05/2010     JG       Added workspace cleanup in Destroy
/// 12/09/2010     HM       Added debug to atomic CSR functions
///                         Fix offset problem in CAFUDev::CSRMapHandler()
/// 09/01/2011     JG       Removed use of Proxys@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/INTCDefs.h"
#include "aalsdk/uaia/FAPPIP_AFUdev.h"
#include "aalsdk/faptrans/FAP10.h"
#include "aalsdk/kernel/ahmpipdefs.h"
#include "aalsdk/AALLoggerExtern.h"


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


//=============================================================================
// Name: CAFUDev
// Description: The AFUDev object is the proxy to the physical AFU engine
//              implementation on a device. It abstracts a session between the
//              Host AFU and the device through the AIA.
//
//              The CAFUDev send messages to the device through ITransaction
//              objects.
//
//              The MessageRoute object of the AIASession contains the default
//              callback and the context when the device send message back
//              up through the AIA.
//  Inputs: handle   - AAL handle to device this object proxies
//          pSession - Pointer to the  Universal AIA interface (proxy)
// Comments: The uidrvMessage::uidrvMessageRoute contains the default event
//           handler and context for this AIA session and is extracted from the
//           AIA session through the OwnerMessageRoute() method and cached in
//           object.
//=============================================================================
CAFUDev::CAFUDev( void * handle,
                  IuAIASession *pSession)
: CAALBase( pSession->OwnerMessageRoute().Handler(),
            pSession->OwnerMessageRoute().Context()),
  _atomicSetCSR(NULL),
  _atomicGetCSR(NULL),
  m_Handle(handle),
  m_returnAddress(pSession->OwnerMessageRoute()),
  m_WSM(),
  m_pSession(pSession),
  m_csrwritemap(NULL),
  m_csrwritesize(0),
  m_csrwrite_item_size(0),
  m_csrreadmap(NULL),
  m_csrreadsize(0),
  m_csrread_item_size(0)
{
   m_bIsOK = false;         // CAASBase set it to true

   if(SetInterface(iidCAFUDev,dynamic_cast<CAFUDev*>(this))!= EObjOK) {
      return;
   }

   if(SetSubClassInterface(iidAFUDev,dynamic_cast<IAFUDev*>(this))!= EObjOK) {
      return;
   }

   // Get the message marshaller
   m_AIAMarshaller = m_pSession->ruAIA().GetMarshaller();
   m_bIsOK = true;
}

CAFUDev::~CAFUDev() {}

//=============================================================================
// Name: Initialize
// Description: Initialize the CAFUdev
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
void CAFUDev::Initialize(struct aalui_extbindargs * extBindParmsp, TransactionID const &rtid )
{
    // Store the extended bin parameters (TODO bind parms need to be deep copied)
    m_extBindParms = *extBindParmsp;

    // If mappable API enabled for this device then map CSRs
    if(m_extBindParms.m_mappableAPI){
       // Need to keep track of number of CSR map transactions to wait for.
       unsigned int *pnumEvents = NULL;

       AAL_VERBOSE(LM_AFU, "CAFUDev::Initialize() : m_mappableAPI " << m_extBindParms.m_mappableAPI << endl);

       // Create the Transaction
       // Send both transactions in parallel
       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_CSRREAD ) {
          AAL_VERBOSE(LM_AFU, "CAFUDev::Initialize() : m_mappableAPI Read" << endl);

          pnumEvents  = new unsigned;
          *pnumEvents = 1;

          struct CAFUDev::wrapper *preadwrap = new CAFUDev::wrapper(this, WSID_CSRMAP_READAREA, rtid, pnumEvents);
          TransactionID rdtid(static_cast<btApplicationContext>(preadwrap), _CSRMapHandler, true );

          FAP_10::Sig_MapCSRSpace_AFUTransaction MapReadTran( WSID_CSRMAP_READAREA );
          SendTransaction(&MapReadTran, rdtid);
       }

       if( m_extBindParms.m_mappableAPI & AAL_DEV_APIMAP_CSRWRITE ) {
          AAL_VERBOSE(LM_AFU, "CAFUDev::Initialize() : m_mappableAPI Write" << endl);

          if ( NULL == pnumEvents ) {
             pnumEvents = new unsigned;
             *pnumEvents = 0;
          }

          ++(*pnumEvents);

          struct CAFUDev::wrapper *pwriterwrap = new CAFUDev::wrapper(this,WSID_CSRMAP_WRITEAREA,rtid, pnumEvents);
          TransactionID wrtid(static_cast<btApplicationContext>(pwriterwrap), _CSRMapHandler, true );

          FAP_10::Sig_MapCSRSpace_AFUTransaction MapWriteTran( WSID_CSRMAP_WRITEAREA );
          SendTransaction(&MapWriteTran, wrtid);
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
// Comments: calls CAFUDev method
//=============================================================================
void CAFUDev::_CSRMapHandler(AAL::IEvent const &theEvent)
{
   // Check for exception
   if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
      PrintExceptionDescription(theEvent);
      return;
   }

   // Get the wrapper from the context of the event's TransactionID
   struct CAFUDev::wrapper *pwrapper = reinterpret_cast< struct CAFUDev::wrapper *>(dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).TranID().Context());

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
CAFUDev::CSRMapHandler(AAL::IEvent const   &theEvent,
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
         _atomicSetCSR = &AAL::AAS::AIA::CAFUDev::atomicSetCSR_32x4B;
         m_csrwritemap = pResult->wsParms.ptr;
      }
      // FSB
      else if ((8 == pResult->wsParms.itemsize) && (128 == pResult->wsParms.itemspacing)) {
         _atomicSetCSR = &AAL::AAS::AIA::CAFUDev::atomicSetCSR_64x128B;
         m_csrwritemap = pResult->wsParms.ptr;
      }
      else {
      // Can't do Atomic CSR (use default values)
      }

      m_csrwritesize       = (btUnsigned32bitInt)pResult->wsParms.size;
      m_csrwrite_item_size = (btUnsigned32bitInt)pResult->wsParms.itemsize;

      AAL_VERBOSE(LM_AFU, "CAFUDev::CSRMapHandler() : WSID_CSRMAP_WRITEAREA initialized" << endl);

   }  // if (id == WSID_CSRMAP_WRITEAREA)

   if ( WSID_CSRMAP_READAREA == id ) {

      // CCI
      if ((4 == pResult->wsParms.itemsize) && (4 == pResult->wsParms.itemspacing)) {
         _atomicGetCSR = &AAL::AAS::AIA::CAFUDev::atomicGetCSR_32x4B;
         m_csrreadmap  = pResult->wsParms.ptr;
      }
      // FSB/QPI/PCIe
      else if ((8 == pResult->wsParms.itemsize) && (128 == pResult->wsParms.itemspacing)) {
         _atomicGetCSR = &AAL::AAS::AIA::CAFUDev::atomicGetCSR_64x128B;
         m_csrreadmap  = pResult->wsParms.ptr;
      }
      else {
      // Can't do Atomic CSR (use default values)
      }

      m_csrreadsize       = (btUnsigned32bitInt)pResult->wsParms.size;
      m_csrread_item_size = (btUnsigned32bitInt)pResult->wsParms.itemsize;

      AAL_VERBOSE(LM_AFU, "CAFUDev::CSRMapHandler() : WSID_CSRMAP_READAREA initialized" << endl);

   }  // if (id == WSID_CSRMAP_READAREA)

   // Lock this critical region since 2 transactions are in flight simultaneously
   Lock();

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

   Unlock();
   return;
}

//=============================================================================
// Name: Handler
// Description: Return the event handler of the owner
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
btEventHandler CAFUDev::Handler()
{
   return m_returnAddress.Handler();
}

//=============================================================================
// Name: SendTransaction
// Description: Send a message to the device
// Inputs: pAFUmessage - Transaction object
//         rtid - Transaction ID
// Outputs: true - success
// Comments:
//=============================================================================
btBool CAFUDev::SendTransaction(IAFUTransaction *pAFUmessage, TransactionID const &rtid)
{
   // Wrap the message in a AFUTransaction object and have the AIA
   //  marshal it
   m_pSession->ruAIA().SendMessage( AFUTransaction( m_AIAMarshaller,
                                                    this,
                                                    m_Handle,
                                                    pAFUmessage,
                                                    rtid,
                                                    &m_returnAddress));

   return true;  /// SendMessage is a void TDO cleanup
}

// This provides a Direct Mapping from user space to the AFU's registers - very fast
// NOT YET IMPLEMENTED - PLACEHOLDER, currently returns NULL
IAFUCSRMap* CAFUDev::GetCSRMAP() { return NULL; }

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
CAFUDev::atomicSetCSR_64x128B( btUnsignedInt CSR, btUnsigned64bitInt Value )
{
   if ((m_csrwritemap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
      AAL_WARNING(LM_AFU, "CAFUDev::atomicSetCSR_64x128B Abort: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR << ", Value " << Value <<
                  showbase << hex << " " <<Value << endl);
      return false;
   }
   else {
      AAL_VERBOSE(LM_AFU, "CAFUDev::atomicSetCSR_64x128B: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<7) <<
                  ", Value " << Value <<
                  showbase << hex << " " <<Value << endl);
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
CAFUDev::atomicGetCSR_64x128B( btUnsignedInt CSR, btUnsigned64bitInt *pValue )
{
   if ((m_csrreadmap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
      AAL_WARNING(LM_AFU, "CAFUDev::atomicGetCSR_64x128B Abort: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR << endl);
      return false;
   }
   else {
      *pValue = *(reinterpret_cast <btUnsigned64bitInt*> (m_csrreadmap + (CSR << 7)));
      AAL_VERBOSE(LM_AFU, "CAFUDev::atomicGetCSR_64x128B: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<7) <<
                  ", Value " << *pValue <<
                  showbase << hex << " " << *pValue << endl);
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
btBool CAFUDev::atomicSetCSR_32x4B(btUnsignedInt CSR, btUnsigned64bitInt Value)
{
//   if((m_csrwritemap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)){
// disabling check for AHMPIP_MAX_AFU_CSR_INDEX for now to enable CCI devices.
   if( m_csrwritemap == NULL ) {
      AAL_WARNING(LM_AFU, "CAFUDev::atomicSetCSR_32x4B Abort: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR << ", Value " << Value <<
                  showbase << hex << " " <<Value << endl);
//      cout << "Map "<<m_csrwritemap << "CSR index " <<"CSR" <<endl;
      return false;
   }
   else {
      AAL_VERBOSE(LM_AFU, "CAFUDev::atomicSetCSR_32x4B: Base Address " << (void*)m_csrwritemap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<2) <<
                  ", Value " << Value <<
                  showbase << hex << " " <<Value << endl);
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
CAFUDev::atomicGetCSR_32x4B( btUnsignedInt CSR, btUnsigned64bitInt *pValue )
{
//   if ((m_csrreadmap == NULL) || (CSR > AHMPIP_MAX_AFU_CSR_INDEX)) {
// disabling check for AHMPIP_MAX_AFU_CSR_INDEX for now to enable CCI devices.
   if ( m_csrreadmap == NULL ) {
      AAL_WARNING(LM_AFU, "CAFUDev::atomicGetCSR_32x4B Abort: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR << endl);
      return false;
   } else {
      btUnsigned32bitInt tmp;
      tmp     = *( reinterpret_cast<btUnsigned32bitInt*>(m_csrreadmap + (CSR << 2)) );
      *pValue = (btUnsigned64bitInt)tmp;

      AAL_VERBOSE(LM_AFU, "CAFUDev::atomicGetCSR_32x4B: Base Address " << (void*)m_csrreadmap <<
                  ", CSR Index " << CSR <<
                  ", CSR Offset " << (CSR<<2) <<
                  ", Value " << *pValue <<
                  showbase << hex << " " << *pValue << endl);
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
btBool CAFUDev::MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet)
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
void CAFUDev::UnMapWSID(btVirtAddr ptr, btWSSize Size, btWSID wsid)
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
WorkSpaceMapper & CAFUDev::WSM() { return m_WSM; }


//=============================================================================
// Name: Destroy
// Description: Destroy this object async
// Inputs: TranID - Transaction ID
// Outputs: true - success
// Comments:
//=============================================================================
void CAFUDev::Destroy(TransactionID const &TranID){
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
void CAFUDev::FreeAllWS()
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

}

//=============================================================================
// Name: Handle
// Description: Low level handle
// Inputs: None
// Outputs: handle
// Comments:
//=============================================================================
void * CAFUDev::Handle()
{
   return m_Handle;
}


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


