// Copyright (c) 2014, Intel Corporation
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
/// @file ISPLClient.h
/// @brief ISPLClient definition.
/// @ingroup ISPLAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/21/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_ISPLCLIENT_H__
#define __AALSDK_SERVICE_ISPLCLIENT_H__
#include <aalsdk/service/ICCIClient.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup ISPLAFU
/// @{

/// SPL Client interface.
/// @brief Defines the notification interface for clients of SPL AFU's.
class ISPLClient : public ICCIClient
{
public:
   virtual ~ISPLClient() {}

   /// @brief Notification callback for SPL transaction started success.
   ///
   /// Sent in response to a successful transaction start request (ISPLAFU::StartTransactionContext).
   ///
   /// @param[in]  TranID      The transaction ID provided in the call to ISPLAFU::StartTransactionContext.
   /// @param[in]  AFUDSM      The user virtual address of the AFU Device Status Memory.
   /// @param[in]  AFUDSMSize  The size in bytes of the AFU Device Status Memory.
   virtual void  OnTransactionStarted(TransactionID const &TranID,
                                      btVirtAddr           AFUDSM,
                                      btWSSize             AFUDSMSize) = 0;

   /// @brief Notification callback for SPL context workspace set success.
   ///
   /// Sent in response to a successful transaction start request (ISPLAFU::SetContextWorkspace).
   ///
   /// @param[in]  TranID  The transaction ID provided in the call to ISPLAFU::SetContextWorkspace.
   virtual void OnContextWorkspaceSet(TransactionID const &TranID)     = 0;

   /// @brief Notification callback for SPL transaction failed.
   ///
   /// Sent in response to a failed transaction start request (ISPLAFU::StartTransactionContext, ISPLAFU::SetContextWorkspace).
   ///
   /// @param[in]  Event  An IExceptionTransactionEvent describing the failure.
   virtual void   OnTransactionFailed(const IEvent &Event)             = 0;

   /// @brief Notification callback for SPL transaction complete.
   ///
   /// Sent when an SPL transaction previously started by ISPLAFU::StartTransactionContext or
   /// ISPLAFU::SetContextWorkspace has run to completion.
   ///
   /// @param[in]  TranID  The transaction ID provided in the call to ISPLAFU::StartTransactionWorkspace
   ///                     or ISPLAFU::SetContextWorkspace.
   virtual void OnTransactionComplete(TransactionID const &TranID)     = 0;

   /// @brief Notification callback for SPL transaction force stop.
   ///
   /// Sent in response to ISPLAFU::StopTransactionContext. The active transaction, if any, at the
   /// time of the call to ISPLAFU::StopTransactionContext may be safely assumed to have stopped
   /// upon receiving this notification.
   virtual void OnTransactionStopped(TransactionID const &TranID)      = 0;
};

#define iidSPLClient __INTC_IID(INTC_sysSampleAFU, 0x000b)

/// Notification functor for transaction started success.
class SPLClientTransactionStarted : public IDispatchable
{
public:
   SPLClientTransactionStarted(ISPLClient   *pRecipient,
                               TransactionID TranID,
                               btVirtAddr    AFUDSM,
                               btWSSize      AFUDSMSize) :
      m_pRecipient(pRecipient),
      m_TranID(TranID),
      m_AFUDSM(AFUDSM),
      m_AFUDSMSize(AFUDSMSize)
   {}

   virtual void operator() ()
   {
      m_pRecipient->OnTransactionStarted(m_TranID, m_AFUDSM, m_AFUDSMSize);
      delete this;
   }

protected:
   ISPLClient   *m_pRecipient;
   TransactionID m_TranID;
   btVirtAddr    m_AFUDSM;
   btWSSize      m_AFUDSMSize;
};

/// Notification functor for AFU Context workspace set success.
class SPLClientContextWorkspaceSet : public IDispatchable
{
public:
   SPLClientContextWorkspaceSet(ISPLClient   *pRecipient,
                                TransactionID TranID) :
      m_pRecipient(pRecipient),
      m_TranID(TranID)
   {}

   virtual void operator() ()
   {
      m_pRecipient->OnContextWorkspaceSet(m_TranID);
      delete this;
   }

protected:
   ISPLClient   *m_pRecipient;
   TransactionID m_TranID;
};

/// Notification functor for transaction failure.
class SPLClientTransactionFailed : public IDispatchable
{
public:
   SPLClientTransactionFailed(ISPLClient *pRecipient,
                              IEvent     *pExcept) :
      m_pRecipient(pRecipient),
      m_pExcept(pExcept)
   {
      ASSERT(NULL != m_pRecipient);
      ASSERT(NULL != m_pExcept);
   }
   ~SPLClientTransactionFailed()
   {
      if ( NULL != m_pExcept ) {
         delete m_pExcept;
      }
   }

   virtual void operator() ()
   {
      m_pRecipient->OnTransactionFailed(*m_pExcept);
      delete this;
   }

protected:
   ISPLClient *m_pRecipient;
   IEvent     *m_pExcept;
};

/// Notification functor for transaction complete success.
class SPLClientTransactionComplete : public IDispatchable
{
public:
   SPLClientTransactionComplete(ISPLClient   *pRecipient,
                                TransactionID TranID) :
      m_pRecipient(pRecipient),
      m_TranID(TranID)
   {}

   virtual void operator() ()
   {
      m_pRecipient->OnTransactionComplete(m_TranID);
      delete this;
   }

protected:
   ISPLClient   *m_pRecipient;
   TransactionID m_TranID;
};

/// Notification functor for transaction stopped.
class SPLClientTransactionStopped : public IDispatchable
{
public:
   SPLClientTransactionStopped(ISPLClient   *pRecipient,
                               TransactionID TranID) :
      m_pRecipient(pRecipient),
      m_TranID(TranID)
   {}

   virtual void operator() ()
   {
      m_pRecipient->OnTransactionStopped(m_TranID);
      delete this;
   }

protected:
   ISPLClient   *m_pRecipient;
   TransactionID m_TranID;
};

/// @} group ISPLAFU

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_ISPLCLIENT_H__

