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
//        FILE: _xlRuntimeServices.h
//     CREATED: Mar 27, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Interface to accessor and mutators for XLRuntime services
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __XLRUNTIMESERVICES_H__
#define __XLRUNTIMESERVICES_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALBase.h>
#include <aalsdk/osal/IDispatchable.h>

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(XL)
     BEGIN_NAMESPACE(RT)

class XLRT_API IXLRuntimeServices
{
public:

   virtual AAL::IBase *getMessageDeliveryService()                               =0;
   virtual void setMessageDeliveryService(AAL::IBase *pMDSbase)                  =0;
   virtual btBool SendMsg(IDispatchable *pobject, btObjectType parm=NULL)        =0;
   virtual IRuntimeClient *getRuntimeClient()                                    =0;

   virtual ~IXLRuntimeServices(){}
};

      END_NAMESPACE(RT)
   END_NAMESPACE(XL)
END_NAMESPACE(AAL)

#endif /* XLRUNTIMESERVICES_H_ */