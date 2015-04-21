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
/// @file ThreadGroup.h
/// @brief Interface for the ThreadGroup class
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
/// 03/06/2014     JG       Complete rewrite.
//****************************************************************************
#ifndef __AALSDK_OSAL_THREADGROUP_H__
#define __AALSDK_OSAL_THREADGROUP_H__
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/osal/IDispatchable.h>

class IThreadGroup
{
public:
   virtual ~IThreadGroup() {}

   // Basic status check.
   virtual AAL::btBool IsOK() const = 0;

   // Retrieve a snapshot of the current number of threads, which may be less than that originally
   // in the thread group when created. Threads remove themselves from the group as they exit.
   virtual AAL::btUnsignedInt GetNumThreads()   const = 0;

   // Retrieve a snapshot of the number of work items in the queue, which is not guaranteed to
   // be true even upon the return of this call (threads may have consumed more work).
   virtual AAL::btUnsignedInt GetNumWorkItems() const = 0;

   // Add a work item to the queue for processing.
   // returns false, and does not queue the item, if the thread group is stopped or draining.
   virtual AAL::btBool Add(IDispatchable *) = 0;

   // Wait for all worker threads to exit.
   virtual AAL::btBool Join(AAL::btTime ) = 0;

   // Halt the dispatching of all queued work items and prevent new work items from being added.
   // Any work items residing in the queue at the time of the call are removed and deleted, without
   // being executed.
   virtual void         Stop() = 0;

   // Resume a Stopped thread group.
   // returns false if the thread group could not be resumed; otherwise true.
   virtual AAL::btBool Start() = 0;

   // Execute all of the work items currently in the work queue, waiting until they are all
   // complete before returning. Prevent new work items from being added to the thread group
   // while draining.
   // return false if the queue could not be drained; otherwise true.
   virtual AAL::btBool Drain() = 0;

protected:
   enum eState {
      Running = 0,
      Stopped,
      Draining,
      Joining
   };
   // Accessor/Mutator for the thread group state.
   virtual eState State() const  = 0;
   virtual eState State(eState ) = 0;

   virtual AAL::btBool CreateWorkerThread(ThreadProc , OSLThread::ThreadPriority , void * ) = 0;
   virtual AAL::btBool WaitForAllWorkersToStart(AAL::btTime ) = 0;

   virtual AAL::btBool Destroy(AAL::btTime ) = 0;
};


//=============================================================================
// Name: OSLThreadGroup
// Description: ThreadGroup (a.k.a. thread pooled) functor scheduler.
// Interface: public
// Comments: 
//=============================================================================
class OSAL_API OSLThreadGroup : public IThreadGroup
{
public:
   // Default Min thread is to let the TG decide. Max < min then Max == Min
   OSLThreadGroup(AAL::btUnsignedInt        uiMinThreads=0,
                  AAL::btUnsignedInt        uiMaxThreads=0,
                  OSLThread::ThreadPriority nPriority=OSLThread::THREADPRIORITY_NORMAL,
                  AAL::btTime               JoinTimeout=AAL_INFINITE_WAIT);

   virtual ~OSLThreadGroup();

   // <IThreadGroup>
   virtual AAL::btBool IsOK()                   const { return NULL != m_pState;            }
   virtual AAL::btUnsignedInt GetNumThreads()   const { return m_pState->GetNumThreads();   }
   virtual AAL::btUnsignedInt GetNumWorkItems() const { return m_pState->GetNumWorkItems(); }
   virtual AAL::btBool Add(IDispatchable *pDisp)      { return m_pState->Add(pDisp);        }
   virtual AAL::btBool Join(AAL::btTime timeout)      { return m_pState->Join(timeout);     }
   virtual AAL::btBool Drain()                        { return m_pState->Drain();           }
   virtual void Stop()                                { m_pState->Stop();                   }
   virtual AAL::btBool Start()                        { return m_pState->Start();           }
   // </IThreadGroup>

protected:
   virtual IThreadGroup::eState State() const                  { return m_pState->State();   }
   virtual IThreadGroup::eState State(IThreadGroup::eState st) { return m_pState->State(st); }

   virtual AAL::btBool CreateWorkerThread(ThreadProc fn, OSLThread::ThreadPriority pri, void *context)
   { return m_pState->CreateWorkerThread(fn, pri, context); }

   virtual AAL::btBool WaitForAllWorkersToStart(AAL::btTime Timeout)
   { return m_pState->WaitForAllWorkersToStart(Timeout); }

   virtual AAL::btBool Destroy(AAL::btTime Timeout)
   { return m_pState->Destroy(Timeout); }

private:
   //
   // Object that holds state and semaphores for the
   //  thread group. This object "lives" outside the
   //  ThreadGroup object so that Threads may continue to
   //  clean-up even if the ThreadGroup proper has been destroyed.
   class ThrGrpState : public IThreadGroup,
                       public CriticalSection
   {
#define THRGRPSTATE_FLAG_OK        0x00000001
#define THRGRPSTATE_FLAG_DRAINING  0x00000002
#define THRGRPSTATE_FLAG_SELF_JOIN 0x00000004
#define THRGRPSTATE_FLAG_JOINED    0x00000008
#define THRGRPSTATE_FLAG_JOINING   0x00000010
   public:
      ThrGrpState(AAL::btUnsignedInt NumThreads);
      virtual ~ThrGrpState();

      // <IThreadGroup>
      virtual AAL::btBool IsOK() const { return flag_is_set(m_Flags, THRGRPSTATE_FLAG_OK); }
      virtual AAL::btUnsignedInt GetNumThreads()   const;
      virtual AAL::btUnsignedInt GetNumWorkItems() const;
      virtual AAL::btBool Add(IDispatchable * );
      virtual AAL::btBool Join(AAL::btTime );
      virtual AAL::btBool Drain();
      virtual void Stop();
      virtual AAL::btBool Start();
      // </IThreadGroup>

   protected:
      typedef std::queue<IDispatchable *>  work_queue_t;
      typedef std::list<OSLThread      *>  thr_list_t;
      typedef thr_list_t::iterator         thr_list_iter;
      typedef thr_list_t::const_iterator   const_thr_list_iter;

      enum IThreadGroup::eState m_eState;
      AAL::btUnsignedInt        m_Flags;
      AAL::btTime               m_WorkSemTimeout;
      AAL::btTID                m_Joiner;
      CSemaphore                m_ThrStartSem;
      CSemaphore                m_ThrExitSem;
      CSemaphore                m_WorkSem;

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      work_queue_t              m_workqueue;
      thr_list_t                m_RunningThreads;
      thr_list_t                m_ExitedThreads;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

      class DrainManager
      {
      protected:
         typedef std::list<AAL::btTID>       self_drain_list_t;
         typedef self_drain_list_t::iterator self_drain_list_iter;

         class ExternalDrainer
         {
         public:
            ExternalDrainer() :
               m_tid(0),
               m_pSem(NULL)
            {}
            ExternalDrainer(AAL::btTID tid, CSemaphore *pSem) :
               m_tid(tid),
               m_pSem(pSem)
            {}
            virtual ~ExternalDrainer() {}

            void DrainComplete()
            {
               if ( NULL != m_pSem ) {
                  AAL::btInt c = 0;
                  AAL::btInt m = 0;
                  m_pSem->CurrCounts(c, m);
                  m_pSem->Post(1 - c);
               }
            }

            void Destroy()
            {
               if ( NULL != m_pSem ) {
                  delete m_pSem;
                  m_pSem = NULL;
               }
            }

            AAL::btBool operator == (const ExternalDrainer &right) const
            { return ThreadIDEqual(m_tid, right.m_tid); }

            AAL::btTID  m_tid;
            CSemaphore *m_pSem;
         };

         typedef std::list<ExternalDrainer> ext_drain_list_t;
         typedef ext_drain_list_t::iterator ext_drain_list_iter;

      public:
         DrainManager(ThrGrpState *pTGS) :
            m_pTGS(pTGS),
            m_DrainNestLevel(0)
         {}
         virtual ~DrainManager()
         {
            ASSERT(0 == m_DrainNestLevel);
            ASSERT(0 == m_SelfDrainers.size());
            ASSERT(0 == m_ExternalDrainers.size());
         }

         CSemaphore * Begin(AAL::btTID tid, AAL::btInt items)
         {
            AutoLock(m_pTGS);

            // non-NULL means self-referential Drain().
            OSLThread *pThread = m_pTGS->ThreadRunningInThisGroup(tid);

            ++m_DrainNestLevel;

            if ( 1 == m_DrainNestLevel ) {
               // Beginning a new series of Drain() call(s).
               flag_setf(m_pTGS->m_Flags, THRGRPSTATE_FLAG_DRAINING);
            }

            if ( NULL != pThread ) {
               // Add tid to the list of self-drainers, allowing duplicates.
               m_SelfDrainers.push_back(tid);

               // No sem used for self-drainers.
               return NULL;
            }

            CSemaphore *pDrainSem = new(std::nothrow) CSemaphore();

            pDrainSem->Create(-items, 1);

            m_ExternalDrainers.push_back(ExternalDrainer(tid, pDrainSem));

            return pDrainSem;
         }

         // This version of the call is made from Drain(), where pDrainSem is known (or NULL),
         //  after we have woken from sleep on pDrainSem.
         AAL::btUnsignedInt End(AAL::btTID tid, CSemaphore *pDrainSem)
         {
            AutoLock(m_pTGS);

            if ( NULL == pDrainSem ) {
               // self-referential Drain(). Remove one instance of tid from m_SelfDrainers.
               self_drain_list_iter siter;

               for ( siter = m_SelfDrainers.begin() ; m_SelfDrainers.end() != siter ; ++siter ) {
                  if ( ThreadIDEqual(tid, *siter) ) {
                     m_SelfDrainers.erase(siter);
                     --m_DrainNestLevel;
                     break;
                  }
               }

            } else {
               // external Drain().
               ext_drain_list_iter eiter = std::find(m_ExternalDrainers.begin(),
                                                     m_ExternalDrainers.end(),
                                                     ExternalDrainer(tid, pDrainSem));

               if ( m_ExternalDrainers.end() != eiter ) {
                  ASSERT((*eiter).m_pSem == pDrainSem);
                  (*eiter).Destroy();
                  m_ExternalDrainers.erase(eiter);
                  --m_DrainNestLevel;
               }
            }

            return m_DrainNestLevel;
         }

         // In the case of self-referential Join() and self-referential Destroy(), a worker may
         //  be required to terminate before returning up the call stack to complete a previous
         //  self-referential Drain(). Use this to forcibly complete such Drain()'s.
         void WorkerHasExited(AAL::btTID tid)
         {
            AutoLock(m_pTGS);

            self_drain_list_iter siter;

            // Process all instances of tid found in m_SelfDrainers.
            for ( siter = m_SelfDrainers.begin() ; m_SelfDrainers.end() != siter ; ) {

               if ( ThreadIDEqual(tid, *siter) ) {
                  self_drain_list_iter trash(siter);
                  ++siter;
                  m_SelfDrainers.erase(trash);

                  --m_DrainNestLevel;
                  if ( 0 == m_DrainNestLevel ) {
                     // This series of Drain() calls(s) is complete.
                     flag_clrf(m_pTGS->m_Flags, THRGRPSTATE_FLAG_DRAINING);
                  }
               } else {
                  ++siter;
               }

            }
         }

         // External Drain()'ers block on a semaphore to wait for Drain() completion.
         // In the case of self-referential Join() and self-referential Destroy(), a worker is
         //  forced to terminate before it can Post() the semaphore. We Post() all external
         //  Drain()'ers here so that they can complete.
         void ReleaseExternalDrainers()
         {
            AutoLock(m_pTGS);

            ext_drain_list_iter eiter;
            for ( eiter = m_ExternalDrainers.begin() ; m_ExternalDrainers.end() != eiter ; ++eiter ) {
               (*eiter).DrainComplete();
            }
         }

      protected:
         ThrGrpState       *m_pTGS;
         AAL::btUnsignedInt m_DrainNestLevel;
         self_drain_list_t  m_SelfDrainers;
         ext_drain_list_t   m_ExternalDrainers;
      };

      DrainManager              m_DrainManager;

      AAL::btTime PollingInterval() const { return 50; /* millis */ }

      // <IThreadGroup>
      virtual IThreadGroup::eState State() const { return m_eState; }
      virtual IThreadGroup::eState State(IThreadGroup::eState );

      virtual AAL::btBool CreateWorkerThread(ThreadProc , OSLThread::ThreadPriority , void * );
      virtual AAL::btBool WaitForAllWorkersToStart(AAL::btTime Timeout);

      virtual AAL::btBool Destroy(AAL::btTime );

      // </IThreadGroup>

      AAL::btBool    DestroyWhileDraining(AAL::btTime );
      AAL::btBool     DestroyWhileJoining(AAL::btTime );

      AAL::btBool WaitForAllWorkersToExit(AAL::btTime );
      AAL::btBool PollForAllWorkersToJoin(AAL::btTime );
      AAL::btBool  PollForDrainCompletion(AAL::btTime );

      AAL::btBool       JoinExitedWorkers(AAL::btTime );

      AAL::btBool Quiesce(AAL::btTime );


      void  WorkerHasStarted(OSLThread * );
      void   WorkerHasExited(OSLThread * );
      void   RunningToExited(OSLThread * );
      void RemoveFromRunning(OSLThread * );

      // returns NULL if tid not in group.
      OSLThread * ThreadRunningInThisGroup(AAL::btTID tid) const;

      IThreadGroup::eState GetWorkItem(IDispatchable * &pWork);

      friend class OSLThreadGroup;
   };

   // lpParms is a ThrGrpState.
   static void ExecProc(OSLThread *pThread, void *lpParms);

   AAL::btTime  m_JoinTimeout;
   ThrGrpState *m_pState;
};

#endif // __AALSDK_OSAL_THREADGROUP_H__

