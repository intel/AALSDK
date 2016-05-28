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
/// @file HWALIBase.cpp
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
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/uaia/IAFUProxy.h>
#include "ALIAIATransactions.h"
#include "HWALIBase.h"
#include "aalsdk/aas/Dispatchables.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

//
// ctor, CHWALIBase Base constructor.
//
CHWALIBase::CHWALIBase( IBase *pSvcClient,
                        IServiceBase *pServiceBase,
                        TransactionID transID,
                        IAFUProxy *pAFUProxy):
                        CALIBase(pSvcClient,pServiceBase,transID),
                        m_pAFUProxy(pAFUProxy),
                        m_mapWkSpc(),
                        m_MMIORmap(NULL),
                        m_MMIORsize(0)

{

}

// ---------------------------------------------------------------------------
// IALIMMIO interface implementation
// ---------------------------------------------------------------------------

//
// mmioGetAddress. Return address of MMIO space.
//
btVirtAddr CHWALIBase::mmioGetAddress( void )
{
   return m_MMIORmap;
}

//
// mmioGetLength. Return length of MMIO space.
//
btCSROffset CHWALIBase::mmioGetLength( void )
{
   return m_MMIORsize;
}

//
// mmioRead32. Read 32bit CSR. Offset given in bytes.
//
btBool CHWALIBase::mmioRead32(const btCSROffset Offset, btUnsigned32bitInt * const pValue)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   // FIXME: volatile might not be necessary, need to investigate further
   *pValue = *( reinterpret_cast<volatile btUnsigned32bitInt *>(m_MMIORmap + Offset) );

   return true;
}

//
// mmioWrite32. Write 32bit CSR. Offset given in bytes.
//
btBool CHWALIBase::mmioWrite32(const btCSROffset Offset, const btUnsigned32bitInt Value)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   // FIXME: volatile might not be necessary, need to investigate further
   *( reinterpret_cast<volatile btUnsigned32bitInt *>(m_MMIORmap + Offset) ) = Value;

   return true;
}

//
// mmioRead64. Read 64bit CSR. Offset given in bytes.
//
btBool CHWALIBase::mmioRead64(const btCSROffset Offset, btUnsigned64bitInt * const pValue)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   volatile btUnsigned64bitInt * vPtr = reinterpret_cast<btUnsigned64bitInt *>( m_MMIORmap + Offset );

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
//   *pValue = *( reinterpret_cast<btUnsigned64bitInt *>(m_MMIORmap + Offset) );
   *pValue = *vPtr;

   return true;
}

//
// mmioWrite64. Write 64bit CSR. Offset given in bytes.
//
btBool CHWALIBase::mmioWrite64(const btCSROffset Offset, const btUnsigned64bitInt Value)
{
   if ( (NULL == m_MMIORmap) || (Offset > m_MMIORsize) ) {
      return false;
   }

   // m_MMIORmap is btVirtAddr is char*, so offset is in bytes
   // FIXME: volatile might not be necessary, need to investigate further
   *( reinterpret_cast<volatile btUnsigned64bitInt *>(m_MMIORmap + Offset) ) = Value;

   return true;
}


//
// mmioGetFeature. Get pointer to feature's DFH, if found.
//
btBool CHWALIBase::mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                        NamedValueSet const &rInputArgs,
                                        NamedValueSet       &rOutputArgs )
{
   btBool             filterByID;
   btUnsigned64bitInt filterID;
   btBool             filterByType;
   btUnsigned64bitInt filterType;
   btBool             filterByGUID;
   btcString          filterGUID;

   // extract filters
   filterByID = false;
   if (rInputArgs.Has(ALI_GETFEATURE_ID_KEY)) {
      if (ENamedValuesOK != rInputArgs.Get(ALI_GETFEATURE_ID_KEY, &filterID)) {
         AAL_ERR(LM_All, "rInputArgs.Get(ALI_GETFEATURE_ID) failed -- " <<
                         "wrong datatype?" << std::endl);
         return false;
      } else {
         filterByID = true;
      }
   }

   filterByType = false;
   if (rInputArgs.Has(ALI_GETFEATURE_TYPE_KEY)) {
      if (ENamedValuesOK != rInputArgs.Get(ALI_GETFEATURE_TYPE_KEY, &filterType)) {
         AAL_ERR(LM_All, "rInputArgs.Get(ALI_GETFEATURE_TYPE) failed -- " <<
                         "wrong datatype?" << std::endl);
         return false;
      } else {
         filterByType = true;
      }
   }

   filterByGUID = false;
   if (rInputArgs.Has(ALI_GETFEATURE_GUID_KEY)) {
      if (ENamedValuesOK != rInputArgs.Get(ALI_GETFEATURE_GUID_KEY, &filterGUID)) {
         AAL_ERR(LM_All, "rInputArgs.Get(ALI_GETFEATURE_GUID) failed -- " <<
                         "wrong datatype?" << std::endl);
         return false;
      } else {
         filterByGUID = true;
      }
   }

   //
   // Spec and sanity checks
   //

   // Can't search for GUID in private features
   if ((filterByGUID && filterByType && (filterType == ALI_DFH_TYPE_PRIVATE))) {
      AAL_ERR(LM_AFU, "Can't search for GUIDs in private features." << std::endl);
      return false;
   }


   // walk features
   AAL_DEBUG(LM_AFU, "Walking feature list..." << std::endl);
   for ( FeatureList::iterator iter = m_featureList.begin();
         iter != m_featureList.end(); ++iter ) {
      // NOTE: if we had C++1 or later, we coud do:
      // for ( FeatureDefinition feat : m_featureList )
      // and work with "feat" directly.
      FeatureDefinition &feat = *iter;

      // return first matching feature
      if (
            ( !filterByID   || (feat.dfh.Feature_ID == filterID  )              ) &&
            ( !filterByType || (feat.dfh.Type       == filterType)              ) &&
            ( !filterByGUID || ( (feat.dfh.Type != ALI_DFH_TYPE_PRIVATE) &&
                                 ( 0 == strncmp(filterGUID,
                                          GUIDStringFromStruct(
                                            GUIDStructFrom2xU64(
                                              feat.guid[1],
                                              feat.guid[0]
                                            )
                                          ).c_str(),
                                          16
                                        )
                                 )
                               )
            )
         ) {

         AAL_INFO(LM_AFU, "Found matching feature." << std::endl);
         *pFeatureAddress = (btVirtAddr)(m_MMIORmap + feat.offset);   // return pointer to DFH
         // populate output args
         rOutputArgs.Add(ALI_GETFEATURE_ID_KEY, feat.dfh.Feature_ID);
         rOutputArgs.Add(ALI_GETFEATURE_TYPE_KEY, feat.dfh.Type);
         if (feat.dfh.Type != ALI_DFH_TYPE_PRIVATE) {
            rOutputArgs.Add(ALI_GETFEATURE_GUID_KEY, GUIDStringFromStruct(
                                                       GUIDStructFrom2xU64(
                                                         feat.guid[1],
                                                         feat.guid[0]
                                                       )
                                                     ).c_str()
                           );
         }
         return true;
      }
   }

   // if not found, do not modify ppFeature, return false.
   AAL_INFO(LM_AFU, "No matching feature found." << std::endl);
   return false;
}

btBool CHWALIBase::mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                        NamedValueSet const &rInputArgs )
{
   NamedValueSet temp;
   return mmioGetFeatureAddress(pFeatureAddress, rInputArgs, temp);
}

btBool CHWALIBase::mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                       NamedValueSet const &rInputArgs,
                                       NamedValueSet       &rOutputArgs )
{
   btVirtAddr pFeatAddr;
   if (true == mmioGetFeatureAddress(&pFeatAddr, rInputArgs, rOutputArgs)) {
      *pFeatureOffset = pFeatAddr - mmioGetAddress();
      return true;
   }
   return false;
}

// overloaded version without rOutputArgs
btBool CHWALIBase::mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                       NamedValueSet const &rInputArgs )
{
   NamedValueSet temp;
   return mmioGetFeatureOffset(pFeatureOffset, rInputArgs, temp);
}

//
// mapMMIO,maps MMIO region.
//
btBool CHWALIBase:: mapMMIO()
{
   // Create the Transactions
   GetMMIOBufferTransaction mmioTransaction;

   // Check the parameters
   if ( mmioTransaction.IsOK()/* && umsgTransaction.IsOK()*/) {
      // Will return to AFUEvent(), below.
      m_pAFUProxy->SendTransaction(&mmioTransaction);

      if(uid_errnumOK == mmioTransaction.getErrno() ){
         struct AAL::aalui_WSMEvent wsevt = mmioTransaction.getWSIDEvent();

         // mmap
         if (!m_pAFUProxy->MapWSID(wsevt.wsParms.size, wsevt.wsParms.wsid, &wsevt.wsParms.ptr)) {
            AAL_ERR( LM_All, "FATAL: MapWSID failed");
            m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                       m_tidSaved,
                                                                       errCreationFailure,
                                                                       reasUnknown,
                                                                       "Error: MapWSID failed."));
            return false;
         }

         // Remember workspace parameters associated with virtual ptr (if we ever need it)
         if (m_mapWkSpc.find(wsevt.wsParms.ptr) != m_mapWkSpc.end()) {
            AAL_ERR( LM_All, "FATAL: WSID already exists in m_mapWSID");
            m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                       m_tidSaved,
                                                                       errCreationFailure,
                                                                       reasUnknown,
                                                                       "Error: Duplicate WSID."));
            return false;
         } else {
            // store entire aalui_WSParms struct in map
            m_mapWkSpc[wsevt.wsParms.ptr] = wsevt.wsParms;
         }

         m_MMIORmap = wsevt.wsParms.ptr;
         m_MMIORsize = wsevt.wsParms.size;

         ASSERT( m_MMIORmap != NULL );
         ASSERT( m_MMIORsize > 0 );

         // Register MMIO interface
         if ( EObjOK !=  SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(this)) ) {

            m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                       m_tidSaved,
                                                                       errCreationFailure,
                                                                       reasUnknown,
                                                                       "Error: Could not register interface."));
            return false;
         }

         if (m_MMIORmap != NULL && m_MMIORsize > 0) {

            if (! _discoverFeatures() ) {
               // FIXME: use correct error classes
               m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                          m_tidSaved,
                                                                          errBadParameter,
                                                                          reasMissingInterface,
                                                                          "Failed to discover features."));
               return false;
            }

            // Print warnings for malformed device feature lists
            _validateDFL();
         }

         return true;
      } else {

         m_pServiceBase->initFailed(new CExceptionTransactionEvent(NULL,
                                                                   m_tidSaved,
                                                                   errAFUWorkSpace,
                                                                   mmioTransaction.getErrno(),
                                                                   "GetMMIOBuffer/GetUMSGBuffer transaction validity check failed"));
         return false;
      }
      //      m_pAFUProxy->SendTransaction(&umsgTransaction);
   } else {
      m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                 m_tidSaved,
                                                                 errAFUWorkSpace,
                                                                 reasAFUNoMemory,
                                                               " GetMMIOBuffer/GetUMSGBuffer transaction validity check failed"));
      return false;
   }

}

//
// _discoverFeatures,Enumerates CCIp device features.
//
btBool CHWALIBase::_discoverFeatures() {

   btBool retVal;

   // walk DFH list and populate internal data structure
   // also do some sanity checking
   AAL_DEBUG(LM_AFU, "Populating feature list from DFH list..." << std::endl);
   FeatureDefinition feat;
   btUnsigned32bitInt offset = 0;         // offset that we are currently at

   // look at AFU CSR (mandatory) to get first feature header offset
   retVal = mmioRead64(0, (btUnsigned64bitInt *)&feat.dfh);
   ASSERT( retVal );
   _printDFH(feat.dfh);
   feat.offset = offset;

   // read AFUID
   retVal = mmioRead64(offset +  8, &feat.guid[0]);
   ASSERT( retVal );
   retVal = mmioRead64(offset + 16, &feat.guid[1]);
   ASSERT( retVal );

   // Add AFU feature to list
   m_featureList.push_back(feat);
   offset = feat.dfh.next_DFH_offset;

   // look at chained DFHs until end of list bit is set or next offset is 0
   while (feat.dfh.eol == 0 && feat.dfh.next_DFH_offset != 0) {

      // populate fields
      feat.offset = offset;
      // read feature header
      retVal = mmioRead64(offset, (btUnsigned64bitInt *)&feat.dfh);
      ASSERT( retVal );
      _printDFH(feat.dfh);
      // read guid, if present
      if (feat.dfh.Type == ALI_DFH_TYPE_BBB) {
         retVal = mmioRead64(offset +  8, &feat.guid[0]);
         ASSERT( retVal );
         retVal = mmioRead64(offset + 16, &feat.guid[1]);
         ASSERT( retVal );
      } else {
         feat.guid[0] = feat.guid[1] = 0;
      }

      // create new list entry
      m_featureList.push_back(feat);          // copy dfh to list

      // check for next header
      offset += feat.dfh.next_DFH_offset;
   }

   return true;
}

//
// _validateDFL,Validate CCIP device features.
//
btBool CHWALIBase::_validateDFL()
{
   // check for ambiguous featureID/GUID
   AAL_DEBUG(LM_AFU, "Checking detected features for ambiguous IDs..." <<
         std::endl);
   for (FeatureList::iterator a = m_featureList.begin(); a !=
         m_featureList.end(); ++a) {
      for (FeatureList::iterator b = a+1; b != m_featureList.end(); ++b ) {

         AAL_DEBUG(LM_AFU, "Comparing features at 0x" << std::hex << a->offset <<
                  " and 0x" << b->offset << std::endl);

         if (a->dfh.Feature_ID == b->dfh.Feature_ID) {
            AAL_INFO(LM_AFU, "Features at 0x" << std::hex << a->offset <<
                  " and 0x" << b->offset << " share feature ID " <<
                  std::dec << a->dfh.Feature_ID << std::endl);

            if (a->dfh.Type != b->dfh.Type) {
               AAL_INFO(LM_AFU, "   Features can be disambiguated by type - OK."
                     << std::endl);
            } else {
               if (a->dfh.Type == ALI_DFH_TYPE_BBB) {
                  if (a->guid[0] != b->guid[0] || a->guid[1] != b->guid[1]) {
               AAL_INFO(LM_AFU, "   Features can be disambiguated by BBB GUID - OK."
                     << std::endl);
                  } else {
                     AAL_WARNING(LM_AFU,
                           "   Features have same BBB GUID! This is not recommended,"
                           << std::endl);
                     AAL_WARNING(LM_AFU,
                           "   as it complicates disambiguation." << std::endl);
                     AAL_WARNING(LM_AFU,
                           "   Please consider giving out separate feature IDs for"
                           << std::endl);
                     AAL_WARNING(LM_AFU, "   each." << std::endl);
                  }
               } else {
                  AAL_WARNING(LM_AFU,
                        "   Features have same feature ID and no other standard"
                        << std::endl);
                  AAL_WARNING(LM_AFU,
                        "   mechanism for disambiguation! This is not recommended."
                        << std::endl);
                  AAL_WARNING(LM_AFU,
                        "   Please consider giving out separate feature IDs for"
                        << std::endl);
                  AAL_WARNING(LM_AFU, "   each." << std::endl);
               }
            }
         }
      }
   }
   return true;
}

//
// _printDFH,Prints CCIP Device features.
//
void CHWALIBase::_printDFH( const struct CCIP_DFH &dfh )
{
   AAL_DEBUG(LM_AFU, "Type: " << std::hex << std::setw(2)
         << std::setfill('0') << dfh.Type <<
         ", Next DFH offset: " << dfh.next_DFH_offset <<
         ", Feature Rev: " << dfh.Feature_rev <<
         ", Feature ID: " << dfh.Feature_ID <<
         ", eol: " << std::dec << dfh.eol << std::endl);
}


// ---------------------------------------------------------------------------
// IAFUProxyClient interface implementation
// ---------------------------------------------------------------------------

//
// AFUEvent,AFU Event Handler.
// Callback for ALIAFUProxy
//
void CHWALIBase::AFUEvent(AAL::IEvent const &theEvent)
{
   IUIDriverEvent *puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                           theEvent);

   ASSERT(NULL != puidEvent);

   std::cerr << "Got CHWALIBase AFU event BASE type " << puidEvent->MessageID() << "\n" << std::endl;

   switch(puidEvent->MessageID())
   {

   case rspid_AFU_PR_Revoke_Event:
   {
      getRuntime()->schedDispatchable( new ServiceRevoke(dynamic_ptr<IServiceRevoke>(iidServiceRevoke,this)) );

   }
   break;

   case rspid_AFU_PR_Release_Request_Event:
   {
      struct aalui_PREvent *pResult = reinterpret_cast<struct aalui_PREvent *>(puidEvent->Payload());

      getRuntime()->schedDispatchable( new ReleaseServiceRequest(m_pSvcClient, new CReleaseRequestEvent(NULL,
                                                                                                        pResult->reconfTimeout,
                                                                                                        IReleaseRequestEvent::resource_revokeing,
                                                                                                        "AFU Release Request")) );
   }
   break;

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

   default:
      ASSERT(false); // unexpected event
   }

}

/// @} group ALI

END_NAMESPACE(AAL)
