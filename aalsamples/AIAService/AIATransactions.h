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
/// @file AIATransactions.h
/// @brief Transaction Objects represent Commands/Messages to be sent down
///        to the AFU object. This file defines AIA specific Transaction
///        objects.
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
#ifndef __AALSDK_AIATRANSACTIONS_H__
#define __AALSDK_AIATRANSACTIONS_H__
#include <aalsdk/kernel/aalids.h>
#include <aalsdk/kernel/ccipdriver.h>

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/OSServiceModule.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      A I A   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:          BindAFUDevice
// Description:   Send a Bind operation to the Driver stack
// Input: devHandl - Device Handle received from Resource Manager
//        tranID   - Transaction ID
// Comments:
//=============================================================================
class UAIA_API BindAFUDevice : public IAIATransaction
{
public:
   // Used by the AFU Proxy to bind to AFU device
   BindAFUDevice( AAL::TransactionID const &tranID = AAL::TransactionID() );

   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;

}; // class BindAFUDevice

//=============================================================================
// Name:          UnBindAFUDevice
// Description:   Send a UnBind operation to the Driver stack
// Input: devHandl - Device Handle received from Resource Manager
//        tranID   - Transaction ID
// Comments:
//=============================================================================
class UAIA_API UnBindAFUDevice : public IAIATransaction
{
public:
   // Used by the AFU Proxy to bind to AFU device
   UnBindAFUDevice( AAL::TransactionID const &tranID = AAL::TransactionID() );

   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;

}; // class BindAFUDevice

//=============================================================================
// Name:          ShutdownMDT
// Description:   Send a shutdown message to UIDrv to kill MDT
// Comments:
//=============================================================================
class UAIA_API ShutdownMDT : public IAIATransaction
{
public:
   // Used by the AFU Proxy to bind to AFU device
   ShutdownMDT(AAL::TransactionID const &tranID = AAL::TransactionID(), AAL::btTime timeout = 0);

   AAL::btBool                    IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btTime                   m_timeout;

}; // class BindAFUDevice

#endif // __AALSDK_AIATRANSACTIONS_H__

