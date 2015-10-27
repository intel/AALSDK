
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
/// @file AIATransactions.cpp
/// @brief Transaction Objects represent Commands/Messages to be sent down
///        to the AFU object.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/23/2015     JG       Initial version@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aalsdk/AALLoggerExtern.h"

#include "aalsdk/INTCDefs.h"
#include "aalsdk/kernel/aalui.h"

#include "ALIAIATransactions.h"

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__


#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


USING_NAMESPACE(AAL)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////                                           ///////////////////
////////////      A L I   A I A   T R A N S A C T I O N S      ////////////////
/////////////////                                           ///////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


BufferAllocateTransaction::BufferAllocateTransaction( TransactionID const &tranID, btWSSize len ) :
   m_msgID(reqid_UID_SendPIP),
   m_tid_t(tranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_AFUmessage packaged in an
   // BufferAllocate-AIATransaction.

   // Allocate structs
   struct aalui_AFUmessage *afumsg  = (new (std::nothrow) struct aalui_AFUmessage);
   struct ahm_req *req              = (new (std::nothrow) struct ahm_req);

   // fill out aalui_AFUmessage
   afumsg->cmd     = fappip_afucmdWKSP_VALLOC;
   afumsg->payload = (btVirtAddr) req;
   afumsg->size    = sizeof(struct ahm_req);
   afumsg->apiver  = AAL_AHMAPI_IID_1_0;
   afumsg->pipver  = AAL_AHMPIP_IID_1_0;

   // fill out ahm_req
   req->u.wksp.m_wsid   = 0;        // not used?
   req->u.wksp.m_size   = len;
   req->u.wksp.m_pgsize = 0;        // not used?

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;
   m_size = sizeof(struct aalui_AFUmessage);

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    BufferAllocateTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                BufferAllocateTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  BufferAllocateTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   BufferAllocateTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              BufferAllocateTransaction::getMsgID()const {return m_msgID;}

BufferAllocateTransaction::~BufferAllocateTransaction() {
   // unpack payload and free memory
   struct aalui_AFUmessage *afumsg = (aalui_AFUmessage *)m_payload;
   struct ahm_req          *req    = (ahm_req *)afumsg->payload;
   delete req;
   delete afumsg;
}




BufferFreeTransaction::BufferFreeTransaction( TransactionID const &tranID, btWSID wsid ) :
   m_msgID(reqid_UID_SendPIP),
   m_tid_t(tranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_AFUmessage packaged in an
   // BufferAllocate-AIATransaction.

   // Allocate structs
   struct aalui_AFUmessage *afumsg  = (new (std::nothrow) struct aalui_AFUmessage);
   struct ahm_req *req              = (new (std::nothrow) struct ahm_req);

   // fill out aalui_AFUmessage
   afumsg->cmd     = fappip_afucmdWKSP_VFREE;
   afumsg->payload = (btVirtAddr) req;
   afumsg->size    = sizeof(struct ahm_req);
   afumsg->apiver  = AAL_AHMAPI_IID_1_0;
   afumsg->pipver  = AAL_AHMPIP_IID_1_0;

   // fill out ahm_req
   req->u.wksp.m_wsid   = wsid;
   req->u.wksp.m_size   = 0;        // not used
   req->u.wksp.m_pgsize = 0;        // not used?

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;
   m_size = sizeof(struct aalui_AFUmessage);

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    BufferFreeTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                BufferFreeTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  BufferFreeTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   BufferFreeTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              BufferFreeTransaction::getMsgID()const {return m_msgID;}

BufferFreeTransaction::~BufferFreeTransaction() {
   // unpack payload and free memory
   struct aalui_AFUmessage *afumsg = (aalui_AFUmessage *)m_payload;
   struct ahm_req          *req    = (ahm_req *)afumsg->payload;
   delete req;
   delete afumsg;
}



GetMMIOBufferTransaction::GetMMIOBufferTransaction( TransactionID const &tranID ) :
   m_msgID(reqid_UID_SendPIP),
   m_tid_t(tranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_AFUmessage packaged in an
   // BufferAllocate-AIATransaction.

   // Allocate structs
   struct aalui_AFUmessage *afumsg  = (new (std::nothrow) struct aalui_AFUmessage);
   struct ahm_req *req              = (new (std::nothrow) struct ahm_req);

   // fill out aalui_AFUmessage
   afumsg->cmd     = fappip_getMMIORmap;
   afumsg->payload = (btVirtAddr) req;
   afumsg->size    = sizeof(struct ahm_req);
   afumsg->apiver  = AAL_AHMAPI_IID_1_0;
   afumsg->pipver  = AAL_AHMPIP_IID_1_0;

   req->u.wksp.m_wsid = WSID_MAP_MMIOR;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;
   m_size = sizeof(struct aalui_AFUmessage);

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    GetMMIOBufferTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                GetMMIOBufferTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  GetMMIOBufferTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   GetMMIOBufferTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              GetMMIOBufferTransaction::getMsgID()const {return m_msgID;}

GetMMIOBufferTransaction::~GetMMIOBufferTransaction() {
   // unpack payload and free memory
   struct aalui_AFUmessage *afumsg = (aalui_AFUmessage *)m_payload;
   struct ahm_req          *req    = (ahm_req *)afumsg->payload;
   delete req;
   delete afumsg;
}



GetUMSGBufferTransaction::GetUMSGBufferTransaction( TransactionID const &tranID ) :
   m_msgID(reqid_UID_SendPIP),
   m_tid_t(tranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_AFUmessage packaged in an
   // BufferAllocate-AIATransaction.

   // Allocate structs
   struct aalui_AFUmessage *afumsg  = (new (std::nothrow) struct aalui_AFUmessage);
   struct ahm_req *req              = (new (std::nothrow) struct ahm_req);

   // fill out aalui_AFUmessage
   afumsg->cmd     = fappip_getuMSGmap;
   afumsg->payload = (btVirtAddr) req;
   afumsg->size    = sizeof(struct ahm_req);
   afumsg->apiver  = AAL_AHMAPI_IID_1_0;
   afumsg->pipver  = AAL_AHMPIP_IID_1_0;

   req->u.wksp.m_wsid = WSID_MAP_UMSG;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;
   m_size = sizeof(struct aalui_AFUmessage);

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    GetUMSGBufferTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                GetUMSGBufferTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  GetUMSGBufferTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   GetUMSGBufferTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              GetUMSGBufferTransaction::getMsgID()const {return m_msgID;}

GetUMSGBufferTransaction::~GetUMSGBufferTransaction() {
   // unpack payload and free memory
   struct aalui_AFUmessage *afumsg = (aalui_AFUmessage *)m_payload;
   struct ahm_req          *req    = (ahm_req *)afumsg->payload;
   delete req;
   delete afumsg;
}



/*
BufferGetIOVA::BufferGetIOVA( TransactionID const &tranID ) :
   m_msgID(???),
   m_tid_t(tranID),
   m_bIsOK(true)
{}

AAL::btVirtAddr                BufferGetIOVA::getPayloadPtr()const {return NULL;}
AAL::btWSSize                  BufferGetIOVA::getPayloadSize()const {return 0;}
AAL::stTransactionID_t const   BufferGetIOVA::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              BufferGetIOVA::getMsgID()const {return m_msgID;}


GetMMIORMap::GetMMIORMap( TransactionID const &tranID ) :
   m_msgID(fappip_getMMIORmap),
   m_tid_t(tranID),
   m_bIsOK(true)
{}

AAL::btVirtAddr                GetMMIORMap::getPayloadPtr()const {return NULL;}
AAL::btWSSize                  GetMMIORMap::getPayloadSize()const {return 0;}
AAL::stTransactionID_t const   GetMMIORMap::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              GetMMIORMap::getMsgID()const {return m_msgID;}

*/


/*
UnBindAFUDevice::UnBindAFUDevice( TransactionID const &tranID ) :
   m_msgID(reqid_UID_UnBind),
   m_tid_t(tranID),
   m_bIsOK(true)
{}

AAL::btVirtAddr                UnBindAFUDevice::getPayloadPtr()const {return NULL;}
AAL::btWSSize                  UnBindAFUDevice::getPayloadSize()const {return 0;}
AAL::stTransactionID_t const   UnBindAFUDevice::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              UnBindAFUDevice::getMsgID()const {return m_msgID;}


ShutdownMDT::ShutdownMDT(AAL::TransactionID const &tranID, AAL::btTime timeout) :
   m_msgID(reqid_UID_Shutdown),
   m_tid_t(tranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_timeout(timeout)
{
   union{
      AAL::btVirtAddr         ppayload;
      struct aalui_Shutdown  *puiShutdown;
   };

   // Allocate to union and save pointer avoiding casting ugliness
   puiShutdown = (new (std::nothrow) struct aalui_Shutdown);
   m_payload = ppayload;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_size = sizeof(struct aalui_Shutdown);

   puiShutdown->m_reason = ui_shutdownReasonNormal;
   puiShutdown->m_timeout = m_timeout;
   m_bIsOK = true;

}
AAL::btBool                    ShutdownMDT::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                ShutdownMDT::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  ShutdownMDT::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   ShutdownMDT::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              ShutdownMDT::getMsgID()const {return m_msgID;}
*/
