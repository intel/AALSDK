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
#ifndef __MessageDelivery_H__
#define __MessageDelivery_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/eds/AASEventDeliveryService.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/osal/ThreadGroup.h>
#include <aalsdk/osal/OSServiceModule.h>

/// @addtogroup MDS
/// @{

#define AALMDS_SVC_MOD         "localMDS" AAL_SVC_MOD_EXT
#define AALMDS_SVC_ENTRY_POINT "localMDS" AAL_SVC_MOD_ENTRY_SUFFIX

//=============================================================================
// Name: AAL_DECLARE_SVC_MOD
// Description: Declares a module entry point.
// Comments: Not used for static built-in Services
//=============================================================================
AAL_DECLARE_SVC_MOD(localMDS, AALRUNTIME_API)


BEGIN_NAMESPACE(AAL)

class _MessageDelivery;

//=============================================================================
// Name: _MessageDelivery
// Description: Default message delivery facility
// Interface: IEventDeliveryService
// Comments:  This object is operational and meets its minimum functional
//            requirements pior to init()
//=============================================================================
class _MessageDelivery : public ServiceBase,
                           public IEventDeliveryService
{
public:
   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(_MessageDelivery, ServiceBase)
   {
      // Default is a simple single threaded scheduler.
      m_Dispatcher = new OSLThreadGroup;
      SetSubClassInterface(iidEventDeliveryService,
                           dynamic_cast<IEventDeliveryService *>(this));
      m_pcontainer->getRuntimeServiceProvider()->setMessageDeliveryService(dynamic_cast<IBase *>(this));
   }

   // Initialize the object including any configuration changes based on
   //  start-up config parameters. Once complete the facility is fully functional
   void init(TransactionID const &rtid);


   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Quiet Release. Used when Service is unloaded.
   btBool Release(btTime timeout=AAL_INFINITE_WAIT);

   // Set config parameters
   btBool SetParms(NamedValueSet const &rparms);

   //
   // IEventDeliverService
   EDS_Status Schedule();
   // Nop for now
   void Unblock() { ASSERT(false); }

   void StartEventDelivery();
   void StopEventDelivery();

   btBool QueueEvent(btEventHandler ,
                     CAALEvent * );

   btBool QueueEvent(btObjectType ,
                     CAALEvent * );

   btBool QueueEvent(btObjectType ,
                     IDispatchable *);

   /// Retrieve the IEventDispatcher interface.
   IEventDispatcher *GetEventDispatcher(EDSDispatchClass = EDS_dispatcherNormal);

   ~_MessageDelivery();

protected:
   OSLThreadGroup *m_Dispatcher;
};


END_NAMESPACE(AAL)

/// @} group MDS

#endif // __MessageDelivery_H__
