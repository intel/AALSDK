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

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }
      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
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
   AAL::btUnsignedInt        m_Scratch[4];
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
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(g->GetNumWorkItems() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0105)
{
   // Self-referential Join() calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 24 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0106)
{
   // Self-referential Destroy() calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 24 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0107)
{
   // Nested Drain(), one external, one self-referential, when the external Drain()
   // is encountered first. The Drain()'s nest, and both complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new SleepThenPostD(100, m_Sems[1], w-1) ));
      } else if ( 24 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker. The first worker sleeps, then wakes the remaining workers.
   // Meanwhile, this thread begins the external Drain().
   // Some worker encounters the self-referential Drain().
   // Both Drain()'s complete successfully.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Drain());

   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0108)
{
   // Nested Drain(), one external, one self-referential, when the self-referential Drain()
   // is encountered first. The Drain()'s nest, and both complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker then sleep on m_Sems[0].
   // The first worker invokes the self-referential Drain(), then wakes this thread from sleep
   //  on m_Sems[0].
   // Once awake, this thread wakes the remaining workers, then invokes the external Drain().
   // Both Drain()'s complete successfully.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(m_Sems[0].Wait());
   EXPECT_TRUE(m_Sems[1].Post(w-1));

   EXPECT_TRUE(g->Drain());

   EXPECT_EQ(0, g->GetNumWorkItems());
   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0109)
{
   // Race condition between Join() in the work queue(self-referential Join()) and Join()
   // from outside the work queue, when Join() from outside the work queue wins the race.
   // The Join() from outside the work queue will complete successfully. The self-referential
   // Join() will return false, indicating failure.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 12 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 48 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker. It will sleep briefly, then wake the remaining workers, then attempt
   // to Join() the thread group. Meanwhile, we will have invoked ~OSLThreadGroup(), which will do
   // a Join(). The Join() invoked from ~OSLThreadGroup() will win.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
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

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 12 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1)));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker. It will pulse m_Sems[0], waking us. Then the worker will immediately
   // invoke the self-referential Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   // Wait for the first worker to wake us.
   EXPECT_TRUE(m_Sems[0].Wait());

   // This Join() should fail.
   EXPECT_FALSE(g->Join(AAL_INFINITE_WAIT));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0111)
{
   // Multiple self-referential Drain() calls are allowed to nest, all completing successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[1].Post(w));

   YIELD_WHILE(g->GetNumWorkItems() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0112)
{
   // When multiple self-referential Join() calls are encountered, only the first waits
   // for all threads and returns true indicating success. The other self-referential Join()'s
   // return false immediately, indicating failure.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the first self-referential Join(), then wake the remaining workers.
   // The workers will continue to execute further self-referential Join()'s, which should all fail.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0113)
{
   // When a self-referential Join() is encountered during a self-referential Drain(),
   // both calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the self-referential Drain(), then wake the remaining workers.
   // Some worker will execute the self-referential Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0114)
{
   // When a self-referential Drain() is encountered during a self-referential Join(), the
   // Drain() call returns false immediately, indicating failure. The Join() completes
   // successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the self-referential Join(), then wake the remaining workers.
   // Some worker will execute the self-referential Drain(), which will fail.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0115)
{
   // When an external Join() is executed during a self-referential Drain(), both
   // calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker begins the self-referential Drain(), wakes this thread, then blocks again
   //  on m_Sems[1].
   // This thread resumes, wakes all of the workers, then begins the external Join().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0116)
{
   // When a self-referential Drain() is encountered during an external Join(), the Drain()
   // returns false immediately, indicating failure. The Join() completes successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 20 == i ) {
         EXPECT_TRUE(Add( new UncheckedDrainThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then begin the external Join().
   // The first worker wakes the remaining workers.
   // Some worker attempts a Drain(), which fails.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   EXPECT_EQ(1, x);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0117)
{
   // When a self-referential Destroy() is encountered during a self-referential Drain(),
   // both calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the self-referential Drain(), then wake the remaining workers.
   // Some worker invokes the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0118)
{
   // When a self-referential Drain() is encountered during a self-referential Destroy(),
   // the Drain() will return false immediately, indicating failure.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the self-referential Destroy(), then wake the remaining workers.
   // Some worker will attempt a Drain(), which will fail.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0119)
{
   // When an external Destroy() occurs during a self-referential Drain(), the Destroy() waits
   // for the Drain() to complete. Both calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Drain(), wakes us, then wakes the remaining workers.
   // When this thread resumes, we invoke the external Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0120)
{
   // When a self-referential Drain() is encountered during an external Destroy(), the Drain()
   // fails immediately, returning false. The Destroy() completes successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 20 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 30 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then invoke the external Destroy().
   // The first worker wakes the remaining workers.
   // Some worker attempts a self-referential Drain(), which fails.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0121)
{
   // When a self-referential Join() is encountered during an external Drain(),
   // the Join() waits for the Drain() to complete. Both calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 20 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then invoke the external Drain().
   // The first worker wakes the remaining workers.
   // Some worker executes the self-referential Join().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Drain());

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0122)
{
   // When a external Drain() is attempted during a self-referential Join(), the Drain()
   // returns false immediately, indicating failure. The self-referential Join()
   // completes successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker wakes and begins the self-referential Join().
   // During processing of the Join(), the first worker wakes the remaining workers.
   // Some worker wakes this thread. This thread attempts a Drain(), which fails.
   // Some worker blocks on m_Sems[1], ensuring that this thread attempts the Drain()
   //  before the thread group is Join()'ed.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_FALSE(g->Drain());
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0123)
{
   // When a self-referential Destroy() is encountered during an external Drain(), the
   // Destroy() waits for the Drain() to complete. Both calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 100 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 11 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 22 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 50 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(100, g->GetNumWorkItems());

   // Wake the first worker, then invoke the external Drain().
   // The first worker wakes the remaining workers.
   // Some worker executes the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(g->Drain());

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0124)
{
   // When an external Drain() is attempted during a self-referential Destroy(), the Drain()
   // returns false immediately, indicating failure. The self-referential Destroy() completes
   // successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker wakes and begins the self-referential Destroy().
   // During processing of the Destroy(), the first worker wakes the remaining workers.
   // Some worker wakes this thread. This thread attempts a Drain(), which fails.
   // Some worker blocks on m_Sems[1], ensuring that this thread attempts the Drain()
   //  before the thread group is Destroyed()'ed.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_FALSE(g->Drain());
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0125)
{
   // Race condition between ~OSLThreadGroup() and Join() both executed from within the
   // work queue, when Join() wins the race. The Join() will complete successfully.
   // ~OSLThreadGroup waits for the Join() to complete before destroying the thread group.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 8 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will wake us (we're going to block on m_Sems[0], below).
   // If we happen to get a time slice before the first worker calls Join(), we
   //  execute a cpu_yield() to give the cpu back to the worker.
   // The worker calls Join().
   // When this thread wakes, we wake the remaining workers.
   // In the course of Join()'ing the thread group, one of the thread group workers
   //  executes the work item that delete's the thread group.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0126)
{
   // When a self-referential Join() is encountered during a self-referential Destroy(),
   // the Join() returns false immediately, indicating failure. The self-referential Destroy()
   // completes successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker begins the self-referential Destroy(), then wakes the remaining workers.
   // Some worker attempts the self-referential Join(), which fails.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0127)
{
   // Race condition between Join() in the work queue(self-referential Join()) and
   // ~OSLThreadGroup() from outside the work queue, when the self-referential Join()
   // wins the race. The ~OSLThreadGroup() from outside the work queue will not destroy
   // the thread group until all worker threads have exited. The self-referential Join()
   // will return true, indicating success.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker begins the self-referential Join(), then wakes this thread, then wakes
   //  the remaining workers.
   // This thread begins the external Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0128)
{
   // Race condition between Join() in the work queue(self-referential Join()) and
   // ~OSLThreadGroup() from outside the work queue, when ~OSLThreadGroup() wins the race.
   // The self-referential Join() fails, returning false. ~OSLThreadGroup waits for all workers
   // to complete prior to destroying the thread group.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 24 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // If we get preempted just after waking the first worker, then we will immediately get the
   //  cpu back, as the first work item is a yield.
   // Invoke Destroy(), destructing the thread group.
   // When the first worker wakes, it will wake the remaining workers from sleep on m_Sems[1].
   // Some worker will attempt a self-referential Join(), which will fail, because this thread
   //  is already in the destructor.
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   ASSERT_EQ(0, CurrentThreads());
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0129)
{
   // Race condition between ~OSLThreadGroup() in the work queue(self-referential Destroy()) and
   // Join() from outside the work queue, when Join() wins the race. The self-referential Destroy()
   // waits until the Join() completes before destroying the thread group. Join() return true,
   // indicating success.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 24 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then begin the external Join().
   // If the first worker gets a timeslice before this thread calls Join(), the first few
   //  work items should yield the cpu back to this thread.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0130)
{
   // Race condition between ~OSLThreadGroup() in the work queue(self-referential Destroy())
   // and Join() from outside the work queue, when ~OSLThreadGroup() wins the race. The Join()
   // call return false, indicating failure.

   // * There is danger of de-referencing a pointer to the deleted OSLThreadGroup object.
   //   The Join() call is not coming from a thread within the thread group, so ~OSLThreadGroup()
   //   will not(cannot) synchronize with that thread.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) )); // blocks the Destroy() until the Join has returned false.
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Destroy(), wakes this worker, then wakes
   //  the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_FALSE(g->Join(AAL_INFINITE_WAIT));
   EXPECT_TRUE(m_Sems[1].Post(1)); // Allow the Destroy to complete.

   YIELD_WHILE(CurrentThreads() > 0);
}


INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_1,
                           ::testing::Range((AAL::btUnsignedInt)1, (AAL::btUnsignedInt)11));


class OSAL_ThreadGroup_vp_uint_2 : public OSAL_ThreadGroupSR_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );
   static void Thr5(OSLThread * , void * );
   static void Thr6(OSLThread * , void * );
};

void OSAL_ThreadGroup_vp_uint_2::Thr0(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->m_Sems[2].Wait());
   f->m_Scratch[2] = 1;
   EXPECT_TRUE(f->m_pGroup->Drain());
}

void OSAL_ThreadGroup_vp_uint_2::Thr1(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[1] = 1;
   EXPECT_TRUE(f->m_Sems[3].Wait());
   f->m_Scratch[3] = 1;
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0140)
{
   // When an external Drain(), Join(), and Destroy() overlap in that order,
   //  the later calls wait for the earlier to complete. All calls complete successfully.

   // Thr0 calls Drain(), Thr1 calls Join(), and this thread does the Destroy().

   STAGE_WORKERS(GetParam());

   ASSERT_TRUE(m_Sems[2].Create(0, 1));
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   m_pThrs[1] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[1]->IsOK());
   YIELD_WHILE(0 == m_Scratch[1]);

   EXPECT_EQ(2 + m_MinThreads, CurrentThreads());


   AAL::btInt x = 0;

   for ( i = 0 ; i < 150 ; ++i ) {
      if ( 30 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 149 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(150, g->GetNumWorkItems());

   // Wake Thr0 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[2]);
   YIELD_X(5);

   // Wake Thr1 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[3]);
   YIELD_X(10);

   // Wake the first worker. The first worker will wake the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));
   YIELD_X(5);

   // This thread calls Destroy()
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();

   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_ThreadGroup_vp_uint_2::Thr2(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->m_Sems[2].Wait());
   f->m_Scratch[2] = 1;
   EXPECT_TRUE(f->m_pGroup->Drain());
}

void OSAL_ThreadGroup_vp_uint_2::Thr3(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[1] = 1;
   EXPECT_TRUE(f->m_Sems[3].Wait());
   f->m_Scratch[3] = 1;
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0141)
{
   // When an external Drain(), an external Join(), and a self-referential Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete. All
   //  calls complete successfully.

   // Thr2 calls Drain(), Thr3 calls Join().

   STAGE_WORKERS(GetParam());

   ASSERT_TRUE(m_Sems[2].Create(0, 1));
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   m_pThrs[1] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[1]->IsOK());
   YIELD_WHILE(0 == m_Scratch[1]);

   EXPECT_EQ(2 + m_MinThreads, CurrentThreads());


   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr2 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[2]);
   YIELD_X(5);

   // Wake Thr3 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[3]);
   YIELD_X(5);

   // Wake the first worker. The first worker will wake the remaining workers.
   // Some worker does the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroup_vp_uint_2::Thr4(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->m_Sems[2].Wait());
   f->m_Scratch[2] = 1;
   EXPECT_TRUE(f->m_pGroup->Drain());
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0142)
{
   // When an external Drain(), a self-referential Join(), and an external Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete. All
   //  calls complete successfully.

   // Thr4 does the Drain().

   STAGE_WORKERS(GetParam());

   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());


   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr4 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[2]);
   YIELD_X(5);

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Join(), Post()'s m_Sems[0] to wake this thread,
   //  then wakes the remaining workers.
   // This thread resumes and does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   m_pThrs[0]->Join();

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroup_vp_uint_2::Thr5(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->m_Sems[2].Wait());
   f->m_Scratch[2] = 1;
   EXPECT_TRUE(f->m_pGroup->Drain());
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0143)
{
   // When an external Drain(), a self-referential Join(), and a self-referential Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   // Thr5 does the Drain().

   STAGE_WORKERS(GetParam());

   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr5 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[2]);
   YIELD_X(5);

   // Wake the first worker.
   // The first worker invokes the self-referential Join(), then wakes the remaining workers.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   m_pThrs[0]->Join();

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroup_vp_uint_2::Thr6(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroup_vp_uint_2 *f = static_cast<OSAL_ThreadGroup_vp_uint_2 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->m_Sems[2].Wait());
   f->m_Scratch[2] = 1;
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0144)
{
   // When an self-referential Drain(), an external Join(), and an external Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   // Use Thr6 to do the Join().

   STAGE_WORKERS(GetParam());

   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());


   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[2]) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * Post()'s m_Sems[2], waking Thr6, then yield's to Thr6.
   //  * Post()'s m_Sems[0], waking this thread, then yields to this thread.
   //  * wakes the remaining workers.
   // When this thread wakes, we yield to Thr6 so that Thr6 invokes Join().
   // This thread then does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   YIELD_WHILE(0 == m_Scratch[2]);
   YIELD_X(5);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, x);
   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0145)
{
   // When a self-referential Drain(), an external Join(), and a self-referential Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * Post()'s m_Sems[0] to wake this thread.
   //  * blocks on m_Sems[1].
   // This thread wakes, Post()'s m_Sems[1] to wake a worker, then begins the Join().
   // If the resumed worker gets a timeslice before this thread calls Join(), it executes a
   //  yield, giving the cpu back to this thread.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0146)
{
   // When a self-referential Drain(), a self-referential Join(), and an external Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   STAGE_WORKERS(GetParam());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * invokes the self-referential Join().
   //  * Post()'s m_Sems[0], waking this thread.
   //  * yeilds the cpu to this thread.
   // This thread resumes and begins the Destroy().
   // The thread group worker is time sliced and wakes the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0147)
{
   // When a self-referential Drain(), a self-referential Join(), and a self-referential Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   STAGE_WORKERS(GetParam());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 6 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * invokes the self-referential Join().
   //  * wakes the remaining workers.
   // Some worker performs the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_2,
                           ::testing::Range((AAL::btUnsignedInt)1, (AAL::btUnsignedInt)11));



#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_ThreadGroupSR_vp_tuple_0 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btUnsignedInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_ThreadGroupSR_vp_tuple_0() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL),
      m_JoinTimeout(AAL_INFINITE_WAIT)
   {}
   virtual ~OSAL_ThreadGroupSR_vp_tuple_0() {}

   typedef std::list<IDispatchable *> disp_list_t;
   typedef disp_list_t::iterator      disp_list_iter;

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }
      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
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
   OSLThread                *m_pThrs[16];
   CSemaphore                m_Sems[4];
   AAL::btUnsignedInt        m_Scratch[16];
   disp_list_t               m_WorkList;

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

   static void Thr12(OSLThread * , void * );

   static void Thr13(OSLThread * , void * );
   static void Thr14(OSLThread * , void * );

   static void Thr15(OSLThread * , void * );
   static void Thr16(OSLThread * , void * );

   static void Thr17(OSLThread * , void * );

   static void Thr18(OSLThread * , void * );
};

void OSAL_ThreadGroupSR_vp_tuple_0::Thr0(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr1(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0148)
{
   // When multiple external Drain()'s, an external Join(), and an external Destroy() overlap in
   // that order, the later calls wait for the earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr1 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1..Externals] / Thr0 are the Drain()'ers.
   for ( i = 1 ; i <= Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr0,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(1 + Externals + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( Externals == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr0's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));
   for ( i = 1 ; i <= Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }
   YIELD_X(Externals + Workers + 5);

   // Wake Thr1 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(Externals + Workers + 15);

   // Wake the first worker. The first worker will wake the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // This thread calls Destroy()
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);

   for ( i = 0 ; i <= Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr2(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr3(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0149)
{
   // When multiple external Drain()'s, an external Join(), and a self-referential Destroy()
   // overlap in that order, the later calls wait for the earlier to complete. All calls complete
   // successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr3 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1..Externals] / Thr2 are the Drain()'ers.
   for ( i = 1 ; i <= Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr2,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(1 + Externals + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr2's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));
   for ( i = 1 ; i <= Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }
   YIELD_X(Externals + 3);

   // Wake Thr3 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(5);

   // Wake the first worker. The first worker will wake the remaining workers.
   // Some worker does the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i <= Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr4(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0150)
{
   // When multiple external Drain()'s, a self-referential Join(), and an external Destroy()
   // overlap in that order, the later calls wait for the earlier to complete. All calls
   // complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_Scratch[0] = i;

      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr4,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(Externals + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( Externals == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Externals + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( Externals + 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr4's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }

   YIELD_X(Externals + 5);

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Join(), Post()'s m_Sems[0] to wake this thread,
   //  then wakes the remaining workers.
   // This thread resumes and does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   YIELD_X(Externals + 5);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr5(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0151)
{
   // When multiple external Drain()'s, a self-referential Join(), and a self-referential
   // Destroy() overlap in that order, the later calls wait for the earlier to complete.
   // All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr5,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(Externals + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( Externals == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Externals + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Externals + 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr5's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }

   // Wake the first worker.
   // The first worker invokes the self-referential Join(), then wakes the remaining workers.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr6(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0152)
{
   // When multiple self-referential Drain()'s, an external Join(), and an external Destroy()
   // overlap in that order, the later calls wait for the earlier to complete. All calls complete
   // successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 + Drains ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[2]) ));
      } else if ( Drains + 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( Drains + 10 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50 + Drains, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * Post()'s m_Sems[2], waking Thr6, then yield's to Thr6.
   //  * Post()'s m_Sems[0], waking this thread, then yields to this thread.
   //  * wakes the remaining workers.
   // When this thread wakes, we yield to Thr6 so that Thr6 invokes Join().
   // This thread then does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(5);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, x);
   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0153)
{
   // When multiple self-referential Drain()'s, an external Join(), and a self-referential
   // Destroy() overlap in that order, the later calls wait for the earlier to complete. All
   // calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( Drains + 4 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 9 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * Post()'s m_Sems[0] to wake this thread.
   //  * blocks on m_Sems[1].
   // This thread wakes, Post()'s m_Sems[1] to wake a worker, then begins the Join().
   // If the resumed worker gets a timeslice before this thread calls Join(), it executes a
   //  yield, giving the cpu back to this thread.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0154)
{
   // When multiple self-referential Drain()'s, a self-referential Join(), and an external
   // Destroy() overlap in that order, the later calls wait for the earlier to complete.
   // All calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   AAL::btInt x = 0;

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( Drains + 4 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * invokes the self-referential Join().
   //  * Post()'s m_Sems[0], waking this thread.
   //  * yeilds the cpu to this thread.
   // This thread resumes and begins the Destroy().
   // The thread group worker is time sliced and wakes the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0155)
{
   // When multiple self-referential Drain()'s, a self-referential Join(), and a
   // self-referential Destroy() overlap in that order, the later calls wait for the earlier
   // to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker..
   //  * invokes the self-referential Drain().
   //  * invokes the self-referential Join().
   //  * wakes the remaining workers.
   // Some worker performs the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr7(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr8(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0156)
{
   // When multiple external Drain()'s, a single self-referential Drain(), an external Join(),
   // and an external Destroy() overlap in that order, the later calls wait for the earlier to
   // complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr8 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr8,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1..Externals] / Thr7 are the Drain()'ers.
   for ( i = 1 ; i <= Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr7,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(1 + Externals + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
      } else if ( Externals + 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr7's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));
   for ( i = 1 ; i <= Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }
   YIELD_X(3 * Externals);

   // Wake the first worker, then sleep on m_Sems[0].
   // The first worker begins the self-Drain(), wakes us from sleep on m_Sems[0], then blocks
   //  again on m_Sems[1].
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake Thr8 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);

   YIELD_X(Externals);

   // Wake the first worker. The first worker will wake the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));

   YIELD_X(Externals);

   // This thread calls Destroy()
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);

   for ( i = 0 ; i <= Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr9(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr10(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0157)
{
   // When multiple external Drain()'s, a single self-referential Drain(), an external Join(),
   // and a self-referential Destroy() overlap in that order, the later calls wait for the earlier
   // to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr10 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr10,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1..Externals] / Thr9 are the Drain()'ers.
   for ( i = 1 ; i <= Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr9,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(1 + Externals + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr9's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));
   for ( i = 1 ; i <= Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }
   YIELD_X(Externals);

   // Wake the first worker, then sleep on m_Sems[0].
   // The first worker begins the self-referential Drain(), Post()'s m_Sems[0] to wake this thread,
   //  then sleeps again on m_Sems[1].
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake Thr10 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(3);

   // Wake the first worker. The first worker will wake the remaining workers.
   // Some worker does the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i <= Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr11(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0158)
{
   // When multiple external Drain()'s, a single self-referential Drain(), a self-referential
   // Join(), and an external Destroy() overlap in that order, the later calls wait for the
   // earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_Scratch[0] = i;

      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr11,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(Externals + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( Externals == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Externals + 1 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Externals + 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( Externals + 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr11's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Join(), Post()'s m_Sems[0] to wake this thread,
   //  then wakes the remaining workers.
   // This thread resumes and does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   YIELD_X(Externals);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr12(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0159)
{
   // When multiple external Drain()'s, a single self-referential Drain(), a self-referential
   // Join(), and a self-referential Destroy() overlap in that order, the later calls wait for
   // the earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, (AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_Scratch[0] = i;
      m_pThrs[i] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr12,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[i]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(Externals + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 15 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 20 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr11's to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post((AAL::btInt)Externals));

   for ( i = 0 ; i < Externals ; ++i ) {
      YIELD_WHILE(0 == m_Scratch[i]);
   }
   YIELD_X(Externals + 5);

   // Wake the first worker.
   // The first worker invokes the self-referential Join(), then wakes the remaining workers.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr13(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr14(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0160)
{
   // When a single external Drain(), multiple self-referential Drain()'s, an external Join(),
   // and an external Destroy() overlap in that order, the later calls wait for the earlier to
   // complete. All calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr14 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr14,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1] / Thr13 is the Drain()'er.
   m_Scratch[0] = 1;
   m_pThrs[1] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr13,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[1]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   m_Scratch[0] = 0;

   EXPECT_EQ(2 + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
      } else if ( Drains + 20 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake Thr13 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[1]);
   YIELD_X(5);

   // Wake the first worker, then sleep on m_Sems[0].
   // The first worker begins the self-Drain(), wakes us from sleep on m_Sems[0], then blocks
   //  again on m_Sems[1].
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake Thr14 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(10);

   // Wake the first worker. The first worker will wake the remaining workers.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // This thread calls Destroy()
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);

   for ( i = 0 ; i < 2 ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr15(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr16(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[3].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Join(AAL_INFINITE_WAIT));
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0161)
{
   // When a single external Drain(), multiple self-referential Drain()'s, an external Join(),
   // and a self-referential Destroy() overlap in that order, the later calls wait for the
   // earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Drains = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));
   // The Join()'er waits to be signaled by m_Sems[3].
   ASSERT_TRUE(m_Sems[3].Create(0, 1));

   // m_pThrs[0] / Thr16 is the Join()'er
   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr16,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   // m_pThrs[1] / Thr15 are the Drain()'ers.
   m_Scratch[0] = 1;
   m_pThrs[1] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr15,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[1]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   m_Scratch[0] = 0;

   EXPECT_EQ(2 + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 4 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake Thr15 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[1]);
   YIELD_X(3);

   // Wake the first worker, then sleep on m_Sems[0].
   // The first worker begins the self-referential Drain(), Post()'s m_Sems[0] to wake this thread,
   //  then sleeps again on m_Sems[1].
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake Thr16 to begin the Join().
   EXPECT_TRUE(m_Sems[3].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(3);

   // Wake the first worker. The first worker will wake the remaining workers.
   // Some worker does the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i < 2 ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr17(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0162)
{
   // When a single external Drain(), multiple self-referential Drain()'s, a self-referential
   // Join(), and an external Destroy() overlap in that order, the later calls wait for the
   // earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_Scratch[0] = 0;

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr17,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));
      } else if ( Drains + 5 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake Thr17 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(10);

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Join(), Post()'s m_Sems[0] to wake this thread,
   //  then goes back to sleep on m_Sems[1].
   // This thread resumes, Post()'s m_Sems[1] to wake the first worker, and does the Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));

   m_pThrs[0]->Join();
   EXPECT_EQ(2, m_Scratch[0]);

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

void OSAL_ThreadGroupSR_vp_tuple_0::Thr18(OSLThread *pThread, void *arg)
{
   OSAL_ThreadGroupSR_vp_tuple_0 *f = static_cast<OSAL_ThreadGroupSR_vp_tuple_0 *>(arg);
   ASSERT_NONNULL(pThread);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // My unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[2].Wait());

   ++f->m_Scratch[t];
   EXPECT_TRUE(f->m_pGroup->Drain());
   ++f->m_Scratch[t];
}

TEST_P(OSAL_ThreadGroupSR_vp_tuple_0, aal0163)
{
   // When a single external Drain(), multiple self-referential Drain()'s, a self-referential
   // Join(), and a self-referential Destroy() overlap in that order, the later calls wait for
   // the earlier to complete. All calls complete successfully.

   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   STAGE_WORKERS(Workers);

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr18,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   for ( i = 0 ; i < Drains + 50 ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( Drains == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( Drains + 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( Drains + 4 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(Drains + 50, g->GetNumWorkItems());

   // Wake Thr18 to begin the Drain().
   EXPECT_TRUE(m_Sems[2].Post(1));
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(3);

   // Wake the first worker.
   // The first worker invokes the self-referential Join(), then wakes the remaining workers.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   m_pThrs[0]->Join();
   EXPECT_EQ(2, m_Scratch[0]);

   YIELD_WHILE(CurrentThreads() > 0);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroupSR_vp_tuple_0,
                        ::testing::Combine(
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)2,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10),
                                           ::testing::Values((AAL::btUnsignedInt)2,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)15)
                                          ));

#endif // GTEST_HAS_TR1_TUPLE

