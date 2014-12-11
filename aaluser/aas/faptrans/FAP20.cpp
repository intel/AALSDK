// Copyright (c) 2012-2014, Intel Corporation
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
/// @file FAP20.cpp
/// @brief FAP v2.0 specific details for the Universal Application Interface
///        Adaptor.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/19/2012     TSW      Moved FAP20 declarations from FAP10.cpp@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/INTCDefs.h>
#include <aalsdk/faptrans/FAP20.h>         // Definitions of AFUTransactions
#include <aalsdk/faptrans/FAP20Service.h>
#include <aalsdk/uaia/FAPPIP_AFUdev.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/utils/AALWorkSpaceUtilities.h>


#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

FAPTRANS2_BEGIN_MOD()
   /* No commands other than default, at the moment. */
FAPTRANS2_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)
         BEGIN_NAMESPACE(FAP_20)


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                    SPL2                   ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


SPL2_Start_AFUTransaction::SPL2_Start_AFUTransaction(IAFUDev             *pIAFUDev,
                                                     TransactionID const &tranID,
                                                     btVirtAddr           Address,
                                                     btTime               pollrate) :
   m_bIsOK(false),
   m_big(),
   m_ErrorString()
{
   ::memset(&m_big, 0, sizeof(struct big));

   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev>(iidCAFUDev, dynamic_cast<IBase *>(pIAFUDev));

   /*
   ** Need to get workspace. Input should be only a pointer.
   */

   if ( NULL != Address ) {
      WorkSpaceMapper::pcWkSp_t pWkSp = NULL;          // will point to a WkSp
      WorkSpaceMapper::eWSM_Ret eRet  = pCAFUDev->WSM().GetWkSp(Address, &pWkSp);

      // Address not found anywhere in mapper
      if ( (NULL != pWkSp) &&
           (WorkSpaceMapper::NOT_FOUND != eRet) ) {

         // Address found inside a block, not at the beginning
         if ( WorkSpaceMapper::FOUND_INCLUDED == eRet ) {
            ostringstream oss;
            oss << "Workspace Address " << static_cast<void*>(Address) <<
                   " is not the beginning of its workspace. Did you mean to pass " <<
                      static_cast<void*>(pWkSp->m_ptr) << "?";
            m_ErrorString = oss.str();
            return;
         }
         // At this point, Address must be equal to pWkSp->m_ptr
      }

      // Pass the workspace ID
      m_big.req.ahmreq.u.wksp.m_wsid = (NULL == pWkSp) ? -1 : pWkSp->m_wsid;

      // TODO: error path

   } else {
      m_big.req.ahmreq.u.wksp.m_wsid = 0;
   }

   m_big.req.afutskTranID = tranID;
   m_big.req.pollrate     = pollrate;

   m_big.afuMsg.cmd       = fappip_afucmdSTART_SPL2_TRANSACTION;
   m_big.afuMsg.payload   = (btVirtAddr)&m_big.req;
   m_big.afuMsg.size      = sizeof(struct spl2req);
   m_big.afuMsg.apiver    = GetAPIVer();
   m_big.afuMsg.pipver    = GetPIPVer();

   // Set up our hook to grab the response so that we can mmap the AFU DSM.
   m_tidEmbedded.Context(static_cast<btApplicationContext>(this));
   m_tidEmbedded.Handler(SPL2_Start_AFUTransaction::AFUDSMMappingHandler);
   m_tidEmbedded.Filter(true);

   m_bIsOK = true;
}

btBool          SPL2_Start_AFUTransaction::IsOK()     const { return m_bIsOK;            }
std::string     SPL2_Start_AFUTransaction::GetError() const { return m_ErrorString;      }
btID            SPL2_Start_AFUTransaction::GetPIPVer()      { return SPL2_AFUPIP_IID;    }
btID            SPL2_Start_AFUTransaction::GetAPIVer()      { return SPL2_AFUAPI_IID;    }
btVirtAddr      SPL2_Start_AFUTransaction::GetPayloadPtr()  { return (btVirtAddr)&m_big; }
btWSSize        SPL2_Start_AFUTransaction::GetSize()        { return static_cast<btWSSize>(sizeof(m_big)); }
TransactionID * SPL2_Start_AFUTransaction::GetTranIDPtr()   { return &m_tidEmbedded;     }
uid_msgIDs_e    SPL2_Start_AFUTransaction::GetCommand()     { return reqid_UID_SendAFU;  }

void SPL2_Start_AFUTransaction::AFUDSMMappingHandler(AAL::IEvent const &theEvent)
{
   //////////////////////////////////////////////////////////////////////////////////////
   // Get the original transactionID and CAFUDev.
   //
   // See how it was wrapped up in the TranIDWrapper in AAL::AAS::SendAFUTransaction()
   //
   // Do this first to prevent memory leaks if errors found later on
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the TransactionID
   TransactionID
            wrappingTranID(dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).TranID());

   // Its context points to a TranIDWrapper
   TranIDWrapper *pWrapper = static_cast<TranIDWrapper *>(wrappingTranID.Context());
   ASSERT(pWrapper);

   // Get the contents of the TranIDWrapper
   TransactionID origUserTranID(pWrapper->origTranID);
   CAFUDev *pCAFUDev = reinterpret_cast<CAFUDev *>(pWrapper->pCAFUDev);
   ASSERT(pCAFUDev);

   // Get the This pointer. It is in the event's context
   //SPL2_Start_AFUTransaction *This =
   //         static_cast<SPL2_Start_AFUTransaction *>(pWrapper->Context);
   //ASSERT(This);


   //////////////////////////////////////////////////////////////////////////////////////
   // Get objects and pointers required to send the final event
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the AIA object that sent this Event. Use it to get to the EDS and AFU

   uAIA *pAIA = static_cast <uAIA *>(theEvent.pObject());
   ASSERT(pAIA);

   // Get the Event Dispatcher
   AAL::XL::RT::IXLRuntimeServices  *pEventDispatcher = pAIA->getRuntimeServiceProvider();
   ASSERT(pEventDispatcher);

   // Get the AFU pointer. The AFU created the AIA object, and so the AIA object's Context should be the AFU pointer.
   //    Need it only for the 'this' pointer in event delivery, and there it is an IBase, so just get it that way.
   // TODO: enforce putting the pAFU in the context or in the AIA
   // Note that pIBaseAFU will be different from pAFU
#if AAL_LOG_LEVEL >= LOG_VERBOSE
   IBase *pIBaseAFU = static_cast<IBase *>(theEvent.Object().Context());
   AAL_VERBOSE(LM_UAIA, __AAL_FUNC__ << "() pAFU = " << static_cast<void*> (pIBaseAFU) << endl);
#endif

   // DEBUG CODE, should never happen
   if ( !pEventDispatcher ) {
      AAL_ERR(LM_UAIA, __AAL_FUNC__ << "() pEventHandler is NULL. A message has been lost." << endl);
      return;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   // Get the actual data to be returned.
   //////////////////////////////////////////////////////////////////////////////////////

   // This Event is a IUIDriverClientEvent. Get the results from the call.
   IUIDriverClientEvent *pevt = subclass_ptr<IUIDriverClientEvent>(theEvent);
   ASSERT(pevt);

   if ( uid_errnumOK == pevt->ResultCode() ) {

      ASSERT(rspid_AFU_Response == pevt->MessageID());
      if ( rspid_AFU_Response != pevt->MessageID() ) { // expect this to be rspid_AFU_Response
         AAL_ERR(LM_UAIA, __AAL_FUNC__ << "() MessageID " << pevt->MessageID()
                  << ". LOGIC ERROR, Expected rspid_AFU_Response\n");
         pevt->ResultCode(uid_errnumSystem);
      }

      // Payload is a aalui_WSMParms.
      struct aalui_AFUResponse *pRsp = reinterpret_cast<struct aalui_AFUResponse *>(pevt->Payload());

      if ( uid_afurespTaskStarted == pRsp->respID ) {
         // map the AFU DSM

         struct aalui_WSMParms *pWSParms = reinterpret_cast<struct aalui_WSMParms *>( aalui_AFURespPayload(pRsp) );

         AAL_VERBOSE(LM_UAIA, __AAL_FUNC__ << "() got AFU DSM aalui_WSMParms : " <<
                     "\n\twsid    " << std::hex << pWSParms->wsid <<
                     "\n\tsize    " << std::dec << pWSParms->size <<
                     "\n\tphysptr " << std::hex << pWSParms->physptr <<
                     endl);

         // MMAP the workspace id in order to get the pointer
         btBool fRet;
         fRet = pCAFUDev->MapWSID(pWSParms->size,
                                  pWSParms->wsid,
                                  &pWSParms->ptr);

         // If can't map, store that in the event
         if ( !fRet ) {
            pevt->ResultCode(uid_errnumNoMap);
         } else {
            // Store the pointer in the map
            fRet = pCAFUDev->WSM().AddToMap(pWSParms->ptr,
                                            pWSParms->size,
                                            pWSParms->size,
                                            pWSParms->wsid,
                                            pWSParms->type,
                                            pWSParms->physptr);

            if ( !fRet ) {
               pevt->ResultCode(uid_errnumBadMapping);
            }
         }

         AAL_VERBOSE(LM_UAIA, __AAL_FUNC__ << "() Workspace Map after Start Transaction : " << pCAFUDev->WSM());

      } else if ( uid_afurespTaskComplete == pRsp->respID ) {
         // Task complete - unmap the AFU DSM and delete the wrapper.

         struct aalui_WSMParms *pWSParms = reinterpret_cast<struct aalui_WSMParms *>( aalui_AFURespPayload(pRsp) );

         AAL_VERBOSE(LM_UAIA, __AAL_FUNC__ << "() got AFU DSM aalui_WSMParms : " <<
                     "\n\twsid    " << std::hex << pWSParms->wsid <<
                     "\n\tsize    " << std::dec << pWSParms->size <<
                     "\n\tphysptr " << std::hex << pWSParms->physptr <<
                     endl);


         // Iterate over the workspace mappings until we find the magic xsid.
         WorkSpaceMapper::pcWkSp_t wksp = NULL;
         WorkSpaceMapper::eWSM_Ret wkret;

         wkret = pCAFUDev->WSM().GetWkSp(&wksp, true);
         ASSERT(NULL != wksp);
         ASSERT(WorkSpaceMapper::FOUND == wkret);
         if ( (NULL == wksp) ||
              (WorkSpaceMapper::FOUND != wkret) ) {
            pevt->ResultCode(uid_errnumNoMap);
         }

         if ( NULL != wksp ) {
            do
            {
               if ( wksp->m_wsid == pWSParms->wsid ) {
                  // no error checking for now. munmap() returns -1 on error, with errno set
                  pCAFUDev->UnMapWSID(wksp->m_ptr, wksp->m_len, wksp->m_wsid);
                  pCAFUDev->WSM().RemoveFromMap(wksp->m_ptr);
                  break;
               }

               wkret = pCAFUDev->WSM().GetWkSp(&wksp, false);

            }while( WorkSpaceMapper::NOT_FOUND != wkret );

            if ( WorkSpaceMapper::NOT_FOUND == wkret ) {
               pevt->ResultCode(uid_errnumNoMap);
            }
         }

         // TODO: error path

         // Delete the TranID Wrapper. It was allocated in SendAFUTransaction
         delete pWrapper;
      }

   }  // uid_errnumOK == pevt->ResultCode()

   ReThrow(&theEvent.Object(), // Pointer to AIA as IBase*
           theEvent,
           pEventDispatcher,
           pCAFUDev->Handler(),
           &origUserTranID);
}


SPL2_Stop_AFUTransaction::SPL2_Stop_AFUTransaction(IAFUDev             *pIAFUDev,
                                                     TransactionID const &tranID) :
   m_bIsOK(false),
   m_big(),
   m_ErrorString()
{
   ::memset(&m_big, 0, sizeof(struct big));

   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev>(iidCAFUDev, dynamic_cast<IBase *>(pIAFUDev));


   m_big.req.afutskTranID = tranID;

   m_big.afuMsg.cmd       = fappip_afucmdSTOP_SPL2_TRANSACTION;
   m_big.afuMsg.payload   = (btVirtAddr)&m_big.req;
   m_big.afuMsg.size      = sizeof(struct spl2req);
   m_big.afuMsg.apiver    = GetAPIVer();
   m_big.afuMsg.pipver    = GetPIPVer();

   m_bIsOK = true;
}

btBool          SPL2_Stop_AFUTransaction::IsOK()     const { return m_bIsOK;            }
std::string     SPL2_Stop_AFUTransaction::GetError() const { return m_ErrorString;      }
btID            SPL2_Stop_AFUTransaction::GetPIPVer()      { return SPL2_AFUPIP_IID;    }
btID            SPL2_Stop_AFUTransaction::GetAPIVer()      { return SPL2_AFUAPI_IID;    }
btVirtAddr      SPL2_Stop_AFUTransaction::GetPayloadPtr()  { return (btVirtAddr)&m_big; }
btWSSize        SPL2_Stop_AFUTransaction::GetSize()        { return static_cast<btWSSize>(sizeof(m_big)); }
TransactionID * SPL2_Stop_AFUTransaction::GetTranIDPtr()   { return &m_tidEmbedded;     }
uid_msgIDs_e    SPL2_Stop_AFUTransaction::GetCommand()     { return reqid_UID_SendAFU;  }


SPL2_SetContextWorkspace_AFUTransaction::SPL2_SetContextWorkspace_AFUTransaction(
   IAFUDev             *pIAFUDev,
   TransactionID const &tranID,
   btVirtAddr           Address,
   btTime               pollrate) :
   m_bIsOK(false),
   m_big(),
   m_ErrorString()
{
   ::memset(&m_big, 0, sizeof(struct big));

   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev>(iidCAFUDev, dynamic_cast<IBase *>(pIAFUDev));

   WorkSpaceMapper::pcWkSp_t pWkSp = NULL;          // will point to a WkSp
   WorkSpaceMapper::eWSM_Ret eRet  = pCAFUDev->WSM().GetWkSp(Address, &pWkSp);

   // Address not found anywhere in mapper
   if ( (NULL == pWkSp) ||
        (WorkSpaceMapper::NOT_FOUND != eRet) ) {

      // Address found inside a block, not at the beginning
      if ( WorkSpaceMapper::FOUND_INCLUDED == eRet ) {
         ostringstream oss;
         oss << "Workspace Address " << static_cast<void*>(Address) <<
                " is not the beginning of its workspace. Did you mean to pass " <<
                static_cast<void*>(pWkSp->m_ptr) << "?";
         m_ErrorString = oss.str();
         return;
      }
            // At this point, Address must be equal to pWkSp->m_ptr
   }

   // TODO: error path?

   // Pass the workspace ID
   m_big.req.ahmreq.u.wksp.m_wsid = (NULL == pWkSp) ? -1 : pWkSp->m_wsid;
   m_big.req.afutskTranID         = tranID;
   m_big.req.pollrate             = pollrate;

   m_big.afuMsg.cmd               = fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE;
   m_big.afuMsg.payload           = (btVirtAddr)&m_big.req;
   m_big.afuMsg.size              = sizeof(struct spl2req);
   m_big.afuMsg.apiver            = GetAPIVer();
   m_big.afuMsg.pipver            = GetPIPVer();

   m_bIsOK = true;
}

btBool          SPL2_SetContextWorkspace_AFUTransaction::IsOK()     const { return m_bIsOK;            }
std::string     SPL2_SetContextWorkspace_AFUTransaction::GetError() const { return m_ErrorString;      }
btID            SPL2_SetContextWorkspace_AFUTransaction::GetPIPVer()      { return SPL2_AFUPIP_IID;    }
btID            SPL2_SetContextWorkspace_AFUTransaction::GetAPIVer()      { return SPL2_AFUAPI_IID;    }
btVirtAddr      SPL2_SetContextWorkspace_AFUTransaction::GetPayloadPtr()  { return (btVirtAddr)&m_big; }
btWSSize        SPL2_SetContextWorkspace_AFUTransaction::GetSize()        { return static_cast<btWSSize>(sizeof(m_big)); }
TransactionID * SPL2_SetContextWorkspace_AFUTransaction::GetTranIDPtr()   { return NULL;               }
uid_msgIDs_e    SPL2_SetContextWorkspace_AFUTransaction::GetCommand()     { return reqid_UID_SendAFU;  }


         END_NAMESPACE(FAP_20)
      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

