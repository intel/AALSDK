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
//        FILE: ServiceBroker.h
//     CREATED: Mar 14, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for the Sample default XL Runtime Service Broker
//            facility.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __SERVICEBROKER_H__
#define __SERVICEBROKER_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/INTCDefs.h>
#include <aalsdk/aas/IServiceBroker.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/osal/OSServiceModule.h>

#include <aalsdk/rm/XLResourceManagerClient.h>

#include <CResourceManager.h>




#if defined ( __AAL_WINDOWS__ )
# ifdef RRMBROKER_EXPORTS
#    define RRMBROKER_API __declspec(dllexport)
# else
#    define RRMBROKER_API __declspec(dllimport)
# endif // RRMBROKER_EXPORTS
#else
# define RRMBROKER_API    __declspec(0)
#endif // __AAL_WINDOWS__


//=============================================================================
// Name: AAL_DECLARE_SVC_MOD
// Description: Declares a module entry point.
// Comments: Not used for static built-in Services
//=============================================================================
AAL_DECLARE_SVC_MOD(librrmbroker, RRMBROKER_API)


BEGIN_NAMESPACE(AAL)

class ServiceBroker : public  AAL::AAS::ServiceBase,
                      public  AAL::XL::RT::IServiceBroker,
                      private CUnCopyable,
                      private AAL::AAS::IServiceClient,
                      private IResourceManagerClient
{
public:
   typedef map<std::string, AAL::XL::RT::ServiceHost *> ServiceMap;
   typedef ServiceMap::iterator                         Servicemap_itr;

   // Map to relate internal transactions to the client transaction.

   // Comparison class
   class tidcompare{
   public:
      bool operator() (const TransactionID &lhs, const TransactionID &rhs)const {return lhs.ID() < rhs.ID();}
   };

   typedef map< TransactionID, TransactionID, tidcompare > TransactionMap;
   typedef TransactionMap::iterator                TransactionMap_itr;

   // Map to hold clients of outstanding transactions
   struct ServiceDesc{
      AAL::IBase                             *ServiceBase;
      AAL::XL::RT::IRuntime::eAllocatemode    NoRuntimeEvent;
   };

   typedef map<TransactionID, struct ServiceDesc, tidcompare> ServiceClientMap;
   typedef ServiceMap::iterator           ServiceClientMap_itr;
   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(ServiceBroker, AAL::AAS::ServiceBase),
      m_pRMSvcHost(NULL),
      m_ResMgr(NULL),
      m_ResMgrBase(NULL),
      m_pShutdownThread(NULL),
      m_servicecount(0)
   {
      // Register all exported interfaces
      SetSubClassInterface(iidServiceBroker,
                           dynamic_cast<IServiceBroker *>(this));
      SetInterface(iidServiceClient,
                           dynamic_cast<AAL::AAS::IServiceClient *>(this));
      SetInterface(iidResMgrClient,
                           dynamic_cast<AAL::IResourceManagerClient *>(this));

   }

   // Initialize the object including any configuration changes based on
   //  start-up config parameters. Once complete the facility is fully functional
   void init(AAL::TransactionID const &rtid);

   // Called when the service is released
   btBool Release(AAL::TransactionID const &rTranID, AAL::btTime timeout=AAL_INFINITE_WAIT);

   // Quiet Release. Used when Service is unloaded.
   btBool Release(AAL::btTime timeout=AAL_INFINITE_WAIT);

   void allocService(AAL::IBase                             *pClient,
                     const AAL::NamedValueSet               &rManifest,
                     AAL::TransactionID const               &rTranID,
                     AAL::XL::RT::IRuntime::eAllocatemode    mode =  AAL::XL::RT::IRuntime::NotifyAll);
protected:
   XL::RT::ServiceHost *findServiceHost(std::string const &sName);

   // Internal IServiceClient used to allocate Resource Manager
   void serviceAllocated( AAL::IBase *pServiceBase,
                          TransactionID const &rTranID = TransactionID());
   void serviceAllocateFailed( const IEvent &rEvent);
   void serviceFreed( TransactionID const &rTranID = TransactionID());
   void serviceEvent(const IEvent &rEvent);

   // Internal IResourceManagerClient used to allocate Resources
   void resourceAllocated( NamedValueSet const &nvsInstancerecord,
                                     TransactionID const &tid );
   void resourceRequestFailed( NamedValueSet const &nvsManifest,
                                         const IEvent &rEvent );
   void resourceManagerException( const IEvent &rEvent );

   // Used by Release
   static void        ShutdownThread(OSLThread *pThread, void *pContext);
   btBool                 DoShutdown(AAL::TransactionID const &rTranID, AAL::btTime timeout);
   static void ShutdownHandlerThread(OSLThread *pThread, void *pContext);
   void              ShutdownHandler(Servicemap_itr itr, CSemaphore &cnt);

protected:
   XL::RT::ServiceHost    *m_pRMSvcHost;
   IResourceManager       *m_ResMgr;
   AAL::IBase             *m_ResMgrBase;
   ServiceMap              m_ServiceMap;
   OSLThread              *m_pShutdownThread;
   AAL::btUnsigned32bitInt m_servicecount;
   TransactionMap          m_Transactions;
   ServiceClientMap        m_ServiceClientMap;
};


END_NAMESPACE(AAL)

#endif // __SERVICEBROKER_H__
