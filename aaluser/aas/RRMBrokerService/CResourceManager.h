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

#include "aalsdk/rm/AALResourceManagerClient.h"

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
                                               public  IResourceManager

{
public:
   // Loadable Service
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CResourceManager, ServiceBase),
      m_pResMgrClient(NULL),
      m_RMProxy(),
      m_pProxyPoll(NULL)
   {
      SetSubClassInterface( iidResMgr,
                            dynamic_cast<IResourceManager *>(this));
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

private:
   static void ProxyPollThread( OSLThread *pThread,
                                void      *pContext);
   void ProcessRMMessages();

   IResourceManagerClient        *m_pResMgrClient;
   CResourceManagerProxy          m_RMProxy;
   OSLThread                     *m_pProxyPoll;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_XL_CRESOURCEMANAGER_H__

