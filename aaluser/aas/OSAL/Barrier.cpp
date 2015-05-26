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
   m_AutoResetManager(this)
#if defined( __AAL_WINDOWS__ )
   , m_hEvent(NULL)
#endif // __AAL_WINDOWS__
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

#if defined( __AAL_LINUX__ )
# include <errno.h>
# include <sys/time.h>
#endif // __AAL_LINUX__

// Acquire the lock that protects m_Flags.
void Barrier::StateLock()   { this->Lock();   }
// Release the lock that protects m_Flags.
void Barrier::StateUnlock() { this->Unlock(); }
// Release the lock that protects m_Flags.
// Acquire the lock that protects the counters.
void Barrier::StateUnlockCountLock()
{
#if   defined( __AAL_LINUX__ )
   int res;

   this->Unlock();
   res = pthread_mutex_lock(&m_mutex);
   ASSERT(0 == res);

#elif defined( __AAL_WINDOWS__ )
   // No action here for Windows - use the same lock for both.
#endif // OS
}
// Acquire the lock that protects the counters.
void Barrier::CountLock()
{
#if   defined( __AAL_LINUX__ )
   int res;

   res = pthread_mutex_lock(&m_mutex);
   ASSERT(0 == res);

#elif defined( __AAL_WINDOWS__ )

   this->Lock();

#endif // OS
}
// Release the lock that protects the counters.
void Barrier::CountUnlock()
{
#if   defined( __AAL_LINUX__ )
   int res;

   res = pthread_mutex_unlock(&m_mutex);
   ASSERT(0 == res);

#elif defined( __AAL_WINDOWS__ )

   this->Unlock();

#endif // OS
}

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
   StateLock();
   
   if ( flag_is_set(m_Flags, BARRIER_FLAG_INIT) ) {
      // Already initialized
      StateUnlock();
      return false;
   }

#if   defined( __AAL_WINDOWS__ )
   
   m_hEvent = CreateEvent(NULL,   // no inheritance
                          TRUE,   // manual reset event
                          FALSE,  // not signaled
                          NULL);  // no name
   if ( NULL == m_hEvent ) {
      INIT_UNLOCK();
      return false;
   }
   
#elif defined( __AAL_LINUX__ )
   
   pthread_mutexattr_t attr;
   
   if ( 0 != pthread_mutexattr_init(&attr) ) {
      StateUnlock();
      return false;
   }
   
   if ( 0 != pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) ) {
      pthread_mutexattr_destroy(&attr);
      StateUnlock();
      return false;
   }
   
   if ( 0 != pthread_mutex_init(&m_mutex, &attr) ) {
      pthread_mutexattr_destroy(&attr);
      StateUnlock();
      return false;
   }
   
   pthread_mutexattr_destroy(&attr);
   
   if ( 0 != pthread_cond_init(&m_condition, NULL) ) {
      StateUnlock();
      return false;
   }
   
#endif // OS

   CountLock(); // yes, both locks.

   m_Flags = 0;

   // The current count always starts from zero.
   // The unlock count must be greater than zero upon return from Create().
   m_CurCount    = 0;
   m_UnlockCount = std::max((AAL::btUnsignedInt)1, UnlockCount);

   if ( bAutoReset ) {
      flag_setf(m_Flags, BARRIER_FLAG_AUTO_RESET);
   }

   m_AutoResetManager.Create();

   flag_setf(m_Flags, BARRIER_FLAG_INIT);
   
   CountUnlock();
   StateUnlock();
   
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
   
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      StateUnlock();
      return false;
   }

   flag_setf(m_Flags, BARRIER_FLAG_DESTROYING);

   StateUnlock();

   UnblockAll();

   m_AutoResetManager.WaitForAllWaitersToExit();
   m_AutoResetManager.Destroy();

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

   flag_clrf(m_Flags, BARRIER_FLAG_INIT|BARRIER_FLAG_DESTROYING);
   
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
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_AUTO_RESET|BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Auto-reset is enabled, we're unblocking, we're destroying.
      StateUnlock();
      return false;
   }

   AAL::btBool res = true;
   
   StateUnlockCountLock();

#if defined( __AAL_WINDOWS__ )

   // Reset the manual-reset event to non-signaled.
   if ( !ResetEvent(m_hEvent) ) {
      res = false;
   }
   
#endif // __AAL_WINDOWS__
   
   m_CurCount = 0;
   
   // Note that this treatment of 0 == UnlockCount is different than that of Create().
   // Here, if UnlockCount is zero, we don't change m_UnlockCount. This allows resetting
   //  the Barrier, without having to know the unlock value given to Create().
   if ( UnlockCount > 0 ) {
      m_UnlockCount = UnlockCount;
   }

   CountUnlock();
   
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
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      StateUnlock();
      return false;
   }

   StateUnlockCountLock();
   rCurCount    = m_CurCount;
   rUnlockCount = m_UnlockCount;
   CountUnlock();
   
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
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Unblocking or Destroying.
      StateUnlock();
      return false;
   }

   AAL::btBool res = true;

   StateUnlockCountLock();
   
   const AAL::btBool bWasLocked = (m_CurCount < m_UnlockCount);

   // We let m_CurCount meet, but never exceed, m_UnlockCount.
   AAL::btUnsignedInt c = std::min(nCount, m_UnlockCount - m_CurCount);
   
   m_CurCount += c;

   if ( m_CurCount >= m_UnlockCount ) {

      if ( bWasLocked && flag_is_set(m_Flags, BARRIER_FLAG_AUTO_RESET) ) {
         // Begin a new auto-reset.
         m_AutoResetManager.AutoResetBegin();
      }

#if   defined( __AAL_LINUX__ )

      // Broadcast the condition to all.
      if ( 0 != pthread_cond_broadcast(&m_condition) ) {
         res = false;
      }

#elif defined( __AAL_WINDOWS__ )
      
      // Set the event state to signaled, waking all.
      if ( !SetEvent(m_hEvent) ) {
         res = false;
      }
      
#endif // OS

   }

   CountUnlock();
   
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
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ) {
      // Not initialized.
      StateUnlock();
      return false;
   }

   AAL::btBool res = true;

   StateUnlockCountLock();

   flag_setf(m_Flags, BARRIER_FLAG_UNBLOCKING);

   m_CurCount = m_UnlockCount;

#if   defined( __AAL_LINUX__ )
   
   // Broadcast the condition to all.
   if ( 0 != pthread_cond_broadcast(&m_condition) ) {
      res = false;
   }
      
#elif defined( __AAL_WINDOWS__ )
      
   // Set the event state to signaled, waking all.
   if ( !SetEvent(m_hEvent) ) {
      res = false;
   }
      
#endif // OS

   m_AutoResetManager.UnblockAll();

   CountUnlock();
   
   return res;
}

AAL::btUnsignedInt Barrier::NumWaiters() const
{
   AutoLock(this);
   return m_AutoResetManager.NumWaiters();
}

Barrier::AutoResetManager::AutoResetManager(Barrier *pBarrier) :
   m_pBarrier(pBarrier),
   m_NumWaiters(0),
   m_NumPreWaiters(0)
#if defined( __AAL_WINDOWS__ )
   , m_hREvent(NULL),
     m_hZEvent(NULL)
#endif // __AAL_WINDOWS__
{}

Barrier::AutoResetManager::~AutoResetManager()
{
   ASSERT(0 == m_NumWaiters + m_NumPreWaiters);
}

void Barrier::AutoResetManager::Create()
{
#if   defined( __AAL_WINDOWS__ )

   m_hREvent = CreateEvent(NULL,   // no inheritance
                           TRUE,   // manual reset event
                           FALSE,  // not signaled
                           NULL);  // no name
   ASSERT(NULL != m_hREvent);

   m_hZEvent = CreateEvent(NULL,   // no inheritance
                           TRUE,   // manual reset event
                           FALSE,  // not signaled
                           NULL);  // no name
   ASSERT(NULL != m_hZEvent);

#elif defined( __AAL_LINUX__ )

   int res;
   pthread_mutexattr_t attr;

   res = pthread_mutexattr_init(&attr);
   ASSERT(0 == res);

   res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
   ASSERT(0 == res);

   res = pthread_mutex_init(&m_Rmutex, &attr);
   ASSERT(0 == res);

   res = pthread_mutex_init(&m_Zmutex, &attr);
   ASSERT(0 == res);

   res = pthread_mutexattr_destroy(&attr);
   ASSERT(0 == res);

   res = pthread_cond_init(&m_Rcondition, NULL);
   ASSERT(0 == res);

   res = pthread_cond_init(&m_Zcondition, NULL);
   ASSERT(0 == res);

#endif // OS
}

void Barrier::AutoResetManager::Destroy()
{
   int res;
#if   defined( __AAL_WINDOWS__ )

   res = CloseHandle(m_hREvent) ? 0 : 1;
   ASSERT(0 == res);

   res += CloseHandle(m_hZEvent) ? 0 : 1;
   ASSERT(0 == res);

#elif defined( __AAL_LINUX__ )

   res = pthread_cond_destroy(&m_Rcondition);
   ASSERT(0 == res);

   res += pthread_mutex_destroy(&m_Rmutex);
   ASSERT(0 == res);

   res += pthread_cond_destroy(&m_Zcondition);
   ASSERT(0 == res);

   res += pthread_mutex_destroy(&m_Zmutex);
   ASSERT(0 == res);

#endif // OS
}

void Barrier::AutoResetManager::UnblockAll()
{
   if ( 0 == m_NumWaiters + m_NumPreWaiters ) {
      // There is no waiter to unblock - we're done.
      flag_clrf(m_pBarrier->m_Flags, BARRIER_FLAG_UNBLOCKING);

      if ( flag_is_set(m_pBarrier->m_Flags, BARRIER_FLAG_AUTO_RESET) ) {

         m_pBarrier->m_CurCount = 0;

#if defined( __AAL_WINDOWS__ )
         ResetEvent(m_pBarrier->m_hEvent); // reset the manual-reset event to non-signaled.
#endif // __AAL_WINDOWS__

      }
   }

   AutoResetEnd();
}

void Barrier::AutoResetManager::AddWaiter()
{
   // When doing auto-reset, we cannot increment m_NumWaiters until we have decremented
   //  m_NumWaiters to 0 and reset the Barrier to the locked state. This is to prevent
   //  the case where a thread was unblocked from Wait(), but then called Wait() again,
   //  before m_NumWaiters was decremented to 0. Without this protection, a low-latency
   //  thread executing in a tight loop could "float" m_NumWaiters indefinitely, rendering
   //  the Barrier ineffective.
   // This treatment enforces the hard contract that the Barrier is automatically reset
   //  for each volley of Wait() calls.

   if ( flags_are_set(m_pBarrier->m_Flags, BARRIER_FLAG_AUTO_RESET|BARRIER_FLAG_RESETTING) ) {

      ++m_NumPreWaiters;

WAITLOOP:
      CountUnlock(); // Don't sleep while the outer object is locked.

#if defined( __AAL_LINUX__ )
      pthread_mutex_lock(&m_Rmutex);
#endif // __AAL_LINUX__

      while ( flags_are_set(m_pBarrier->m_Flags, BARRIER_FLAG_AUTO_RESET|BARRIER_FLAG_RESETTING) ) {
         // We must hang out here until the Barrier is fully reset.

#if   defined( __AAL_LINUX__ )

         pthread_cond_wait(&m_Rcondition, &m_Rmutex);

#elif defined( __AAL_WINDOWS__ )

         WaitForSingleObject(m_hREvent, INFINITE);

#endif // OS

      }

#if defined( __AAL_LINUX__ )
      pthread_mutex_unlock(&m_Rmutex);
#endif // __AAL_LINUX__

      CountLock(); // We must grab the outer object's count lock and check again.

      // This check is required because we're crossing lock domains.
      if ( flags_are_set(m_pBarrier->m_Flags, BARRIER_FLAG_AUTO_RESET|BARRIER_FLAG_RESETTING) ) {
         goto WAITLOOP;
      }

      --m_NumPreWaiters;
   }

   // ASSERT: we hold the outer object's count lock, and auto-reset is complete.

   ++m_NumWaiters;
}

AAL::btBool Barrier::AutoResetManager::RemoveWaiter()
{
   if ( 0 == m_NumWaiters ) {
      // Nothing to do.
      return true;
   }

   --m_NumWaiters;

   if ( 0 == m_NumWaiters ) {

      if ( flag_is_set(m_pBarrier->m_Flags, BARRIER_FLAG_RESETTING|BARRIER_FLAG_UNBLOCKING) ) {

         if ( flag_is_set(m_pBarrier->m_Flags, BARRIER_FLAG_AUTO_RESET) ) {
            // The auto-reset is now complete.

            m_pBarrier->m_CurCount = 0;

#if defined( __AAL_WINDOWS__ )
            ResetEvent(m_pBarrier->m_hEvent); // reset the manual-reset event to non-signaled.
#endif // __AAL_WINDOWS__

            AutoResetEnd();

         }

         flag_clrf(m_pBarrier->m_Flags, BARRIER_FLAG_RESETTING|BARRIER_FLAG_UNBLOCKING);
      }

      if ( flag_is_set(m_pBarrier->m_Flags, BARRIER_FLAG_DESTROYING) ) {
#if   defined( __AAL_WINDOWS__ )
         SetEvent(m_hZEvent); // change manual-reset event state to signaled, waking all.
#elif defined( __AAL_LINUX__ )
         pthread_cond_broadcast(&m_Zcondition); // wake all
#endif // OS

         CountUnlock();
         return false;
      }
   }

   return true;
}

AAL::btUnsignedInt Barrier::AutoResetManager::NumWaiters() const
{
   return m_NumWaiters;
}

void Barrier::AutoResetManager::AutoResetBegin()
{
   if ( 0 == m_NumWaiters ) {
      // No waiters means there is no one to block from re-entering Wait() prior to resetting.
      // In this case, we just perform the auto-reset actions here.

      // If we were unblocking, we no longer are.
      flag_clrf(m_pBarrier->m_Flags, BARRIER_FLAG_UNBLOCKING);

      m_pBarrier->m_CurCount = 0;

#if defined( __AAL_WINDOWS__ )
      ResetEvent(m_pBarrier->m_hEvent); // reset the manual-reset event to non-signaled.
#endif // __AAL_WINDOWS__

      AutoResetEnd();

   } else {
      // We have waiters to block from entering new Wait() calls.

      flag_setf(m_pBarrier->m_Flags, BARRIER_FLAG_RESETTING);
#if defined( __AAL_WINDOWS__ )
      ResetEvent(m_hREvent); // change manual-reset event state to non-signaled.
#endif // __AAL_WINDOWS__

   }
}

void Barrier::AutoResetManager::AutoResetEnd()
{
   // Wake all threads blocked in AddWaiter().
   flag_clrf(m_pBarrier->m_Flags, BARRIER_FLAG_RESETTING);

#if   defined( __AAL_WINDOWS__ )
   SetEvent(m_hREvent); // change manual-reset event state to signaled, waking all.
#elif defined( __AAL_LINUX__ )
   pthread_cond_broadcast(&m_Rcondition); // wake all
#endif // OS
}

void Barrier::AutoResetManager::WaitForAllWaitersToExit()
{
#if defined( __AAL_LINUX__ )
   pthread_mutex_lock(&m_Zmutex);
#endif // __AAL_LINUX__

   while ( ( m_NumWaiters + m_NumPreWaiters ) > 0 ) {
      // Wait for the last waiter to call RemoveWaiter().

#if   defined( __AAL_LINUX__ )

      pthread_cond_wait(&m_Zcondition, &m_Zmutex);

#elif defined( __AAL_WINDOWS__ )

      WaitForSingleObject(m_hZEvent, INFINITE);

#endif // OS

   }

#if defined( __AAL_LINUX__ )
   pthread_mutex_unlock(&m_Zmutex);
#endif // __AAL_LINUX__
}

void Barrier::AutoResetManager::CountLock()   { m_pBarrier->CountLock();   }
void Barrier::AutoResetManager::CountUnlock() { m_pBarrier->CountUnlock(); }

#if defined( __AAL_LINUX__ )

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
   StateLock();
   
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Unblocking or Destroying.
      StateUnlock();
      return false;
   }
   
   StateUnlockCountLock();

   AAL::btBool res = true;
   
   m_AutoResetManager.AddWaiter();
   
   while ( m_CurCount < m_UnlockCount ) {

      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
         res = false;
         break;
      }
      
      pthread_cond_wait(&m_condition, &m_mutex);

   }

   if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      res = false;
   }

   if ( m_AutoResetManager.RemoveWaiter() ) {
      CountUnlock();
   }
   
   return res;
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

   StateLock();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Unblocking or Destroying.
      StateUnlock();
      return false;
   }

   // Allow Post()'s and Wait()'s during gettimeofday().
   StateUnlock();

   struct timeval tv;
   gettimeofday(&tv, NULL);

   struct timespec ts;

   ts.tv_sec  = tv.tv_sec;
   ts.tv_nsec = (tv.tv_usec * 1000) + (Timeout * 1000000);

   ts.tv_sec  += ts.tv_nsec / 1000000000;
   ts.tv_nsec %= 1000000000;

   AAL::btBool res = true;

   CountLock();

   m_AutoResetManager.AddWaiter();

   while ( m_CurCount < m_UnlockCount ) {

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
         res = false;
         break;
      }

      // The pthread cond wait API's work as follows:
      // * the mutex object guarding the counter predicate must be locked prior to the call.
      // * when the wait call puts the caller to sleep, it releases the lock prior to doing so.
      // * when the caller wakes, the lock is guaranteed to be held (locked) by the caller.
      // In this way, the examination and mutation of the counter predicate occur atomically.

      if ( ETIMEDOUT == pthread_cond_timedwait(&m_condition,
                                               &m_mutex,
                                               &ts) ) {
         res = false;
         break;
      }

   }

   if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      res = false;
   }

   if ( m_AutoResetManager.RemoveWaiter() ) {
      CountUnlock();
   }

   return res;
}

#elif defined( __AAL_WINDOWS__ )

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

   StateLock();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Unblocking or Destroying.
      StateUnlock();
      return false;
   }

   StateUnlockCountLock();

   m_AutoResetManager.AddWaiter();

WAITLOOP:

   // When un-blocking, don't allow threads to loop and potentially block again.
   if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      if ( m_AutoResetManager.RemoveWaiter() ) {
         CountUnlock();
      }
      return false;
   }

   while ( m_CurCount < m_UnlockCount ) {

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
         if ( m_AutoResetManager.RemoveWaiter() ) {
            CountUnlock();
         }
         return false;
      }

      CountUnlock();

      // ASSERT: we are unlocked - don't wait while locked!
      dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);

      CountLock();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            if ( m_AutoResetManager.RemoveWaiter() ) {
               CountUnlock();
            }
            return false;     // error (using INFINITE above)
         }
      }

   }

   // We must guard both the wake signal and the check of m_CurCount as the atomic operation.
   // If the check below fails, we must wait again (we were preempted, and some other thread
   //  modified m_CurCount before we acquired our internal lock).

   if ( m_CurCount < m_UnlockCount ) {
      goto WAITLOOP;
   }

   if ( m_AutoResetManager.RemoveWaiter() ) {
      CountUnlock();
   }
   
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

   StateLock();
   if ( flag_is_clr(m_Flags, BARRIER_FLAG_INIT) ||
        flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      // Not initialized -or-
      // Unblocking or Destroying.
      StateUnlock();
      return false;
   }

   StateUnlockCountLock();

   m_AutoResetManager.AddWaiter();

WAITLOOP:

   // When un-blocking, don't allow threads to loop and potentially block again.
   if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
      if ( m_AutoResetManager.RemoveWaiter() ) {
         CountUnlock();
      }
      return false;
   }

   while ( m_CurCount < m_UnlockCount ) {

      // If we're being unblocked then immediately return false.
      if ( flag_is_set(m_Flags, BARRIER_FLAG_UNBLOCKING|BARRIER_FLAG_DESTROYING) ) {
         if ( m_AutoResetManager.RemoveWaiter() ) {
            CountUnlock();
         }
         return false;
      }

      CountUnlock();

      // ASSERT: we are unlocked - don't wait while locked!
      dwWaitResult = WaitForSingleObject(m_hEvent, (DWORD)Timeout);

      CountLock();

      switch( dwWaitResult ) {
         case WAIT_OBJECT_0 : break; // event was signaled
         default : {
            if ( m_AutoResetManager.RemoveWaiter() ) {
               CountUnlock();
            }
            return false;     // timeout or error
         }
      }

   }

   // We must guard both the wake signal and the check of m_CurCount as the atomic operation.
   // If the check below fails, we must wait again (we were preempted, and some other thread
   //  modified m_CurCount before we acquired our internal lock).

   if ( m_CurCount < m_UnlockCount ) {
      goto WAITLOOP;
   }

   if ( m_AutoResetManager.RemoveWaiter() ) {
      CountUnlock();
   }
   
   return true;
}

#endif // OS

