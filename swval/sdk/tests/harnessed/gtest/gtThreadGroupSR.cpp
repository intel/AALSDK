// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtThreadGroup.h"

// Value-parameterized test fixture
template <typename T>
class OSAL_ThreadGroupSR_vp : public ::testing::TestWithParam<T>
{
public:
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

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

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

   class aal0107AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0107AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0107AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 25 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes all workers upon entering the Drain() below.
   // Meanwhile, this thread begins the external Drain().
   // Some worker encounters the self-referential Drain().
   // Both Drain()'s complete successfully.

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

   class aal0109AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0109AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnJoin(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0109AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 30 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes all workers upon entering the Join() call below.
   // Meanwhile, we will have invoked the Join().
   // Some worker encounters the Join() in the queue, but it fails because our Join() is in progress.

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

   class aal0116AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0116AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnJoin(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0116AfterThreadGroupAutoLock AfterAutoLock(this, 1);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes one worker as soon as the Join() below is encountered.
   // That worker attempts the Drain(), which fails because Join() is in progress.
   // The first worker wakes the remaining workers, allowing the Join() to complete.

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
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker.
   // The first worker will invoke the self-referential Destroy().
   // That worker then encounters the self-referential Drain(),
   //  which fails because Destroy() is in progress.
   // The first worker wakes the remaining workers, and the ThreadGroup is eventually destroyed.
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

   class aal0120AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0120AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDestroy(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0120AfterThreadGroupAutoLock AfterAutoLock(this, 1);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g, false) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes one worker upon entering the Destroy() call below.
   // The first worker attempts a self-referential Drain(), which fails.
   // The first worker wakes the remaining workers, allowing the Destroy() to complete.
   // The single worker approach guarantees that Drain() doesn't encounter an empty queue,
   //  which results in a true return status.

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0121)
{
   // When a self-referential Join() is encountered during an external Drain(),
   // the Join() waits for the Drain() to complete. Both calls complete successfully.

   class aal0121AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0121AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0121AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes all workers upon entering the Drain() below.
   // Some worker executes the self-referential Join(), which completes successfully.

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

   class aal0122AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0122AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0122AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
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

   // This thread wakes the first worker, then blocks on m_Sems[0].
   // The first worker wakes and begins the self-referential Join().
   // The next queue item is a Post of m_Sems[0], which resumes this thread to enter the Drain() below.
   // The following queue item is a wait on m_Sems[1], which puts the first worker back to sleep.
   // The AfterAutoLock callback wakes all workers upon entering Drain().
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_FALSE(g->Drain());

   YIELD_WHILE(CurrentThreads() > 0);

   EXPECT_TRUE(g->Destroy(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_1, aal0123)
{
   // When a self-referential Destroy() is encountered during an external Drain(), the
   // Destroy() waits for the Drain() to complete. Both calls complete successfully.

   class aal0123AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0123AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0123AfterThreadGroupAutoLock AfterAutoLock(this, 1);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes the first worker upon entering the Drain() below.
   // The first worker begins the self-referential Destroy(), then wakes the remaining workers.

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
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // This thread wakes the first worker, then block on m_Sems[0].
   // The first worker wakes and begins the self-referential Destroy().
   // The first worker Post's m_Sems[0], waking this thread to enter the Drain(),
   //  which fails because Destroy() is in progress.
   // The first worker blocks on m_Sems[1].
   // This thread wakes all workers, allowing the Destroy() to complete.
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_FALSE(g->Drain());
   EXPECT_TRUE(m_Sems[1].Post(w));

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

   // Wake the first worker, which begins the self-referential Join().
   // The first worker wakes the remaining workers.
   // Some worker encounters the Destroy().
   // The Destroy() waits for the queue to empty before completing.

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

   class aal0128AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0128AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDestroy(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0128AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g, AAL_INFINITE_WAIT, false) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes all workers upon entering the Destroy() call below.
   // Some worker attempts a self-referential Join(), which fails because a Destroy() is in progress.

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

   class aal0129AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0129AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_1 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnJoin(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_1 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0129AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // The AfterAutoLock callback wakes all workers upon entering the Join() below.
   // Some worker encounters the self-referential Destroy().
   // The Destroy() waits for the Join() to complete before tearing down the ThreadGroup.

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));
   EXPECT_EQ(1, x);

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

   class aal0140AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0140AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // Post m_Sems[3], waking Thr1 to do the Join().
         m_pFixture->m_Sems[3].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[0], waking the main thread.
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnDestroy(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   // Thr0 calls Drain(), Thr1 calls Join(), and this thread does the Destroy().

   STAGE_WORKERS(GetParam());

   aal0140AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

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

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 15 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr0 (m_Sems[2]) to begin the Drain().
   // Upon entering Drain(), the AfterAutoLock callback wakes Thr1 (m_Sems[3]) to do the Join().
   // Upon entering Join(), the AfterAutoLock callback wakes us (m_Sems[0]) to do the Destroy().
   // Upon entering Destroy(), the AfterAutoLock callback wakes all workers (m_Sems[1]).

   EXPECT_TRUE(m_Sems[2].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

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

   class aal0141AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0141AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // Post m_Sems[3], waking Thr3 to do the Join().
         m_pFixture->m_Sems[3].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   // Thr2 calls Drain(), Thr3 calls Join().

   STAGE_WORKERS(GetParam());

   aal0141AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

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

   btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr2 to begin the Drain().
   // The AfterAutoLock callback wakes Thr3 (m_Sems[3]) upon entering the Drain().
   // Thr3 invokes the Join().
   // The AfterAutoLock callback wakes all workers (m_Sems[1]) upon entering the Join().

   EXPECT_TRUE(m_Sems[2].Post(1));

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
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

   class aal0142AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0142AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // Post m_Sems[1], waking one worker to do the Join().
         m_pFixture->m_Sems[1].Post(1);
      }

      virtual void OnDestroy(btTime )
      {
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   // Thr4 does the Drain().

   STAGE_WORKERS(GetParam());

   aal0142AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

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

   // Wake Thr4 (m_Sems[2]) to begin the Drain().
   // The AfterAutoLock callback wakes the first worker thread (m_Sems[1]) upon entering Drain().
   // The first worker begins the self-referential Join().
   // The first worker wakes this thread (m_Sems[0]) to do the Destroy().
   // The first worker goes back to sleep on m_Sems[1].
   // The AfterAutoLock callback wakes all worker threads (m_Sems[1]) upon entering the Destroy() below.

   EXPECT_TRUE(m_Sems[2].Post(1));
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

   class aal0143AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0143AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnDrain()
      {
         // Post m_Sems[1], waking one worker to do the Join().
         m_pFixture->m_Sems[1].Post(1);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
   } AfterAutoLock(this);

   // Thr5 does the Drain().

   STAGE_WORKERS(GetParam());

   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   ASSERT_TRUE(m_Sems[2].Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_ThreadGroup_vp_uint_2::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake Thr5 to begin the Drain().
   // The AfterAutoLock callback wakes one worker thread upon entering Drain().
   // The first worker invokes the self-referential Join(), then wakes the remaining workers.
   // Some worker invokes the self-referential Destroy().

   EXPECT_TRUE(m_Sems[2].Post(1));

   m_pThrs[0]->Join();

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
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

   class aal0144AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0144AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // Post m_Sems[2], waking Thr6 to do the Join().
         m_pFixture->m_Sems[2].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[0], waking the main thread.
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnDestroy(btTime )
      {
         // Post m_Sems[1], waking all worker threads;
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   // Use Thr6 to do the Join().

   STAGE_WORKERS(GetParam());

   aal0144AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

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
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Drain(), then goes back to sleep on m_Sems[1].
   // The AfterAutoLock callback wakes Thr6 (m_Sems[2]), upon entering the Drain().
   // Thr6 invokes the external Join().
   // The AfterAutoLock callback wakes this thread (m_Sems[0]) upon entering the Join().
   // This thread invokes the Destroy().
   // The AfterAutoLock callback wakes all workers (m_Sems[1]) upon entering Destroy().

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

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

   class aal0145AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0145AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0145AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 10 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker invokes the self-referential Drain().
   // The first worker Post's m_Sems[0], waking this thread.
   // The first worker goes back to sleep on m_Sems[1].
   // This thread invokes the external Join().
   // The AfterAutoLock callback wakes all worker threads upon entering the Join().
   // Some worker invokes the self-referential Destroy().

   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(g->Join(AAL_INFINITE_WAIT));

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
}

TEST_P(OSAL_ThreadGroup_vp_uint_2, aal0146)
{
   // When a self-referential Drain(), a self-referential Join(), and an external Destroy()
   //  overlap in that order, the later calls wait for the earlier to complete.
   //  All calls complete successfully.

   class aal0146AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0146AfterThreadGroupAutoLock(OSAL_ThreadGroup_vp_uint_2 *pFixture,
                                      btInt                       Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDestroy(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroup_vp_uint_2 *m_pFixture;
      btInt                       m_Count;
   };

   STAGE_WORKERS(GetParam());

   aal0146AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[0]) ));
      } else if ( 3 == i ) {
         EXPECT_TRUE(Add( new WaitD(m_Sems[1]) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   // Wake the first worker, then block on Sems[0].
   // The first worker invokes the self-referential Drain().
   // The first worker invokes the self-referential Join().
   // The first worker wakes this thread (m_Sems[0]), then goes back to sleep on m_Sems[1].
   // This thread invokes the external Destroy().
   // The AfterAutoLock callback wakes all workers (m_Sems[1]) upon entering the Destroy().

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

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 6 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
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
   EXPECT_EQ(1, x);
}

INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_2,
                           ::testing::Range((AAL::btUnsignedInt)1, (AAL::btUnsignedInt)11));



#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_ThreadGroupSR_vp_tuple_0 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btUnsignedInt, AAL::btUnsignedInt > >
{
public:
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

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

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

   class aal0148AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0148AfterThreadGroupAutoLock(OSAL_ThreadGroupSR_vp_tuple_0 *pFixture,
                                      btInt                          Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // As the Thr0's enter Drain(), they signal so by m_Sems[0] (count-up Sem).
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[0], waking the main thread.
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnDestroy(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroupSR_vp_tuple_0 *m_pFixture;
      btInt                          m_Count;
   };

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   const AAL::btInt e = (AAL::btInt) Externals;

   STAGE_WORKERS(Workers);

   aal0148AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, e));
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
      if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[0].Reset(-e));
   // Wake Thr0's to begin the Drain().
   // As each enters Drain(), the AfterAutoLock callback Post()'s m_Sems[0].
   EXPECT_TRUE(m_Sems[2].Post(e));
   EXPECT_TRUE(m_Sems[0].Wait());

   // All Drain()'s are active.

   // Wake Thr1 to begin the Join().
   // Upon entering Join(), the AfterAutoLock callback Post()'s m_Sems[0].
   EXPECT_TRUE(m_Sems[0].Reset(0));
   EXPECT_TRUE(m_Sems[3].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // This thread calls Destroy().
   // The AfterAutoLock callback wakes all workers (m_Sems[1]) upon entering Destroy().
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

   class aal0149AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0149AfterThreadGroupAutoLock(OSAL_ThreadGroupSR_vp_tuple_0 *pFixture,
                                      btInt                          Count) :
         m_pFixture(pFixture),
         m_Count(Count)
      {}

      virtual void OnDrain()
      {
         // As the Thr2's enter Drain(), they signal so by m_Sems[0] (count-up Sem).
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[1], waking all workers.
         m_pFixture->m_Sems[1].Post(m_Count);
      }

   protected:
      OSAL_ThreadGroupSR_vp_tuple_0 *m_pFixture;
      btInt                          m_Count;
   };

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   const AAL::btInt e = (AAL::btInt) Externals;

   STAGE_WORKERS(Workers);

   aal0149AfterThreadGroupAutoLock AfterAutoLock(this, w);
   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, e));
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

   btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 3 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[0].Reset(-e));
   // Wake Thr2's to begin the Drain().
   // As each enters Drain(), the AfterAutoLock callback Post()'s m_Sems[0].
   EXPECT_TRUE(m_Sems[2].Post(e));
   EXPECT_TRUE(m_Sems[0].Wait());

   // All Drain()'s are active.

   EXPECT_TRUE(m_Sems[0].Reset(0));
   // Wake Thr3 to begin the Join().
   // Upon entering Join(), the AfterAutoLock callback wakes all workers (m_Sems[1]).
   EXPECT_TRUE(m_Sems[3].Post(1));

   for ( i = 0 ; i <= Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
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

   class aal0150AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0150AfterThreadGroupAutoLock(OSAL_ThreadGroupSR_vp_tuple_0 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnDrain()
      {
         // As the Thr4's enter Drain(), they signal so by m_Sems[0] (count-up Sem).
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // Post m_Sems[0], waking the main thread.
         m_pFixture->m_Sems[0].Post(1);
      }

   protected:
      OSAL_ThreadGroupSR_vp_tuple_0 *m_pFixture;
   } AfterAutoLock(this);

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   const AAL::btInt e = (AAL::btInt) Externals;

   STAGE_WORKERS(Workers);

   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, e));

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
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 2 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[0].Reset(-e));
   // Wake Thr4's to begin the Drain().
   // The AfterAutoLock callback Post()'s m_Sems[0] as each Drain() call is entered.
   EXPECT_TRUE(m_Sems[2].Post(e));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake the first worker, then block on m_Sems[0].
   // The first worker invokes the self-referential Join().
   // The AfterAutoLock callback Post()'s m_Sems[0], waking this thread, upon entering the Join().
   // The first worker wakes the remaining workers.
   EXPECT_TRUE(m_Sems[0].Reset(0));
   EXPECT_TRUE(m_Sems[1].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // This thread resumes and does the Destroy().
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

   class aal0151AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0151AfterThreadGroupAutoLock(OSAL_ThreadGroupSR_vp_tuple_0 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnDrain()
      {
         // As the Thr5's enter Drain(), they signal so by m_Sems[0] (count-up Sem).
         m_pFixture->m_Sems[0].Post(1);
      }

   protected:
      OSAL_ThreadGroupSR_vp_tuple_0 *m_pFixture;
   } AfterAutoLock(this);

   AAL::btUnsignedInt Workers   = 0;
   AAL::btUnsignedInt Externals = 0;

   std::tr1::tie(Workers, Externals) = GetParam();

   const AAL::btInt e = (AAL::btInt) Externals;

   STAGE_WORKERS(Workers);

   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // The Drain()'ers wait to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, e));

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

   AAL::btInt x = 0;

   for ( i = 0 ; i < 50 ; ++i ) {
      if ( 0 == i ) {
         EXPECT_TRUE(Add( new JoinThreadGroupD(g) ));
      } else if ( 1 == i ) {
         EXPECT_TRUE(Add( new PostD(m_Sems[1], w-1) ));
      } else if ( 5 == i ) {
         EXPECT_TRUE(Add( new DestroyThreadGroupD(g) ));
      } else if ( 49 == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(50, g->GetNumWorkItems());

   EXPECT_TRUE(m_Sems[0].Reset(-e));
   // Wake Thr5's to begin the Drain().
   // The AfterAutoLock callback Post()'s m_Sems[0] as each Drain() call is entered.
   EXPECT_TRUE(m_Sems[2].Post(e));
   EXPECT_TRUE(m_Sems[0].Wait());

   // Wake the first worker.
   // The first worker invokes the self-referential Join().
   // The first worker wakes the remaining workers.
   // Some worker encounters the self-referential Destroy().
   EXPECT_TRUE(m_Sems[1].Post(1));

   for ( i = 0 ; i < Externals ; ++i ) {
      m_pThrs[i]->Join();
      EXPECT_EQ(2, m_Scratch[i]);
   }

   YIELD_WHILE(CurrentThreads() > 0);
   EXPECT_EQ(1, x);
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

   class aal0152AfterThreadGroupAutoLock : public ::AAL::Testing::EmptyAfterThreadGroupAutoLock
   {
   public:
      aal0152AfterThreadGroupAutoLock(OSAL_ThreadGroupSR_vp_tuple_0 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnDrain()
      {
         // As the workers enter Drain(), they signal so by m_Sems[0] (count-up Sem).
         m_pFixture->m_Sems[0].Post(1);
      }

      virtual void OnJoin(btTime )
      {
         // When Thr6 enters Join(), it signals so by m_Sems[0].
         m_pFixture->m_Sems[0].Post(1);
      }

   protected:
      OSAL_ThreadGroupSR_vp_tuple_0 *m_pFixture;
   } AfterAutoLock(this);


   AAL::btUnsignedInt Workers = 0;
   AAL::btUnsignedInt Drains  = 0;

   std::tr1::tie(Workers, Drains) = GetParam();

   AAL::btInt d = (AAL::btInt) Drains;

   STAGE_WORKERS(Workers);

   g->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   // This thread blocked on m_Sems[0] until all workers were ready.
   // Thread group workers are currently blocked on m_Sems[1].
   // Thr6 waits to be signaled by m_Sems[2].
   ASSERT_TRUE(m_Sems[2].Create(0, 1));
   ASSERT_TRUE(m_Sems[3].Create(0, INT_MAX));

   m_Scratch[0] = 0;
   m_pThrs[0] = new OSLThread(OSAL_ThreadGroupSR_vp_tuple_0::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(1 + m_MinThreads, CurrentThreads());

   AAL::btInt x = 0;

   for ( i = 0 ; i < 100 + Drains ; ++i ) {
      if ( i < Drains ) {
         EXPECT_TRUE(Add( new DrainThreadGroupD(g) ));
      } else if ( 99 + Drains == i ) {
         EXPECT_TRUE(Add( new UnsafeCountUpD(x) ));
      } else {
         EXPECT_TRUE(Add( new YieldD() ));
      }
   }

   ASSERT_EQ(100 + Drains, g->GetNumWorkItems());

   // Wake all workers, then block on m_Sems[0].
   // The AfterAutoLock callback Posts()'s m_Sems[0] as Drain()'s are entered.
   EXPECT_TRUE(m_Sems[0].Reset(-d));
   EXPECT_TRUE(m_Sems[1].Post(w));
   EXPECT_TRUE(m_Sems[0].Wait());

   // The Drain()'ers are all staged.

   // Wake Thr6 so that Thr6 invokes Join().
   EXPECT_TRUE(m_Sems[0].Reset(0));
   EXPECT_TRUE(m_Sems[2].Post(1));
   EXPECT_TRUE(m_Sems[0].Wait());

   // The Join() is in progress.

   // This thread then does the Destroy().
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

