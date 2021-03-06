// Copyright(c) 2007-2016, Intel Corporation
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
/// @file AFUPkg.h
/// @brief Definitions for AFU Packages
/// @ingroup AFU
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/26/2007     JG       Initial version started
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Comments & License
/// 05/27/2008     HM       Changed last parameter of
///                            CAFUFreeWorkSpaceExceptionTransactionEvent
///                            to btcString from btString - make compiler happy
/// 01/04/2009     HM       Updated Copyright
/// 06/07/2012     JG       Removed AFU namespace@endverbatim
//****************************************************************************
#ifndef __AALSDK_AFUPKG_H__
#define __AALSDK_AFUPKG_H__
#include <aalsdk/AFU.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/CAALBase.h>
#include <aalsdk/CAALEvent.h>

BEGIN_NAMESPACE(AAL)

   class CAASServiceContainer;


   //=============================================================================
   // Events
   //=============================================================================

/// @addtogroup SysEvents
/// @{

//=============================================================================
// Name: CAFUAllocateWorkSpaceTransactionEvent
// Description: CAFUFreeWorkSpaceTransactionEvent
// IID: tranevtAFU_WorkSpaceAllocate
//=============================================================================
class UAIA_API CAFUAllocateWorkSpaceTransactionEvent : public CTransactionEvent,
                                                       public IAFUAllocateWorkSpaceTransactionEvent
{
public:
   /// @brief CAFUAllocateWorkSpaceTransactionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] WorkSpaceID      The Workspace ID.
   /// @param[in] WorkSpaceAddress The virtual address of the Workspace.
   /// @param[in] WorkSpaceSize    The size of the Workspace.
   /// @param[in] TransID          A reference to the Transaction ID.
   /// @return void
   CAFUAllocateWorkSpaceTransactionEvent(IBase               *pObject,
                                         btWSID               WorkSpaceID,
                                         btVirtAddr           WorkSpaceAddress,
                                         btWSSize             WorkSpaceSize,
                                         TransactionID const &TransID);
   virtual ~CAFUAllocateWorkSpaceTransactionEvent();

   /// @brief Get the Workspace ID from the Event.
   /// @return The Workspace ID.
   btWSID     WorkSpaceID()      { return m_WorkSpaceID;      }
   /// @brief Get the virtual address of the Workspace from the Event.
   /// @return The virtual address of the Workspace ID.
   btVirtAddr WorkSpaceAddress() { return m_WorkSpaceAddress; }
   /// @brief Get the size of the Workspace from the Event.
   /// @return The size of the Workspace ID.
   btWSSize   WorkSpaceSize()    { return m_WorkSpaceSize;    }

protected:
   btWSID     m_WorkSpaceID;
   btVirtAddr m_WorkSpaceAddress;
   btWSSize   m_WorkSpaceSize;
};


//=============================================================================
// Name: CAFUAllocateWorkSpaceExceptionTransactionEvent
// Description: CAFUAllocateWorkSpaceExceptionTransactionEvent
// IID: extranevtAFU_WorkSpaceAllocate
//=============================================================================
class UAIA_API CAFUAllocateWorkSpaceExceptionTransactionEvent : public CExceptionTransactionEvent
{
public:
   /// @brief CAFUAllocateWorkSpaceExceptionTransactionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] TranID           A reference to the Transaction ID.
   /// @param[in] ExceptionNumber  The number of the exception that occurred.
   /// @param[in] Reason           The reason code.
   /// @param[in] Description      The string describing the exception.
   /// @return void
   CAFUAllocateWorkSpaceExceptionTransactionEvent(IBase               *pObject,
                                                  TransactionID const &TranID,
                                                  btID                 ExceptionNumber,
                                                  btID                 Reason,
                                                  btcString            Description);

   virtual ~CAFUAllocateWorkSpaceExceptionTransactionEvent();
};

//=============================================================================
// Name: CAFUFreeWorkSpaceTransactionEvent
// Description: CAFUFreeWorkSpaceTransactionEvent
// IID: tranevtAFU_WorkSpaceFree
//=============================================================================
class UAIA_API CAFUFreeWorkSpaceTransactionEvent : public CTransactionEvent, public IAFUFreeWorkSpaceTransactionEvent
{
public:
   /// @brief CAFUFreeWorkSpaceTransactionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] WorkSpaceID      A reference to the Transaction ID.
   /// @param[in] TransID          A reference to the Transaction ID.
   /// @return void
   CAFUFreeWorkSpaceTransactionEvent(IBase               *pObject,
                                     btWSID               WorkSpaceID,
                                     TransactionID const &TransID);
   virtual ~CAFUFreeWorkSpaceTransactionEvent();
   /// @brief Get the Workspace ID from the Event.
   /// @return The Workspace ID.
   btWSID WorkSpaceID() { return m_WorkSpaceID; }

protected:
   btWSID m_WorkSpaceID;
};

//=============================================================================
// Name: CAFUFreeWorkSpaceExceptionTransactionEvent
// Description: CAFUFreeWorkSpaceExceptionTransactionEvent
// IID: extranevtAFU_WorkSpaceFree
//=============================================================================
class UAIA_API CAFUFreeWorkSpaceExceptionTransactionEvent : public CExceptionTransactionEvent
{
public:
   /// @brief CAFUFreeWorkSpaceExceptionTransactionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] TranID           A reference to the Transaction ID.
   /// @param[in] ExceptionNumber  The number of the exception that occurred.
   /// @param[in] Reason           The reason code.
   /// @param[in] Description      The string describing the exception.
   /// @return void
   CAFUFreeWorkSpaceExceptionTransactionEvent(IBase               *pObject,
                                              TransactionID const &TranID,
                                              btID                 ExceptionNumber,
                                              btID                 Reason,
                                              btcString            Description);

   virtual ~CAFUFreeWorkSpaceExceptionTransactionEvent();
};


   //=============================================================================
   // Name: CAFUProcessMessageTransactionEvent
   /// @brief Workspace exception Event
   //=============================================================================
class UAIA_API CAFUProcessMessageTransactionEvent : public CTransactionEvent
{
public:
   /// @brief CAFUProcessMessageTransactionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] TranID           The Transaction ID.
   /// @return void
   CAFUProcessMessageTransactionEvent(IBase        *pObject,
                                      TransactionID TranID);

   virtual ~CAFUProcessMessageTransactionEvent();

protected:
   CAFUProcessMessageTransactionEvent();
   CAFUProcessMessageTransactionEvent(IBase * );
};


//=============================================================================
// Name: CAFUProcessMessageTransactionExceptionEvent
/// @brief AFU Workspace Event
//=============================================================================
class UAIA_API CAFUProcessMessageTransactionExceptionEvent : public CExceptionTransactionEvent
{
public:
   /// @brief CAFUProcessMessageTransactionExceptionEvent Constructor.
   ///
   /// @param[in] pObject          A pointer to an IBase interface.
   /// @param[in] TranID           The Transaction ID.
   /// @param[in] ExceptionNumber  The number of the exception that occurred.
   /// @param[in] Reason           The reason code.
   /// @param[in] Description      The string describing the exception.
   /// @return void
   CAFUProcessMessageTransactionExceptionEvent(IBase        *pObject,
                                               TransactionID TranID,
                                               btID          ExceptionNumber,
                                               btID          Reason,
                                               btString      Description);

   virtual ~CAFUProcessMessageTransactionExceptionEvent();

protected:
   CAFUProcessMessageTransactionExceptionEvent();
   CAFUProcessMessageTransactionExceptionEvent(IBase * );
};

/// @}


//=============================================================================
// Name: IAFUFactory
/// @brief Interface to AFUPackage's AFU Factory
//=============================================================================
class UAIA_API IAFUFactory
{
public:
   virtual ~IAFUFactory();

   /// @brief Create a Service.
   ///
   /// @param[in] nvsManifest      A reference to the NVS defining the Service.
   /// @param[in] theEventHandler  The Service Event Handler.
   /// @param[in] TranID           A reference to the Transaction ID.
   /// @param[in] AppContext       The application supplied context.
   /// @return void
   virtual void  Create(NamedValueSet const &nvsManifest,
                        btEventHandler       theEventHandler,
                        TransactionID const &TranID,
                        btApplicationContext AppContext = NULL) = 0;

   /// @brief Mutator for setting the Service Container
   ///
   /// @param[in] pServiceContainer  A pointer to the Service Container.
   /// @return void
   virtual void ServiceContainer(CAASServiceContainer *pServiceContainer) = 0;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_AFUPKG_H__

