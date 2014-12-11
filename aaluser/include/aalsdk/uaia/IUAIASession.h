// Copyright (c) 2007-2014, Intel Corporation
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
/// @file IUAIASession.h
/// @brief Defines Session interface for the Universal AIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/29/2009     JG       Added accessors for message route and Event Handler@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_IUAIASESSION_H__
#define __AALSDK_UAIA_IUAIASESSION_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/uaia/uidrvMessage.h>           // uidrvMessageRoute
#include <aalsdk/uaia/AIA.h>                    // IManagement
#include <aalsdk/eds/AASEventDeliveryService.h> // IEventDispatcher

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)

class CAIA;

//=============================================================================
// Name: IuAIASession
// Description: Defines the AIA session interface.
// Comments:
//=============================================================================
class UAIA_API IuAIASession : public IManagement
{
public:
   virtual ~IuAIASession();
   virtual btBool IsOK()                                 const = 0;
   // Upstream channel toward owner
   virtual uidrvMessageRoute const & OwnerMessageRoute() const = 0;
   // Event dispatcher to use for upstream route
   virtual void QueueAASEvent(btEventHandler Eventhandler, CAALEvent *pEvent) = 0;
   // Downstream virtual AIA session
   virtual CAIA & ruAIA()                                      = 0;
};


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_UAIA_IUAIASESSION_H__

