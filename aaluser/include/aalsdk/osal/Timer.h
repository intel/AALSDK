// Copyright (c) 2003-2015, Intel Corporation
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
/// @file Timer.h
/// @brief Timer API for OS Abstraction Layer.
/// @ingroup OSAL
/// @verbatim
///
/// AUTHORS: Charlie Lasswell, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_TIMER_H__
#define __AALSDK_OSAL_TIMER_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup OSAL
/// @{

class OSAL_API Timer
{
public:
   Timer();

#if   defined( __AAL_WINDOWS__ )
   Timer(LARGE_INTEGER *p)   { m_Start = *p; }
   Timer & operator = (LARGE_INTEGER    i) { m_Start = i;  return *this; }
#elif defined( __AAL_LINUX__ )
   Timer(struct timespec *p) { m_Start = *p; }
   Timer & operator = (struct timespec ts) { m_Start = ts; return *this; }
#endif // __AAL_LINUX__

   // capture the current timestamp
   Timer Now() const;

   // *this + other
   Timer Add(const Timer &other) const;

   // *this - other
   Timer Subtract(const Timer &other) const;

   // <  0 if *this < other
   // >  0 if *this > other
   // == 0 if *this == other
   int Compare(const Timer &other) const;

   void      AsSeconds(double & )                          const;
   void AsMilliSeconds(double & )                          const;
   void AsMicroSeconds(double & )                          const;
   void  AsNanoSeconds(double & )                          const;

   void      AsSeconds(AAL::btUnsigned64bitInt & )         const;
   void AsMilliSeconds(AAL::btUnsigned64bitInt & )         const;
   void AsMicroSeconds(AAL::btUnsigned64bitInt & )         const;
   void  AsNanoSeconds(AAL::btUnsigned64bitInt & )         const;

   std::string NormalizedUnits()                           const;
   std::string Normalized(AAL::btUnsigned64bitInt * =NULL,
                          double                  * =NULL) const;

#if   defined( __AAL_WINDOWS__ )
   operator LARGE_INTEGER ()   const { return m_Start; }
#elif defined( __AAL_LINUX__ )
   operator struct timespec () const { return m_Start; }
#endif // __AAL_LINUX__

protected:
   friend Timer operator +  (const Timer & , const Timer & );
   friend Timer operator -  (const Timer & , const Timer & );
   friend bool  operator <  (const Timer & , const Timer & );
   friend bool  operator <= (const Timer & , const Timer & );
   friend bool  operator >  (const Timer & , const Timer & );
   friend bool  operator >= (const Timer & , const Timer & );
   friend bool  operator == (const Timer & , const Timer & );
   friend bool  operator != (const Timer & , const Timer & );

#if   defined( __AAL_WINDOWS__ )
   LARGE_INTEGER m_Start;
	static LARGE_INTEGER sm_ClockFreq;
#elif defined( __AAL_LINUX__ )
   struct timespec m_Start;
#endif // __AAL_LINUX__
};

inline Timer operator +  (const Timer &l, const Timer &r) { return l.Add(r);          }
inline Timer operator -  (const Timer &l, const Timer &r) { return l.Subtract(r);     }
inline bool  operator <  (const Timer &l, const Timer &r) { return l.Compare(r) <  0; }
inline bool  operator <= (const Timer &l, const Timer &r) { return l.Compare(r) <= 0; }
inline bool  operator >  (const Timer &l, const Timer &r) { return l.Compare(r) >  0; }
inline bool  operator >= (const Timer &l, const Timer &r) { return l.Compare(r) >= 0; }
inline bool  operator == (const Timer &l, const Timer &r) { return l.Compare(r) == 0; }
inline bool  operator != (const Timer &l, const Timer &r) { return l.Compare(r) != 0; }

/// @}

#endif // __AALSDK_OSAL_TIMER_H__


