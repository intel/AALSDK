// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "dbg_barrier.h"

TEST(OSAL_Barrier, aal0172)
{
   // Barrier::~Barrier() should behave robustly when no call to Barrier::Create() was made.
   Barrier *pBar = new Barrier();
   delete pBar;
}

TEST(OSAL_Barrier, aal0173)
{
   // Barrier::Destroy() should behave robustly when no call to Barrier::Create() was made.
   Barrier *pBar = new Barrier();
   EXPECT_FALSE(pBar->Destroy());
   delete pBar;
}


class OSAL_Barrier_f : public ::testing::Test
{
protected:
   OSAL_Barrier_f() :
      m_UnlockCount(0),
      m_bAutoReset(false)
   {}
   virtual ~OSAL_Barrier_f() {}

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
      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      for ( i = 0 ; i < sizeof(m_Sems) / sizeof(m_Sems[0]) ; ++i ) {
         m_Sems[i].Destroy();
      }

      Destroy();
   }

   AAL::btBool Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset)
   {
      m_UnlockCount = UnlockCount;
      m_bAutoReset  = bAutoReset;
      return m_Barrier.Create(m_UnlockCount, m_bAutoReset);
   }
   AAL::btBool Destroy() { return m_Barrier.Destroy(); }

   AAL::btBool Reset(AAL::btUnsignedInt UnlockCount)
   { return m_Barrier.Reset(UnlockCount); }

   AAL::btBool CurrCounts(AAL::btUnsignedInt &Cur, AAL::btUnsignedInt &Unl)
   { return m_Barrier.CurrCounts(Cur, Unl); }

   AAL::btBool Post(AAL::btUnsignedInt c) { return m_Barrier.Post(c); }

   AAL::btBool UnblockAll() { return m_Barrier.UnblockAll(); }

   AAL::btUnsignedInt NumWaiters() const { return m_Barrier.NumWaiters(); }

   AAL::btBool Wait() { return m_Barrier.Wait(); }
   AAL::btBool Wait(AAL::btTime Timeout) { return m_Barrier.Wait(Timeout); }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[4];
   AAL::btUnsignedInt m_Scratch[4];
   CSemaphore         m_Sems[2];

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );

   static void Thr2(OSLThread * , void * );
   static void Thr5(OSLThread * , void * );

   static void Thr3(OSLThread * , void * );
   static void Thr6(OSLThread * , void * );

   static void Thr4(OSLThread * , void * );
   static void Thr7(OSLThread * , void * );
};

void OSAL_Barrier_f::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->Wait());
   f->m_Scratch[1] = 1;
}

TEST_F(OSAL_Barrier_f, aal0174)
{
   // Calling Barrier::Create() with UnlockCount == 0 results in an unlock count of 1.

   EXPECT_TRUE(Create(0, false));

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   m_pThrs[0] = new OSLThread(OSAL_Barrier_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1, CurrentThreads());

   // Thr0 will block on the Barrier, because the unlock count was set to 1.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // Post()'ing 1 unlocks the Barrier, because the unlock count is 1.
   EXPECT_TRUE(Post(1));
   YIELD_WHILE(0 == m_Scratch[1]);

   m_pThrs[0]->Join();
   EXPECT_EQ(0, CurrentThreads());
}

TEST_F(OSAL_Barrier_f, aal0176)
{
   // When already created, calling Barrier::Create() returns false, indicating an error.
   EXPECT_TRUE(Create(1, false));

   EXPECT_FALSE(Create(3, false));
   EXPECT_FALSE(Create(2, true));

   EXPECT_TRUE(Destroy());
   EXPECT_TRUE(Create(5, true));

   EXPECT_FALSE(Create(3, false));
   EXPECT_FALSE(Create(2, true));
}

TEST_F(OSAL_Barrier_f, aal0177)
{
   // Calling Reset() when not Create()'ed fails, returning false.
   EXPECT_FALSE(Reset(0));
   EXPECT_FALSE(Reset(1));
   EXPECT_FALSE(Reset(5));
   EXPECT_FALSE(Reset(0));

   EXPECT_TRUE(Create(1, false));
   EXPECT_TRUE(Reset(0));
   EXPECT_TRUE(Destroy());

   EXPECT_FALSE(Reset(0));
   EXPECT_FALSE(Reset(1));
   EXPECT_FALSE(Reset(5));
   EXPECT_FALSE(Reset(0));
}

TEST_F(OSAL_Barrier_f, aal0178)
{
   // Calling CurrCounts() when not Create()'ed fails, returning false.

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;

   EXPECT_FALSE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(0, u);

   EXPECT_FALSE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(0, u);

   EXPECT_TRUE(Create(1, false));

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Destroy());

   c = u = 593;
   EXPECT_FALSE(CurrCounts(c, u));
   EXPECT_EQ(593, c);
   EXPECT_EQ(593, u);

   EXPECT_FALSE(CurrCounts(c, u));
   EXPECT_EQ(593, c);
   EXPECT_EQ(593, u);
}

TEST_F(OSAL_Barrier_f, aal0179)
{
   // Calling Post() when not Create()'ed fails, returning false.

   EXPECT_FALSE(Post(0));
   EXPECT_FALSE(Post(1));
   EXPECT_FALSE(Post(2));

   EXPECT_TRUE(Create(1, false));

   EXPECT_TRUE(Post(0));
   EXPECT_TRUE(Post(1));
   EXPECT_TRUE(Post(2));

   EXPECT_TRUE(Destroy());

   EXPECT_FALSE(Post(0));
   EXPECT_FALSE(Post(1));
   EXPECT_FALSE(Post(2));
}

TEST_F(OSAL_Barrier_f, aal0180)
{
   // Calling UnblockAll() when not Create()'ed fails, returning false.

   EXPECT_FALSE(UnblockAll());
   EXPECT_FALSE(UnblockAll());

   EXPECT_TRUE(Create(1, false));
   EXPECT_TRUE(UnblockAll());
   EXPECT_TRUE(Destroy());

   EXPECT_FALSE(UnblockAll());
   EXPECT_FALSE(UnblockAll());
}

TEST_F(OSAL_Barrier_f, aal0181)
{
   // Calling NumWaiters() when not Create()'ed returns 0.
   EXPECT_EQ(0, NumWaiters());
}

TEST_F(OSAL_Barrier_f, aal0182)
{
   // Calling Wait() when not Create()'ed fails, returning false.
   EXPECT_FALSE(Wait());
}

TEST_F(OSAL_Barrier_f, aal0183)
{
   // Calling Wait(Timeout) when not Create()'ed fails, returning false.
   EXPECT_FALSE(Wait(AAL_INFINITE_WAIT));
   EXPECT_FALSE(Wait(100));
}

void OSAL_Barrier_f::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->m_Sems[1].Wait());
   EXPECT_TRUE(f->Wait(AAL_INFINITE_WAIT));
}

TEST_F(OSAL_Barrier_f, aal0186)
{
   // When using an auto-reset Barrier (Barrier::Create() with bAutoReset=true), if
   // the Barrier count is Post()'ed such that it reaches the unlock count when no threads
   // are waiting on the Barrier, the Barrier is automatically reset by the Post().

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);

   EXPECT_TRUE(m_Sems[0].Create(0, 1));
   EXPECT_TRUE(m_Sems[1].Create(0, T));

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]
      m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr1,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(t, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());


   EXPECT_TRUE(Create(2, true));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters()); // threads are blocked on m_Sems[1].

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters()); // threads are blocked on m_Sems[1].

   EXPECT_TRUE(Post(1)); // No waiters - the Barrier will be automatically reset by the Post().

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(0, NumWaiters()); // threads are blocked on m_Sems[1].


   // Resume the threads so that they Wait() on the Barrier.
   EXPECT_TRUE(m_Sems[1].Post(T));

   YIELD_WHILE(NumWaiters() < T);

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(T, NumWaiters());

   // Releases the Barrier, freeing threads to exit. The last thread to return from Wait()
   // resets the Barrier.
   EXPECT_TRUE(Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(0, NumWaiters());


   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());

   EXPECT_TRUE(Post(1)); // No waiters - the Barrier will be automatically reset by the Post().

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());
}

TEST_F(OSAL_Barrier_f, aal0188)
{
   // Calling Barrier::Reset() on an auto-reset Barrier returns false, indicating failure.

   EXPECT_TRUE(Create(1, true));
   EXPECT_FALSE(Reset(1));
}

TEST_F(OSAL_Barrier_f, aal0189)
{
   // Calling Barrier::Reset() (typical case, not unblocking nor destroying) on a manual-reset
   // Barrier resets the current count to zero and optionally changes the unlock count, if
   // parameter UnlockCount is not 0. If UnlockCount is 0, then the Barrier unlock count is not
   // changed.

   EXPECT_TRUE(Create(1, false));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Post(1));
   EXPECT_TRUE(Wait(100)); // Barrier is unlocked.

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(1, u);


   EXPECT_TRUE(Reset(0));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Post(1));
   EXPECT_TRUE(Wait(AAL_INFINITE_WAIT)); // Barrier is unlocked.

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(1, u);


   EXPECT_TRUE(Reset(2));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Post(1));
   EXPECT_TRUE(Wait(5)); // Barrier is unlocked.

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(2, c);
   EXPECT_EQ(2, u);
}

TEST_F(OSAL_Barrier_f, aal0190)
{
   // Calling CurrCounts() on a Create()'ed Barrier retrieves a snapshot of the current
   // and the unlock counts.

   EXPECT_TRUE(Create(2, false)); // manual reset

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(2, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Reset(0));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Destroy());


   EXPECT_TRUE(Create(2, true)); // auto-reset

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);
}

TEST_F(OSAL_Barrier_f, aal0191)
{
   // When UnblockAll() is called on a manual-reset Barrier with no waiters blocked on the Barrier,
   // the current count is set to the unlock count; and the unblock completes immediately. Future
   // Wait() calls proceed through Wait() successfully without blocking.

   EXPECT_TRUE(Create(1, false));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   EXPECT_EQ(0, NumWaiters());
   EXPECT_TRUE(UnblockAll());

   EXPECT_TRUE(Wait()); // won't block, current count == unlock count. Returns true.
   EXPECT_TRUE(Wait(5));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Wait(AAL_INFINITE_WAIT));
}

void OSAL_Barrier_f::Thr2(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_FALSE(f->Wait());
   f->m_Scratch[t] = 1;
}

void OSAL_Barrier_f::Thr5(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0];
   const AAL::btTime        Timeout = f->m_Scratch[1];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_FALSE(f->Wait(Timeout));
   f->m_Scratch[t] = 1;
}

TEST_F(OSAL_Barrier_f, aal0192)
{
   // When UnblockAll() is called on a manual-reset Barrier with waiters blocked on the Barrier,
   // the current count is set to the unlock count; the waiters return false from their Wait()
   // calls; and the unblock completes when the last waiter returns. Future Wait() calls return
   // false immediately, indicating failure, until the last waiter has finished the unblock
   // operation. When the last waiter returns from Wait(), the unblock is complete. Future Wait()
   // calls flow through Wait() without blocking, because the current count remains equal to the
   // unlock count.

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;
         AAL::btUnsignedInt sum;

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   EXPECT_TRUE(Create(2, false));
   EXPECT_TRUE(Post(1));
   EXPECT_EQ(0, NumWaiters());

   m_Scratch[1] = 1000; // Timeout for Thr5's.

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]

      if ( 0 == ( t % 2 ) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr2,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr5,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   // All threads are blocked on m_Sems[1]. Signal them to "go".
   EXPECT_TRUE(m_Sems[1].Post(T));

   YIELD_WHILE(NumWaiters() < T);
   EXPECT_EQ(T, CurrentThreads());

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   EXPECT_TRUE(UnblockAll());

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(2, c);
   EXPECT_EQ(2, u);

   do
   {
      sum = 0;
      for ( t = 0 ; t < T ; ++t ) {
         sum += m_Scratch[t];
      }

      if ( sum < T ) {
         cpu_yield(); // EXPECT_FALSE(Wait()) << "Wait() should fail while unblocking";
      }

   }while ( sum < T );

   EXPECT_EQ(0, NumWaiters());

   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
   }

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(2, c);
   EXPECT_EQ(2, u);

   EXPECT_TRUE(Wait()); // manual-reset Barrier remains unlocked.
   EXPECT_TRUE(Wait(1));
   EXPECT_TRUE(Wait(AAL_INFINITE_WAIT));
}

void OSAL_Barrier_f::Thr3(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_TRUE(f->Wait());
   f->m_Scratch[t] = 1;
}

void OSAL_Barrier_f::Thr6(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0];
   const AAL::btTime        Timeout = f->m_Scratch[1];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_TRUE(f->Wait(Timeout));
   f->m_Scratch[t] = 1;
}

TEST_F(OSAL_Barrier_f, aal0193)
{
   // When UnblockAll() is called on an auto-reset Barrier with no waiters blocked on the Barrier,
   // the current count is reset to 0; and the unblock completes immediately. Future Wait()
   // calls block until the current count becomes equal to the unlock count.

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   m_Scratch[1] = 1000; // Timeout for Thr6's.

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]

      if ( 0 == (t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr3,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr6,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   // All threads are blocked on m_Sems[1].

   EXPECT_TRUE(Create(2, true));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());

   // Post() the count up one. The Barrier remains locked.
   // This allows us to see the count become reset by the UnlbockAll() call.
   EXPECT_TRUE(Post(1));

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());

   c = u = 99;
   EXPECT_TRUE(UnblockAll());

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());

   // Wake all threads so that they block on the Barrier, which is now locked.
   EXPECT_TRUE(m_Sems[1].Post(T));

   // Give threads plenty of time to have resumed from the Barrier Wait() call.
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   YIELD_WHILE(NumWaiters() < T);

   // Resume threads from the auto-reset Barrier Wait().
   EXPECT_TRUE(Post(2));

   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c) << "Should be automatically reset";
   EXPECT_EQ(2, u);
   EXPECT_EQ(0, NumWaiters());

   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_Barrier_f::Thr4(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_FALSE(f->Wait());
   ++f->m_Scratch[t];

   while ( !f->Wait() ) {
      // Wait() calls made while unblocking fail.
      cpu_yield();
   }
   ++f->m_Scratch[t];
}

void OSAL_Barrier_f::Thr7(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0];
   const AAL::btTime        Timeout = f->m_Scratch[1];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_FALSE(f->Wait(Timeout));
   ++f->m_Scratch[t];

   while ( !f->Wait(Timeout) ) {
      // Wait() calls made while unblocking fail.
      cpu_yield();
   }
   ++f->m_Scratch[t];
}

TEST_F(OSAL_Barrier_f, aal0194)
{
   // When UnblockAll() is called on an auto-reset Barrier with waiters blocked on the Barrier,
   // all waiters are resumed, returning false from their Wait() calls. Subsequent Wait() calls
   // return false immediately, indicating failure, until the final unblocked waiter completes
   // the operation. When the final unblocked waiter resumes from Wait(), the current count is
   // reset to 0, and the unblock completes. Future Wait() calls block until the current count
   // becomes equal to the unlock count.

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   EXPECT_TRUE(Create(2, true));

   m_Scratch[1] = 1000; // Timeout for Thr7's.

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]

      if ( 0 == ( t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr4,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr7,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   // All threads are blocked on m_Sems[1].

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   // Post() the count up by one. The Barrier remains locked.
   // This is to verify that the UnblockAll() resets the count.
   EXPECT_TRUE(Post(1));

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   // Unblock all threads to Wait() on the Barrier.
   EXPECT_TRUE(m_Sems[1].Post(T));

   YIELD_WHILE(NumWaiters() < T);
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   // Unblocks all threads from the Barrier Wait(), returning false.
   // If a thread calls Wait() again while unblocking, the Wait() call fails immediately.
   EXPECT_TRUE(UnblockAll());

   // Make sure all of the threads have resumed from the first Wait().
   btBool Continue;
   do
   {
      Continue = true;

      for ( t = 0 ; t < T ; ++t ) {
         if ( 0 == m_Scratch[t] ) {
            Continue = false;
         }
      }

      if ( !Continue ) {
         cpu_yield();
      }
   }while( !Continue );

   // The last unblocked waiter resumes, and completes the UnblockAll(), resetting the Barrier
   // to locked, current count == 0.
   YIELD_WHILE(NumWaiters() < T);

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   // Unblock all waiters from the 2nd Wait() call so that they exit.
   EXPECT_TRUE(Post(2));

   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(2, m_Scratch[t]);
   }

   EXPECT_EQ(0, NumWaiters());
   EXPECT_EQ(0, CurrentThreads());
}


class OSAL_Barrier_Destroy_f : public ::testing::Test
{
public:
   OSAL_Barrier_Destroy_f() :
      m_pBarrier(NULL),
      m_UnlockCount(0),
      m_bAutoReset(false),
      m_ScratchCounter(0)
   {}
   virtual ~OSAL_Barrier_Destroy_f() {}

   virtual void SetUp()
   {
      m_ScratchCounter = 0;

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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
      Destroy();
   }

   AAL::btBool Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset)
   {
      if ( NULL == m_pBarrier ) {
         m_UnlockCount = UnlockCount;
         m_bAutoReset  = bAutoReset;
         m_pBarrier    = new(std::nothrow) Barrier();
      }
      return m_pBarrier->Create(m_UnlockCount, m_bAutoReset);
   }
   AAL::btBool Destroy()
   {
      if ( NULL != m_pBarrier ) {
         m_pBarrier->UserDefined(NULL);
         delete m_pBarrier;
         m_pBarrier = NULL;
         return true;
      }
      return false;
   }

   AAL::btBool Reset(AAL::btUnsignedInt UnlockCount)
   { return m_pBarrier->Reset(UnlockCount); }

   AAL::btBool CurrCounts(AAL::btUnsignedInt &Cur, AAL::btUnsignedInt &Unl)
   { return m_pBarrier->CurrCounts(Cur, Unl); }

   AAL::btBool Post(AAL::btUnsignedInt c) { return m_pBarrier->Post(c); }

   AAL::btBool UnblockAll() { return m_pBarrier->UnblockAll(); }

   AAL::btUnsignedInt NumWaiters() const { return m_pBarrier->NumWaiters(); }

   AAL::btBool Wait() { return m_pBarrier->Wait(); }
   AAL::btBool Wait(AAL::btTime Timeout) { return m_pBarrier->Wait(Timeout); }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   Barrier           *m_pBarrier;
   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   AAL::btUnsignedInt m_ScratchCounter;
   OSLThread         *m_pThrs[5];
   AAL::btUnsignedInt m_Scratch[5];
   CSemaphore         m_Sems[2];

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );

   static void Thr2(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );
};

TEST_F(OSAL_Barrier_Destroy_f, aal0197)
{
   // Calling Barrier::Destroy() on a Create()'ed Barrier with no waiters completes successfully.

   EXPECT_TRUE(Create(2, true)); // auto-reset Barrier.

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(Post(1));

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(0, NumWaiters());
   EXPECT_TRUE(Destroy());


   EXPECT_TRUE(Create(2, false)); // manual-reset Barrier.

   EXPECT_TRUE(Post(1));

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(0, NumWaiters());
   EXPECT_TRUE(Destroy());
}

void OSAL_Barrier_Destroy_f::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_Destroy_f *f = reinterpret_cast<OSAL_Barrier_Destroy_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_FALSE(f->Wait());
   f->m_Scratch[t] = 1;
}

void OSAL_Barrier_Destroy_f::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_Destroy_f *f = reinterpret_cast<OSAL_Barrier_Destroy_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0]; // my unique thread index.
   const AAL::btTime        Timeout = f->m_Scratch[1]; // Wait() timeout.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_FALSE(f->Wait(Timeout));
   f->m_Scratch[t] = 1;
}

TEST_F(OSAL_Barrier_Destroy_f, aal0198)
{
   // When calling Barrier::Destroy() on a Create()'ed auto-reset Barrier with waiters, all waiters are
   // unblocked and return false from their Wait() calls. The Barrier::Destroy() waits for all
   // waiters to return from Wait() calls before destroying the Barrier.

   class aal0198AfterBarrierAutoLock : public ::AAL::Testing::EmptyAfterBarrierAutoLock
   {
   public:
      aal0198AfterBarrierAutoLock(OSAL_Barrier_Destroy_f *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnWait()
      {
         ++m_pFixture->m_ScratchCounter;
      }

      virtual void OnWait(btTime )
      {
         ++m_pFixture->m_ScratchCounter;
      }

   protected:
      OSAL_Barrier_Destroy_f *m_pFixture;
   } AfterAutoLock(this);

   EXPECT_TRUE(Create(2, true)); // auto-reset Barrier.

   m_pBarrier->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   EXPECT_TRUE(m_Sems[0].Create(0, 1));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;

   m_Scratch[1] = 1000; // Timeout for Thr1's.

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]

      if ( 0 == ( t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_Destroy_f::Thr0,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_Destroy_f::Thr1,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   // Wait until all threads are asleep in Barrier::Wait() calls.
   YIELD_WHILE(m_ScratchCounter < T);

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(T, NumWaiters());

   EXPECT_TRUE(Destroy());

   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }
}

void OSAL_Barrier_Destroy_f::Thr2(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_Destroy_f *f = reinterpret_cast<OSAL_Barrier_Destroy_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_FALSE(f->Wait());
   f->m_Scratch[t] = 1;
}

void OSAL_Barrier_Destroy_f::Thr3(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_Destroy_f *f = reinterpret_cast<OSAL_Barrier_Destroy_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0]; // my unique thread index.
   const AAL::btTime        Timeout = f->m_Scratch[1]; // Wait() timeout.
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_FALSE(f->Wait(Timeout));
   f->m_Scratch[t] = 1;
}

TEST_F(OSAL_Barrier_Destroy_f, aal0199)
{
   // When calling Barrier::Destroy() on a Create()'ed manual-reset Barrier with waiters,
   // all waiters are unblocked and return false from their Wait() calls. The Barrier::Destroy()
   // waits for all waiters to return from Wait() calls before destroying the Barrier.

   class aal0199AfterBarrierAutoLock : public ::AAL::Testing::EmptyAfterBarrierAutoLock
   {
   public:
      aal0199AfterBarrierAutoLock(OSAL_Barrier_Destroy_f *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnWait()
      {
         ++m_pFixture->m_ScratchCounter;
      }

      virtual void OnWait(btTime)
      {
         ++m_pFixture->m_ScratchCounter;
      }

   protected:
      OSAL_Barrier_Destroy_f *m_pFixture;
   } AfterAutoLock(this);

   EXPECT_TRUE(Create(2, false)); // manual-reset Barrier.

   m_pBarrier->UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   EXPECT_TRUE(m_Sems[0].Create(0, 1));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;

   m_Scratch[1] = 1000; // Timeout for Thr3's.

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]

      if ( 0 == ( t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_Destroy_f::Thr2,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_Destroy_f::Thr3,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   // Wait until all threads are asleep in Barrier::Wait() calls.
   YIELD_WHILE(m_ScratchCounter < T);

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(T, NumWaiters());

   EXPECT_TRUE(Destroy());

   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }
}


// Value-parameterized test fixture
template <typename T>
class OSAL_Barrier_vp : public ::testing::TestWithParam<T>
{
public:
   OSAL_Barrier_vp() :
      m_UnlockCount(0),
      m_bAutoReset(false),
      m_ScratchCounter(0)
   {}
   virtual ~OSAL_Barrier_vp() {}

   virtual void SetUp()
   {
      m_ScratchCounter = 0;

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
      m_Barrier.UserDefined(NULL);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
      Destroy();
   }

   AAL::btBool Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset)
   {
      m_UnlockCount = UnlockCount;
      m_bAutoReset  = bAutoReset;
      return m_Barrier.Create(m_UnlockCount, m_bAutoReset);
   }
   AAL::btBool Destroy() { return m_Barrier.Destroy(); }

   AAL::btBool Reset(AAL::btUnsignedInt UnlockCount)
   { return m_Barrier.Reset(UnlockCount); }

   AAL::btBool CurrCounts(AAL::btUnsignedInt &Cur, AAL::btUnsignedInt &Unl)
   { return m_Barrier.CurrCounts(Cur, Unl); }

   AAL::btBool Post(AAL::btUnsignedInt c) { return m_Barrier.Post(c); }

   AAL::btBool UnblockAll() { return m_Barrier.UnblockAll(); }

   AAL::btUnsignedInt NumWaiters() const { return m_Barrier.NumWaiters(); }

   AAL::btBool Wait() { return m_Barrier.Wait(); }
   AAL::btBool Wait(AAL::btTime Timeout) { return m_Barrier.Wait(Timeout); }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   AAL::btUnsignedInt m_ScratchCounter;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[5];
   AAL::btUnsignedInt m_Scratch[5];
   CSemaphore         m_Sems[4];
};

class OSAL_Barrier_vp_uint_0 : public OSAL_Barrier_vp< AAL::btUnsignedInt >
{
public:
   static void Thr0(OSLThread * , void * );

   static void Thr1(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );

   static void Thr2(OSLThread * , void * );
};

void OSAL_Barrier_vp_uint_0::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   EXPECT_TRUE(f->Wait());
   EXPECT_TRUE(f->Wait()); // not an auto-reset Barrier, so we run through all the Wait()'s once unlocked..
   EXPECT_TRUE(f->Wait());
}

TEST_P(OSAL_Barrier_vp_uint_0, aal0175)
{
   // Calling Barrier::Create() with UnlockCount > 0 results in an unlock count of UnlockCount.

   class aal0175AfterBarrierAutoLock : public ::AAL::Testing::EmptyAfterBarrierAutoLock
   {
   public:
      aal0175AfterBarrierAutoLock(OSAL_Barrier_vp_uint_0 *pFixture) :
         m_pFixture(pFixture),
         m_Count(0)
      {}

      virtual void OnWait()
      {
         m_pFixture->m_Scratch[m_Count++] = 1;
      }

   protected:
      OSAL_Barrier_vp_uint_0 *m_pFixture;
      btUnsignedInt           m_Count;
   } AfterAutoLock(this);


   const AAL::btUnsignedInt U = GetParam();

   ASSERT_TRUE(Create(U, false));

   m_Barrier.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(u, U);

   m_pThrs[0] = new OSLThread(OSAL_Barrier_vp_uint_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_EQ(1, CurrentThreads());

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_UnlockCount - 1 ; ++i ) {
      EXPECT_EQ(0, m_Scratch[1] + m_Scratch[2]);

      c = u = 0;
      EXPECT_TRUE(CurrCounts(c, u));
      EXPECT_EQ(i, c);
      EXPECT_EQ(u, U);

      EXPECT_TRUE(Post(1));
   }

   EXPECT_EQ(1, NumWaiters());

   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1] + m_Scratch[2]));

   // The Barrier is at m_UnlockCount - 1, so Post()'ing 1 unlocks it.
   EXPECT_TRUE(Post(1));

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(c, U);
   EXPECT_EQ(u, U);

   YIELD_WHILE(0 == m_Scratch[2]);

   m_pThrs[0]->Join();
   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_Barrier_vp_uint_0::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   // We're careful to prevent threads from looping back into the Wait() call here.
   // The Barrier does provide protection against "floating" the wait count, but that is
   //  the subject of separate test cases.
   AAL::btUnsignedInt i;
   for ( i = 0 ; i < 2 ; ++i ) {
      EXPECT_TRUE(f->Wait());
      f->m_Scratch[t] = 1;
      EXPECT_TRUE(f->m_Sems[2].Wait());
      f->m_Scratch[t] = 0;
   }
}

void OSAL_Barrier_vp_uint_0::Thr3(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0];
   const AAL::btTime        Timeout = f->m_Scratch[1];
   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   // We're careful to prevent threads from looping back into the Wait() call here.
   // The Barrier does provide protection against "floating" the wait count, but that is
   //  the subject of separate test cases.
   AAL::btUnsignedInt i;
   for ( i = 0 ; i < 2 ; ++i ) {
      EXPECT_TRUE(f->Wait(Timeout));
      f->m_Scratch[t] = 1;
      EXPECT_TRUE(f->m_Sems[2].Wait());
      f->m_Scratch[t] = 0;
   }
}

TEST_P(OSAL_Barrier_vp_uint_0, aal0184)
{
   // When Barrier::Create() is called with bAutoReset=true, once the current count reaches
   //  the unlock count, the Barrier is unlocked. All threads blocked on the Barrier resume
   //  from Wait(), returning true. When the last waiter resumes, the Barrier is automatically
   //  reset to the locked state, with current count = 0. Future Wait() calls block on the
   //  Barrier until the current count reaches the unlock count again.

   class aal0184AfterBarrierAutoLock : public ::AAL::Testing::EmptyAfterBarrierAutoLock
   {
   public:
      aal0184AfterBarrierAutoLock(OSAL_Barrier_vp_uint_0 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnWait()
      {
         ++m_pFixture->m_ScratchCounter;
      }

      virtual void OnWait(btTime )
      {
         ++m_pFixture->m_ScratchCounter;
      }

   protected:
      OSAL_Barrier_vp_uint_0 *m_pFixture;
   } AfterAutoLock(this);


   const AAL::btUnsignedInt U = GetParam();
   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));
   ASSERT_TRUE(m_Sems[2].Create(0, T));

   ASSERT_TRUE(Create(U, true));

   m_Barrier.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   m_Scratch[1] = 1000; // Timeout for Thr3's.

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < T ; ++t ) {
      // pass unique thread index via m_Scratch[0]
      m_Scratch[0] = t;

      if ( 0 == (t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_uint_0::Thr1,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_uint_0::Thr3,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   EXPECT_EQ(t, CurrentThreads());

   // All threads are blocked on m_Sems[1].
   EXPECT_EQ(0, NumWaiters());

   // Allow threads to proceed into the Barrier Wait() loop.
   EXPECT_TRUE(m_Sems[1].Post(T));

   YIELD_WHILE(m_ScratchCounter < T);

   // Make m_UnlockCount - 1 Post()'s. All threads remain blocked in Barrier::Wait().
   AAL::btUnsignedInt i;
   for ( i = 0 ; i < U - 1 ; ++i ) {
      EXPECT_TRUE(Post(1));
      for ( t = 0 ; t < T ; ++t ) {
         EXPECT_EQ(0, m_Scratch[t]);
      }
   }

   // Give plenty of time for threads to have resumed.
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   EXPECT_EQ(t, NumWaiters());

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the Barrier waiters.
   EXPECT_TRUE(Post(1));

   // Wait for all waiters to resume from Barrier Wait().
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 == m_Scratch[t]);
   }

   EXPECT_EQ(t, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());

   // We expect the current count to be reset to 0.
   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   // Release all threads so that they call Wait() the 2nd time.
   EXPECT_TRUE(m_Sems[2].Post(T));

   // Wait for all waiters to loop back for the 2nd Barrier Wait() call.
   YIELD_WHILE(m_ScratchCounter < (2 * T));

   // Make m_UnlockCount - 1 Post()'s. All threads remain blocked in Barrier::Wait().
   for ( i = 0 ; i < U - 1 ; ++i ) {
      EXPECT_TRUE(Post(1));
      for ( t = 0 ; t < T ; ++t ) {
         EXPECT_EQ(0, m_Scratch[t]);
      }
   }

   // Give plenty of time for threads to have resumed.
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   EXPECT_EQ(t, NumWaiters());

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 2nd Barrier Wait().
   EXPECT_TRUE(Post(1));

   // Wait for all waiters to resume.
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 == m_Scratch[t]);
   }

   EXPECT_EQ(t, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());

   // We expect the current count to be reset to 0.
   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   // Release all threads.
   EXPECT_TRUE(m_Sems[2].Post(T));

   // Join() all the waiters.
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(1 == m_Scratch[t]);
      m_pThrs[t]->Join();
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());
}

void OSAL_Barrier_vp_uint_0::Thr2(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   const AAL::btUnsignedInt N = f->m_Scratch[1];
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(f->Wait());
      ++f->m_Scratch[t];
   }

   EXPECT_TRUE(f->m_Sems[1].Wait());
   f->m_Scratch[t] = 0;

   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(f->Wait());
      ++f->m_Scratch[t];
   }

   EXPECT_TRUE(f->m_Sems[1].Wait());
   f->m_Scratch[t] = 0;

   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(f->Wait());
      ++f->m_Scratch[t];
   }
}

TEST_P(OSAL_Barrier_vp_uint_0, aal0187)
{
   // An auto-reset Barrier can be transformed to a manual-reset Barrier (via Barrier::Create()),
   // so long as an intervening Barrier::Destroy() call is made. The same is true for a
   // manual-reset Barrier transitioning to an auto-reset Barrier.

   class aal0187AfterBarrierAutoLock : public ::AAL::Testing::EmptyAfterBarrierAutoLock
   {
   public:
      aal0187AfterBarrierAutoLock(OSAL_Barrier_vp_uint_0 *pFixture) :
         m_pFixture(pFixture)
      {}

      virtual void OnSleep()
      {
         ++m_pFixture->m_ScratchCounter;
      }

   protected:
      OSAL_Barrier_vp_uint_0 *m_pFixture;
   } AfterAutoLock(this);


   const AAL::btUnsignedInt U = GetParam();
   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   ASSERT_TRUE(Create(U, true)); // auto-reset Barrier.

   m_Barrier.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   m_Scratch[1] = 2; // number of loops in m_Scratch[1]

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]
      m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_uint_0::Thr2,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0].
      EXPECT_TRUE(m_Sems[0].Wait());
   }

   m_Scratch[0] = m_Scratch[1] = 0;

   EXPECT_EQ(t, CurrentThreads());

   YIELD_WHILE(m_ScratchCounter < T);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   EXPECT_EQ(t, NumWaiters());
   EXPECT_EQ(T, m_ScratchCounter);

   AAL::btUnsignedInt c = 0;
   AAL::btUnsignedInt u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 1st Barrier::Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads loop back into Wait().
   YIELD_WHILE(m_ScratchCounter < (2 * T));

   // Give plenty of time for threads to have resumed.
   YIELD_N();

   // This is an auto-reset Barrier, so we expect threads to have blocked again.
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }
   EXPECT_EQ(2 * T, m_ScratchCounter);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }
   EXPECT_EQ(2 * T, m_ScratchCounter);

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 2nd Barrier::Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads resume from Barrier::Wait().
   // Threads will block on m_Sems[1].
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(1 == m_Scratch[t]);
   }



   // Destroy the auto-reset Barrier.
   EXPECT_TRUE(Destroy());

   // Re-create as a manual-reset Barrier.
   ASSERT_TRUE(Create(U, false)); // manual-reset Barrier.

   // Allow threads to resume from m_Sems[1], progressing to the next phase of the test.
   EXPECT_TRUE(m_Sems[1].Post(T));

   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 != m_Scratch[t]);
   }

   YIELD_WHILE(m_ScratchCounter < (3 * T));
   EXPECT_EQ(t, NumWaiters());

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 1st Wait() on the manual-reset Barrier.
   EXPECT_TRUE(Post(1));

   // The Barrier is now a manual-reset Barrier, and nobody is resetting it. The threads calling
   // Wait() will flow through the Wait() call twice without blocking.
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(2 != m_Scratch[t]);
   }
   EXPECT_EQ(3 * T, m_ScratchCounter);


   // Destroy the manual-reset Barrier.
   EXPECT_TRUE(Destroy());

   // Re-create as an auto-reset Barrier.
   ASSERT_TRUE(Create(U, true)); // auto-reset Barrier.

   // Allow threads to progress to the next phase of the test.
   EXPECT_TRUE(m_Sems[1].Post(T));

   YIELD_WHILE(m_ScratchCounter < (4 * T));


   EXPECT_EQ(t, NumWaiters());
   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   // This Post() unblocks all the waiters from the 1st Barrier Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads loop back into Wait().
   YIELD_WHILE(m_ScratchCounter < (5 * T));


   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();

   // This is an auto-reset Barrier, so we expect threads to have blocked again.
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have errantly resumed from Barrier Wait().
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 2nd Wait(), allowing them to exit.
   EXPECT_TRUE(Post(1));

   // Join() all the waiters.
   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());
}

INSTANTIATE_TEST_CASE_P(My, OSAL_Barrier_vp_uint_0,
                           ::testing::Values((AAL::btUnsignedInt)1,
                                             (AAL::btUnsignedInt)5,
                                             (AAL::btUnsignedInt)10,
                                             (AAL::btUnsignedInt)25,
                                             (AAL::btUnsignedInt)50));





#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Barrier_vp_tuple_0 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btUnsignedInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_Barrier_vp_tuple_0() :
      m_UnlockCount(0),
      m_bAutoReset(false),
      m_Signals(0)
   {}
   virtual ~OSAL_Barrier_vp_tuple_0() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }
      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
      m_Signals = 0;
   }
   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      for ( i = 0 ; i < sizeof(m_Sems) / sizeof(m_Sems[0]) ; ++i ) {
         m_Sems[i].Destroy();
      }

      Destroy();
   }

   AAL::btBool Create(AAL::btUnsignedInt UnlockCount, AAL::btBool bAutoReset)
   {
      m_UnlockCount = UnlockCount;
      m_bAutoReset  = bAutoReset;
      return m_Barrier.Create(m_UnlockCount, m_bAutoReset);
   }
   AAL::btBool Destroy() { return m_Barrier.Destroy(); }

   AAL::btBool Reset(AAL::btUnsignedInt UnlockCount)
   { return m_Barrier.Reset(UnlockCount); }

   AAL::btBool CurrCounts(AAL::btUnsignedInt &Cur, AAL::btUnsignedInt &Unl)
   { return m_Barrier.CurrCounts(Cur, Unl); }

   AAL::btBool Post(AAL::btUnsignedInt c) { return m_Barrier.Post(c); }

   AAL::btBool UnblockAll() { return m_Barrier.UnblockAll(); }

   AAL::btUnsignedInt NumWaiters() const { return m_Barrier.NumWaiters(); }

   AAL::btBool Wait() { return m_Barrier.Wait(); }
   AAL::btBool Wait(AAL::btTime Timeout) { return m_Barrier.Wait(Timeout); }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   AAL::btUIntPtr     m_Signals;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[25];
   AAL::btUnsignedInt m_Scratch[25];
   CSemaphore         m_Sems[3];

   static void Thr0(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );

   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
};

void OSAL_Barrier_vp_tuple_0::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_0 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.
   const AAL::btUnsignedInt W = f->m_Scratch[1]; // number of Wait() calls.

   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   AAL::btUnsignedInt w;
   for ( w = 0 ; w < W ; ++w ) {
      EXPECT_TRUE(f->Wait());
      ++f->m_Scratch[t];
      EXPECT_TRUE(f->m_Sems[2].Post(1));
   }
}

void OSAL_Barrier_vp_tuple_0::Thr3(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_0 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t       = f->m_Scratch[0]; // my unique thread index.
   const AAL::btUnsignedInt W       = f->m_Scratch[1]; // number of Wait() calls.
   const AAL::btTime        Timeout = f->m_Scratch[2]; // Timeout for Wait().

   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   AAL::btUnsignedInt w;
   for ( w = 0 ; w < W ; ++w ) {
      EXPECT_TRUE(f->Wait(Timeout));
      ++f->m_Scratch[t];
      EXPECT_TRUE(f->m_Sems[2].Post(1));
   }
}

TEST_P(OSAL_Barrier_vp_tuple_0, aal0185)
{
   // When using an auto-reset Barrier (Barrier::Create() with bAutoReset=true), when the
   //  Barrier becomes unlocked, all threads blocked in Wait() calls at the time of unlock
   //  are resumed from Wait() before any thread is allowed to proceed through another Wait()
   //  call without blocking. This prevents a low-latency thread from "floating" the Wait()
   //  count, rendering the Barrier ineffective.

   AAL::btUnsignedInt       w;
   const AAL::btUnsignedInt W = 50;

   AAL::btUnsignedInt       c;
   AAL::btUnsignedInt       u;

   AAL::btUnsignedInt       T;
   AAL::btUnsignedInt       U;

   std::tr1::tie(T, U) = GetParam();

   ASSERT_TRUE(Create(U, true));

   // For Thr0 init.
   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));
   // Count up sem for Wait() loop. This threads Wait()'s. Thr0's and Thr3's Post().
   ASSERT_TRUE(m_Sems[2].Create(-((AAL::btInt)T), 1));

   // Pass the number of Wait() calls in m_Scratch[1]
   m_Scratch[1] = W;
   m_Scratch[2] = 3000; // Timeout for Thr3's.

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t;

      if ( 0 == (t % 2) ) {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_tuple_0::Thr0,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
      } else {
         m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_tuple_0::Thr3,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);

      }
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = m_Scratch[2] = 0;

   EXPECT_EQ(t, CurrentThreads());

   // All threads are blocked on m_Sems[1]. Resume them.
   EXPECT_EQ(0, NumWaiters());
   EXPECT_TRUE(m_Sems[1].Post(T));

   for ( w = 0 ; w < W ; ++w ) {
      // Verify that all m_Scratch[t] == w. If not, then some thread made it
      //  through Wait() twice without blocking.
      for ( t = 0 ; t < T ; ++t ) {
         ASSERT_EQ(w, m_Scratch[t]);
      }

      c = u = 99;
      EXPECT_TRUE(CurrCounts(c, u));
      EXPECT_EQ(0, c);
      EXPECT_EQ(U, u);

      // Post() to just before the unlock value.
      EXPECT_TRUE(Post(U - 1));

      c = u = 0;
      EXPECT_TRUE(CurrCounts(c, u));
      EXPECT_EQ(U-1, c);
      EXPECT_EQ(U,   u);

      // Make sure all of the threads are blocked in Wait().
      YIELD_WHILE(NumWaiters() < t);
      for ( t = 0 ; t < T ; ++t ) {
         ASSERT_EQ(w, m_Scratch[t]);
      }

      // This will unlock the Barrier, beginning the auto-reset.
      EXPECT_TRUE(Post(1));

      // Wait for NumThreads Post()'s to m_Sems[2].
      EXPECT_TRUE(m_Sems[2].Wait());
      EXPECT_TRUE(m_Sems[2].Reset(-((AAL::btInt)T)));
   }

   // Join() all of the threads.
   for ( t = 0 ; t < T ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(w, m_Scratch[t]);
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());
}

#if defined( __AAL_LINUX__ )
// Linux-only. The only candidates for user-defined signals in Windows are SIGILL and SIGTERM, per:
// https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
//
// Both SIGILL and SIGTERM cause the process to terminate, as determined by running this test case in Windows.

void OSAL_Barrier_vp_tuple_0::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_0 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_0 *>(pArg);
   ASSERT_NONNULL(f);

   SignalHelper &sig = SignalHelper::GetInstance();

   btUnsignedInt idx = sig.ThreadRegister(GetThreadID());

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.

   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->Wait());

#if   defined( __AAL_WINDOWS__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) < f->m_Signals);
#elif defined( __AAL_LINUX__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGIO, idx) < f->m_Signals);
#endif // __AAL_LINUX__

   f->m_Scratch[t] = 1;
}

TEST_P(OSAL_Barrier_vp_tuple_0, aal0195)
{
   // When the Barrier is Create()'ed and a thread blocks in Barrier::Wait(void), given that no
   // call to Barrier::Post() nor Barrier::UnblockAll() is made by another thread, the blocked
   // thread waits infinitely, even in the presence of signals.

   SignalHelper &sig = SignalHelper::GetInstance();

   sig.RegistryReset();
   sig.ThreadRegister(GetThreadID());

   AAL::btUnsignedInt NumThreads;
   AAL::btUnsignedInt SignalAttempts;

   std::tr1::tie(NumThreads, SignalAttempts) = GetParam();

   m_Signals = SignalAttempts;

   ASSERT_TRUE(Create(1, true));

   // For Thr1 init.
   ASSERT_TRUE(m_Sems[0].Create(0, 1));

   // Each thread Post()'s m_Sems[0] when ready.

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_Scratch[0] = t;
      m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_tuple_0::Thr1,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   EXPECT_EQ(t, CurrentThreads());
   YIELD_WHILE(NumWaiters() < NumThreads);

   const btTime  sleepeach = 5;
   btBool        Done      = false;
   btUnsignedInt idx;

   while ( !Done ) {

      for ( t = 0 ; t < NumThreads ; ++t ) {

         idx = sig.ThreadLookup(m_pThrs[t]->tid());

#if   defined( __AAL_WINDOWS__ )

         if ( sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) >= m_Signals ) {
            continue;
         }
         EXPECT_EQ(0, sig.Raise(SignalHelper::IDX_SIGUSR1, m_pThrs[t]->tid()));

#elif defined( __AAL_LINUX__ )

         if ( sig.GetCount(SignalHelper::IDX_SIGIO, idx) >= m_Signals ) {
            continue;
         }
         EXPECT_EQ(0, sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[t]->tid()));

#endif // OS

      }

      sleep_millis(sleepeach);

      for ( t = 0 ; t < NumThreads ; ++t ) {
         EXPECT_EQ(0, m_Scratch[t]);
      }

      EXPECT_EQ(t, CurrentThreads());
      EXPECT_EQ(t, NumWaiters());

      Done = true;

      for ( t = 0 ; t < NumThreads ; ++t ) {
         idx = sig.ThreadLookup(m_pThrs[t]->tid());
#if   defined( __AAL_WINDOWS__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) < m_Signals) {
#elif defined( __AAL_LINUX__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGIO, idx) < m_Signals ) {
#endif // OS
            Done = false;
            break;
         }
      }

   }

   // Unlock the Barrier, allowing threads to exit.
   EXPECT_TRUE(Post(1));

   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());

   sig.RegistryReset();
}
#endif // __AAL_LINUX__

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Barrier_vp_tuple_0,
                        ::testing::Combine(
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)25),
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)25,
                                                             (AAL::btUnsignedInt)50)
                                          ));

#endif // GTEST_HAS_TR1_TUPLE



#if GTEST_HAS_TR1_TUPLE

#if defined( __AAL_LINUX__ )

// Value-parameterized test fixture (tuple)
class OSAL_Barrier_vp_tuple_1 : public OSAL_Barrier_vp_tuple_0
{
protected:
   OSAL_Barrier_vp_tuple_1() {}
   virtual ~OSAL_Barrier_vp_tuple_1() {}

   static void Thr0(OSLThread * , void * );
};

// Linux-only. The only candidates for user-defined signals in Windows are SIGILL and SIGTERM, per:
// https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
//
// Both SIGILL and SIGTERM cause the process to terminate, as determined by running this test case in Windows.
void OSAL_Barrier_vp_tuple_1::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_1 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_1 *>(pArg);
   ASSERT_NONNULL(f);

   SignalHelper &sig = SignalHelper::GetInstance();

   btUnsignedInt idx = sig.ThreadRegister(GetThreadID());

   const AAL::btUnsignedInt t       = f->m_Scratch[0]; // my unique thread index.
   const AAL::btTime        Timeout = f->m_Scratch[1]; // timeout to Wait().

   EXPECT_TRUE(f->m_Sems[0].Post(1));
   EXPECT_TRUE(f->m_Sems[1].Wait());

   EXPECT_FALSE(f->Wait(Timeout));
   ++f->m_Scratch[t];

#if   defined( __AAL_WINDOWS__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) < f->m_Signals);
#elif defined( __AAL_LINUX__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGIO, idx) < f->m_Signals);
#endif // __AAL_LINUX__
}

TEST_P(OSAL_Barrier_vp_tuple_1, aal0196)
{
   // When the Barrier is Create()'ed and a thread blocks in Barrier::Wait(btTime X), given that
   // no call to Barrier::Post() nor Barrier::UnblockAll() is made by another thread, the blocked
   // thread waits at least X milliseconds before resuming, even in the presence of signals.
   // Upon timing out, the Wait() call returns false, indicating failure.

   SignalHelper &sig = SignalHelper::GetInstance();

   sig.RegistryReset();
   sig.ThreadRegister(GetThreadID());

   m_Signals = 5;

   AAL::btUnsignedInt NumThreads;
   AAL::btTime        Timeout;

   std::tr1::tie(NumThreads, Timeout) = GetParam();

   ASSERT_TRUE(Create(2, true));

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(2, u);

   // Post() the Barrier up one so that we can verify that the count is not affected by the timeout.
   EXPECT_TRUE(Post(1));

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   // For Thr2 init.
   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   // For the "go" signal.
   ASSERT_TRUE(m_Sems[1].Create(0, NumThreads));

   m_Scratch[1] = (AAL::btUnsignedInt) Timeout;

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_Scratch[0] = t;
      m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_tuple_1::Thr0,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   EXPECT_EQ(t, CurrentThreads());

   // All threads should be blocked on m_Sems[1].
   EXPECT_EQ(0, NumWaiters());

   btTime       slept     = 0;
   const btTime sleepeach = 5;
   const btTime thresh    = Timeout - ( Timeout / 2 ); // within 50%

   // Wake all threads, allowing them to block on the Barrier.
   EXPECT_TRUE(m_Sems[1].Post(NumThreads));

   btUnsignedInt idx;
   btBool        Done = false;

   while ( !Done ) {
      sleep_millis(sleepeach);
      slept += sleepeach;

      for ( t = 0 ; t < NumThreads ; ++t ) {

         idx = sig.ThreadLookup(m_pThrs[t]->tid());
#if   defined( __AAL_WINDOWS__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) >= m_Signals ) {
#elif defined( __AAL_LINUX__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGIO, idx) >= m_Signals ) {
#endif // OS

            // All of the signals have been delivered.
            continue;
         }

         EXPECT_EQ(0, m_Scratch[t]);

#if   defined( __AAL_WINDOWS__ )
         sig.Raise(SignalHelper::IDX_SIGUSR1, m_pThrs[t]->tid());
#elif defined( __AAL_LINUX__ )
         sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[t]->tid());
#endif // OS

      }

      if ( slept < thresh ) {
         // Only perform the following checks if we're not within our timeout threshold.
         c = u = 0;
         EXPECT_TRUE(CurrCounts(c, u));
         EXPECT_EQ(1, c);
         EXPECT_EQ(2, u);

         EXPECT_EQ(t, CurrentThreads());
         //EXPECT_EQ(t, NumWaiters());
      }

      Done = true;
      for ( t = 0 ; t < NumThreads ; ++t ) {

         idx = sig.ThreadLookup(m_pThrs[t]->tid());

#if   defined( __AAL_WINDOWS__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) < m_Signals ) {
            if ( 0 == sig.Raise(SignalHelper::IDX_SIGUSR1, m_pThrs[t]->tid()) ) {
               Done = false;
               break;
            }
         }
#elif defined( __AAL_LINUX__ )
         if ( sig.GetCount(SignalHelper::IDX_SIGIO, idx) < m_Signals ) {
            if ( ESRCH != sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[t]->tid()) ) {
               Done = false;
               break;
            }
         }
#endif // OS

      }
   }

   // Allow the Wait() calls to time out.

   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(2, u);

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());

   sig.RegistryReset();
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Barrier_vp_tuple_1,
                        ::testing::Combine(
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)25),
                                           ::testing::Values((AAL::btTime)150,
                                                             (AAL::btTime)200,
                                                             (AAL::btTime)250)
                                          ));

#endif // __AAL_LINUX__

#endif // GTEST_HAS_TR1_TUPLE

