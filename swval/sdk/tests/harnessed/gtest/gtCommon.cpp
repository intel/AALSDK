// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

GlobalTestConfig GlobalTestConfig::sm_Instance;
const GlobalTestConfig & GlobalTestConfig::GetInstance()
{
   return GlobalTestConfig::sm_Instance;
}

GlobalTestConfig::GlobalTestConfig() {}
GlobalTestConfig::~GlobalTestConfig() {}

////////////////////////////////////////////////////////////////////////////////

const char TestStatus::sm_Red[]   = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char TestStatus::sm_Green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char TestStatus::sm_Reset[] = { 0x1b, '[', '0', 'm', 0 };

void TestStatus::Report(TestStatus::Status st)
{
   switch ( st ) {
      case STATUS_PASS :
         TestStatus::OnPass();
      break;

      case STATUS_FAIL :
         TestStatus::OnFail();
      break;

      case STATUS_SEGFAULT :
         TestStatus::OnSegFault();
      break;

      case TestStatus::STATUS_TERMINATED :
         TestStatus::OnTerminated();
      break;

      case STATUS_KEEPALIVE_TIMEOUT :
         TestStatus::OnKeepaliveTimeout();
      break;
   }
}

void TestStatus::OnPass()
{
   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Green;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Green;
   }

   std::cout << "\nPASS\n";
   std::cerr << "\nPASS\n";

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;
}

void TestStatus::OnFail()
{
   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nFAIL\n";
   std::cerr << "\nFAIL\n";

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;
}

void TestStatus::OnSegFault()
{
   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nSegmentation Fault\n";
   std::cerr << "\nSegmentation Fault\n";

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;

   ::exit(97);
}

void TestStatus::OnTerminated()
{
   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nProcess Terminated\n";
   std::cerr << "\nProcess Terminated\n";

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;

   ::exit(98);
}

void TestStatus::OnKeepaliveTimeout()
{
   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nKeep-alive Timer Expired\n";
   std::cerr << "\nKeep-alive Timer Expired\n";

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;

   ::exit(99);
}

////////////////////////////////////////////////////////////////////////////////

// Retrieve the current test and test case name from gtest.
// Must be called within the context of a test case/fixture.
void TestCaseName(std::string &Test, std::string &TestCase)
{
   const ::testing::TestInfo * const pInfo =
      ::testing::UnitTest::GetInstance()->current_test_info();

   Test     = std::string(pInfo->name());
   TestCase = std::string(pInfo->test_case_name());
}

#if defined( __AAL_LINUX__ )

// Make sure that the given path appears in LD_LIBRARY_PATH, preventing duplication.
// Return non-zero on error.
int RequireLD_LIBRARY_PATH(const char *path)
{
   int   res  = 1;
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL == pvar ) {
      // not found, so set it.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   char *pcopyvar  = strdup(pvar);
   char *psavecopy = pcopyvar;

   if ( NULL == pcopyvar ) {
      return res;
   }

   char *pcolon;
   while ( NULL != (pcolon = strchr(pcopyvar, ':')) ) {

      *pcolon = 0;

      if ( 0 == strcmp(pcopyvar, path) ) {
         // path already found in LD_LIBRARY_PATH
         res = 0;
         goto _DONE;
      }

      pcopyvar = pcolon + 1;

   }

   if ( 0 == strcmp(pcopyvar, path) ) {
      // path already found in LD_LIBRARY_PATH
      res = 0;
      goto _DONE;
   }

   // LD_LIBRARY_PATH exists, but does not contain path.

   free(psavecopy);

   if ( 0 == strcmp(pvar, "") ) {
      // LD_LIBRARY_PATH is defined, but empty.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   psavecopy = (char *) malloc(strlen(pvar) + strlen(path) + 2);
   if ( NULL == psavecopy ) {
      return res;
   }

   sprintf(psavecopy, "%s:%s", pvar, path);

   res = setenv("LD_LIBRARY_PATH", psavecopy, 1);

_DONE:
   free(psavecopy);

   return res;
}

// Print streamer for LD_LIBRARY_PATH.
// Ex.
//   cout << LD_LIBRARY_PATH << endl;
std::ostream & LD_LIBRARY_PATH(std::ostream &os)
{
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL != pvar ) {
      os << pvar;
   }

   return os;
}

#endif // __AAL_LINUX__


#if   defined( __AAL_WINDOWS__ )
# error TODO implement SignalHelper class for windows.
#elif defined( __AAL_LINUX__ )

SignalHelper SignalHelper::sm_GlobalInstance;

SignalHelper & SignalHelper::GlobalInstance()
{
   return SignalHelper::sm_GlobalInstance;
}

SignalHelper::SignalHelper() {}

SignalHelper::~SignalHelper()
{
   // re-map each signal to its original handler.
   const_sigiter iter;
   for ( iter = m_sigmap.begin() ; m_sigmap.end() != iter ; ++iter ) {
      ::sigaction(iter->first, &iter->second, NULL);
   }

}

int SignalHelper::Install(int signum, handler h, bool oneshot)
{
   if ( NULL == h ) {
      return -1;
   }

   struct sigaction act;
   memset(&act, 0, sizeof(act));

   act.sa_flags     = SA_SIGINFO;
   if ( oneshot ) {
      act.sa_flags |= SA_RESETHAND;
   }
   act.sa_sigaction = h;

   struct sigaction orig;
   memset(&orig, 0, sizeof(orig));

   int res = ::sigaction(signum, &act, &orig);

   if ( 0 != res ) {
      return res;
   }

   std::pair<sigiter, bool> ins = m_sigmap.insert(std::make_pair(signum, orig));

   return ins.second ? res : -2;
}

void SignalHelper::EmptySIGIOHandler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGIO,    sig);
   EXPECT_EQ(SIGIO,    info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}

void SignalHelper::EmptySIGUSR1Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR1,  sig);
   EXPECT_EQ(SIGUSR1,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}

void SignalHelper::EmptySIGUSR2Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR2,  sig);
   EXPECT_EQ(SIGUSR2,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);
}

void SignalHelper::SIGSEGVHandler(int sig, siginfo_t *info, void * /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_SEGFAULT);
}

void SignalHelper::SIGINTHandler(int sig, siginfo_t *info, void * /* unused */)
{
   TestStatus::Report(TestStatus::STATUS_TERMINATED);
}

#endif // OS

////////////////////////////////////////////////////////////////////////////////

KeepAliveTimerEnv *KeepAliveTimerEnv::sm_pInstance = NULL;
KeepAliveTimerEnv * KeepAliveTimerEnv::GetInstance()
{
   if ( NULL == KeepAliveTimerEnv::sm_pInstance ) {
      KeepAliveTimerEnv::sm_pInstance = new KeepAliveTimerEnv();
   }
   return KeepAliveTimerEnv::sm_pInstance;
}

KeepAliveTimerEnv::KeepAliveTimerEnv() :
   m_KeepAliveRunning(true),
   m_KeepAliveCounter(0),
   m_KeepAliveTimeouts(0)
#if defined( __AAL_WINDOWS__ )
   , m_hEvent(NULL),
   m_hJoinEvent(NULL)
#endif // __AAL_WINDOWS__
{}

void KeepAliveTimerEnv::SetUp()
{
   m_KeepAliveRunning  = true;
   m_KeepAliveCounter  = 0;
   m_KeepAliveTimeouts = 0;
   KeepAliveTimerEnv::sm_KeepAliveFreqMillis = MINUTES_IN_TERMS_OF_MILLIS(1);

#if   defined( __AAL_LINUX__ )
   pthread_mutexattr_t attr;

   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init(&m_mutex, &attr);
   pthread_mutexattr_destroy(&attr);
   pthread_cond_init(&m_condition, NULL);

   pthread_create(&m_thread, NULL, KeepAliveTimerEnv::KeepAliveThread, this);

#elif defined( __AAL_WINDOWS__ )

   m_hEvent = CreateEvent(NULL,   // no inheritance
                          FALSE,  // auto-reset event
                          FALSE,  // not signaled
                          NULL);  // no name

   m_hJoinEvent = CreateEvent(NULL,   // no inheritance
                              TRUE,   // manual-reset event
                              FALSE,  // not signaled
                              NULL);  // no name

   _beginthread(KeepAliveTimerEnv::KeepAliveThread, 0, this);

#endif // OS
}

void KeepAliveTimerEnv::TearDown()
{
   KeepAliveTimerEnv::sm_KeepAliveFreqMillis = 100;
   m_KeepAliveRunning = false;

#if   defined( __AAL_LINUX__ )

   pthread_cond_signal(&m_condition);
   pthread_join(m_thread, NULL);

   pthread_cond_destroy(&m_condition);
   pthread_mutex_destroy(&m_mutex);

#elif defined( __AAL_WINDOWS__ )

   SetEvent(m_hEvent);
   WaitForSingleObject(m_hJoinEvent, INFINITE);

   CloseHandle(m_hEvent);
   CloseHandle(m_hJoinEvent);

   m_hEvent = m_hJoinEvent = NULL;

#endif // OS
}

void KeepAliveTimerEnv::KeepAliveExpired()
{
   TestStatus::Report(TestStatus::STATUS_KEEPALIVE_TIMEOUT);
}

      btTime        KeepAliveTimerEnv::sm_KeepAliveFreqMillis  = MINUTES_IN_TERMS_OF_MILLIS(1);
const btUnsignedInt KeepAliveTimerEnv::sm_MaxKeepAliveTimeouts = 3;

#if   defined( __AAL_LINUX__ )
void * KeepAliveTimerEnv::KeepAliveThread(void *arg)
#elif defined ( __AAL_WINDOWS__ )
void   KeepAliveTimerEnv::KeepAliveThread(void *arg)
#endif // OS
{
   KeepAliveTimerEnv *e = reinterpret_cast<KeepAliveTimerEnv *>(arg);

#if   defined( __AAL_LINUX__ )
   struct timeval  tv;
   struct timespec ts;
#elif defined( __AAL_WINDOWS__ )

#endif // OS

   btUnsigned64bitInt LastKeepAliveCounter = e->m_KeepAliveCounter;

   while ( e->m_KeepAliveRunning ) {
#if   defined( __AAL_LINUX__ )

      gettimeofday(&tv, NULL);

      ts.tv_sec  = tv.tv_sec;
      ts.tv_nsec = (tv.tv_usec * 1000) + (KeepAliveTimerEnv::sm_KeepAliveFreqMillis * 1000000);

      ts.tv_sec  += ts.tv_nsec / 1000000000;
      ts.tv_nsec %= 1000000000;

      pthread_mutex_lock(&e->m_mutex);

      if ( ETIMEDOUT != pthread_cond_timedwait(&e->m_condition,
                                               &e->m_mutex,
                                               &ts) ) {
         if ( !e->m_KeepAliveRunning ) {
            pthread_mutex_unlock(&e->m_mutex);
            break;
         }
      }

      pthread_mutex_unlock(&e->m_mutex);

#elif defined( __AAL_WINDOWS__ )

      if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent, (DWORD)KeepAliveTimerEnv::sm_KeepAliveFreqMillis) ) {
         if ( !e->m_KeepAliveRunning ) {
             break;
         }
      }

#endif // OS

      if ( e->m_KeepAliveCounter == LastKeepAliveCounter ) {
         // keep-alive not updated before timer expired.
         ++e->m_KeepAliveTimeouts;
         if ( e->m_KeepAliveTimeouts >= KeepAliveTimerEnv::sm_MaxKeepAliveTimeouts ) {
            e->KeepAliveExpired();
            break;
         }
      } else {
         e->m_KeepAliveTimeouts = 0;
      }

      LastKeepAliveCounter = e->m_KeepAliveCounter;
   }

#if   defined( __AAL_LINUX__ )
   return NULL;
#elif defined( __AAL_WINDOWS__ )
   SetEvent(e->m_hJoinEvent);
   return;
#endif // OS
}

