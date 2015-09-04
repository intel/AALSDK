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
//        FILE: runtime.cpp
//     CREATED: Mar 3, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: Implementation of AAL Runtime classes
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 06/25/2015     JG       Removed XL from name
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// AAL Runtime definitions
#include "aalsdk/AALTypes.h"
#include "aalsdk/Runtime.h"
#include "_RuntimeImpl.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/Dispatchables.h"

/// @addtogroup AAL Runtime
/// @{

BEGIN_NAMESPACE(AAL)

//=============================================================================
/// Constructor of Runtime class
///
/// Need to call start() after construction to actually get the
///   runtime initialized and functional
//=============================================================================
Runtime::Runtime(IRuntimeClient *pClient) :
// : Runtime(pClient, true)  //C++11 only
m_pImplementation(NULL),
m_status(false),
m_pClient(pClient)

{
   init(pClient, true);
}

//=============================================================================
/// Constructor of Runtime class
///
/// Need to call start() after construction to actually get the
///   runtime initialized and functional
//=============================================================================
Runtime::Runtime(IRuntimeClient *pClient, btBool bFirstTime) :
// : Runtime(pClient, true)  //C++11 only
 m_pImplementation(NULL),
 m_status(false),
 m_pClient(pClient)
{
   init(pClient, bFirstTime);
}

//=============================================================================
/// Initializer of Runtime class
///
/// Need to call start() after construction to actually get the
///   runtime initialized and functional
//=============================================================================
void Runtime::init(IRuntimeClient *pClient, btBool bFirstTime)
 {
   // Must have a RuntimeClient
   if(NULL == pClient){
      return;
   }

   // Get the Runtime instance
   m_pImplementation = _getnewRuntimeInstance(this, pClient, bFirstTime);

   // If failed _getnewRuntimeInstance() will generate the message
   if(NULL == m_pImplementation){
      return;
   }
   m_status = m_pImplementation != NULL ? true : false;

   // Register interfaces
   // Add the public interfaces
   if ( SetSubClassInterface(iidRuntime, dynamic_cast<IRuntime *>(this)) != EObjOK ) {
      m_status = false;
      return;
   }
}

//=============================================================================
/// Checks status of object
///
/// @return       true if functional.
//=============================================================================
btBool Runtime::IsOK()
{
   // If either are bad return false
   return m_status && m_pImplementation->IsOK();
}

//=============================================================================
/// Starts the AAL Runtime implementation after initial construction
///
/// @param[in]    pClient        Pointer to the Runtime's client object (IBase).
/// @param[in]    rConfigParms   Configuration parameter NVS
/// @return       True if successful. Possible to fail, e.g. the pClient does
///                  not contain an IRuntimeClient to call back. But more
///                  typically start() results in a call back to IRuntimeClient
///                  runtimeStarted() or runtimeStartFailed().
//=============================================================================
btBool  Runtime::start(const NamedValueSet &rConfigParms)
{
   // Try and start the system
   if ( IsOK() ) {
      return m_pImplementation->start(this, rConfigParms);
   }
   return false;
}

//=============================================================================
/// Stop the runtime
/// @return       void
//=============================================================================
void Runtime::stop()
{
   if ( IsOK() ) {
      m_pImplementation->stop(this);
   }
}

//=============================================================================
/// Allocate a service
///
/// @param[in]    pClient        IBase of owner, containing an IServiceClient
/// @param[in]    rManifest      Optional manifest defining the service
/// @param[in]    rTranID        Optional transactionID
/// @param[in]    mode           Option mode whether to notify just the service
///                                 or also the runtime client
/// @return       void
//=============================================================================
void Runtime::allocService( IBase                  *pClient,
                            NamedValueSet const    &rManifest,
                            TransactionID const    &rTranID)
{
   if ( IsOK() ) {
      m_pImplementation->allocService(this, pClient, rManifest, rTranID);
   }
}

//=============================================================================
/// Schedule a Dispatchable object for dispatch (i.e. delivery to its target function)
///
/// @param[in]    pdispatchable  Dispatchable object
/// @return       void
//=============================================================================
btBool Runtime::schedDispatchable(IDispatchable *pdispatchable)
{
   if (IsOK()) {
      m_pImplementation->schedDispatchable(pdispatchable);
   }

}

//=============================================================================
/// Get a new pointer to the Runtime
///
/// @param[in]    pClient - Pointer to client for Proxy
/// @return       void
//=============================================================================
IRuntime *Runtime::getRuntimeProxy(IRuntimeClient *pClient)
{
   // Create the new proxy storing this as the parent. Last parameter indicates
   //  that this is not the first time
   Runtime *newProxy = new Runtime(pClient, false);
   if(!newProxy->IsOK()){

   }
   return newProxy;
}

//=============================================================================
// Name: getRuntimeClient
// Description: return the Runtime's Client's Interface
// Interface: public
// Inputs: pobject - Dispatchable object to send
//         parm - Parameter
// Outputs: Pointer to client interface.
// Comments:
//=============================================================================
IRuntimeClient *Runtime::getRuntimeClient()
{
   AutoLock(this);
   return m_pClient;
}

//=============================================================================
/// Get a new pointer to the Runtime
///
/// @param[in]    pRuntime - Pointer to Proxy to release
/// @return       true - Success
//=============================================================================
btBool Runtime::releaseRuntimeProxy()
{
   AutoLock(this);

   delete this;
   return true;
}

//=============================================================================
/// Destructor releases all proxies created though this object and releases
///   the Runtime Instance
//=============================================================================
Runtime::~Runtime()
{
   m_pImplementation->releaseRuntimeInstance(this);
}


END_NAMESPACE(AAL)

/// @} group AALRUNTIME


