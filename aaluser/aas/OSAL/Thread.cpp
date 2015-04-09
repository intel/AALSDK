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
/// @file Thread.cpp
/// @brief Implementation of the OSLThread class.
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
/// 12/16/2007     JG       Changed include path to expose aas/
/// 04/04/2008     JG       Added IsOK methid to OLThread object
///                         Fixed bug in thread destructor that caused a
///                         resource leak if in Linux you do not do a Join()
/// 05/08/2008     HM       Cleaned up windows includes
///                         Fixed bug in OSLThread constructor
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 07/14/2009     HM/JG    Explicit initialization of all fields in OSLThread
///                         Added m_Joined. If the thread has been Joined, then
///                            it is already clean and should not be cancelled &
///                            detached in the dtor.
/// 06/14/2012     HM       Added Cancel()@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/Thread.h"
#include <aalsdk/AALTypes.h>

#if   defined( __AAL_WINDOWS__ )
# include <stdlib.h> // errno_t rand_s(unsigned int *RandomValue);
# include <process.h>
#elif defined( __AAL_LINUX__ )
# include <cstdlib>  // int rand_r(unsigned int *seed);
# include <signal.h>
# include <unistd.h>
#endif // OS


const AAL::btInt OSLThread::sm_PriorityTranslationTable[(AAL::btInt)THREADPRIORITY_COUNT] =
{
#if   defined( __AAL_WINDOWS__ )
   THREAD_PRIORITY_TIME_CRITICAL,
   THREAD_PRIORITY_HIGHEST,
   THREAD_PRIORITY_ABOVE_NORMAL,
   THREAD_PRIORITY_NORMAL,
   THREAD_PRIORITY_BELOW_NORMAL
#elif defined( __AAL_LINUX__ )
   5,
   4,
   3,
   2,
   1
#endif // OS
};

const AAL::btInt OSLThread::sm_DefaultPriority =
               OSLThread::sm_PriorityTranslationTable[THREADPRIORITY_NORMAL];


//=============================================================================
// Name: OSLThread
// Description: Thread abstraction
// Interface: public
// Inputs: pProc - Pointer to procedure to execute
//         nPriority - OSAL thread priority
//         pContext - Context
//         ThisThread - btBool indicating if proc should run in this thread
// Outputs: none.
// Comments:
//=============================================================================
OSLThread::OSLThread(ThreadProc                     pProc,
                     OSLThread::ThreadPriority      nPriority,
                     void                          *pContext,
                     AAL::btBool                    ThisThread) :
#if   defined( __AAL_WINDOWS__ )
   m_hEvent(NULL),
   m_hJoinEvent(NULL),
#elif defined( __AAL_LINUX__ )
   m_Thread(),
   m_Semaphore(),
#endif // OS
   m_tid(),
   m_pProc(pProc),
   m_nPriority(THREADPRIORITY_INVALID),
   m_pContext(pContext),
   m_State(0)
{
   ASSERT(NULL != pProc);

   if ( ( nPriority >= 0 ) &&
        ( (unsigned)nPriority < (sizeof(OSLThread::sm_PriorityTranslationTable) / sizeof(OSLThread::sm_PriorityTranslationTable[0])) ) ) {
      m_nPriority = OSLThread::sm_PriorityTranslationTable[(AAL::btInt)nPriority];
   } else {
      m_nPriority = OSLThread::sm_DefaultPriority;
   }

#if   defined( __AAL_WINDOWS__ )

   m_hEvent     = CreateEvent(NULL, false, false, NULL);
   m_hJoinEvent = CreateEvent(NULL, false, false, NULL);

   if ( ( NULL == m_hEvent ) || ( NULL == m_hJoinEvent ) ) {
      return;
   }

#elif defined( __AAL_LINUX__ )

   if ( !m_Semaphore.Create(0, INT_MAX) ) { // (Without setting THR_ST_OK.)
      return;
   }

#endif // OS

   if ( NULL == pProc ) { // (Without setting THR_ST_OK.)
      return;
   }

   if ( ThisThread ) {
      // Run the thread function locally in this thread.

      flag_setf(m_State, THR_ST_OK|THR_ST_LOCAL);
      OSLThread::StartThread(this);

   } else {

      flag_setf(m_State, THR_ST_OK);

#if   defined( __AAL_WINDOWS__ )
      // Create a new thread to run the thread function.

      uintptr_t res = _beginthread(OSLThread::StartThread, 0, this);
      if ( -1L == res ) {
         flag_clrf(m_State, THR_ST_OK);
      }

#elif defined( __AAL_LINUX__ )

      int res = pthread_create(&m_Thread, NULL, OSLThread::StartThread, this);
      if ( 0 != res ) {
         flag_clrf(m_State, THR_ST_OK);
      }

#endif // OS

   }
}

//=============================================================================
// Name: StartThread
// Description: Thread entry point for newly-spawned and local thread cases.
// Interface: private
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================

#ifdef DBG_OSLTHREAD
# include "dbg_oslthread_0.cpp"
#endif // DBG_OSLTHREAD

#if   defined( __AAL_WINDOWS__ )
void   OSLThread::StartThread(void *p)
#elif defined( __AAL_LINUX__ )
void * OSLThread::StartThread(void *p)
#endif // OS
{
   OSLThread *pThread = reinterpret_cast<OSLThread *>(p);

   if ( NULL == pThread ) {
#if   defined( __AAL_WINDOWS__ )
      return;
#elif defined( __AAL_LINUX__ )
      return NULL;
#endif // OS
   }

   if ( OSLThread::sm_DefaultPriority != pThread->m_nPriority ) {

#if   defined( __AAL_WINDOWS__ )

      ::SetThreadPriority(GetCurrentThread(), pThread->m_nPriority);

#elif defined( __AAL_LINUX__ )

      struct sched_param sp;
      memset(&sp, 0, sizeof(struct sched_param));


      sp.sched_priority = pThread->m_nPriority;
      pthread_setschedparam(pthread_self(), SCHED_RR, &sp);

#endif // OS

   }

   pThread->m_tid = CurrentThreadID();

   ThreadProc fn = pThread->m_pProc;

   if ( NULL != fn ) {
#ifdef DBG_OSLTHREAD
# include "dbg_oslthread_1.cpp"
#endif // DBG_OSLTHREAD

      fn(pThread, pThread->m_pContext);

#ifdef DBG_OSLTHREAD
# include "dbg_oslthread_2.cpp"
#endif // DBG_OSLTHREAD
   }

#if   defined( __AAL_WINDOWS__ )
   SetEvent(pThread->m_hJoinEvent);
#endif // __AAL_WINDOWS__

   if ( flag_is_set(pThread->m_State, THR_ST_DETACHED) ) {
      // Detached threads are responsible for cleaning up after themselves.
      delete pThread;
   }

#if defined( __AAL_LINUX__ )
   return NULL;
#endif // __AAL_LINUX__
}

//=============================================================================
// Name: ~OSLThread
// Description: Destructor
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
OSLThread::~OSLThread()
{
#if   defined( __AAL_WINDOWS__ )

   SetEvent(m_hJoinEvent);
   CloseHandle(m_hJoinEvent);
   CloseHandle(m_hEvent);

#elif defined( __AAL_LINUX__ )

   // The thread is exiting. Post to the internal semaphore so that all waiters can wake (Wait() / Signal()).
   AAL::btInt CurrentCount = 0;
   AAL::btInt MaxCount     = 0;

   m_Semaphore.CurrCounts(CurrentCount, MaxCount);
   m_Semaphore.Post(INT_MAX - CurrentCount);

#endif // OS

   Lock();

   if ( flag_is_set(m_State, THR_ST_OK) &&
        flags_are_clr(m_State, THR_ST_LOCAL|THR_ST_JOINED|THR_ST_DETACHED) ) {

      Detach();  // Detach it to free resources - this thread won't be Join()'ed.

      Unlock();

      if ( IsThisThread( GetThreadID() ) ) {
         // self-destruct
         ExitCurrentThread(0);
      } else {

         Cancel();  // Mark the thread for termination at the next cancellation point.
                    // TODO: refer to man pthread_setcanceltype
                    //       the default cancellation type for threads is PTHREAD_CANCEL_DEFERRED,
                    //       which means that cancellation requests are deferred until the target
                    //       thread next calls a cancellation point.
                    //       Use pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) to cancel
                    //       a thread asap.
      }

   } else {
      Unlock();
   }

}

//=============================================================================
// Name: Unblock
// Description: Send a kill signal to a thread to unblock a system call.
// Interface: public
// Inputs:
// Outputs: none.
// Comments: signum defaults to SIGIO from header file
//=============================================================================
void OSLThread::Unblock()
{
   Lock();

   // We can only manipulate m_Thread when OK and !Local.
   if ( flag_is_clr(m_State, THR_ST_OK) ||
        flag_is_set(m_State, THR_ST_LOCAL) ) {

      if ( flag_is_set(m_State, THR_ST_LOCAL) ) {
         flag_setf(m_State, THR_ST_UNBLOCKED);
      }
      // Nothing else to do.
      Unlock();
      return;
   }

   flag_setf(m_State, THR_ST_UNBLOCKED);

   Unlock();

#if   defined( __AAL_WINDOWS__ )

   //TODO
   SetEvent(m_hEvent);

#elif defined( __AAL_LINUX__ )

   // Mark the thread for termination
   pthread_kill(m_Thread, SIGIO);

#endif // OS
}

//=============================================================================
// Name: Signal
// Description: Signal the event on thread local sync object
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void OSLThread::Signal()
{
#if   defined( __AAL_WINDOWS__ )

   SetEvent(m_hEvent);

#elif defined( __AAL_LINUX__ )

   m_Semaphore.Post(1);

#endif // OS
}

//=============================================================================
// Name: Wait
// Description: Thread local sync object (timed)
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void OSLThread::Wait(AAL::btTime ulMilliseconds)
{
#if   defined( __AAL_WINDOWS__ )

   WaitForSingleObject(m_hEvent, (DWORD)ulMilliseconds);

#elif defined( __AAL_LINUX__ )

   m_Semaphore.Wait(ulMilliseconds);

#endif // OS
}

//=============================================================================
// Name: Wait
// Description: Thread local sync object
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void OSLThread::Wait()
{
#if   defined( __AAL_WINDOWS__ )

   Wait(INFINITE);

#elif defined( __AAL_LINUX__ )

   m_Semaphore.Wait();

#endif // OS
}

//=============================================================================
// Name: Join
// Description: Wait for thread to exit
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void OSLThread::Join()
{
   Lock();

   if ( flag_is_clr(m_State, THR_ST_OK) ||
        flag_is_set(m_State, THR_ST_LOCAL|THR_ST_JOINED|THR_ST_DETACHED) ) {

      if ( flag_is_set(m_State, THR_ST_LOCAL) ) {
         flag_setf(m_State, THR_ST_JOINED);
      }
#if ENABLE_ASSERT
      else {
         const AAL::btBool OSLThread_attempt_to_Join_when_already_Joined_or_Detached = false;
         ASSERT(OSLThread_attempt_to_Join_when_already_Joined_or_Detached);
      }
#endif // ENABLE_ASSERT

      // Nothing else to do.
      Unlock();
      return;
   }

   flag_setf(m_State, THR_ST_JOINED);

   // Don't join while locked.
   Unlock();

#if   defined( __AAL_WINDOWS__ )

   WaitForSingleObject(m_hJoinEvent, INFINITE);

#elif defined( __AAL_LINUX__ )

   void *ret;
   pthread_join(m_Thread, &ret);

#endif // OS
}

// The underlying thread resource will never be join()'ed.
void OSLThread::Detach()
{
   Lock();

   if ( flag_is_clr(m_State, THR_ST_OK) ||
        flag_is_set(m_State, THR_ST_LOCAL|THR_ST_JOINED|THR_ST_DETACHED) ) {

      if ( flag_is_set(m_State, THR_ST_LOCAL) ) {
         flag_setf(m_State, THR_ST_DETACHED);
      }
#if ENABLE_ASSERT
      else {
         const AAL::btBool OSLThread_attempt_to_Detach_when_already_Joined_or_Detached = false;
         ASSERT(OSLThread_attempt_to_Detach_when_already_Joined_or_Detached);
      }
#endif // ENABLE_ASSERT

      // Nothing else to do.
      Unlock();
      return;
   }

   flag_setf(m_State, THR_ST_DETACHED);

   Unlock();

#if   defined( __AAL_WINDOWS__ )

#elif defined( __AAL_LINUX__ )

   // Detach it to free resources.
   pthread_detach(m_Thread);

#endif // OS
}

//=============================================================================
// Name: Cancel
// Description: Do a pthread cancel
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void OSLThread::Cancel()
{
   Lock();

   if ( flag_is_clr(m_State, THR_ST_OK) ||
        flag_is_set(m_State, THR_ST_LOCAL|THR_ST_JOINED) ) {

      if ( flag_is_set(m_State, THR_ST_LOCAL) ) {
         flag_setf(m_State, THR_ST_CANCELED);
      }
#if ENABLE_ASSERT
      else {
         const AAL::btBool OSLThread_attempt_to_Cancel_when_already_Joined = false;
         ASSERT(OSLThread_attempt_to_Cancel_when_already_Joined);
      }
#endif // ENABLE_ASSERT

      // Nothing to do.
      Unlock();
      return;
   }

   flag_setf(m_State, THR_ST_CANCELED);

   Unlock();

#if   defined( __AAL_WINDOWS__ )

   // TODO OSLThread::Cancel for Windows

#elif defined( __AAL_LINUX__ )

   // Mark the thread for termination
   pthread_cancel(m_Thread);

#endif // OS
}

AAL::btBool OSLThread::IsThisThread(AAL::btID id) const
{
#if   defined( __AAL_WINDOWS__ )
   return id == m_tid;
#elif defined( __AAL_LINUX__ )
   return 0 != pthread_equal(id, m_tid);
#endif // OS
}

//=============================================================================
// Name: tid
// Description: return the OS threadID
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
AAL::btTID OSLThread::tid()
{
   return m_tid;
}

/*
//=============================================================================
// Name: SetThreadPriority
// Description:
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
OSAL_API void SetThreadPriority(OSLThread::ThreadPriority nPriority)
{
   if ( ( nPriority >= 0 ) &&
        ( (AAL::btUnsignedInt)nPriority < (sizeof(OSLThread::sm_PriorityTranslationTable) / sizeof(OSLThread::sm_PriorityTranslationTable[0])) ) ) {

      AAL::btInt pri = OSLThread::sm_PriorityTranslationTable[nPriority];

#if   defined( __AAL_WINDOWS__ )

      ::SetThreadPriority(GetCurrentThread(), pri);

#elif defined( __AAL_LINUX__ )

      struct sched_param sp;
      memset(&sp, 0, sizeof(struct sched_param));

      sp.sched_priority = pri;
      pthread_setschedparam(pthread_self(), SCHED_RR, &sp);

#endif // OS

   }
}
*/

//=============================================================================
// Name: GetProcessID
// Description: Return processID
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
AAL::btPID GetProcessID()
{
#if   defined( __AAL_WINDOWS__ )

   return (AAL::btPID) GetCurrentProcessId();

#elif defined( __AAL_LINUX__ )

   return (AAL::btPID) getpid();

#endif // OS
}

//=============================================================================
// Name: GetThreadID
// Description: Return threadID
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
AAL::btTID GetThreadID()
{
#if   defined( __AAL_WINDOWS__ )

   return (AAL::btTID) GetCurrentThreadId();

#elif defined( __AAL_LINUX__ )

   return (AAL::btTID) pthread_self();

#endif // OS
}

void ExitCurrentThread(AAL::btUIntPtr ExitStatus)
{
#if   defined( __AAL_WINDOWS__ )

   ExitThread((DWORD)ExitStatus);

#elif defined( __AAL_LINUX__ )

   pthread_exit((void *)ExitStatus);

#endif // OS
}

/*
//=============================================================================
// Name: GetNumProcessors
// Description:
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
OSAL_API AAL::btInt GetNumProcessors()
{
#if   defined( __AAL_WINDOWS__ )

   return 1;   // TODO GetNumProcessors() fixed for now
   // SYSTEM_INFO si;
   // GetSystemInfo(&si);
   // return (int) si.dwNumberOfProcessors;

#elif defined( __AAL_LINUX__ )

   return 1;   // TODO GetNumProcessors() fixed for now
   // return sysconf(_SC_NPROCESSORS_ONLN);

#endif // OS
}
*/

OSAL_API AAL::btUnsigned32bitInt GetRand(AAL::btUnsigned32bitInt *storage)
{
   ASSERT(NULL != storage);
#if   defined( __AAL_WINDOWS__ )
   unsigned int seed = (unsigned int)*storage;
   rand_s(&seed);
   return (*storage = (AAL::btUnsigned32bitInt)seed);
#elif defined( __AAL_LINUX__ )
   unsigned int            seed = (unsigned int)*storage;
   AAL::btUnsigned32bitInt val  = (AAL::btUnsigned32bitInt)rand_r(&seed);
   *storage = (AAL::btUnsigned32bitInt)seed;
   return val;
#endif // OS
}

