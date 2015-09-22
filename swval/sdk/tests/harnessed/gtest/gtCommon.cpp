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

   return true;
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

////////////////////////////////////////////////////////////////////////////////

MethodCallLogEntry::MethodCallLogEntry(btcString method, Timer timestamp) :
   m_TimeStamp(timestamp)
{
   m_NVS.Add((btStringKey)"__func__", method);
}

btcString MethodCallLogEntry::MethodName() const
{
   btcString n = NULL;
   m_NVS.Get((btStringKey)"__func__", &n);
   return n;
}

void MethodCallLogEntry::AddParam(btcString name, btBool             value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btByte             value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, bt32bitInt         value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btUnsigned32bitInt value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, bt64bitInt         value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btUnsigned64bitInt value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btFloat            value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btcString          value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btObjectType       value) { m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, INamedValueSet    *value) { m_NVS.Add(name, value); }

void MethodCallLogEntry::AddParam(btcString name, const TransactionID &value)
{
   std::string var(name);
   std::string key;

   stTransactionID_t tidStruct = (stTransactionID_t)value;

   key = var + std::string(".m_ID");
   m_NVS.Add(key.c_str(), reinterpret_cast<btObjectType>(tidStruct.m_ID));

   key = var + std::string(".m_Handler");
   m_NVS.Add(key.c_str(), reinterpret_cast<btObjectType>(tidStruct.m_Handler));

   key = var + std::string(".m_IBase");
   m_NVS.Add(key.c_str(), reinterpret_cast<btObjectType>(tidStruct.m_IBase));

   key = var + std::string(".m_Filter");
   m_NVS.Add(key.c_str(), tidStruct.m_Filter);

   key = var + std::string(".m_intID");
   m_NVS.Add(key.c_str(), tidStruct.m_intID);
}

unsigned MethodCallLogEntry::Params() const
{
   btUnsignedInt u = 0;
   m_NVS.GetNumNames(&u);
   return (unsigned)(u - 1);
}

void MethodCallLogEntry::GetParam(btcString name, btBool                *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btByte                *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, bt32bitInt            *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btUnsigned32bitInt    *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, bt64bitInt            *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btUnsigned64bitInt    *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btFloat               *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btcString             *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, btObjectType          *pValue) const { m_NVS.Get(name, pValue); }
void MethodCallLogEntry::GetParam(btcString name, INamedValueSet const **pValue) const { m_NVS.Get(name, pValue); }

void MethodCallLogEntry::GetParam(btcString name, TransactionID &tid) const
{
   std::string var(name);
   std::string key;

   stTransactionID_t tidStruct;

   key = var + std::string(".m_ID");
   m_NVS.Get(key.c_str(), reinterpret_cast<btObjectType *>(&tidStruct.m_ID));

   key = var + std::string(".m_Handler");
   m_NVS.Get(key.c_str(), reinterpret_cast<btObjectType *>(&tidStruct.m_Handler));

   key = var + std::string(".m_IBase");
   m_NVS.Get(key.c_str(), reinterpret_cast<btObjectType *>(&tidStruct.m_IBase));

   key = var + std::string(".m_Filter");
   m_NVS.Get(key.c_str(), &tidStruct.m_Filter);

   key = var + std::string(".m_intID");
   m_NVS.Get(key.c_str(), &tidStruct.m_intID);

   tid = tidStruct;
}

MethodCallLogEntry * MethodCallLog::AddToLog(btcString method)
{
   AutoLock(this);

   m_LogList.push_back(MethodCallLogEntry(method));

   iterator iter = m_LogList.end();
   --iter;
   return &(*iter);
}

unsigned MethodCallLog::LogEntries() const
{
   AutoLock(this);
   return m_LogList.size();
}

const MethodCallLogEntry & MethodCallLog::Entry(unsigned i) const
{
   AutoLock(this);

   const_iterator iter;
   for ( iter = m_LogList.begin() ; i-- ; ++iter ) { ; /* traverse */ }
   return *iter;
}

void MethodCallLog::ClearLog()
{
   m_LogList.clear();
}

////////////////////////////////////////////////////////////////////////////////

EmptyIServiceClient::EmptyIServiceClient(btApplicationContext Ctx) :
   CAASBase(Ctx)
{
   if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ) {
      m_bIsOK = false;
   }
}

////////////////////////////////////////////////////////////////////////////////

EmptyIRuntimeClient::EmptyIRuntimeClient(btApplicationContext Ctx) :
   CAASBase(Ctx)
{
   if ( EObjOK != SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this)) ) {
      m_bIsOK = false;
   }
}

////////////////////////////////////////////////////////////////////////////////

EmptyISvcsFact::EmptyISvcsFact() :
   m_CreateServiceObject_returns(NULL),
   m_InitializeService_returns(true)
{}

IBase * EmptyISvcsFact::CreateServiceObject(AALServiceModule * ,
                                            IRuntime         * ) { return m_CreateServiceObject_returns; }

btBool EmptyISvcsFact::InitializeService(IBase               * ,
                                         TransactionID const & ,
                                         NamedValueSet const & ) { return m_InitializeService_returns; }

IMPLEMENT_RETVAL_ACCESSORS(EmptyISvcsFact, CreateServiceObject, IBase * , m_CreateServiceObject_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyISvcsFact, InitializeService,   btBool ,  m_InitializeService_returns)


EmptyIRuntime::EmptyIRuntime(btApplicationContext Ctx) :
   CAASBase(Ctx),
   m_start_returns(true),
   m_schedDispatchable_returns(true),
   m_getRuntimeProxy_returns(NULL),
   m_releaseRuntimeProxy_0_returns(true),
   m_releaseRuntimeProxy_1_returns(true),
   m_getRuntimeClient_returns(NULL),
   m_IsOK_returns(true)
{
   if ( EObjOK != SetInterface(iidRuntime, dynamic_cast<IRuntime *>(this)) ) {
      m_bIsOK = false;
   }
}

btBool                      EmptyIRuntime::start(const NamedValueSet & ) { return m_start_returns;                 }
btBool          EmptyIRuntime::schedDispatchable(IDispatchable * )       { return m_schedDispatchable_returns;     }
IRuntime *        EmptyIRuntime::getRuntimeProxy(IRuntimeClient * )      { return m_getRuntimeProxy_returns;       }
btBool        EmptyIRuntime::releaseRuntimeProxy(IRuntime * )            { return m_releaseRuntimeProxy_0_returns; }
btBool        EmptyIRuntime::releaseRuntimeProxy()                       { return m_releaseRuntimeProxy_1_returns; }
IRuntimeClient * EmptyIRuntime::getRuntimeClient()                       { return m_getRuntimeClient_returns;      }
btBool                       EmptyIRuntime::IsOK()                       { return m_IsOK_returns;                  }

IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, start,                btBool,            m_start_returns                )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, schedDispatchable,    btBool,            m_schedDispatchable_returns    )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, getRuntimeProxy,      IRuntime * ,       m_getRuntimeProxy_returns      )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, releaseRuntimeProxy0, btBool,            m_releaseRuntimeProxy_0_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, releaseRuntimeProxy1, btBool,            m_releaseRuntimeProxy_1_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, getRuntimeClient,     IRuntimeClient * , m_getRuntimeClient_returns     )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, IsOK,                 btBool,            m_IsOK_returns                 )


EmptyIAALTransport::EmptyIAALTransport() :
   m_connectremote_returns(true),
   m_waitforconnect_returns(true),
   m_disconnect_returns(true),
   m_getmsg_returns(""),
   m_putmsg_returns(0)
{}

btBool  EmptyIAALTransport::connectremote(NamedValueSet const & ) { return m_connectremote_returns;    }
btBool EmptyIAALTransport::waitforconnect(NamedValueSet const & ) { return m_waitforconnect_returns;   }
btBool     EmptyIAALTransport::disconnect()                       { return m_disconnect_returns;       }
btcString      EmptyIAALTransport::getmsg(btWSSize *pSz)          { *pSz = 0; return m_getmsg_returns; }
int            EmptyIAALTransport::putmsg(btcString , btWSSize )  { return m_putmsg_returns;           }

IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALTransport, connectremote,  btBool,    m_connectremote_returns )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALTransport, waitforconnect, btBool,    m_waitforconnect_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALTransport, disconnect,     btBool,    m_disconnect_returns    )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALTransport, getmsg,         btcString, m_getmsg_returns        )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALTransport, putmsg,         int,       m_putmsg_returns        )


EmptyIAALMarshaller::EmptyIAALMarshaller() :
   m_pmsgp_returns(NULL)
{}

ENamedValues   EmptyIAALMarshaller::Empty()                                              { return m_NVS.Empty();               }
btBool           EmptyIAALMarshaller::Has(btStringKey Name)                        const { return m_NVS.Has(Name);             }
ENamedValues  EmptyIAALMarshaller::Delete(btStringKey Name)                              { return m_NVS.Delete(Name);          }
ENamedValues EmptyIAALMarshaller::GetSize(btStringKey Name,    btWSSize    *pSz)   const { return m_NVS.GetSize(Name, pSz);    }
ENamedValues    EmptyIAALMarshaller::Type(btStringKey Name,    eBasicTypes *pTy)   const { return m_NVS.Type(Name, pTy);       }
ENamedValues EmptyIAALMarshaller::GetName(btUnsignedInt index, btStringKey *pName) const { return m_NVS.GetName(index, pName); }

ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btBool                Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btByte                Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, bt32bitInt            Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btUnsigned32bitInt    Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, bt64bitInt            Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btUnsigned64bitInt    Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btFloat               Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btcString             Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, btObjectType          Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name, const INamedValueSet *Value) { return m_NVS.Add(Name, Value); }

ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btByteArray             Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      bt32bitIntArray         Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      bt64bitIntArray         Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btFloatArray            Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btStringArray           Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btNumberKey Name,
                                      btObjectArray           Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }

ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btBool                Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btByte                Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, bt32bitInt            Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btUnsigned32bitInt    Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, bt64bitInt            Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btUnsigned64bitInt    Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btFloat               Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btcString             Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, btObjectType          Value) { return m_NVS.Add(Name, Value); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name, const INamedValueSet *Value) { return m_NVS.Add(Name, Value); }

ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btByteArray             Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      bt32bitIntArray         Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      bt64bitIntArray         Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btFloatArray            Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btStringArray           Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }
ENamedValues EmptyIAALMarshaller::Add(btStringKey Name,
                                      btObjectArray           Value, btUnsigned32bitInt NumElements)
{ return m_NVS.Add(Name, Value, NumElements); }

char const * EmptyIAALMarshaller::pmsgp(btWSSize *len) { return m_pmsgp_returns; }

IMPLEMENT_RETVAL_ACCESSORS(EmptyIAALMarshaller, pmsgp, char const *, m_pmsgp_returns)

EmptyIAALUnMarshaller::EmptyIAALUnMarshaller() {}

ENamedValues   EmptyIAALUnMarshaller::Empty()                                              { return m_NVS.Empty();               }
btBool           EmptyIAALUnMarshaller::Has(btStringKey Name)                        const { return m_NVS.Has(Name);             }
ENamedValues  EmptyIAALUnMarshaller::Delete(btStringKey Name)                              { return m_NVS.Delete(Name);          }
ENamedValues EmptyIAALUnMarshaller::GetSize(btStringKey Name,    btWSSize    *pSz)   const { return m_NVS.GetSize(Name, pSz);    }
ENamedValues    EmptyIAALUnMarshaller::Type(btStringKey Name,    eBasicTypes *pTy)   const { return m_NVS.Type(Name, pTy);       }
ENamedValues EmptyIAALUnMarshaller::GetName(btUnsignedInt index, btStringKey *pName) const { return m_NVS.GetName(index, pName); }

ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btBool                  *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btByte                  *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, bt32bitInt              *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btUnsigned32bitInt      *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, bt64bitInt              *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btUnsigned64bitInt      *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btFloat                 *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btcString               *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btObjectType            *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, INamedValueSet const   **pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btByteArray             *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, bt32bitIntArray         *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, bt64bitIntArray         *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btFloatArray            *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btStringArray           *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btNumberKey Name, btObjectArray           *pValue) const { return m_NVS.Get(Name, pValue); }

ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btBool                  *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btByte                  *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, bt32bitInt              *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btUnsigned32bitInt      *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, bt64bitInt              *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btUnsigned64bitInt      *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btFloat                 *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btcString               *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btObjectType            *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, INamedValueSet const   **pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btByteArray             *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, bt32bitIntArray         *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, bt64bitIntArray         *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btFloatArray            *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btStringArray           *pValue) const { return m_NVS.Get(Name, pValue); }
ENamedValues EmptyIAALUnMarshaller::Get(btStringKey Name, btObjectArray           *pValue) const { return m_NVS.Get(Name, pValue); }

void EmptyIAALUnMarshaller::importmsg(char const *pmsg, btWSSize len) {}

EmptyServiceBase::EmptyServiceBase(AALServiceModule *container,
                                   IRuntime         *pAALRUNTIME,
                                   IAALTransport    *ptransport,
                                   IAALMarshaller   *marshaller,
                                   IAALUnMarshaller *unmarshaller) :
   ServiceBase(container, pAALRUNTIME, ptransport, marshaller, unmarshaller)
{}

void EmptyServiceBase::init(TransactionID const & ) {}

////////////////////////////////////////////////////////////////////////////////

CallTrackingIServiceClient::CallTrackingIServiceClient(btApplicationContext Ctx) :
   EmptyIServiceClient(Ctx)
{}

void CallTrackingIServiceClient::serviceAllocated(IBase               *pBase,
                                                  TransactionID const &tid)
{
   MethodCallLogEntry *l = AddToLog("IServiceClient::serviceAllocated");
   l->AddParam("pBase", reinterpret_cast<btObjectType>(pBase));
   l->AddParam("tid",   tid);
}

void CallTrackingIServiceClient::serviceAllocateFailed(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IServiceClient::serviceAllocateFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIServiceClient::serviceReleased(TransactionID const &tid)
{
   MethodCallLogEntry *l = AddToLog("IServiceClient::serviceReleased");
   l->AddParam("tid", tid);
}

void CallTrackingIServiceClient::serviceReleaseFailed(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IServiceClient::serviceReleaseFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIServiceClient::serviceEvent(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IServiceClient::serviceEvent");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}


CallTrackingIRuntimeClient::CallTrackingIRuntimeClient(btApplicationContext Ctx) :
   EmptyIRuntimeClient(Ctx)
{}

void CallTrackingIRuntimeClient::runtimeCreateOrGetProxyFailed(IEvent const &e)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeCreateOrGetProxyFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIRuntimeClient::runtimeStarted(IRuntime            *pRuntime,
                                                const NamedValueSet &nvs)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStarted");
   l->AddParam("pRuntime", reinterpret_cast<btObjectType>(pRuntime));
   l->AddParam("nvs",      &nvs);
}

void CallTrackingIRuntimeClient::runtimeStopped(IRuntime *pRuntime)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStopped");
   l->AddParam("pRuntime", reinterpret_cast<btObjectType>(pRuntime));
}

void CallTrackingIRuntimeClient::runtimeStartFailed(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStartFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIRuntimeClient::runtimeStopFailed(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStopFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIRuntimeClient::runtimeAllocateServiceFailed(IEvent const &e)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeAllocateServiceFailed");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}

void CallTrackingIRuntimeClient::runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                                 TransactionID const &tid)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeAllocateServiceSucceeded");
   l->AddParam("pServiceBase", reinterpret_cast<btObjectType>(pServiceBase));
   l->AddParam("tid",          tid);
}

void CallTrackingIRuntimeClient::runtimeEvent(const IEvent &e)
{
   MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeEvent");
   l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
}



IBase * CallTrackingISvcsFact::CreateServiceObject(AALServiceModule *container,
                                                   IRuntime         *pRuntime)
{
   MethodCallLogEntry *l = AddToLog("ISvcsFact::CreateServiceObject");
   l->AddParam("container", reinterpret_cast<btObjectType>(container));
   l->AddParam("pRuntime",  reinterpret_cast<btObjectType>(pRuntime));
   return EmptyISvcsFact::CreateServiceObject(container, pRuntime);
}

btBool CallTrackingISvcsFact::InitializeService(IBase               *Client,
                                                TransactionID const &rtid,
                                                NamedValueSet const &optArgs)
{
   MethodCallLogEntry *l = AddToLog("ISvcsFact::InitializeService");
   l->AddParam("Client",  reinterpret_cast<btObjectType>(Client));
   l->AddParam("rtid",    rtid);
   l->AddParam("optArgs", dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(optArgs) ));
   return EmptyISvcsFact::InitializeService(Client, rtid, optArgs);
}


CallTrackingIRuntime::CallTrackingIRuntime(btApplicationContext Ctx) :
   EmptyIRuntime(Ctx)
{}

btBool CallTrackingIRuntime::start(const NamedValueSet &rconfigParms)
{
   MethodCallLogEntry *l = AddToLog("IRuntime::start");
   l->AddParam("rconfigParms", dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(rconfigParms) ) );
   return EmptyIRuntime::start(rconfigParms);
}

void CallTrackingIRuntime::stop()
{
   AddToLog("IRuntime::stop");
}

void CallTrackingIRuntime::allocService(IBase               *pClient,
                                        NamedValueSet const &rManifest,
                                        TransactionID const &rTranID)
{
   MethodCallLogEntry *l = AddToLog("IRuntime::allocService");
   l->AddParam("pClient",   reinterpret_cast<btObjectType>(pClient));
   l->AddParam("rManifest", dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(rManifest) ) );
   l->AddParam("rTranID",   rTranID);
}

btBool CallTrackingIRuntime::schedDispatchable(IDispatchable *pDisp)
{
   MethodCallLogEntry *l = AddToLog("IRuntime::schedDispatchable");
   l->AddParam("pDisp", reinterpret_cast<btObjectType>(pDisp));
   return EmptyIRuntime::schedDispatchable(pDisp);
}

IRuntime * CallTrackingIRuntime::getRuntimeProxy(IRuntimeClient *pClient)
{
   MethodCallLogEntry *l = AddToLog("IRuntime::getRuntimeProxy");
   l->AddParam("pClient", reinterpret_cast<btObjectType>(pClient));
   return EmptyIRuntime::getRuntimeProxy(pClient);
}

btBool CallTrackingIRuntime::releaseRuntimeProxy(IRuntime *pRuntime)
{
   MethodCallLogEntry *l = AddToLog("IRuntime::releaseRuntimeProxy0");
   l->AddParam("pRuntime", reinterpret_cast<btObjectType>(pRuntime));
   return EmptyIRuntime::releaseRuntimeProxy(pRuntime);
}

btBool CallTrackingIRuntime::releaseRuntimeProxy()
{
   AddToLog("IRuntime::releaseRuntimeProxy1");
   return EmptyIRuntime::releaseRuntimeProxy();
}

IRuntimeClient * CallTrackingIRuntime::getRuntimeClient()
{
   AddToLog("IRuntime::getRuntimeClient");
   return EmptyIRuntime::getRuntimeClient();
}

btBool CallTrackingIRuntime::IsOK()
{
   AddToLog("IRuntime::IsOK");
   return EmptyIRuntime::IsOK();
}


CallTrackingServiceBase::CallTrackingServiceBase(AALServiceModule *container,
                                                 IRuntime         *pAALRUNTIME,
                                                 IAALTransport    *ptransport,
                                                 IAALMarshaller   *marshaller,
                                                 IAALUnMarshaller *unmarshaller) :
   EmptyServiceBase(container, pAALRUNTIME, ptransport, marshaller, unmarshaller)
{}

void CallTrackingServiceBase::init(TransactionID const &rtid)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::init");
   l->AddParam("rtid", rtid);
}
