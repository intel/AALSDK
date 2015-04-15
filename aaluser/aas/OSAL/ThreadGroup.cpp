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
/// @file ThreadGroup.cpp
/// @brief Implementation of the ThreadGroup class
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Cleaned up windows includes
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
/// 03/06/2014     JG       Complete rewrite.
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/ThreadGroup.h"
#include "aalsdk/osal/Sleep.h"

////////////////////////////////////////////////////////////////////////////////

// Functor that signals completion of a call to OSLThreadGroup::Drain().
class OSLThreadGroupNestedSemPostD : public IDispatchable
{
public:
   OSLThreadGroupNestedSemPostD(IDispatchable *pContained,
                                CSemaphore    &Sem,
                                AAL::btInt     Value=1) :
      m_pContained(pContained),
      m_Sem(Sem),
      m_Value(Value)
   {}

   void operator() ()
   {
      (*m_pContained) ();  // Execute the contained work item.
      m_Sem.Post(m_Value); // signal its completion.
      delete this;
   }

protected:
   IDispatchable *m_pContained;
   CSemaphore    &m_Sem;
   AAL::btInt     m_Value;
};

////////////////////////////////////////////////////////////////////////////////
// OSLThreadGroup::ThrGroupState

OSLThreadGroup::ThrGrpState::ThrGrpState(AAL::btUnsignedInt NumThreads) :
   m_eState(IThreadGroup::Running),
   m_Flags(THRGRPSTATE_FLAG_OK),
   m_DrainNestLevel(0),
   m_WorkSemTimeout(AAL_INFINITE_WAIT),
   m_SelfJoiner(0)
{
   AAL::btInt SemInitCount = (AAL::btInt)NumThreads;
   SemInitCount = -SemInitCount;

   if ( !m_ThrStartSem.Create(SemInitCount, INT_MAX) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }

   if ( !m_ThrExitSem.Create(SemInitCount, INT_MAX) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }

   if ( !m_WorkSem.Create(0, INT_MAX) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }
}

//=============================================================================
// Name: GetNumThreads
// Description: Get Number of Threads
// Interface: public
// Comments:
//=============================================================================
AAL::btUnsignedInt OSLThreadGroup::ThrGrpState::GetNumThreads() const
{
   AutoLock(this);
   return (AAL::btUnsignedInt) m_RunningThreads.size();
}

//=============================================================================
// Name: GetNumWorkItems
// Description: Get Number of workitems
// Interface: public
// Comments:
//=============================================================================
AAL::btUnsignedInt OSLThreadGroup::ThrGrpState::GetNumWorkItems() const
{
   AutoLock(this);
   return (AAL::btUnsignedInt) m_workqueue.size();
}

//=============================================================================
// Name: Add
// Description: Submits a work object for disposition
// Interface: public
// Inputs: pwi - pointer to work item object.
// Outputs: none.
// Comments: Object gets placed on the dispatch queue where it gets picked up
//           by the next available thread.
//=============================================================================
AAL::btBool OSLThreadGroup::ThrGrpState::Add(IDispatchable *pDisp)
{
   ASSERT(NULL != pDisp);
   if ( NULL == pDisp ) {
      return false;
   }

   Lock();

   const IThreadGroup::eState state = State();

   // We allow new work items when Running or Joining.
   if ( ( IThreadGroup::Stopped  == state ) ||
        ( IThreadGroup::Draining == state ) ) {
      Unlock();
      return false;
   }

   m_workqueue.push(pDisp);

   Unlock();

   // Signal the semaphore outside the critical section so that waking threads have an
   // opportunity to immediately acquire it.
   m_WorkSem.Post(1);

   return true;
}

OSLThread * OSLThreadGroup::ThrGrpState::ThreadRunningInThisGroup(AAL::btTID tid) const
{
   AutoLock(this);

   const_thr_list_iter iter;
   for ( iter = m_RunningThreads.begin() ; m_RunningThreads.end() != iter ; ++iter ) {
      if ( (*iter)->IsThisThread(tid) ) {
         return *iter;
      }
   }
   return NULL;
}

void OSLThreadGroup::ThrGrpState::WorkerHasStarted(OSLThread *pThread)
{
   Lock();
   m_RunningThreads.push_back(pThread);
   Unlock();

   m_ThrStartSem.Post(1);
}

AAL::btBool OSLThreadGroup::ThrGrpState::WaitForAllWorkersToStart(AAL::btTime Timeout)
{
   return m_ThrStartSem.Wait(Timeout);
}

void OSLThreadGroup::ThrGrpState::WorkerHasExited(OSLThread *pThread)
{
   RunningToExited(pThread);
   m_ThrExitSem.Post(1);
}

void OSLThreadGroup::ThrGrpState::RunningToExited(OSLThread *pThread)
{
   AutoLock(this);

   thr_list_iter iter;

   // Move the worker from Running to Exited.
   for ( iter = m_RunningThreads.begin() ; m_RunningThreads.end() != iter ; ++iter ) {
      if ( (*iter) == pThread ) {
         m_RunningThreads.erase(iter);
         m_ExitedThreads.push_back(pThread);
         break;
      }
   }
}

void OSLThreadGroup::ThrGrpState::RemoveFromRunning(OSLThread *pThread)
{
   AutoLock(this);

   thr_list_iter iter;

   // Remove the worker from Running.
   for ( iter = m_RunningThreads.begin() ; m_RunningThreads.end() != iter ; ++iter ) {
      if ( (*iter) == pThread ) {
         m_RunningThreads.erase(iter);
         break;
      }
   }

}

AAL::btBool OSLThreadGroup::ThrGrpState::WaitForAllWorkersToExit(AAL::btTime Timeout)
{
   return m_ThrExitSem.Wait(Timeout);
}

AAL::btBool OSLThreadGroup::ThrGrpState::PollForAllWorkersToJoin(AAL::btTime Timeout)
{
   const AAL::btTime Interval = PollingInterval();

   Lock();

   if ( AAL_INFINITE_WAIT == Timeout ) {

      while ( flag_is_clr(m_Flags, THRGRPSTATE_FLAG_JOINED) ) {
         Unlock();
         AAL::SleepMilli(Interval);
         Lock();
      }

   } else {
      AAL::btTime Subtract;

      while ( flag_is_clr(m_Flags, THRGRPSTATE_FLAG_JOINED) ) {
         Unlock();
         Subtract = std::min(Timeout, Interval);
         AAL::SleepMilli(Subtract);
         Timeout -= Subtract;
         if ( 0 == Timeout ) {
            return false;
         }
         Lock();
      }
   }

   Unlock();

   return true;
}

AAL::btBool OSLThreadGroup::ThrGrpState::PollForDrainCompletion(AAL::btTime Timeout)
{
   const AAL::btTime Interval = PollingInterval();

   Lock();

   if ( AAL_INFINITE_WAIT == Timeout ) {

      while ( flag_is_clr(m_Flags, THRGRPSTATE_FLAG_DRAINED) ) {
         Unlock();
         AAL::SleepMilli(Interval);
         Lock();
      }

   } else {
      AAL::btTime Subtract;

      while ( flag_is_clr(m_Flags, THRGRPSTATE_FLAG_DRAINED) ) {
         Unlock();
         Subtract = std::min(Timeout, Interval);
         AAL::SleepMilli(Subtract);
         Timeout -= Subtract;
         if ( 0 == Timeout ) {
            return false;
         }
         Lock();
      }
   }

   Unlock();

   return true;
}

AAL::btBool OSLThreadGroup::ThrGrpState::Join(AAL::btTime Timeout)
{
   thr_list_iter iter;
   AAL::btBool   res;

   Lock();

   const IThreadGroup::eState st = State();

   if ( IThreadGroup::Joining == st ) {
      // Prevent nested / multiple Join().
      Unlock();
      return false;
   }

   const AAL::btTID MyThrID = GetThreadID();
   OSLThread       *pThread = ThreadRunningInThisGroup(MyThrID);
   IDispatchable   *pWork;

   // We are now Joining.
   State(IThreadGroup::Joining);

   // Workers are no longer required to block infinitely on work items. Give them a
   // chance to wake periodically and see that the thread group is being joined.
   m_WorkSemTimeout = PollingInterval();

   // Wake any threads that happen to be blocked infinitely.
   m_WorkSem.Post( (AAL::btInt) m_RunningThreads.size() );

   if ( NULL != pThread ) {
      // self-referential Join()
      m_SelfJoiner = MyThrID;
      flag_setf(m_Flags, THRGRPSTATE_FLAG_SELF_JOIN);

      // We need to continue to execute work.
      while ( m_workqueue.size() > 0 ) {
         pWork = m_workqueue.front();
         m_workqueue.pop();
         Unlock();

         pWork->operator() ();

         Lock();
      }

      // Remove this thread from the Running list of workers. This is to make sure that we're
      //  not moved into the Exited list, so that we don't attempt to Join() ourself below.
      RemoveFromRunning(pThread);

      // Post()'s m_ThrExitSem so that we don't block on ourself below.
      WorkerHasExited(pThread);

      Unlock();

      res = WaitForAllWorkersToExit(Timeout);
      if ( !res ) {
         return false;
      }

      // Join() all workers other than this thread.
      for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
         if ( NULL == *iter ) {
            continue;
         }
         (*iter)->Join();
         delete *iter;
         *iter = NULL;
      }

      flag_setf(m_Flags, THRGRPSTATE_FLAG_JOINED);

      // self-destruct. This thread terminates.
      delete pThread;

      // We must not return from this call.

      ASSERT(false);
      return false;
   }

   // Not a self-referential Join().

   Unlock();

   // Wait for all workers to exit.
   res = WaitForAllWorkersToExit(Timeout);
   if ( !res ) {
      return false;
   }

   for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
      if ( NULL == *iter ) {
         continue;
      }
      (*iter)->Join();
      delete *iter;
      *iter = NULL;
   }

   flag_setf(m_Flags, THRGRPSTATE_FLAG_JOINED);
   return true;
}

//=============================================================================
// Name: Drain
// Description: Synchronize with the execution of any work items currently in
//              the queue. All work items in the queue will be executed
//              before returning.
// Returns: true - success
//          false - Attempt to Drain from a member Thread
// Interface: public
// Comments:
//=============================================================================
AAL::btBool OSLThreadGroup::ThrGrpState::Drain()
{
   // Refer to the state checking mutator:
   //   IThreadGroup::eState OSLThreadGroup::ThrGrpState::State(IThreadGroup::eState st);
   //
   // The only valid transitions to state Draining are
   //   Running  -> Draining
   //   Draining -> Draining (nested Drain() calls)
   //
   // Allowing nested Drain()'s presents a problem - as the inner Drain()'s complete, they
   // must not set the state of the OSLThreadGroup to Running (this would allow Add()'s during
   // the outer Drain()'s, eg). The inner Drain()'s must leave the state set to Draining, and
   // only the last Drain() to complete can set the state back to Running.

   Lock();

   AAL::btInt         items = (AAL::btInt) m_workqueue.size();
   AAL::btUnsignedInt nesting;

   // No need to drain if already empty.
   if ( 0 == items ) {
      Unlock();
      return true;
   }

   // Check for other state conflicts.
   if ( IThreadGroup::Draining != State(IThreadGroup::Draining) ) {
      // Can't drain now - state conflict.
      ASSERT(false);
      Unlock();
      return false;
   }

   nesting = DrainNestingUp();

   if ( 1 == nesting ) {
      // New Drain() call.
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_DRAINED);
   }

   const AAL::btTID MyThrID = GetThreadID();
   const OSLThread *pThread = ThreadRunningInThisGroup(MyThrID);

   IDispatchable *pWork;
   CSemaphore     DrainSem;
   work_queue_t   tmpq;

   if ( NULL != pThread ) {
      // Self-referential Drain() - we need to continue to execute work.

      while ( m_workqueue.size() > 0 ) {
         pWork = m_workqueue.front();
         m_workqueue.pop();
         Unlock();

         pWork->operator() ();

         Lock();
      }

      goto DONE;
   }

   if ( !DrainSem.Create(-items, 1) ) {
      nesting = DrainNestingDown();
      State((0 == nesting) ? IThreadGroup::Running : IThreadGroup::Draining);
      ASSERT(false);
      Unlock();
      return false;
   }

   // Pull each item from the work queue, and wrap it in a OSLThreadGroupNestedSemPostD() object.
   while ( m_workqueue.size() > 0 ) {
      IDispatchable *pContained = m_workqueue.front();
      m_workqueue.pop();
      tmpq.push( new(std::nothrow) OSLThreadGroupNestedSemPostD(pContained, DrainSem) );
   }

   // Re-populate the work queue.
   while ( tmpq.size() > 0 ) {
      m_workqueue.push(tmpq.front());
      tmpq.pop();
   }

   Unlock();

   // Wait for the work items to complete. Don't wait while locked.
   DrainSem.Wait();

   Lock();
   // Protect Drain() nesting level.

DONE:

   nesting = DrainNestingDown();

   IThreadGroup::eState st = State();

   if ( IThreadGroup::Joining == st ) {
      // Join() during Drain() is allowed. In this case, an attempt to set the
      // state to Running or Draining below would be denied. We want the success indicator here,
      // however, to reflect that the thread group was drained of items, hence this check.
      Unlock();

      if ( 0 == nesting ) {
         // Drain() is complete.
         flag_setf(m_Flags, THRGRPSTATE_FLAG_DRAINED);
      }

      return true;
   }

   st = (0 == nesting) ? IThreadGroup::Running : IThreadGroup::Draining;

   AAL::btBool res = (State(st) == st);

   Unlock();

   if ( 0 == nesting ) {
      // Drain() is complete.
      flag_setf(m_Flags, THRGRPSTATE_FLAG_DRAINED);
   }

   return res;
}

//=============================================================================
// Name: Stop
// Description: Stop all threads.
// Interface: public
// Input: flush - If true will cause all messages to be destroyed before
//        return. Some items may not be dispatched
// Comments:
//=============================================================================
void OSLThreadGroup::ThrGrpState::Stop()
{
   AutoLock(this);

   if ( IThreadGroup::Stopped != State(IThreadGroup::Stopped) ) {
      // State conflict, can't stop right now.
      ASSERT(false);
      return;
   }

   // We're flushing all current work items. This Reset() will fail if there are currently
   // blockers on m_WorkSem; but if workers are blocked, then the work queue must be empty,
   // which is what we want anyway - no need to check the return value from Reset().
   m_WorkSem.Reset(0);

   // If there is something on the queue then remove it and destroy it.
   while ( m_workqueue.size() > 0 ) {
      IDispatchable *wi = m_workqueue.front();
      m_workqueue.pop();
      delete wi;
   }
}

//=============================================================================
// Name: Start
// Description: Restart the Thread Group
// Interface: public
// Comments:
//=============================================================================
AAL::btBool OSLThreadGroup::ThrGrpState::Start()
{
   AutoLock(this);

   if ( IThreadGroup::Running == State(IThreadGroup::Running) ) {
      AAL::btInt s = (AAL::btInt) m_workqueue.size();

      AAL::btInt c = 0;
      AAL::btInt m = 0;
      m_WorkSem.CurrCounts(c, m);

      if ( c >= s ) {
         // Nothing to do.
         return true;
      }

      return m_WorkSem.Post(s - c);
   }

   ASSERT(false);
   return false;
}

// Do a state transition.
IThreadGroup::eState OSLThreadGroup::ThrGrpState::State(IThreadGroup::eState st)
{
   AutoLock(this);

   if ( IThreadGroup::Joining == m_eState ) {
      // Joining is a final state. Deny all requests to do otherwise.
      return m_eState;
   }

   switch ( m_eState ) {
      case IThreadGroup::Running : {
         switch ( st ) {
            /* case IThreadGroup::Running :  break; */ // Running -> Running (nop)
            case IThreadGroup::Stopped  : m_eState = st; break; // Running -> Stopped  [ Stop()  ]
            case IThreadGroup::Draining : m_eState = st; break; // Running -> Draining [ Drain() ]
            case IThreadGroup::Joining  : m_eState = st; break; // Running -> Joining  [ Join()  ]
         }
      } break;

      case IThreadGroup::Stopped : {
         switch ( st ) {
            case IThreadGroup::Running  : m_eState = st; break; // Stopped -> Running   [ Start() ]
            /* case IThreadGroup::Stopped : break; */ // Stopped -> Stopped (nop)
            case IThreadGroup::Draining : ASSERT(IThreadGroup::Draining != st); break; // Invalid (queue empty check)
            case IThreadGroup::Joining  : m_eState = st; break; // Stopped -> Joining   [ Join(), ~OSLThreadGroup() ]
         }
      } break;

      case IThreadGroup::Draining : {
        switch ( st ) {
            case IThreadGroup::Running  : m_eState = st; break; // Draining -> Running  [ Drain() ]
            case IThreadGroup::Stopped  : ASSERT(IThreadGroup::Stopped != st);  break; // Invalid
            /* case IThreadGroup::Draining : break; */ // Draining -> Draining (nop)
            case IThreadGroup::Joining  : m_eState = st; break; // Draining -> Joining  [ Join()  ]
         }
      } break;
   }

   return m_eState;
}

AAL::btBool OSLThreadGroup::ThrGrpState::CreateWorkerThread(ThreadProc                fn,
                                                            OSLThread::ThreadPriority pri,
                                                            void                     *context)
{
   OSLThread *pthread = new(std::nothrow) OSLThread(fn, pri, context);

   ASSERT(NULL != pthread);
   if ( NULL == pthread ) {
      ASSERT(false);
      return false;
   }

   return true;
}

AAL::btBool OSLThreadGroup::ThrGrpState::Destroy(AAL::btTime Timeout)
{
   Lock();

   const IThreadGroup::eState st = State();

   if ( IThreadGroup::Draining == st ) {
      return DestroyWhileDraining(Timeout);
   }

   if ( IThreadGroup::Joining == st ) {
      return DestroyWhileJoining(Timeout);
   }

   // We're not Drain()'ing and we haven't Join()'ed workers, yet.

   AAL::btBool    res;
   thr_list_iter  iter;
   IDispatchable *pWork;
   OSLThread     *pThread = ThreadRunningInThisGroup( GetThreadID() );

   // Workers are no longer required to block infinitely on work items. Give them a chance
   // to wake periodically and see that the thread group is being joined.
   m_WorkSemTimeout = PollingInterval();

   State(IThreadGroup::Joining);

   // Wake any threads that happen to be blocked infinitely.
   m_WorkSem.Post( (AAL::btInt) m_RunningThreads.size() );

   if ( NULL != pThread ) {
      // Self-referential Destroy().

      while ( m_workqueue.size() > 0 ) {
         pWork = m_workqueue.front();
         m_workqueue.pop();
         Unlock();

         pWork->operator() ();

         Lock();
      }

      // Remove this thread from the Running list of workers. This is to make sure that we're
      //  not moved into the Exited list, so that we don't attempt to Join() ourself below.
      RemoveFromRunning(pThread);

      // Post()'s m_ThrExitSem so that we don't block ourself below.
      WorkerHasExited(pThread);

      Unlock();

      // Wait for all workers to Exit.
      res = WaitForAllWorkersToExit(Timeout);
      if ( !res ) {
         return false;
      }

      for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
         if ( NULL == *iter ) {
            continue;
         }
         (*iter)->Join();
         delete *iter;
         (*iter) = NULL;
      }

      delete this;

      // self-destruct - this thread will terminate.
      delete pThread;

      // We must not return from this fn.
      ASSERT(false);
      return true;
   }

   // Not a self-referential Destroy(). First time Join()'ing.

   Unlock();

   res = WaitForAllWorkersToExit(Timeout);
   if ( !res ) {
      return false;
   }

   for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
      if ( NULL == *iter ) {
         continue;
      }
      (*iter)->Join();
      delete *iter;
      *iter = NULL;
   }

   delete this;
   return true;
}

AAL::btBool OSLThreadGroup::ThrGrpState::DestroyWhileDraining(AAL::btTime Timeout)
{
   // ** We are still Lock()'ed from the initial call to Destroy().

   // Destroy while another thread is actively Drain()'ing.
   // We must wait for the Drain() to complete.

   // We know that a Drain() is actively in progress, because Drain() will transition the
   //  state back to Running or Joining when all Drain()'ers are finished.

   thr_list_iter  iter;
   IDispatchable *pWork;
   AAL::btBool    res;
   OSLThread     *pThread = ThreadRunningInThisGroup( GetThreadID() );

   // Workers are no longer required to block infinitely on work items. Give them a chance
   // to wake periodically and see that the thread group is being joined.
   m_WorkSemTimeout = PollingInterval();

   // Transition the state to Joining here. This is to prevent another thread from beginning
   //  a Join() once we unlock the critical section. Destroy() is essentially a Join(),
   //  and we don't allow Join()'s to nest.
   State(IThreadGroup::Joining);

   // Wake any threads that happen to be blocked infinitely.
   m_WorkSem.Post( (AAL::btInt) m_RunningThreads.size() );

   if ( NULL != pThread ) {
      // self-referential Destroy() during Drain().

      // continue to execute work on behalf of the Drain().

      while ( m_workqueue.size() > 0 ) {
         pWork = m_workqueue.front();
         m_workqueue.pop();
         Unlock();

         pWork->operator() ();

         Lock();
      }

      // Remove this thread from the Running list of workers. This is to make sure that we're
      //  not moved into the Exited list, so that we don't attempt to Join() ourself below.
      RemoveFromRunning(pThread);

      // Post()'s m_ThrExitSem so that we don't block on ourself below.
      WorkerHasExited(pThread);

      Unlock();

      res = PollForDrainCompletion(Timeout);
      if ( !res ) {
         return false;
      }

      res = WaitForAllWorkersToExit(Timeout);
      if ( !res ) {
         return false;
      }

      // Join() all workers other than this thread.
      for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
         if ( NULL == *iter ) {
            continue;
         }
         (*iter)->Join();
         delete *iter;
         *iter = NULL;
      }

      delete this;

      // Self-destruct - this thread will terminate.
      delete pThread;

      // We must not return from this call.
      ASSERT(false);
      return true;
   }

   Unlock();

   // non-self-referential Destroy() during Drain().

   res = PollForDrainCompletion(Timeout);
   if ( !res ) {
      return false;
   }

   res = WaitForAllWorkersToExit(Timeout);
   if ( !res ) {
      return false;
   }

   // Join all workers.
   for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
      if ( NULL == *iter ) {
         continue;
      }
      (*iter)->Join();
      delete *iter;
      *iter = NULL;
   }

   delete this;
   return true;
}

AAL::btBool OSLThreadGroup::ThrGrpState::DestroyWhileJoining(AAL::btTime Timeout)
{
   // ** We are still Lock()'ed from the initial call to Destroy().

   // Destroy while another thread may be actively Join()'ing.
   // We must wait for the Join() to complete.

   thr_list_iter  iter;
   IDispatchable *pWork;
   AAL::btBool    res;
   OSLThread     *pThread = ThreadRunningInThisGroup( GetThreadID() );

   // Workers are no longer required to block infinitely on work items. Give them a chance
   // to wake periodically and see that the thread group is being joined.
   m_WorkSemTimeout = PollingInterval();

   // Wake any threads that happen to be blocked infinitely.
   m_WorkSem.Post( (AAL::btInt) m_RunningThreads.size() );

   if ( NULL != pThread ) {

      // Continue to dispatch work on behalf of the Join().
      while ( m_workqueue.size() > 0 ) {
         pWork = m_workqueue.front();
         m_workqueue.pop();
         Unlock();

         pWork->operator() ();

         Lock();
      }

      // Remove this thread from the Running list of workers. This is to make sure that we're
      //  not moved into the Exited list, so that the Join()'er doesn't attempt to Join() us.
      RemoveFromRunning(pThread);

      // Post()'s m_ThrExitSem so that the Join()'er doesn't attempt to Join() us.
      WorkerHasExited(pThread);

      Unlock();

      if ( flag_is_set(m_Flags, THRGRPSTATE_FLAG_SELF_JOIN) &&
           pThread->IsThisThread(m_SelfJoiner) ) {
         // We executed an OSLThreadGroup::Join() call for this thread group earlier.
         // Now, we just do the Join() here, before we self-destruct below.

         res = WaitForAllWorkersToExit(Timeout);
         if ( !res ) {
            return false;
         }

         for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
            if ( NULL == *iter ) {
               continue;
            }
            (*iter)->Join();
            delete *iter;
            *iter = NULL;
         }

      } else {

         // Wait for the Join() to complete.
         res = PollForAllWorkersToJoin(Timeout);
         if ( !res ) {
            return false;
         }

      }

      delete this;

      // Self-destruct - this thread will terminate.
      delete pThread;

      // We must not return from this call.
      ASSERT(false);
      return true;
   }

   // non-self-referential Destroy() during Join().

   Unlock();

   res = PollForAllWorkersToJoin(Timeout);
   if ( !res ) {
      return false;
   }

   delete this;
   return true;
}

//=============================================================================
// Name: GetWorkItem
// Description: Get next work item and the current ThreadGroup state
// Interface: public
// Comments:
//=============================================================================
IThreadGroup::eState OSLThreadGroup::ThrGrpState::GetWorkItem(IDispatchable * &pWork)
{
   // Wait for work item
   m_WorkSem.Wait(m_WorkSemTimeout);

   // Lock until flag and queue have been processed
   Lock();

   const IThreadGroup::eState state = State();

   switch ( state ) {
      case IThreadGroup::Joining  : // FALL THROUGH
      case IThreadGroup::Draining : // FALL THROUGH
      case IThreadGroup::Running  : {
         if ( m_workqueue.size() > 0 ) {
            pWork = m_workqueue.front();
            m_workqueue.pop();
         }
      } break;

      case IThreadGroup::Stopped : {
         ; // return without setting pWork to a work item.
      } break;

      default : {
         // Invalid state.
         ASSERT(false);
         ;
      } break;
   }

   Unlock();

   return state;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: OSLThreadGroup
// Description: Constructor
// Interface: public
// Inputs: uiMinThreads - Minimum number of threads to use (default = 0 = auto).
//         uiMaxThreads - Maximum threads. (default = 0 = auto)
//         nPriority - Thread priority
// Outputs: none.
// Comments: Setting min == max != 0 results in a static thread pool.
//           The algorithm summary:
//               - The work queue and its semaphore are initialized to zero
//                 for all threads to start.
//               - The number of worker threads is determined and
//                 the threads are created.
//               - A count-up semaphore is use to wait for workers to start
//=============================================================================
OSLThreadGroup::OSLThreadGroup(AAL::btUnsignedInt        uiMinThreads,
                               AAL::btUnsignedInt        uiMaxThreads,
                               OSLThread::ThreadPriority nPriority,
                               AAL::btTime               JoinTimeout) :
   m_JoinTimeout(JoinTimeout),
   m_pState(NULL)
{
   // If Min Threads is zero then determine a good number based on
   //  configuration
   if ( 0 == uiMinThreads ) {
      // TODO Use GetNumProcessors(), eventually.
       // m_nNumThreads = GetNumProcessors();
       uiMinThreads = 1;
   }

//TODO implement MaxThreads and dynamic sizing
   if ( uiMaxThreads < uiMinThreads ) {
      uiMaxThreads = uiMinThreads;
   }

   // Create the State object. The state object is a standalone object
   //  whose life is somewhat independent of the ThreadGroup.  This is to
   //  allow for the case that ThreadGroup is destroyed before the worker threads
   //  have been deleted. By making the state and synchronization members outside
   //  the ThreadGroup, the Threads can safely access them even if the Group object
   //  is gone.
   m_pState = new(std::nothrow) OSLThreadGroup::ThrGrpState(uiMinThreads);
   if ( NULL == m_pState ) {
      ASSERT(false);
      return;
   }

   // Create the workers.

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < uiMinThreads ; ++i ) {
      CreateWorkerThread(OSLThreadGroup::ExecProc, nPriority, m_pState);
   }

   // Wait for all of the works to signal started.
   // TODO Add timeout.
   WaitForAllWorkersToStart(AAL_INFINITE_WAIT);
}

//=============================================================================
// Name: ~OSLThreadGroup
// Description: Destructor - Stop all threads.
// Interface: public
// Comments: The state of the dispatch queue is not deterministic.
//           Normally one would stop each thread AND wait for them to end with
//           a join, however ThreadGroup can be killed from within a Thread in
//           in the Group itself. So best that can be done for now is to have
//           each thread delete itself.
//=============================================================================
OSLThreadGroup::~OSLThreadGroup()
{
   if ( !Destroy(m_JoinTimeout) ) {
      ;
   }
}

//=============================================================================
// Name: ExecProc
// Description: Worker Thread entry point
// Interface: private
// Comments:
//=============================================================================
void OSLThreadGroup::ExecProc(OSLThread *pThread, void *lpParms)
{
   OSLThreadGroup::ThrGrpState *pState = reinterpret_cast<OSLThreadGroup::ThrGrpState *>(lpParms);

   ASSERT(NULL != pState);
   if ( NULL == pState ) {
      ASSERT(false);
      return;
   }

   // Notify the constructor that we are up.
   pState->WorkerHasStarted(pThread);

   IDispatchable       *pWork;
   IThreadGroup::eState state;
   AAL::btBool          bRunning = true;

   while ( bRunning ) {

      pWork = NULL;
      state = pState->GetWorkItem(pWork);

      switch ( state ) {

         case IThreadGroup::Joining : {
            if ( NULL == pWork ) {
               // Queue has emptied - we are done.
               bRunning = false;
            } else {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         case IThreadGroup::Draining : // FALL THROUGH
         case IThreadGroup::Running  : {
            if ( NULL != pWork ) {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         case IThreadGroup::Stopped : {
            // don't dispatch any items
            if ( NULL != pWork ) {
               ASSERT(false);
               delete pWork;
            }
         } break;

         default : // keep looping
            break;
      }

   }

   pState->WorkerHasExited(pThread);
}

