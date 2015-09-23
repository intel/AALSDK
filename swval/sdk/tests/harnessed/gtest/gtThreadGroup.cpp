// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtThreadGroup.h"

// Simple test fixture
class OSAL_ThreadGroup_f : public ::testing::Test
{
protected:
   OSAL_ThreadGroup_f() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~OSAL_ThreadGroup_f() {}

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   // virtual void SetUp() {}
   virtual void TearDown()
   {
      if ( NULL != m_pGroup ) {
         delete m_pGroup;
         m_pGroup = NULL;
      }

      disp_list_iter iter;
      for ( iter = m_WorkList.begin() ; iter != m_WorkList.end() ; ++iter ) {
         delete *iter;
      }
      m_WorkList.clear();

      unsigned i;
      for ( i = 0 ; i < sizeof(m_Sems) / sizeof(m_Sems[0]) ; ++i ) {
         m_Sems[i].Destroy();
      }
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs/*=0*/,
                           OSLThread::ThreadPriority nPriority/*=OSLThread::THREADPRIORITY_NORMAL*/,
                           AAL::btTime               JoinTimeout/*=AAL_INFINITE_WAIT*/)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      m_JoinTimeout   = JoinTimeout;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority, m_JoinTimeout);
   }

   AAL::btBool Add(IDispatchable *pDisp)
   {
      if ( NULL != pDisp ) {
         m_WorkList.push_back(pDisp);
      }
      return m_pGroup->Add(pDisp);
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   AAL::btTime               m_JoinTimeout;
   CSemaphore                m_Sems[4];
   disp_list_t               m_WorkList;
};

// Value-parameterized test fixture
template <typename T>
class OSAL_ThreadGroup_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_ThreadGroup_vp() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~OSAL_ThreadGroup_vp() {}

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }
   }
   virtual void TearDown()
   {
      if ( NULL != m_pGroup ) {
         delete m_pGroup;
         m_pGroup = NULL;
      }

      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      disp_list_iter iter;
      for ( iter = m_WorkList.begin() ; iter != m_WorkList.end() ; ++iter ) {
         delete *iter;
      }
      m_WorkList.clear();

      for ( i = 0 ; i < sizeof(m_Sems) / sizeof(m_Sems[0]) ; ++i ) {
         m_Sems[i].Destroy();
      }
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs,
                           OSLThread::ThreadPriority nPriority,
                           AAL::btTime               JoinTimeout)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      m_JoinTimeout   = JoinTimeout;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority, m_JoinTimeout);
   }

   AAL::btBool Add(IDispatchable *pDisp)
   {
      if ( NULL != pDisp ) {
         m_WorkList.push_back(pDisp);
      }
      return m_pGroup->Add(pDisp);
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   AAL::btTime               m_JoinTimeout;
   OSLThread                *m_pThrs[5];
   CSemaphore                m_Sems[4];
   disp_list_t               m_WorkList;
};

#define STAGE_WORKERS(__init)                                         \
   EXPECT_EQ(0, CurrentThreads());                                    \
                                                                      \
   OSLThreadGroup *g = Create(__init,                                 \
                              0,                                      \
                              OSLThread::THREADPRIORITY_NORMAL,       \
                              AAL_INFINITE_WAIT);                     \
   ASSERT_NONNULL(g);                                                 \
   ASSERT_TRUE(g->IsOK());                                            \
                                                                      \
   AAL::btInt w = (AAL::btInt)m_MinThreads;                           \
                                                                      \
   /* m_Sems[0] - count up sem, Post()'ed by each worker thread. */   \
   /* m_Sems[1] - blocks worker threads in their dispatchables. */    \
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));                              \
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));                         \
                                                                      \
   AAL::btUnsignedInt i;                                              \
   for ( i = 0 ; i < m_MinThreads ; ++i ) {                           \
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));    \
   }                                                                  \
                                                                      \
   EXPECT_EQ(m_MinThreads, CurrentThreads());                         \
                                                                      \
   /* Block until w counts have been Post()'ed to WorkerCount. */     \
   EXPECT_TRUE(m_Sems[0].Wait())


TEST_F(OSAL_ThreadGroup_f, aal0072)
{
   // When OSLThreadGroup is constructed with 0 == MinThreads and 0 == MaxThreads, min = max = 1.

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(0,
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);

   EXPECT_EQ(1, CurrentThreads());

   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());
   EXPECT_EQ(1, g->GetNumThreads());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_F(OSAL_ThreadGroup_f, aal0075)
{
   // OSLThreadGroup objects are created in a state that immediately
   // accepts IDispatchable's for execution.

   ASSERT_TRUE(m_Sems[0].Create(0, 1));

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(1,
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_EQ(1, CurrentThreads());

   EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_F(OSAL_ThreadGroup_f, aal0076)
{
   // OSLThreadGroup::Add() does not queue / returns false for NULL work items.

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(1,
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_FALSE(Add(NULL));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

////////////////////////////////////////////////////////////////////////////////

class OSAL_ThreadGroup_vp_uint_0 : public OSAL_ThreadGroup_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );
   static void Thr5(OSLThread * , void * );
   static void Thr6(OSLThread * , void * );
   static void Thr7(OSLThread * , void * );
   static void Thr8(OSLThread * , void * );
   static void Thr9(OSLThread * , void * );
   static void Thr10(OSLThread * , void * );
   static void Thr11(OSLThread * , void * );
};

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0074)
{
   // OSLThreadGroup::OSLThreadGroup() creates a thread group with uiMinThreads worker threads.

   STAGE_WORKERS(GetParam());

   AAL::btInt Cur;
   AAL::btInt Max;

   EXPECT_TRUE(m_Sems[0].CurrCounts(Cur, Max));
   EXPECT_EQ(0, Cur); // Current count must equal 0 (count up sem).

   EXPECT_EQ(m_MinThreads, g->GetNumThreads());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0079)
{
   // OSLThreadGroup::GetNumWorkItems() returns a snapshot of the number of work items in the queue.

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, 1));

   OSLThreadGroup *g = Create(1,
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_EQ(0, g->GetNumWorkItems());

   // Queue a dispatchable that will block the worker thread.
   EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));

   // Wait for the initial work item to be dispatched, guaranteeing that the work queue is empty (1 worker).
   EXPECT_TRUE(m_Sems[0].Wait());
   EXPECT_EQ(0, g->GetNumWorkItems());

   EXPECT_EQ(1, g->GetNumThreads());

   // There is one worker thread in the group, and that thread will block, unable to consume
   // more work items.
   // The work queue is currently empty.
   // Adding work items here will cause them to back up in the queue.

   AAL::btInt               counter = 0;
   AAL::btUnsignedInt       i;
   const AAL::btUnsignedInt N = GetParam();

   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(Add( new UnsafeCountUpD(counter) ));
      EXPECT_EQ(i + 1, g->GetNumWorkItems());
   }

   // none of the work items should have been dispatched.
   EXPECT_EQ(0, counter);

   // free up the worker thread, allowing it to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // wait for the worker to drain the queue.
   EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(counter, (AAL::btInt)N);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0080)
{
   // When constructed with bAutoJoin=true and no explicit call to OSLThreadGroup::Join(),
   // OSLThreadGroup::~OSLThreadGroup() joins all workers. (Running -> Joining)

   STAGE_WORKERS(GetParam());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Destroy the thread group now.
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0081)
{
   // When constructed with bAutoJoin=true and when calling OSLThreadGroup::Join() explicitly,
   // OSLThreadGroup::~OSLThreadGroup() skips the join step. (Running -> Joining)

   STAGE_WORKERS(GetParam());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Explicitly join all workers.
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0084)
{
   // Calling OSLThreadGroup::Start() on a Running thread group does not
   // affect the object. (Running -> Running)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Start());

      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));

      EXPECT_TRUE(g->Start());
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0085)
{
   // Calling OSLThreadGroup::Stop() on a Running thread group removes and destroys
   // all work items currently in the queue and prevents new items from being added
   // to the queue. (Running -> Stopped)

   STAGE_WORKERS(GetParam());

   // All workers are dispatching and are currently blocked or will block on m_Sems[1].
   // No worker is blocked on the work queue sem.

   // Add a few more items to the thread group. They should accumulate in the queue.
   EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   EXPECT_EQ(3, g->GetNumWorkItems());

   // Now, stop the thread group - this should purge the work queue of these last 3 Add()'s.
   g->Stop();
   EXPECT_EQ(0, g->GetNumWorkItems());

   IDispatchable *pDisp = new PostThenWaitD(m_Sems[0], m_Sems[1]);
   ASSERT_NONNULL(pDisp);

   // thread group is Stopped - should deny new Add()'s.
   ASSERT_FALSE(g->Add(pDisp));
   delete pDisp;
   ASSERT_EQ(0, g->GetNumWorkItems());

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // Start the thread group again.
   EXPECT_TRUE(g->Start());

   // The thread group should accept new work items now.
   AAL::btInt x = 0;
   EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());

   // We're using auto-join, so all work items should be complete.
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0086)
{
   // Calling OSLThreadGroup::Drain() on a Running thread group executes all work
   // items currently in the work queue. Upon return from Drain(), the thread group
   // will be in the Running state again. While draining, requests to add new work
   // items are denied. (Running -> Draining) (Draining -> Running)

   STAGE_WORKERS(GetParam());

   // All workers are dispatching and are currently blocked or will block on m_Sems[1].

   // Add a few more work items. They will hang out in the queue.
   AAL::btInt x = 0;

   EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));

   for ( i = 0 ; i < 49 ; ++i ) {
      if ( 0 == i % 5 ) {
         EXPECT_TRUE(Add( new AddNopToThreadGroupD(g, 1, false) ));
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Unblock one worker. That worker will sleep briefly, giving us a chance to call Drain.
   // We will go to sleep on Drain() below. When the first worker wakes, he will wake the rest of
   // the workers.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Wait for the Drain to complete.
   EXPECT_TRUE(g->Drain());

   EXPECT_EQ(1, x);
   EXPECT_EQ(0, g->GetNumWorkItems());

   // The thread group should remain in the Running state, able to accept work.
   EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));

   // Block until this newly-added work item executes, proving that the group is still Running.
   EXPECT_TRUE(m_Sems[0].Wait());

   ASSERT_EQ(m_MinThreads, CurrentThreads());
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0087)
{
   // Calling OSLThreadGroup::Stop() on a Stopped thread group does
   // not affect the object. (Stopped -> Stopped)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;
   AAL::btInt i;
   for ( i = 0 ; i < 1000 ; ++i ) {
      EXPECT_TRUE(g->Add( new DelUnsafeCountUpD(x) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   g->Stop();
   EXPECT_LT(0, x);
   EXPECT_EQ(0, g->GetNumWorkItems());

   IDispatchable *pDisp = new NopD();
   ASSERT_NONNULL(pDisp);
   ASSERT_FALSE(g->Add(pDisp));

   g->Stop();
   EXPECT_EQ(0, g->GetNumWorkItems());
   ASSERT_FALSE(g->Add(pDisp));

   g->Stop();
   EXPECT_EQ(0, g->GetNumWorkItems());
   ASSERT_FALSE(g->Add(pDisp));

   delete pDisp;
   i = 0;

   EXPECT_TRUE(g->Start());
   EXPECT_TRUE(Add( new UnsafeCountUpD(i) ));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());

   EXPECT_EQ(1, i);
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0088)
{
   // Calling OSLThreadGroup::Drain() on a Stopped thread group does not affect the object.
   // (No state transition. Is queue empty check.)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   AAL::btInt y = 0;
   AAL::btInt i;
   for ( i = 0 ; i < 1000 ; ++i ) {
      EXPECT_TRUE(g->Add( new DelUnsafeCountUpD(y) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   YIELD_X(3);

   g->Stop();
   EXPECT_LT(0, y);
   EXPECT_EQ(0, g->GetNumWorkItems());

   EXPECT_TRUE(g->Drain());

   // Start the thread group again.
   EXPECT_TRUE(g->Start());

   // The thread group should accept new work items now.
   AAL::btInt x = 0;
   EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());

   // We're using auto-join, so all work items should be complete.
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0089)
{
   // When constructed with bAutoJoin=true and when the thread group is Stopped,
   // OSLThreadGroup::~OSLThreadGroup() joins all workers. (Stopped -> Joining)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;
   AAL::btInt i;
   for ( i = 0 ; i < 1000 ; ++i ) {
      EXPECT_TRUE(g->Add( new DelUnsafeCountUpD(x) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   YIELD_X(10);

   g->Stop();
   EXPECT_LE(0, x);
   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(m_MinThreads, CurrentThreads());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0090)
{
   // When a thread group is Stopped, OSLThreadGroup::Join() joins all workers,
   // and ~OSLThreadGroup() deletes the ThrGrpState object. (Stopped -> Joining)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;
   AAL::btInt i;
   for ( i = 0 ; i < 1000 ; ++i ) {
      EXPECT_TRUE(g->Add( new DelUnsafeCountUpD(x) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   g->Stop();
   EXPECT_LT(0, x);
   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(m_MinThreads, CurrentThreads());

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

void OSAL_ThreadGroup_vp_uint_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   EXPECT_TRUE(pTC->m_pGroup->Drain());
   EXPECT_EQ(0, pTC->m_pGroup->GetNumWorkItems());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0092)
{
   // Nesting of OSLThreadGroup::Drain() calls is supported. Only the final Drain() call
   // may transition the state back to Running. (Draining -> Draining)

   ASSERT_EQ(0, CurrentThreads());

   const AAL::btInt ThrCount = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
   AAL::btInt       i;

   // m_Sems[2] is a count-up sem. Instances of Thr0 pulse the sem as they start to execute.
   // Thr0's will block on m_Sems[3] until this thread is ready.
   ASSERT_TRUE(m_Sems[2].Create(-ThrCount, 1));
   ASSERT_TRUE(m_Sems[3].Create(0, INT_MAX));

   for ( i = 0 ; i < ThrCount ; ++i ) {
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr0,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
   }

   // Wait for all Thr0's to ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(ThrCount, (AAL::btInt)CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(ThrCount + w, (AAL::btInt)CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   for ( i = 0 ; i < (AAL::btInt) m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All of the thread group workers are in dispatch and will block on m_Sems[1].
   // Adding more work to the thread group will back up in the queue.

   EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));

   const AAL::btInt ItemsToAdd = 250;

   AAL::btInt xCount = 0;
   AAL::btInt x[25];
   AAL::btInt yCount = 0;
   AAL::btInt y[25];
   AAL::btInt t      = 0;

   memset(x, 0, sizeof(x));
   memset(y, 0, sizeof(y));

   AAL::btUnsigned32bitInt r = 123;

   for ( i = 0 ; i < ItemsToAdd ; ++i ) {

      switch ( GetRand(&r) % 10 ) {
         case 0 : {
            if ( xCount < sizeof(x) / sizeof(x[0]) ) {
               EXPECT_TRUE(Add( new UnsafeCountUpD(x[xCount]) ));
               ++xCount;
            } else {
               EXPECT_TRUE(Add( new YieldD() ));
            }
         } break;

         case 1 : {
            if ( yCount < sizeof(y) / sizeof(y[0]) ) {
               EXPECT_TRUE(Add( new UnsafeCountDownD(y[yCount]) ));
               ++yCount;
            } else {
               EXPECT_TRUE(Add( new YieldD() ));
            }
         } break;

         case 2 : { // Add() during Drain() is invalid.
            EXPECT_TRUE(Add( new AddNopToThreadGroupD(g, 1, false) ));
         } break;

         case 3 : { // Posts to m_Sems[3] will wake the Thr0 threads to perform nested Drain()'s.
            if ( t < ThrCount ) {
               EXPECT_TRUE(Add( new PostD(m_Sems[3]) ));
               ++t;
            } else {
               EXPECT_TRUE(Add( new YieldD() ));
            }
         } break;

         default : {
            EXPECT_TRUE(Add( new NopD() ));
         } break;
      }

   }

   // Be sure to wake all of the Thr0's.
   while ( t < ThrCount ) {
      EXPECT_TRUE(Add( new PostD(m_Sems[3]) ));
      ++t;
   }

   EXPECT_LE(ItemsToAdd + 1, g->GetNumWorkItems());

   // Wake one worker. He will consume the first item and wake the rest of the workers,
   // which will in turn randomly wake the Thr0's as the queue drains.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Drain());
   EXPECT_EQ(0, g->GetNumWorkItems());

   // Join all of the Thr0's.
   for ( i = 0 ; i < ThrCount ; ++i ) {
      m_pThrs[i]->Join();
   }

   for ( i = 0 ; i < xCount ; ++i ) {
      EXPECT_EQ(1, x[i]);
   }
   for ( i = 0 ; i < yCount ; ++i ) {
      EXPECT_EQ(-1, y[i]);
   }

   EXPECT_EQ(w, (AAL::btInt)CurrentThreads());

   // The thread group must be in the Running state.
   EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, (AAL::btInt)CurrentThreads());
}

void OSAL_ThreadGroup_vp_uint_0::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);
   EXPECT_TRUE(pTC->m_pGroup->Drain());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0093)
{
   // When constructed with bAutoJoin=true and when the thread group is Draining,
   // deleting the thread group will join all worker threads in ~OSLThreadGroup
   // (Draining -> Joining). Both the Drain() and the Join() calls will return true
   // to indicate success.

   // Use Thr1 to perform the Drain(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr1 is ready.
   // m_Sems[3] - Thr1 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr1 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr1 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 40 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake one worker, then block on m_Sems[0].
   // The first worker wakes Thr1, then yields the cpu to Thr1.
   // Thr1 begins the Drain().
   // The first worker wakes this thread, then yields the cpu to this thread.
   // This thread calls Destroy(), and begins to wait for the Drain() to complete.
   // The first worker wakes the remaining workers. All workers process the work queue.
   // The queue becomes empty, and the Drain() completes successfully.
   // Seeing the Drain() completion, the Destroy() proceeds.

   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_X(20);

   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group - destruct during Drain().
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   // Join Thr1.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_0::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   EXPECT_TRUE(pTC->m_Sems[0].Post(1));
   EXPECT_TRUE(pTC->m_pGroup->Drain());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0094)
{
   // When the thread group is Draining, the group may be Join()'ed successfully.
   // (Draining -> Joining) Both the Drain() and the Join() calls will return true
   // to indicate success.

   // Use Thr2 to perform the Drain(), this thread to do the Join().

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr2 is ready.
   // m_Sems[3] - Thr2 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr2 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr2 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr2 to invoke the Drain().
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w - 1)) );
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr2 to do the Drain(), then wake us to perform the
   // Join(), then wake the remaining workers to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   YIELD_X(5);

   // Join the thread group.
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr2.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
}

void OSAL_ThreadGroup_vp_uint_0::Thr4(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);
   EXPECT_FALSE(pTC->m_pGroup->Start());

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0096)
{
   // When ~OSLThreadGroup() has transitioned a thread group to the Joining state,
   // attempts to Start() return false to indicate failure. The thread group remains
   // in the Joining state.

   // Use Thr4 to attempt the Start(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr4 is ready.
   // m_Sems[3] - Thr4 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr4 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr4 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1)) );
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr4 to attempt the Start().
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[2]) )); // Some worker waits for Thr4 to exit.
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be in the destructor, doing the Join().
   // Some worker wakes Thr4 to attempt the Start().
   // Some worker waits for Thr4 to exit so that Thr4 doesn't de-reference a pointer to the
   //  deleted thread group.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   // Join Thr4.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_0::Thr5(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);
   EXPECT_FALSE(pTC->m_pGroup->Start());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0097)
{
   // When an explicit call to Join() has transitioned a thread group to the Joining state,
   // attempts to Start() return false to indicate failure. The thread group remains in
   // the Joining state.

   // Use Thr5 to attempt the Start(), this thread to do the Join().

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr5 is ready.
   // m_Sems[3] - Thr5 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr5 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr5 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1)) );
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr5 to attempt the Start().
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Some worker will eventually wake Thr5 to attempt the Start().
   // Meanwhile, this thread will be doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr5.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

void OSAL_ThreadGroup_vp_uint_0::Thr6(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   pTC->m_pGroup->Stop();

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0098)
{
   // When ~OSLThreadGroup() has transitioned a thread group to the Joining state,
   // attempts to Stop() fail. The thread group remains in the Joining state.

   // Use Thr6 to attempt the Stop(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr6 is ready.
   // m_Sems[3] - Thr6 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr6 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr6 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1)) );
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr6 to attempt the Stop().
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[2]) )); // Some worker waits for Thr6 to exit.
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be in the destructor, doing the Join().
   // Some worker wakes Thr6, which attempts a Stop(), but fails.
   // Some worker waits for Thr6 to exit, so that Thr6 doesn't de-reference a pointer to the
   //  deleted thread group.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   // Join Thr6.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_0::Thr7(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);
   pTC->m_pGroup->Stop();
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0099)
{
   // When an explicit call to Join() has transitioned a thread group to the Joining state,
   // attempts to Stop() fail. The thread group remains in the Joining state.

   // Use Thr7 to attempt the Stop(), this thread to do the Join().

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr7 is ready.
   // m_Sems[3] - Thr7 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr7,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr7 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr7 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1)) );
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr7 to attempt the Stop().
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Some worker wakes Thr7 to attempt a Stop(), which fails.
   // Meanwhile, this thread will be doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr7.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

void OSAL_ThreadGroup_vp_uint_0::Thr8(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   YIELD_X(5);
   EXPECT_FALSE(pTC->m_pGroup->Drain());

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0100)
{
   // When ~OSLThreadGroup() has transitioned a thread group to the Joining state,
   // attempts to Drain() return false to indicate failure. The thread group remains
   // in the Joining state.

   // Use Thr8 to attempt the Drain(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr8 is ready.
   // m_Sems[3] - Thr8 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr8,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr8 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr8 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 30 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[2]) )); // Some worker waits for Thr8 to exit.
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be in the destructor, doing the Join().
   // Some worker will wake Thr8 to attempt a Drain(), which should fail.
   // Some worker waits for Thr8 to complete, so that Thr8 doesn't de-reference the
   //  thread group pointer after it has been deleted.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[3].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   // Join Thr8.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_0::Thr9(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   YIELD_X(5);
   EXPECT_FALSE(pTC->m_pGroup->Drain());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0101)
{
   // When an explicit call to Join() has transitioned a thread group to the Joining state,
   // attempts to Drain() return false to indicate failure. The thread group remains in
   // the Joining state.

   // Use Thr9 to attempt the Drain(), this thread to do the Join().

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr9 is ready.
   // m_Sems[3] - Thr9 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr9,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr9 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr9 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 20 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be in Join().
   // Some worker will wake Thr9 to attempt a Drain(), which should fail.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[3].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr9.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

void OSAL_ThreadGroup_vp_uint_0::Thr10(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);
   EXPECT_FALSE(pTC->m_pGroup->Join(AAL_INFINITE_WAIT));

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0102)
{
   // When ~OSLThreadGroup() has transitioned a thread group to the Joining state,
   // attempts to Join() return false to indicate failure for the second Join().
   // The first Join() is allowed to complete successfully.

   // Use Thr10 to attempt the 2nd Join(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr10 is ready.
   // m_Sems[3] - Thr10 waits for a signal to do the Join().
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr10,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr10 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr10 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1)) );
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr10 to attempt the Join().
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[2]) )); // Some worker waits for Thr10 to exit.
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be in the destructor, doing the 1st Join().
   // Some worker wakes Thr10, which attempts another Join(), but fails.
   // Some worker waits for Thr10 to exit, so that Thr10 doesn't attempt to de-reference a
   //  pointer to the deleted thread group.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   // Join Thr10.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_0::Thr11(OSLThread *pThread, void *pContext)
{
   OSAL_ThreadGroup_vp_uint_0 *pTC = static_cast<OSAL_ThreadGroup_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   // signal that we're ready.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));

   // wait for the "go" signal.
   EXPECT_TRUE(pTC->m_Sems[3].Wait());

   ASSERT_NONNULL(pTC->m_pGroup);

   EXPECT_FALSE(pTC->m_pGroup->Join(AAL_INFINITE_WAIT));
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0103)
{
   // When an explicit call to Join() has transitioned a thread group to the Joining state,
   // attempts to Join() return false to indicate failure for the second Join().
   // The first Join() is allowed to complete successfully.

   // Use Thr11 to attempt the 2nd Join(), this thread to do the 1st Join().

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr11 is ready.
   // m_Sems[3] - Thr11 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr11,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr11 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr11 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[3]) )); // Wakes Thr11 to attempt the 2nd Join().
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   EXPECT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, which will sleep then wake the remaining workers.
   // Meanwhile, this thread will be doing the 1st Join().
   // Some worker wakes Thr11 to attempt a 2nd Join(), which fails.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr11.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_0,
                           ::testing::Values((AAL::btUnsignedInt)1,
                                             (AAL::btUnsignedInt)5,
                                             (AAL::btUnsignedInt)10,
                                             (AAL::btUnsignedInt)25));


TEST(FireAndWait, aal0691)
{
   // FireAndWait() waits for the given IDispatchable to complete execution before returning.

   btInt i = 0;

   UnsafeCountUpD d(i);

   EXPECT_EQ(0, GlobalTestConfig::GetInstance().CurrentThreads());

   FireAndWait(&d);

   EXPECT_EQ(0, GlobalTestConfig::GetInstance().CurrentThreads());
   EXPECT_EQ(1, i);
}

TEST(FireAndForget, aal0692)
{
   // FireAndForget() provides asynchronous event dispatch.

   btInt i = 0;

   UnsafeCountUpD d(i);

   EXPECT_EQ(0, GlobalTestConfig::GetInstance().CurrentThreads());

   FireAndForget(&d);

   YIELD_WHILE( GlobalTestConfig::GetInstance().CurrentThreads() > 0 );
   EXPECT_EQ(1, i);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
class ThrGrFAF : public ::testing::TestWithParam< unsigned long >
{
protected:
   ThrGrFAF() {}

   virtual void SetUp()
   {
      EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
   }
   // virtual void TearDown() { }

   CSemaphore m_Sem;
};

TEST_P(ThrGrFAF, DISABLED_FireAndForget)
{
   // Functor for Fire-and-Forget test.
   class FireAndForget : public IDispatchable
   {
   public:
      FireAndForget(CSemaphore   *psem,
                    unsigned long delay_in_micros) :
         m_psem(psem),
         m_micros(delay_in_micros),
         m_tg()
      {}

      void Fire()
      {
         m_tg.Add(this);
      }

      void operator() ()
      {
         SleepMicro(m_micros);
         m_psem->Post(1);
         delete this;
      }

   protected:
      CSemaphore    *m_psem;
      unsigned long  m_micros;
      OSLThreadGroup m_tg;
   };

   (new FireAndForget(&m_Sem, GetParam()))->Fire();

   EXPECT_TRUE(m_Sem.Wait());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrFAF,
                        ::testing::Values(1, 10, 100, 1000));

////////////////////////////////////////////////////////////////////////////////

// Functor for single Thread Group queuing.
class Numbered : public IDispatchable
{
public:
   Numbered(CSemaphore *psem, int x) :
      m_psem(psem),
      m_x(x)
   {}

   void operator() ()
   {
      m_psem->Post(1);
   }

protected:
  CSemaphore *m_psem;
  int         m_x;
};

// Value-parameterized test fixture
class ThrGrSingle : public ::testing::TestWithParam< int >
{
protected:
   ThrGrSingle() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~ThrGrSingle() {}

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   // virtual void SetUp() { }
   virtual void TearDown()
   {
      Destroy();

      YIELD_WHILE(CurrentThreads() > 0);

      disp_list_iter iter;
      for ( iter = m_WorkList.begin() ; iter != m_WorkList.end() ; ++iter ) {
         delete *iter;
      }
      m_WorkList.clear();

      m_Sem.Destroy();
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs/*=0*/,
                           OSLThread::ThreadPriority nPriority/*=OSLThread::THREADPRIORITY_NORMAL*/,
                           AAL::btTime               JoinTimeout/*=AAL_INFINITE_WAIT*/)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      m_JoinTimeout   = JoinTimeout;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority, m_JoinTimeout);
   }

   AAL::btBool Destroy()
   {
      if ( NULL == m_pGroup ) {
         return false;
      }
      delete m_pGroup;
      m_pGroup = NULL;
      return true;
   }

   AAL::btBool Add(IDispatchable *pDisp)
   {
      if ( NULL != pDisp ) {
         m_WorkList.push_back(pDisp);
      }
      return m_pGroup->Add(pDisp);
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   AAL::btTime               m_JoinTimeout;
   OSLThread::ThreadPriority m_ThrPriority;
   disp_list_t               m_WorkList;
   CSemaphore                m_Sem;
};

TEST_P(ThrGrSingle, DISABLED_Queuing)
{
   const int count = GetParam();

   OSLThreadGroup *g = Create(1, 0, OSLThread::THREADPRIORITY_NORMAL, AAL_INFINITE_WAIT);

   ASSERT_TRUE(m_Sem.Create(-count, INT_MAX));

   int i;
   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_TRUE( Add( new Numbered(&m_Sem, i) ));
   }

   EXPECT_TRUE(m_Sem.Wait());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrSingle,
                        ::testing::Values(1, 10, 100, 1000));

////////////////////////////////////////////////////////////////////////////////

// Functor for Multi-threaded Group queuing.
class Multi : public IDispatchable
{
public:
   Multi(CriticalSection &cs, volatile btUnsignedInt &counter) :
      m_CS(cs),
      m_Counter(counter)
   {}

   void operator() ()
   {
      AutoLock(&m_CS);
      ++m_Counter;
   }

protected:
   CriticalSection        &m_CS;
   volatile btUnsignedInt &m_Counter;
};

// Value-parameterized test fixture
class ThrGrMulti : public ::testing::TestWithParam< btUnsignedInt >
{
protected:
   ThrGrMulti() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~ThrGrMulti() {}

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   // virtual void SetUp() { }
   virtual void TearDown()
   {
      Destroy();

      YIELD_WHILE(CurrentThreads() > 0);

      disp_list_iter iter;
      for ( iter = m_WorkList.begin() ; iter != m_WorkList.end() ; ++iter ) {
         delete *iter;
      }
      m_WorkList.clear();
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs/*=0*/,
                           OSLThread::ThreadPriority nPriority/*=OSLThread::THREADPRIORITY_NORMAL*/,
                           AAL::btTime               JoinTimeout/*=AAL_INFINITE_WAIT*/)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      m_JoinTimeout   = JoinTimeout;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority, m_JoinTimeout);
   }

   AAL::btBool Destroy()
   {
      if ( NULL == m_pGroup ) {
         return false;
      }
      delete m_pGroup;
      m_pGroup = NULL;
      return true;
   }

   AAL::btBool Add(IDispatchable *pDisp)
   {
      if ( NULL != pDisp ) {
         m_WorkList.push_back(pDisp);
      }
      return m_pGroup->Add(pDisp);
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   AAL::btTime               m_JoinTimeout;
   OSLThread::ThreadPriority m_ThrPriority;
   disp_list_t               m_WorkList;
};

TEST_P(ThrGrMulti, DISABLED_SharedMemory)
{
   btUnsignedInt thrds = GetParam();

   OSLThreadGroup *g = Create(thrds, 0, OSLThread::THREADPRIORITY_NORMAL, AAL_INFINITE_WAIT);
   ASSERT_NONNULL(g);

   CriticalSection        cs;         // to synchronize accesses to counter
   volatile btUnsignedInt counter = 0;

   const btUnsignedInt loops = 1000;
   btUnsignedInt       i;
   for ( i = 0 ; i < loops ; ++i ) {
      ASSERT_TRUE( Add( new(std::nothrow) Multi(cs, counter) ) );
   }

   g->Drain();

   while ( counter != loops ) {
      SleepMilli(10);
   }

   EXPECT_EQ(counter, loops);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrMulti,
                        ::testing::Range<btUnsignedInt>(2, 10));

