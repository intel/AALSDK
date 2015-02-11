// Copyright (c) 2014-2015, Intel Corporation
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
//        FILE: XLRuntimeMessages.h
//     CREATED: Mar 27, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for XL Runtime Messages and Dispatchables
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __XLRUNTIMEMESSAGES_H__
#define __XLRUNTIMEMESSAGES_H__
#include <aalsdk/osal/IDispatchable.h>
#include <aalsdk/IServiceClient.h>

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: ServiceClientMessage
// Description: Dispatchable for generating ServiceClient callbacks
// Comments: Use ThreadGroup or XL MDS to dispatch
//=============================================================================
class ServiceClientMessage : public IDispatchable
{
public:
   enum ServiceClientMessageType{
//      Allocated,
      Freed,
      Event
   };
   ServiceClientMessage( IServiceClient               *po,
                         IBase                        *pServiceBase,
                         enum ServiceClientMessageType type,
                         const IEvent                 *pEvent)
   : m_pobject(po),
     m_pServiceBase(pServiceBase),
     m_type(type),
     m_pEvent(pEvent){}

   ServiceClientMessage( IServiceClient               *po,
                         IBase                        *pServiceBase,
                         enum ServiceClientMessageType type,
                         TransactionID const          &rTranID = TransactionID() )
   : m_pobject(po),
     m_pServiceBase(pServiceBase),
     m_type(type),
     m_pEvent(NULL),
     m_rTranID(rTranID){}

   void  operator()()
   {
      switch(m_type)
      {
//         case Allocated:
//            m_pobject->serviceAllocated(m_pServiceBase, m_rTranID);
//            break;
         case Freed:
            m_pobject->serviceFreed(m_rTranID);
            break;
         case Event:
            m_pobject->serviceEvent(*m_pEvent);
            break;
         default:
            break;
      };

      // Delete the event if there is one
      if(NULL != m_pEvent) delete m_pEvent;

      // Clean up self
      delete this;
   }

virtual ~ServiceClientMessage(){}

protected:
   IServiceClient                *m_pobject;
   IBase                         *m_pServiceBase;
   enum ServiceClientMessageType  m_type;
   TransactionID const            m_rTranID;
   IEvent const                  *m_pEvent;
};


END_NAMESPACE(AAL)

#endif /* XLRUNTIMEMESSAGES_H_ */

