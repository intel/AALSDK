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
/// @file CriticalSection.h
/// @brief CriticalSection interface.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
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
#include <aalsdk/AALTypes.h>

#ifdef __AAL_UNKNOWN_OS__
# error TODO Add CriticalSection support for unknown OS.
#endif // __AAL_UNKNOWN_OS__


/// @addtogroup OSAL
/// @{

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

   /// Try to obtain the critical section. (non-blocking)
   AAL::btBool TryLock()
   {
#if   defined( __AAL_WINDOWS__ )
      if ( !TryEnterCriticalSection(&m_Lock) ) {
         return false;
      }
#elif defined( __AAL_LINUX__ )
      if ( 0 != pthread_mutex_trylock(&m_Lock) ) {
         return false;
      }
#endif // OS
      return true;
   }

private:
#if   defined( __AAL_WINDOWS__ )
                CRITICAL_SECTION m_Lock;
#elif defined( __AAL_LINUX__ )
                pthread_mutex_t  m_Lock;
   static const pthread_mutex_t  sm_MutexInitializer;
#endif // OS
};


/// It is possible that the object may be immutable or in a const method so safely cast away the const
#define AutoLock(p) _AutoLock LockObj(const_cast<CriticalSection *>(dynamic_cast<CriticalSection const *>(p)))
/// Stack-based convenience auto-Mutex objects. Use AutoLock to declare.
class _AutoLock
{
public:
   CriticalSection *m_p;
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
   _AutoLock(CriticalSection *p)
   {
      m_p = p;
      m_p->Lock();
   }

   /// _AutoLock Destructor.
   ~_AutoLock()
   {
      Unlock();
   }

   /// Unlock the auto-lock before the destructor does.
   void Unlock()
   {
      if ( NULL != m_p ) {
         m_p->Unlock();
         m_p = NULL;
      }
   }
};


#if DEPRECATED
#define AutoUnlock(p) _AutoUnlock UnLockObj(p)
class _AutoUnlock
{
public:
   CriticalSection *m_p;
   _AutoUnlock(CriticalSection *p)
   {
      m_p = p;
      m_p->Unlock();
   }

   ~_AutoUnlock()
   {
      if ( NULL != m_p ) {
         m_p->Lock();
         m_p = NULL;
      }
   }
};
#endif // DEPRECATED


#if DEPRECATED
#define TryAutoLock(p) _TryAutoLock LockObj(const_cast<CriticalSection *>(dynamic_cast< CriticalSection const *>(p)))
#define GotAutoLock() LockObj.GotLock()

#define ifTryAutoLock(p) _TryAutoLock LockObj(const_cast<CriticalSection *>(dynamic_cast< CriticalSection const *>(p))) ; if(LockObj.GotLock())
class _TryAutoLock
{
public:
   CriticalSection *m_p;
   _TryAutoLock(CriticalSection *p)
   {
      m_p = p;
      m_locked = m_p->TryLock();
   }

   ~_TryAutoLock()
   {
      if ( NULL != m_p ) {
          m_p->Unlock();
          m_p = NULL;
      }
   }

   btBool GotLock()
   {
      if ( NULL != m_p ) {
         return m_locked;
      }
      return false;
   }
protected:
   btBool m_locked;
};
#endif // DEPRECATED

/// @} group OSAL

#endif // __AALSDK_OSAL_CRITICALSECTION_H__
