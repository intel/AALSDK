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
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Cleaned up windows includes
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 03/06/2014     JG       Complete rewrite
/// 05/07/2015     TSW      Complete rewrite@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/ThreadGroup.h"
#include "aalsdk/osal/Sleep.h"

BEGIN_NAMESPACE(AAL)

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
OSLThreadGroup::OSLThreadGroup(btUnsignedInt             uiMinThreads,
                               btUnsignedInt             uiMaxThreads,
                               OSLThread::ThreadPriority nPriority,
                               btTime                    JoinTimeout) :
   m_bDestroyed(false),
   m_JoinTimeout(JoinTimeout),
   m_pState(NULL)
{
   {
      AutoLock(this);

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
         m_bDestroyed = true;
         ASSERT(false);
         return;
      }

      ASSERT(m_pState->IsOK());
      if ( !m_pState->IsOK() ) {
         m_bDestroyed = true;
         delete m_pState;
         m_pState = NULL;
         return;
      }

      // Create the workers.

      btUnsignedInt i;
      for ( i = 0 ; i < uiMinThreads ; ++i ) {
         // TODO: if worker thread creation fails.
         CreateWorkerThread(OSLThreadGroup::ExecProc, nPriority, m_pState);
      }
   }

   // Wait for all of the works to signal started.
   // TODO Add timeout.
   WaitForAllWorkersToStart(AAL_INFINITE_WAIT);
}

btBool OSLThreadGroup::Destroy(btTime Timeout)
{
   {
      AutoLock(this);

      if ( m_bDestroyed ) {
         return true;
      }

      m_bDestroyed = true;
   }

   return m_pState->Destroy(Timeout);
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

   OSLThreadGroup::ThrGrpState::eState state;

   IDispatchable *pWork;
   btBool         bRunning = true;

   while ( bRunning ) {

      pWork = NULL;
      state = pState->GetWorkItem(pWork);

      switch ( state ) {

         case OSLThreadGroup::ThrGrpState::Joining : {
            if ( NULL == pWork ) {
               // Queue has emptied - we are done.
               bRunning = false;
            } else {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         case OSLThreadGroup::ThrGrpState::Draining : // FALL THROUGH
         case OSLThreadGroup::ThrGrpState::Running  : {
            if ( NULL != pWork ) {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         case OSLThreadGroup::ThrGrpState::Stopped : {
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

////////////////////////////////////////////////////////////////////////////////
// OSLThreadGroup::ThrGroupState

OSLThreadGroup::ThrGrpState::ThrGrpState(btUnsignedInt NumThreads) :
   m_eState(Running),
   m_Flags(THRGRPSTATE_FLAG_OK),
   m_WorkSemTimeout(AAL_INFINITE_WAIT),
   m_Joiner(0),
   m_ThrStartBarrier(),
   m_ThrJoinBarrier(),
   m_ThrExitBarrier(),
   m_WorkSem(),
   m_workqueue(),
   m_RunningThreads(),
   m_ExitedThreads(),
   m_DrainManager(this)
{
   if ( !m_ThrStartBarrier.Create(NumThreads) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }

   if ( !m_ThrJoinBarrier.Create(1) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }

   if ( !m_ThrExitBarrier.Create(NumThreads) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }

   if ( !m_WorkSem.Create(0, INT_MAX) ) {
      flag_clrf(m_Flags, THRGRPSTATE_FLAG_OK);
   }
}

OSLThreadGroup::ThrGrpState::~ThrGrpState()
{
   ASSERT(Joining == m_eState);
   ASSERT(m_workqueue.empty());
   ASSERT(m_RunningThreads.empty());
   ASSERT(m_ExitedThreads.empty());
   DestructMembers();
}

//=============================================================================
// Name: GetNumThreads
// Description: Get Number of Threads
// Interface: public
// Comments:
//=============================================================================
btUnsignedInt OSLThreadGroup::ThrGrpState::GetNumThreads() const
{
   AutoLock(this);
   return (btUnsignedInt) m_RunningThreads.size();
}

//=============================================================================
// Name: GetNumWorkItems
// Description: Get Number of workitems
// Interface: public
// Comments:
//=============================================================================
btUnsignedInt OSLThreadGroup::ThrGrpState::GetNumWorkItems() const
{
   AutoLock(this);
   return (btUnsignedInt) m_workqueue.size();
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
btBool OSLThreadGroup::ThrGrpState::Add(IDispatchable *pDisp)
{
   ASSERT(NULL != pDisp);
   if ( NULL == pDisp ) {
      return false;
   }

   {
      AutoLock(this);

      const eState state = State();

      // We allow new work items when Running or Joining.
      if ( ( Stopped  == state ) ||
           ( Draining == state ) ) {
         return false;
      }

      m_workqueue.push(pDisp);
   }

   // Signal the semaphore outside the critical section so that waking threads have an
   // opportunity to immediately acquire it.
   m_WorkSem.Post(1);

   return true;
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

   if ( Stopped != State(Stopped) ) {
      // State conflict, can't stop right now.
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
btBool OSLThreadGroup::ThrGrpState::Start()
{
   AutoLock(this);

   if ( Running == State(Running) ) {
      btInt s = (btInt) m_workqueue.size();

      btInt c = 0;
      btInt m = 0;
      m_WorkSem.CurrCounts(c, m);

      if ( c >= s ) {
         // Nothing to do.
         return true;
      }

      return m_WorkSem.Post(s - c);
   }

   return false;
}

// Do a state transition.
OSLThreadGroup::ThrGrpState::eState OSLThreadGroup::ThrGrpState::State(eState st)
{
   if ( Joining == m_eState ) {
      // Joining is a final state. Deny all requests to do otherwise.
      return m_eState;
   }

   switch ( m_eState ) {
      case Running : {
         switch ( st ) {
            /* case Running :  break; */ // Running -> Running (nop)
            case Stopped  : m_eState = st; break; // Running -> Stopped  [ Stop()  ]
            case Draining : m_eState = st; break; // Running -> Draining [ Drain() ]
            case Joining  : m_eState = st; break; // Running -> Joining  [ Join()  ]
         }
      } break;

      case Stopped : {
         switch ( st ) {
            case Running  : m_eState = st; break; // Stopped -> Running   [ Start() ]
            /* case Stopped : break; */ // Stopped -> Stopped (nop)
            case Draining : ASSERT(Draining != st); break; // Invalid (queue empty check)
            case Joining  : m_eState = st; break; // Stopped -> Joining   [ Join(), ~OSLThreadGroup() ]
         }
      } break;

      case Draining : {
        switch ( st ) {
            case Running  : m_eState = st; break; // Draining -> Running  [ Drain() ]
            case Stopped  : ASSERT(Stopped != st);  break; // Invalid
            /* case Draining : break; */ // Draining -> Draining (nop)
            case Joining  : m_eState = st; break; // Draining -> Joining  [ Join()  ]
         }
      } break;
   }

   return m_eState;
}

btBool OSLThreadGroup::ThrGrpState::CreateWorkerThread(ThreadProc                fn,
                                                       OSLThread::ThreadPriority pri,
                                                       void                     *context)
{
   OSLThread *pThread = new(std::nothrow) OSLThread(fn, pri, context);

   ASSERT(NULL != pThread);
   if ( NULL == pThread ) {
      return false;
   }

   ASSERT(pThread->IsOK());
   if ( !pThread->IsOK() ) {
      delete pThread;
      return false;
   }

   {
      AutoLock(this);
      m_RunningThreads.push_back(pThread);
   }

   return true;
}

//=============================================================================
// Name: GetWorkItem
// Description: Get next work item and the current ThreadGroup state
// Interface: public
// Comments:
//=============================================================================
OSLThreadGroup::ThrGrpState::eState OSLThreadGroup::ThrGrpState::GetWorkItem(IDispatchable * &pWork)
{
   // Wait for work item
   m_WorkSem.Wait(m_WorkSemTimeout);

   // Lock until flag and queue have been processed

   eState state;

   {
      AutoLock(this);

      state = State();

      switch ( state ) {
         case Joining  : // FALL THROUGH
         case Draining : // FALL THROUGH
         case Running  : {
            if ( m_workqueue.size() > 0 ) {
               pWork = m_workqueue.front();
               m_workqueue.pop();
            }
         } break;

         case Stopped : {
            ; // return without setting pWork to a work item.
         } break;

         default : {
            // Invalid state.
            ASSERT(false);
            ;
         } break;
      }
   }

   return state;
}

OSLThread * OSLThreadGroup::ThrGrpState::ThreadRunningInThisGroup(btTID tid) const
{
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
   m_ThrStartBarrier.Post(1);
}

btBool OSLThreadGroup::ThrGrpState::WaitForAllWorkersToStart(btTime Timeout)
{
   return m_ThrStartBarrier.Wait(Timeout);
}

void OSLThreadGroup::ThrGrpState::WorkerIsSelfTerminating(OSLThread *pThread)
{
   // Remove the worker from the Running list, without placing it in the Exited list, because
   //  we don't want to attempt to Join() this worker.
   thr_list_iter iter = std::find(m_RunningThreads.begin(), m_RunningThreads.end(), pThread);

   if ( m_RunningThreads.end() != iter ) {
      m_RunningThreads.erase(iter);
   }

   // When self-terminating, if pThread is also a self-Drain()'er, we would not
   //  otherwise be able to Post() the drain Barrier object. Do it here, instead.
   while ( m_DrainManager.End(pThread->tid(), NULL) ) {
      m_DrainManager.ForciblyCompleteWorkItem();
   }

   // The worker has now "exited".
   m_ThrExitBarrier.Post(1);
}

void OSLThreadGroup::ThrGrpState::WorkerHasExited(OSLThread *pThread)
{
   {
      AutoLock(this);

      // Move the worker from Running to Exited.
      thr_list_iter iter = std::find(m_RunningThreads.begin(), m_RunningThreads.end(), pThread);

      if ( m_RunningThreads.end() != iter ) {
         m_RunningThreads.erase(iter);
         m_ExitedThreads.push_back(pThread);
      }
   }

   m_ThrExitBarrier.Post(1);
}

btBool OSLThreadGroup::ThrGrpState::Quiesce(btTime Timeout)
{
   btBool res;

   // Wait for all thread group workers to Post() m_ThrExitBarrier, signaling that they
   //  are exiting.
   res = m_ThrExitBarrier.Wait(Timeout);
   ASSERT(res);
   if ( !res ) {
      return false;
   }

   // If we are the designated Join()'er, then Join() all workers. If not, then wait for
   //  the Join()'er to do its thing.

   const btTID MyThrID     = GetThreadID();
   btBool      ImTheJoiner = false;

   {
      AutoLock(this);
      // We need the check on THRGRPSTATE_FLAG_JOINING, because we don't know if 0 is a valid tid.
      if ( flag_is_set(m_Flags, THRGRPSTATE_FLAG_JOINING) && ( MyThrID == m_Joiner ) ) {
         ImTheJoiner = true;
      }
   }

   if ( ImTheJoiner ) {
      // ASSERT: all workers have exited (we waited on m_ThrExitBarrier above).
      // ASSERT: we are the sole Join()'er

      thr_list_iter iter;
      for ( iter = m_ExitedThreads.begin() ; m_ExitedThreads.end() != iter ; ++iter ) {
         (*iter)->Join();
         delete *iter;
      }
      m_ExitedThreads.clear();

      // Are any external Drain()'ers blocked on our work item? When a self-referential Join() or
      // a self-referential Destroy() is allowed to progress when there is an external Drain()'er(s),
      // the thread group worker must signal the completion of the Drain() here, before
      // self-terminating. Otherwise, the external Drain()'ers will become deadlocked.
      m_DrainManager.ReleaseAllDrainers();

      m_DrainManager.WaitForAllDrainersDone();

      res = m_ThrJoinBarrier.Post(1);
      ASSERT(res);
      if ( !res ) {
         return false;
      }

   } else {

      // We're not the Join()'er.

      res = m_ThrJoinBarrier.Wait(Timeout);
      ASSERT(res);
      if ( !res ) {
         return false;
      }

   }

   return true;
}

void OSLThreadGroup::ThrGrpState::DestructMembers()
{
   m_ThrStartBarrier.Destroy();
   m_ThrExitBarrier.Destroy();
   m_DrainManager.DestructMembers();
   m_ThrJoinBarrier.Destroy();
}

OSLThreadGroup::ThrGrpState::DrainManager::DrainManager(ThrGrpState *pTGS) :
   m_pTGS(pTGS),
   m_DrainNestLevel(0),
   m_WaitTimeout(AAL_INFINITE_WAIT)
{
   m_DrainerDoneBarrier.Create(1);
}

OSLThreadGroup::ThrGrpState::DrainManager::~DrainManager()
{
   ASSERT(0 == m_DrainNestLevel);
   ASSERT(0 == m_SelfDrainers.size());
   ASSERT(0 == m_NestedWorkItems.size());
}

Barrier * OSLThreadGroup::ThrGrpState::DrainManager::Begin(btTID tid, btUnsignedInt items)
{
   ++m_DrainNestLevel;

   if ( 1 == m_DrainNestLevel ) {
      // Beginning a new series of (possibly nested) Drain() calls.
      ASSERT((btUnsignedInt) m_pTGS->m_workqueue.size() == items);
      ASSERT(0 == m_NestedWorkItems.size());

      m_DrainerDoneBarrier.Reset();

      m_DrainBarrier.Destroy();
      m_DrainBarrier.Create(items);

      work_queue_t        tmpq;
      IDispatchable      *pWork;
      NestedBarrierPostD *pNested;

      // Pull each item from the work queue, and wrap it in a NestedBarrierPostD() object.
      while ( m_pTGS->m_workqueue.size() > 0 ) {
         pWork = m_pTGS->m_workqueue.front();
         m_pTGS->m_workqueue.pop();

         pNested = new(std::nothrow) NestedBarrierPostD(pWork, this);
         m_NestedWorkItems.push_back(pNested);

         tmpq.push( pNested );
      }

      // Re-populate the work queue.
      while ( tmpq.size() > 0 ) {
         m_pTGS->m_workqueue.push(tmpq.front());
         tmpq.pop();
      }
   }

   // non-NULL means self-referential Drain().
   OSLThread *pThread = m_pTGS->ThreadRunningInThisGroup(tid);

   if ( NULL != pThread ) {
      // Add tid to the list of self-drainers, allowing duplicates.
      m_SelfDrainers.push_back(tid);
      // Self-drainers don't block on the Barrier.
      return NULL;
   }

   return &m_DrainBarrier;
}

void OSLThreadGroup::ThrGrpState::DrainManager::CompleteNestedWorkItem(OSLThreadGroup::ThrGrpState::DrainManager::NestedBarrierPostD *pItem)
{
   {
      AutoLock(m_pTGS);

      nested_list_iter iter = std::find(m_NestedWorkItems.begin(), m_NestedWorkItems.end(), pItem);

      ASSERT(m_NestedWorkItems.end() != iter);
      if ( m_NestedWorkItems.end() != iter ) {
         m_NestedWorkItems.erase(iter);
      }
   }

   delete pItem;
   m_DrainBarrier.Post(1);
}

btBool OSLThreadGroup::ThrGrpState::DrainManager::End(btTID    tid,
                                                      Barrier *pDrainBarrier)
{
   btBool res = false;

   if ( NULL == pDrainBarrier ) {
      // self-referential Drain(). Remove one instance of tid from m_SelfDrainers.

      drainer_list_iter iter;
      for ( iter = m_SelfDrainers.begin() ; m_SelfDrainers.end() != iter ; ++iter ) {
         if ( ThreadIDEqual(tid, *iter) ) {
            m_SelfDrainers.erase(iter);
            if ( m_DrainNestLevel > 0 ) {
               --m_DrainNestLevel;
            }
            res = true;
            break;
         }
      }

   } else {
      // external Drain().
      res = true;
      if ( m_DrainNestLevel > 0 ) {
         --m_DrainNestLevel;
      }
   }

   return res;
}

btBool OSLThreadGroup::ThrGrpState::DrainManager::ReleaseAllDrainers()
{
   nested_list_iter iter;
   for ( iter = m_NestedWorkItems.begin() ; iter != m_NestedWorkItems.end() ; ++iter ) {
      delete *iter;
   }
   m_NestedWorkItems.clear();
   m_WaitTimeout = 100;
   return m_DrainBarrier.UnblockAll();
}

void OSLThreadGroup::ThrGrpState::DrainManager::ForciblyCompleteWorkItem()
{
   m_DrainBarrier.Post(1);
}

void OSLThreadGroup::ThrGrpState::DrainManager::AllDrainersAreDone()
{
   m_DrainerDoneBarrier.Post(1);
}

void OSLThreadGroup::ThrGrpState::DrainManager::WaitForAllDrainersDone()
{
   // Don't wait while locked..

   class BarrierWaitDisp : public IDispatchable
   {
   public:
      BarrierWaitDisp(Barrier *b, btTime Timeout) :
         m_Barrier(b),
         m_Timeout(Timeout)
      {}
      virtual void operator() ()
      {
         m_Barrier->Wait(m_Timeout);
      }
   protected:
      Barrier *m_Barrier;
      btTime   m_Timeout;
   };

   AutoLock(m_pTGS);

   BarrierWaitDisp disp(&m_DrainerDoneBarrier, m_WaitTimeout);

   while ( m_DrainNestLevel > 0 ) {
      __LockObj.UnlockedDispatch(&disp);
   }
}

void OSLThreadGroup::ThrGrpState::DrainManager::DestructMembers()
{
   m_DrainBarrier.Destroy();
   m_DrainerDoneBarrier.Destroy();
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
btBool OSLThreadGroup::ThrGrpState::Drain()
{
   // Refer to the state checking mutator:
   //   eState OSLThreadGroup::ThrGrpState::State(eState st);
   //
   // The only valid transitions to state Draining are
   //   Running  -> Draining
   //   Draining -> Draining (nested Drain() calls)
   //
   // Allowing nested Drain()'s presents a problem - as the inner Drain()'s complete, they
   // must not set the state of the OSLThreadGroup to Running (this would allow Add()'s during
   // the outer Drain()'s, eg). The inner Drain()'s must leave the state set to Draining, and
   // only the last Drain() to complete can set the state back to Running.

   btTID    MyThrID       = GetThreadID();
   Barrier *pDrainBarrier = NULL;
   btBool   res;

   {
      AutoLock(this);

      const btUnsignedInt items = (btUnsignedInt) m_workqueue.size();

      // No need to drain if already empty.
      if ( 0 == items ) {
         return true;
      }

      // Check for other state conflicts.
      if ( Draining != State(Draining) ) {
         // Can't drain now - state conflict.
         return false;
      }

      pDrainBarrier = m_DrainManager.Begin(MyThrID, items);

      if ( NULL == pDrainBarrier ) {
         // Self-referential Drain().

         // We need to continue to execute work.
         IDispatchable *pWork;
         while ( m_workqueue.size() > 0 ) {
            pWork = m_workqueue.front();
            m_workqueue.pop();
            __LockObj.UnlockedDispatch(pWork);
         }
      }

      // (AutoLock opens)
   }

   if ( NULL != pDrainBarrier ) {
      // Wait for the work items to complete. Don't wait while locked.
      pDrainBarrier->Wait();
   }

   {
      AutoLock(this);

      eState st = State();

      // DrainManager::End() will unlock the critical section.
      m_DrainManager.End(MyThrID, pDrainBarrier);
      btUnsignedInt level = m_DrainManager.DrainNestLevel();

      if ( 0 == level ) {
         m_DrainManager.AllDrainersAreDone();
      }

      if ( Joining == st ) {
         // Join() during Drain() is allowed. In this case, an attempt to set the
         // state to Running or Draining below would be denied. We want the success indicator here,
         // however, to reflect that the thread group was drained of items, hence this check.
         return true;
      }

      st = (0 == level) ? Running : Draining;

      res = (State(st) == st);
   }

   return res;
}

btBool OSLThreadGroup::ThrGrpState::Join(btTime Timeout)
{
   btBool     res;
   OSLThread *pThread = NULL;

   {
      AutoLock(this);

      const eState st = State();

      if ( Joining == st ) {
         // Prevent nested / multiple Join().
         return false;
      }

      const btTID MyThrID = GetThreadID();
      pThread = ThreadRunningInThisGroup(MyThrID);

      // We are now Joining.
      State(Joining);

      // Workers are no longer required to block infinitely on work items. Give them a
      // chance to wake periodically and see that the thread group is being joined.
      m_WorkSemTimeout = PollingInterval();

      // Wake any threads that happen to be blocked infinitely.
      m_WorkSem.Post( (btInt) m_RunningThreads.size() );

      // Claim the Join().
      ASSERT(flag_is_clr(m_Flags, THRGRPSTATE_FLAG_JOINING));
      m_Joiner = MyThrID;
      flag_setf(m_Flags, THRGRPSTATE_FLAG_JOINING);

      if ( NULL != pThread ) {
         // self-referential Join()
         flag_setf(m_Flags, THRGRPSTATE_FLAG_SELF_JOIN);

         // We need to continue to execute work.
         IDispatchable *pWork;
         while ( m_workqueue.size() > 0 ) {
            pWork = m_workqueue.front();
            m_workqueue.pop();
            __LockObj.UnlockedDispatch(pWork);
         }

         WorkerIsSelfTerminating(pThread);
      }
      // (AutoLock opens)
   }

   res = Quiesce(Timeout);
   if ( !res ) {
      return false;
   }

   if ( NULL != pThread ) {
      // self-destruct.
      delete pThread;

      // terminate this thread
      ExitCurrentThread(0);

      // We must not return from this call.
      ASSERT(false);
      return false;
   }

   return true;
}

btBool OSLThreadGroup::ThrGrpState::Destroy(btTime Timeout)
{
   btBool     res;
   OSLThread *pThread = NULL;

   {
      AutoLock(this);

      const btTID MyThrID = GetThreadID();
      pThread = ThreadRunningInThisGroup(MyThrID);

      // Workers are no longer required to block infinitely on work items. Give them a chance
      // to wake periodically and see that the thread group is being joined.
      m_WorkSemTimeout = PollingInterval();

      const eState st = State();

      State(Joining);

      // Wake any threads that happen to be blocked infinitely.
      m_WorkSem.Post( (btInt) m_RunningThreads.size() );

      if ( Joining != st ) {
         // We weren't being joined before this call. Claim the join now.
         ASSERT(flag_is_clr(m_Flags, THRGRPSTATE_FLAG_JOINING));
         m_Joiner = MyThrID;
         flag_setf(m_Flags, THRGRPSTATE_FLAG_JOINING);
      }

      if ( NULL != pThread ) {
         // Self-referential Destroy().

         flag_setf(m_Flags, THRGRPSTATE_FLAG_SELF_JOIN);

         IDispatchable *pWork;
         while ( m_workqueue.size() > 0 ) {
            pWork = m_workqueue.front();
            m_workqueue.pop();
            __LockObj.UnlockedDispatch(pWork);
         }

         WorkerIsSelfTerminating(pThread);
      }

      // (AutoLock opens)
   }

   res = Quiesce(Timeout);
   if ( !res ) {
      return false;
   }

   if ( NULL != pThread ) {
      // Self-referential Destroy().
      delete this;

      // self-destruct
      delete pThread;

      // terminate this thread
      ExitCurrentThread(0);

      // We must not return from this fn.
      ASSERT(false);
      return false;
   }

   // Not a self-referential Destroy(), not Drain()'ing, first time Join()'ing.

   delete this;
   return true;
}

btBool FireAndWait(IDispatchable            *pDisp,
                   btUnsignedInt             MinThrs,
                   btUnsignedInt             MaxThrs,
                   OSLThread::ThreadPriority ThrPriority,
                   btTime                    JoinTimeout)
{
   // ~OSLThreadGroup() waits for all items to dispatch, prior to returning.
   OSLThreadGroup ThrGrp(MinThrs, MaxThrs, ThrPriority, JoinTimeout);

   const btBool fire_and_wait_thread_group_ok = ThrGrp.IsOK();

   ASSERT(fire_and_wait_thread_group_ok);
   if ( !fire_and_wait_thread_group_ok ) {
      return false;
   }

   const btBool fire_and_wait_thread_group_added_disp = ThrGrp.Add(pDisp);

   ASSERT(fire_and_wait_thread_group_added_disp);
   if ( !fire_and_wait_thread_group_added_disp ) {
      return false;
   }

   const btBool fire_and_wait_thread_group_drained = ThrGrp.Drain();

   ASSERT(fire_and_wait_thread_group_drained);
   return fire_and_wait_thread_group_drained;
}

namespace FAF {

   struct Parms
   {
      Parms(OSLThreadGroup           *pThrGrp,
            OSLThread::ThreadPriority ThrPriority,
            btTime                    JoinTimeout) :
         m_pThrGrp(pThrGrp),
         m_ThrPriority(ThrPriority),
         m_JoinTimeout(JoinTimeout)
      {}
      OSLThreadGroup           *m_pThrGrp;
      OSLThread::ThreadPriority m_ThrPriority;
      btTime                    m_JoinTimeout;
   };

   static void FireAndForgetDelete(OSLThread *pThread, void *lpParms)
   {
      Parms *p = reinterpret_cast<Parms *>(lpParms);
      ASSERT(NULL != p);

      p->m_pThrGrp->Destroy(p->m_JoinTimeout); // deletes OSLThreadGroup::m_pState
      delete p->m_pThrGrp;                     // deletes OSLThreadGroup
      delete p;                                // deletes FAF::Parms
      delete pThread;                          // deletes pThread. d'tor runs to completion.

      ExitCurrentThread(0);
   }

} // FAF

btBool FireAndForget(IDispatchable            *pDisp,
                     btUnsignedInt             MinThrs,
                     btUnsignedInt             MaxThrs,
                     OSLThread::ThreadPriority ThrPriority,
                     btTime                    JoinTimeout)
{
   OSLThreadGroup *pThrGrp = NULL;
   FAF::Parms     *pParms  = NULL;
   OSLThread      *pThr    = NULL;
   btBool          fire_and_forget_add_ok;

   pThrGrp = new(std::nothrow) OSLThreadGroup(MinThrs, MaxThrs, ThrPriority, JoinTimeout);

   ASSERT(NULL != pThrGrp);
   if ( NULL == pThrGrp ) {
      return false;
   }

   pParms = new(std::nothrow) FAF::Parms(pThrGrp, ThrPriority, JoinTimeout);

   ASSERT(NULL != pParms);
   if ( NULL == pParms ) {
      goto _CLEANUP;
   }

   fire_and_forget_add_ok = pThrGrp->Add(pDisp);
   ASSERT(fire_and_forget_add_ok);

   pThr = new(std::nothrow) OSLThread(FAF::FireAndForgetDelete, ThrPriority, pParms);

   // The thread cleans up after itself.

   ASSERT(NULL != pThr);
   return NULL != pThr;

_CLEANUP:
   if ( NULL != pParms ) {
      delete pParms;
   }
   if ( NULL != pThrGrp ) {
      delete pThrGrp;
   }
   return false;
}

END_NAMESPACE(AAL)
