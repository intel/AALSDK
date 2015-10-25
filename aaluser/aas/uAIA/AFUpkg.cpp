// Copyright (c) 2007-2015, Intel Corporation
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
/// @file AFUpkg.cpp
/// @brief Universal Application Interface Adaptor.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Tim Whisonant, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/24/2013     TSW      Place AFU.h and AFUpkg.h implementations here.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AFU.h"
#include "aalsdk/AFUpkg.h"


BEGIN_NAMESPACE(AAL)


IAFUAllocateWorkSpaceTransactionEvent::~IAFUAllocateWorkSpaceTransactionEvent() {}
IAFUFreeWorkSpaceTransactionEvent::~IAFUFreeWorkSpaceTransactionEvent() {}
IWorkspace::~IWorkspace() {}

CAFUAllocateWorkSpaceTransactionEvent::CAFUAllocateWorkSpaceTransactionEvent(IBase               *pObject,
                                                                             btWSID               WorkSpaceID,
                                                                             btVirtAddr           WorkSpaceAddress,
                                                                             btWSSize             WorkSpaceSize,
                                                                             TransactionID const &TransID) :
   CTransactionEvent(pObject, TransID),
   m_WorkSpaceID(WorkSpaceID),
   m_WorkSpaceAddress(WorkSpaceAddress),
   m_WorkSpaceSize(WorkSpaceSize)
{
   SetInterface(tranevtAFU_WorkSpaceAllocate,
                        dynamic_cast<IAFUAllocateWorkSpaceTransactionEvent *>(this));
}

CAFUAllocateWorkSpaceTransactionEvent::~CAFUAllocateWorkSpaceTransactionEvent() {}

CAFUAllocateWorkSpaceExceptionTransactionEvent::CAFUAllocateWorkSpaceExceptionTransactionEvent(IBase               *pObject,
                                                                                               TransactionID const &TranID,
                                                                                               btID                 ExceptionNumber,
                                                                                               btID                 Reason,
                                                                                               btcString            Description) :
   CExceptionTransactionEvent(pObject, TranID, ExceptionNumber, Reason, Description)
{
   m_SubClassID = extranevtAFU_WorkSpaceAllocate;
}

CAFUAllocateWorkSpaceExceptionTransactionEvent::~CAFUAllocateWorkSpaceExceptionTransactionEvent() {}

CAFUFreeWorkSpaceTransactionEvent::CAFUFreeWorkSpaceTransactionEvent(IBase               *pObject,
                                                                     btWSID               WorkSpaceID,
                                                                     TransactionID const &TransID) :
   CTransactionEvent(pObject, TransID),
   m_WorkSpaceID(WorkSpaceID)
{
   SetInterface(tranevtAFU_WorkSpaceFree,
                        dynamic_cast<IAFUFreeWorkSpaceTransactionEvent *>(this));
}

CAFUFreeWorkSpaceTransactionEvent::~CAFUFreeWorkSpaceTransactionEvent() {}

CAFUFreeWorkSpaceExceptionTransactionEvent::CAFUFreeWorkSpaceExceptionTransactionEvent(IBase               *pObject,
                                                                                       TransactionID const &TranID,
                                                                                       btID                 ExceptionNumber,
                                                                                       btID                 Reason,
                                                                                       btcString            Description) :
   CExceptionTransactionEvent(pObject, TranID, ExceptionNumber, Reason, Description)
{
   m_SubClassID = extranevtAFU_WorkSpaceFree;
}

CAFUFreeWorkSpaceExceptionTransactionEvent::~CAFUFreeWorkSpaceExceptionTransactionEvent() {}

CAFUProcessMessageTransactionEvent::CAFUProcessMessageTransactionEvent(IBase        *pObject,
                                                                       TransactionID TranID) :
   CTransactionEvent(pObject, TranID)
{
   m_SubClassID = tranevtAFU_ProcessMessage;
}

CAFUProcessMessageTransactionEvent::~CAFUProcessMessageTransactionEvent() {}
CAFUProcessMessageTransactionEvent::CAFUProcessMessageTransactionEvent() {/*empty*/}
CAFUProcessMessageTransactionEvent::CAFUProcessMessageTransactionEvent(IBase * ) {/*empty*/}

CAFUProcessMessageTransactionExceptionEvent::CAFUProcessMessageTransactionExceptionEvent(IBase        *pObject,
                                                                                         TransactionID TranID,
                                                                                         btID          ExceptionNumber,
                                                                                         btID          Reason,
                                                                                         btString      Description) :
   CExceptionTransactionEvent(pObject, TranID, ExceptionNumber, Reason, Description)
{
   m_SubClassID = extranevtAFU_ProcessMessage;
}

CAFUProcessMessageTransactionExceptionEvent::~CAFUProcessMessageTransactionExceptionEvent() {}
CAFUProcessMessageTransactionExceptionEvent::CAFUProcessMessageTransactionExceptionEvent() {/*empty*/}
CAFUProcessMessageTransactionExceptionEvent::CAFUProcessMessageTransactionExceptionEvent(IBase * ) {/*empty*/}

IAFUFactory::~IAFUFactory() {}


END_NAMESPACE(AAL)


