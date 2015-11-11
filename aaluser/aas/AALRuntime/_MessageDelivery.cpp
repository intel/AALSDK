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
/// @file _MessageDelivery.cpp
/// @brief Definitions for the AAL Runtime internal default Message Delivery facility.
/// @ingroup MDS
///
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 03/13/2014     JG       Initial version
/// 06/25/2015     JG       Removed RT from name
/// 07/01/02015    JG       Removed the Service attributes and made it into
///                         normal IBase object. This simplified boot and
///                         cleanup endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"
#include "aalsdk/aas/AALRuntimeModule.h"
#include "aalsdk/osal/OSServiceModule.h"
#include "aalsdk/aas/AALInProcServiceFactory.h"  // Defines InProc Service Factory
#include "aalsdk/Dispatchables.h"
#include "_MessageDelivery.h"


#if defined ( __AAL_WINDOWS__ )
#pragma warning( pop )
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)

/// @addtogroup MDS
/// @{

_MessageDelivery::_MessageDelivery() :
   m_Dispatcher() // Default is a simple single threaded scheduler.
{
   if ( EObjOK != SetInterface(iidMDS,
                               dynamic_cast<IMessageDeliveryService *>(this)) ) {
      m_bIsOK = false;
   }
}

//=============================================================================
// Name: ~_MessageDelivery
// Description: Retrieve the IEventDispatcher interface.
// Interface: public
// Comments:
//=============================================================================
_MessageDelivery::~_MessageDelivery()
{
   StopMessageDelivery();
}

//=============================================================================
// Name: StartMessageDelivery
// Description: Start the service
// Interface: public
// Comments:
//=============================================================================
void _MessageDelivery::StartMessageDelivery()
{
   AutoLock(this);
   m_Dispatcher.Start();
}

//=============================================================================
// Name: StopMessageDelivery
// Description: Stop the service
// Interface: public
// Comments: Brute force method
//=============================================================================
void _MessageDelivery::StopMessageDelivery()
{
   AutoLock(this);
   m_Dispatcher.Drain();
   m_Dispatcher.Stop();
}

//=============================================================================
// Name: ~scheduleMessage
// Description: Schedule a message for processing
// Interface: public
// Comments:
//=============================================================================
btBool _MessageDelivery::scheduleMessage(IDispatchable *pDispatchable)
{
   AutoLock(this);
   return m_Dispatcher.Add(pDispatchable);
}

/// @}

END_NAMESPACE(AAL)

