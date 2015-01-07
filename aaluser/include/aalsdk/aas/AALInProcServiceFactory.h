// Copyright (c) 2011-2015, Intel Corporation
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
/// @file AALInProcServiceFactory.h
/// @brief This file contains the implementation of classes and templates
///    for the In-proc service factory.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/02/2011      JG      Initial version
/// 09/07/2011      JG      Removed from AAService
/// 01/12/2012      JG      Modified for cleaned up Service init() protocol@endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALINPROCSERVICEFACTORY_H__
#define __AALSDK_AAS_AALINPROCSERVICEFACTORY_H__
#include <aalsdk/aas/AALServiceModule.h>


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////                                                   //////////////
///////                          AAL SERVICE                            ///////
//////////////                                                   //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/// @addtogroup InProcFactory
/// @{


//
// The AAL Service is comprised of a Service Library which implements the
//  the load-able module  (e.g., shared or dynamic link library).
//
// The module  becomes load-able by defining a factory class using one of the
//  provided templates InProcSvcsFact<> which defines how the service is
//  implemented. For example InProcSvcsFact<> specifies that the service is
//  implemented in-proc (i.e., in the same process space) where IPCSvcsFact<>
//  defines a factory that constructs a service that uses IPC to communicate.
// An AALServiceModule class implements a wrapper for the service
//  IServiceModule interface, instantiates the "real" service through a
//  factory and optional marshaler and transport.
//
// The Service container is implemented as a class AALServiceModule.
//  The container creates and holds a pointer to the Service object.
//
//  The Service object must be an IServiceModule derived class. This allows
//  the service to provide implementation for most of the IServiceModule
//  interfaces.  The container forwards all such calls through to the
//  service, performing canonical behavior for the service.
//
/// The service factory is a template that instantiates the service. The factory
///  is responsible for construction details such as in-proc, daemon, remote.
//
// The marshaler is a template class that is responsible for marshaling the
//  interface appropriately for the object. The default in-proc marshaler is
//  a stub.
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// ISvcsFact implementation capable of creating multiple instances of the desired Service
/// in the same process address space as the Factory itself.
///
/// @param  I  The type of Service to be created. The type must be derived from ServiceBase.
template <typename I>
class InProcSvcsFact : public ISvcsFact
{
public:
   IBase * CreateServiceObject(AALServiceModule    *container,
                               btEventHandler       eventHandler,
                               btApplicationContext context,
                               TransactionID const &rtid,
                               NamedValueSet const &optArgs)
   {
      m_pService = new I(container);
      if ( NULL == m_pService ) {
         return NULL;
      }

      // Initialize the service
      IBase *ptr = m_pService->_init(eventHandler, context, rtid, optArgs);

      if( NULL == ptr ) {
         delete m_pService;
         m_pService = NULL;
      }

      return ptr;
   }


   IBase * CreateServiceObject(AALServiceModule    *container,
                               AAL::IBase          *pclient,
                               TransactionID const &rtid,
                               NamedValueSet const &optArgs,
                               AAL::btBool          NoRuntimeEvent)
   {
      m_pService = new I(container);
      if ( NULL == m_pService ) {
         return NULL;
      }

      // Initialize the service
      IBase *ptr = m_pService->_init(pclient, rtid, optArgs, NULL, NoRuntimeEvent);

      if( NULL == ptr ) {
         delete m_pService;
         m_pService = NULL;
      }

      return ptr;
   }
protected:
   I *m_pService;
};


/// ISvcsFact implementation capable of creating exactly one instance of the desired Service
/// in the same process address space as the Factory itself.
///
/// @param  I  The type of Service to be created. The type must be derived from ServiceBase.
template <typename I>
class InProcSingletonSvcsFact : public ISvcsFact
{
public:
   InProcSingletonSvcsFact() :
      m_pService(NULL)
   {}

   IBase * CreateServiceObject(AALServiceModule    *container,
                               btEventHandler       eventHandler,
                               btApplicationContext context,
                               TransactionID const &rtid,
                               NamedValueSet const &optArgs)

   {
      // Only crate the new instance if one does not exist
      if ( NULL == m_pService ) {

         m_pService = new I(container);
         if ( NULL == m_pService ) {
            return NULL;
         }

      }

      // Initialize the service
      IBase *ptr = m_pService->_init(eventHandler, context, rtid, optArgs);

      if ( NULL == ptr ) {
         delete m_pService;
         m_pService = NULL;
      }

      // Return what the service gives
      return ptr;
   }

   IBase * CreateServiceObject(AALServiceModule    *container,
                                AAL::IBase         *pclient,
                                TransactionID const &rtid,
                                NamedValueSet const &optArgs,
                                AAL::btBool          NoRuntimeEvent)

    {
       // Only crate the new instance if one does not exist
       if ( NULL == m_pService ) {

          m_pService = new I(container);
          if ( NULL == m_pService ) {
             return NULL;
          }

       }

       // Initialize the service
       IBase *ptr = m_pService->_init(pclient, rtid, optArgs, NULL, NoRuntimeEvent);

       if ( NULL == ptr ) {
          delete m_pService;
          m_pService = NULL;
       }

       // Return what the service gives
       return ptr;
    }

protected:
   I *m_pService;
};


/// @} group InProcFactory


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif // __AALSDK_AAS_AALINPROCSERVICEFACTORY_H__

