// Copyright (c) 2012-2015, Intel Corporation
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
/// @file AALServiceModule.cpp
/// @brief AALServiceModule implementation.  The AAL Service Module is an
///        object embedded in the Service Library that:
///        - Implements interface to outside for Service Construction
///        - Keeps track of all Services constructed through it.
///        - NOTE: Some Services may expose more objects than are tracked by
///                the ServiceModule. For example a singleton Service may
///                expose smart pointers or Proxies to allow the singleton to
///                to be shared. This type service may appear as 1 Service.
///                It is up to the Service to track these sub-objects.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/22/2013     TSW      Moving C++ inlined definitions to .cpp file
/// 09/15/2015     JG       Redesigned to fix flow bugs@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/aas/AALServiceModule.h"


BEGIN_NAMESPACE(AAL)


IAALTransport::~IAALTransport() {}
IAALMarshalUnMarshallerUtil::~IAALMarshalUnMarshallerUtil() {}
IAALMarshaller::~IAALMarshaller() {}
IAALUnMarshaller::~IAALUnMarshaller() {}
ISvcsFact::~ISvcsFact() {}
IServiceModule::~IServiceModule() {}
IServiceModuleCallback::~IServiceModuleCallback() {}

//=============================================================================
// Name: AALServiceModule()
// Description: Constructor
//=============================================================================
AALServiceModule::AALServiceModule(ISvcsFact &fact) :
   CAASBase(),
   m_pService(NULL),
   m_pendingcount(0),
   m_SvcsFact(fact)
{
   SetInterface(iidServiceProvider, dynamic_cast<IServiceModule *>(this));
}

//=============================================================================
// Name: ~AALServiceModule()
// Description: Destructor
//=============================================================================
AALServiceModule::~AALServiceModule() {}

btBool AALServiceModule::Construct(IRuntime           *pAALRuntime,
                                   IBase              *Client,
                                   TransactionID const &tranID,
                                   NamedValueSet const &optArgs)
{

   AutoLock(this);

   // Create the actual object
   IBase *pNewService = m_SvcsFact.CreateServiceObject( this,
                                                        pAALRuntime);
   // Add the service to the list of services the module
   if ( NULL == pNewService ) {
      return false;
   }

   // Initialize the Service. It  will issue serviceAllocated or failure.
   //   If it fails here we cleanup immediately.
   if(!m_SvcsFact.InitializeService(pNewService,
                                    Client,
                                    tranID,
                                    optArgs)){

      // Use the factory to Destroy the uninitialized object
      m_SvcsFact.DestroyServiceObject(pNewService);
      return false;
   }else {
      // Keep track of outstanding transactions so that
      //  we don't disappear before they are complete.
      m_pendingcount++;
   }

   return false;
}

//=============================================================================
// Name: Destroy()
// Description: Destroy all registered Services
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
void AALServiceModule::Destroy()
{
   // Protect the counter calculation
   //  so nothing can be released until
   //  we are waiting
   {
      AutoLock(this);

      list_type::size_type size = m_serviceList.size();
      if ( 0 == size ) {
         return;
      }

      // Initialize the semaphore as a count up by initializing
      //  count to a negative number.
      //  The waiter will block until the semaphore
      //  counts up to zero.
      m_srvcCount.Create( - static_cast<btInt>(size) );

      // TODO CHECK RETURN

      // Loop through all services and shut them down
      SendReleaseToAll();
   }

   // Wait for all to complete.
   m_srvcCount.Wait();
}

//=============================================================================
// Name: ServiceReleased()
// Description: Callback invoked when a Servcie has been released
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
void AALServiceModule::ServiceReleased(IBase *pService)
{
   AutoLock(this);
   RemovefromServiceList(pService);
}

//=============================================================================
// Name: ServiceInitialized()
// Description: Callback invoked when the Service has been successfully
//              initialized
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
void AALServiceModule::ServiceInitialized(IBase *pService)
{
   AutoLock(this);

   // Reduce pending count
   m_pendingcount--;

   // Add Service to the List
   AddToServiceList(pService);

}

//=============================================================================
// Name: AddToServiceList()
// Description: Add a Service to the list of constructed Services
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
btBool AALServiceModule::AddToServiceList(IBase *pService)
{
   AutoLock(this);

   if ( ServiceInstanceRegistered(pService) ) {
      return false;
   }

   m_serviceList[pService] = pService;
   return true;
}

//=============================================================================
// Name: RemovefromServiceList()
// Description: Remove a Released Service
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
btBool AALServiceModule::RemovefromServiceList(IBase *pService)
{
   AutoLock(this);

   if ( !ServiceInstanceRegistered(pService) ) {
      return false;
   }

   m_serviceList.erase(pService);

   // Post to the count up semaphore
   //  in case the service is shutting down
   m_srvcCount.Post(1);
   return true;
}

//=============================================================================
// Name: ServiceInstanceRegistered()
// Description: Determine if the Service has already been registered
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
btBool AALServiceModule::ServiceInstanceRegistered(IBase *pService)
{
   AutoLock(this);

   const_iterator itr = m_serviceList.end();

   // Find the named value pair
   if ( m_serviceList.find(pService) == itr ) {
      return false;
   }

   return true;
}

//=============================================================================
// Name: SendReleaseToAll()
// Description: Broadcast a hard Release to all Services
// Interface: public
// Inputs: none
// Outputs: none.
// Comments: THIS IS HARD CORE AND MAY WANT TO BE REMOVED
//=============================================================================
void AALServiceModule::SendReleaseToAll()
{
   AutoLock(this);   // Lock until done issuing releases

   const_iterator iter = m_serviceList.begin();

   while ( m_serviceList.end() != iter ) {

      // Get the IAALService from the IBase
      IAALService *pService = dynamic_ptr<IAALService>(iidService, (*iter).second);

      if ( NULL != pService ) {
         // Release with no event. Service will call Released() as a notification
         pService->Release(AAL_INFINITE_WAIT);
      }

      iter++;
   }
}


END_NAMESPACE(AAL)


