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
//        FILE: rtlib.cpp
//     CREATED: Feb 28, 2014
//      AUTHOR: Joseph Grecco
//
// PURPOSE: This file contains the top level implementation of the AAL Runtime
//          library.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 06/25/2015     JG       Removed XL from name
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"
#include "aalsdk/aas/AALRuntimeModule.h"

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning(disable:68)         // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:383)        // remark: value copied to temporary, reference to temporary used
   #pragma warning(disable:869)        // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)        // remark: operands are evaluated in unspecified order
   #pragma warning(disable:1418)       // remark: external function definition with no prior declaration
   #pragma warning(disable:1419)       // remark: external declaration in primary source file
//   #pragma warning(disable:1572)       // remark: floating-point equality and inequality comparisons are unreliable
#endif

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_MOD(libAALRUNTIME, AALRUNTIME_API, AALRUNTIME_VERSION, AALRUNTIME_VERSION_CURRENT, AALRUNTIME_VERSION_REVISION, AALRUNTIME_VERSION_AGE)
   // Only default commands for now.
AAL_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


#ifdef __AAL_WINDOWS__

volatile AAL::btBool DllProcAttached = true; // Used in Windows for Assassin cleanup

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
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         DllProcAttached = false;
      break;
   }
   return TRUE;
}
#endif // __AAL_WINDOWS__

