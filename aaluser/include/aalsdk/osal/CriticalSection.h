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
/// @file CriticalSection.h
/// @brief CriticalSection interface.
/// @ingroup OSAL
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Charlie Lasswell, Intel Corporation
///          Joseph Grecco,    Intel Corporation
///          Henry Mitchel,    Intel Corporation
///          Tim Whisonant,    Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 03/23/2007     JG       built for AAL
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 06/21/2009     JG       Added TryLock()@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_CRITICALSECTION_H__
#define __AALSDK_OSAL_CRITICALSECTION_H__
#include <aalsdk/osal/IDispatchable.h>

#ifdef __AAL_UNKNOWN_OS__
# error TODO Add CriticalSection support for unknown OS.
#endif // __AAL_UNKNOWN_OS__


/// @addtogroup OSAL
/// @{

BEGIN_NAMESPACE(AAL)

/// Abstraction of Critical Sections (Mutex).
class OSAL_API CriticalSection
{
public:
   /// CriticalSection Default Constructor.
   CriticalSection()
#ifdef __AAL_LINUX__
   : m_Lock(CriticalSection::sm_MutexInitializer)
#endif // __AAL_LINUX__
   {
#ifdef __AAL_WINDOWS__
      InitializeCriticalSection(&m_Lock);
#endif // __AAL_WINDOWS__
   }

   /// CriticalSection Destructor.
   virtual ~CriticalSection()
   {
#if   defined( __AAL_WINDOWS__ )
      DeleteCriticalSection(&m_Lock);
#elif defined( __AAL_LINUX__ )
      pthread_mutex_destroy(&m_Lock);
#endif // OS
   }

protected:

   /// Obtain the critical section. (blocking)
   void Lock()
   {
#if   defined( __AAL_WINDOWS__ )
      EnterCriticalSection(&m_Lock);
#elif defined( __AAL_LINUX__ )
   //   int res;
      // We are not using PTHREAD_PRIO_PROTECT, so we do not expect lock to fail
      // due to priority restrictions. We have initialized the mutex in our constructor,
      // so we should not cause uninitialized mutex errors. We are, however, using
      // recursive mutexes, so we are prone to exceeding the max number of recursive
      // locks, which results in EAGAIN.
   //   do
   //   {
   //      res =
           pthread_mutex_lock(&m_Lock);
   //      if ( EAGAIN == res ) {
   //         pthread_yield();
   //      }
   //   }while(EAGAIN == res);
   //   ASSERT(0 == res);
#endif // OS
   }

   /// Release the critical section.
   void Unlock()
   {
#if   defined( __AAL_WINDOWS__ )
      LeaveCriticalSection(&m_Lock);
#elif defined( __AAL_LINUX__ )
  //    int res;
      // We are using mutex type PTHREAD_MUTEX_RECURSIVE_NP. This means that
      // unlock can fail only if 1) the mutex is already unlocked or 2) a non-owning
      // thread attempts to unlock.
  //    res =
      pthread_mutex_unlock(&m_Lock);
  //    ASSERT(0 == res);
#endif // OS
   }

private:
#if   defined( __AAL_WINDOWS__ )
                CRITICAL_SECTION m_Lock;
#elif defined( __AAL_LINUX__ )
                pthread_mutex_t  m_Lock;
   static const pthread_mutex_t  sm_MutexInitializer;
#endif // OS

   friend class _AutoLock;

#if   defined( __AAL_WINDOWS__ )
   friend class _UnlockedWaitForSingleObject;
#elif defined( __AAL_LINUX__ )
   friend class _PThreadCondWait;
   friend class _PThreadCondTimedWait;
#endif // OS

   friend class _UnlockedDispatch;
};


/// It is possible that the object may be immutable or in a const method so safely cast away the const
#define AutoLock(__p) AAL::_AutoLock __LockObj(const_cast<AAL::CriticalSection *>(dynamic_cast<AAL::CriticalSection const *>(__p)))
/// Stack-based convenience auto-Mutex objects. Use AutoLock to declare.
class OSAL_API _AutoLock
{
public:

   /// _AutoLock Construct with CriticalSection *.
   ///
   /// Typical usage is:
   ///
   /// @code
   /// class MyClass : public CriticalSection
   /// {
   ///    ...
   /// };
   ///
   /// void MyClass::MyMemberFn()
   /// {
   ///    AutoLock(this);
   ///
   ///    // This code is protected by the Critical Section.
   ///
   /// }@endcode
   _AutoLock(CriticalSection *p) :
      m_p(p)
   {
      ASSERT(NULL != m_p);
      m_p->Lock();
   }

   /// _AutoLock Destructor.
   virtual ~_AutoLock()
   {
      ASSERT(NULL != m_p);
      m_p->Unlock();
   }

protected:
   CriticalSection *m_p;
};

#if defined( __AAL_WINDOWS__ )

// Unlocks the CriticalSection, then waits for the HANDLE with timeout.
// Declare on the stack so that destructor ensures lock safety.
class OSAL_API _UnlockedWaitForSingleObject
{
public:
   _UnlockedWaitForSingleObject(CriticalSection *pCS, HANDLE h, DWORD dwTimeout) :
      m_pCS(pCS),
      m_Result(WAIT_FAILED)
   {
      ASSERT(NULL != pCS);
      ASSERT(NULL != h);
      m_pCS->Unlock(); // Don't sleep while locked.
      m_Result = WaitForSingleObject(h, dwTimeout);
   }

   virtual ~_UnlockedWaitForSingleObject()
   {
      m_pCS->Lock();
   }

   DWORD Result() const { return m_Result; }

protected:
   CriticalSection *m_pCS;
   DWORD            m_Result;
};

#elif defined( __AAL_LINUX__ )

/// Infinite wait on condition variable.
class _PThreadCondWait
{
public:
   _PThreadCondWait(CriticalSection *pCS, pthread_cond_t *pCond) :
      m_Result(EINVAL)
   {
      ASSERT(NULL != pCS);
      ASSERT(NULL != pCond);
      m_Result = ::pthread_cond_wait(pCond, &pCS->m_Lock);
   }
   virtual ~_PThreadCondWait() {}

   int Result() const { return m_Result; }

protected:
   int m_Result;
};

class _PThreadCondTimedWait
{
public:
   _PThreadCondTimedWait(CriticalSection *pCS, pthread_cond_t *pCond, struct timespec *pTS) :
      m_Result(EINVAL)
   {
      ASSERT(NULL != pCS);
      ASSERT(NULL != pCond);
      ASSERT(NULL != pTS);
      m_Result = ::pthread_cond_timedwait(pCond, &pCS->m_Lock, pTS);
   }
   virtual ~_PThreadCondTimedWait() {}

   int Result() const { return m_Result; }

protected:
   int m_Result;
};

#endif // __AAL_WINDOWS__

// Unlock the CriticalSection, then fire the Dispatchable.
// Declare on the stack so that destructor ensures lock safety.
class OSAL_API _UnlockedDispatch
{
public:
   _UnlockedDispatch(CriticalSection *pCS, IDispatchable *pDisp) :
      m_pCS(pCS)
   {
      ASSERT(NULL != m_pCS);
      ASSERT(NULL != pDisp);
      m_pCS->Unlock();
      pDisp->operator() ();
   }

   virtual ~_UnlockedDispatch()
   {
      m_pCS->Lock();
   }

protected:
   CriticalSection *m_pCS;
};

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_OSAL_CRITICALSECTION_H__

