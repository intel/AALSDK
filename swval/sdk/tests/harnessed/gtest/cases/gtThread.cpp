// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef __AAL_LINUX__
# include <time.h>       // struct timespec, nanosleep()
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <signal.h>
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


// Value-parameterized test fixture
class OSAL_Thread_vp_bool : public ::testing::TestWithParam< AAL::btBool >
{
protected:
   OSAL_Thread_vp_bool() {}

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
   // virtual void TearDown() { }

   OSLThread *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[3];

   static void Thr0(OSLThread *pThread, void *pContext);

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

   OSLThread          *m_pThrs[3];
   btTID               m_TIDs[3];
   volatile btUIntPtr  m_Scratch[10];

   static void Thr0(OSLThread * , void * );

   static void Thr1(OSLThread * , void * );
#if defined( __AAL_LINUX__ )
   static long tgkill(int tgid, int tid, int sig);

   static int RegisterOneShotSIGIOHandler(void (*h)(int , siginfo_t * , void * ));
   static void EmptySIGIOHandler(int , siginfo_t * , void * );

   static int RegisterPersistentSigHandler(int signum,
                                           void (*h)(int , siginfo_t * , void * ),
                                           struct sigaction *pold);
   static void EmptySIGUSR1Handler(int , siginfo_t * , void * );
#endif // OS

   static void Thr2(OSLThread * , void * );

   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );

   static void Thr5(OSLThread * , void * );
   static void Thr6(OSLThread * , void * );
};

#if defined( __AAL_LINUX__ )
int OSAL_Thread_f::RegisterOneShotSIGIOHandler(void (*h)(int , siginfo_t * , void * ))
{
   struct sigaction act;
   memset(&act, 0, sizeof(act));

   act.sa_flags     = SA_SIGINFO|SA_RESETHAND;
   act.sa_sigaction = h;

   return sigaction(SIGIO, &act, NULL);
}

void OSAL_Thread_f::EmptySIGIOHandler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGIO,    sig);
   EXPECT_EQ(SIGIO,    info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}

int OSAL_Thread_f::RegisterPersistentSigHandler(int signum,
                                                void (*h)(int , siginfo_t * , void * ),
                                                struct sigaction *pold)
{
   struct sigaction act;
   memset(&act, 0, sizeof(act));

   //act.sa_flags     = SA_SIGINFO|SA_RESETHAND;
   act.sa_flags     = SA_SIGINFO;
   act.sa_sigaction = h;

   return sigaction(signum, &act, pold);
}

void OSAL_Thread_f::EmptySIGUSR1Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR1,  sig);
   EXPECT_EQ(SIGUSR1,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}
#endif // OS

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
   // Register a one-shot signal handler for SIGIO so that the process is not
   // reaped as a result of having received an un-handled signal.
   ASSERT_EQ(0, OSAL_Thread_f::RegisterOneShotSIGIOHandler(OSAL_Thread_f::EmptySIGIOHandler));
#endif // OS

   pTC->m_Scratch[0] = 1; // set the first scratch space to 1 to signify that we're running.

#if   defined( __AAL_WINDOWS__ )

   FAIL() << "need to implement for windows";

#elif defined( __AAL_LINUX__ )

   struct timespec ts;

   ts.tv_sec  = 3 * 60;
   ts.tv_nsec = 0;

   // go to sleep for a long time - our parent thread is going to kill us. =O
   int res = nanosleep(&ts, NULL);
   EXPECT_EQ(-1, res);
   EXPECT_EQ(EINTR, errno);

   pTC->m_Scratch[1] = 1; // set the 2nd scratch space to 1 to signify that we're exiting.

#endif // OS
}

TEST_F(OSAL_Thread_f, aal0011)
{
   // OSLThread::Unblock() causes the recepient thread to be canceled (false==ThisThread).

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   // wait for Thr1 to begin.
   while ( 0 == m_Scratch[0] ) {
      cpu_yield();
   }

   // Thr1 has started and is going to be sleeping.
   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // reap the child thread.
   m_pThrs[0]->Unblock();

   // wait for Thr1 to return.
   while ( 0 == m_Scratch[1] ) {
      cpu_yield();
   }

   m_pThrs[0]->Join();

   delete m_pThrs[0];
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

TEST_F(OSAL_Thread_f, DISABLED_aal0012)
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

   //
   while ( ( 0 == m_Scratch[0] ) || ( 0 == m_Scratch[1] ) ) {
      cpu_yield();
   }

   m_Scratch[0] = m_Scratch[1] = 0;

   // Post one count to B. A will wake and consume the count.
   m_pThrs[1]->Signal();

   // Wait for A to wake.
   while ( 0 == m_Scratch[3] ) {
      cpu_yield();
   }

   ASSERT_EQ(0, m_Scratch[4]); // A should again immediately block again on B's count.

   ASSERT_EQ(0, m_Scratch[2]); // We haven't told B to exit.
   ASSERT_EQ(0, m_Scratch[1]); // B should not have exited.

   // Tell B to exit.
   m_Scratch[2] = 1;

   // Wait for B to exit.
   while ( 0 == m_Scratch[1] ) {
      cpu_yield();
   }

   // B exiting will post its count again, causing A to exit.
   while ( 0 == m_Scratch[4] ) {
      cpu_yield();
   }

   m_pThrs[1]->Join();
   m_pThrs[0]->Join();

   delete m_pThrs[0];
   delete m_pThrs[1];
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

   // Register a persistent signal handler for SIGUSR1 so that the process is not
   // reaped as a result of having received an un-handled signal.
   struct sigaction orig;
   ASSERT_EQ(0, OSAL_Thread_f::RegisterPersistentSigHandler(SIGUSR1, OSAL_Thread_f::EmptySIGUSR1Handler, &orig));

#endif // OS

   pTC->m_Scratch[0] = 1; // signal that we're ready.

   // wait for our sibling to be active before we try to wait for it.
   while ( 0 == pTC->m_Scratch[1] ) {
      cpu_yield();
   }

   pTC->m_Scratch[5] = 1;

   do
   {
      pTC->m_pThrs[1]->Wait();
      ++pTC->m_Scratch[4];
   }while( 0 == pTC->m_Scratch[3] );

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   // Restore the default SIGUSR1 handler.
   ASSERT_EQ(0, sigaction(SIGUSR1, &orig, NULL));
#endif // OS
}

void OSAL_Thread_f::Thr6(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[1] = 1; // signal that we're ready.

   // keep this thread alive until it's time to stop the test.
   while ( 0 == pTC->m_Scratch[2] ) {
      cpu_yield();
   }

   pTC->m_Scratch[3] = 1; // signal that we're done.
}

TEST_F(OSAL_Thread_f, aal0013)
{
   // OSLThread::Wait(void) is an infinite wait, even in the presence of signals.

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
   while ( 0 == m_Scratch[5] ) {
      cpu_yield();
   }

   // A is blocked in Wait() for B - send A signals to try to wake it.
   unsigned i;
   for ( i = 0 ; i < 100 ; ++i ) {
#if   defined( __AAL_WINDOWS__ )
      FAIL() << "implement for windows";
#elif defined( __AAL_LINUX__ )

      EXPECT_EQ(0, pthread_kill(m_TIDs[0], SIGUSR1));
      ASSERT_EQ(0, m_Scratch[4]);

#endif // OS
   }

   m_Scratch[2] = 1; // tell B to stop and wait for it to exit.
   while ( 0 == m_Scratch[3] ) {
      cpu_yield();
   }

   m_pThrs[1]->Signal(); // Wake A. Seeing that B has stopped, A will exit.

   m_pThrs[0]->Join();
   delete m_pThrs[0];
   m_pThrs[1]->Join();
   delete m_pThrs[1];

   ASSERT_EQ(1, m_Scratch[4]); // A should have woken only once.
}


