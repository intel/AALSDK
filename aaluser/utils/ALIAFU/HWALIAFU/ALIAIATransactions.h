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
#include <aalsdk/uaia/IAFUProxy.h>

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/OSServiceModule.h>


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
class UAIA_API BufferAllocateTransaction : public IAIATransaction
{
public:
   BufferAllocateTransaction( AAL::btWSSize len );
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~BufferAllocateTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class BufferAllocateTransaction


//=============================================================================
// Name:          BufferFreeTransaction
// Description:   Send a Workspace Free operation to the driver stack
// Input:         tranID   - Transaction ID
//                addr     - address of buffer to free
// Comments:
//=============================================================================
class UAIA_API BufferFreeTransaction : public IAIATransaction
{
public:
   BufferFreeTransaction( AAL::btWSID wsid );
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~BufferFreeTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

};


//=============================================================================
// Name:          GetMMIOBufferTransaction
// Description:   Send a Get MMIO Buffer operation to the Driver stack
// Input:         tranID   - Transaction ID
// Comments:
//=============================================================================
class UAIA_API GetMMIOBufferTransaction : public IAIATransaction
{
public:
   GetMMIOBufferTransaction();
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~GetMMIOBufferTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class GetMMIOBufferTransaction


//=============================================================================
// Name:          AFUQuiesceAndHalt
// Description:   Quisce the AFU and put it into a halted state
// Input:         None
// Comments: This is an atomic transaction. Check error for result
//=============================================================================
class UAIA_API AFUQuiesceAndHalt : public IAIATransaction
{
public:
   AFUQuiesceAndHalt();
   AAL::btBool                    IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~AFUQuiesceAndHalt();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class AFUQuiesceAndHalt

//=============================================================================
// Name:          AFUEnable
// Description:   Takes the AFU put of halted state
// Input:         None
// Comments: This is an atomic transaction. Check error for result
//=============================================================================
class UAIA_API AFUEnable : public IAIATransaction
{
public:
   AFUEnable( );
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~AFUEnable();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class AFUEnable

//=============================================================================
// Name:          UmsgGetNumber
// Description:   Get the number of uMSGs are available
// Input: devHandl - Device Handle received from Resource Manager
// Comments: Atomic
//=============================================================================
class UAIA_API UmsgGetNumber : public IAIATransaction
{
public:
   UmsgGetNumber();
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   AAL::btUnsignedInt             getNumber() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~UmsgGetNumber();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class UmsgGetNumber

//=============================================================================
// Name:          UmsgGetBaseAddress
// Description:   Get the base address of the uMSG area.
// Input: devHandl - Device Handle received from Resource Manager
// Comments: Atomic
//=============================================================================
class UAIA_API UmsgGetBaseAddress : public IAIATransaction
{
public:
   UmsgGetBaseAddress();
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~UmsgGetBaseAddress();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class UmsgGetBaseAddress

//=============================================================================
// Name:          UmsgSetAttributes
// Description:   Set the attibutes of the uMSG.
// Input: devHandl - Device Handle received from Resource Manager
// Comments: Atomic
//=============================================================================
class UAIA_API UmsgSetAttributes : public IAIATransaction
{
public:
   UmsgSetAttributes(AAL::NamedValueSet const &);
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);


   ~UmsgSetAttributes();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class umsgSetAttributes

//=============================================================================
// Name:          PerfCounterGet
// Description:   Get the Performance Countersk
// Input:         size   - TBD
// Comments:
//=============================================================================
class UAIA_API PerfCounterGet : public IAIATransaction
{
public:
   PerfCounterGet(AAL::btWSSize size);
   AAL::btBool                IsOK() const;

   AAL::btVirtAddr                getPayloadPtr() const;
   AAL::btWSSize                  getPayloadSize() const;
   AAL::stTransactionID_t const   getTranID() const;
   AAL::uid_msgIDs_e              getMsgID() const;
   struct AAL::aalui_WSMEvent     getWSIDEvent() const;
   AAL::uid_errnum_e              getErrno()const;
   void                           setErrno(AAL::uid_errnum_e);
   AAL::btVirtAddr                getBuffer() const;

   ~PerfCounterGet();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

}; // class PerfCounterGet

//=============================================================================
// Name:          AFUActivateTransaction
// Description:   PR object Transaction for activating the User AFU associated
//                with this PR.
// Comments: Causes the AAL Object associated with the uAFU to be exposed
//           and available for allocation. Note that the uAFU must be
//           already programmed.
//=============================================================================
class UAIA_API AFUActivateTransaction : public IAIATransaction
{
public:
   AFUActivateTransaction(AAL::TransactionID const &rTranID);
   AAL::btBool                      IsOK() const;

   AAL::btVirtAddr                  getPayloadPtr() const;
   AAL::btWSSize                    getPayloadSize() const;
   AAL::stTransactionID_t const     getTranID() const;
   AAL::uid_msgIDs_e                getMsgID() const;
   AAL::uid_errnum_e                getErrno()const;
   void                             setErrno(AAL::uid_errnum_e);


   ~AFUActivateTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

};

//=============================================================================
// Name:          AFUDeactivateTransaction
// Description:   PR object Transaction for deactivating the User AFU associated
//                with this PR.
// Comments: Causes the AAL Object associated with the uAFU to be removed from
//           the objects  available for allocation.
//=============================================================================
class UAIA_API AFUDeactivateTransaction : public IAIATransaction
{
public:
   AFUDeactivateTransaction(AAL::TransactionID const &rTranID);
   AAL::btBool                      IsOK() const;

   AAL::btVirtAddr                  getPayloadPtr() const;
   AAL::btWSSize                    getPayloadSize() const;
   AAL::stTransactionID_t const     getTranID() const;
   AAL::uid_msgIDs_e                getMsgID() const;
   AAL::uid_errnum_e                getErrno()const;
   void                             setErrno(AAL::uid_errnum_e);


   ~AFUDeactivateTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

};

//=============================================================================
// Name:          AFUConfigureTransaction
// Description:   PR object Transaction for programming the User AFU associated
//                with this PR.
// Comments: Causes the AFU to be reprogrammed, deactivating if necessary.
//           A number of options are available to tell the system how to deal
//           with AFU's currently in use.
//=============================================================================
class UAIA_API AFUConfigureTransaction : public IAIATransaction
{
public:
   AFUConfigureTransaction(AAL::btVirtAddr pBuf,
                           AAL::btWSSize len,
                           AAL::TransactionID const &rTranID,
                           AAL::NamedValueSet const &rNVS = AAL::NamedValueSet());
   AAL::btBool                      IsOK() const;

   AAL::btVirtAddr                  getPayloadPtr() const;
   AAL::btWSSize                    getPayloadSize() const;
   AAL::stTransactionID_t const     getTranID() const;
   AAL::uid_msgIDs_e                getMsgID() const;
   AAL::uid_errnum_e                getErrno()const;
   void                             setErrno(AAL::uid_errnum_e);


   ~AFUConfigureTransaction();

private:
   AAL::uid_msgIDs_e             m_msgID;
   AAL::stTransactionID_t        m_tid_t;
   AAL::btBool                   m_bIsOK;
   AAL::btVirtAddr               m_payload;
   AAL::btWSSize                 m_size;
   AAL::btWSSize                 m_bufLength;
   AAL::uid_errnum_e             m_errno;

};

#endif // __AALSDK_AIATRANSACTIONS_H__

