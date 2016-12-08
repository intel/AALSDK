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
//****************************************************************************
/// @file AIATransactions.cpp
/// @brief Transaction Objects represent Commands/Messages to be sent down
///        to the AFU object.
/// @ingroup uAIA
/// @verbatim
/// Accelerator Abstraction Layer
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

//=============================================================================
// Name:          BufferAllocateTransaction
// Description:   Send a Workspace Allocate operation to the Driver stack
// Input: devHandl - Device Handle received from Resource Manager
//        tranID   - Transaction ID
// Comments:
//=============================================================================
BufferAllocateTransaction::BufferAllocateTransaction( btWSSize len ) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   union msgpayload{
      struct ahm_req                req;    // [IN]
      struct AAL::aalui_WSMEvent    resp;   // [OUT]
   };

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferAllocate-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(union msgpayload );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
      setErrno(uid_errnumNoMem);
      return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdWKSP_ALLOC;
   afumsg->size    =  sizeof(union msgpayload );

   // fill out ahm_req
   req->u.wksp.m_wsid   = 0;        // not used?
   req->u.wksp.m_size   = len;
   req->u.wksp.m_pgsize = 0;        // not used?

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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

//=============================================================================
// Name:          BufferFreeTransaction
// Description:   Send a Workspace Free operation to the driver stack
// Input:         tranID   - Transaction ID
//                addr     - address of buffer to free
// Comments:
//=============================================================================
BufferFreeTransaction::BufferFreeTransaction( btWSID wsid ) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferFree-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

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

//=============================================================================
// Name:          GetMMIOBufferTransaction
// Description:   Send a Get MMIO Buffer operation to the Driver stack
// Input:         tranID   - Transaction ID
// Comments:
//=============================================================================
GetMMIOBufferTransaction::GetMMIOBufferTransaction() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   union msgpayload{
      struct ahm_req                req;    // [IN]
      struct AAL::aalui_WSMEvent    resp;   // [OUT]
   };

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(union msgpayload );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_getMMIORmap;
   afumsg->size    = sizeof(union msgpayload);

   req->u.wksp.m_wsid = WSID_MAP_MMIOR;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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



//=============================================================================
// Name:          AFUQuiesceAndHalt
// Description:   Quisce the AFU and put it into a halted state
// Input:         None
// Comments: This is an atomic transaction. Check error for result
//=============================================================================
AFUQuiesceAndHalt::AFUQuiesceAndHalt() :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
      setErrno(uid_errnumNoMem);
      return;
   }

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdPort_afuQuiesceAndHalt;
   afumsg->size    = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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

//=============================================================================
// Name:          AFUEnable
// Description:   Takes the AFU put of halted state
// Input:         None
// Comments: This is an atomic transaction. Check error for result
//=============================================================================
AFUEnable::AFUEnable() :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
      setErrno(uid_errnumNoMem);
      return;
   }

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdPort_afuEnable;
   afumsg->size    = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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

//=============================================================================
// Name:          UmsgGetNumber
// Description:   Get the number of uMSGs are available
// Input: devHandl - Device Handle received from Resource Manager
// Comments: Atomic
//=============================================================================
UmsgGetNumber::UmsgGetNumber() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferAllocate-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdGetNumUmsgs;
   afumsg->size    = sizeof(struct ahm_req);

   // fill out ahm_req
   req->u.mem_uv2id.mem_id = 0;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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

//=============================================================================
// Name:          UmsgGetBaseAddress
// Description:   Send a Get UMSG Buffer operation to the Driver stack
// Input:         tranID   - Transaction ID
// Comments:
//=============================================================================
UmsgGetBaseAddress::UmsgGetBaseAddress() :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{

   union msgpayload{
       struct ahm_req                req;    // [IN]
       struct AAL::aalui_WSMEvent    resp;   // [OUT]
    };

    // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
    // GetMMIOBufferTransaction-AIATransaction.
    m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(union msgpayload );

    // Allocate structs
    struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdGet_UmsgBase;
   afumsg->size    = sizeof(union msgpayload);

   req->u.wksp.m_wsid = WSID_MAP_MMIOR;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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


//=============================================================================
// Name:          UmsgSetAttributes
// Description:   Set the attibutes of the uMSG.
// Input: devHandl - Device Handle received from Resource Manager
// Comments: Atomic
//=============================================================================
UmsgSetAttributes::UmsgSetAttributes(AAL::NamedValueSet const &nvsArgs) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{

   if( true != nvsArgs.Has(UMSG_HINT_MASK_KEY)){
      AAL_ERR( LM_ALI,"Missing Parameter or Key"<< std::endl);
      return;
   }
   eBasicTypes nvsType;
   if(ENamedValuesOK !=  nvsArgs.Type(UMSG_HINT_MASK_KEY, &nvsType)){
      AAL_ERR( LM_ALI,"Unable to get key value type."<< std::endl);
      return;
   }

   if(btUnsigned64bitInt_t !=  nvsType){
      AAL_ERR( LM_ALI,"Bad value type."<< std::endl);
      return;
   }

   btUnsigned64bitInt  val;
   if(ENamedValuesOK !=  nvsArgs.Get(UMSG_HINT_MASK_KEY, &val)){
      AAL_ERR( LM_ALI,"Unable to get key value type."<< std::endl);
      return;
   }


   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // GetMMIOBufferTransaction-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_afucmdSetUmsgMode;
   afumsg->size    = sizeof(struct ahm_req);
   req->u.wksp.m_wsid = val;

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

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

//=============================================================================
// Name:          PerfCounterGet
// Description:   Get the Performance Countersk
// Input:         size   - TBD
// Comments:
//=============================================================================
PerfCounterGet::PerfCounterGet(btWSSize size) :
   m_msgID(reqid_UID_SendAFU),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload returns performance counters
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req ) + size ;

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_getPerfMonitor;
   afumsg->size    = sizeof(struct ahm_req) ;

// package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   m_bIsOK = true;
}

AAL::btBool                    PerfCounterGet::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                PerfCounterGet::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  PerfCounterGet::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   PerfCounterGet::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              PerfCounterGet::getMsgID()const {return m_msgID;}
AAL::btVirtAddr                PerfCounterGet::getBuffer() const {return m_payload;}
AAL::uid_errnum_e              PerfCounterGet::getErrno()const {return m_errno;};
void                           PerfCounterGet::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

PerfCounterGet::~PerfCounterGet() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


//=============================================================================
// Name:          AFUActivateTransaction
// Description:   PR object Transaction for activating the User AFU associated
//                with this PR.
// Comments: Causes the AAL Object associated with the uAFU to be exposed
//           and available for allocation. Note that the uAFU must be
//           already programmed.
//=============================================================================
AFUActivateTransaction::AFUActivateTransaction(AAL::TransactionID const &rTranID) :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(rTranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // BufferFree-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_activateAFU;
   afumsg->size    = 0;       // No Payload

   m_payload = (btVirtAddr)afumsg;


   m_bIsOK = true;
}

AAL::btBool                    AFUActivateTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                AFUActivateTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  AFUActivateTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   AFUActivateTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              AFUActivateTransaction::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              AFUActivateTransaction::getErrno()const {return m_errno;};
void                           AFUActivateTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
AFUActivateTransaction::~AFUActivateTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


//=============================================================================
// Name:          AFUDeactivateTransaction
// Description:   PR object Transaction for deactivating the User AFU associated
//                with this PR.
// Comments: Causes the AAL Object associated with the uAFU to be removed from
//           the objects  available for allocation.
// TODO: Initially a synchronous function. This function does NOT affect already
//       allocated Objects. I.e., If the AFU is in use it will remain in use
//       until released. It simply is not available for reallocation. If we
//       want to change the behavior so that it notifies owners and/or yanks
//       AFU away then it must become asynchronous.
//=============================================================================
AFUDeactivateTransaction::AFUDeactivateTransaction(AAL::TransactionID const &rTranID,
                                                   AAL::NamedValueSet const &rInputArgs):
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(rTranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   btTime             reconfTimeout = 0;
   btUnsigned64bitInt reconfAction  = 0;

   // DeActive Timeout
   if ( rInputArgs.Has(AALCONF_MILLI_TIMEOUT) ) {
      rInputArgs.Get(AALCONF_MILLI_TIMEOUT, &reconfTimeout);
   }

   // ReConfiguration Action Flags
   if( rInputArgs.Has(AALCONF_RECONF_ACTION) ) {

      rInputArgs.Get(AALCONF_RECONF_ACTION, &reconfAction);

      if( ( AALCONF_RECONF_ACTION_HONOR_REQUEST_ID != reconfAction ) &&
          ( AALCONF_RECONF_ACTION_HONOR_OWNER_ID   != reconfAction ) ) {
         m_bIsOK = false;
         return;
      }

   }

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in an
   // AFUDeactivate-AIATransaction.
   m_size = sizeof(struct aalui_CCIdrvMessage) + sizeof(struct ahm_req);

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if ( afumsg == NULL ) {
      setErrno(uid_errnumNoMem);
      return;
   }

   // Point at payload
   struct ahm_req *req = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   req->u.pr_config.reconfTimeout = reconfTimeout;
   req->u.pr_config.reconfAction  = reconfAction;

   // fill out aalui_CCIdrvMessage
   afumsg->cmd  = ccipdrv_deactivateAFU;
   afumsg->size = sizeof(struct ahm_req);

   // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   m_bIsOK = true;
}

AAL::btBool                    AFUDeactivateTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                AFUDeactivateTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  AFUDeactivateTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   AFUDeactivateTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              AFUDeactivateTransaction::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              AFUDeactivateTransaction::getErrno()const {return m_errno;};
void                           AFUDeactivateTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
AFUDeactivateTransaction::~AFUDeactivateTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


//=============================================================================
// Name:          AFUConfigureTransaction
// Description:   PR object Transaction for programming the User AFU associated
//                with this PR.
// Comments: Causes the AFU to be reprogrammed, deactivating if necessary.
//           A number of options are available to tell the system how to deal
//           with AFU's currently in use.
//=============================================================================
AFUConfigureTransaction::AFUConfigureTransaction(AAL::btVirtAddr pBuf,
                                                 AAL::btWSSize len,
                                                 AAL::TransactionID const &rTranID,
                                                 AAL::NamedValueSet const &rNVS) :
   m_msgID(reqid_UID_SendAFU),
   m_tid_t(rTranID),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{

   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload returns performance counters
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req );

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   btTime reconfTimeout = 0;

   // DeActive Timeout
   if(rNVS.Has(AALCONF_MILLI_TIMEOUT)){

      rNVS.Get(AALCONF_MILLI_TIMEOUT, &reconfTimeout);
   }

   // ReConfiguration Action Flags
   // Assume default action
   req->u.pr_config.reconfAction = ReConf_Action_Honor_request;
   if(rNVS.Has(AALCONF_RECONF_ACTION)){

      btUnsigned64bitInt reconfAction = 0;
      rNVS.Get(AALCONF_RECONF_ACTION, &reconfAction);

      if(reconfAction == AALCONF_RECONF_ACTION_HONOR_OWNER_ID){
         req->u.pr_config.reconfAction = ReConf_Action_Honor_Owner;
      }
   }

   // What state should the AFU be left in?
   if(rNVS.Has(AALCONF_REACTIVATE_DISABLED)){
      btBool bDisabled = true;   // Asssume if the key present it probably true
      rNVS.Get(AALCONF_REACTIVATE_DISABLED, &bDisabled);

      if(bDisabled == true){
         req->u.pr_config.reconfAction |= ReConf_Action_InActive;
         //printf("Creating disabled\n");
      }
   }

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = ccipdrv_configureAFU;
   afumsg->size    = sizeof(struct ahm_req) ;

   req->u.pr_config.vaddr  = pBuf;
   req->u.pr_config.size   = len;
   req->u.pr_config.reconfTimeout  = reconfTimeout;

// package in AIA transaction
   m_payload = reinterpret_cast<AAL::btVirtAddr>(afumsg);

   m_bIsOK = true;
}

AAL::btBool                    AFUConfigureTransaction::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                AFUConfigureTransaction::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  AFUConfigureTransaction::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   AFUConfigureTransaction::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              AFUConfigureTransaction::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              AFUConfigureTransaction::getErrno()const {return m_errno;};
void                           AFUConfigureTransaction::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}
AFUConfigureTransaction::~AFUConfigureTransaction() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}


//=============================================================================
// Name:          ErrorGet
// Description:   Get fpga gets error and error mask
// Input:         size   - size of ccip error structure.
// Input:         error  - 64bit ccip error csr
// Comments:
//=============================================================================
ErrorGet::ErrorGet(btWSSize size,btUnsigned64bitInt cmd) :
   m_msgID(reqid_UID_SendAFU),
   m_cmd(cmd),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload returns errors
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req ) + size ;

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = m_cmd;
   afumsg->size    = sizeof(struct ahm_req) ;

// package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    ErrorGet::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                ErrorGet::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  ErrorGet::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   ErrorGet::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              ErrorGet::getMsgID()const {return m_msgID;}
AAL::btVirtAddr                ErrorGet::getBuffer() const {return m_payload;}
AAL::uid_errnum_e              ErrorGet::getErrno()const {return m_errno;};
void                           ErrorGet::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

ErrorGet::~ErrorGet() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

//=============================================================================
// Name:          SetError
// Description:   Get fpga Sets error and error mask
// Input:         size   - size of ccip error structure.
// Input:         error  - 64bit ccip error csr
// Comments:
//=============================================================================
SetError::SetError(btUnsigned64bitInt cmd,struct CCIP_ERROR ccip_error ) :
   m_msgID(reqid_UID_SendAFU),
   m_cmd(cmd),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload sets error or mask in csr.
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req )  ;

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = m_cmd;
   afumsg->size    = sizeof(struct ahm_req) ;

   req->u.error_csr.error0          = ccip_error.error0;
   req->u.error_csr.pcie0_err       = ccip_error.pcie0_error;
   req->u.error_csr.pcie1_err       = ccip_error.pcie1_error;

   req->u.error_csr.first_error     = ccip_error.first_error;
   req->u.error_csr.next_error      = ccip_error.next_error;

   req->u.error_csr.ras_gerr        = ccip_error.ras_gerr;
   req->u.error_csr.ras_berror      = ccip_error.ras_berror;
   req->u.error_csr.ras_warnerror   = ccip_error.ras_warnerror;

  // package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   m_bIsOK = true;
}

AAL::btBool                    SetError::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                SetError::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  SetError::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   SetError::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              SetError::getMsgID()const {return m_msgID;}
AAL::btVirtAddr                SetError::getBuffer() const {return m_payload;}
AAL::uid_errnum_e              SetError::getErrno()const {return m_errno;};
void                           SetError::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

SetError::~SetError() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

//=============================================================================
// Name:          ThermalPwrGet
// Description:   Get fpga Thermal and Power values
// Input:         size   - size of ccip theraml power structure.
// Input:         cmd    - Transaction command
// Comments:
//=============================================================================
ThermalPwrGet::ThermalPwrGet(btWSSize size,btUnsigned64bitInt cmd) :
   m_msgID(reqid_UID_SendAFU),
   m_cmd(cmd),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_errno(uid_errnumOK)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload returns errors
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req ) + size ;

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   //check afumsg is non-NULL before using it
   ASSERT(NULL != afumsg);
   if (afumsg == NULL){
     setErrno(uid_errnumNoMem);
     return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = m_cmd;
   afumsg->size    = sizeof(struct ahm_req) ;

// package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   m_bIsOK = true;
}

AAL::btBool                    ThermalPwrGet::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                ThermalPwrGet::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  ThermalPwrGet::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   ThermalPwrGet::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              ThermalPwrGet::getMsgID()const {return m_msgID;}
AAL::btVirtAddr                ThermalPwrGet::getBuffer() const {return m_payload;}
AAL::uid_errnum_e              ThermalPwrGet::getErrno()const {return m_errno;};
void                           ThermalPwrGet::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

ThermalPwrGet::~ThermalPwrGet() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}

