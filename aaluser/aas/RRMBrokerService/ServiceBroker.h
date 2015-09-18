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

#include <aalsdk/rm/AALResourceManagerClient.h>

#include <CResourceManager.h>


//=============================================================================
// Name: AAL_DECLARE_SVC_MOD
// Description: Declares a module entry point.
// Comments: Not used for static built-in Services
//=============================================================================
AAL_DECLARE_SVC_MOD(librrmbroker, RRMBROKER_API)


BEGIN_NAMESPACE(AAL)

class ServiceBroker : public  ServiceBase,
                      public  IServiceBroker,
                      private CUnCopyable,
                      private IServiceClient,
                      private IResourceManagerClient
{
public:
   typedef std::map<std::string, ServiceHost *> ServiceMap;
   typedef ServiceMap::iterator                 Servicemap_itr;

   // Map to relate internal transactions to the client transaction.

   // Comparison class
   class tidcompare{
   public:
      bool operator() (const TransactionID &lhs, const TransactionID &rhs)const {return lhs.ID() < rhs.ID();}
   };

   typedef std::map< TransactionID, TransactionID, tidcompare > TransactionMap;
   typedef TransactionMap::iterator                             TransactionMap_itr;

   // Map to hold clients of outstanding transactions
   struct ServiceDesc{
      IBase                  *ServiceBase;
      IRuntime               *pProxy;
      IRuntimeClient         *pRuntimeClient;
   };

   typedef std::map<TransactionID, struct ServiceDesc, tidcompare> ServiceClientMap;
   typedef ServiceMap::iterator                                    ServiceClientMap_itr;
   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(ServiceBroker, ServiceBase),
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
                           dynamic_cast<IServiceClient *>(this));
      SetInterface(iidResMgrClient,
                           dynamic_cast<IResourceManagerClient *>(this));

   }

   // Initialize the object including any configuration changes based on
   //  start-up config parameters. Once complete the facility is fully functional
   btBool init(IBase *pclientBase,
               NamedValueSet const &optArgs,
               TransactionID const &rtid);

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   void allocService(IRuntime               *pProxy,
                     IRuntimeClient         *pRuntimClient,
                     IBase                  *pServiceClientBase,
                     const NamedValueSet    &rManifest,
                     TransactionID const    &rTranID);
protected:
   ServiceHost *findServiceHost(std::string const &sName);

   // Internal IServiceClient used to allocate Resource Manager
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID = TransactionID());
   void serviceAllocateFailed( const IEvent &rEvent);
   void serviceReleased( TransactionID const &rTranID = TransactionID());
   void serviceReleaseFailed( const IEvent &rEvent);
   void serviceEvent(const IEvent &rEvent);

   // Internal IResourceManagerClient used to allocate Resources
   void resourceAllocated( NamedValueSet const &nvsInstancerecord,
                                     TransactionID const &tid );
   void resourceRequestFailed( NamedValueSet const &nvsManifest,
                                         const IEvent &rEvent );
   void resourceManagerException( const IEvent &rEvent );

   // Used by Release
   static void        ShutdownThread(OSLThread *pThread, void *pContext);
   btBool                 DoShutdown(TransactionID const &rTranID, btTime timeout);
   static void ShutdownHandlerThread(OSLThread *pThread, void *pContext);
   void              ShutdownHandler(Servicemap_itr itr, CSemaphore &cnt);

protected:
   ServiceHost        *m_pRMSvcHost;
   IResourceManager   *m_ResMgr;
   IBase              *m_ResMgrBase;
   ServiceMap          m_ServiceMap;
   OSLThread          *m_pShutdownThread;
   btUnsigned32bitInt  m_servicecount;
   TransactionMap      m_Transactions;
   ServiceClientMap    m_ServiceClientMap;
};


END_NAMESPACE(AAL)

#endif // __SERVICEBROKER_H__
