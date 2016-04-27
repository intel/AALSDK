// Copyright(c) 2003-2016, Intel Corporation
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
/// @file Timer.cpp
/// @brief Time related functions
/// @ingroup OSAL
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Charlie Lasswell, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
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

#include "aalsdk/osal/Timer.h"

BEGIN_NAMESPACE(AAL)

#if defined( __AAL_WINDOWS__ )
LARGE_INTEGER Timer::sm_ClockFreq = { 0, 0 };
#endif // __AAL_WINDOWS__

Timer::Timer()
{
#if   defined( __AAL_WINDOWS__ )
   m_Start.QuadPart = 0;
   if ( 0 == Timer::sm_ClockFreq.QuadPart ) {
      QueryPerformanceFrequency(&Timer::sm_ClockFreq);
   }
   QueryPerformanceCounter(&m_Start);
#elif defined( __AAL_LINUX__ )
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec = tv.tv_usec * 1000;
#endif // OS
}

Timer Timer::Now() const { return Timer(); }

Timer Timer::Add(const Timer &other) const
{
#if   defined( __AAL_WINDOWS__ )
   LARGE_INTEGER i;

   i.QuadPart = m_Start.QuadPart + other.m_Start.QuadPart;

   return Timer(&i);
#elif defined( __AAL_LINUX__ )
   struct timespec ts;

   ts.tv_sec  = m_Start.tv_sec  + other.m_Start.tv_sec;
   ts.tv_nsec = m_Start.tv_nsec + other.m_Start.tv_nsec;

   ts.tv_sec  += ts.tv_nsec / ( 1000 * 1000 * 1000 );
   ts.tv_nsec %= 1000 * 1000 * 1000;

   return Timer(&ts);
#endif // OS
}

Timer Timer::Subtract(const Timer &other) const
{
#if   defined( __AAL_WINDOWS__ )
   LARGE_INTEGER i;

   i.QuadPart = m_Start.QuadPart - other.m_Start.QuadPart;

   return Timer(&i);
#elif defined( __AAL_LINUX__ )
   unsigned        sec;
   unsigned        nsec;
   struct timespec m; // minuend
   struct timespec s; // subtrahend
   struct timespec d; // difference

   m = m_Start;
   s = other.m_Start;

   if ( m.tv_nsec < s.tv_nsec ) {
      // Prevent underflow by adding to tv_sec.
      sec  = ((s.tv_nsec - m.tv_nsec) / (1000 * 1000 * 1000)) + 1;
      nsec = sec * (1000 * 1000 * 1000);
      s.tv_nsec -= nsec;
      s.tv_sec  += sec;
   }

   if ( (m.tv_nsec - s.tv_nsec) > (1000 * 1000 * 1000) ) {
      // Balance any non-canonical tv_nsec value.
      sec  = (m.tv_nsec - s.tv_nsec) / (1000 * 1000 * 1000);
      nsec = sec * (1000 * 1000 * 1000);
      s.tv_nsec += nsec;
      s.tv_sec  -= sec;
   }

   d.tv_sec  = m.tv_sec  - s.tv_sec;
   d.tv_nsec = m.tv_nsec - s.tv_nsec;

   return Timer(&d);
#endif // OS
}

int Timer::Compare(const Timer &other) const
{
#if   defined( __AAL_WINDOWS__ )

   return (int)(m_Start.QuadPart - other.m_Start.QuadPart);

#elif defined( __AAL_LINUX__ )

   if ( m_Start.tv_sec == other.m_Start.tv_sec ) {
      return m_Start.tv_nsec - other.m_Start.tv_nsec;
   }

   return m_Start.tv_sec - other.m_Start.tv_sec;

#endif // __AAL_LINUX__
}

void Timer::AsSeconds(double &d) const
{
#if   defined( __AAL_WINDOWS__ )
   // Counts / (Counts/Sec)
   d = (double)(m_Start.QuadPart) / (double)(Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   double s = (double)m_Start.tv_sec;
   double n = (double)m_Start.tv_nsec;
   n *= 1.0e-9;
   d = s + n;
#endif // OS
}

void Timer::AsMilliSeconds(double &d) const
{
#if   defined( __AAL_WINDOWS__ )
   d = (1.0e+3 * (double)(m_Start.QuadPart)) / (double)(Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   double s = (double)m_Start.tv_sec;
   double n = (double)m_Start.tv_nsec;
   s *= 1.0e+3;
   n *= 1.0e-6;
   d = s + n;
#endif // OS
}

void Timer::AsMicroSeconds(double &d) const
{
#if   defined( __AAL_WINDOWS__ )
   d = (1.0e+6 * (double)(m_Start.QuadPart)) / (double)(Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   double s = (double)m_Start.tv_sec;
   double n = (double)m_Start.tv_nsec;
   s *= 1.0e+6;
   n *= 1.0e-3;
   d = s + n;
#endif // OS
}

void Timer::AsNanoSeconds(double &d) const
{
#if   defined( __AAL_WINDOWS__ )
   d = (1.0e+9 * (double)(m_Start.QuadPart)) / (double)(Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   double s = (double)m_Start.tv_sec;
   double n = (double)m_Start.tv_nsec;
   s *= 1.0e+9;
   d = s + n;
#endif // OS
}

void Timer::AsSeconds(btUnsigned64bitInt &u) const
{
#if   defined( __AAL_WINDOWS__ )
   u = (btUnsigned64bitInt) (m_Start.QuadPart / Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   btUnsigned64bitInt s = (btUnsigned64bitInt)m_Start.tv_sec;
   btUnsigned64bitInt n = (btUnsigned64bitInt)m_Start.tv_nsec;
   n /= (1000ULL * 1000ULL * 1000ULL);
   u = s + n;
#endif // OS
}

void Timer::AsMilliSeconds(btUnsigned64bitInt &u) const
{
#if   defined( __AAL_WINDOWS__ )
   u = (btUnsigned64bitInt) ((m_Start.QuadPart * 1000) / Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   btUnsigned64bitInt s = (btUnsigned64bitInt)m_Start.tv_sec;
   btUnsigned64bitInt n = (btUnsigned64bitInt)m_Start.tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   u = s + n;
#endif // OS
}

void Timer::AsMicroSeconds(btUnsigned64bitInt &u) const
{
#if   defined( __AAL_WINDOWS__ )
   u = (btUnsigned64bitInt) ((m_Start.QuadPart * 1000000) / Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   btUnsigned64bitInt s = (btUnsigned64bitInt)m_Start.tv_sec;
   btUnsigned64bitInt n = (btUnsigned64bitInt)m_Start.tv_nsec;
   s *= (1000ULL * 1000ULL);
   n /= 1000ULL;
   u = s + n;
#endif // OS
}

void Timer::AsNanoSeconds(btUnsigned64bitInt &u) const
{
#if   defined( __AAL_WINDOWS__ )
   u = (btUnsigned64bitInt) ((m_Start.QuadPart * 1000000000) / Timer::sm_ClockFreq.QuadPart);
#elif defined( __AAL_LINUX__ )
   btUnsigned64bitInt s = (btUnsigned64bitInt)m_Start.tv_sec;
   btUnsigned64bitInt n = (btUnsigned64bitInt)m_Start.tv_nsec;
   s *= (1000ULL * 1000ULL * 1000ULL);
   u = s + n;
#endif // OS
}

std::string Timer::NormalizedUnits() const
{
#if   defined( __AAL_WINDOWS__ )
   LARGE_INTEGER sec;
   LARGE_INTEGER nanos;
   sec.QuadPart   = m_Start.QuadPart / Timer::sm_ClockFreq.QuadPart;
   nanos.QuadPart = (m_Start.QuadPart * 1000000000) / Timer::sm_ClockFreq.QuadPart;

   if( sec.QuadPart >= 60 * 60 ) {
      std::string str("Hour");
      if ( sec.QuadPart > 60 * 60 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( sec.QuadPart >= 60 ) {
      std::string str("Minute");
      if ( sec.QuadPart > 60 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( sec.QuadPart > 0 ) {
      std::string str("Second");
      if ( sec.QuadPart > 1 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( nanos.QuadPart >= 1000 * 1000 ) {
      std::string str("MilliSecond");
      if ( nanos.QuadPart > 1000 * 1000 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( nanos.QuadPart >= 1000 ) {
      std::string str("MicroSecond");
      if ( nanos.QuadPart > 1000 ) {
         str += std::string("s");
      }
      return str;
   }

   std::string str("NanoSecond");
   if ( (nanos.QuadPart > 1) || (0 == nanos.QuadPart) ) {
      str += std::string("s");
   }

   return str;
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_sec >= 60 * 60 ) {
      std::string str("Hour");
      if ( m_Start.tv_sec > 60 * 60 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( m_Start.tv_sec >= 60 ) {
      std::string str("Minute");
      if ( m_Start.tv_sec > 60 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( m_Start.tv_sec > 0 ) {
      std::string str("Second");
      if ( m_Start.tv_sec > 1 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( m_Start.tv_nsec >= 1000 * 1000 ) {
      std::string str("MilliSecond");
      if ( m_Start.tv_nsec > 1000 * 1000 ) {
         str += std::string("s");
      }
      return str;
   }
   if ( m_Start.tv_nsec >= 1000 ) {
      std::string str("MicroSecond");
      if ( m_Start.tv_nsec > 1000 ) {
         str += std::string("s");
      }
      return str;
   }

   std::string str("NanoSecond");
   if ( (m_Start.tv_nsec > 1) || (0 == m_Start.tv_nsec) ) {
      str += std::string("s");
   }
   return str;
#endif // OS
}

std::string Timer::Normalized(btUnsigned64bitInt *i, double *d) const
{
   double             x = 0.0;
   std::ostringstream oss;

   oss << std::dec;

#if   defined( __AAL_WINDOWS__ )
   LARGE_INTEGER sec;
   LARGE_INTEGER nanos;
   sec.QuadPart   = m_Start.QuadPart / Timer::sm_ClockFreq.QuadPart;
   nanos.QuadPart = (m_Start.QuadPart * 1000000000) / Timer::sm_ClockFreq.QuadPart;

   if ( sec.QuadPart >= 3600 ) {
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_sec >= 3600 ) {
#endif // OS
      AsSeconds(x);
      x /= 3600.0;
      if ( NULL != i ) {
         AsSeconds(*i);
         *i /= 3600ULL;
      }
      if ( NULL != d ) {
         *d = x;
      }
      oss << x << " Hour";
#if   defined( __AAL_WINDOWS__ )
      if ( sec.QuadPart > 3600 ) {
#elif defined( __AAL_LINUX__ )
      if ( m_Start.tv_sec > 3600 ) {
#endif // OS
         oss << 's';
      }

      return oss.str();
   }

#if   defined( __AAL_WINDOWS__ )
   if ( sec.QuadPart >= 60 ) {
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_sec >= 60 ) {
#endif // OS
      AsSeconds(x);
      x /= 60.0;
      if ( NULL != i ) {
         AsSeconds(*i);
         *i /= 60ULL;
      }
      if ( NULL != d ) {
         *d = x;
      }
      oss << x << " Minute";
#if   defined( __AAL_WINDOWS__ )
      if ( sec.QuadPart > 60 ) {
#elif defined( __AAL_LINUX__ )
      if ( m_Start.tv_sec > 60 ) {
#endif // OS
         oss << 's';
      }

      return oss.str();
   }

#if   defined( __AAL_WINDOWS__ )
   if ( sec.QuadPart > 0 ) {
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_sec > 0 ) {
#endif // OS
      AsSeconds(x);
      if ( NULL != i ) {
         AsSeconds(*i);
      }
      if ( NULL != d ) {
         *d = x;
      }
      oss << x << " Second";
#if   defined( __AAL_WINDOWS__ )
      if ( sec.QuadPart > 1 ) {
#elif defined( __AAL_LINUX__ )
      if ( m_Start.tv_sec > 1 ) {
#endif // OS
         oss << 's';
      }
      return oss.str();
   }

#if   defined( __AAL_WINDOWS__ )
   if ( nanos.QuadPart >= 1000 * 1000 ) {
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_nsec >= 1000 * 1000 ) {
#endif // OS
      AsMilliSeconds(x);
      if ( NULL != i ) {
         AsMilliSeconds(*i);
      }
      if ( NULL != d ) {
         *d = x;
      }
      oss << x << " MilliSecond";
#if   defined( __AAL_WINDOWS__ )
      if ( nanos.QuadPart > 1000 * 1000 ) {
#elif defined( __AAL_LINUX__ )
      if ( m_Start.tv_nsec > 1000 * 1000 ) {
#endif // OS
         oss << 's';
      }
      return oss.str();
   }

#if   defined( __AAL_WINDOWS__ )
   if ( nanos.QuadPart >= 1000 ) {
#elif defined( __AAL_LINUX__ )
   if ( m_Start.tv_nsec >= 1000 ) {
#endif // OS
      AsMicroSeconds(x);
      if ( NULL != i ) {
         AsMicroSeconds(*i);
      }
      if ( NULL != d ) {
         *d = x;
      }
      oss << x << " MicroSecond";
#if   defined( __AAL_WINDOWS__ )
      if ( nanos.QuadPart > 1000 ) {
#elif defined( __AAL_LINUX__ )
      if ( m_Start.tv_nsec > 1000 ) {
#endif // OS
         oss << 's';
      }
      return oss.str();
   }

   AsNanoSeconds(x);
   if ( NULL != i ) {
      AsNanoSeconds(*i);
   }
   if ( NULL != d ) {
      *d = x;
   }
   oss << x << " NanoSecond";
#if   defined( __AAL_WINDOWS__ )
   if ( (nanos.QuadPart > 1) || (0 == nanos.QuadPart) ) {
#elif defined( __AAL_LINUX__ )
   if ( (m_Start.tv_nsec > 1) || (0 == m_Start.tv_nsec) ) {
#endif // OS
      oss << 's';
   }

   return oss.str();
}

END_NAMESPACE(AAL)
