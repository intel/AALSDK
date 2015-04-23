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
/// @file Barrier.cpp
/// @brief Implementation of the Barrier synchronization primitive.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/21/2015     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/Barrier.h"

#ifdef __AAL_UNKNOWN_OS__
# error TODO: Barrier for unknown OS.
#endif // __AAL_UNKNOWN_OS__

//=============================================================================
// Name: Barrier
// Description: Constructor
// Interface: public
// Comments:
//=============================================================================
Barrier::Barrier() :
   m_Flags(0),
   m_UnlockCount(0),
   m_CurCount(0),
   m_NumWaiters(0)
#if   defined( __AAL_WINDOWS__ )
   , m_hEvent(NULL)
#endif // OS
{}

//=============================================================================
// Name: ~Barrier
// Description: Destructor
// Interface: public
// Comments:
//=============================================================================
Barrier::~Barrier()
{
   Destroy();
}

#if   defined( __AAL_LINUX__ )
# include <errno.h>
# include <sys/time.h>

// Acquire the lock that protects m_Flags.
# define INIT_LOCK()               this->Lock()
// Release the lock that protects m_Flags.
# define INIT_UNLOCK()             this->Unlock()
// Release the lock that protects m_Flags.
// Acquire the lock that protects the counters.
# define INIT_UNLOCK_COUNT_LOCK()  \
do                                 \
{                                  \
   this->Unlock();                 \
   ::pthread_mutex_lock(&m_mutex); \
}while(0)
// Acquire the lock that protects the counters.
# define COUNT_LOCK()              ::pthread_mutex_lock(&m_mutex);
// Release the lock that protects the counters.
# define COUNT_UNLOCK()            ::pthread_mutex_unlock(&m_mutex);

#elif defined( __AAL_WINDOWS__ )

// Acquire the lock that protects m_Flags.
# define INIT_LOCK()    this->Lock()
// Release the lock that protects m_Flags.
# define INIT_UNLOCK()  this->Unlock()
// noop for Windows - there is only one lock.
# define INIT_UNLOCK_COUNT_LOCK()
// Acquire the lock that protects the counters.
# define COUNT_LOCK()   this->Lock()
// Release the lock that protects the counters.
# define COUNT_UNLOCK() this->Unlock()

#endif // OS



//=============================================================================
// Name: Create
// Description: Creates or initializes a Barrier
// Interface: public
// Inputs: UnlockCount - the number of Post()'s required before the Barrier is unlocked.
//         bAutoReset  - whether the Barrier should automatically reset the current count to
//                        zero when all waiters have resumed.
// Outputs:
// Comments: m_UnlockCount = std::max(1, UnlockCount);
//=============================================================================
AAL::btBool Barrier::Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset)
{
   INIT_LOCK();
   
   if ( flag_is_set(m_Flags, BARRIER_FLAG_INIT) ) {
      // Already initialized
      INIT_UNLOCK();
      return false;
   }

#if   defined( __AAL_WINDOWS__ )
   
   m_hEvent = CreateEvent(NULL,   // no inheritance
                          TRUE,   // manual reset event
                          FALSE,  // not signaled
                          NULL);  // no name
   if ( NULL == m_hEvent ) {
      COUNT_UNLOCK();
      INIT_UNLOCK();
      return false;
   }
   
#elif defined( __AAL_LINUX__ )
   
   pthread_mutexattr_t attr;
   
   if ( 0 != pthread_mutexattr_init(&attr) ) {
      INIT_UNLOCK();
      return false;
   }
   
   if ( 0 != pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) ) {
      pthread_mutexattr_destroy(&attr);
      INIT_UNLOCK();
      return false;
   }
   
   if ( 0 != pthread_mutex_init(&m_mutex, &attr) ) {
      pthread_mutexattr_destroy(&attr);
      INIT_UNLOCK();
      return false;
   }
   
   pthread_mutexattr_destroy(&attr);
   
   if ( 0 != pthread_cond_init(&m_condition, NULL) ) {
      INIT_UNLOCK();
      return false;
   }
   
#endif // OS

   COUNT_LOCK(); // yes, both locks.

   m_Flags = 0;

   // The current count always starts from zero.
   // The unlock count must be greater than zero upon return from Create().
   m_CurCount    = 0;
   m_UnlockCount = std::max((AAL::btUnsignedInt)1, UnlockCount);

   // No waiters.
   m_NumWaiters = 0;

   if ( bAutoReset ) {
      flag_setf(m_Flags, BARRIER_FLAG_AUTO_RESET);
   }

   flag_setf(m_Flags, BARRIER_FLAG_INIT);
   
   COUNT_UNLOCK();
   INIT_UNLOCK();
   
   return true;
}
//=============================================================================
// Name: Destroy 
// Description: Destroy the Barrier, releasing its resources.
// Interface: public
// Returns: false if not initialized or fails
// Comments: Note that Destroying a Barrier with something waiting will 
//           result in non-deterministic behavior.
//=============================================================================
AAL::btBool Barrier::Destroy()
{
   AAL::btBool res = true;
   
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

#if   defined( __AAL_WINDOWS__ )
   
   res = ( 0 != CloseHandle(m_hEvent) );
   
#elif defined( __AAL_LINUX__ )

   if ( 0 != pthread_cond_destroy(&m_condition) ) {
      res = false;
   }
   
   if ( 0 != pthread_mutex_destroy(&m_mutex) ) {
      res = false;
   }
   
#endif // OS

   flag_clrf(m_Flags, BARRIER_FLAG_INIT);

   INIT_UNLOCK();
   
   return res;
}


//=============================================================================
// Name: Reset
// Description:
// Interface: public
// Inputs: Update m_UnlockCount when UnlockCount != 0.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool Barrier::Reset(AAL::btUnsignedInt UnlockCount)
{
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   AAL::btBool res = true;
   
   INIT_UNLOCK_COUNT_LOCK();

#if defined( __AAL_WINDOWS__ )

   // Reset the manual-reset event to non-signaled.
   if ( !ResetEvent(m_hEvent) ) {
      res = false;
   }
   
#endif // __AAL_WINDOWS__
   
   // We're resetting things, so..
   // * we're no longer unblocking, if we were before this call.
   // * the current count is back to zero.
   flag_clrf(m_Flags, BARRIER_FLAG_UNBLOCKING);
   m_CurCount = 0;
   
   // Note that this treatment of 0 == UnlockCount is different than that of Create().
   // Here, if UnlockCount is zero, we don't change m_UnlockCount. This allows resetting
   //  the barrier, without having to know the unlock value given to Create().
   if ( UnlockCount > 0 ) {
      m_UnlockCount = UnlockCount;
   }

   COUNT_UNLOCK();
   
   return res;
}

//=============================================================================
// Name: CurrCounts
// Description: Get the current count and the unlock count of the Barrier.
// Interface: public
// Inputs: rCurCount    = current count
//         rUnlockCount = unlock count
// Outputs:
// Comments:
//=============================================================================
AAL::btBool Barrier::CurrCounts(AAL::btUnsignedInt &rCurCount, AAL::btUnsignedInt &rUnlockCount)
{
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();
   rCurCount    = m_CurCount;
   rUnlockCount = m_UnlockCount;
   COUNT_UNLOCK();
   
   return true;
}


//=============================================================================
// Name: Post
// Description: Increment the current count by min(nCount, m_UnlockCount - m_CurCount).
//              If the current count becomes equal to the unlock count, then all waiters are resumed.
// Interface: 
// Inputs: nCount - the number to add to the current count.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool Barrier::Post(AAL::btUnsignedInt nCount)
{
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   AAL::btBool res = true;

   INIT_UNLOCK_COUNT_LOCK();
   
   // We let m_CurCount meet, but never exceed m_UnlockCount.
   AAL::btUnsignedInt c = std::min(nCount, m_UnlockCount - m_CurCount);
   
   m_CurCount += c;

   if ( m_CurCount >= m_UnlockCount ) {

#if defined( __AAL_LINUX__ )

      // Broadcast the condition to all.
      if ( 0 != pthread_cond_broadcast(&m_condition) ) {
         res = false;
      }

#elif  defined( __AAL_WINDOWS__ )
      
      // Set the event state to signaled, waking all.
      if ( !SetEvent(m_hEvent) ) {
         res = false;
      }
      
#endif

   }

   COUNT_UNLOCK();
   
   return res;
}

//=============================================================================
// Name: UnblockAll
// Description: Unblocks all waiting threads, setting the current count equal to the unlock count.
//              The resumed waiters will return false from their respective Wait() calls.
// Interface: public
// Inputs: none.
// Returns: False if the Barrier isn't initialized
// Comments: This function will cause all threads to wake but it is not
//            guaranteed that all threads have unblocked when the call returns.
//            There is nothing preventing threads from returning to wait()
//            after unblocking.
//=============================================================================
AAL::btBool Barrier::UnblockAll()
{
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   AAL::btBool res = true;

   INIT_UNLOCK_COUNT_LOCK();
   
   flag_setf(m_Flags, BARRIER_FLAG_UNBLOCKING);
   m_CurCount = m_UnlockCount;

#if defined( __AAL_LINUX__ )
   
   // Broadcast the condition to all.
   if ( 0 != pthread_cond_broadcast(&m_condition) ) {
      res = false;
   }
      
#elif  defined( __AAL_WINDOWS__ )
      
   // Set the event state to signaled, waking all.
   if ( !SetEvent(m_hEvent) ) {
      res = false;
   }
      
#endif

   COUNT_UNLOCK();
   
   return res;
}

AAL::btUnsignedInt Barrier::NumWaiters() const
{
   return m_NumWaiters;
}

#define ADD_WAITER() ++m_NumWaiters

#if defined( __AAL_LINUX__ )

# define DEL_WAITER()                                        \
do                                                           \
{                                                            \
   --m_NumWaiters;                                           \
   if ( 0 == m_NumWaiters ) {                                \
      flag_clrf(m_Flags, BARRIER_FLAG_UNBLOCKING);           \
      if ( flag_is_set(m_Flags, BARRIER_FLAG_AUTO_RESET) ) { \
         m_CurCount = 0;                                     \
      }                                                      \
   }                                                         \
}while(0)

//=============================================================================
// Name: Wait
// Description: Block infinitely until the current count becomes equal to the unlock count.
// Interface: public
// Inputs: none.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool Barrier::Wait()
{
   INIT_LOCK();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }
   
   INIT_UNLOCK_COUNT_LOCK();
   
   ADD_WAITER();
   
   while ( m_CurCount < m_UnlockCount ) {

      pthread_cond_wait(&m_condition, &m_mutex);

      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING) ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
      
   }

   DEL_WAITER();
   COUNT_UNLOCK();
   
   return true;
}

//=============================================================================
// Name: Wait
// Description: Block until the current count becomes equal to the unlock count or until a
//              timeout expires.
// Interface: public
// Inputs: none.
// Returns: False if the Barrier is bad or it times out waiting
// Comments:
//=============================================================================
AAL::btBool Barrier::Wait(AAL::btTime Timeout) // milliseconds
{
   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   INIT_LOCK();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      INIT_UNLOCK();
      return false;
   }

   // Allow other waits and posts
   INIT_UNLOCK();

   struct timeval tv;
   gettimeofday(&tv, NULL);

   struct timespec ts;

   ts.tv_sec  = tv.tv_sec;
   ts.tv_nsec = (tv.tv_usec * 1000) + (Timeout * 1000000);

   ts.tv_sec  += ts.tv_nsec / 1000000000;
   ts.tv_nsec %= 1000000000;

   // Protect the predicate check (locks the mutex used in wait)
   COUNT_LOCK();

   ADD_WAITER();

   while ( m_CurCount < m_UnlockCount ) {
      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      if ( ETIMEDOUT == pthread_cond_timedwait(&m_condition,
                                               &m_mutex,
                                               &ts) ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING) ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
   }

   DEL_WAITER();
   
   COUNT_UNLOCK();

   return true;
}

#elif defined( __AAL_WINDOWS__ )

# define DEL_WAITER()                                        \
do                                                           \
{                                                            \
   --m_NumWaiters;                                           \
   if ( 0 == m_NumWaiters ) {                                \
      flag_clrf(m_Flags, BARRIER_FLAG_UNBLOCKING);           \
      if ( flag_is_set(m_Flags, BARRIER_FLAG_AUTO_RESET) ) { \
         m_CurCount = 0;                                     \
         ResetEvent(m_hEvent); /* manual-reset event */      \
      }                                                      \
   }                                                         \
}while(0)

//=============================================================================
// Name: Wait
// Description: Block infinitely until the current count becomes equal to the unlock count.
// Interface: public
// Inputs: none.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool Barrier::Wait()
{
   DWORD dwWaitResult;

   INIT_LOCK();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();

   ADD_WAITER();

WAITLOOP:
   while ( m_CurCount < m_UnlockCount ) {

      COUNT_UNLOCK();

      // ASSERT: we are unlocked - don't wait while locked!
      dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);

      COUNT_LOCK();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            DEL_WAITER();
            COUNT_UNLOCK();
            return false;     // error (using INFINITE above)
         }
      }

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING) ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
   }

   // We must guard both the wake signal and the check of m_CurCount as the atomic operation.
   // If the check below fails, we must wait again (we were preempted, and some other thread
   //  modified m_CurCount before we acquired our internal lock).

   if ( m_CurCount < m_UnlockCount ) {
      goto WAITLOOP;
   }

   DEL_WAITER();
   
   COUNT_UNLOCK();
   
   return true;
}

//=============================================================================
// Name: Wait
// Description: Block until the current count becomes equal to the unlock count or until a
//              timeout expires.
// Interface: public
// Inputs: none.
// Returns: False if the Barrier is bad or it times out waiting
// Comments:
//=============================================================================
AAL::btBool Barrier::Wait(AAL::btTime Timeout) // milliseconds
{
   DWORD dwWaitResult;

   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   INIT_LOCK();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();

   ADD_WAITER();

WAITLOOP:
   while ( m_CurCount < m_UnlockCount ) {

      COUNT_UNLOCK();

      // ASSERT: we are unlocked - don't wait while locked!
      dwWaitResult = WaitForSingleObject(m_hEvent, (DWORD)Timeout);

      COUNT_LOCK();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            DEL_WAITER();
            COUNT_UNLOCK();
            return false;     // timeout or error
         }
      }

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING) ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }

   }

   // We must guard both the wake signal and the check of m_CurCount as the atomic operation.
   // If the check below fails, we must wait again (we were preempted, and some other thread
   //  modified m_CurCount before we acquired our internal lock).

   if ( m_CurCount < m_UnlockCount ) {
      goto WAITLOOP;
   }

   DEL_WAITER();
   
   COUNT_UNLOCK();
   
   return true;
}

#endif // OS

