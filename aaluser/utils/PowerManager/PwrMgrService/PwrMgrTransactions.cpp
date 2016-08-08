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
/// @file PwrMgr.cpp
/// @brief Transaction Objects represent Commands/Messages to be sent down
///        to the AFU object.
/// @ingroup PwrMger
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Ananda Ravuri, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/2016     AR       Initial version@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aalsdk/AALLoggerExtern.h"

#include "aalsdk/INTCDefs.h"
#include "aalsdk/kernel/ccipdriver.h"

#include "PwrMgrTransactions.h"
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
////////////      A L I   PERMGR   T R A N S A C T I O N S     ////////////////
/////////////////                                           ///////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:          PwrMgrResponse
// Description:   Response to pr power request
// Input:         cmd    - Transaction command
// Input:         pwr_status   -PR power status.
// Comments:
//============================================================================

PwrMgrResponse::PwrMgrResponse(AAL::stTransactionID_t transID,btUnsigned64bitInt cmd,btInt pr_PwrMgmtStatus) :
   m_msgID(reqid_UID_SendAFU),
   m_cmd(cmd),
   m_bIsOK(false),
   m_payload(NULL),
   m_size(0),
   m_bufLength(0),
   m_tid_t(transID)
{
   // We need to send an ahm_req within an aalui_CCIdrvMessage packaged in
   // must allocate enough memory , so playload returns errors
   //
   m_size = sizeof(struct aalui_CCIdrvMessage) +  sizeof(struct ahm_req ) ;

   // Allocate structs
   struct aalui_CCIdrvMessage *afumsg  = reinterpret_cast<struct aalui_CCIdrvMessage *>(new (std::nothrow) btByte[m_size]);

   ASSERT(NULL != afumsg);
   if(NULL == afumsg){
      return;
   }

   // Point at payload
   struct ahm_req *req                 = reinterpret_cast<struct ahm_req *>(afumsg->payload);

   // fill out aalui_CCIdrvMessage
   afumsg->cmd     = m_cmd;
   afumsg->size    = sizeof(struct ahm_req) ;

   req->u.pr_pwrmgmt.pr_pwrmgmt_status  = pr_PwrMgmtStatus;

// package in AIA transaction
   m_payload = (btVirtAddr) afumsg;

   ASSERT(NULL != m_payload);
   if(NULL == m_payload){      return;
   }

   m_bIsOK = true;
}

AAL::btBool                    PwrMgrResponse::IsOK() const {return m_bIsOK;}
AAL::btVirtAddr                PwrMgrResponse::getPayloadPtr()const {return m_payload;}
AAL::btWSSize                  PwrMgrResponse::getPayloadSize()const {return m_size;}
AAL::stTransactionID_t const   PwrMgrResponse::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              PwrMgrResponse::getMsgID()const {return m_msgID;}
AAL::btVirtAddr                PwrMgrResponse::getBuffer() const {return m_payload;}
AAL::uid_errnum_e              PwrMgrResponse::getErrno()const {return m_errno;};
void                           PwrMgrResponse::setErrno(AAL::uid_errnum_e errnum){m_errno = errnum;}

PwrMgrResponse::~PwrMgrResponse() {
   // unpack payload and free memory
   struct aalui_CCIdrvMessage *afumsg = (aalui_CCIdrvMessage *)m_payload;
   delete afumsg;
}
