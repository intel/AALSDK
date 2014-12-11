// Copyright (c) 2014, Intel Corporation
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
/// @file XLResourceManager.h
/// @brief XLResourceManager - Public Interface to the AAL ResourceManager
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
#ifndef __AALSDK_XL_AASRESOURCEMANAGER_H__
#define __AALSDK_XL_AASRESOURCEMANAGER_H__
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/rm/XLResourceManagerClient.h>
//#include <aalsdk/kernel/aalrm_client.h>


BEGIN_NAMESPACE(AAL)

//=============================================================================
// Constants and Events for methods in IResourceManagerClientService
//=============================================================================

//=============================================================================
// Name: ResourceManagerClientMessage
// Description: Dispatchable for generating ResourceManagerClient callbacks
// Comments: Use ThreadGroup or XL MDS to dispatch
//=============================================================================
class ResourceManagerClientMessage : public IDispatchable
{
public:
   enum ResourceManagerClientMessageType{
      Allocated,
      AllocateFailed,
      Exception
   };
   ResourceManagerClientMessage( IResourceManagerClient *po,
                                 NamedValueSet nvs,
                                 enum ResourceManagerClientMessageType type,
                                 const IEvent *pEvent)
   : m_pobject(po),
     m_nvs(nvs),
     m_type(type),
     m_pEvent(pEvent){}

   ResourceManagerClientMessage( IResourceManagerClient *po,
                                 NamedValueSet nvs,
                                 enum ResourceManagerClientMessageType type,
                                 AAL::TransactionID const &rTranID=TransactionID())
   : m_pobject(po),
     m_nvs(nvs),
     m_type(type),
     m_pEvent(NULL),
     m_rTranID(rTranID){}

   void  operator()()
   {
      switch(m_type)
      {
         case Allocated:
            m_pobject->resourceAllocated(m_nvs, m_rTranID);
            break;
         case AllocateFailed:
            m_pobject->resourceRequestFailed(m_nvs, *m_pEvent);
            break;
         case Exception:
            m_pobject->resourceManagerException(*m_pEvent);
            break;
         default:
            break;
      };

      // Delete the event if there is one
      if(NULL != m_pEvent) delete m_pEvent;

      // Clean up self
      delete this;
   }

virtual ~ResourceManagerClientMessage(){}

protected:
   IResourceManagerClient                 *m_pobject;
   NamedValueSet                           m_nvs;
   enum ResourceManagerClientMessageType   m_type;
   TransactionID const                     m_rTranID;
   IEvent const                           *m_pEvent;
};


//=============================================================================
// Name: IResourceManager
// Description: Public interface to the Resource Manager
//=============================================================================
class XLRESOURCEMANAGER_API IResourceManager
{
public:
   virtual ~IResourceManager(){};

   // Request a resource.
   virtual void RequestResource( NamedValueSet const &nvsManifest,
                                 TransactionID const &tid) = 0;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_RM_AASRESOURCEMANAGER_H__

