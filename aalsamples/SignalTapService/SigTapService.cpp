// Copyright (c) 2015, Intel Corporation
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
/// @file SigTapService.cpp
/// @brief Implementation of the CCIP SignalTap Service.
/// @ingroup SigTapService
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          
///
/// This sample demonstrates how to create an AFU Service that uses a the ALI HW
///  Service to access the SignalTap debug hardware..
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/19/2015     JG       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>

#include <aalsdk/service/ICCIClient.h>
#include "SigTapService.h"

#ifdef INFO
# undef INFO
#endif // INFO
#if 0
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif
#ifdef ERR
# undef ERR
#endif // ERR
#if 1
# define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define ERR(x)
#endif


//=============================================================================
// Typedefs and Constants
//=============================================================================
#if 0
#ifndef SIGTAPSERVICE_VERSION_CURRENT
# define SIGTAPSERVICE_VERSION_CURRENT  4
#endif // SIGTAPSERVICE_VERSION_CURRENT
#ifndef SIGTAPSERVICE_VERSION_REVISION
# define SIGTAPSERVICE_VERSION_REVISION 2
#endif // SIGTAPSERVICE_VERSION_REVISION
#ifndef SIGTAPSERVICE_VERSION_AGE
# define SIGTAPSERVICE_VERSION_AGE      0
#endif // SIGTAPSERVICE_VERSION_AGE
#ifndef SIGTAPSERVICE_VERSION
# define SIGTAPSERVICE_VERSION          "4.2.0"
#endif // SIGTAPSERVICE_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef SIGTAP_SERVICE_EXPORTS
#    define SIGTAP_SERVICE_API __declspec(dllexport)
# else
#    define SIGTAP_SERVICE_API __declspec(dllimport)
# endif // SIGTAP_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define SIGTAP_SERVICE_API    __declspec(0)
#endif // __AAL_WINDOWS__


// The following declarations implement the AAL Service factory and entry
//  point.

// Define the factory to use for this service. In this example the service
//  will be implemented in-process.  There are other implementations available for
//  services implemented remotely, for example via TCP/IP.
#define SERVICE_FACTORY AAL::InProcSvcsFact< SigTapService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libsigtapservice, SIGTAP_SERVICE_API, SIGTAPSERVICE_VERSION, SIGTAPSERVICE_VERSION_CURRENT, SIGTAPSERVICE_VERSION_REVISION, SIGTAPSERVICE_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()
#endif

USING_NAMESPACE(AAL)

btBool SigTapService::init( IBase               *pclientBase,
                            NamedValueSet const &optArgs,
                            TransactionID const &TranID)
{
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase());
   ASSERT( NULL != pClient );
   if ( NULL == pClient ) {
      /// ObjectCreatedExceptionEvent Constructor.
      initFailed( new CExceptionTransactionEvent( this,
                                                  TranID,
                                                  errBadParameter,
                                                  reasMissingInterface,
                                                  "Client did not publish ICCIClient Interface") );
      return false;
   }



   initComplete(TranID);
   return true;
}

btBool SigTapService::Release(TransactionID const &TranID, btTime timeout)
{
   return ServiceBase::Release(TranID, timeout);
}


#if defined( __AAL_WINDOWS__ )

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
         break;
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

