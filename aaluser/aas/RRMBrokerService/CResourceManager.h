// Copyright(c) 2014-2016, Intel Corporation
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
/// @file CResourceManager.h
/// @brief CResourceManager - Public Interface to the AAL ResourceManager
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///          
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 10/30/2008     JG       Initial version started@endverbatim
//****************************************************************************
#ifndef __AALSDK_XL_CRESOURCEMANAGER_H__
#define __AALSDK_XL_CRESOURCEMANAGER_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/OSServiceModule.h>           // For Service Enabling
#include <aalsdk/aas/AALService.h>

#include <aalsdk/AALIDDefs.h>

#include "aalsdk/AALTransactionID.h"
#include "aalsdk/rm/AALResourceManager.h"
#if defined( __AAL_LINUX__ )
#  include "CResourceManagerProxy.h"
#elif  defined( __AAL_WINDOWS__ )
#  include "win/CResourceManagerProxy.h"
#endif

#include <aalsdk/rm/CAASResourceManager.h>

//#include <aalsdk/kernel/aalrm_client.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Constants and typedefs
//=============================================================================

// Hooks to enable this as an AAL Service
#define XLRM_SVC_MOD         "localRM" AAL_SVC_MOD_EXT
#define XLRM_SVC_ENTRY_POINT "localRM" AAL_SVC_MOD_ENTRY_SUFFIX

//=============================================================================
// Name: AAL_DECLARE_BUILTIN_SVC_MOD
// Description: Declares a module entry point.
// Comments: Its recommended that the 1st argument be a name that could later
//           be used as the module's name (e.g., libxyz) to make it easier to
//           convert built-in into plug-in service.
//=============================================================================
//AAL_DECLARE_SVC_MOD(localrm, AALRESOURCEMANAGER_API)
AAL_DECLARE_BUILTIN_SVC_MOD(librrm, AALRESOURCEMANAGER_API)

//=============================================================================
// Name: CResourceManager
// Description: Concrete definition of the Remote Resource Manager
//=============================================================================
class AALRESOURCEMANAGER_API CResourceManager : private CUnCopyable,
                                               public  ServiceBase,
                                               public  IResourceManager,
                                               public IServiceClient
{
public:
   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CResourceManager, ServiceBase),
      m_pResMgrClient(NULL),
      m_RMProxy(),
      m_pProxyPoll(NULL),
      m_pRRMAALService(NULL),
      m_pRRMService(NULL),
      m_rrmStartupMode(automatic)
{
      SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
      SetInterface( iidResMgr,
                    dynamic_cast<IResourceManager *>(this));
      m_sem.Create(0, 1);
   }

   ~CResourceManager();

   // Initialize the object including any configuration changes based on
   //  start-up config parameters. Once complete the facility is fully functional
   btBool init( IBase *pclientBase,
                NamedValueSet const &optArgs,
                TransactionID const &rtid);

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Request a resource.
   void RequestResource( NamedValueSet const &nvsManifest,
                         TransactionID const &tid);

   // <IServiceClient>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID = TransactionID());
   virtual void serviceAllocateFailed(const IEvent &rEvent);
   virtual void serviceReleased(TransactionID const &rTranID = TransactionID());
   virtual void serviceReleaseRequest(const IEvent &rEvent){};  // Ignored TODO better implementation
   virtual void serviceReleaseFailed(const IEvent &rEvent);
   virtual void serviceEvent(const IEvent &rEvent);
   // </IServiceClient>

protected:
   void StopMessagePump();

private:
   static void ProxyPollThread( OSLThread *pThread,
                                void      *pContext);
   void ProcessRMMessages();

   /// @brief Allocates the Remote Resource Manager service.
   ///
   /// The service thread will be started in serviceAllocated().
   void allocRRMService(TransactionID const &rTid = TransactionID());


   /// @brief Checks if remote resource manager is already running by trying to
   /// open its device file.
   ///
   /// This is used primarily to determine whether it is safe to start
   /// a Remote Resource Manager instance to service resource requests from
   /// this (and possibly other) processes.
   ///
   /// @return true if another RRM is already running, false otherwise.
   btBool isRRMPresent();


   /// @brief Remote Resource Manager startup mode
   ///
   /// Three modes are possible: 'always' will try to start a Remote Resource
   /// Manager (RRM) thread during initialization, regardless of any other
   /// RRM instances that are already running; 'automatic' will start a RRM
   /// thread only when allocating a service requiring hardware access, and
   /// only if no RRM is already running; and 'never' will not start a RRM but
   /// assume that there is already one present on the system, possibly in
   /// another process.
   ///
   /// The actual mode is passed either through setting the environment variable
   /// AAL_RESOURCEMANAGER_CONFIG_INPROC to 'always', 'auto', or 'never'; or
   /// by setting the AAL_RESOURCEMANAGER_CONFIG_INPROC key in the runtime
   /// config record to 'always', 'auto', or 'never'.
   enum RRMStartupMode {
      always = 1,
      automatic,
      never
   };

   IResourceManagerClient        *m_pResMgrClient;
   CResourceManagerProxy          m_RMProxy;
   OSLThread                     *m_pProxyPoll;

   CSemaphore                     m_sem;

   TransactionID                  m_initTid;       // for proper initComplete()
   TransactionID                  m_releaseTid;    // for proper Service::Release()

   // Remote Resource Manager
   RRMStartupMode                 m_rrmStartupMode;
   IResMgrService                *m_pRRMService;
   IAALService                   *m_pRRMAALService;

   // TODO: we might use STL's pair instead of custom structs, but I need to
   //       figure out how pair constructs its members (by value or by ref)
   //       (EL)

   // Context for properly wrapping transaction IDs on Release()
   struct ReleaseContext {
      const TransactionID   tranID;
      const btTime          timeout;
      ReleaseContext(const TransactionID &rtid, btTime to) :
         tranID(rtid),
         timeout(to)
      {}
   };

   // Context for tunneling resource request through RRM allocation
   struct ResourceRequestContext {
      const NamedValueSet nvsManifest;
      const TransactionID tranID;
      ResourceRequestContext(const NamedValueSet &nvs, const TransactionID &rtid) :
         nvsManifest(nvs),
         tranID(rtid)
      {}
   };

};

END_NAMESPACE(AAL)

#endif // __AALSDK_XL_CRESOURCEMANAGER_H__

