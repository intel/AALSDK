// Copyright (c) 2007-2015, Intel Corporation
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
/// @file OSSemaphore.cpp
/// @brief Implementation of the CSemaphore class.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 02/22/2007     JG       Initial version.
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Cleaned up windows includes
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 11/24/2009     JG       Fixed bug in CSemaphore Post() that did not do
///                           an pthread_mutext_unlock() if maxcount exceeded
/// 10/21/2011     JG       Moved Destroy() for Windows
/// 04/19/2012     TSW      Deprecate __FUNCTION__ in favor of platform-
///                          agnostic __AAL_FUNC__.
/// 08/20/2013     JG       Fixed Windows implementation. Modified object to
///                          be a CriticalSection to facilitate thread safety
/// 04/16/2014     JG       Fixed Reset() for count ups that had the count off
///                           by 1.
/// 04/17/2014     JG       Added CurrCount() accessor. Fixed bugs in Destroy 
///                          @endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/OSSemaphore.h"

#ifdef __AAL_UNKNOWN_OS__
# error TODO: Semaphore for unknown OS.
#endif // __AAL_UNKNOWN_OS__

#if defined( __AAL_LINUX__ )
# include <errno.h>
# include <sys/time.h>

// Acquire the lock that protects m_bInitialized.
# define INIT_LOCK()               this->Lock()
// Release the lock that protects m_bInitialized.
# define INIT_UNLOCK()             this->Unlock()
// Release the lock that protects m_bInitialized.
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

// Acquire the lock that protects m_bInitialized.
# define INIT_LOCK()    this->Lock()
// Release the lock that protects m_bInitialized.
# define INIT_UNLOCK()  this->Unlock()
// noop for Windows - there is only one lock.
# define INIT_UNLOCK_COUNT_LOCK()
// Acquire the lock that protects the counters.
# define COUNT_LOCK()   this->Lock()
// Release the lock that protects the counters.
# define COUNT_UNLOCK() this->Unlock()

#endif // OS

//=============================================================================
// Name: CSemaphore
// Description: Constructor
// Interface: public
// Comments:
//=============================================================================
CSemaphore::CSemaphore() :
   m_bInitialized(false),
   m_MaxCount(0),
   m_CurCount(0),
   m_WaitCount(0),
   m_bUnBlocking(false)
#if   defined( __AAL_WINDOWS__ )
   , m_hEvent(NULL)
#endif // OS
{}

//=============================================================================
// Name: ~CSemaphore
// Description: Destructor
// Interface: public
// Comments:
//=============================================================================
CSemaphore::~CSemaphore()
{
   Destroy();
}


//=============================================================================
// Name: Create
// Description: Creates or initializes a semaphore
// Interface: public
// Inputs: int nInitialCount - Initial count could be negative to count up
//         unsigned int nMaxCount - Must be a positive number.
// Outputs:
// Comments: Uses Mutexs to implement semaphores to implement count up
//           functionality
//=============================================================================
AAL::btBool CSemaphore::Create(AAL::btInt nInitialCount, AAL::btUnsignedInt nMaxCount)
{
   // We always protect m_bInitialized with our internal lock (CriticalSection).
   INIT_LOCK();
   if ( m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

#if   defined( __AAL_WINDOWS__ )
   m_hEvent = CreateSemaphore(NULL,                           // no inheritance
                              0,                              // nonsignaled
                              1,                              // max of 1
                              NULL);
   if ( NULL == m_hEvent ) {
      INIT_UNLOCK();
      return false;
   }
#elif defined( __AAL_LINUX__ )
   if ( pthread_mutex_init(&m_mutex, NULL) ) {
      INIT_UNLOCK();
      return false;
   }
   if ( pthread_cond_init(&m_condition, NULL) ) {
      pthread_mutex_destroy(&m_mutex);
      INIT_UNLOCK();
      return false;
   }
#endif // OS

   COUNT_LOCK(); // Yes, lock the init lock then lock the count lock.

   m_bUnBlocking = false;

   if ( nInitialCount < 0 ) {
      // count up sem

      // Increment the current count if it is negative
      //  so that nInitialCount Posts will make count == 1 (not zero)
      //  So if CurCount is -2 making it -1 will result in 2 Posts() bring the semaphore to
      //  a positive 1 and unblocking as we would expect.
      m_CurCount = nInitialCount + 1;

      if ( 0 == nMaxCount ) {
         m_MaxCount = 1;
      } else {
         m_MaxCount = (AAL::btInt) nMaxCount;
      }

   } else {
      // count down sem

      if ( 0 == nMaxCount ) {
         m_MaxCount = (0 == nInitialCount) ? 1 : nInitialCount;
         m_CurCount = nInitialCount;
      } else {
         m_MaxCount = (AAL::btInt) nMaxCount;
         // Cap the initial count at the max.
         m_CurCount = (nInitialCount > (AAL::btInt) nMaxCount) ? (AAL::btInt) nMaxCount : nInitialCount;
      }

   }

   COUNT_UNLOCK();

   m_bInitialized = true;

   INIT_UNLOCK();
   return true;
}
//=============================================================================
// Name: Destroy 
// Description: Destroy the Semaphore, releasing its resources.
// Interface: public
// Returns: false if not initialized or fails
// Comments: Note that Destroying a Semaphore with something waiting will 
//           result in undeterministic behavior.
//=============================================================================
AAL::btBool CSemaphore::Destroy()
{
   AAL::btBool res = false;

   INIT_LOCK();

   if ( m_bInitialized ) {

#if   defined( __AAL_WINDOWS__ )
      res = ( 0 != CloseHandle(m_hEvent) );
#elif defined( __AAL_LINUX__ )
      res = true;
      if ( 0 != pthread_mutex_destroy(&m_mutex) ) {
         res = false;
      }
      if ( 0 != pthread_cond_destroy(&m_condition) ) {
         res = false;
      }
#endif // OS

      m_bInitialized = false;
   }

   INIT_UNLOCK();
   return res;
}


//=============================================================================
// Name: Reset
// Description:
// Interface: public
// Inputs: nCount - count to set to.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Reset(AAL::btInt nCount)
{
   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();

   // Do not reset while someone is waiting.
   if ( m_WaitCount > 0 ) {
      COUNT_UNLOCK();
      return false;
   }

   if ( nCount > m_MaxCount ) {
      COUNT_UNLOCK();
      return false;
   }

#if defined( __AAL_WINDOWS__ )
   // The Windows semaphore count is managed internal to the Semaphore API's.
   // We have to interact with the API's here to affect the counter..
   long prevcount = 0;
   BOOL ret = ReleaseSemaphore(m_hEvent, 1, &prevcount);
   if( (false != ret) && (prevcount) ){
       // Clear the semaphore count
      while ( prevcount-- ) {
         WaitForSingleObject(m_hEvent, 0);
      }
   }
   // Set to the new value
   if(0 != nCount) {
      ReleaseSemaphore(m_hEvent, nCount, &prevcount);
   }
#endif // OS

   m_CurCount = nCount;
   // Increment the current count if it is negative
   //  so that nInitialCount Posts will make count == 1 (not zero)
   //  So if CurCount is -2 making it -1 will result in 2 Posts() bring the semaphore to
   //  a positive 1 and unblocking as we would expect.
   if ( m_CurCount < 0 ) {
      m_CurCount++;
   }

   COUNT_UNLOCK();

   return true;
}

//=============================================================================
// Name: CurrCounts
// Description: Get the current counts
// Interface: public
// Inputs: rcurrCount = return current count
//         rmaxCount = return max count
// Outputs:
// Comments:
//=============================================================================
AAL::btBool CSemaphore::CurrCounts(AAL::btInt &rcurrCount, AAL::btInt &rmaxCount)
{
   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();
   rcurrCount = m_CurCount;
   rmaxCount  = m_MaxCount;
   COUNT_UNLOCK();

   return true;
}


//=============================================================================
// Name: Post
// Description:
// Interface: public
// Inputs: none.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Post(AAL::btInt nCount)
{
   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   // Lock the mutex used in the wait
   //  to protect the shared predicate values (counts)
   INIT_UNLOCK_COUNT_LOCK();

   // Can't post such that you exceed MaxCount
   if ( ( m_CurCount + nCount ) > m_MaxCount ) {
      COUNT_UNLOCK();
      return false;
   }

   // Calculate new count and determine if we need to signal.
   m_CurCount += nCount;

   if ( m_CurCount > 0 ) {

#if defined( __AAL_LINUX__ )

      if ( 1 == m_CurCount ) {
         // Release 1 (or at least minimal) thread
         pthread_cond_signal(&m_condition);      // Signal
      } else {
         // Release all waiting threads
         pthread_cond_broadcast(&m_condition);      // Signal
      }

#elif  defined( __AAL_WINDOWS__ )
      // Release m_CurCount waiting threads
      ReleaseSemaphore(m_hEvent, m_CurCount, NULL );   // Release  (signal)
#endif

   }

   COUNT_UNLOCK();

   return true;
}

//=============================================================================
// Name: UnblockAll
// Description: Unblocks all waiting threads and resets currcount to 0. Unblocked
//              threads will return false.
// Interface: public
// Inputs: none.
// Returns: False if the semaphore isn't initialized
// Comments: This function will cause all threads to wake but it is not
//            guaranteed that all threads have unblocked when the call returns.
//            There is nothing preventing threads from returning to wait()
//            after unblocking.
//=============================================================================
AAL::btBool CSemaphore::UnblockAll()
{
   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   AAL::btBool res = true;

   // Protect predicate and block any waking threads
   INIT_UNLOCK_COUNT_LOCK();

   m_bUnBlocking = true;
   m_CurCount    = 0;

   // Check to see if there is anyone waiting
   if ( m_WaitCount > 0 ) {
#if defined( __AAL_LINUX__ )
      // Wake ALL threads
      if ( 0 != pthread_cond_broadcast(&m_condition) ) {      // Signal
         res = false;
      }
#elif  defined( __AAL_WINDOWS__ )
      // Release All waiting threads
      ReleaseSemaphore(m_hEvent, m_WaitCount, NULL );   // Release  (signal)
#endif
   }

   COUNT_UNLOCK();

   return res;
}

//=============================================================================
// Name: NumWaiters
// Description: Returns the current number of waiters.
// NOTE: This is a snapshot and may change by the time the
//       caller examines the value
// Interface: public
// Inputs: none.
// Returns: Current number of waiters
// Comments:
//=============================================================================
AAL::btUnsignedInt CSemaphore::NumWaiters()
{
   return m_WaitCount;
}


#define ADD_WAITER() ++m_WaitCount
#define DEL_WAITER()                            \
do                                              \
{                                               \
   --m_WaitCount;                               \
   if ( (m_WaitCount <= 0) && m_bUnBlocking ) { \
      m_WaitCount   = 0;                        \
      m_bUnBlocking = false;                    \
   }                                            \
}while(0)


#ifdef __AAL_LINUX__
//=============================================================================
// Name: Wait
// Description: Timed Wait
// Interface: public
// Inputs: none.
// Returns: False if the semaphore is bad or it times out waiting
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Wait(AAL::btTime Timeout) // milliseconds
{
   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   // Allow other waits and posts
   INIT_UNLOCK();

   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);

   struct timespec ts;

   ts.tv_sec = tv.tv_sec;
   ts.tv_nsec = (tv.tv_usec * 1000) + (Timeout * 1000000);

   ts.tv_sec += ts.tv_nsec / 1000000000;
   ts.tv_nsec = ts.tv_nsec % 1000000000;

   // Protect the predicate check (locks the mutex used in wait
   COUNT_LOCK();

   ADD_WAITER();

   // Both the comparison of m_CurCount to 0 and the decrement of m_CurCount must occur atomically.

   while ( m_CurCount <= 0 ) {
      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      if ( ETIMEDOUT == pthread_cond_timedwait(&m_condition,
                                               &m_mutex,
                                               &ts) ) {
         DEL_WAITER();
         // Unlock the mutex locked by the return from wait
         COUNT_UNLOCK();
         return false;
      }

      // If we being unblocked then immediately return false and do not
      //   modify predicate.
      if ( m_bUnBlocking ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
   }
   // ASSERT(m_CurCount > 0);

   // Decrement count. Note we are still protected by lock from return from wait()
   m_CurCount--;

   DEL_WAITER();

   // Unlock predicate protection
   COUNT_UNLOCK();

   return true;
}



//=============================================================================
// Name: Wait
// Description:
// Interface: public
// Inputs: none.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Wait()
{
   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   // Both the comparison of m_CurCount to 0 and the decrement of m_CurCount must occur atomically.
   //  This is done by assuring that manipulation is always done within a lock()
   // Protect the predicate check (locks the mutex used in wait)
   INIT_UNLOCK_COUNT_LOCK();

   ADD_WAITER();

   while ( m_CurCount <= 0 ) {
      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      pthread_cond_wait(&m_condition, &m_mutex);   // Infinite wait

      // If we being unblocked then immediately return false and do not
      //   modify predicate.
      if ( m_bUnBlocking ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
   }

   m_CurCount--;

   DEL_WAITER();

   COUNT_UNLOCK();

   return true;
}
#elif defined(__AAL_WINDOWS__)
//=============================================================================
// Name: Wait
// Description: Timed Wait
// Interface: public
// Inputs: none.
// Returns: False if the semaphore is bad or it times out waiting
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Wait(AAL::btTime Timeout) // milliseconds
{
   DWORD dwWaitResult;

   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();

   ADD_WAITER();

   // We don't have easy access to the counter inside the Windows semaphore object, so
   // we implement our own, redundant, copy.
   // Because we implement our own counter, we must guard it with a lock. We use the lock
   // inherited from CriticalSection to do so.
   // We mustn't put the caller to sleep while we hold our internal lock, else we are exposed
   // to deadlock wrt other threads attempting to query this CSemaphore object.

WAITLOOP:
   while ( m_CurCount <= 0 ) {

      COUNT_UNLOCK();

      // ASSERT: we are unlocked.
      dwWaitResult = WaitForSingleObject(m_hEvent,        // handle to Semaphore
                                         (DWORD)Timeout); // time-out interval

      COUNT_LOCK();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // semaphore was signaled
         default : {
            DEL_WAITER();
            COUNT_UNLOCK();
            return false;     // timeout or error
         }
      }

      // If we being unblocked then immediately return false and do not
      //   modify predicate.
      if ( m_bUnBlocking ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }

   }

   // ASSERT: we are locked.

   // We must guard both the check and the update of m_CurCount as the atomic operation.
   // If the check fails, we must wait again (we were preempted, and some other thread got our
   // posted counter before we acquired our internal lock).

   if ( m_CurCount <= 0 ) {
      goto WAITLOOP;
   }

   m_CurCount--;

   COUNT_UNLOCK();
   return true;
}

//=============================================================================
// Name: Wait
// Description:
// Interface: public
// Inputs: none.
// Outputs:
// Comments:
//=============================================================================
AAL::btBool CSemaphore::Wait()
{
   DWORD dwWaitResult;

   INIT_LOCK();
   if ( !m_bInitialized ) {
      INIT_UNLOCK();
      return false;
   }

   INIT_UNLOCK_COUNT_LOCK();

   ADD_WAITER();

   // We don't have easy access to the counter inside the Windows semaphore object, so
   // we implement our own, redundant, copy.
   // Because we implement our own counter, we must guard it with a lock. We use the lock
   // inherited from CriticalSection to do so.
   // We mustn't put the caller to sleep while we hold our internal lock, else we are exposed
   // to deadlock wrt other threads attempting to query this CSemaphore object.

WAITLOOP:
   while ( m_CurCount <= 0 ) {

      COUNT_UNLOCK();

      // ASSERT: we are unlocked
      dwWaitResult = WaitForSingleObject(m_hEvent,    // handle to Semaphore
                                         INFINITE);   // no time-out interval

      COUNT_LOCK();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // semaphore was signaled
         default : {
            DEL_WAITER();
            COUNT_UNLOCK();
            return false;     // error (using INFINITE above)
         }
      }

      // If we being unblocked then immediately return false and do not
      //   modify predicate.
      if ( m_bUnBlocking ) {
         DEL_WAITER();
         COUNT_UNLOCK();
         return false;
      }
   }

   // We must guard both the check and the update of m_CurCount as the atomic operation.
   // If the check fails, we must wait again (we were preempted, and some other thread got our
   // posted counter before we acquired our internal lock).

   if ( m_CurCount <= 0 ) {
      goto WAITLOOP;
   }

   m_CurCount--;

   COUNT_UNLOCK();
   return true;
}

#endif // OS

