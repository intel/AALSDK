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
   // virtual void SetUp() {}
   virtual void TearDown()
   {
      Destroy();
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
   AAL::btUnsignedInt CurrentThreads() const { return Config.CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   AAL::btTime               m_JoinTimeout;
   CSemaphore                m_Sems[4];
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

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }
   }
   virtual void TearDown()
   {
      Destroy();
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
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

   AAL::btBool Destroy()
   {
      if ( NULL == m_pGroup ) {
         return false;
      }
      delete m_pGroup;
      m_pGroup = NULL;
      return true;
   }

   AAL::btUnsignedInt CurrentThreads() const { return Config.CurrentThreads(); }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   AAL::btTime               m_JoinTimeout;
   OSLThread                *m_pThrs[5];
   CSemaphore                m_Sems[4];
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) )); \
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

   EXPECT_TRUE(Destroy());
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

   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(Destroy());
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

   EXPECT_FALSE(g->Add(NULL));

   EXPECT_TRUE(Destroy());
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

   EXPECT_TRUE(Destroy());
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
   EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));

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
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(counter) ));
      EXPECT_EQ(i + 1, g->GetNumWorkItems());
   }

   // none of the work items should have been dispatched.
   EXPECT_EQ(0, counter);

   // free up the worker thread, allowing it to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // wait for the worker to drain the queue.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(counter, (AAL::btInt)N);
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0080)
{
   // When constructed with bAutoJoin=true and no explicit call to OSLThreadGroup::Join(),
   // OSLThreadGroup::~OSLThreadGroup() joins all workers. (Running -> Joining)

   STAGE_WORKERS(GetParam());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Destroy the thread group now.
   EXPECT_TRUE(Destroy());
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

   EXPECT_TRUE(Destroy());
   ASSERT_EQ(0, CurrentThreads());
}

#if DEPRECATED
TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0082)
{
   // When constructed with bAutoJoin=false and no explicit call to OSLThreadGroup::Join(),
   // OSLThreadGroup::~OSLThreadGroup() detaches all worker threads, leaving the ThrGrpState
   // object to be destroyed by the last worker. (Running -> Detached)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              false);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // m_Sems[0,2] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[2].Create(-w, 1));
   // m_Sems[1] - count down sem, Wait()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitThenPostD(m_Sems[0],
                                                    m_Sems[1],
                                                    m_Sems[2]) ));
   }

   // Block until w counts have been Post()'ed to m_Sems[0].
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers are blocked on m_Sems[1], and the work queue is empty.

   // Destroy the OSLThreadGroup. This will detach all of the worker threads.
   EXPECT_TRUE(Destroy());

   // All workers are still blocked on m_Sems[1], and the thread queue remains empty.

   // Wake all of the worker threads from the Wait() on m_Sems[1]. Each will post m_Sems[2]
   // and then immediately exit, seeing that the state is Detached and that the work queue
   // is empty.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Block until w counts have been Post()'ed to m_Sems[2].
   EXPECT_TRUE(m_Sems[2].Wait());

   // All workers have executed the work items, but may not have exited just yet.
   YIELD_WHILE(CurrentThreads() > 0);
}
#endif // DEPRECATED

#if DEPRECATED
TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0083)
{
   // When constructed with bAutoJoin=false and an explicit call to OSLThreadGroup::Join()
   // is made, OSLThreadGroup::~OSLThreadGroup() destroys the ThrGrpState object. (Running -> Joining)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              false);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Explicitly join all workers.
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());

   EXPECT_TRUE(Destroy());
   ASSERT_EQ(0, CurrentThreads());
}
#endif // DEPRECATED

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

      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));

      EXPECT_TRUE(g->Start());
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(Destroy());
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
   ASSERT_EQ(0, g->GetNumWorkItems());
   delete pDisp;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // Start the thread group again.
   EXPECT_TRUE(g->Start());

   // The thread group should accept new work items now.
   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(Destroy());
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
   AAL::btInt x[4] = { 0, 0, 0, 0 };

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[1], w-1) ));
   EXPECT_TRUE(g->Add( new   UnsafeCountUpD(x[0]) ));
   EXPECT_TRUE(g->Add( new AddNopToThreadGroupD(g, 1, false) ));
   EXPECT_TRUE(g->Add( new UnsafeCountDownD(x[1]) ));
   EXPECT_TRUE(g->Add( new AddNopToThreadGroupD(g, 1, false) ));
   EXPECT_TRUE(g->Add( new   UnsafeCountUpD(x[2]) ));
   EXPECT_TRUE(g->Add( new AddNopToThreadGroupD(g, 1, false) ));
   EXPECT_TRUE(g->Add( new UnsafeCountDownD(x[3]) ));
   EXPECT_TRUE(g->Add( new AddNopToThreadGroupD(g, 1, false) ));
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));

   EXPECT_EQ(10, g->GetNumWorkItems());

   // Unblock one worker. That worker will sleep briefly, giving us a chance to call Drain.
   // We will go to sleep on Drain() below. When the first worker wakes, he will wake the rest of
   // the workers.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Wait for the Drain to complete.
   EXPECT_TRUE(g->Drain());
   EXPECT_EQ(1,  x[0]);
   EXPECT_EQ(-1, x[1]);
   EXPECT_EQ(1,  x[2]);
   EXPECT_EQ(-1, x[3]);
   EXPECT_EQ(0, g->GetNumWorkItems());

   // The thread group should remain in the Running state, able to accept work.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));

   // Block until this newly-added work item executes, proving that the group is still Running.
   EXPECT_TRUE(m_Sems[0].Wait());

   ASSERT_EQ(m_MinThreads, CurrentThreads());
   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
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
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(i) ));

   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(y) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   g->Stop();
   EXPECT_LT(0, y);
   EXPECT_EQ(0, g->GetNumWorkItems());

   EXPECT_TRUE(g->Drain());

   // Start the thread group again.
   EXPECT_TRUE(g->Start());

   // The thread group should accept new work items now.
   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   g->Stop();
   EXPECT_LT(0, x);
   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(m_MinThreads, CurrentThreads());

   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
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

   EXPECT_TRUE(Destroy());
}

#if DEPRECATED
TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0091)
{
   // When constructed with bAutoJoin=false and when the thread group is Stopped,
   // OSLThreadGroup::~OSLThreadGroup() detaches all workers. (Stopped -> Detached)

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              false);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;
   AAL::btInt i;
   for ( i = 0 ; i < 1000 ; ++i ) {
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      if ( 0 == (i % 100) ) {
         cpu_yield();
      }
   }

   g->Stop();
   EXPECT_LT(0, x);
   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(m_MinThreads, CurrentThreads());

   EXPECT_TRUE(Destroy());

   // Not joining, so need to wait for the detached threads to exit.
   YIELD_WHILE(CurrentThreads() > 0);
}
#endif // DEPRECATED

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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All of the thread group workers are in dispatch and will block on m_Sems[1].
   // Adding more work to the thread group will back up in the queue.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[1], w-1) ));

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
               EXPECT_TRUE(g->Add( new UnsafeCountUpD(x[xCount]) ));
               ++xCount;
            } else {
               EXPECT_TRUE(g->Add( new YieldD() ));
            }
         } break;

         case 1 : {
            if ( yCount < sizeof(y) / sizeof(y[0]) ) {
               EXPECT_TRUE(g->Add( new UnsafeCountDownD(y[yCount]) ));
               ++yCount;
            } else {
               EXPECT_TRUE(g->Add( new YieldD() ));
            }
         } break;

         case 2 : { // Add() during Drain() is invalid.
            EXPECT_TRUE(g->Add( new AddNopToThreadGroupD(g, 1, false) ));
         } break;

         case 3 : { // Posts to m_Sems[3] will wake the Thr0 threads to perform nested Drain()'s.
            if ( t < ThrCount ) {
               EXPECT_TRUE(g->Add( new PostD(m_Sems[3]) ));
               ++t;
            } else {
               EXPECT_TRUE(g->Add( new YieldD() ));
            }
         } break;

         default : {
            EXPECT_TRUE(g->Add( new NopD() ));
         } break;
      }

   }

   // Be sure to wake all of the Thr0's.
   while ( t < ThrCount ) {
      EXPECT_TRUE(g->Add( new PostD(m_Sems[3]) ));
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
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(Destroy());
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

   // wake the main thread.
   EXPECT_TRUE(pTC->m_Sems[0].Post(1));

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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   AAL::btInt x = 0;

   for ( i = 0 ; i < 500 ; ++i ) {
      if ( 250 == i ) {
         EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   EXPECT_EQ(500, g->GetNumWorkItems());

   // Wake Thr1, then block on m_Sems[0].
   // Thr1 Post()'s m_Sems[0]. If Thr1 is preempted after Post()'ing m_Sems[0], but before
   //  invoking the Drain(), this thread yield's the cpu back to Thr1.
   // Thr1 invokes the Drain().
   // This thread wakes and resumes all of the thread group workers by Post()'ing m_Sems[1].
   // This thread then calls Destroy() to invoke ~OSLThreadGroup().
   EXPECT_TRUE(m_Sems[3].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());
   cpu_yield();

   // Wake all workers. The Drain() progresses.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Delete the thread group - destruct during Drain().
   EXPECT_TRUE(Destroy());

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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new PostD(m_Sems[3]) )); // Wakes Thr2 to invoke the Drain().
   EXPECT_TRUE(g->Add( new YieldD() ));         // Yield the worker, giving the cpu to Thr2.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) )); // Wakes this thread to do the Join().
   EXPECT_TRUE(g->Add( new YieldD() ));         // Yield the worker, giving the cpu to this thread.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );

   for ( i = 0 ; i < 494 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr2 to do the Drain(), then wake us to perform the
   // Join(), then wake the remaining workers to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());

   // Join the thread group.
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr2.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   EXPECT_TRUE(Destroy());
   ASSERT_EQ(0, CurrentThreads());
}

#if DEPRECATED
void OSAL_ThreadGroup_vp_uint_0::Thr3(OSLThread *pThread, void *pContext)
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

TEST_P(OSAL_ThreadGroup_vp_uint_0, DISABLED_aal0095)
{
   // When constructed with bAutoJoin=false and when the thread group is Draining, deleting
   // the thread group will detach the ThrGrpState object. (Draining -> Detached) The Drain()
   // will return true to indicate success.

   // Use Thr3 to perform the Drain(), this thread to do the delete.

   ASSERT_EQ(0, CurrentThreads());

   // m_Sems[2] - Thr3 is ready.
   // m_Sems[3] - Thr3 waits for a signal to do the drain.
   EXPECT_TRUE(m_Sems[2].Create(0, 1));
   EXPECT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_0::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // Wait until Thr3 is ready.
   ASSERT_TRUE(m_Sems[2].Wait());
   ASSERT_EQ(1, CurrentThreads());

   // Thr3 is blocked waiting for m_Sems[3]

   OSLThreadGroup *g = Create(GetParam(),
                              1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              false);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(1 + m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new PostD(m_Sems[3]) )); // Wakes Thr3 to invoke the Drain().
   EXPECT_TRUE(g->Add( new YieldD() ));         // Yield the worker, giving the cpu to Thr3.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) )); // Wakes this thread to do the delete.
   EXPECT_TRUE(g->Add( new YieldD() ));         // Yield the worker, giving the cpu to this thread.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );

   for ( i = 0 ; i < 494 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr3 to do the Drain(), then wake us to perform the
   // delete, then wake the remaining workers to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());

   // Delete the thread group, detaching all of the threads.
   EXPECT_TRUE(Destroy());

   // Join Thr3.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}
#endif // DEPRECATED

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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Wakes Thr4 to attempt the Start().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // The single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr4 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr4 to sleep then attempt the Start().
   // Meanwhile, this thread will be in the destructor, doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(Destroy());

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

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr5 to attempt the Start().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr5 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr5 to sleep then attempt the Start().
   // Meanwhile, this thread will be doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr5.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr6 to attempt the Stop().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr6 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr6 to sleep then attempt the Start().
   // Meanwhile, this thread will be in the destructor, doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(Destroy());

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

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr7 to attempt the Stop().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr7 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr7 to sleep then attempt the Stop().
   // Meanwhile, this thread will be doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr7.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr8 to attempt the Drain().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr8 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr8 to sleep then attempt the Drain().
   // Meanwhile, this thread will be in the destructor, doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(Destroy());

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

   EXPECT_FALSE(pTC->m_pGroup->Drain());

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr9 to attempt the Drain().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr9 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr9 to sleep then attempt the Drain().
   // Meanwhile, this thread will be doing the Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr9.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(Destroy());
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr10 to attempt the 2nd Join().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr10 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr10 to sleep then attempt the 2nd Join().
   // Meanwhile, this thread will be in the destructor, doing the 1st Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Delete the thread group, invoking Join().
   EXPECT_TRUE(Destroy());

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

   // signal that we're done.
   EXPECT_TRUE(pTC->m_Sems[2].Post(1));
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
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // All workers will be blocking on m_Sems[1]. Any new work items will queue up.

   EXPECT_TRUE(g->Add( new SleepThenPostD(100, m_Sems[3]) )); // Single worker wakes Thr11 to attempt the 2nd Join().
   EXPECT_TRUE(g->Add( new PostD(m_Sems[1], w - 1)) );        // Single worker wakes the remaining workers.
   EXPECT_TRUE(g->Add( new WaitD(m_Sems[2]) ));               // Some worker waits for Thr11 to exit.

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   EXPECT_EQ(4, g->GetNumWorkItems());

   // Wake the first worker, which will wake Thr11 to sleep then attempt the 2nd Join().
   // Meanwhile, this thread will be doing the 1st Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // Join Thr11.
   m_pThrs[0]->Join();

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);

   // Delete the thread group.
   EXPECT_TRUE(Destroy());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_0,
                           ::testing::Values((AAL::btUnsignedInt)1,
                                             (AAL::btUnsignedInt)5,
                                             (AAL::btUnsignedInt)10,
                                             (AAL::btUnsignedInt)25,
                                             (AAL::btUnsignedInt)50));



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



// Special case - when created with 0 threads, a Thread Group creates GetNumProcessors()'s threads.
TEST(ThrGr, DISABLED_ZeroThreads) {
   OSLThreadGroup tg(0, 0);
   // EXPECT_EQ(GetNumProcessors(), tg.GetNumThreads());
   EXPECT_EQ(0, tg.GetNumWorkItems());
   EXPECT_FALSE(tg.Start()) << "Thread groups are created in the Running state";
}

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
      delete this;
   }

protected:
  CSemaphore *m_psem;
  int         m_x;
};

// Value-parameterized test fixture
class ThrGrSingle : public ::testing::TestWithParam< int >
{
protected:
ThrGrSingle() {}
// virtual void SetUp() { }
// virtual void TearDown() { }

   CSemaphore     m_Sem;
   OSLThreadGroup m_ThrGrp;
};

TEST_P(ThrGrSingle, DISABLED_Queuing)
{
   const int count = GetParam();

   ASSERT_TRUE(m_Sem.Create(-count, INT_MAX));

   int i;
   for ( i = 0 ; i < count ; ++i ) {
      m_ThrGrp.Add( new Numbered(&m_Sem, i) );
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
      m_CS.Lock();
      ++m_Counter;
      m_CS.Unlock();
      delete this;
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
   m_pThrGrp(NULL)
{}

// virtual void SetUp() { }
virtual void TearDown()
{
   if ( NULL != m_pThrGrp ) {
      delete m_pThrGrp;
      m_pThrGrp = NULL;
   }
}

   OSLThreadGroup *m_pThrGrp;
};

TEST_P(ThrGrMulti, DISABLED_SharedMemory)
{
   btUnsignedInt thrds = GetParam();

   m_pThrGrp = new(std::nothrow) OSLThreadGroup(thrds);
   ASSERT_NONNULL(m_pThrGrp);

   CriticalSection        cs;         // to synchronize accesses to counter
   volatile btUnsignedInt counter = 0;

   const btUnsignedInt loops = 1000;
   btUnsignedInt       i;
   for ( i = 0 ; i < loops ; ++i ) {
      ASSERT_TRUE( m_pThrGrp->Add( new(std::nothrow) Multi(cs, counter) ) );
   }

   m_pThrGrp->Drain();

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

