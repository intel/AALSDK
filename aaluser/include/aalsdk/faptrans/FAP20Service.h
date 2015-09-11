// Copyright (c) 2013-2015, Intel Corporation
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
/// @file FAP20Service.h
/// @brief AAL Service Module definitions for FAP Transactions 2.0
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/12/2013     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __FAPTRANS_FAP20SERVICE_H__
#define __FAPTRANS_FAP20SERVICE_H__
#include <aalsdk/osal/OSServiceModule.h>

#define FAPTRANS2_SVC_MOD         "libFAPTrans2" AAL_SVC_MOD_EXT
#define FAPTRANS2_SVC_ENTRY_POINT "libFAPTrans2" AAL_SVC_MOD_ENTRY_SUFFIX

#define FAPTRANS2_BEGIN_MOD() AAL_BEGIN_MOD(libFAPTrans2, FAPTRANS2_API, FAPTRANS2_VERSION, FAPTRANS2_VERSION_CURRENT, FAPTRANS2_VERSION_REVISION, FAPTRANS2_VERSION_AGE)
#define FAPTRANS2_END_MOD()   AAL_END_MOD()

AAL_DECLARE_MOD(libFAPTrans2, FAPTRANS2_API)

#endif // __FAPTRANS_FAP20SERVICE_H__

