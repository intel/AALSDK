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
                               OSLThread::ThreadPriority nPriority) :
   m_nNumThreads(0),
   m_nMinThreads(uiMinThreads),
   m_nMaxThreads(uiMaxThreads),
   m_ThreadPriority(nPriority),
   m_pState(NULL)
{
	// If Min Threads is zero then determine a good number based on
	//  configuration
	if ( 0 == uiMinThreads ) {
	   // TODO Use GetNumProcessors(), eventually.
       // m_nNumThreads = GetNumProcessors();
       m_nNumThreads = 1;
	} else {
       m_nNumThreads = uiMinThreads;
	}

//TODO implement MaxThreads and dynamic sizing
   if ( m_nMaxThreads < m_nNumThreads ) {
	   m_nMaxThreads = m_nNumThreads;
   }

   // Create the State object. The state object is a standalone object
   //  whose life is somewhat independent of the ThreadGroup.  This is to
   //  allow for the case that ThreadGroup is destroyed before the worker threads
   //  have been deleted. By making the state and synchronization members outside
   //  the ThreadGroup, the Threads can safely access them even if the Group object
   //  is gone.
   m_pState = new(std::nothrow) OSLThreadGroup::ThrGrpState(m_nNumThreads);
   if ( NULL == m_pState ) {
      return;
   }

   // Create the workers TODO No error checking. Perhaps Is OK.
   m_pState->m_CritSect.Lock();

   AAL::btInt n;
   for ( n = 0 ; n < m_nNumThreads ; ++n ) {
      OSLThread *pthread = new(std::nothrow) OSLThread(OSLThreadGroup::ExecProc, nPriority, m_pState);
      ASSERT(NULL != pthread);
      m_pState->m_Threads.push_back(pthread);
   }

   m_pState->m_CritSect.Unlock();

   m_pState->m_StartupSem.Wait();     // TODO Change to use timeout
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
   // Dispatch all outstanding work items
   Drain();
   m_pState->m_CritSect.Lock();

   AAL::btBool DontWaitforThreads = IsCurThreadInGroup();
   // If we are in a thread in this group we can't
   //  wait for threads to die. Let state kill itself.
   if(DontWaitforThreads){
      m_pState->DontJoinThreads();
   }

   // Set the state to destruct and then reset the
   //  count up Semaphore we Join on.
   m_pState->m_eState = Destruct;

   AAL::btBool ret = m_pState->m_JoinSem.Reset( -(m_nNumThreads));
   if( false == ret){
      // If the reset fails then we can't wait for
      //  threads in join.  This should never happen and
      //  should be logged as a fault.
      DontWaitforThreads = true;
   }
   m_pState->m_CritSect.Unlock();

   // Wake up all threads which should now die off.
   //  Wait for all of them to die if we can.
   m_pState->m_WorkSem.Post(m_nNumThreads);

   // If we are trying to destroy from a thread
   //  in this group we can't wait for ALL threads to delete
   if(DontWaitforThreads == false){
      m_pState->JoinAllThreads(100);
   }
}

#include <stdio.h>
#include <iostream>
//=============================================================================
// Name: IsCurThreadInGroup
// Description: Tells whether the calling thread is a member of the
//              ThreadGroup.
// Interface: public
// Comments:
//=============================================================================
AAL::btBool OSLThreadGroup::IsCurThreadInGroup()
{
   m_pState->m_CritSect.Lock();
   AAL::btBool ret = false;
   // Get my current Thread ID and determine
   //  if we are in one of the worker threads
   AAL::btTID curTid = CurrentThreadID();

   // If we are in one of our own worker threads
   //  we cannot wait until all threads die (we are one).
   for (int n=0; n < m_nNumThreads; n++){
     OSLThread *pThread = m_pState->m_Threads[n];
     if(NULL== pThread){
        std::cerr << "Thread already dead " << n << std::endl;
     }
     if(pThread->tid() == curTid){
        ret = true;
        break;
     }
   }
   m_pState->m_CritSect.Unlock();
   return ret;
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
AAL::btBool OSLThreadGroup::Add(IDispatchable *pwi)
{
   m_pState->m_CritSect.Lock();

   // Prevent adding new work during these states.
   if ( (Stopped  == m_pState->m_eState) ||
        (Draining == m_pState->m_eState) ) {
      m_pState->m_CritSect.Unlock();
      return false;
   }

   ASSERT(NULL != pwi);
   if ( NULL == pwi ) {
      m_pState->m_CritSect.Unlock();
      return false;
   }

   m_pState->m_workqueue.push(pwi);

   m_pState->m_CritSect.Unlock();

   // Signal the semaphore outside the critical section so that waking threads have an
   // opportunity to immediately acquire it.
   m_pState->m_WorkSem.Post(1);

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
void OSLThreadGroup::Stop()
{
   m_pState->m_CritSect.Lock();

   m_pState->m_eState = Stopped;

   m_pState->m_WorkSem.Reset(0);

   // If there is something on the queue then remove it and destroy it
   while ( 0 != m_pState->m_workqueue.size() ) {
      IDispatchable *wi = m_pState->m_workqueue.front();
      m_pState->m_workqueue.pop();
      delete wi;
   }

   m_pState->m_CritSect.Unlock();
}

//=============================================================================
// Name: Drain
// Description: Drain all of the outstanding requests, returning as soon as the
//              work queue is empty.
// Interface: public
// Comments:
//=============================================================================

// Functor that signals completion of a call to OSLThreadGroup::Drain().
class DrainComplete : public IDispatchable
{
public:
   DrainComplete(CSemaphore &sem) :
      m_Sem(sem)
   {}

   void operator() ()
   {
      m_Sem.Post(1);
      delete this;
   }

protected:
   CSemaphore &m_Sem;
};


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

// Functor that signals completion of a call to OSLThreadGroup::Drain().
class DrainingFunctor : public IDispatchable
{
public:
   DrainingFunctor(IDispatchable *pContained, CSemaphore &Sem) :
      m_pContained(pContained),
      m_Sem(Sem)
   {}

   void operator() ()
   {
      (*m_pContained) (); // Execute the contained work item.
      m_Sem.Post(1);      // signal its completion.
      delete this;
   }

protected:
   IDispatchable *m_pContained;
   CSemaphore    &m_Sem;
};

AAL::btBool OSLThreadGroup::Drain()
{
   m_pState->m_CritSect.Lock();

   // No need to join if already empty.
   if ( 0 == m_pState->m_workqueue.size() ) {
      m_pState->m_CritSect.Unlock();
      return true;
   }

   // Using count up sem.
   if( false == m_pState->m_JoinSem.Reset( - ( (AAL::btInt)m_pState->m_workqueue.size() ) )){
      // Someone is probably waiting()
      m_pState->m_CritSect.Unlock();
      return false;
   }

   m_pState->m_eState = Draining;

   work_queue_t tmpq;

   // Pull each item from the work queue, and wrap it in a JoiningFunctor() object.
   while ( m_pState->m_workqueue.size() > 0 ) {
      IDispatchable *pContained = m_pState->m_workqueue.front();
      m_pState->m_workqueue.pop();
      tmpq.push( new(std::nothrow) DrainingFunctor(pContained, m_pState->m_JoinSem) );
   }

   // Re-populate the work queue.
   while ( tmpq.size() > 0 ) {
      m_pState->m_workqueue.push(tmpq.front());
      tmpq.pop();
   }

   m_pState->m_CritSect.Unlock();

   // Wait for the work items to complete.
   m_pState->m_JoinSem.Wait();

   m_pState->m_CritSect.Lock();
   m_pState->m_eState = Running;
   m_pState->m_CritSect.Unlock();
   return true;
}

//=============================================================================
// Name: Start
// Description: Restart the Thread Group
// Interface: public
// Comments:
//=============================================================================
AAL::btBool OSLThreadGroup::Start()
{
   m_pState->m_CritSect.Lock();

   if ( Stopped != m_pState->m_eState ) {
      m_pState->m_CritSect.Unlock();
      return false;
   }

   m_pState->m_eState = Running;
   m_pState->m_WorkSem.Reset((AAL::btInt)m_pState->m_workqueue.size());

   m_pState->m_CritSect.Unlock();

   return true;
}

//=============================================================================
// Name: GetWorkItem
// Description: Get next work item and the current ThreadGroup state
// Interface: public
// Comments:
//=============================================================================
OSLThreadGroup::eState OSLThreadGroup::GetWorkItem(OSLThreadGroup::ThrGrpState *pState,
                                                   IDispatchable             * &pWork)
{
   pState->m_WorkSem.Wait(); // for work item

   // Lock until flag and queue have been processed
   pState->m_CritSect.Lock();

   const OSLThreadGroup::eState state = pState->m_eState;

   switch ( state ) {

      // If the thread group object has destructed, but work items remain in the queue,
      // let the remaining threads process the work items until the queue empties.
      case Destruct : // FALL THROUGH
      case Draining : // FALL THROUGH
      case Running  : {
         if ( pState->m_workqueue.size() > 0 ) {
            pWork = pState->m_workqueue.front();
            pState->m_workqueue.pop();
         }
         pState->m_CritSect.Unlock();
         return state;
      }

      default : { // Stopped
         pState->m_CritSect.Unlock();
         return state; // (without setting pWork to a work item.)
      }
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
      return;
   }

   pState->m_StartupSem.Post(1); // Notify the constructor that we are up

   IDispatchable *pWork;
   AAL::btBool    bRunning = true;

   while ( bRunning ) {

      pWork = NULL;

      switch ( OSLThreadGroup::GetWorkItem(pState, pWork) ) {

         case Destruct : {
            if ( NULL == pWork ) {
               // Queue has emptied - we are done.
               bRunning = false;
            } else {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         case Draining : // FALL THROUGH
         case Running  : {
            if ( NULL != pWork ) {
               (*pWork) (); // invoke the functor via operator() ()
            }
         } break;

         default : // keep looping
            break;
      }

   }

   if ( NULL != pThread ) {
      pState->DeleteThr(pThread);
      // Delete self
      delete pThread;
   }
}

//=============================================================================
// Name: GetNumThreads
// Description: Get Number of Threads
// Interface: public
// Comments:
//=============================================================================
AAL::btInt OSLThreadGroup::GetNumThreads() const
{
	return m_nNumThreads;
}

//=============================================================================
// Name: GetNumWorkItems
// Description: Get Number of workitems
// Interface: public
// Comments:
//=============================================================================
AAL::btInt OSLThreadGroup::GetNumWorkItems() const
{
   AutoLock(&m_pState->m_CritSect);
	return (AAL::btInt) m_pState->m_workqueue.size();
}

