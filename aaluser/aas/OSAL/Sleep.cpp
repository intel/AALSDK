// Copyright (c) 2007-2014, Intel Corporation
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
/// @file Sleep.cpp
/// @brief Time related functions
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Cleaned up windows includes
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/Sleep.h"

#ifdef __AAL_UNKNOWN_OS__
# error TODO: implement Sleep for unknown OS.
#endif // __AAL_UNKNOWN_OS__

#ifdef __AAL_LINUX__
# include <time.h>       // nanosleep
# include <unistd.h>     // usleep
#endif // __AAL_LINUX__


BEGIN_NAMESPACE(AAL)

static void SleepWorker(unsigned long seconds, unsigned long long nanoseconds)
{
   // if time is zero, call yield, don't set a time
   if ( ( 0UL == seconds ) && ( 0ULL == nanoseconds ) ) {
      SleepZero();
      return;
   }

   const unsigned long long oneBillion = 1000000000ULL;

   if ( nanoseconds >= oneBillion ) {
      seconds += (unsigned long)(nanoseconds / oneBillion);
      nanoseconds %= oneBillion;
   }

#if   defined( __AAL_WINDOWS__ )

   const unsigned long oneMillion = 1000000UL; // note that nanoseconds is now < oneBillion
   unsigned long milliseconds = ( seconds * 1000UL ) + ( (unsigned long)nanoseconds / oneMillion );

   ::Sleep(milliseconds); // Windows sleep

#elif defined( __AAL_LINUX__ )

   struct timespec tDelay;

   tDelay.tv_sec  = seconds;
   tDelay.tv_nsec = (long)nanoseconds;

#if 1

   nanosleep(&tDelay, NULL);

#else // use this if not sleeping long enough

   struct timespec tRemaining;
   while ( EINTR == nanosleep(&tDelay, &tRemaining) ) { // loop if interrupted
      tDelay = tRemaining;
   }

#endif

#endif // OS
}

void OSAL_API SleepZero(void)
{
#if   defined( __AAL_WINDOWS__ )
   ::Sleep(0); // Windows Yield
#elif defined( __AAL_LINUX__ )
   usleep(0);
#endif // OS
}

/////////////////////////////////////////////////////////////////////////////
//   NON - OS-Specific Functions Here
/////////////////////////////////////////////////////////////////////////////

// Sleep for some seconds
void OSAL_API SleepSec(unsigned long seconds)
{
   SleepWorker(seconds, 0ULL);
}

// Sleep for milliseconds
void OSAL_API SleepMilli(unsigned long msecs)
{
   SleepWorker(0UL, (unsigned long long)msecs * 1000000ULL);
}

// Sleep for microseconds
void OSAL_API SleepMicro(unsigned long usecs)
{
   SleepWorker(0UL, (unsigned long long)usecs * 1000ULL);
}

// Sleep for nanoseconds
void OSAL_API SleepNano(unsigned long nsecs)
{
   SleepWorker(0UL, nsecs);
}

END_NAMESPACE(AAL)

