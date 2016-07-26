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
//     CREATED: Mar 22, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
///        @file: ServiceHost.cpp
///
/// @brief   Wrapper class for Service Host plug-ins
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 09/5/2015      JG       Added Provider Release. Changed for 4.x@endverbatim
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"
#include "aalsdk/aas/ServiceHost.h"
#include <aalsdk/Runtime.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: ServiceHost
// Description: Constructor
// Interface: public
// Inputs: root_name - Name of the Service to load.
// Comments:
//=============================================================================
ServiceHost::ServiceHost(btcString root_name) :
   m_bIsOK(false),
   m_pDynLinkLib(NULL),
   m_pProvider(NULL),
   m_base(NULL)
{
   OSServiceModuleInit(&m_modparms, root_name);

   // Load the Class factory's module
   m_pDynLinkLib = new(std::nothrow) DynLinkLibrary(std::string(m_modparms.full_name));

   if ( ( NULL == m_pDynLinkLib ) || !m_pDynLinkLib->IsOK() ) {
      return; // m_bIsOK remains false.
   }

   m_modparms.entry_point_fn = (AALSvcEntryPoint)m_pDynLinkLib->GetSymAddress(std::string(m_modparms.entry_point_name));
   
   if ( ( NULL == m_modparms.entry_point_fn ) ||
        ( 0 != m_modparms.entry_point_fn(AAL_SVC_CMD_GET_PROVIDER, &m_pProvider) ) ||
        ( NULL == m_pProvider ) ) {
      return; // m_bIsOK remains false.
   }

   m_name  = std::string(root_name);
   m_bIsOK = true;

   return;
}

//=============================================================================
// Name: ServiceHost
// Description: Constructor
// Interface: public
// Inputs: EntryPoint - Entry point of loaded Service Module.
// Comments:
//=============================================================================
ServiceHost::ServiceHost(AALSvcEntryPoint EP) :
   m_bIsOK(false),
   m_pDynLinkLib(NULL),
   m_pProvider(NULL),
   m_base(NULL)
{
   memset(&m_modparms, 0, sizeof(OSServiceModule));

   ASSERT(NULL != EP);
   if ( NULL == EP ) {
      return; // m_bIsOK remains false.
   }

   m_modparms.entry_point_fn = EP;

   if ( ( 0 != m_modparms.entry_point_fn(AAL_SVC_CMD_GET_PROVIDER, &m_pProvider) ) ||
        ( NULL == m_pProvider ) ) {
      return;
   }

   m_bIsOK = true;
}

//=============================================================================
// Name: freeProvider
// Description: Frees the provider in eth Servcie Library
// Interface: public
// Comments:
//=============================================================================
void ServiceHost::freeProvider()
{
   if ( NULL != m_modparms.entry_point_fn ) {
      m_modparms.entry_point_fn(AAL_SVC_CMD_FREE_PROVIDER, NULL);
      m_modparms.entry_point_fn = NULL;
   }
}

//=============================================================================
// Name: ~ServiceHost
// Description: Destructor
// Interface: public
// Comments: Unloads the Service Library.
//=============================================================================
ServiceHost::~ServiceHost()
{
   freeProvider();

   if ( NULL != m_pDynLinkLib ) {
      delete m_pDynLinkLib;
   }
}

//=============================================================================
// Name: InstantiateService
// Description: Allocate an instance of the Service
// Interface: public
// Inputs: pRuntime - Pointer to runtime
//         pClient - Owners ServiceCient interface
//         rManifest - Manifest and optional arguments
//         rTranID - Optional Transaction ID
// Comments: If this fails then do not expect an event is generated by the
//           Service.
//=============================================================================
btBool ServiceHost::InstantiateService(IRuntime            *pRuntime,
                                       IBase               *pClientBase,
                                       NamedValueSet const &rManifest,
                                       TransactionID const &rTranID)
{
   // Assign a runtime proxy to this Service  TODO DEPRECATE
   ASSERT(NULL != pRuntime);
   if ( NULL == pRuntime ) {
      return false;
   }

   if ( !IsOK() ) {
      return false;
   }

   ASSERT(NULL != m_pProvider);
   if ( NULL == m_pProvider ) {
      return false;
   }

   return m_pProvider->Construct(pRuntime, pClientBase, rTranID, rManifest);
}

END_NAMESPACE(AAL)

