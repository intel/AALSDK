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
/// @file PwrMgrTransactions.h
/// @brief Transaction Objects represent Commands/Messages to be sent down
///        to the AFU object. This file defines AIA specific Transaction
///        objects.
/// @ingroup uAIA
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Ananda Ravuri, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/201      AR       Initial version@endverbatim
//****************************************************************************
#ifndef __AALSDK_PWRMGRTRANSACTIONS_H__
#define __AALSDK_PWRMGRTRANSACTIONS_H__
#include <aalsdk/kernel/aalids.h>
#include <aalsdk/uaia/IAFUProxy.h>

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/OSServiceModule.h>


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////                                           ///////////////////
////////////      A L I   PWRMGR   T R A N S A C T I O N S     ////////////////
/////////////////                                           ///////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:          PwrMgrResponse
// Description:   Response to pr power request
// Input:         cmd    - Transaction command
// Input:         pr_PwrMgmtStatus   -PR power management  status.
// Comments:
//=============================================================================
class UAIA_API PwrMgrResponse : public IAIATransaction
{
public:
   PwrMgrResponse(AAL::stTransactionID_t transID,AAL::btUnsigned64bitInt cmd,AAL::btInt pr_PwrMgmtStatus);
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);
   AAL::btVirtAddr                getBuffer() const;

   ~PwrMgrResponse();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;
   AAL::btUnsigned64bitInt       m_cmd;

}; // class PwrMgrResponse

#endif // __AALSDK_PWRMGRTRANSACTIONS_H__

