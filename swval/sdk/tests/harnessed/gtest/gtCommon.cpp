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

FILEMixin::~FILEMixin()
{
   iterator iter;
   for ( iter = m_FileMap.begin() ; iter != m_FileMap.end() ; ++iter ) {
      ::fclose((*iter).first);
      ::remove((*iter).second.m_fname.c_str());
   }
}

FILE * FILEMixin::fopen_tmp()
{
   char tmplt[13] = { 'g', 't', 'e', 's', 't', '.', 'X', 'X', 'X', 'X', 'X', 'X', 0 };

   int fd = ::mkstemp(tmplt);

   if ( -1 == fd ) {
      return NULL;
   }

   FILE *fp = ::fdopen(fd, "w+b");

   if ( NULL == fp ) {
      ::close(fd);
      ::remove(tmplt);
      return NULL;
   }

   FILEInfo info(tmplt, fd);

   std::pair<iterator, bool> res = m_FileMap.insert(std::make_pair(fp, info));

   if ( !res.second ) {
      ::fclose(fp);
      ::remove(tmplt);
      return NULL;
   }

   return fp;
}

btBool FILEMixin::fclose(FILE *fp)
{
   iterator iter = m_FileMap.find(fp);

   if ( m_FileMap.end() == iter ) {
      // fp not found
      return false;
   }

   ::fclose(fp);
   ::remove((*iter).second.m_fname.c_str());

   m_FileMap.erase(iter);

   return false;
}

void FILEMixin::rewind(FILE *fp) const
{
   ::rewind(fp);
}

int FILEMixin::feof(FILE *fp) const
{
   return ::feof(fp);
}

int FILEMixin::ferror(FILE *fp) const
{
   return ::ferror(fp);
}

long FILEMixin::InputBytesRemaining(FILE *fp) const
{
   long curpos = ::ftell(fp);

   ::fseek(fp, 0, SEEK_END);

   long endpos = ::ftell(fp);

   ::fseek(fp, curpos, SEEK_SET);

   return endpos - curpos;
}

////////////////////////////////////////////////////////////////////////////////

const char TestStatus::sm_Red[]   = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char TestStatus::sm_Green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char TestStatus::sm_Blue[]  = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
const char TestStatus::sm_Reset[] = { 0x1b, '[', '0', 'm', 0 };

bool       TestStatus::sm_HaltOnSegFault         = false;
bool       TestStatus::sm_HaltOnKeepaliveTimeout = false;

void TestStatus::HaltOnSegFault(bool b)
{
   TestStatus::sm_HaltOnSegFault = b;
}

void TestStatus::HaltOnKeepaliveTimeout(bool b)
{
   TestStatus::sm_HaltOnKeepaliveTimeout = b;
}

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
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nSegmentation Fault during " << testcase << "." << test << std::endl;
   std::cerr << "\nSegmentation Fault during " << testcase << "." << test << std::endl;

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;

   if ( TestStatus::sm_HaltOnSegFault ) {

      KeepAliveTimerEnv::GetInstance()->StopThread();

      int i = 0;
      while ( TestStatus::sm_HaltOnSegFault ) {
         if ( 0 == (i % (5 * 60)) ) {
            if ( ::isatty(2) ) {
               std::cerr << TestStatus::sm_Blue;
            }
            std::cerr << "Halted for debugger attach.\n";
            if ( ::isatty(2) ) {
               std::cerr << TestStatus::sm_Reset;
            }

            i = 0;
         }
         ::sleep(1);
         ++i;
      }

   }

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
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Red;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Red;
   }

   std::cout << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;
   std::cerr << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;

   if ( ::isatty(1) ) {
      std::cout << TestStatus::sm_Reset;
   }
   if ( ::isatty(2) ) {
      std::cerr << TestStatus::sm_Reset;
   }

   std::cout << std::flush;

   int i = 0;
   while ( TestStatus::sm_HaltOnKeepaliveTimeout ) {
      if ( 0 == (i % (5 * 60)) ) {
         if ( ::isatty(2) ) {
            std::cerr << TestStatus::sm_Blue;
         }
         std::cerr << "Halted for debugger attach.\n";
         if ( ::isatty(2) ) {
            std::cerr << TestStatus::sm_Reset;
         }

         i = 0;
      }
      ::sleep(1);
      ++i;
   }

   ::exit(99);
}

////////////////////////////////////////////////////////////////////////////////

// Retrieve the current test and test case name from gtest.
// Must be called within the context of a test case/fixture.
void TestCaseName(std::string &TestCase, std::string &Test)
{
   const ::testing::TestInfo * const pInfo =
      ::testing::UnitTest::GetInstance()->current_test_info();

   TestCase = std::string(pInfo->test_case_name());
   Test     = std::string(pInfo->name());
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

#if   defined( __AAL_WINDOWS__ )
# error TODO implement SignalHelper class for windows.
#elif defined( __AAL_LINUX__ )

SignalHelper SignalHelper::sm_Instance;
SignalHelper & SignalHelper::GetInstance()
{
   return SignalHelper::sm_Instance;
}

int SignalHelper::Install(SignalHelper::SigIndex i)
{
   if ( (i < IDX_FIRST) || (i >= IDX_COUNT) ) {
      // Invalid SigIndex.
      return -1;
   }

   if ( m_Tracker[i].installed ) {
      // attempt to double install.
      return -2;
   }

   struct sigaction act;
   memset(&act, 0, sizeof(act));

   act.sa_flags     = SA_SIGINFO;
   act.sa_sigaction = m_Tracker[i].h;
   if ( ( IDX_SIGSEGV == i ) || ( IDX_SIGINT == i ) ) {
      act.sa_flags |= SA_RESETHAND;
   }

   int res = ::sigaction(m_Tracker[i].signum, &act, &m_Tracker[i].orig);

   if ( 0 != res ) {
      return res;
   }

   m_Tracker[i].installed = true;
   memset(&m_Tracker[i].Counts, 0, sizeof(m_Tracker[0].Counts));

   return 0;
}

int SignalHelper::Uninstall(SignalHelper::SigIndex i)
{
   if ( (i < IDX_FIRST) || (i >= IDX_COUNT) ) {
      // Invalid SigIndex.
      return -1;
   }

   if ( !m_Tracker[i].installed ) {
      // Not hooked.
      return -2;
   }

   int res = ::sigaction(m_Tracker[i].signum, &m_Tracker[i].orig, NULL);

   m_Tracker[i].installed = false;

   return res;
}

btUIntPtr SignalHelper::GetCount(SigIndex i, btUnsignedInt thr)
{
   return m_Tracker[i].Counts[thr];
}

void SignalHelper::PutCount(SigIndex i, btUnsignedInt thr)
{
   ++m_Tracker[i].Counts[thr];
}

SignalHelper::SignalHelper()
{
   memset(&m_Tracker, 0, sizeof(m_Tracker));

   m_Tracker[IDX_SIGINT].signum  = SIGINT;
   m_Tracker[IDX_SIGINT].h       = SignalHelper::SIGINTHandler;

   m_Tracker[IDX_SIGSEGV].signum = SIGSEGV;
   m_Tracker[IDX_SIGSEGV].h      = SignalHelper::SIGSEGVHandler;

   m_Tracker[IDX_SIGIO].signum   = SIGIO;
   m_Tracker[IDX_SIGIO].h        = SignalHelper::SIGIOHandler;

   m_Tracker[IDX_SIGUSR1].signum = SIGUSR1;
   m_Tracker[IDX_SIGUSR1].h      = SignalHelper::SIGUSR1Handler;

   m_Tracker[IDX_SIGUSR2].signum = SIGUSR2;
   m_Tracker[IDX_SIGUSR2].h      = SignalHelper::SIGUSR2Handler;
}

SignalHelper::~SignalHelper()
{
   int i;
   for ( i = (int)IDX_FIRST ; i < (int)IDX_COUNT ; ++i ) {
      Uninstall((SigIndex)i);
   }
}

void SignalHelper::SIGIOHandler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGIO,    sig);
   EXPECT_EQ(SIGIO,    info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt i = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, i);
   if ( (btUnsignedInt)-1 != i ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGIO, i);
   }
}

void SignalHelper::SIGUSR1Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR1,  sig);
   EXPECT_EQ(SIGUSR1,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt i = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, i);
   if ( (btUnsignedInt)-1 != i ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR1, i);
   }
}

void SignalHelper::SIGUSR2Handler(int sig, siginfo_t *info, void * /* unused */)
{
   EXPECT_EQ(SIGUSR2,  sig);
   EXPECT_EQ(SIGUSR2,  info->si_signo);
   EXPECT_EQ(SI_TKILL, info->si_code);

   btUnsignedInt i = SignalHelper::GetInstance().ThreadLookup(GetThreadID());

   EXPECT_NE((btUnsignedInt)-1, i);
   if ( (btUnsignedInt)-1 != i ) {
      SignalHelper::GetInstance().PutCount(IDX_SIGUSR2, i);
   }
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

void KeepAliveTimerEnv::StopThread()
{
   KeepAliveTimerEnv::sm_KeepAliveFreqMillis = 100;
   m_KeepAliveRunning = false;

#if   defined( __AAL_LINUX__ )

   pthread_mutex_lock(&m_mutex);
   pthread_cond_signal(&m_condition);
   pthread_mutex_unlock(&m_mutex);

   pthread_join(m_thread, NULL);

#elif defined( __AAL_WINDOWS__ )

   SetEvent(m_hEvent);
   WaitForSingleObject(m_hJoinEvent, INFINITE);

#endif // OS
}

void KeepAliveTimerEnv::SetUp()
{
   m_KeepAliveRunning  = true;
   m_KeepAliveCounter  = 0;
   m_KeepAliveTimeouts = 0;
   KeepAliveTimerEnv::sm_KeepAliveFreqMillis = MINUTES_IN_TERMS_OF_MILLIS(1);

#if   defined( __AAL_LINUX__ )
   pthread_mutexattr_t mattr;

   pthread_mutexattr_init(&mattr);
   pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init(&m_mutex, &mattr);
   pthread_mutexattr_destroy(&mattr);
   pthread_cond_init(&m_condition, NULL);

   pthread_attr_t tattr;

   pthread_attr_init(&tattr);
   //pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

   pthread_create(&m_thread, &tattr, KeepAliveTimerEnv::KeepAliveThread, this);

   pthread_attr_destroy(&tattr);

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
   StopThread();

#if   defined( __AAL_LINUX__ )

   pthread_cond_destroy(&m_condition);
   pthread_mutex_destroy(&m_mutex);

#elif defined( __AAL_WINDOWS__ )

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
//void KeepAliveTimerEnv::KeepAliveCleanup(void *arg) {}
void * KeepAliveTimerEnv::KeepAliveThread(void *arg)
#elif defined ( __AAL_WINDOWS__ )
void   KeepAliveTimerEnv::KeepAliveThread(void *arg)
#endif // OS
{
   KeepAliveTimerEnv *e = reinterpret_cast<KeepAliveTimerEnv *>(arg);

#if   defined( __AAL_LINUX__ )
   struct timeval  tv;
   struct timespec ts;

   class _AutoMtx
   {
   public:
      _AutoMtx(pthread_mutex_t *mutex) :
         m_mutex(mutex)
      {
         pthread_mutex_lock(m_mutex);
      }
      ~_AutoMtx()
      {
         pthread_mutex_unlock(m_mutex);
      }
   protected:
      pthread_mutex_t *m_mutex;
   };

//   pthread_cleanup_push(KeepAliveTimerEnv::KeepAliveCleanup, e);
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

      {
         _AutoMtx lock(&e->m_mutex);
         if ( ETIMEDOUT != pthread_cond_timedwait(&e->m_condition,
                                                  &e->m_mutex,
                                                  &ts) ) {

            if ( !e->m_KeepAliveRunning ) {
               break;
            }
         }
      }

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
//   pthread_cleanup_pop(1);
   return NULL;
#elif defined( __AAL_WINDOWS__ )
   SetEvent(e->m_hJoinEvent);
   return;
#endif // OS
}

