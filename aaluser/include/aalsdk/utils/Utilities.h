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
/// @file Utilities.h
/// @brief Handy little miscellaneous things needed in lots of places.
/// @ingroup AASUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/04/2008     HM       Initial version started
/// 11/20/2008     JG       Added SmartPtr home brew
/// 11/27/2008     HM       Added RDTSC (standard Intel code), + getTSC home brew
/// 01/04/2009     HM       Updated Copyright
/// 10/21/2011     JG       Added __linux__ for Windows port
/// 10/12/2014     HM       Moved memory #defines in from SingleAFUApp.h@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_UTILITIES_H__
#define __AALSDK_UTILS_UTILITIES_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup AASUtils
/// @{

#ifndef CACHELINE_BYTES
# define CACHELINE_BYTES 64
#endif // CACHELINE_BYTES
#ifndef LOG2_CL
# define LOG2_CL         6
#endif // LOG2_CL
#ifndef CACHELINE_ALIGNED_ADDR
#define CACHELINE_ALIGNED_ADDR(p) ((p) >> LOG2_CL)
#endif // CACHELINE_ALIGNED_ADDR
#ifndef CL
# define CL(x) ((x)   * CACHELINE_BYTES)
#endif // CL
#ifndef KB
# define KB(x) ((x)   * __UINT64_T_CONST(1024))
#endif // KB
#ifndef MB
# define MB(x) (KB(x) * __UINT64_T_CONST(1024))
#endif // MB
#ifndef GB
# define GB(x) (MB(x) * __UINT64_T_CONST(1024))
#endif // GB


#if !defined(NUM_ELEMENTS)
#  define NUM_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#endif

//#if __GNUC__ >= 3
//#  if !defined(likely)
//#     define likely(x)   __builtin_expect (!!(x), 1)
//#  endif
//#  if !defined(unlikely)
//#     define unlikely(x) __builtin_expect (!!(x), 0)
//#  endif
//#else
//#  if !defined(likely)
//#     define likely(x)   (x)
//#  endif
//#  if !defined(unlikely)
//#     define unlikely(x) (x)
//#  endif
//#endif

// Read Time Stamp Clock atomically
#ifndef RDTSC
#if defined( __GNUC__ )
	#define RDTSC(low, high) \
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#else
	#define RDTSC(low, high)
#endif
#endif

/// Read Time Stamp Clock unified. Doesn't affect the RDTSC, but makes math easier.
static inline unsigned long long getTSC(void)
{
   unsigned low, high;
   RDTSC( low, high);
   return static_cast<unsigned long long>(low) | (static_cast<unsigned long long>(high) << 32);
}

/// @}

#endif // __AALSDK_UTILS_UTILITIES_H__

