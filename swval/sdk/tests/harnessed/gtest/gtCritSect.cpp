// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

// Simple test fixture
class OSAL_CritSect_f : public ::testing::Test
{
protected:
   OSAL_CritSect_f() {}
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

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CriticalSection     m_CS;

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );
   static void Thr5(OSLThread * , void * );
};

void OSAL_CritSect_f::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_CS.Lock();      // blocks if already locked.
   pTC->m_Scratch[0] = 1; // signal that we're exiting.
}

TEST_F(OSAL_CritSect_f, aal0021)
{
   // A CriticalSection is created in the unlocked state.

/*
   m_CS was constructed in our thread context.
   Spawn a new thread. The new thread will attempt to lock m_CS.
   If m_CS was created in the locked state, then the thread will hang.
*/

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_CritSect_f::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   pTC->m_CS.Lock();
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_CritSect_f, aal0022)
{
   // CriticalSection::Lock() locks the CriticalSection.

/*
Lock the critical section.
Spawn thread A(Thr1), and wait for A to be ready via scratch[0].
A attempts to lock the critical section, but blocks because it's already held.
Seeing that A is running, this thread verifies that scratch[1] remains unset.
This thread unlocks the critical section, waking A.
A sets scratch[1] and exits.
*/

   m_CS.Lock();

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   EXPECT_EQ(0, m_Scratch[1]);

   m_CS.Unlock();

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

TEST_F(OSAL_CritSect_f, aal0023)
{
   // CriticalSection::Lock() can be called recursively by the same owner.

   const unsigned count = 10;
   unsigned       i;

   for ( i = 0 ; i < count ; ++i ) {
      m_CS.Lock();
   }

   for ( i = 0 ; i < count ; ++i ) {
      m_CS.Unlock();
   }
}


void OSAL_CritSect_f::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   pTC->m_CS.Lock();
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_CritSect_f, aal0025)
{
   // CriticalSection::Unlock() releases the critical section when called by the owner when 1 == count.

/*
Lock the critical section twice.
Spawn thread A(Thr2) and wait for it to become ready via scratch[0].
A starts, sets scratch[0], then waits for the critical section, but blocks.

Seeing that A has set scratch[0], this thread verifies that scratch[1] remains unset.
This thread unlocks the critical section once, then enters a polling loop for N tries, verifying
 that scratch[1] remains unset. The cpu is yielded once for each poll.

After N verifications that A is still blocked, this thread unlocks the critical section and waits
 for A to exit.
*/

   m_CS.Lock();
   m_CS.Lock();

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   EXPECT_EQ(0, m_Scratch[1]);

   m_CS.Unlock(); // We still hold 1 count on the critical section.

   const unsigned count = 1000;
   unsigned       i;

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_EQ(0, m_Scratch[1]);
      cpu_yield();
   }

   m_CS.Unlock(); // This will allow A to unblock and exit.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_CritSect_f::Thr5(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   pTC->m_CS.Lock();
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_CritSect_f, aal0027)
{
   // When a CriticalSection is unlocked, calling Unlock() on it does not cause
   // the counter to decrement past 0.

/*
Critical sections are created in the unlocked state.
Unlock() the critical section again. This should have no affect on the counter.
Lock() the critical section. The critical section should be locked by this thread.

Spawn a new thread A(Thr5), and wait for it to become ready via scratch[0].

Thread A starts, sets scratch[0], and attempts to lock the critical section, but blocks, because
 it is already locked.

This thread enters a polling loop for N counts, verifying that scratch[1] remains clear, yielding
 the cpu for each iteration. This is to give A a chance to lock the critical section, set scratch[1],
 and exit, in the case that the critical section counter was allowed to go below 0.

Verifying that A has not exited, this thread unlocks the critical section and waits for A to exit.
*/

   // The critical section was created unlocked. Unlock it again.
   m_CS.Unlock();

   // Now, lock it. We should now hold the critical section, because the counter should not
   // be decremented past 0.
   m_CS.Lock();

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   const unsigned count = 100;
   unsigned       i;

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_EQ(0, m_Scratch[1]);
      cpu_yield();
   }

   m_CS.Unlock(); // This will wake A and allow it to exit.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_CritSect_f::Thr3(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   pTC->m_CS.Lock();
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_CritSect_f, aal0028)
{
   // When a CriticalSection is held by a thread and that same thread calls
   // CriticalSection::TryLock(), the CriticalSection's counter is incremented.

   // The man page for pthread_mutex_trylock() says:
   // the mutex type is PTHREAD_MUTEX_RECURSIVE and the mutex is currently owned by the calling
   // thread, the mutex lock count shall be incremented by one and the pthread_mutex_trylock()
   // function shall immediately return success.


/*
This thread locks the critical section.
Thread A(Thr3) is spawned.
Once running, A sets scratch[0] and then attempts to lock the critical section, but blocks.

Seeing that A has set scratch[0], this thread calls TryLock(), verifying that the return value is true.
This thread then calls Unlock() once, and enters a loop verifying for N tries that scratch[1] remains
 clear, yielding the cpu for each iteration.

This thread then calls Unlock() once again to wake A and allow it to exit.
*/

   m_CS.Lock();

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   EXPECT_EQ(0, m_Scratch[1]);

   EXPECT_TRUE(m_CS.TryLock());

   m_CS.Unlock(); // The critical section should still be locked.

   const unsigned count = 100;
   unsigned       i;

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_EQ(0, m_Scratch[1]);
      cpu_yield();
   }

   m_CS.Unlock(); // This will allow A to unblock and exit.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_CritSect_f::Thr4(OSLThread *pThread, void *pContext)
{
   OSAL_CritSect_f *pTC = static_cast<OSAL_CritSect_f *>(pContext);
   ASSERT_NONNULL(pTC);

   EXPECT_FALSE(pTC->m_CS.TryLock());
   pTC->m_Scratch[0] = 1;

   pTC->m_CS.Lock();
   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_CritSect_f, aal0042)
{
   // When a CriticalSection is held by a thread and another thread calls
   // CriticalSection::TryLock(), the CriticalSection's counter remains unaffected.

/*
This thread locks the critical section, then spawns a thread A(Thr4).
This thread waits for A to become ready via scratch[0].

Once running, A calls TryLock() and verifies that it returns false, because the critical section is held.
A then sets scratch[0] to signal that it is ready, and calls Lock() on the critical section to block.

Seeing scratch[0], this thread enters a polling loop, verifying that scratch[1] remains unset, yielding
 the cpu for each iteration.

Upon exiting the loop, this thread Unlock()'s the critical section and waits for A to exit.
This one call to Unlock() must relinquish the critical section to A, or else the counter was affected
 by TryLock().
*/

   m_CS.Lock();

   m_pThrs[0] = new OSLThread(OSAL_CritSect_f::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   const unsigned count = 100;
   unsigned       i;

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_EQ(0, m_Scratch[1]);
      cpu_yield();
   }

   m_CS.Unlock(); // This will allow A to unblock and exit.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

TEST_F(OSAL_CritSect_f, DestroyWhileLocked)
{
   // What happens when a critical section is destroyed with a non-zero lock count?
   m_CS.Lock();
}

