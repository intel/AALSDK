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
/// @file Barrier.h
/// @brief Interface for the Barrier synchronization primitive class.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/21/2015     TSW      Original version @endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_BARRIER_H__
#define __AALSDK_OSAL_BARRIER_H__
#include <aalsdk/osal/CriticalSection.h>

#ifdef __AAL_UNKNOWN_OS__
# error Define Barrier support for unknown OS.
#endif // __AAL_UNKNOWN_OS__

/// @addtogroup OSAL
/// @{


/// Interface abstraction for the Barrier synchronization primitive.
///
class OSAL_API Barrier : private CriticalSection
{
public:
   /// Barrier Constructor.
   Barrier();

   /// Barrier Destructor.
   virtual ~Barrier();

   /// Initialize the Barrier object before its first use.
   ///
   /// The Barrier is created in the locked state, and will remain locked
   /// until UnlockCount values have been Post()'ed.
   ///
   /// @param[in]  UnlockCount  The required number of Post() counts before the Barrier opens.
   /// @param[in]  bAutoReset   Whether the Barrier is to automatically close itself when all
   ///                           waiters have resumed from Wait() calls.
   ///
   /// @retval  true   The Barrier was created.
   /// @retval  false  An error occurred during creation.
   AAL::btBool Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset=false);

   /// Destroy the Barrier, releasing its resources.
   ///
   /// @retval  true   The Barrier was destroyed.
   /// @retval  false  Barrier not initialized or an error occurred during destruction.
   AAL::btBool Destroy();

   /// Reset the Barrier to locked (manual-reset Barrier, only).
   ///
   /// The current count is set to zero. The unlock count is modified only if UnlockCount != 0.
   ///
   /// @retval  true   The Barrier was reset.
   /// @retval  false  Barrier not initialized or error during the reset attempt.
   AAL::btBool Reset(AAL::btUnsignedInt UnlockCount=0);

   /// Get the current values of the Barrier counts.
   ///
   /// @param[out]  rCurCount  Current count value.
   /// @param[out]  rMaxCount  Unlock count value.
   ///
   /// @retval  true   The counts were retrieved.
   /// @retval  false  Barrier not initialized.
   AAL::btBool CurrCounts(AAL::btUnsignedInt &rCurCount, AAL::btUnsignedInt &rUnlockCount);

   /// Increment the current count by the minimum of nCount and the unlock count - current count.
   ///
   /// If the current count becomes equal to the unlock count, wake all threads blocked
   /// on the Barrier.
   ///
   /// @retval  true   The operation was successful. Does not necessarily indicate that threads were resumed.
   /// @retval  false  Barrier not initialized or error during the post attempt.
   AAL::btBool Post(AAL::btUnsignedInt nCount);

   /// Causes all Blocked Threads to unblock with a failure.
   ///
   /// @retval  true   All waiters became unblocked and will return false from their Wait() calls.
   /// @retval  false  Barrier not initialized or error during the unblock attempt.
   AAL::btBool UnblockAll();

   /// Returns the current number of waiters.
   /// NOTE: This is a snapshot and may change by the time the caller examines the value.
   AAL::btUnsignedInt NumWaiters() const;

   /// Wait for the Barrier count to become equal to the unlock count.
   ///
   /// @retval  false  Barrier not initialized, failed wait attempt, or this call was canceled by
   ///          UnblockAll().
   AAL::btBool Wait();

   /// Wait for the Barrier count to become equal to the unlock count, or until a timeout expires.
   ///
   /// - Passing AAL_INFINITE_WAIT for Timeout results in the same behavior as the un-timed Wait.
   ///
   /// @param[in]  Timeout  The amount of time to wait from now, in milliseconds.
   ///
   /// @retval  true   The count became equal to the unlock count before the Timeout expired.
   /// @retval  false  Barrier not initialized, failed wait attempt, this call was canceled by
   ///                 UnblockAll(), or Timeout expired.
   AAL::btBool Wait(AAL::btTime Timeout);

private:
   AAL::btUnsignedInt m_Flags;
#define BARRIER_FLAG_INIT       0x00000001
#define BARRIER_FLAG_AUTO_RESET 0x00000002
#define BARRIER_FLAG_RESETTING  0x00000004
#define BARRIER_FLAG_UNBLOCKING 0x00000008
#define BARRIER_FLAG_DESTROYING 0x00000010
   AAL::btUnsignedInt m_UnlockCount;
   AAL::btUnsignedInt m_CurCount;

   // We always reference m_AutoResetManager within the context of the Barrier
   //  locks, so there is no need to be concerned with locking within AutoResetManager.
   class AutoResetManager
   {
   public:
      AutoResetManager(Barrier *pBarrier);
      virtual ~AutoResetManager();

      void Create();
      void Destroy();

      void UnblockAll();
      void AddWaiter();
      // true if the caller is to unlock the count mutex (CountUnlock()).
      // false if the count mutex was already unlocked.
      AAL::btBool RemoveWaiter();

      AAL::btUnsignedInt NumWaiters() const;

      void AutoResetBegin();
      void AutoResetEnd();

      void WaitForAllWaitersToExit();

   protected:
      Barrier           *m_pBarrier;
      AAL::btUnsignedInt m_NumWaiters;    // number of threads blocked in Wait() calls on a locked Barrier.
      AAL::btUnsignedInt m_NumPreWaiters; // number of threads blocked in Wait() by an auto-reset.
      AAL::btTime        m_WaitTimeout;
#if   defined( __AAL_WINDOWS__ )
      HANDLE             m_hREvent;       // manual-reset event for auto-reset done.
      HANDLE             m_hZEvent;       // manual-reset event for zero waiters.
#elif defined( __AAL_LINUX__ )
      pthread_mutex_t    m_Rmutex;
      pthread_cond_t     m_Rcondition;
      pthread_mutex_t    m_Zmutex;
      pthread_cond_t     m_Zcondition;
#endif // OS

      void WaitForAutoResetCompletion();
      void WaitForAutoResetCompletion(AAL::btTime );

      void CountLock();
      void CountUnlock();
   };

   AutoResetManager   m_AutoResetManager;

#if   defined( __AAL_WINDOWS__ )
   HANDLE             m_hEvent;
#elif defined( __AAL_LINUX__ )
   pthread_mutex_t    m_mutex;
   pthread_cond_t     m_condition;
#endif // OS

   friend class AutoResetManager;

   void StateLock();
   void StateUnlock();
   void StateUnlockCountLock();
   void CountLock();
   void CountUnlock();
};

/// @} group OSAL

#endif // __AALSDK_OSAL_BARRIER_H__

