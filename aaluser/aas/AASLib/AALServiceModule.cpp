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
/// @brief AALServiceModule implementation.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/22/2013     TSW      Moving C++ inlined definitions to .cpp file@endverbatim
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

AALServiceModule::AALServiceModule(ISvcsFact &fact) :
   CAASBase(),
   m_pBase(NULL),
   m_pService(NULL),
   m_refcount(0),
   m_SvcsFact(fact)
{
   SetInterface(iidServiceProvider, dynamic_cast<IServiceModule *>(this));
}

AALServiceModule::~AALServiceModule() {}

IBase *AALServiceModule::Construct(btEventHandler       Listener,
                                   TransactionID const &tranID,
                                   btApplicationContext context,
                                   NamedValueSet const &optArgs)
{
   // Add this one to the list of objects this container holds.
   //  It's up to the factory to enforce singletons.
   m_pBase = m_SvcsFact.CreateServiceObject(this,
                                            Listener,
                                            context,
                                            tranID,
                                            optArgs);

   // Add the service to the list of services the module
   if ( NULL != m_pBase ) {
      AddToServiceList(m_pBase);
   }

   return m_pBase;
}

IBase *AALServiceModule::Construct(IBase               *Client,
                                   TransactionID const &tranID,
                                   NamedValueSet const &optArgs,
                                   btBool               NoRuntimeEvent)
{
   // Add this one to the list of objects this container holds.
   //  It's up to the factory to enforce singletons.
   m_pBase = m_SvcsFact.CreateServiceObject(this,
                                            Client,
                                            tranID,
                                            optArgs,
                                            NoRuntimeEvent);

   // Add the service to the list of services the module
   if ( NULL != m_pBase ) {
      AddToServiceList(m_pBase);
   }

   return m_pBase;
}

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

void AALServiceModule::ServiceReleased(IBase *pService)
{
   AutoLock(this);
   RemovefromServiceList(pService);
}

btBool AALServiceModule::AddToServiceList(IBase *pService)
{
   AutoLock(this);

   if ( ServiceInstanceRegistered(pService) ) {
      return false;
   }

   m_serviceList[pService] = pService;
   return true;
}

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


