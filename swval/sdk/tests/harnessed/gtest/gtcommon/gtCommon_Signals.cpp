// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

ThreadRegistry::ThreadRegistry()
{
   RegistryReset();
}

btUnsignedInt ThreadRegistry::ThreadRegister(btTID tid)
{
   AutoLock(this);
   btUnsignedInt i = m_NextThread;
   ++m_NextThread;
   m_RegisteredThreads[i] = tid;
   return i;
}

btUnsignedInt ThreadRegistry::ThreadLookup(btTID tid)
{
   btUnsignedInt i;
   for ( i = 0 ; i < sizeof(m_RegisteredThreads) / sizeof(m_RegisteredThreads[0]) ; ++i ) {
      if ( tid == m_RegisteredThreads[i] ) {
         return i;
      }
   }
   return (btUnsignedInt)-1;
}

void ThreadRegistry::RegistryReset()
{
   AutoLock(this);
   m_NextThread = 0;
   memset(&m_RegisteredThreads, 0, sizeof(m_RegisteredThreads));
}


SignalHelper SignalHelper::sm_Instance;
SignalHelper & SignalHelper::GetInstance()
{
   return SignalHelper::sm_Instance;
}

int SignalHelper::Install(SignalHelper::SigIndex i)
{
   if ((i < IDX_FIRST) || (i >= IDX_COUNT)) {
      // Invalid SigIndex.
      return -1;
   }

   if (m_Tracker[i].installed) {
      // attempt to double install.
      return -2;
   }

#if   defined( __AAL_WINDOWS__ )

   handler orig;

   orig = ::signal(m_Tracker[i].signum, m_Tracker[i].h);

   if ( SIG_ERR == orig ) {
      return errno;
   }

   m_Tracker[i].orig      = orig;
   m_Tracker[i].installed = true;

#elif defined( __AAL_LINUX__ )

   struct sigaction act;
   memset(&act, 0, sizeof(act));

   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = m_Tracker[i].h;
   if ((IDX_SIGSEGV == i) || (IDX_SIGINT == i)) {
      act.sa_flags |= SA_RESETHAND;
   }

   int res = ::sigaction(m_Tracker[i].signum, &act, &m_Tracker[i].orig);

   if (0 != res) {
      return res;
   }

   m_Tracker[i].installed = true;

#endif // OS

   memset(&m_Tracker[i].Counts, 0, sizeof(m_Tracker[0].Counts));

   return 0;
}

int SignalHelper::Uninstall(SignalHelper::SigIndex i)
{
   if ((i < IDX_FIRST) || (i >= IDX_COUNT)) {
      // Invalid SigIndex.
      return -1;
   }

   if (!m_Tracker[i].installed) {
      // Not hooked.
      return -2;
   }

   int res = 0;

#if   defined( __AAL_WINDOWS__ )

   handler orig;

   orig = ::signal(m_Tracker[i].signum, m_Tracker[i].orig);

   if ( SIG_ERR == orig ) {
      res = errno;
   }

#elif defined( __AAL_LINUX__ )

   res = ::sigaction(m_Tracker[i].signum, &m_Tracker[i].orig, NULL);

#endif // OS

   m_Tracker[i].installed = false;

   return res;
}

btUIntPtr SignalHelper::GetCount(SignalHelper::SigIndex i, btUnsignedInt thr)
{
   return m_Tracker[i].Counts[thr];
}

void SignalHelper::PutCount(SignalHelper::SigIndex i, btUnsignedInt thr)
{
   ++m_Tracker[i].Counts[thr];
}

int SignalHelper::Raise(SignalHelper::SigIndex i, btTID tid)
{
   int signum;

   switch ( i ) {
      case IDX_SIGINT :
         signum = SIGINT;
      break;
      case IDX_SIGSEGV :
         signum = SIGSEGV;
      break;
      case IDX_SIGUSR1 :
#if   defined( __AAL_WINDOWS__ )
         signum = SIGILL;
#elif defined( __AAL_LINUX__ )
         signum = SIGUSR1;
#endif // OS
      break;
      case IDX_SIGUSR2 :
#if   defined( __AAL_WINDOWS__ )
         signum = SIGTERM;
#elif defined( __AAL_LINUX__ )
         signum = SIGUSR2;
#endif // OS
      break;
#if defined( __AAL_LINUX__ )
      case IDX_SIGIO   :
         signum = SIGIO;
      break;
#endif // __AAL_LINUX__
      default : return 1; // unsupported signal
   }

#if   defined( __AAL_WINDOWS__ )
   return raise(signum);
#elif defined( __AAL_LINUX__ )
   return pthread_kill(tid, signum);
#endif // OS
}

SignalHelper::SignalHelper()
{
   memset(&m_Tracker, 0, sizeof(m_Tracker));

   m_Tracker[IDX_SIGINT].signum  = SIGINT;
   m_Tracker[IDX_SIGINT].h       = SignalHelper::SIGINTHandler;

   m_Tracker[IDX_SIGSEGV].signum = SIGSEGV;
   m_Tracker[IDX_SIGSEGV].h      = SignalHelper::SIGSEGVHandler;

#if   defined( __AAL_WINDOWS__ )

   // According to https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
   //
   //    "The SIGILL and SIGTERM signals are not generated under Windows.
   //    They are included for ANSI compatibility. Therefore, you can set
   //    signal handlers for these signals by using signal, and you can also
   //    explicitly generate these signals by calling raise."

   m_Tracker[IDX_SIGUSR1].signum = SIGILL;
   m_Tracker[IDX_SIGUSR1].h      = SignalHelper::SIGUSR1Handler;

   m_Tracker[IDX_SIGUSR2].signum = SIGTERM;
   m_Tracker[IDX_SIGUSR2].h      = SignalHelper::SIGUSR2Handler;

#elif defined( __AAL_LINUX__ )

   m_Tracker[IDX_SIGIO].signum   = SIGIO;
   m_Tracker[IDX_SIGIO].h        = SignalHelper::SIGIOHandler;

   m_Tracker[IDX_SIGUSR1].signum = SIGUSR1;
   m_Tracker[IDX_SIGUSR1].h      = SignalHelper::SIGUSR1Handler;

   m_Tracker[IDX_SIGUSR2].signum = SIGUSR2;
   m_Tracker[IDX_SIGUSR2].h      = SignalHelper::SIGUSR2Handler;

#endif // OS
}

SignalHelper::~SignalHelper()
{
   int i;
   for (i = (int)IDX_FIRST; i < (int)IDX_COUNT; ++i) {
      Uninstall((SigIndex)i);
   }
}


#if   defined( __AAL_WINDOWS__ )

void SignalHelper::SIGINTHandler(int /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_TERMINATED);
}

void SignalHelper::SIGSEGVHandler(int /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_SEGFAULT);
}

void SignalHelper::SIGUSR1Handler(int sig)
{
   EXPECT_EQ(SIGILL, sig);

   btUnsignedInt i = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, i);
   if ((btUnsignedInt)-1 != i) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR1, i);
   }
}

void SignalHelper::SIGUSR2Handler(int sig)
{
   EXPECT_EQ(SIGTERM, sig);

   btUnsignedInt i = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, i);
   if ((btUnsignedInt)-1 != i) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR2, i);
   }
}

#elif defined( __AAL_LINUX__ )

void SignalHelper::SIGINTHandler(int sig, siginfo_t *info, void * /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_TERMINATED);
}

void SignalHelper::SIGSEGVHandler(int sig, siginfo_t *info, void * /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_SEGFAULT);
}

void SignalHelper::SIGIOHandler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGIO,    sig);
   EXPECT_EQ(SIGIO,    info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt thr = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, thr);
   if ( (btUnsignedInt)-1 != thr ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGIO, thr);
   }
}

void SignalHelper::SIGUSR1Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR1,  sig);
   EXPECT_EQ(SIGUSR1,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt thr = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, thr);
   if ( (btUnsignedInt)-1 != thr ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR1, thr);
   }
}

void SignalHelper::SIGUSR2Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR2,  sig);
   EXPECT_EQ(SIGUSR2,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt thr = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, thr);
   if ( (btUnsignedInt)-1 != thr ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR2, thr);
   }
}

#endif // OS

