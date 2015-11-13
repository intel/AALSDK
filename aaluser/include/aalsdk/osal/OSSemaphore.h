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
/// @file OSSemaphore.h
/// @brief Interface for the CSemaphore class.
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
/// 02/27/2007     JG       Original version
/// 08/23/2007     JG       Modified to use pthread_mutex
/// 12/01/2007     JG       Fixed Windows build problems
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 12/01/2010     AG       Handle semaphore and condition variable teardown in
///                         destructor.
/// 06/28/2011     JG       Added Destroy() to Windows version.
/// 10/21/2011     JG       Some Windows fixes. for constructor and create().
///                           Added Reset for Windows.
/// 06/07/2012     JG       Fixed bug in Create (Windows and Linux) where
///                           Maxcount was improperly set if Initial count
///                           set to positive number.
/// 07/13/2012     TSW      Refactor Windows/Linux split to help prevent
///                          interface divergence and errors.
/// 08/20/2013     JG       Fixed Windows implementation. Modified object to
///                          be a CriticalSection to facilitate thread safety
/// 04/17/2014    JG       Added CurrCount() accessor. Fixed bugs in Destroy 
///                           and Reset()
///                          @endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_OSSEMAPHORE_H__
#define __AALSDK_OSAL_OSSEMAPHORE_H__
#include <aalsdk/osal/CriticalSection.h>

#ifdef __AAL_UNKNOWN_OS__
# error Define Semaphore support for unknown OS.
#endif // __AAL_UNKNOWN_OS__

/// @addtogroup OSAL
/// @{

BEGIN_NAMESPACE(AAL)

/// Interface abstraction for Semaphores.
///
/// The Semaphore is Created with an initial and maximum count value.
/// Calls to Post() increments the value, failing if the maximum
/// value would be exceeded.  Calls to Wait() block if the current
/// value is zero. The Wait() unblocks when the value becomes
/// positive, decrementing the value on the way out. Reset() allows the
/// Semaphore to be reset to a value (less that or equal to MaxCount).
///
/// The CSemaphore class implements a count down feature as well as
/// a count up feature.
///
/// To use the count down feature the user initializes the Semaphore
/// with a positive value (typically equal to the number of resources
/// being controlled).  Calls to Wait() will not block until the total
/// number of Wait() calls equals the Semaphore count. So an initial
/// value of 2 sent into the Create() method will result in 2 calls
/// to Wait() not blocked. The third call will block until a Post()
/// is issued.
///
/// The count up feature is used to have a thread block until a certain
/// number of "Events" (i.e. Post()s ) have occurred.  To use this
/// feature the Semaphore is set to a negative value.  The maximum
/// value will default to 1 if not set otherwise. Calls to Wait()
/// will block until the number of events have occurred.
///
/// NOTES:
/// - Only 1 thread will Wait() at a time.
/// - Issuing multiple Wait()s will result in 1 thread unblocking at the prescribed
///   time and the other blocking on a non-zero event.
/// - Issuing a Create(-1) is the same as a Create(0).
/// - Passing a positive Initial value implies that is the MaxCount.
/// - Reset() does not affect threads currently waiting on a Semaphore.
class OSAL_API CSemaphore : private CriticalSection
{
public:
   /// CSemaphore Constructor.
   CSemaphore();

   /// CSemaphore Destructor.
   virtual ~CSemaphore();

   /// Initialize the Semaphore object before its first use.
   ///
   /// The Semaphore will block when its value is less than or equal to 0.
   ///
   /// @param[in]  nInitialCount  Initial value.
   /// @param[in]  nMaxCount      Maximum value.
   ///
   /// @retval  true   The Semaphore was created.
   /// @retval  false  An error occurred during creation.
   btBool Create(btInt nInitialCount, btUnsignedInt nMaxCount=0);

   /// Destroy the Semaphore, releasing its resources.
   ///
   /// @retval  true   The Semaphore was destroyed.
   /// @retval  false  Semaphore not initialized or an error occurred during destruction.
   btBool Destroy();

   /// Get the current value of the Semaphore counts to nCount.
   ///
   /// @retval  true   The count was set to nCount.
   /// @retval  false  Semaphore not initialized or nCount > max count.
   btBool Reset(btInt nCount = 0);

   /// Get the current value of the Semaphore count to nCount.
   ///
   /// @param[out]  rCurCount    Current count value.
   /// @param[pit]  rMaxCount    Maximum value.
   ///
   /// @retval The count.
   btBool CurrCounts(btInt &rCurCount, btInt &rMaxCount);

   /// If the current count + nCount is less than or equal to the max count, then increment by nCount.
   ///
   /// @retval  true   The count was incremented by nCount.
   /// @retval  false  Semaphore not initialized or current count + nCount > max count.
   btBool Post(btInt nCount);

   /// Causes all Blocked Threads to unblock with a failure.
   ///
   /// @retval  true   All waiters unblocked and return false.
   /// @retval  false  Semaphore not initialized.
   btBool UnblockAll();

   /// Returns the current number of waiters.
   /// NOTE: This is a snapshot and may change by the time the
   ///       caller examines the value
   btUnsignedInt NumWaiters();

   /// Wait for the Semaphore count to become greater than 0 or until the supplied timeout occurs.
   ///
   /// - Passing -1 for Timeout results in the same behavior as the un-timed Wait.
   /// - The count is decremented by one, only if the count becomes greater than zero before the Timeout.
   ///
   /// @param[in]  Timeout  The amount of time to wait from now, in milliseconds.
   ///
   /// @retval  true   The count became greater than 0 before the Timeout expired.
   /// @retval  false  Semaphore not initialized or ( Timeout expired and count not greater than 0 ).
   btBool Wait(btTime Timeout);
   /// Wait for the Semaphore count to become greater than 0, then decrement the count by one.
   ///
   /// @retval  false  Semaphore not initialized.
   btBool Wait();

private:
   btBool          m_bInitialized;
   btInt           m_MaxCount;
   btInt           m_CurCount;
   btUnsignedInt   m_WaitCount;
   btBool          m_bUnBlocking;

#if   defined( __AAL_WINDOWS__ )
   HANDLE          m_hEvent;
#elif defined( __AAL_LINUX__ )
   pthread_cond_t  m_condition;
#endif // OS
};

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_OSAL_OSSEMAPHORE_H__

