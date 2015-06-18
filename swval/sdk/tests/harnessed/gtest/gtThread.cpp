// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#ifdef __AAL_LINUX__
# include <time.h>       // struct timespec, nanosleep()
#endif // __AAL_LINUX__


TEST(OSAL_Thread, aal0000) {
   // GetProcessID() returns the pid of the current process.

   btPID pid = (btPID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentProcessId();
#elif defined( __AAL_LINUX__ )
   getpid();
#endif

   ASSERT_EQ(pid, GetProcessID());
}

TEST(OSAL_Thread, aal0001) {
   // GetThreadID() returns the tid of the current thread.

   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   ASSERT_EQ(tid, GetThreadID());
}



TEST(OSAL_Thread, aal0003) {
   // GetRand() returns a pseudo-random 32-bit integer.

/*
In general, this is a very difficult problem. We don't want to dive into the domain of
verifying the underlying implementation of the RNG. We just want a basic sanity check to
ensure that we're calling the RNG correctly.
*/
   const int          count = 50;
   btUnsigned32bitInt randoms[count];
   btUnsigned32bitInt seed = 1234;

   // We should have no repeated values in count calls to the RNG.
   int i;
   int j;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);

      for ( j = 0 ; j < i ; ++j ) {
         EXPECT_NE(r, randoms[j]);
      }

      randoms[i] = r;
   }
}


TEST(OSAL_Thread, aal0005) {
   // The GetRand() sequence is repeatable for a given input seed value.

/*
Another difficult problem to solve completely. Looking for a basic sanity check here.
*/

   const int          count = 50;
   btUnsigned32bitInt randoms[count];
   btUnsigned32bitInt seed = 1234;
   btUnsigned32bitInt save_seed = seed;

   int i;
   int j;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);

      for ( j = 0 ; j < i ; ++j ) {
         EXPECT_NE(r, randoms[j]);
      }

      randoms[i] = r;
   }

   seed = save_seed;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);
      EXPECT_EQ(r, randoms[i]);
   }
}

TEST(OSAL_Thread, aal0006) {
   // CurrentThreadID::getTID() returns the tid of the current thread.
   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   CurrentThreadID thrid;
   ASSERT_EQ(tid, thrid.getTID());
}

TEST(OSAL_Thread, aal0007) {
   // CurrentThreadID::operator btTID () returns the tid of the current thread.
   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   CurrentThreadID thrid;
   ASSERT_EQ(tid, (AAL::btTID)thrid);
   ASSERT_EQ(tid, thrid.operator AAL::btTID());
}

TEST(OSAL_Thread, aal0008)
{
   // OSLThread::OSLThread() gracefully handles a NULL pProc.
   OSLThread t(NULL, OSLThread::THREADPRIORITY_NORMAL, NULL);
   EXPECT_FALSE(t.IsOK());
}

// Value-parameterized test fixture
class OSAL_Thread_vp_bool : public ::testing::TestWithParam< AAL::btBool >
{
protected:
   OSAL_Thread_vp_bool() {}
   virtual ~OSAL_Thread_vp_bool() {}

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
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThread *m_pThrs[3];
   btUIntPtr  m_Scratch[10];

   static void Thr0(OSLThread *pThread, void *pContext);
   static void Thr1(OSLThread *pThread, void *pContext);
   static void Thr2(OSLThread *pThread, void *pContext);
   static void Thr3(OSLThread *pThread, void *pContext);
};

void OSAL_Thread_vp_bool::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_vp_bool *pTC = static_cast<OSAL_Thread_vp_bool *>(pContext);
   ASSERT_NONNULL(pTC);

   EXPECT_NONNULL(pThread);
   /* EXPECT_EQ(pThread, pTC->m_pThrs[0]); Not a valid check - the assignment may not have completed. */

   ++(pTC->m_Scratch[0]);
}

TEST_P(OSAL_Thread_vp_bool, aal0009)
{
   // OSLThread::OSLThread() when given an invalid nPriority.
   AAL::btBool ThisThread = GetParam();

   OSLThread::ThreadPriority invalid;

   invalid = (OSLThread::ThreadPriority) (OSLThread::THREADPRIORITY_COUNT + 100);

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr0,
                              invalid,
                              this,
                              ThisThread);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[0]);

   delete m_pThrs[0];


   invalid = OSLThread::THREADPRIORITY_INVALID;

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr0,
                              invalid,
                              this,
                              ThisThread);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(2, m_Scratch[0]);

   delete m_pThrs[0];
}

void OSAL_Thread_vp_bool::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_vp_bool *pTC = static_cast<OSAL_Thread_vp_bool *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[0] = GetThreadID();
   pTC->m_Scratch[1] = pThread->tid();

#if defined( __AAL_LINUX__ )
   pTC->m_Scratch[2] = pthread_self();
#endif // OS

}

TEST_P(OSAL_Thread_vp_bool, aal0018)
{
   // OSLThread::tid() returns the recipient thread's tid.
   AAL::btBool ThisThread = GetParam();

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              ThisThread);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   delete m_pThrs[0];

   ASSERT_EQ(m_Scratch[0], m_Scratch[1]);
   ASSERT_NE(0, m_Scratch[0]);

#if defined( __AAL_LINUX__ )
   ASSERT_EQ(m_Scratch[0], m_Scratch[2]);
#endif // OS
}

#if 0
SetThreadPriority() was defeatured.
void OSAL_Thread_vp_bool::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_vp_bool *pTC = static_cast<OSAL_Thread_vp_bool *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

   struct sched_param sp;
   int policy;

   ASSERT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
   pTC->m_Scratch[0] = sp.sched_priority;

#endif // OS

   SetThreadPriority(OSLThread::THREADPRIORITY_INVALID);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

   ASSERT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
   pTC->m_Scratch[1] = sp.sched_priority;

#endif // OS

   SetThreadPriority(OSLThread::THREADPRIORITY_COUNT);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

   ASSERT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
   pTC->m_Scratch[2] = sp.sched_priority;

#endif // OS

   pTC->m_Scratch[3] = 1;
}

TEST_P(OSAL_Thread_vp_bool, aal0019) {
   // SetThreadPriority() gracefully handles an invalid nPriority value.
   AAL::btBool ThisThread = GetParam();

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              ThisThread);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(m_Scratch[0], m_Scratch[1]);
   EXPECT_EQ(m_Scratch[0], m_Scratch[2]);
}
#endif

#if 0
SetThreadPriority() was defeatured.
void OSAL_Thread_vp_bool::Thr3(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_vp_bool *pTC = static_cast<OSAL_Thread_vp_bool *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

   struct sched_param sp;
   int policy;

   ASSERT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
   pTC->m_Scratch[0] = sp.sched_priority;

#endif // OS

   OSLThread::ThreadPriority pri;
   unsigned i;
   for ( pri = OSLThread::THREADPRIORITY_FIRST, i = 1 ;
            pri <= OSLThread::THREADPRIORITY_LAST ;
               pri = (OSLThread::ThreadPriority)((int)pri + 1), ++i ) {
      SetThreadPriority(pri);

#if   defined( __AAL_WINDOWS__ )
      FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

      ASSERT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
      pTC->m_Scratch[i] = sp.sched_priority;

#endif // OS
   }

}

TEST_P(OSAL_Thread_vp_bool, DISABLED_aal0020) {
   // SetThreadPriority() adjusts the priority of the current thread.
   AAL::btBool ThisThread = GetParam();

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              ThisThread);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   delete m_pThrs[0];

   OSLThread::ThreadPriority pri;
   unsigned i;
   for ( pri = OSLThread::THREADPRIORITY_FIRST, i = 1 ;
            pri <= OSLThread::THREADPRIORITY_LAST ;
               pri = (OSLThread::ThreadPriority)((int)pri + 1), ++i ) {

      if ( OSLThread::THREADPRIORITY_NORMAL == pri ) {
         EXPECT_EQ(m_Scratch[0], m_Scratch[i]);
      } else {
         EXPECT_NE(m_Scratch[0], m_Scratch[i]);
      }

   }
}
#endif

#if defined( __AAL_LINUX__ )
TEST(LinuxPthreads, DISABLED_SetPriority) {

   pthread_attr_t attr;
   int            policy = 0;

   EXPECT_EQ(0, pthread_attr_init(&attr));
   EXPECT_EQ(0, pthread_attr_getschedpolicy(&attr, &policy));
   EXPECT_EQ(0, pthread_attr_destroy(&attr));

   int max_priority = sched_get_priority_max(policy);

   EXPECT_EQ(0, pthread_setschedprio(pthread_self(), max_priority));

   // EXPECT_TRUE(false) << "max priority was " << max_priority;

#if 0
   struct sched_param sp;
   int                policy;
   int                pri;

   memset(&sp, 0, sizeof(struct sched_param));
   policy = 0;

   EXPECT_EQ(0, pthread_getschedparam(pthread_self(), &policy, &sp));
   pri = sp.sched_priority;

   policy = SCHED_RR;

   int i;
   for ( i = 5 ; i >= 0 ; --i ) {
      sp.sched_priority = 5;

      EXPECT_EQ(0, pthread_setschedparam(pthread_self(), policy, &sp)) <<
               "failed to set new priority to " << i << " (previous was " << pri <<
               " previous policy was " << policy <<  " error: " << strerror(errno);
   }
#endif
}
#endif // __AAL_LINUX__

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Thread_vp_bool,
                        ::testing::Bool());


// Simple test fixture
class OSAL_Thread_f : public ::testing::Test
{
protected:
   OSAL_Thread_f() {}
   virtual ~OSAL_Thread_f() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
         m_TIDs[i]  = 0;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }

   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);
      m_Semaphore.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return GlobalTestConfig::GetInstance().CurrentThreads(); }

   OSLThread  *m_pThrs[3];
   btTID       m_TIDs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Semaphore;

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
   static void Thr19(OSLThread * , void * );
   static void Thr20(OSLThread * , void * );
   static void Thr21(OSLThread * , void * );
   static void Thr22(OSLThread * , void * );
   static void Thr23(OSLThread * , void * );
};

void OSAL_Thread_f::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = GetThreadID();
}

TEST_F(OSAL_Thread_f, aal0010)
{
   // OSLThread - verify that pProc is executed in the current thread when ThisThread == true.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(m_Scratch[0], GetThreadID());

   delete m_pThrs[0];
}

TEST_F(OSAL_Thread_f, aal0040)
{
   // OSLThread - verify that pProc is executed in a separate thread when ThisThread == false.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_NE(m_Scratch[0], GetThreadID());
   EXPECT_NE(0, m_Scratch[0]);

   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // OS

   pTC->m_Scratch[0] = 1; // set the first scratch space to 1 to signify that we're running.

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )

   struct timespec ts;

   ts.tv_sec  = 10 * 60;
   ts.tv_nsec = 0;

   // go to sleep for a long time - our parent thread is going to kill us. =O
   int res = nanosleep(&ts, NULL);
   EXPECT_EQ(-1, res);
   EXPECT_EQ(EINTR, errno);

#endif // OS

   pTC->m_Scratch[1] = 1; // set the 2nd scratch space to 1 to signify that we're exiting.
}

TEST_F(OSAL_Thread_f, aal0011)
{
   // OSLThread::Unblock() causes the recepient thread to be canceled (false==ThisThread).

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   // wait for Thr1 to begin.
   YIELD_WHILE(0 == m_Scratch[0]);

   // Thr1 has started and is going to be sleeping.
   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_X(5);

   m_pThrs[0]->Unblock();

   // wait for Thr1 to return.
   YIELD_WHILE(0 == m_Scratch[1]);

   m_pThrs[0]->Join();
   delete m_pThrs[0];

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
#endif // __AAL_LINUX__
}

void OSAL_Thread_f::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);
   pThread->Unblock();
   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0041)
{
   // OSLThread::Unblock() is a do-nothing operation when true==ThisThread.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Unblock();
   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[0]);

   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr3(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[0] = 1; // A is ready.

   pTC->m_pThrs[1]->Wait();
   pTC->m_Scratch[3] = 1; // A woke once.

   pTC->m_pThrs[1]->Wait();
   pTC->m_Scratch[4] = 1; // A woke twice.
}

void OSAL_Thread_f::Thr4(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[1] = 1; // B is ready.

   // wait for the test to be done.
   while ( 0 == pTC->m_Scratch[2] ) {
      cpu_yield();
   }

   pTC->m_Scratch[1] = 1; // B is done.
}

TEST_F(OSAL_Thread_f, aal0012)
{
   // OSLThread::Signal() posts a single count to the recipient's internal synchronization object.

/*
   This thread is the controller thread.
   Spawn 2 threads A(Thr3) and B(Thr4).
   Wait on scratch[0] for A and on scratch[1] for B to become ready.

   B sits in a spin loop on scratch[2], yielding the cpu.
   A waits infinitely (OSLThread::Wait()) for B to become signaled.
   The controller signals B, by issuing an OSLThread::Signal().
   As a result, A wakes, and sets scratch[3] indicating that it woke once.
     A's Wait() call on B takes B's internal count back down to 0.
     A immediately OSLThread::Wait()'s for B again.

   Seeing scratch[3] as set by A, the controller wakes and verifies that scratch[4] is still 0.
     If scratch[4] is non-zero, then A was allowed to wake from multiple OSLThread::Wait() calls. <- failing case

   The controller sets scratch[2], indicating that B should exit.

   Seeing scratch[2] set, B sets scratch[1] and exits.


*/

  m_pThrs[1] = new OSLThread(OSAL_Thread_f::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[1]->IsOK());

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE((0 == m_Scratch[0]) || (0 == m_Scratch[1]));
   m_Scratch[0] = m_Scratch[1] = 0;

   // Give A plenty of time to have set m_Scratch[3].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // Post one count to B. A will wake and consume the count.
   m_pThrs[1]->Signal();

   // Wait for A to wake.
   YIELD_WHILE(0 == m_Scratch[3]);

   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[4]));

   ASSERT_EQ(0, m_Scratch[2]); // We haven't told B to exit.
   ASSERT_EQ(0, m_Scratch[1]); // B should not have exited.

   // Tell B to exit.
   m_Scratch[2] = 1;

   // Wait for B to exit.
   YIELD_WHILE(0 == m_Scratch[1]);

   m_pThrs[1]->Join();
   delete m_pThrs[1];

   // Destructing B will Post its internal sem, causing A to wake and exit.
   YIELD_WHILE(0 == m_Scratch[4]);

   m_pThrs[0]->Join();
   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr5(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_TIDs[0] = GetThreadID();

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   btUnsignedInt idx = SignalHelper::GetInstance().ThreadRegister(pTC->m_TIDs[0]);
#endif // OS

   // wait for B to be active.
   YIELD_WHILE(0 == pTC->m_Scratch[1]);

   pTC->m_Scratch[2] = 1; // both threads are ready.

   do
   {
      pTC->m_pThrs[1]->Wait();
      ++pTC->m_Scratch[4];
   }while( 0 == pTC->m_Scratch[3] );

#if defined( __AAL_LINUX__ )
   YIELD_WHILE(SignalHelper::GetInstance().GetCount(SignalHelper::IDX_SIGUSR1, idx) < pTC->m_Scratch[0]);
#endif // OS

   sleep_millis(25);
   pTC->m_Scratch[5] = 1;
}

void OSAL_Thread_f::Thr6(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__

   pTC->m_Scratch[1] = 1; // signal that we're ready.

   // keep this thread alive until it's time to stop the test.
   YIELD_WHILE(0 == pTC->m_Scratch[3]);
}

TEST_F(OSAL_Thread_f, DISABLED_aal0013)
{
   // OSLThread::Wait(void) is an infinite wait, even in the presence of signals.

/*
   This thread is the controller thread.
   Spawn 2 threads A(Thr5) and B(Thr6).
   A installs a signal handler for SIGUSR1, then polls for B to become ready.

   Once A and B are both ready..
      A enters a loop, calling B->Wait() and incrementing scratch[4] after each B->Wait() call.
      B enters a loop, yielding the cpu, waiting for scratch[3] to become non-zero.

   The controller thread sends A a number of SIGUSR1 signals, yielding the cpu after each.
   Once the controller has sent all of the signals to A, the controller sets scratch[3] to 1, then
      waits for A and B to exit.

   The controller Join()'s then delete's A and B.

   The controller verifies that scratch[4] is exactly 1.
      scratch[4] must at least be 1, because the controller called B->Signal() to wake A.
      if scratch[4] is greater than 1, then A woke before it was told.
*/

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__


   // Number of signals in m_Scratch[0].
   m_Scratch[0] = 1;

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[1] = new OSLThread(OSAL_Thread_f::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[1]->IsOK());

   // Wait for the threads to become ready.
   YIELD_WHILE(0 == m_Scratch[2]);

#if defined( __AAL_LINUX__ )
   btUnsignedInt idx = SignalHelper::GetInstance().ThreadLookup(m_TIDs[0]);
#endif // __AAL_LINUX__

   // A is blocked in Wait() for B - send A signals to try to wake it.
   btUIntPtr i = 0;
   while ( i < m_Scratch[0] ) {

#if   defined( __AAL_WINDOWS__ )
      FAIL() << "implement for windows";
#elif defined( __AAL_LINUX__ )

      EXPECT_EQ(0, pthread_kill(m_TIDs[0], SIGUSR1));
      sleep_millis(10);

      i = SignalHelper::GetInstance().GetCount(SignalHelper::IDX_SIGUSR1, idx);

      ASSERT_EQ(0, m_Scratch[4]);

#endif // OS
   }

   sleep_millis(10);

   m_Scratch[3] = 1;     // Join the child threads.
   m_pThrs[1]->Signal();
   YIELD_WHILE(0 == m_Scratch[5]);

   //m_pThrs[0]->Join();
   delete m_pThrs[0];

   m_pThrs[1]->Join();
   delete m_pThrs[1];

   ASSERT_EQ(1, m_Scratch[4]); // A should have woken only once.

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
#endif // __AAL_LINUX__
}

void OSAL_Thread_f::Thr7(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_TIDs[0] = GetThreadID();

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   btUnsignedInt idx = SignalHelper::GetInstance().ThreadRegister(pTC->m_TIDs[0]);
#endif // OS

   // wait for B to be active before we try to wait for it.
   YIELD_WHILE(0 == pTC->m_Scratch[1]);

   pTC->m_Scratch[2] = 1; // both threads are ready.

   do
   {
      pTC->m_pThrs[1]->Wait(5000);
      ++pTC->m_Scratch[4];
   }while( 0 == pTC->m_Scratch[3] );

#if defined( __AAL_LINUX__ )
   YIELD_WHILE(SignalHelper::GetInstance().GetCount(SignalHelper::IDX_SIGUSR1, idx) < pTC->m_Scratch[0]);
#endif // OS

   sleep_millis(25);
   pTC->m_Scratch[5] = 1;
}

void OSAL_Thread_f::Thr8(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__

   pTC->m_Scratch[1] = 1; // signal that we're ready.

   // keep this thread alive until it's time to stop the test.
   YIELD_WHILE(0 == pTC->m_Scratch[3]);
}

TEST_F(OSAL_Thread_f, DISABLED_aal0014)
{
   // OSLThread::Wait(btTime ) waits for at least the given number of milliseconds, even in the presence of signals.

/*
   This thread is the controller thread.
   Spawn 2 threads A(Thr7) and B(Thr8).
   A installs a signal handler for SIGUSR1, then polls for B to become ready.

   Once A and B are both ready..
      A enters a loop, calling B->Wait(1 second) and incrementing scratch[4] after each B->Wait(1 second) call.
      B enters a loop, yielding the cpu, waiting for scratch[2] to become non-zero.

   The controller thread sends A a number of SIGUSR1 signals, yielding the cpu after each.
   Once the controller has sent all of the signals to A, the controller sets scratch[2] to 1, then
      waits for B to exit via scratch[3].

   Seeing scratch[2], B sets scratch[3] and exits.

   A's call to B->Wait(1 second) is allowed to time out.

   A wakes, increments scratch[4], sees that scratch[3] has been set, and breaks from the B->Wait(1 second) loop.
   A resets the default signal handler for SIGUSR1, then exits.

   The controller Join()'s then delete's A and B.

   The controller verifies that scratch[4] is exactly 1.
      scratch[4] must at least be 1, because the A's B->Wait(1 second) call timed out, causing A to wake.
      if scratch[4] is greater than 1, then A woke before the timeout.
*/

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__

   // Number of signals in m_Scratch[0].
   m_Scratch[0] = 1;

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr7,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[1] = new OSLThread(OSAL_Thread_f::Thr8,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[1]->IsOK());

   // Wait for the threads to become ready.
   YIELD_WHILE(0 == m_Scratch[2]);

#if defined( __AAL_LINUX__ )
   btUnsignedInt idx = SignalHelper::GetInstance().ThreadLookup(m_TIDs[0]);
#endif // __AAL_LINUX__

   // A is blocked in Wait() for B - send A signals to try to wake it.
   btUIntPtr i = 0;
   while ( i < m_Scratch[0] ) {

#if   defined( __AAL_WINDOWS__ )
      FAIL() << "implement for windows";
#elif defined( __AAL_LINUX__ )

      EXPECT_EQ(0, pthread_kill(m_TIDs[0], SIGUSR1));
      sleep_millis(10);

      i = SignalHelper::GetInstance().GetCount(SignalHelper::IDX_SIGUSR1, idx);

      ASSERT_EQ(0, m_Scratch[4]);

#endif // OS
   }

   sleep_millis(10);

   m_Scratch[3] = 1;     // Join both child threads.
   m_pThrs[1]->Signal();

   YIELD_WHILE(0 == m_Scratch[5]);

   //m_pThrs[0]->Join();
   delete m_pThrs[0];

   m_pThrs[1]->Join();
   delete m_pThrs[1];

   ASSERT_EQ(1, m_Scratch[4]);

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
#endif // __AAL_LINUX__
}

void OSAL_Thread_f::Thr9(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[0] = 1; // signal that we're ready.

   while ( 0 == pTC->m_Scratch[1] ) {
      cpu_yield();
   }

   pTC->m_Scratch[2] = 1;
}

TEST_F(OSAL_Thread_f, aal0015)
{
   // OSLThread::Join() waits for the target thread to end normally.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr9,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   // Wait for A to become ready.
   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   // Prove that A has not exited.
   EXPECT_EQ(0, m_Scratch[2]);

   // Tell A to stop.
   m_Scratch[1] = 1;
   m_pThrs[0]->Join();

   // We should not see m_Scratch[2] set until after the Join().
   EXPECT_EQ(1, m_Scratch[2]);
   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr10(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   btUnsignedInt idx = SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // OS

   pTC->m_Scratch[0] = 1; // set the first scratch space to 1 to signify that we're running.

   pTC->m_Semaphore.Wait();

#if defined( __AAL_LINUX__ )
   YIELD_WHILE(SignalHelper::GetInstance().GetCount(SignalHelper::IDX_SIGIO, idx) < 1);
#endif // OS

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, DISABLED_aal0016)
{
   // OSLThread::Join() - what is expected behavior of Join() when the target thread is OSLThread::Unblock()'ed?

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
   SignalHelper::GetInstance().ThreadRegister(GetThreadID());
#endif // __AAL_LINUX__

   ASSERT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr10,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // wait for Thr10 to begin.
   YIELD_WHILE(0 == m_Scratch[0]);

   // reap the child thread.
   m_pThrs[0]->Unblock();

   // wait for Thr1 to return.
   YIELD_WHILE(0 == m_Scratch[1]);

   // Don't attempt to join Thr10. ~OSLThread will detach then cancel it.
   delete m_pThrs[0];

#if defined( __AAL_LINUX__ )
   SignalHelper::GetInstance().RegistryReset();
#endif // __AAL_LINUX__
}

void OSAL_Thread_f::Thr11(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1; // set the first scratch space to 1 to signify that we're running.
   pTC->m_Semaphore.Wait();
}

TEST_F(OSAL_Thread_f, DISABLED_aal0017)
{
   // After calling OSLThread::Cancel() for a thread other than the current thread, the current
   // thread should be able to immediately OSLThread::Join() the canceled thread.

   ASSERT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr11,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // wait for Thr10 to begin.
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(5);

   // cancel the child thread.
   m_pThrs[0]->Cancel();

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Join();
   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr12(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0065)
{
   // OSLThread::Join() behaves robustly when true == ThisThread.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr12,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr13(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0164)
{
   // OSLThread::Join() safeguards against multiple Join().

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr13,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   m_pThrs[0]->Join();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr14(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(pTC->m_Semaphore.Wait());

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, aal0165)
{
   // OSLThread::Join() safeguards against Join() after Detach().

   EXPECT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr14,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   m_pThrs[0]->Detach();

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Join();

   YIELD_WHILE(0 == m_Scratch[1]);

   EXPECT_EQ(1, m_Scratch[0]);
   EXPECT_EQ(1, m_Scratch[1]);

   YIELD_WHILE(CurrentThreads() > 0);
   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr15(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0166)
{
   // OSLThread::Detach() behaves robustly when true == ThisThread.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr15,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Detach();
   // We still need to delete here, because StartThread was called from the constructor.
   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr16(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(pTC->m_Semaphore.Wait());

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, aal0167)
{
   // OSLThread::Detach() safeguards against multiple Detach().

   EXPECT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr16,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   m_pThrs[0]->Detach();
   m_pThrs[0]->Detach();

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Detach();

   YIELD_WHILE(0 == m_Scratch[1]);

   EXPECT_EQ(1, m_Scratch[0]);
   EXPECT_EQ(1, m_Scratch[1]);

   YIELD_WHILE(CurrentThreads() > 0);
   delete m_pThrs[0];
}

void OSAL_Thread_f::Thr17(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0168)
{
   // OSLThread::Detach() safeguards against Detach() after Join().

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr17,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   m_pThrs[0]->Detach();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr18(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0169)
{
   // OSLThread::Cancel() behaves robustly when true == ThisThread.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr18,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Cancel();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr19(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
}

TEST_F(OSAL_Thread_f, aal0170)
{
   // OSLThread::Cancel() safeguards against Cancel() after Join().

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr19,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   m_pThrs[0]->Cancel();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
}

void OSAL_Thread_f::Thr20(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(pTC->m_Semaphore.Wait());

#ifdef __AAL_LINUX__
   // The following may be redundant, as pthread_cond_wait() is a cancellation point.
   // Such assumptions are dangerous, however, thus the following.
   pthread_testcancel();
#endif // __AAL_LINUX__

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, DISABLED_aal0171)
{
   // OSLThread::Cancel() cancels the thread according to the cancellation policy
   // for pthread_cancel() [Linux].

   EXPECT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr20,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(5);

   m_pThrs[0]->Cancel();

   EXPECT_TRUE(m_Semaphore.Post(1));

   YIELD_WHILE(CurrentThreads() > 0);

   delete m_pThrs[0];

   EXPECT_EQ(0, m_Scratch[1]);
}

void OSAL_Thread_f::Thr21(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_TIDs[1]    = GetThreadID();
   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(pTC->m_Semaphore.Wait());

   EXPECT_TRUE(ThreadIDEqual(pTC->m_TIDs[0], pTC->m_TIDs[0]));
   EXPECT_TRUE(ThreadIDEqual(pTC->m_TIDs[1], pTC->m_TIDs[1]));

   EXPECT_TRUE(ThreadIDEqual(GetThreadID(),  pTC->m_TIDs[1]));
   EXPECT_TRUE(ThreadIDEqual(pThread->tid(), pTC->m_TIDs[1]));

   EXPECT_FALSE(ThreadIDEqual(pTC->m_TIDs[0], pTC->m_TIDs[1]));

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, aal0200)
{
   // ThreadIDEqual() returns true if the two arguments identify the same thread,
   // and false otherwise.

   m_TIDs[0] = GetThreadID();

   EXPECT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr21,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_TRUE(ThreadIDEqual(m_TIDs[0], m_TIDs[0]));
   EXPECT_TRUE(ThreadIDEqual(m_TIDs[1], m_TIDs[1]));

   EXPECT_TRUE(ThreadIDEqual(GetThreadID(), m_TIDs[0]));
   EXPECT_TRUE(ThreadIDEqual(m_pThrs[0]->tid(), m_TIDs[1]));

   EXPECT_FALSE(ThreadIDEqual(m_TIDs[0], m_TIDs[1]));

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Join();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_Thread_f::Thr22(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_TIDs[1]    = GetThreadID();
   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(pTC->m_Semaphore.Wait());

   EXPECT_FALSE(pThread->IsThisThread(pTC->m_TIDs[0]));
   EXPECT_TRUE(pThread->IsThisThread(pTC->m_TIDs[1]));
   EXPECT_TRUE(pThread->IsThisThread(pThread->tid()));

   pTC->m_Scratch[1] = 1;
}

TEST_F(OSAL_Thread_f, aal0201)
{
   // OSLThread::IsThisThread() returns true if the argument matches the thread ID of the
   // OSLThread object, false otherwise.

   m_TIDs[0] = GetThreadID();

   EXPECT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr22,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   EXPECT_FALSE(m_pThrs[0]->IsThisThread(m_TIDs[0]));
   EXPECT_TRUE(m_pThrs[0]->IsThisThread(m_TIDs[1]));
   EXPECT_TRUE(m_pThrs[0]->IsThisThread(m_pThrs[0]->tid()));

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Join();

   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[1]);
}

void OSAL_Thread_f::Thr23(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;

   ExitCurrentThread(pTC->m_Scratch[1]);

   pTC->m_Scratch[2] = 1;
}

TEST_F(OSAL_Thread_f, aal0202)
{
   // ExitCurrentThread() causes the current thread to exit, with the given exit status.

   m_Scratch[1] = 0xdeadbeef;

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr23,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   delete m_pThrs[0];

   EXPECT_EQ(1, m_Scratch[0]);
   EXPECT_EQ(0, m_Scratch[2]);
}

