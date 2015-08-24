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
//        FILE: IServiceBroker.h
//     CREATED: Mar 14, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for the public IServiceBroker interface
// HISTORY:
// COMMENTS:  Service Broker is responsibel for servicing requests for Service
//            allocation. It interacts with components such as the Resource
//            manager.
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __ISERVICEBROKER_H__
#define __ISERVICEBROKER_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/IServiceClient.h>

BEGIN_NAMESPACE(AAL)

/// @ingroup ServiceBroker
/// @{
///
/// Interface to the Service Broker Platform Service.
///
//AAL implements a Factory design pattern to abstract the instantiation of Services from the application.
//The AAL Service Broker provides the interface used to request a Service. The Service Broker interacts with
//the Registrar to determine how a Service is to be instantiated.
/// Service Brokers are Platform Services loaded by the AAL Runtime object.
class IServiceBroker
{
public:

   /// Allocate a Service
   ///@param[in] pProxy Runtime Proxy for this Service
   ///@param[in] pRuntimClient of client for this Service
   ///@param[in] pServiceClient IBase of client for this Service
   ///@param[in] rManifest Manifest describing the Service to allocate
   ///@param[in] rTranID Trasnaction ID
   ///@param[in] eAllocatemode Allocation mode. NoRuntimeClientNotification will squelch the notification to the Runtime.
   virtual void allocService( IRuntime               *pProxy,
                              IRuntimeClient         *pRuntimClient,
                              IBase                  *pServiceClientBase,
                              const NamedValueSet    &rManifest,
                              TransactionID const    &rTranID) =0;

   virtual ~IServiceBroker(){}
};

///@}

END_NAMESPACE(AAL)

#endif /* ISERVICEBROKER_H_ */

