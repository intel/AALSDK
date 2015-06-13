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
//        FILE: _xlServiceBroker.h
//     CREATED: Mar 14, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Defintions for the internal default XL Runtime Service Broker
//            facility.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef ___XLSERVICEBROKER_H__
#define ___XLSERVICEBROKER_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/aas/IServiceBroker.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/osal/OSServiceModule.h>

#include <map>

//=============================================================================
// Name: AAL_DECLARE_SVC_MOD
// Description: Declares a module entry point.
//=============================================================================
AAL_DECLARE_SVC_MOD(localServiceBroker, XLRT_API)


BEGIN_NAMESPACE(AAL)


class _xlServiceBroker : public  ServiceBase,
                         private CUnCopyable,
                         public  IServiceBroker
{
public:
   typedef std::map<std::string, ServiceHost *> ServiceMap;
   typedef ServiceMap::iterator                 Servicemap_itr;


   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(_xlServiceBroker, ServiceBase),
      m_pShutdownThread(NULL),
      m_servicecount(0)
   {
      SetSubClassInterface(iidServiceBroker,
                           dynamic_cast<IServiceBroker *>(this));
   }

   // Initialize the object including any configuration changes based on
   //  start-up config parameters. Once complete the facility is fully functional
   void init(TransactionID const &rtid);

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Quiet Release. Used when Service is unloaded.
   btBool Release(btTime timeout=AAL_INFINITE_WAIT);

   void allocService(IRuntime                 *pProxy,
                     IRuntimeClient           *pRuntimeClient,
                     IBase                    *pServiceClientBase,
                     const NamedValueSet      &rManifest,
                     TransactionID const      &rTranID);


   ~_xlServiceBroker();

protected:
   ServiceHost *findServiceHost(std::string const &sName);

   // Used by Release
   static void ShutdownThread(OSLThread           *pThread, void  *pContext);
   btBool          DoShutdown(TransactionID const &rTranID, btTime timeout);

   static void ShutdownHandlerThread(OSLThread   *pThread,  void       *pContext);
   void              ShutdownHandler(ServiceHost *pSvcHost, CSemaphore &cnt);

protected:
   ServiceMap          m_ServiceMap;
   OSLThread          *m_pShutdownThread;
   btUnsigned32bitInt  m_servicecount;
};


END_NAMESPACE(AAL)


#endif // ___XLSERVICEBROKER_H__
