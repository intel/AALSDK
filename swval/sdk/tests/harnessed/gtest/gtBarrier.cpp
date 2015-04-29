// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

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

   AAL::btUnsignedInt CurrentThreads() const { return Config.CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[4];
   AAL::btUnsignedInt m_Scratch[4];
   CSemaphore         m_Sems[2];

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );
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
   EXPECT_TRUE(f->Wait());
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
   EXPECT_TRUE(Wait()); // Barrier is unlocked.

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
   EXPECT_TRUE(Wait()); // Barrier is unlocked.

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
   EXPECT_TRUE(Wait()); // Barrier is unlocked.

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
   EXPECT_TRUE(Wait());

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(1, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Wait());
}

void OSAL_Barrier_f::Thr2(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_f *f = reinterpret_cast<OSAL_Barrier_f *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_FALSE(f->Wait());
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

   EXPECT_TRUE(Create(1, false));
   EXPECT_EQ(0, NumWaiters());

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]
      m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr2,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   YIELD_WHILE(NumWaiters() < T);
   EXPECT_EQ(T, CurrentThreads());

   AAL::btUnsignedInt c = 99;
   AAL::btUnsignedInt u = 0;

   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(1, u);

   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   EXPECT_TRUE(UnblockAll());

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
   EXPECT_EQ(1, c);
   EXPECT_EQ(1, u);

   EXPECT_TRUE(Wait()); // manual-reset Barrier remains unlocked.
   EXPECT_TRUE(Wait());
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

TEST_F(OSAL_Barrier_f, aal0193)
{
   // When UnblockAll() is called on an auto-reset Barrier with no waiters blocked on the Barrier,
   // the current count is reset to 0; and the unblock completes immediately. Future Wait()
   // calls block until the current count becomes equal to the unlock count.

   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);
         AAL::btUnsignedInt t;

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]
      m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr3,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

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
   EXPECT_EQ(0, c);
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

   EXPECT_FALSE(f->Wait());
   ++f->m_Scratch[t];

   while ( !f->Wait() ) {
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

   EXPECT_TRUE(Create(2, true));

   for ( t = 0 ; t < T ; ++t ) {
      m_Scratch[0] = t; // unique thread index in m_Scratch[0]
      m_pThrs[t] = new OSLThread(OSAL_Barrier_f::Thr4,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = 0;

   YIELD_WHILE(NumWaiters() < T);
   EXPECT_EQ(T, CurrentThreads());

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

   EXPECT_EQ(T, NumWaiters());
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   // Unblocks all threads from the Barrier Wait(), returning false.
   // If a thread calls Wait() again while unblocking, the Wait() call fails immediately.
   EXPECT_TRUE(UnblockAll());

   YIELD_N(); // Give plenty of time for waiters to have made it through Wait() a 2nd time.

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

// Value-parameterized test fixture
template <typename T>
class OSAL_Barrier_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_Barrier_vp() :
      m_UnlockCount(0),
      m_bAutoReset(false)
   {}
   virtual ~OSAL_Barrier_vp() {}

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

   AAL::btUnsignedInt CurrentThreads() const { return Config.CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[5];
   AAL::btUnsignedInt m_Scratch[5];
   CSemaphore         m_Sems[4];
};

class OSAL_Barrier_vp_uint_0 : public OSAL_Barrier_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
};

void OSAL_Barrier_vp_uint_0::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   f->m_Scratch[0] = 1;
   EXPECT_TRUE(f->Wait());
   EXPECT_TRUE(f->Wait()); // not an auto-reset Barrier..
   EXPECT_TRUE(f->Wait());
   f->m_Scratch[1] = 1;
}

TEST_P(OSAL_Barrier_vp_uint_0, aal0175)
{
   // Calling Barrier::Create() with UnlockCount > 0 results in an unlock count of UnlockCount.

   const AAL::btUnsignedInt U = GetParam();

   ASSERT_TRUE(Create(U, false));

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
      EXPECT_EQ(0, m_Scratch[1]);

      c = u = 0;
      EXPECT_TRUE(CurrCounts(c, u));
      EXPECT_EQ(i, c);
      EXPECT_EQ(u, U);

      EXPECT_TRUE(Post(1));
   }

   EXPECT_EQ(1, NumWaiters());

   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // Post()'ing 1 unlocks the Barrier, because the unlock count is 1.
   EXPECT_TRUE(Post(1));

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(c, U);
   EXPECT_EQ(u, U);

   YIELD_WHILE(0 == m_Scratch[1]);

   m_pThrs[0]->Join();
   EXPECT_EQ(0, CurrentThreads());
}

void OSAL_Barrier_vp_uint_0::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_uint_0 *f = reinterpret_cast<OSAL_Barrier_vp_uint_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0];
   EXPECT_TRUE(f->m_Sems[0].Post(1));

   // We're careful to prevent threads from looping back into the Wait() call here.
   // The Barrier does provide protection against "floating" the wait count, but that is
   //  the subject of separate test cases.
   AAL::btUnsignedInt i;
   for ( i = 0 ; i < 2 ; ++i ) {
      EXPECT_TRUE(f->Wait());
      f->m_Scratch[t] = 1;
      EXPECT_TRUE(f->m_Sems[1].Wait());
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

   const AAL::btUnsignedInt U = GetParam();
   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   ASSERT_TRUE(Create(U, true));

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < T ; ++t ) {
      // pass unique thread index via m_Scratch[0]
      m_Scratch[0] = t;
      m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_uint_0::Thr1,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }

   m_Scratch[0] = 0;

   EXPECT_EQ(t, CurrentThreads());

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

   // This Post() unblocks all the waiters.
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

   // Release all threads so that they call Wait() the 2nd time.
   EXPECT_TRUE(m_Sems[1].Post(T));

   // Wait for all waiters to loop back for the 2nd Wait() call.
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(1 == m_Scratch[t]);
   }

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

   // This Post() unblocks all the waiters from the 2nd Wait().
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
   EXPECT_TRUE(m_Sems[1].Post(T));

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

   const AAL::btUnsignedInt U = GetParam();
   const AAL::btUnsignedInt T = sizeof(m_pThrs) / sizeof(m_pThrs[0]);

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, T));

   ASSERT_TRUE(Create(U, true)); // auto-reset Barrier.

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

   AAL::btUnsignedInt i;

   EXPECT_TRUE(Post(U - 1));
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

   // This Post() unblocks all the waiters from the 1st Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads loop back into Wait().
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 == m_Scratch[t]);
   }

   // Give plenty of time for threads to have resumed.
   YIELD_N();

   // This is an auto-reset Barrier, so we expect threads to have blocked again.
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have resumed.
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   c = u = 0;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(U - 1, c);
   EXPECT_EQ(U,     u);

   // This Post() unblocks all the waiters from the 2nd Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads loop back into Wait().
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(1 == m_Scratch[t]);
   }



   // Destroy the auto-reset Barrier.
   EXPECT_TRUE(Destroy());

   // Re-create as a manual-reset Barrier.
   ASSERT_TRUE(Create(U, false)); // manual-reset Barrier.

   // Allow threads to progress to the next phase of the test.
   EXPECT_TRUE(m_Sems[1].Post(T));

   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 != m_Scratch[t]);
   }

   EXPECT_EQ(t, NumWaiters());

   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have resumed.
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
   // Wait() will flow through the Wait() call twice.
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(2 != m_Scratch[t]);
   }



   // Destroy the manual-reset Barrier.
   EXPECT_TRUE(Destroy());

   // Re-create as an auto-reset Barrier.
   ASSERT_TRUE(Create(U, true)); // auto-reset Barrier.

   // Allow threads to progress to the next phase of the test.
   EXPECT_TRUE(m_Sems[1].Post(T));

   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 != m_Scratch[t]);
   }

   EXPECT_EQ(t, NumWaiters());
   c = u = 99;
   EXPECT_TRUE(CurrCounts(c, u));
   EXPECT_EQ(0, c);
   EXPECT_EQ(U, u);

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have resumed.
   YIELD_N();
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(0, m_Scratch[t]);
   }

   // This Post() unblocks all the waiters from the 1st Wait().
   EXPECT_TRUE(Post(1));

   // Pause until all of the threads loop back into Wait().
   for ( t = 0 ; t < T ; ++t ) {
      YIELD_WHILE(0 == m_Scratch[t]);
   }

   // Give plenty of time for threads to have resumed.
   YIELD_N();

   // This is an auto-reset Barrier, so we expect threads to have blocked again.
   for ( t = 0 ; t < T ; ++t ) {
      EXPECT_EQ(1, m_Scratch[t]);
   }

   EXPECT_TRUE(Post(U - 1));
   // Give plenty of time for threads to have resumed.
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
      m_bAutoReset(false)
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

   AAL::btUnsignedInt CurrentThreads() const { return Config.CurrentThreads(); }

   AAL::btUnsignedInt m_UnlockCount;
   AAL::btBool        m_bAutoReset;
   Barrier            m_Barrier;
   OSLThread         *m_pThrs[25];
   AAL::btUnsignedInt m_Scratch[25];
   CSemaphore         m_Sems[2];

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
};

void OSAL_Barrier_vp_tuple_0::Thr0(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_0 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.
   const AAL::btUnsignedInt W = f->m_Scratch[1]; // number of Wait() calls.

   EXPECT_TRUE(f->m_Sems[0].Post(1));

   AAL::btUnsignedInt w;
   for ( w = 0 ; w < W ; ++w ) {
      EXPECT_TRUE(f->Wait());
      ++f->m_Scratch[t];
      EXPECT_TRUE(f->m_Sems[1].Post(1));
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

   AAL::btUnsignedInt c;
   AAL::btUnsignedInt u;

   AAL::btUnsignedInt NumThreads;
   AAL::btUnsignedInt U;

   std::tr1::tie(NumThreads, U) = GetParam();

   ASSERT_TRUE(Create(U, true));

   // For Thr0 init.
   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   // Count up sem for Wait() loop. This threads Wait()'s. Thr0's Post().
   ASSERT_TRUE(m_Sems[1].Create(-((AAL::btInt)NumThreads), 1));

   // Pass the number of Wait() calls in m_Scratch[1]
   m_Scratch[1] = W;
   // Each thread Post()'s m_Sems[0] when ready.

   AAL::btUnsignedInt t;
   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_Scratch[0] = t;
      m_pThrs[t] = new OSLThread(OSAL_Barrier_vp_tuple_0::Thr0,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
      EXPECT_TRUE(m_pThrs[t]->IsOK());

      // So that each thread receives a unique index via m_Scratch[0]
      EXPECT_TRUE(m_Sems[0].Wait());
   }
   m_Scratch[0] = m_Scratch[1] = 0;

   EXPECT_EQ(t, CurrentThreads());

   for ( w = 0 ; w < W ; ++w ) {
      // Verify that all m_Scratch[t] == w. If not, then some thread made it
      //  through Wait() twice without blocking.
      for ( t = 0 ; t < NumThreads ; ++t ) {
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
      for ( t = 0 ; t < NumThreads ; ++t ) {
         ASSERT_EQ(w, m_Scratch[t]);
      }

      // This will unlock the Barrier, beginning the auto-reset.
      EXPECT_TRUE(Post(1));

      // Wait for NumThreads Post()'s to m_Sems[1].
      EXPECT_TRUE(m_Sems[1].Wait());
      EXPECT_TRUE(m_Sems[1].Reset(-((AAL::btInt)NumThreads)));
   }

   // Join() all of the threads.
   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_pThrs[t]->Join();
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());
}

void OSAL_Barrier_vp_tuple_0::Thr1(OSLThread *pThread, void *pArg)
{
   OSAL_Barrier_vp_tuple_0 *f = reinterpret_cast<OSAL_Barrier_vp_tuple_0 *>(pArg);
   ASSERT_NONNULL(f);

   const AAL::btUnsignedInt t = f->m_Scratch[0]; // my unique thread index.

   EXPECT_TRUE(f->m_Sems[0].Post(1));

   EXPECT_TRUE(f->Wait());
   f->m_Scratch[t] = 1;
}

TEST_P(OSAL_Barrier_vp_tuple_0, aal0195)
{
   // When the Barrier is Create()'ed and a thread blocks in Barrier::Wait(void), given that no
   // call to Barrier::Post() nor Barrier::UnblockAll() is made by another thread, the blocked
   // thread waits infinitely, even in the presence of signals.

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   // Register a signal handler for SIGIO so that the process is not
   // reaped as a result of having received an un-handled signal.
   SignalHelper sig;
   ASSERT_EQ(0, sig.Install(SIGIO, SignalHelper::EmptySIGIOHandler, false));
#endif // OS

   AAL::btUnsignedInt NumThreads;
   AAL::btUnsignedInt SignalAttempts;

   std::tr1::tie(NumThreads, SignalAttempts) = GetParam();

   ASSERT_TRUE(Create(1, true));

   // For Thr1 init.
   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   // Count up sem for Wait() loop. This threads Wait()'s. Thr0's Post().
   // ASSERT_TRUE(m_Sems[1].Create(-((AAL::btInt)NumThreads), 1));

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

   AAL::btUnsignedInt s;

   const btTime sleepeach = 5;

   for ( s = 0 ; s < SignalAttempts ; ++s ) {

      for ( t = 0 ; t < NumThreads ; ++t ) {
#if   defined( __AAL_WINDOWS__ )
         FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
         EXPECT_EQ(0, pthread_kill(m_pThrs[t]->tid(), SIGIO));
#endif // OS
      }

      sleep_millis(sleepeach);

      for ( t = 0 ; t < NumThreads ; ++t ) {
         EXPECT_EQ(0, m_Scratch[t]);
      }

      EXPECT_EQ(t, CurrentThreads());
      EXPECT_EQ(t, NumWaiters());
   }

   // Unlock the Barrier, allowing threads to exit.
   EXPECT_TRUE(Post(1));

   for ( t = 0 ; t < NumThreads ; ++t ) {
      m_pThrs[t]->Join();
      EXPECT_EQ(1, m_Scratch[t]);
   }

   EXPECT_EQ(0, CurrentThreads());
   EXPECT_EQ(0, NumWaiters());
}



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














#if 0
TEST(OSAL_Sem, aal0036)
{
   // CSemaphore::Reset(nCount) (created=true, waiters=0, nCount<=Max) is successful.

   CSemaphore *pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(-1, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(0, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(1, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;


   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(-1, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(0, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(1, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;
}

#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_0 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_Sem_vp_tuple_0() {}
   virtual ~OSAL_Sem_vp_tuple_0() {}

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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_0 *pTC = static_cast<OSAL_Sem_vp_tuple_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

   AAL::btInt               i;
   AAL::btUnsignedInt       u;

   // we should be able to consume nMaxCount's from the sem without blocking.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_0, aal0049)
{
   // Calling CSemaphore::Create() with nInitialCount > nMaxCount and nMaxCount > 0
   //  results in an initial count of nMaxCount.

   std::tr1::tie(m_nInitialCount, m_nMaxCount) = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);
   EXPECT_EQ(i, m);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_0,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)5,
                                                             (AAL::btInt)10,
                                                             (AAL::btInt)100
                                                            ),
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)2,
                                                             (AAL::btUnsignedInt)3
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE


#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_1 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_Sem_vp_tuple_1() {}
   virtual ~OSAL_Sem_vp_tuple_1() {}

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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_1::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_1 *pTC = static_cast<OSAL_Sem_vp_tuple_1 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

   AAL::btInt               i;
   AAL::btUnsignedInt       u;

   pTC->m_Scratch[0] = 1;   // signal that we're ready.

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // We should be able to make nMaxCount Posts()'s.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Post(1));
   }

   // Attempting to Post() again should fail.
   EXPECT_FALSE(sem.Post(1));
   pTC->m_Scratch[2] = 1;

   // Wait for the other thread to reset the sem.
   YIELD_WHILE(0 == pTC->m_Scratch[3]);
   pTC->m_Scratch[4] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.

   // We should be able to make nMaxCount Posts()'s.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Post(1));
   }

   // Attempting to Post() again should fail.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[5] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_1, aal0054)
{
   // Calling CSemaphore::Create() with nInitialCount < 0 and
   //  nMaxCount > 0 creates a count up semaphore.

   std::tr1::tie(m_nInitialCount, m_nMaxCount) = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   const AAL::btInt N = -m_nInitialCount;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_1::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // We should be required to Post() abs(m_nInitialCount) times before unblocking A.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Reset the semaphore and repeat.
   ASSERT_TRUE(m_Sem.Reset(m_nInitialCount));

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_Scratch[3] = 1;

   YIELD_WHILE(0 == m_Scratch[4]);

   // We should be required to Post() abs(m_nInitialCount) times before unblocking A.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   // Give A plenty of opportunity to have set scratch[5]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[5]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_1,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)-1,
                                                             (AAL::btInt)-5,
                                                             (AAL::btInt)-10,
                                                             (AAL::btInt)-25
                                                            ),
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)25
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE


#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_2 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btInt > >
{
protected:
   OSAL_Sem_vp_tuple_2() {}
   virtual ~OSAL_Sem_vp_tuple_2() {}

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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_2::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_vp_tuple_2::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_vp_tuple_2::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[5] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_2, aal0059)
{
   // For a created sem, CSemaphore::Post() allows at least one blocking thread
   //  to wake when the semaphore count becomes greater than 0.

   AAL::btInt NumToPost = 0;

   std::tr1::tie(m_nInitialCount, NumToPost) = GetParam();

   m_nMaxCount = 100;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   AAL::btInt i;
   AAL::btInt N = m_nInitialCount;
   if ( N < 0 ) {
      N = -N;
   }

   // We need to Post() the sem N times before any waiters will unblock.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3] + m_Scratch[4] + m_Scratch[5]));

   // Now, this should wake at least one thread.
   EXPECT_TRUE(m_Sem.Post(NumToPost));

   YIELD_WHILE(0 == m_Scratch[3] + m_Scratch[4] + m_Scratch[5]);

   // Give all child threads an opportunity to wake before we continue.
   YIELD_N();

   AAL::btInt woke = 0;

   // Join() the threads that woke.
   for ( i = 0 ; i < 3 ; ++i ) {
      if ( 1 == m_Scratch[3 + i] ) {
         m_pThrs[i]->Join();
         ++m_Scratch[3 + i]; // increment to 2.
         ++woke;
      }
   }

   // The number of waiters that woke must be <= the number posted to the sem.
   ASSERT_LE(woke, NumToPost);

   while ( woke < 3 ) {

      EXPECT_TRUE(m_Sem.Post(NumToPost));

      // Give all child threads an opportunity to wake before we continue.
      YIELD_N();

      for ( i = 0 ; i < 3 ; ++i ) {
         if ( 1 == m_Scratch[3 + i] ) {
            m_pThrs[i]->Join();
            ++m_Scratch[3 + i]; // increment to 2.
            ++woke;
         }
      }

   }
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_2,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)-1,
                                                             (AAL::btInt)0
                                                            ),
                                           ::testing::Values((AAL::btInt)1,
                                                             (AAL::btInt)2,
                                                             (AAL::btInt)3
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE



// Value-parameterized test fixture
template <typename T>
class OSAL_Sem_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_Sem_vp() {}
   virtual ~OSAL_Sem_vp() {}

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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;
};

class OSAL_Sem_vp_int_0 : public OSAL_Sem_vp< AAL::btInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_int_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_int_0 *pTC = static_cast<OSAL_Sem_vp_int_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

   AAL::btInt               i;
   AAL::btUnsignedInt       u;

   // we should be able to consume nInitialCount's from the sem without blocking.
   for ( i = 0 ; i < nInitialCount ; ++i ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // Adjust the count to max.
   ASSERT_TRUE(sem.Post(nInitialCount));

   // Trying to Post() any more should fail.
   ASSERT_FALSE(sem.Post(1));

   // we should be able to consume nInitialCount's from the sem without blocking.
   for ( i = 0 ; i < nInitialCount ; ++i ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[2] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_P(OSAL_Sem_vp_int_0, aal0050)
{
   // Calling CSemaphore::Create() with nInitialCount > 0 and nMaxCount = 0 results in nMaxCount = nInitialCount.

   m_nInitialCount = GetParam();
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m_nInitialCount, i);
   EXPECT_EQ(m, i);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_int_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set scratch[3]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_int_0, ::testing::Values(1, 10, 100, 250));


class OSAL_Sem_vp_int_1 : public OSAL_Sem_vp< AAL::btInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_int_1::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_int_1 *pTC = static_cast<OSAL_Sem_vp_int_1 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

   AAL::btInt               i;
   AAL::btUnsignedInt       u;

   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(sem.Wait());
   pTC->m_Scratch[1] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[2] = 1;

   // Reset and repeat the test.
   EXPECT_TRUE(sem.Reset(nInitialCount));

   EXPECT_TRUE(sem.Wait());
   pTC->m_Scratch[3] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[4] = 1;
}

TEST_P(OSAL_Sem_vp_int_1, aal0052)
{
   // Calling CSemaphore::Create() with nInitialCount < 0 and nMaxCount = 0 results in
   //  nInitialCount = nInitialCount + 1 and nMaxCount = 1.

   m_nInitialCount = GetParam();
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(1, m);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_int_1::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of time to have set m_Scratch[1].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // We should be able to Post() the semaphore abs(m_nInitialCount) times before waking A.
   m = -m_nInitialCount;
   for ( i = 0 ; i < m - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1)); // This should wake A.

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of time to have set m_Scratch[3].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // We should be able to Post() the semaphore abs(m_nInitialCount) times before waking A.
   m = -m_nInitialCount;
   for ( i = 0 ; i < m - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   EXPECT_TRUE(m_Sem.Post(1)); // This should wake A.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[4]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_int_1, ::testing::Values(-1, -10, -100, -250));


class OSAL_Sem_vp_uint_0 : public OSAL_Sem_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_uint_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_uint_0 *pTC = static_cast<OSAL_Sem_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;
   AAL::btUnsignedInt       u;

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // Adjust the count to max.
   ASSERT_TRUE(sem.Post((AAL::btInt)nMaxCount));

   // Trying to Post() any more should fail.
   ASSERT_FALSE(sem.Post(1));

   // we should now be able to consume nMaxCount's from the sem without blocking.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[2] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_P(OSAL_Sem_vp_uint_0, aal0053)
{
   // Calling CSemaphore::Create() with nInitialCount = 0 and nMaxCount > 0 creates
   //  a traditional count down semaphore that is locked.

   m_nInitialCount = 0;
   m_nMaxCount     = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m_nInitialCount, i);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_uint_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set scratch[3]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_uint_0, ::testing::Values(1, 10, 100, 250));


// Simple test fixture
class OSAL_Sem_f : public ::testing::Test
{
protected:
   OSAL_Sem_f() {}
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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

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
};

void OSAL_Sem_f::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   CSemaphore &sem = pTC->m_Sem;

   EXPECT_TRUE(sem.CurrCounts(i, m));
   ASSERT_EQ(0, i);
   ASSERT_EQ(1, m);

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait());  // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   EXPECT_TRUE(sem.Reset(0));

   pTC->m_Scratch[2] = 1;
   ASSERT_TRUE(sem.Wait());  // (0 == count) should block.

   pTC->m_Scratch[3] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[4] = 1;
}

TEST_F(OSAL_Sem_f, aal0051)
{
   // Calling CSemaphore::Create() with nInitialCount = 0 and nMaxCount = 0 results in nInitialCount = 0 and nMaxCount = 1.

   m_nInitialCount = 0;
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set m_Scratch[1].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set m_Scratch[3].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[3]);

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[4]);
}

TEST_F(OSAL_Sem_f, aal0055)
{
   // CSemaphore::Reset(nCount) (created=false, waiters=X, nCount<=X) fails.

   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));

   EXPECT_TRUE(m_Sem.Create(1, 1));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
}

TEST_F(OSAL_Sem_f, aal0056)
{
   // CSemaphore::Reset(nCount) (created=true, waiters=0, nCount>Max) fails.

   EXPECT_TRUE(m_Sem.Create(-1, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(0, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(1, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(-1, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(0, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(1, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());
}

void OSAL_Sem_f::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   CSemaphore &sem = pTC->m_Sem;

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[2] = 1;
}

void OSAL_Sem_f::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   CSemaphore &sem = pTC->m_Sem;

   pTC->m_Scratch[1] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_F(OSAL_Sem_f, aal0057)
{
   // CSemaphore::Reset(nCount) (created=true, waiters>0, nCount<=Max) fails.

   ASSERT_TRUE(m_Sem.Create(0, 2));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);

   // Give the child threads plenty of time to have blocked on the sem.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(0, m_Scratch[2]);


   // Try to Reset() the sem. This should fail.
   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));

   // Give the child threads plenty of time to have woken.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(0, m_Scratch[2]);


   // Post() the sem once, allowing one of the child threads to wake and exit.
   EXPECT_TRUE(m_Sem.Post(1));

   // Wait until the child exits.
   YIELD_WHILE(0 == m_Scratch[2] && 0 == m_Scratch[3]);

   // Determine which thread exited.
   AAL::btInt t = (0 == m_Scratch[2]) ? 1 : 0;

   // Thread t exited.
   m_pThrs[t]->Join();
   t = 1 - t;


   // Repeat the tests with the one remaining child thread.

   // Try to Reset() the sem. This should fail.
   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));

   // Give the child thread plenty of time to have woken.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[t + 2]));

   // Post() the sem once, allowing the last child thread to wake and exit.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[t]->Join();
   EXPECT_EQ(1, m_Scratch[t + 2]);
}

TEST_F(OSAL_Sem_f, aal0038)
{
   // CSemaphore::CurrCounts() retrieves the values of the current and max counters,
   //  when the sem has been created.

   AAL::btInt         i;
   AAL::btUnsignedInt m;

   AAL::btInt         j;
   AAL::btInt         n;

   i = -1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 0; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 1; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 2; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 2; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 2; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());
}

TEST_F(OSAL_Sem_f, aal0039)
{
   // CSemaphore::CurrCounts() returns false when sem not initialized.
   AAL::btInt cur;
   AAL::btInt max;
   EXPECT_FALSE(m_Sem.CurrCounts(cur, max));
}

TEST_F(OSAL_Sem_f, aal0058)
{
   // CSemaphore::Post() returns false when sem not initialized.
   EXPECT_FALSE(m_Sem.Post(1));
}

TEST_F(OSAL_Sem_f, aal0065)
{
   // CSemaphore::Post() does not affect the current count and
   // returns false when (current + postval) > maxcount.

   AAL::btInt         i;
   AAL::btUnsignedInt m;

   AAL::btInt         j;
   AAL::btInt         n;

   i = -1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 0; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 1; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());
}

TEST_F(OSAL_Sem_f, aal0061)
{
   // CSemaphore::UnblockAll() returns false when sem not initialized.
   EXPECT_FALSE(m_Sem.UnblockAll());
}

void OSAL_Sem_f::Thr3(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait()); // (0 == count) this will block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr4(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT)); // (0 == count) this will block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr5(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1))); // (0 == count) this will block.
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0062)
{
   // When the sem has been initialized, CSemaphore::UnblockAll() causes all
   // threads currently blocked in Wait() calls for the sem to wake, returning
   // false from CSemaphore::Wait(). The CSemaphore::UnblockAll() call itself returns true.

   ASSERT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   EXPECT_TRUE(m_Sem.UnblockAll());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

void OSAL_Sem_f::Thr6(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait()); // (0 == count) this will block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr7(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT)); // (0 == count) this will block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr8(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1))); // (0 == count) this will block.
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0066)
{
   // Once all of the threads that were blocked on a CSemaphore::Wait() call have woken
   // and returned false from Wait(), the next call to CSemaphore::Wait() blocks,
   // because the CSemaphore is no longer unblocking. When the CSemaphore is next Post()'ed,
   // the Wait() call returns true.

   ASSERT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr7,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);


   EXPECT_TRUE(m_Sem.UnblockAll());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr8,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give the thread plenty of opportunity to have set m_Scratch[5].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Post(1)); // will wake the thread, allowing it to exit.

   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[5]);
}

void OSAL_Sem_f::Thr9(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait());
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr10(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT));
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr11(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1)));
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0063)
{
   // CSemaphore::NumWaiters() returns the number of threads currently blocked in CSemaphore::Wait() calls.

   ASSERT_TRUE(m_Sem.Create(0, 3));
   EXPECT_EQ(0, m_Sem.NumWaiters());

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr9,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(1, m_Sem.NumWaiters());


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr10,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[4]));
   EXPECT_EQ(2, m_Sem.NumWaiters());


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr11,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));
   EXPECT_EQ(3, m_Sem.NumWaiters());

   AAL::btInt i;
   AAL::btInt j;

   m_Scratch[0] = 0;
   m_Scratch[1] = 0;
   m_Scratch[2] = 0;

   for ( i = 0 ; i < 3 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
      YIELD_WHILE(i == m_Scratch[3] + m_Scratch[4] + m_Scratch[5]);

      EXPECT_EQ(3 - (i + 1), m_Sem.NumWaiters()) << " i=" << i;

      for ( j = 0 ; j < 3 ; ++j ) {
         if ( ( 0 == m_Scratch[j] ) && ( 1 == m_Scratch[3 + j] ) ) {
            m_Scratch[j] = 1;
            m_pThrs[j]->Join();
         }
      }
   }

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

TEST_F(OSAL_Sem_f, aal0064)
{
   // CSemaphore::Wait(btTime ) returns false when the sem is not initialized.
   EXPECT_FALSE(m_Sem.Wait(1000));
}

TEST_F(OSAL_Sem_f, aal0067)
{
   // When the sem is initialized and the time duration given to CSemaphore::Wait(btTime )
   // expires, Wait() returns false.

   EXPECT_TRUE(m_Sem.Create(0, 1));
   EXPECT_FALSE(m_Sem.Wait(1));
}

void OSAL_Sem_f::Thr12(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   // Register a signal handler for SIGIO so that the process is not
   // reaped as a result of having received an un-handled signal.
   SignalHelper sig;
   ASSERT_EQ(0, sig.Install(SIGIO, SignalHelper::EmptySIGIOHandler, false));
#endif // OS

   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(pTC->m_Scratch[2]));
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Sem_f, aal0068)
{
   // When the sem is initialized and a thread blocks in CSemaphore::Wait(btTime X),
   // given that no call to CSemaphore::Post() nor CSemaphore::UnblockAll() is made by
   // another thread, the blocked thread waits at least X milliseconds before resuming,
   // even in the presence of signals.

   m_Scratch[2] = 250; // timeout (millis) for the Wait() call in m_Scratch[2].

   btTime       slept     = 0;
   const btTime sleepeach = 5;
   const btTime thresh    = m_Scratch[2] - ( m_Scratch[2] / 20 ); // within 5%

   EXPECT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr12,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   while ( slept < m_Scratch[2] ) {
      sleep_millis(sleepeach);
      slept += sleepeach;
      if ( slept < thresh ) {
         // we still expect the thread to be blocked.
         EXPECT_EQ(0, m_Scratch[1]);

#if   defined( __AAL_WINDOWS__ )
         FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
         EXPECT_EQ(0, pthread_kill(m_pThrs[0]->tid(), SIGIO));
#endif // OS
      }
   }

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[1]);
}

TEST_F(OSAL_Sem_f, aal0069)
{
   // CSemaphore::Wait(void) returns false when the sem is not initialized.
   EXPECT_FALSE(m_Sem.Wait());
}

void OSAL_Sem_f::Thr13(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   // Register a signal handler for SIGIO so that the process is not
   // reaped as a result of having received an un-handled signal.
   SignalHelper sig;
   ASSERT_EQ(0, sig.Install(SIGIO, SignalHelper::EmptySIGIOHandler, false));
#endif // OS

   pTC->m_Scratch[0] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait());
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Sem_f, aal0070)
{
   // When the sem is initialized and a thread blocks in CSemaphore::Wait(void),
   // given that no call to CSemaphore::Post() nor CSemaphore::UnblockAll() is made
   // by another thread, the blocked thread waits infinitely, even in the presence of signals.

   EXPECT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr13,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   int          i;
   const int    count     = 50;
   const btTime sleepeach = 5;

   for ( i = 0 ; i < count ; ++i ) {
      sleep_millis(sleepeach);
      EXPECT_EQ(0, m_Scratch[1]);

#if   defined( __AAL_WINDOWS__ )
      FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
      EXPECT_EQ(0, pthread_kill(m_pThrs[0]->tid(), SIGIO));
#endif // OS
   }

   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_Sem_f::Thr14(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr15(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr16(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, DISABLED_aal0071)
{
   // CSemaphore::Destroy() should unblock all current waiters and prevent new waiters from blocking.

   EXPECT_TRUE(m_Sem.Create(0, 3));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr14,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr15,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr16,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE((0 == m_Scratch[0]) || (0 == m_Scratch[1]) || (0 == m_Scratch[2]));

   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3] + m_Scratch[4] + m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Destroy());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

/////////////////////////////

class SemBasic : public ::testing::Test
{
protected:
SemBasic() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
   CSemaphore m_Sem;
};

TEST_F(SemBasic, TwoCreates)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
   EXPECT_FALSE(m_Sem.Create(1, INT_MAX)) << "two Create's without an intervening Destroy";
}

TEST_F(SemBasic, DestroyBeforeCreate)
{
   EXPECT_FALSE(m_Sem.Destroy()) << "Destroy before Create";
   EXPECT_FALSE(m_Sem.Reset(1)) << "Reset before Create";

   btInt cur = 0, max = 0;
   EXPECT_FALSE(m_Sem.CurrCounts(cur, max));
   EXPECT_EQ(0, cur);
   EXPECT_EQ(0, max);

   EXPECT_FALSE(m_Sem.Post(1)) << "Post before Create";
   EXPECT_FALSE(m_Sem.Wait(1)) << "Timed Wait before Create";
   EXPECT_FALSE(m_Sem.Wait())  << "Wait before Create";
}

////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
class SemWait : public ::testing::TestWithParam< btInt >
{
protected:
SemWait() {}

virtual void SetUp()
{
   m_pThr  = NULL;
   m_Count = 0;
}

virtual void TearDown()
{
   if ( NULL != m_pThr ) {
      delete m_pThr;
   }

   EXPECT_TRUE(m_Sem.Destroy());
}

static void ThrLoop(OSLThread * , void * );
static void ThrOnce(OSLThread * , void * );

   CSemaphore m_Sem;
   OSLThread *m_pThr;
   btInt m_Count;
};

void SemWait::ThrLoop(OSLThread * /* unused */, void *arg)
{
   SemWait *pFixture = reinterpret_cast<SemWait *>(arg);

   btInt       i;
   const btInt count = pFixture->m_Count;

   pFixture->m_Count = 0;
   for ( i = 0 ; i < count ; ++i ) {
      //SleepMilli(1);
      ++pFixture->m_Count;
      pFixture->m_Sem.Post(1);
   }
}

void SemWait::ThrOnce(OSLThread * /* unused */, void *arg)
{
   SemWait *pFixture = reinterpret_cast<SemWait *>(arg);

   pFixture->m_Sem.Post(pFixture->m_Count);
}

TEST_P(SemWait, InfiniteWait)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));

   btInt       i;
   const btInt count = GetParam();

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrLoop,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_TRUE(m_Sem.Wait()) << "Infinite wait loop";
   }
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";
   m_pThr->Join();


   EXPECT_TRUE(m_Sem.Destroy());


   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrOnce,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_TRUE(m_Sem.Wait()) << "Infinite wait once";
   }
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();
}

TEST_P(SemWait, CountUp)
{
   const btInt count = GetParam();

   EXPECT_TRUE(m_Sem.Create(-count, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrLoop,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   EXPECT_TRUE(m_Sem.Wait(10000)) << "Count up timed out loop";
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();


   EXPECT_TRUE(m_Sem.Destroy());


   EXPECT_TRUE(m_Sem.Create(-count, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrOnce,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   EXPECT_TRUE(m_Sem.Wait(10000)) << "Count up timed out loop";
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MySem, SemWait,
                        ::testing::Range(1, 100, 5));
#endif


