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
//        FILE: xlruntime.cpp
//     CREATED: Mar 3, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: Implementation of XL Runtime classes
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// XL Runtime definitions
#include "aalsdk/AALTypes.h"
#include "aalsdk/xlRuntime.h"
#include "_xlRuntimeImpl.h"
#include "aalsdk/CAALEvent.h"

/// @addtogroup XLRuntime
/// @{

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(XL)
      BEGIN_NAMESPACE(RT)

//=============================================================================
/// Default Constructor of Runtime class
///
/// Need to call start() after construction to actually get the
///   runtime initialized and functional
//=============================================================================
Runtime::Runtime() :
   m_pImplementation(NULL),
   m_status(false)
{
   m_pImplementation = _getnewXLRuntimeInstance();
   m_status          = m_pImplementation != NULL;
}

//=============================================================================
/// Checks status of object
///
/// @return       true if functional.
//=============================================================================
btBool Runtime::IsOK()
{
   return m_pImplementation->IsOK();
}

//=============================================================================
/// Starts the XL Runtime implementation after initial construction
///
/// @param[in]    pClient        Pointer to the Runtime's client object (IBase).
/// @param[in]    rConfigParms   Configuration parameter NVS
/// @return       True if successful. Possible to fail, e.g. the pClient does
///                  not contain an IRuntimeClient to call back. But more
///                  typically start() results in a call back to IRuntimeClient
///                  runtimeStarted() or runtimeStartFailed().
//=============================================================================
AAL::btBool  Runtime::start( AAL::IBase *pClient,
                             const NamedValueSet &rConfigParms)
{
   // Try and start the system
   if ( IsOK() ) {
      return m_pImplementation->start(pClient, rConfigParms);
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
      m_pImplementation->stop();
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
void Runtime::allocService(
      AAL::IBase                            *pClient,
      NamedValueSet const                   &rManifest,
      TransactionID const                   &rTranID,
      AAL::XL::RT::IRuntime::eAllocatemode   mode)
{
   if ( IsOK() ) {
      m_pImplementation->allocService(pClient, rManifest, rTranID, mode);
   }
}

//=============================================================================
/// Schedule a Dispatchable object for dispatch (i.e. delivery to its target function)
///
/// @param[in]    pdispatchable  Dispatchable object
/// @return       void
//=============================================================================
void Runtime::schedDispatchable(IDispatchable *pdispatchable)
{
   if (IsOK()) {
      m_pImplementation->schedDispatchable(pdispatchable);
   }

}

//=============================================================================
/// Virtual Destructor with empty implementation
//=============================================================================
Runtime::~Runtime() {/*empty*/}


      END_NAMESPACE(RT)
   END_NAMESPACE(XL)
END_NAMESPACE(AAL)

/// @} group XLRuntime


