// Copyright(c) 2007-2016, Intel Corporation
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
/// Accelerator Abstraction Layer
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
#endif // OS

#ifdef DBG_CSEMAPHORE
# include "dbg_csemaphore.cpp"
#else
# define AutoLock0(__x) AutoLock(__x)
# define AutoLock1(__x) AutoLock(__x)
# define AutoLock2(__x) AutoLock(__x)
# define AutoLock3(__x) AutoLock(__x)
# define AutoLock4(__x) AutoLock(__x)
# define AutoLock5(__x) AutoLock(__x)
# define AutoLock6(__x) AutoLock(__x)
# define AutoLock7(__x) AutoLock(__x)
#endif // DBG_CSEMAPHORE

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: CSemaphore
// Description: Constructor
// Interface: public
// Comments:
//=============================================================================
CSemaphore::CSemaphore() :
   m_State(0),
   m_MaxCount(0),
   m_CurCount(0),
   m_WaitCount(0),
   m_UserDefined(NULL)
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
btBool CSemaphore::Create(btInt nInitialCount, btUnsignedInt nMaxCount)
{
   // We always protect m_bInitialized with our internal lock (CriticalSection).
   AutoLock0(this);

   if ( flag_is_set(m_State, SEM_ST_OK) ) {
      // Already initialized.
      return false;
   }

#if   defined( __AAL_WINDOWS__ )
   m_hEvent = CreateEvent(NULL,  // Default security attributes, no inheritance.
                          true,  // Manual reset event.
                          false, // Not signaled.
                          NULL); // No name.

   if ( NULL == m_hEvent ) {
      return false;
   }
#elif defined( __AAL_LINUX__ )
   if ( pthread_cond_init(&m_condition, NULL) ) {
      return false;
   }
#endif // OS

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
         m_MaxCount = (btInt) nMaxCount;
      }

   } else {
      // count down sem

      if ( 0 == nMaxCount ) {
         m_MaxCount = (0 == nInitialCount) ? 1 : nInitialCount;
         m_CurCount = nInitialCount;
      } else {
         m_MaxCount = (btInt) nMaxCount;
         // Cap the initial count at the max.
         m_CurCount = (nInitialCount > (btInt) nMaxCount) ? (btInt) nMaxCount : nInitialCount;
      }

   }

   flag_clrf(m_State, SEM_ST_UNBLOCKED);
   flag_setf(m_State, SEM_ST_OK);

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
btBool CSemaphore::Destroy()
{
   btBool res = false;

   AutoLock1(this);

   if ( flag_is_set(m_State, SEM_ST_OK) ) {

      UnblockAll();

#if   defined( __AAL_WINDOWS__ )
      res = ( 0 != CloseHandle(m_hEvent) );
#elif defined( __AAL_LINUX__ )
      res = true;
      if ( 0 != pthread_cond_destroy(&m_condition) ) {
         res = false;
      }
#endif // OS

      // No longer initialized.
      flag_clrf(m_State, SEM_ST_OK);
   }

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
btBool CSemaphore::Reset(btInt nCount)
{
   AutoLock2(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   // Do not reset while someone is waiting.
   if ( m_WaitCount > 0 ) {
      return false;
   }

   if ( nCount > m_MaxCount ) {
      return false;
   }

   m_CurCount = nCount;

   // Increment the current count if it is negative
   //  so that nInitialCount Posts will make count == 1 (not zero)
   //  So if CurCount is -2 making it -1 will result in 2 Posts() bring the semaphore to
   //  a positive 1 and unblocking as we would expect.
   if ( m_CurCount < 0 ) {
      m_CurCount++;
   }

#ifdef __AAL_WINDOWS__
   if ( m_CurCount <= 0 ) {
      // Cause new waiters to block. (manual reset event)
      ResetEvent(m_hEvent);
   }
#endif // __AAL_WINDOWS__

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
btBool CSemaphore::CurrCounts(btInt &rcurrCount, btInt &rmaxCount)
{
   AutoLock3(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   rcurrCount = m_CurCount;
   rmaxCount  = m_MaxCount;

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
btBool CSemaphore::Post(btInt nCount)
{
   AutoLock4(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   // Can't post such that you exceed MaxCount
   if ( ( m_CurCount + nCount ) > m_MaxCount ) {
      return false;
   }

   // Calculate new count and determine if we need to signal.
   m_CurCount += nCount;

   if ( m_CurCount > 0 ) {

#if   defined( __AAL_LINUX__ )

      if ( 1 == m_CurCount ) {
         // Release 1 (or at least minimal) thread
         pthread_cond_signal(&m_condition);      // Signal
      } else {
         // Release all waiting threads
         pthread_cond_broadcast(&m_condition);      // Signal
      }

#elif defined( __AAL_WINDOWS__ )

      // Resume waiters from sleep.
      SetEvent(m_hEvent);

#endif

   }

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
btBool CSemaphore::UnblockAll()
{
   AutoLock5(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   btBool res = true;

   flag_setf(m_State, SEM_ST_UNBLOCKED);
   m_CurCount = 0;

   // Check to see if there is anyone waiting
   if ( m_WaitCount > 0 ) {

#if   defined( __AAL_LINUX__ )

      // Wake ALL threads
      if ( 0 != pthread_cond_broadcast(&m_condition) ) {      // Signal
         res = false;
      }

#elif defined( __AAL_WINDOWS__ )

      // Resume waiters from sleep.
      if ( !SetEvent(m_hEvent) ) {
         res = false;
      }

#endif

   }

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
btUnsignedInt CSemaphore::NumWaiters()
{
   AutoLock(this);
   return m_WaitCount;
}


#define ADD_WAITER() ++m_WaitCount
#define DEL_WAITER()                        \
do                                          \
{                                           \
   --m_WaitCount;                           \
   if ( 0 == m_WaitCount ) {                \
      flag_clrf(m_State, SEM_ST_UNBLOCKED); \
   }                                        \
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
btBool CSemaphore::Wait(btTime Timeout) // milliseconds
{
   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   {
      AutoLock(this);
      if ( flag_is_clr(m_State, SEM_ST_OK) ) {
         // Not initialized.
         return false;
      }
   }

   // Unlock to allow other waits and posts..

   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);

   struct timespec ts;

   ts.tv_sec = tv.tv_sec;
   ts.tv_nsec = (tv.tv_usec * 1000) + (Timeout * 1000000);

   ts.tv_sec += ts.tv_nsec / 1000000000;
   ts.tv_nsec = ts.tv_nsec % 1000000000;

   // Protect the predicate check (locks the mutex used in wait)
   AutoLock7(this);

   ADD_WAITER();

   // Both the comparison of m_CurCount to 0 and the decrement of m_CurCount must occur atomically.

   while ( m_CurCount <= 0 ) {
      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      int WaitRes = EINVAL;

      {
         _PThreadCondTimedWait wait(this, &m_condition, &ts);
         WaitRes = wait.Result();
      }

      if ( ETIMEDOUT == WaitRes ) {
         DEL_WAITER();
         // Unlock the mutex locked by the return from wait
         return false;
      }

      // If we are being unblocked, then immediately return false and do not
      //   modify the predicate.
      if ( flag_is_set(m_State, SEM_ST_UNBLOCKED) ) {
         DEL_WAITER();
         return false;
      }
   }
   // ASSERT(m_CurCount > 0);

   // Decrement count. Note we are still protected by lock from return from wait()
   m_CurCount--;

   DEL_WAITER();

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
btBool CSemaphore::Wait()
{
   AutoLock6(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   // Both the comparison of m_CurCount to 0 and the decrement of m_CurCount must occur atomically.
   //  This is done by assuring that manipulation is always done within a lock()
   // Protect the predicate check (locks the mutex used in wait)

   ADD_WAITER();

   while ( m_CurCount <= 0 ) {
      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      {
         _PThreadCondWait wait(this, &m_condition); // Infinite wait
      }

      // If we are being unblocked, then immediately return false and do not
      //   modify the predicate.
      if ( flag_is_set(m_State, SEM_ST_UNBLOCKED) ) {
         DEL_WAITER();
         return false;
      }
   }

   m_CurCount--;

   DEL_WAITER();

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
btBool CSemaphore::Wait(btTime Timeout) // milliseconds
{
   DWORD dwWaitResult = WAIT_FAILED;

   if ( AAL_INFINITE_WAIT == Timeout ) {
      return Wait();
   }

   AutoLock7(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   ADD_WAITER();

   // We mustn't put the caller to sleep while we hold our internal lock, else we are exposed
   // to deadlock wrt other threads attempting to query this CSemaphore object. This is handled
   // by the _UnlockedWaitForSingleObject variable and the surrounding scope block, which forces
   // its destructor to fire in the appropriate place.

WAITLOOP:
   while ( m_CurCount <= 0 ) {

      {
         _UnlockedWaitForSingleObject wait(this, m_hEvent, (DWORD)Timeout);
         dwWaitResult = wait.Result();
      }

      // If we are being unblocked, then immediately return false and do not
      //   modify the predicate.
      if ( flag_is_set(m_State, SEM_ST_UNBLOCKED) ) {
         DEL_WAITER();
         return false;
      }

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            DEL_WAITER();
            return false;     // timeout or error
         }
      }

   }

   // We must guard both the check and the update of m_CurCount as the atomic operation.
   // If the check fails, we must wait again (we were preempted, and some other thread got our
   // posted counter before we acquired our internal lock).
   // This is a fundamental distinction from pthreads, where the mutex is guaranteed to be
   // locked upon resume from sleep. Not so here.

   if ( m_CurCount <= 0 ) {
      goto WAITLOOP;
   }

   // m_CurCount > 0

   m_CurCount--;

   DEL_WAITER();

   if ( 0 == m_CurCount ) {
      // Make subsequent calls to Wait() block on the event. (manual reset event)
      ResetEvent(m_hEvent);
   }

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
btBool CSemaphore::Wait()
{
   DWORD dwWaitResult = WAIT_FAILED;

   AutoLock6(this);

   if ( flag_is_clr(m_State, SEM_ST_OK) ) {
      // Not initialized.
      return false;
   }

   ADD_WAITER();

   // We mustn't put the caller to sleep while we hold our internal lock, else we are exposed
   // to deadlock wrt other threads attempting to query this CSemaphore object. This is handled
   // by the _UnlockedWaitForSingleObject variable and the surrounding scope block, which forces
   // its destructor to fire in the appropriate place.

WAITLOOP:
   while ( m_CurCount <= 0 ) {

      {
         _UnlockedWaitForSingleObject wait(this, m_hEvent, INFINITE);
         dwWaitResult = wait.Result();
      }

      // If we are being unblocked, then immediately return false and do not
      //   modify the predicate.
      if ( flag_is_set(m_State, SEM_ST_UNBLOCKED) ) {
         DEL_WAITER();
         return false;
      }

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            DEL_WAITER();
            return false;     // error (using INFINITE above)
         }
      }

   }

   // We must guard both the check and the update of m_CurCount as the atomic operation.
   // If the check fails, we must wait again (we were preempted, and some other thread got our
   // posted counter before we acquired our internal lock).
   // This is a fundamental distinction from pthreads, where the mutex is guaranteed to be
   // locked upon resume from sleep. Not so here.

   if ( m_CurCount <= 0 ) {
      goto WAITLOOP;
   }

   // m_CurCount > 0

   m_CurCount--;

   DEL_WAITER();

   if ( 0 == m_CurCount ) {
      // Make subsequent calls to Wait() block on the event. (manual reset event)
      ResetEvent(m_hEvent);
   }

   return true;
}

#endif // OS

void CSemaphore::UserDefined(btObjectType User)
{
   AutoLock(this);
   m_UserDefined = User;
}

btObjectType CSemaphore::UserDefined() const
{
   AutoLock(this);
   return m_UserDefined;
}

END_NAMESPACE(AAL)

