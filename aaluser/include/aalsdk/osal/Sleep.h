// Copyright(c) 2008-2016, Intel Corporation
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
/// @file Sleep.h
/// @brief Encapsulate various Sleep calls.
/// @ingroup OSAL
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/26/2008     HM       Created
/// 02/03/2008     HM       Removed extraneous ;
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_SLEEP_H__
#define __AALSDK_OSAL_SLEEP_H__
#include <aalsdk/AALDefs.h>

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

/// @addtogroup OSAL
/// @{

/// Put the calling thread to sleep for the specified time in seconds.
/// @param[in] secs The number of seconds the thread will sleep.
/// @return void
void OSAL_API SleepSec(unsigned long secs);

// Put the calling thread to sleep for the specified time in milliseconds.
/// @param[in] msecs The number of milliseconds the thread will sleep.
/// @return void
void OSAL_API SleepMilli(unsigned long msecs);

/// Put the calling thread to sleep for the specified time in microseconds.
/// @param[in] usecs The number of microseconds the thread will sleep.
/// @return void

void OSAL_API SleepMicro(unsigned long usecs);
/// Put the calling thread to sleep for the specified time in nanoseconds.
/// @param[in] nsecs The number of nanoseconds the thread will sleep.
/// @return void

void OSAL_API SleepNano(unsigned long nsecs);
/// Yield the CPU.
/// @return void
void OSAL_API SleepZero(void);

/// @}

END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_OSAL_SLEEP_H__

