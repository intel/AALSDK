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
/// @file HWCCIv3AFUService.h
/// @brief AAL Service Module definitions for Hardware CCIv3 AFU
/// @ingroup HWCCIv3AFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifndef __SERVICE_HWCCIV3AFUSERVICE_H__
#define __SERVICE_HWCCIV3AFUSERVICE_H__
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/INTCDefs.h>

/// @addtogroup HWCCIv3AFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# ifdef HWCCIV3AFU_EXPORTS
#    define HWCCIV3AFU_API __declspec(dllexport)
# else
#    define HWCCIV3AFU_API __declspec(dllimport)
# endif // HWCCIV3AFU_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define HWCCIV3AFU_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define HWCCIV3AFU_SVC_MOD         "libHWCCIv3AFU" AAL_SVC_MOD_EXT
#define HWCCIV3AFU_SVC_ENTRY_POINT "libHWCCIv3AFU" AAL_SVC_MOD_ENTRY_SUFFIX

#define HWCCIV3AFU_BEGIN_SVC_MOD(__svcfactory) AAL_BEGIN_SVC_MOD(__svcfactory, libHWCCIv3AFU, HWCCIV3AFU_API, HWCCIV3AFU_VERSION, HWCCIV3AFU_VERSION_CURRENT, HWCCIV3AFU_VERSION_REVISION, HWCCIV3AFU_VERSION_AGE)
#define HWCCIV3AFU_END_SVC_MOD()               AAL_END_SVC_MOD()

AAL_DECLARE_SVC_MOD(libHWCCIv3AFU, HWCCIV3AFU_API)

/// CCIv3 Hardware AFU interface ID.
#define iidHWCCIv3AFU __INTC_IID(INTC_sysSampleAFU, 0x000f)

// 64-bit  AFU-ID: 00000000-0000-0000-9AEF-FE5F84570612
// 128-bit AFU-ID: C000C966-0D82-4272-9AEF-FE5F84570612

#define HWCCIV3AFU_MANIFEST \
"\t9 16 AAL_keyRegAFU_ID\n \
\t\t9 36 C000C966-0D82-4272-9AEF-FE5F84570612\n \
9 20 ConfigRecordIncluded\n \
\t10\n \
\t\t9 16 AAL_keyRegAFU_ID\n \
\t\t\t9 36 C000C966-0D82-4272-9AEF-FE5F84570612\n \
\t\t9 13 AIAExecutable\n \
\t\t\t9 10 libAASUAIA\n \
\t\t9 17 ServiceExecutable\n \
\t\t\t9 13 libHWCCIv3AFU\n \
9 29 ---- End of embedded NVS ----\n \
\t9999\n \
9 11 ServiceName\n \
\t9 10 HWCCIv3AFU\n"

/// @}

#endif // __SERVICE_HWCCIV3AFUSERVICE_H__

