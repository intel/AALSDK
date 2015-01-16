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
//        FILE: ServiceHost.h
//     CREATED: Mar 22, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Wrapper class for Service Host plug-ins
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __SERVICEHOST_H__
#define __SERVICEHOST_H__
#include <aalsdk/CUnCopyable.h>
#include <aalsdk/osal/DynLinkLibrary.h>
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/aas/AALServiceModule.h>

#include "aalsdk/aas/_xlRuntimeServices.h"

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(XL)
      BEGIN_NAMESPACE(RT)

//=============================================================================
// Name: ServiceHost
// Description: Wrapper for service plug-ins
// Interface: public
// Comments:
//=============================================================================
class XLRT_API ServiceHost : private CUnCopyable
{
public:
   ServiceHost( btcString root_name, 
                AAL::XL::RT::IRuntime *,
                AAL::XL::RT::IXLRuntimeServices  *);
   ServiceHost( AALSvcEntryPoint EP, 
                AAL::XL::RT::IRuntime *,
                AAL::XL::RT::IXLRuntimeServices  *);

   btBool IsOK() const {return m_bIsOK;}
   ~ServiceHost(){}

   btBool allocService( AAL::IBase *pClient,
                        NamedValueSet const &rManifest = NamedValueSet(),
                        TransactionID const &rTranID = TransactionID(),
                        AAL::btBool          NoRuntimeEvent = false);

   std::string const &getName() const                 { return m_name; }
   AAL::IBase * getIBase()const                       { return m_base; };
   operator AAL::IBase *()const                       { return m_base; }
   AAL::AAS::IServiceModule *getProvider()const       { return m_pProvider; }

private:
   btBool                         m_bIsOK;
   DynLinkLibrary                *m_pDynLinkLib;
   AAL::AAS::IServiceModule      *m_pProvider;
   AAL::IBase                    *m_base;
   OSServiceModule                m_modparms;
   std::string                    m_name;
   ServiceHost(){};
};


      END_NAMESPACE(RT)
   END_NAMESPACE(XL)
END_NAMESPACE(AAL)


#endif // __SERVICEHOST_H__