// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtThreadGroup.h"

// Value-parameterized test fixture
template <typename T>
class OSAL_ThreadGroupSR_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_ThreadGroupSR_vp() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~OSAL_ThreadGroupSR_vp() {}

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


class OSAL_ThreadGroup_vp_uint_1 : public OSAL_ThreadGroupSR_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0104)
{
   // Self-referential Drain() calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 24 == i ) {
         EXPECT_TRUE(g->Add( new DrainThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(g->GetNumWorkItems() > 0);

   EXPECT_EQ(1, x);

   EXPECT_TRUE(Destroy());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0105)
{
   // Self-referential Join() calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 24 == i ) {
         EXPECT_TRUE(g->Add( new JoinThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_EQ(1, x);

   EXPECT_TRUE(Destroy());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0106)
{
   // Self-referential Destroy() calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 24 == i ) {
         EXPECT_TRUE(g->Add( new DeleteThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(g->Add( new SetThreadGroupPtrToNULLD(m_pGroup) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(NULL != m_pGroup);
   YIELD_WHILE(CurrentThreads() > 0);
}



TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0111)
{
   // Self-referential Drain() calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      EXPECT_TRUE(g->Add( new DrainThreadGroupD(g) ));
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(g->GetNumWorkItems() > 0);

   EXPECT_TRUE(Destroy());
}





TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0109)
{
   // Race condition between Join() in the work queue(self-referential Join()) and Join()
   // from outside the work queue, when Join() from outside the work queue wins the race.
   // The Join() from outside the work queue will complete successfully. The self-referential
   // Join() will return false, indicating failure.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new SleepThenPostThenJoinThreadGroupD(100,               // SleepMilli()
                                                             m_Sems[1],         // CSemaphore to Post()
                                                             w-1,               // Value to Post
                                                             g,                 // OSLThreadGroup
                                                             true,              // Expected Post() result
                                                             AAL_INFINITE_WAIT, // Join() timeout
                                                             false)             // Expected Join() result
              ));

   for ( i = 0 ; i < 498 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker. It will sleep briefly, then wake the remaining workers, then attempt
   // to Join() the thread group. Meanwhile, we will have invoked ~OSLThreadGroup(), which will do
   // a Join(). The Join() invoked from ~OSLThreadGroup() will win.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(Destroy());

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0110)
{
   // Race condition between Join() in the work queue(self-referential Join()) and Join()
   // from outside the work queue, when the self-referential Join() wins the race. The Join()
   // from outside the work queue will return false immediately, indicating failure, not
   // waiting for workers to exit. The self-referential Join() will wait for all workers to
   // complete, then return true, indicating success.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new PostThenJoinThreadGroupD(m_Sems[0],         // CSemaphore to Post() (yes, m_Sems[0])
                                                    1,                 // Value to Post        (yes, 1)
                                                    g,                 // OSLThreadGroup
                                                    true,              // Expected Post() result
                                                    AAL_INFINITE_WAIT, // Join() timeout
                                                    true)              // Expected Join() result
              ));

   for ( i = 0 ; i < 498 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   AAL::btInt x = 0;
   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker. It will pulse m_Sems[0], waking us. Then the worker will immediately
   // invoke the self-referential Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Wait for the first worker to wake us.
   EXPECT_TRUE(m_Sems[0].Wait());
   // In case the worker was preempted after posting m_Sems[0] but before calling Join().
   cpu_yield();

   // Now, wake the remaining workers so that the self-referential Join() can complete.
   EXPECT_TRUE(m_Sems[1].Post(w-1));
   cpu_yield();

   // This Join() should fail.
   EXPECT_FALSE(g->Join(AAL_INFINITE_WAIT));

   EXPECT_TRUE(Destroy());

   // All workers must have exited before this Join() returns.
   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0125)
{
   // Race condition between ~OSLThreadGroup() and Join() both executed from within the
   // work queue, when Join() wins the race. The Join() will complete successfully.
   // ~OSLThreadGroup waits for the Join() to complete before destroying the thread group.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new PostThenJoinThreadGroupD(m_Sems[0],
                                                    1,
                                                    g) ));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 497 ; ++i ) {
      if ( 200 == i ) {
         EXPECT_TRUE(g->Add( new DeleteThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
   EXPECT_TRUE(g->Add( new SetThreadGroupPtrToNULLD(m_pGroup) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will wake us (we're going to block on m_Sems[0], below).
   // If we happen to get a time slice before the first worker calls Join(), we
   //  execute a cpu_yield() to give the cpu back to the worker.
   // The worker calls Join().
   // When this thread wakes, we wake the remaining workers.
   // In the course of Join()'ing the thread group, one of the thread group workers
   //  executes the work item that delete's the thread group.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());
   cpu_yield();

   EXPECT_TRUE(m_Sems[1].Post(w-1));
   cpu_yield();

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);

   // Because Destroy() will attempt to delete m_pGroup when non-NULL.
   YIELD_WHILE(NULL != m_pGroup);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, DISABLED_aal0127)
{
   // Race condition between Join() in the work queue(self-referential Join()) and
   // ~OSLThreadGroup() from outside the work queue, when the self-referential Join()
   // wins the race. The ~OSLThreadGroup() from outside the work queue will not destroy
   // the thread group until all worker threads have exited. The self-referential Join()
   // will return true, indicating success.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new PostThenJoinThreadGroupD(m_Sems[0],         // CSemaphore to Post() (yes m_Sems[0])
                                                    1,                 // Value to Post
                                                    g,                 // OSLThreadGroup
                                                    true,              // Expected Post() result
                                                    AAL_INFINITE_WAIT, // Join() timeout
                                                    true)              // Expected Join() result
              ));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 499 ; ++i ) {
      if ( 250 == i ) {
         EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(g->Add( new YieldD() ));
      }
   }

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker. It will pulse m_Sems[0], waking us. Then the worker will immediately
   // invoke the self-referential Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Wait for a worker to wake us.
   EXPECT_TRUE(m_Sems[0].Wait());

   // Now, wake the remaining workers so that the self-referential Join() can complete.
   EXPECT_TRUE(m_Sems[1].Post(w-1));

   // Make sure that we don't get into Destroy() before the first worker gets into Join().
   YIELD_WHILE(0 == x);

   EXPECT_TRUE(Destroy());

   ASSERT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, DISABLED_aal0128)
{
   // Race condition between Join() in the work queue(self-referential Join()) and
   // ~OSLThreadGroup() from outside the work queue, when ~OSLThreadGroup() wins the race.
   // The self-referential Join() fails, returning false. ~OSLThreadGroup waits for all workers
   // to complete prior to destroying the thread group.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new YieldD() ));
   EXPECT_TRUE(g->Add( new SleepThenPostThenJoinThreadGroupD(100,
                                                             m_Sems[1],         // CSemaphore to Post()
                                                             w-1,               // Value to Post
                                                             g,                 // OSLThreadGroup
                                                             true,              // Expected Post() result
                                                             AAL_INFINITE_WAIT, // Join() timeout
                                                             false)             // Expected Join() result
              ));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 497 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker.
   // If we get preempted just after waking the first worker, then we will immediately get the
   //  cpu back, as the first work item is a yield.
   // Invoke Destroy(), destructing the thread group.
   // When the first worker wakes, it will wake the remaining workers from sleep on m_Sems[1].
   // The first worker will then attempt a Join(), which will fail, because this thread is already
   //  in the destructor.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(Destroy());
   ASSERT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, DISABLED_aal0129)
{
   // Race condition between ~OSLThreadGroup() in the work queue(self-referential Destroy()) and
   // Join() from outside the work queue, when Join() wins the race. The self-referential Destroy()
   // waits until the Join() completes before destroying the thread group. Join() return true,
   // indicating success.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new YieldD() ));
   EXPECT_TRUE(g->Add( new SleepThenPostThenDeleteThreadGroupD(100,
                                                               m_Sems[1],
                                                               w-1,
                                                               m_pGroup) ));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 496 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
   EXPECT_TRUE(g->Add( new SetThreadGroupPtrToNULLD(m_pGroup) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker.
   // If we happen to get preempted just after waking the worker, we'll get the cpu back, as the
   //  first work item is a yield.
   // This thread immediately calls Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   // We can't touch g any longer. It has been deleted.

   // Because the worker executing the delete became detached.
   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_EQ(1, x);

   // Because Destroy() will attempt to delete m_pGroup when non-NULL.
   YIELD_WHILE(NULL != m_pGroup);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, DISABLED_aal0130)
{
   // Race condition between ~OSLThreadGroup() in the work queue(self-referential Destroy())
   // and Join() from outside the work queue, when ~OSLThreadGroup() wins the race. The Join()
   // call return false, indicating failure.

   // * There is danger of de-referencing a pointer to the deleted OSLThreadGroup object.
   //   The Join() call is not coming from a thread within the thread group, so ~OSLThreadGroup()
   //   will not(cannot) synchronize with that thread.

   STAGE_WORKERS(GetParam());

   EXPECT_TRUE(g->Add( new PostThenDeleteThreadGroupD(m_Sems[0],
                                                      1,
                                                      m_pGroup) ));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 497 ; ++i ) {
      EXPECT_TRUE(g->Add( new YieldD() ));
   }

   EXPECT_TRUE(g->Add( new UnsafeCountUpD(x) ));
   EXPECT_TRUE(g->Add( new SetThreadGroupPtrToNULLD(m_pGroup) ));

   ASSERT_EQ(500, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will wake us (we're going to block on m_Sems[0], below).
   // If we happen to get a time slice before the first worker deletes the thread group, we
   //  execute a cpu_yield() to give the cpu back to the worker.
   // The worker delete's the thread group.
   // When this thread wakes, we wake the remaining workers, then invoke the Join(), which
   //  fails because the destruct is already in progress by the first worker.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());
   cpu_yield();

   EXPECT_TRUE(m_Sems[1].Post(w-1));
   cpu_yield();

   EXPECT_FALSE(g->Join(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);

   // Because Destroy() will attempt to delete m_pGroup when non-NULL.
   YIELD_WHILE(NULL != m_pGroup);
}



INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_1,
                           ::testing::Range((AAL::btUnsignedInt)1, (AAL::btUnsignedInt)11));

