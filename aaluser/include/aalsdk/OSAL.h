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
/// @file OSAL.h
/// @brief Master include file for OS Abstraction Layer
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/24/2013     TSW      Create convenience header for OSAL.@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_H__
#define __AALSDK_OSAL_H__
# include <aalsdk/AALDefs.h>
# include <aalsdk/AALTypes.h>


/// @addtogroup OSAL
/// @{

#define RESMGR_UTILSAFU_BACKDOOR(__cr, __mfst)                                 \
do                                                                             \
{                                                                              \
   __cr.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libUtilsAFU"); \
   __cr.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME,     "libAASUAIA");  \
   __mfst.Add(AAL_FACTORY_CREATE_SERVICENAME,                  "UtilsAFU");    \
}while(0)

#define RESMGR_SAMPLEENCODER_BACKDOOR(__cr, __mfst)                                           \
do                                                                                            \
{                                                                                             \
   __cr.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSampleEncoderAFUService"); \
   __cr.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME,     "libAASUAIA");                 \
   __cr.Add(AAL_FACTORY_CREATE_CONFIGRECORD_SERVICENAME,       "Encoder");                    \
   __mfst.Add(AAL_FACTORY_CREATE_SERVICENAME,                  "Encoder");                    \
}while(0)


#if DEPRECATED
#ifdef __AAL_LINUX__
//=============================================================================
// Macros and constants
//=============================================================================

// --- Increment --- long int version -----------------------------------------
inline long InterlockedIncrement (volatile long * pCounter)
{
   int result;

   asm volatile ("lock; xaddl %0, %1" :
		 "=r" (result),
		 "=m" (*pCounter) :
		 "0" (1),
		 "m" (*pCounter));

   return (long) result + 1;
}

// --- Decrement --- long int versions ----------------------------------------
inline long InterlockedDecrement (volatile long * pCounter)
{
   int result;

   asm volatile ("lock; xaddl %0, %1" :

		 "=r" (result),
		 "=m" (*pCounter) :
		  "0" (-1),
		  "m" (*pCounter));

   return (long) result - 1;
}

#endif // __AAL_LINUX__
#endif // DEPRECATED

/// Find the first bit set, scanning low to high.
/// @param[in] value is input bitmask
OSAL_API unsigned long FindLowestBitSet64(AAL::btUnsigned64bitInt value);

/// @}


# include <aalsdk/osal/Timer.h>
# include <aalsdk/osal/Sleep.h>
# include <aalsdk/osal/DynLinkLibrary.h>
# include <aalsdk/osal/CriticalSection.h>
# include <aalsdk/osal/OSSemaphore.h>
# include <aalsdk/osal/Barrier.h>
# include <aalsdk/osal/Thread.h>
# include <aalsdk/osal/ThreadGroup.h>
# include <aalsdk/osal/OSServiceModule.h>
# include <aalsdk/osal/OSALService.h>

#endif // __AALSDK_OSAL_H__

