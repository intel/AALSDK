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
#include "aalsdk/kernel/ccipdriver.h"

#include "ALIAIATransactions.h"
#include "aalsdk/service/IALIAFU.h"

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


BufferAllocateTransaction::BufferAllocateTransaction( btWSSize len ) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferAllocate-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdWKSP_ALLOC;
   afumsg->size    = sizeof(struct ahm_req);

   // fill out ahm_req
   req->u.wksp.m_wsid   = 0;        // not used?
   req->u.wksp.m_size   = len;
   req->u.wksp.m_pgsize = 0;        // not used?

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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
struct AAL::aalui_WSMEvent     BufferAllocateTransaction::getWSIDEvent() const {return *(reinterpret_cast<struct AAL::aalui_WSMEvent*>(m_payload));}
AAL::uid_errnum_e              BufferAllocateTransaction::getErrno()const {return m_errno;};
void                           BufferAllocateTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

BufferAllocateTransaction::~BufferAllocateTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;     // FIXME: use C++ style casts
   delete afumsg;
}


BufferFreeTransaction::BufferFreeTransaction( btWSID wsid ) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferFree-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);



   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdWKSP_FREE;
   afumsg->size    = sizeof(struct ahm_req);

   // fill out ahm_req
   req->u.wksp.m_wsid   = wsid;
   req->u.wksp.m_size   = 0;        // not used
   req->u.wksp.m_pgsize = 0;        // not used?

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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
AAL::uid_errnum_e              BufferFreeTransaction::getErrno()const {return m_errno;};
void                           BufferFreeTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
BufferFreeTransaction::~BufferFreeTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


GetMMIOBufferTransaction::GetMMIOBufferTransaction() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);



   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_getMMIORmap;
   afumsg->size    = sizeof(struct ahm_req);

   req->u.wksp.m_wsid = WSID_MAP_MMIOR;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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
struct AAL::aalui_WSMEvent     GetMMIOBufferTransaction::getWSIDEvent() const {return *(reinterpret_cast<struct AAL::aalui_WSMEvent*>(m_payload));}
AAL::uid_errnum_e              GetMMIOBufferTransaction::getErrno()const {return m_errno;};
void                           GetMMIOBufferTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
GetMMIOBufferTransaction::~GetMMIOBufferTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

AFUQuiesceAndHalt::AFUQuiesceAndHalt() :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdPort_afuQuiesceAndHalt;
   afumsg->size    = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    AFUQuiesceAndHalt::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                AFUQuiesceAndHalt::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  AFUQuiesceAndHalt::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   AFUQuiesceAndHalt::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              AFUQuiesceAndHalt::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              AFUQuiesceAndHalt::getErrno()const {return m_errno;};
void                           AFUQuiesceAndHalt::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
AFUQuiesceAndHalt::~AFUQuiesceAndHalt() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

AFUEnable::AFUEnable() :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdPort_afuEnable;
   afumsg->size    = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    AFUEnable::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                AFUEnable::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  AFUEnable::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   AFUEnable::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              AFUEnable::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              AFUEnable::getErrno()const {return m_errno;};
void                           AFUEnable::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
AFUEnable::~AFUEnable() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


UmsgGetNumber::UmsgGetNumber() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferAllocate-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdGetNumUmsgs;
   afumsg->size    = sizeof(struct ahm_req);

   // fill out ahm_req
   req->u.mem_uv2id.mem_id = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    UmsgGetNumber::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                UmsgGetNumber::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  UmsgGetNumber::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   UmsgGetNumber::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              UmsgGetNumber::getMsgID()const {return m_msgID;}
AAL::btUnsignedInt             UmsgGetNumber::getNumber() const {return (reinterpret_cast<struct ahm_req *>(m_payload)->u.mem_uv2id.mem_id);}
AAL::uid_errnum_e              UmsgGetNumber::getErrno()const {return m_errno;};
void                           UmsgGetNumber::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

UmsgGetNumber::~UmsgGetNumber() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;     // FIXME: use C++ style casts
   delete afumsg;
}

UmsgGetBaseAddress::UmsgGetBaseAddress() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);



   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdGet_UmsgBase;
   afumsg->size    = sizeof(struct ahm_req);

   req->u.wksp.m_wsid = WSID_MAP_MMIOR;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    UmsgGetBaseAddress::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                UmsgGetBaseAddress::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  UmsgGetBaseAddress::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   UmsgGetBaseAddress::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              UmsgGetBaseAddress::getMsgID()const {return m_msgID;}
struct AAL::aalui_WSMEvent     UmsgGetBaseAddress::getWSIDEvent() const {return *(reinterpret_cast<struct AAL::aalui_WSMEvent*>(m_payload));}
AAL::uid_errnum_e              UmsgGetBaseAddress::getErrno()const {return m_errno;};
void                           UmsgGetBaseAddress::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
UmsgGetBaseAddress::~UmsgGetBaseAddress() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

UmsgSetAttributes::UmsgSetAttributes(AAL::NamedValueSet const &nvsArgs) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0)
{

   if( ENamedValuesOK != nvsArgs.Has(UMSG_HINT_MASK_KEY)){
      AAL_ERR( LM_All,"Missing Parameter or Key");
      return;
   }
   eBasicTypes nvsType;
   if(ENamedValuesOK !=  nvsArgs.Type(UMSG_HINT_MASK_KEY, &nvsType)){
      AAL_ERR( LM_All,"Unable to get key value type.");
      return;
   }

   if(btUnsigned64bitInt_t !=  nvsType){
      AAL_ERR( LM_All,"Bad value type.");
      return;
   }

   btUnsigned64bitInt  val;
   if(ENamedValuesOK !=  nvsArgs.Get(UMSG_HINT_MASK_KEY, &val)){
      AAL_ERR( LM_All,"Unable to get key value type.");
      return;
   }


   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdSetUmsgMode;
   afumsg->size    = sizeof(struct ahm_req);
   req->u.wksp.m_wsid = val;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){
      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    UmsgSetAttributes::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                UmsgSetAttributes::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  UmsgSetAttributes::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   UmsgSetAttributes::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              UmsgSetAttributes::getMsgID()const {return m_msgID;}
struct AAL::aalui_WSMEvent     UmsgSetAttributes::getWSIDEvent() const {return *(reinterpret_cast<struct AAL::aalui_WSMEvent*>(m_payload));}
AAL::uid_errnum_e              UmsgSetAttributes::getErrno()const {return m_errno;};
void                           UmsgSetAttributes::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
UmsgSetAttributes::~UmsgSetAttributes() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


