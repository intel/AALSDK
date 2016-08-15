// Copyright(c) 2015-2016, Intel Corporation
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
/// @file HelloAALService.cpp
/// @brief Implementation of HelloAALService - a Simple AFU Service.
/// @ingroup hello_service
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHOR: Joseph Grecco, Intel Corporation
/// This sample demonstrates how to create an AAL Service that is software only.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/10/2015     JG       Initial version@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Sleep.h>

#include <aalsdk/AALLoggerExtern.h>              // Logger
#include <aalsdk/utils/AALEventUtilities.h>      // Used for UnWrapAndReThrow()

#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory

#include "HelloAALService-internal.h"


//=============================================================================
// Typedefs and Constants
//=============================================================================

#ifndef HELLOAALSERVICE_VERSION_CURRENT
# define HELLOAALSERVICE_VERSION_CURRENT  4
#endif // HELLOAALSERVICE_VERSION_CURRENT
#ifndef HELLOAALSERVICE_VERSION_REVISION
# define HELLOAALSERVICE_VERSION_REVISION 2
#endif // HELLOAALSERVICE_VERSION_REVISION
#ifndef HELLOAALSERVICE_VERSION_AGE
# define HELLOAALSERVICE_VERSION_AGE      0
#endif // HELLOAALSERVICE_VERSION_AGE
#ifndef HELLOAALSERVICE_VERSION
# define HELLOAALSERVICE_VERSION          "4.2.0"
#endif // HELLOAALSERVICE_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef HELLOAAL_SERVICE_EXPORTS
#    define HELLOAAL_SERVICE_API __declspec(dllexport)
# else
#    define HELLOAAL_SERVICE_API __declspec(dllimport)
# endif // HELLOAAL_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define HELLOAAL_SERVICE_API    __declspec(0)
#endif // __AAL_WINDOWS__


// The following declarations implement the AAL Service factory and entry
//  point.

// Define the factory to use for this service. In this example the service
//  will be implemented in-process.  There are other implementations available for
//  services implemented remotely, for example via TCP/IP.
#define SERVICE_FACTORY AAL::InProcSvcsFact< HelloAALService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libhelloaalservice, HELLOAAL_SERVICE_API, HELLOAALSERVICE_VERSION, HELLOAALSERVICE_VERSION_CURRENT, HELLOAALSERVICE_VERSION_REVISION, HELLOAALSERVICE_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


/////////////////////////////////////////////////////////////////////////////
//////                                                                ///////
//////                                                                ///////
/////                         Hello AAL Service                        //////
//////                                                                ///////
//////                                                                ///////
/////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name: init()
// Description: Initialize the Service
// Interface: public
// Inputs: pclientBase - Pointer to the IBase for the Service Client
//         optArgs - Arguments passed to the Service
//         rtid - Transaction ID
// Outputs: none.
// Comments: Should only return False in case of severe failure that prevents
//           sending a response or calling initFailed.
//=============================================================================
btBool HelloAALService::init( IBase *pclientBase,
                              NamedValueSet const &optArgs,
                              TransactionID const &rtid)
{
   m_pClient = dynamic_ptr<IHelloAALClient>(iidSampleHelloAALClient, pclientBase);
   ASSERT( NULL != m_pClient ); //QUEUE object failed
   if(NULL == m_pClient){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Client did not publish IHelloAALClient Interface"));
      return true;
   }
   initComplete(rtid);
   return true;
}

//=============================================================================
// Name: Hello
// Description: Hello method send a reply
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void HelloAALService::Hello(btcString sMessage, TransactionID const &rTranID)
{
   AutoLock(this);

   MSG("Received a hello from '"<< sMessage << "'. Saying hello back.");
   getRuntime()->schedDispatchable(new HelloAppDispatchable(m_pClient, (IBase *)this, rTranID));

}


btBool HelloAALService::Release(TransactionID const &rTranID, btTime timeout)
{
   return ServiceBase::Release(rTranID, timeout);
}

HelloAppDispatchable::HelloAppDispatchable(IHelloAALClient      *pClient,
                                           IBase                *pHelloAALService,
                                           TransactionID const  &rTranID) :
   m_pSvcClient(pClient),
   m_pSevice(pHelloAALService),
   m_TranID(rTranID)
{}

void HelloAppDispatchable::operator() ()
{
   m_pSvcClient->HelloApp(m_TranID);
   delete this;
}


