// Copyright(c) 2003-2016, Intel Corporation
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
/// @file ThreadGroup.h
/// @brief Interface for the ThreadGroup class
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
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 03/06/2014     JG       Complete rewrite
/// 05/07/2015     TSW      Complete rewrite@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_THREADGROUP_H__
#define __AALSDK_OSAL_THREADGROUP_H__
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/osal/Barrier.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/osal/IDispatchable.h>

/// @addtogroup OSAL
/// @{

BEGIN_NAMESPACE(AAL)

class OSAL_API IThreadGroup
{
public:
   virtual ~IThreadGroup() {}

   /// @brief Basic status check.
   virtual btBool                   IsOK() const          = 0;

   /// @brief Retrieve a snapshot of the current number of threads, which may be less than that originally
   /// in the thread group when created. Threads remove themselves from the group as they exit.
   virtual btUnsignedInt   GetNumThreads() const          = 0;

   /// @brief Retrieve a snapshot of the number of work items in the queue, which is not guaranteed to
   /// be true even upon the return of this call (threads may have consumed more work).
   virtual btUnsignedInt GetNumWorkItems() const          = 0;

   /// @brief  Add a work item to the queue for processing.
   /// returns false, and does not queue the item, if the thread group is stopped or draining.
   virtual btBool                    Add(IDispatchable *) = 0;

   /// @brief   Wait for all worker threads to exit.
   virtual btBool                   Join(btTime )         = 0;

   /// @brief   Halt the dispatching of all queued work items and prevent new work items from being added.
   /// Any work items residing in the queue at the time of the call are removed and deleted, without
   /// being executed.
   virtual void                     Stop()                = 0;

   /// @brief   Resume a Stopped thread group.
   /// returns false if the thread group could not be resumed; otherwise true.
   virtual btBool                  Start()                = 0;

   /// @brief   Execute all of the work items currently in the work queue, waiting until they are all
   /// complete before returning. Prevent new work items from being added to the thread group
   /// while draining.
   /// return false if the queue could not be drained; otherwise true.
   virtual btBool                  Drain()                = 0;

   virtual btBool                Destroy(btTime )         = 0;

   /// Associate a User-Defined pointer with this IThreadGroup.
   ///
   /// @param[in]  User  A User-Defined data item, not touched by IThreadGroup.
   virtual void              UserDefined(btObjectType )   = 0;

   /// Retrieve the User-Defined pointer associated with this IThreadGroup object.
   ///
   /// @return The User-Defined pointer, or NULL if none was set.
   virtual btObjectType      UserDefined() const          = 0;

protected:
   virtual btBool     CreateWorkerThread(ThreadProc ,
                                         OSLThread::ThreadPriority ,
                                         void * )         = 0;
   virtual btBool WaitForAllWorkersToStart(btTime )       = 0;
};


//=============================================================================
// Name: OSLThreadGroup
// Description: ThreadGroup (a.k.a. thread pooled) functor scheduler.
// Interface: public
// Comments: 
//=============================================================================

/// @brief Thread pooled functor scheduler
class OSAL_API OSLThreadGroup : public IThreadGroup,
                                public CriticalSection
{
public:
   ///  If uiMinThreads is the default 0, the Thread Group will determine the minimum
   ///  number of threads in the group.
   ///
   ///  If uiMaxThreads < uiMinThreads then uiMaxThreads is set to uiMinThreads.
   OSLThreadGroup(btUnsignedInt             uiMinThreads=0,
                  btUnsignedInt             uiMaxThreads=0,
                  OSLThread::ThreadPriority nPriority=OSLThread::THREADPRIORITY_NORMAL,
                  btTime                    JoinTimeout=AAL_INFINITE_WAIT);

   virtual ~OSLThreadGroup();

   // <IThreadGroup>

   /// @brief  Returns whether or not the state of the Thread Group is OK.
   /// @retval true  if the Thread Group has been properly initialized.
   /// @retval false if the Thread Group was not properly initialized, or is being destroyed.
   virtual btBool                   IsOK() const               { return NULL != m_pState;            }

   /// @brief  Returns the number of threads in the Thread Group.
   /// @note   The number returned is a snapshot.  The number of threads can change between the time
   ///         GetNumThreads() gets the number and the time it is returned to the caller.
   /// @return The number of threads.
   virtual btUnsignedInt   GetNumThreads() const               { return m_pState->GetNumThreads();   }

   /// @brief  Returns the number of work items on the Thread Group work queue.
   /// @note   The number returned is a snapshot.  The number of work items can change between the time
   ///         GetNumWorkItems() gets the number and the time it is returned to the caller.
   /// @return The number of threads.
   virtual btUnsignedInt GetNumWorkItems() const               { return m_pState->GetNumWorkItems(); }

   /// @brief  Adds the work item to the Thread Group work queue.
   /// @param[in] pDisp     - Pointer to the work item to be added to the work queue.
   /// @retval true  if the work item has been added to the Thread Group work queue.
   /// @retval false if the work item has NOT been added to the Thread Group work queue.
   virtual btBool                    Add(IDispatchable *pDisp) { return m_pState->Add(pDisp);        }

   /// @brief  Wait for all threads in the Thread Group to exit.
   /// @note   All items on the Thread Group work queue are processed.
   /// @param[in] timeout   - Maximum time to wait for the threads to exit.
   /// @retval true   if all threads exited.
   /// @retval false  if not all threads exited.
   virtual btBool                   Join(btTime timeout)       { return m_pState->Join(timeout);     }

   /// @brief  Synchronously execute all work items currently in the Thread Group work queue.
   /// @note   All work items in the Thread Group work queue will be executed before Drain() returns.
   /// @retval true   if no items remain on the work queue.
   /// @retval false  if the work queue cannot be drained.
   virtual btBool                  Drain()                     { return m_pState->Drain();           }

   /// @brief  Stops all thread in the Thread Group.
   /// @note   All work items in the Thread Group work queue will be removed but not executed.
   /// @return void
   virtual void                     Stop()                     { m_pState->Stop();                   }

   /// @brief  Restarts threads in the Thread Group.
   /// @note   Adjusts the Thread Group work queue semaphore to ensure threads will execute the work items.
   /// @retval true   if threads are restarted or enough threads are already running to execute the work
   ///                items on the queue.
   /// @retval false  if the Thread Group is not in a running state.
   virtual btBool                  Start()                     { return m_pState->Start();           }

   virtual btBool                Destroy(btTime Timeout);

   /// @brief  Associate a pointer to a User-Defined object with the Thread Group.
   /// @note   The Thread Group does not access or alter the User-Defined object.
   /// @return void
   virtual void              UserDefined(btObjectType User)    { m_pState->UserDefined(User);        }

   /// @brief  Retrieve the pointer to a User-Defined object that was associated with this Thread Group
   ///         by calling UserDefined(btObjectType).
   /// @retval Pointer to an object.
   /// @retval NULL if no User-Defined object is associated with this Thread Group.
   virtual btObjectType      UserDefined() const               { return m_pState->UserDefined();     }
   // </IThreadGroup>

protected:
   virtual btBool CreateWorkerThread(ThreadProc fn, OSLThread::ThreadPriority pri, void *context)
   { return m_pState->CreateWorkerThread(fn, pri, context); }

   virtual btBool WaitForAllWorkersToStart(btTime Timeout)
   { return m_pState->WaitForAllWorkersToStart(Timeout); }

private:
   //
   // Object that holds state and semaphores for the
   //  thread group. This object "lives" outside the
   //  ThreadGroup object so that Threads may continue to
   // clean-up even if the ThreadGroup proper has been destroyed.
   class ThrGrpState : public IThreadGroup,
                       public CriticalSection
   {
#define THRGRPSTATE_FLAG_OK        0x00000001
#define THRGRPSTATE_FLAG_SELF_JOIN 0x00000002
#define THRGRPSTATE_FLAG_JOINING   0x00000004
   public:
      ThrGrpState(btUnsignedInt NumThreads);
      virtual ~ThrGrpState();

      // <IThreadGroup>
      virtual btBool                   IsOK() const { return flag_is_set(m_Flags, THRGRPSTATE_FLAG_OK); }
      virtual btUnsignedInt   GetNumThreads() const;
      virtual btUnsignedInt GetNumWorkItems() const;
      virtual btBool                    Add(IDispatchable * );
      virtual btBool                   Join(btTime );
      virtual btBool                  Drain();
      virtual void                     Stop();
      virtual btBool                  Start();
      virtual btBool                Destroy(btTime );
      virtual void              UserDefined(btObjectType );
      virtual btObjectType      UserDefined() const;
      // </IThreadGroup>

   protected:
      enum eState {
         Running = 0,
         Stopped,
         Draining,
         Joining
      };

      typedef std::queue<IDispatchable *> work_queue_t;
      typedef std::list<OSLThread      *> thr_list_t;
      typedef thr_list_t::iterator        thr_list_iter;
      typedef thr_list_t::const_iterator  const_thr_list_iter;

      eState        m_eState;
      btUnsignedInt m_Flags;
      btTime        m_WorkSemTimeout;
      btTID         m_Joiner;
      btObjectType  m_UserDefined;
      Barrier       m_ThrStartBarrier;
      Barrier       m_ThrJoinBarrier;
      Barrier       m_ThrExitBarrier;
      CSemaphore    m_WorkSem;

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      work_queue_t  m_workqueue;
      thr_list_t    m_RunningThreads;
      thr_list_t    m_ExitedThreads;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

      class DrainManager
      {
      public:
         DrainManager(ThrGrpState *pTGS);
         virtual ~DrainManager();

         /// Registers tid as a participant in the current volley of possibly-nested
         ///  Drain() calls. If m_DrainNestLevel is zero upon entering Begin(), a new Drain()
         ///  operation is started.
         /// The returned Barrier object is NULL if tid is a member of the thread group workers
         ///  (self-referential Drain()). If an external Drain(), then the caller must wait on
         ///  the Barrier until the Drain() is complete.
         Barrier * Begin(btTID tid, btUnsignedInt items);

         /// Unregisters tid as a participant in the current Drain(). pDrainBarrier must be
         ///  NULL if tid is a member of the thread group workers. Adjust m_DrainNestLevel.
         ///
         /// return true if tid found to be a Drain()'er.
         btBool End(btTID tid, Barrier *pDrainBarrier);

         /// External Drain()'ers block on m_DrainBarrier to wait for Drain() completion.
         /// In the case of self-referential Join() and self-referential Destroy(), a worker is
         ///  forced to self-terminate before it can Post() the Barrier object stored within
         ///  the OSLThreadGroupNestedBarrierPostD. We Post() that Barrier here so that external
         ///  drainers can resume.
         btBool ReleaseAllDrainers();

         /// A thread group worker may execute a self-referential Drain(), and go on to execute
         ///  a self-referential Join() or Destroy(). When this happens, the worker will self-
         ///  terminate within the Join() or Destroy() call, never completing the Drain(). Before
         ///  self-terminating, the worker must check for instances of its tid in m_SelfDrainers
         ///  and forcibly complete the work items matching its tid using this call.
         void ForciblyCompleteWorkItem();

         /// Query the current Drain() nesting level (number of overlapping Drain() calls).
         btUnsignedInt DrainNestLevel() const { return m_DrainNestLevel; }

         /// Signal that the last participant in the current Drain() is hands-off the thread group.
         void AllDrainersAreDone();

         /// Wait for all threads involved in Drain() to be hands-off the object.
         void WaitForAllDrainersDone();

      protected:

         // Functor that signals completion of a call to OSLThreadGroup::Drain().
         class NestedBarrierPostD : public IDispatchable
         {
         public:
            NestedBarrierPostD(IDispatchable *pContained,
                               DrainManager  *pDrainMgr) :
               m_pContained(pContained),
               m_pDrainMgr(pDrainMgr)
            {}

            void operator() ()
            {
               (*m_pContained) ();                        // Execute the contained work item.
               m_pDrainMgr->CompleteNestedWorkItem(this); // signal its completion.
            }

         protected:
            IDispatchable *m_pContained;
            DrainManager  *m_pDrainMgr;
         };

         typedef std::list<btTID>                drainer_list_t;
         typedef drainer_list_t::iterator        drainer_list_iter;

         typedef std::list<NestedBarrierPostD *> nested_list_t;
         typedef nested_list_t::iterator         nested_list_iter;

         void CompleteNestedWorkItem(NestedBarrierPostD *);
         void DestructMembers();

         ThrGrpState       *m_pTGS;
         btUnsignedInt      m_DrainNestLevel;
         btTime             m_WaitTimeout;
         Barrier            m_DrainBarrier;       // follows the # of work items involved in the Drain().
         Barrier            m_DrainerDoneBarrier; // becomes signaled when the last nested Drain()'er is hands-off.
         drainer_list_t     m_SelfDrainers;
         nested_list_t      m_NestedWorkItems;

         friend class ThrGrpState;
      };

      DrainManager       m_DrainManager;

      btTime PollingInterval() const { return 50; /* millis */ }

      // <IThreadGroup>
      virtual btBool CreateWorkerThread(ThreadProc , OSLThread::ThreadPriority , void * );
      virtual btBool WaitForAllWorkersToStart(btTime Timeout);
      // </IThreadGroup>

      btBool               Quiesce(btTime );
      void         DestructMembers();

      void        WorkerHasStarted(OSLThread * );
      void         WorkerHasExited(OSLThread * );
      void WorkerIsSelfTerminating(OSLThread * );

      /// returns NULL if tid not in group.
      OSLThread * ThreadRunningInThisGroup(btTID ) const;

      eState GetWorkItem(IDispatchable * &pWork);
      eState       State() const { return m_eState; }
      eState       State(eState );

      friend class OSLThreadGroup;
   };

   /// @brief ExecProc is the body of a worker thread.
   ///
   /// @param[in] pThread The pointer to the Thread that will execute the function.
   /// @param[in] lpParms The ThrGrpState of the Thread Group the worker thread will be part of.
   /// @return void
   static void ExecProc(OSLThread *pThread, void *lpParms);

   btBool       m_bDestroyed;
   btTime       m_JoinTimeout;
   ThrGrpState *m_pState;
};

/// @brief Create an OSLThreadGroup whose sole purpose is to dispatch the work item pDisp.
///
/// Waits for the thread group to dispatch pDisp and then destroys the thread group before returning.
/// @param[in] pDisp       - Work item to execute.
/// @param[in] MinThrs     - Minimum number of threads to create in the Thread Group (default = 0).
/// @param[in] MaxThrs     - Maximum number of threads to create in the Thread Group (default = 0).
/// @param[in] ThrPriority - Thread priority of created threads (default = OSLThread::THREADPRIORITY_NORMAL).
/// @param[in] JoinTimeout - Maximum time to wait for the thread(s) to exit when destroying the Thread Group (default = AAL_INFINITE_WAIT).
/// @retval true   if the work item was executed.
/// @retval false  if the work item could not be executed.
OSAL_API btBool   FireAndWait( IDispatchable            *pDisp,
                               btUnsignedInt             MinThrs=0,
                               btUnsignedInt             MaxThrs=0,
                               OSLThread::ThreadPriority ThrPriority=OSLThread::THREADPRIORITY_NORMAL,
                               btTime                    JoinTimeout=AAL_INFINITE_WAIT);

/// @brief Create an OSLThreadGroup whose sole purpose is to dispatch pDisp.
///
/// pDisp is not guaranteed to have dispatched prior to returning from this function.
/// An OSLThread is created to ensure that the thread group is destroyed and its memory reclaimed
///  upon dispatching pDisp. The thread self-destructs after reclaiming the thread group resources.
/// @param[in] pDisp       - Work item to execute.
/// @param[in] MinThrs     - Minimum number of threads to create in the Thread Group (default = 0).
/// @param[in] MaxThrs     - Maximum number of threads to create in the Thread Group (default = 0).
/// @param[in] ThrPriority - Thread priority of created threads (default = OSLThread::THREADPRIORITY_NORMAL).
/// @param[in] JoinTimeout - Maximum time to wait for the thread(s) to exit when destroying the Thread Group (default = AAL_INFINITE_WAIT).
/// @retval true   if the work item was executed.
/// @retval false  if the work item could not be executed.
OSAL_API btBool FireAndForget( IDispatchable            *pDisp,
                               btUnsignedInt             MinThrs=0,
                               btUnsignedInt             MaxThrs=0,
                               OSLThread::ThreadPriority ThrPriority=OSLThread::THREADPRIORITY_NORMAL,
                               btTime                    JoinTimeout=AAL_INFINITE_WAIT);

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_OSAL_THREADGROUP_H__

