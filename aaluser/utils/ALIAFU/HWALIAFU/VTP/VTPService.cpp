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
/// @file VTPService.cpp
/// @brief Implementation of IVTPService.
/// @ingroup vtp_service
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
/// Virtual-to-Physical address translation service
///
/// TODO: add verbose description
///
/// Provides service for access to the VTP BBB for address translation.
/// Assumes a VTP BBB DFH to be detected and present.
///
/// On initialization, allocates shared buffer for VTP page hash and
/// communicates its location to VTP BBB.
///
/// Provides synchronous methods to update page hash on shared buffer
/// allocation.
///
/// Does not have an explicit client callback interface, as all published
/// service methods are synchronous.
///
/// AUTHORS: Enno Luebbers, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/15/2016     EL       Initial version@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Sleep.h>

#include <aalsdk/AALLoggerExtern.h>              // Logger
#include <aalsdk/utils/AALEventUtilities.h>      // Used for UnWrapAndReThrow()

#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory

#include "VTPService-internal.h"


//=============================================================================
// Typedefs and Constants
//=============================================================================

#ifndef VTPSERVICE_VERSION_CURRENT
# define VTPSERVICE_VERSION_CURRENT  5
#endif // VTPSERVICE_VERSION_CURRENT
#ifndef VTPSERVICE_VERSION_REVISION
# define VTPSERVICE_VERSION_REVISION 0
#endif // VTPSERVICE_VERSION_REVISION
#ifndef VTPSERVICE_VERSION_AGE
# define VTPSERVICE_VERSION_AGE      0
#endif // VTPSERVICE_VERSION_AGE
#ifndef VTPSERVICE_VERSION
# define VTPSERVICE_VERSION          "5.0.0"
#endif // VTPSERVICE_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef VTP_SERVICE_EXPORTS
#    define VTP_SERVICE_API __declspec(dllexport)
# else
#    define VTP_SERVICE_API __declspec(dllimport)
# endif // VTP_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define VTP_SERVICE_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define SERVICE_FACTORY AAL::InProcSvcsFact< VTPService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libVTPService, VTP_SERVICE_API, VTPSERVICE_VERSION, VTPSERVICE_VERSION_CURRENT, VTPSERVICE_VERSION_REVISION, VTPSERVICE_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


/////////////////////////////////////////////////////////////////////////////
//////                                                                ///////
//////                                                                ///////
/////                            VTP Service                           //////
//////                                                                ///////
//////                                                                ///////
/////////////////////////////////////////////////////////////////////////////


/// @addtogroup vtp_service
/// @{

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
btBool VTPService::init( IBase *pclientBase,
                              NamedValueSet const &optArgs,
                              TransactionID const &rtid)
{
   btObjectType tmp;

   // check for HWALIAFU's IBase in optargs
   if ( ENamedValuesOK != optArgs.Get(HWALIAFU_IBASE, &tmp) ) {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingParameter,
                                                 "No HWALIAFU IBase in optArgs."));
      return true;
   }
   m_pHWALIAFU = reinterpret_cast<IBase *>(tmp);

   // check for VTP MMIO base in optargs
   if ( ENamedValuesOK != optArgs.Get(VTP_DFH_BASE, &tmp) ) {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingParameter,
                                                 "No VTP DFH base address in optArgs."));
      return true;
   }
   m_pDFHBaseAddr = reinterpret_cast<btVirtAddr>(tmp);

   initComplete(rtid);
   return true;
}

btBool VTPService::Release(TransactionID const &rTranID, btTime timeout)
{
   // freedom to all buffers!
   // bufferFreeAll();
   return ServiceBase::Release(rTranID, timeout);
}

ali_errnum_e VTPService::bufferAllocate( btWSSize       Length,
                                         btVirtAddr    *pBufferptr,
                                         NamedValueSet *pOptArgs) {
}

btPhysAddr   VTPService::bufferGetIOVA(  btVirtAddr     Address) {
}

/// @}


