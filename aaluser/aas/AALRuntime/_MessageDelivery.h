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
/// @file _MessageDelivery.h
/// @brief Definitions for the AAL Runtime internal default Message Delivery facility.
/// @ingroup MDS
///
/// The Message Delivery is designed as a pluggable AAL Service even
/// though it is a built-in. This makes for a more consistent model
/// and provides for some desirable functionality like IBase.
///
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 03/13/2014     JG       Initial version@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALRUNTIME__MESSAGEDELIVERY_H__
#define __AALSDK_AALRUNTIME__MESSAGEDELIVERY_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/eds/AASEventDeliveryService.h>
#include <aalsdk/osal/ThreadGroup.h>

/// @addtogroup MDS
/// @{

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: _MessageDelivery
// Description: Default message delivery facility
// Interface: IEventDeliveryService
// Comments:  This object is operational and meets its minimum functional
//            requirements pior to init()
//=============================================================================
class _MessageDelivery : public CAASBase,
                         public IMessageDeliveryService
{
public:
   _MessageDelivery();
   ~_MessageDelivery();

   // <IMessageDeliveryService>
   virtual void StartMessageDelivery();
   virtual void  StopMessageDelivery();
   virtual btBool    scheduleMessage(IDispatchable * );
   // </IMessageDeliveryService>

#if DEPRECATED
   EDS_Status Schedule();
#endif // DEPRECATED

protected:
   OSLThreadGroup m_Dispatcher;
};

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_AALRUNTIME__MESSAGEDELIVERY_H__
