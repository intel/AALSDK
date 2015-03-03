// Copyright (c) 2007-2015, Intel Corporation
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
/// @file AASRegistrar.cpp
/// @brief AASRegistrar - Implements DLL entry points (DLLmain) in Windows.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/06/2007     HM       Initial Version Created
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"
#include "aalsdk/registrar/AASRegistrarService.h"
#include "aalsdk/registrar/CAASRegistrar.h"


#ifdef __AAL_WINDOWS__

//=============================================================================
// Name: DllMain
// Description: Windows dll entry points.
// Interface: public
// Inputs: hModule - Module (or instance) handle. IDs the instance of the DLL
//         ul_reason_for_call - Type of attach or detach
// Outputs: Success or failure.
// Comments:
//=============================================================================
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   HANDLE hTemp  = hModule;
   LPVOID lpVoid = lpReserved;
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
      case DLL_THREAD_ATTACH  :
      case DLL_THREAD_DETACH  :
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__


AASREGISTRAR_API AAL::CRegistrar *
CreateRegistrarService(AAL::btcString            DatabasePath,
                       AAL::btEventHandler       theEventHandler,
                       AAL::btApplicationContext Context,
                       AAL::btcObjectType        _tranID,
                       AAL::btcObjectType        _optArgs);


BEGIN_C_DECLS

static AAL::bt32bitInt CreateRegistrar(struct RegistrarCreateParms *pCreateParms)
{
   pCreateParms->Result = CreateRegistrarService(pCreateParms->DatabasePath,
                                                 pCreateParms->theEventHandler,
                                                 pCreateParms->Context,
                                                 pCreateParms->tranID,
                                                 pCreateParms->optArgs);
   return (NULL == pCreateParms->Result ? -2 : 0);
}

END_C_DECLS


#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AASREGISTRAR_BEGIN_MOD()
   AAL_BEGIN_SVC_MOD_CMD(AASREGISTRAR_SVC_CMD_CREATE_REGISTRAR)

      res = CreateRegistrar((struct RegistrarCreateParms *)arg);

   AAL_END_SVC_MOD_CMD()
AASREGISTRAR_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

