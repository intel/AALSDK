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
#include "aalsdk/kernel/ccipdriver.h"

#include "AIATransactions.h"

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__


#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      A I A   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BindAFUDevice::BindAFUDevice( TransactionID const &tranID ) :
   m_msgID(reqid_UID_Bind),
   m_tid_t(tranID),
   m_bIsOK(true)
{}

AAL::btVirtAddr                BindAFUDevice::getPayloadPtr()const {return NULL;}
AAL::btWSSize                  BindAFUDevice::getPayloadSize()const {return 0;}
AAL::stTransactionID_t const   BindAFUDevice::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              BindAFUDevice::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              BindAFUDevice::getErrno()const {return m_errno;}
void                           BindAFUDevice::setErrno(AAL::uid_errnum_e errnum) {m_errno = errnum;}

UnBindAFUDevice::UnBindAFUDevice( TransactionID const &tranID ) :
   m_msgID(reqid_UID_UnBind),
   m_tid_t(tranID),
   m_bIsOK(true)
{}

AAL::btVirtAddr                UnBindAFUDevice::getPayloadPtr()const {return NULL;}
AAL::btWSSize                  UnBindAFUDevice::getPayloadSize()const {return 0;}
AAL::stTransactionID_t const   UnBindAFUDevice::getTranID()const {return m_tid_t;}
AAL::uid_msgIDs_e              UnBindAFUDevice::getMsgID()const {return m_msgID;}
AAL::uid_errnum_e              UnBindAFUDevice::getErrno()const {return m_errno;};
void                           UnBindAFUDevice::setErrno(AAL::uid_errnum_e errnum) {m_errno = errnum;}
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
AAL::uid_errnum_e              ShutdownMDT::getErrno()const {return m_errno;};
void                           ShutdownMDT::setErrno(AAL::uid_errnum_e errnum) {m_errno = errnum;}

END_NAMESPACE(AAL)


