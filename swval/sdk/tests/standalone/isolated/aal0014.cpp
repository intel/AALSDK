// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aal0014.h"

static void aal0014_testcase()
{
   aal0014Fixture f;
   Execute(&f);
}

void aal0014()
{
   int       i = 0;
   const int N = 1000;

   while ( 1 ) {
      aal0014_testcase();

      ++i;
      if ( i == N ) {
         std::cerr << "14 ";
         i = 0;
      }
   }
}

void aal0014Fixture::Thr0(OSLThread *pThread, void *pContext)
{
   aal0014Fixture *pTC = static_cast<aal0014Fixture *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_TIDs[0] = GetThreadID();

#if   defined( __AAL_WINDOWS__ )
   FAIL() << "need to implement for windows";
#elif defined( __AAL_LINUX__ )
   // Register a persistent signal handler for SIGUSR1 so that the process is not
   // reaped as a result of having received an un-handled signal.
//   SignalHelper sig;
//   ASSERT_EQ(0, sig.Install(SIGUSR1, SignalHelper::EmptySIGUSR1Handler));

   struct sigaction act;
   memset(&act, 0, sizeof(act));

   act.sa_flags     = SA_SIGINFO;
   act.sa_sigaction = aal0014Fixture::EmptySIGUSR1Handler;

   struct sigaction orig;
   memset(&orig, 0, sizeof(orig));

   ASSERT_EQ(0, ::sigaction(SIGUSR1, &act, &orig));

#endif // OS

   pTC->m_Scratch[0] = 1; // signal that we're ready.

   // wait for B to be active before we try to wait for it.
   YIELD_WHILE(0 == pTC->m_Scratch[1]);

   pTC->m_Scratch[5] = 1;
   do
   {
      pTC->m_pThrs[1]->Wait(5000);
      ++pTC->m_Scratch[4];
   }while( 0 == pTC->m_Scratch[3] );

#if defined( __AAL_LINUX__ )
   ::sigaction(SIGUSR1, &orig, NULL);
#endif // OS

   pTC->m_Scratch[6] = 1;
}

void aal0014Fixture::Thr1(OSLThread *pThread, void *pContext)
{
   aal0014Fixture *pTC = static_cast<aal0014Fixture *>(pContext);
   ASSERT_NONNULL(pTC);
   ASSERT_NONNULL(pThread);

   pTC->m_Scratch[1] = 1; // signal that we're ready.

   // keep this thread alive until it's time to stop the test.
   YIELD_WHILE(0 == pTC->m_Scratch[2]);

   pTC->m_Scratch[3] = 1; // signal that we're done.
}

void aal0014Fixture::EmptySIGUSR1Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR1,  sig);
   EXPECT_EQ(SIGUSR1,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}

void aal0014Fixture::Run()
{
   // OSLThread::Wait(btTime ) waits for at least the given number of milliseconds, even in the presence of signals.

   m_pThrs[0] = new OSLThread(aal0014Fixture::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[1] = new OSLThread(aal0014Fixture::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   ASSERT_TRUE(m_pThrs[1]->IsOK());

   // Wait for the threads to become ready.
   YIELD_WHILE(0 == m_Scratch[5]);

   // A is blocked in Wait() for B - send A signals to try to wake it.
   unsigned i;
   for ( i = 0 ; i < 10 ; ++i ) {
#if   defined( __AAL_WINDOWS__ )
      FAIL() << "implement for windows";
#elif defined( __AAL_LINUX__ )

      EXPECT_EQ(0, pthread_kill(m_TIDs[0], SIGUSR1));
      cpu_yield();
      ASSERT_EQ(0, m_Scratch[4]);

#endif // OS
   }

   m_Scratch[2] = 1; // tell B to stop and wait for it to exit.
   YIELD_WHILE(0 == m_Scratch[3]);

   // Resume A from sleep on B. A will wake and exit.
   m_pThrs[1]->Signal();

   YIELD_WHILE(0 == m_Scratch[6]);

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();

   delete m_pThrs[0];
   delete m_pThrs[1];

   ASSERT_EQ(1, m_Scratch[4]);
}

