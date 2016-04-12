// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

GlobalTestConfig GlobalTestConfig::sm_Instance;
GlobalTestConfig & GlobalTestConfig::GetInstance()
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
   int   fd;
   FILE *fp = NULL;

#if   defined( __AAL_WINDOWS__ )

   char tmpdir[265];
   char fname[MAX_PATH];
   const char *prefix = "gtest";

   if ( !GetTempPath(sizeof(tmpdir), tmpdir) ) {
      return NULL;
   }

   if ( !GetTempFileName(tmpdir, prefix, 0, fname) ) {
      return NULL;
   }

   if ( ::fopen_s(&fp, fname, "w+b") ) {
      return NULL;
   }

   fd = _fileno(fp);

#elif defined( __AAL_LINUX__ )

   char fname[13] = { 'g', 't', 'e', 's', 't', '.', 'X', 'X', 'X', 'X', 'X', 'X', 0 };

   fd = ::mkstemp(fname);

   if ( -1 == fd ) {
      return NULL;
   }

   fp = ::fdopen(fd, "w+b");

   if ( NULL == fp ) {
      ::close(fd);
      ::remove(fname);
      return NULL;
   }

#endif // OS

   FILEInfo info(fname, fd);

   std::pair<iterator, bool> res = m_FileMap.insert(std::make_pair(fp, info));

   if ( !res.second ) {
      ::fclose(fp);
      ::remove(fname);
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

ConsoleColorizer ConsoleColorizer::sm_Instance;
ConsoleColorizer & ConsoleColorizer::GetInstance()
{
   return ConsoleColorizer::sm_Instance;
}

bool ConsoleColorizer::HasColors(ConsoleColorizer::Stream s)
{
#if   defined( __AAL_WINDOWS__ )
   return _isatty((int)s) ? true : false;
#elif defined( __AAL_LINUX__ )
   return isatty((int)s) ? true : false;
#endif // OS
}

void ConsoleColorizer::Red(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT :

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Red;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR :
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Red;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Green(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT:

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Green;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR:
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Green;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Blue(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT:

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Blue;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR:
#if defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Blue;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Reset(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
      switch ( s ) {
         case STD_COUT:
            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Reset;
#elif defined( __AAL_WINDOWS__ )
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_OldStdoutAttrs);
#endif // OS

            fflush(stdout);
         break;

         case STD_CERR:
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Reset;
#elif defined( __AAL_WINDOWS__ )
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), m_OldStderrAttrs);
#endif // OS
         break;
      }
   }
}

ConsoleColorizer::ConsoleColorizer()
{
#if defined( __AAL_WINDOWS__ )
   CONSOLE_SCREEN_BUFFER_INFO bufinfo;

   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufinfo);
   m_OldStdoutAttrs = bufinfo.wAttributes;

   GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &bufinfo);
   m_OldStderrAttrs = bufinfo.wAttributes;
#endif // __AAL_WINDOWS__
}

#ifdef __AAL_LINUX__
const char ConsoleColorizer::sm_Red[]   = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char ConsoleColorizer::sm_Green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char ConsoleColorizer::sm_Blue[]  = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
const char ConsoleColorizer::sm_Reset[] = { 0x1b, '[', '0', 'm', 0 };
#endif // OS

////////////////////////////////////////////////////////////////////////////////

bool TestStatus::sm_HaltOnSegFault         = false;
bool TestStatus::sm_HaltOnKeepaliveTimeout = false;

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
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Green(ConsoleColorizer::STD_COUT);
   std::cout << "\nPASS\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Green(ConsoleColorizer::STD_CERR);
   std::cerr << "\nPASS\n";
   color.Reset(ConsoleColorizer::STD_CERR);
}

void TestStatus::OnFail()
{
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nFAIL\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nFAIL\n";
   color.Reset(ConsoleColorizer::STD_CERR);
}

void TestStatus::OnSegFault()
{
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nSegmentation Fault during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nSegmentation Fault during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_CERR);

   if ( TestStatus::sm_HaltOnSegFault ) {

      KeepAliveTimerEnv::GetInstance()->StopThread();

      int i = 0;
      while ( TestStatus::sm_HaltOnSegFault ) {
         if ( 0 == (i % (5 * 60)) ) {
            color.Blue(ConsoleColorizer::STD_CERR);
            std::cerr << "Halted for debugger attach.\n";
            color.Reset(ConsoleColorizer::STD_CERR);

            i = 0;
         }
         sleep_sec(1);
         ++i;
      }

   }

   ::exit(97);
}

void TestStatus::OnTerminated()
{
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nProcess Terminated\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nProcess Terminated\n";
   color.Reset(ConsoleColorizer::STD_CERR);

   ::exit(98);
}

void TestStatus::OnKeepaliveTimeout()
{
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_CERR);

   int i = 0;
   while ( TestStatus::sm_HaltOnKeepaliveTimeout ) {
      if ( 0 == (i % (5 * 60)) ) {
         color.Blue(ConsoleColorizer::STD_CERR);
         std::cerr << "Halted for debugger attach.\n";
         color.Reset(ConsoleColorizer::STD_CERR);

         i = 0;
      }
      sleep_sec(1);
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

int UnRequireLD_LIBRARY_PATH(const char *path)
{
   int   res  = 0;
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL == pvar ) {
      // not found, so nothing can possibly be removed.
      return 1;
   }

   if ( NULL == strchr(pvar, ':') ) {
      // There is only one path present.
      if ( 0 == strcmp(pvar, path) ) {
         // The variable is set to the one path to remove. Unset LD_LIBRARY_PATH.
         return unsetenv("LD_LIBRARY_PATH");
      } else {
         // path is not found in LD_LIBRARY_PATH. Nothing to do.
         return 2;
      }
   }

   // LD_LIBRARY_PATH contains a list of paths separated by :

   char *pnewval = (char *) malloc(strlen(pvar) + 1);
   *pnewval = 0;

   char *pcopyvar  = strdup(pvar);
   char *psavecopy = pcopyvar;

   int   cnt = 0;
   char *pcolon;

   if ( NULL == pcopyvar ) {
      res = 3;
      goto _DONE;
   }

   while ( NULL != (pcolon = strchr(pcopyvar, ':')) ) {

      *pcolon = 0;

      if ( 0 != strcmp(pcopyvar, path) ) {
         // This one is not an instance of path, so copy it into the new value..

         if ( cnt > 0 ) {
            strcat(pnewval, ":");
         }

         strcat(pnewval, pcopyvar);

         ++cnt;
      }

      pcopyvar = pcolon + 1;

   }

   // Check the last instance..
   if ( 0 != strcmp(pcopyvar, path) ) {
      if ( cnt > 0 ) {
         strcat(pnewval, ":");
      }
      strcat(pnewval, pcopyvar);
   }

   if ( 0 == strlen(pnewval) ) {
      // There were multiple copies of path, but nothing else.
      res = unsetenv("LD_LIBRARY_PATH");
   } else {
      res = setenv("LD_LIBRARY_PATH", pnewval, 1);
   }

_DONE:
   if ( NULL != psavecopy ) {
      free(psavecopy);
   }
   if ( NULL != pnewval ) {
      free(pnewval);
   }
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

////////////////////////////////////////////////////////////////////////////////

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
   , m_hThread(NULL),
   m_hEvent(NULL)
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
   WaitForSingleObject(m_hThread, INFINITE);

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

   m_hEvent  =  CreateEvent(NULL,   // no inheritance
                            FALSE,  // auto-reset event
                            FALSE,  // not signaled
                            NULL);  // no name

   m_hThread = CreateThread(NULL,   // no inheritance
                            0,      // default stack size
                            KeepAliveTimerEnv::KeepAliveThread, // fn
                            this,                               // arg
                            0,      // begin thread immediately
                            NULL);  // don't retrieve tid

#endif // OS
}

void KeepAliveTimerEnv::TearDown()
{
   StopThread();

#if   defined( __AAL_LINUX__ )

   pthread_cond_destroy(&m_condition);
   pthread_mutex_destroy(&m_mutex);

#elif defined( __AAL_WINDOWS__ )

   CloseHandle(m_hThread);
   CloseHandle(m_hEvent);

   m_hThread = m_hEvent = NULL;

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
DWORD WINAPI KeepAliveTimerEnv::KeepAliveThread(LPVOID arg)
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

      if ( WAIT_OBJECT_0 == WaitForSingleObject(e->m_hEvent, (DWORD)KeepAliveTimerEnv::sm_KeepAliveFreqMillis) ) {
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
   return 0;
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

void MethodCallLogEntry::AddParam(btcString name, btBool             value)
{ m_Order.push_back(TracksParamOrder(name, btBool_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btByte             value)
{ m_Order.push_back(TracksParamOrder(name, btByte_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, bt32bitInt         value)
{ m_Order.push_back(TracksParamOrder(name, bt32bitInt_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btUnsigned32bitInt value)
{ m_Order.push_back(TracksParamOrder(name, btUnsigned32bitInt_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, bt64bitInt         value)
{ m_Order.push_back(TracksParamOrder(name, bt64bitInt_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btUnsigned64bitInt value)
{ m_Order.push_back(TracksParamOrder(name, btUnsigned64bitInt_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btFloat            value)
{ m_Order.push_back(TracksParamOrder(name, btFloat_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btcString          value)
{ m_Order.push_back(TracksParamOrder(name, btString_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, btObjectType       value)
{ m_Order.push_back(TracksParamOrder(name, btObjectType_t)); m_NVS.Add(name, value); }
void MethodCallLogEntry::AddParam(btcString name, INamedValueSet    *value)
{ m_Order.push_back(TracksParamOrder(name, btNamedValueSet_t)); m_NVS.Add(name, value); }

void MethodCallLogEntry::AddParam(btcString name, const TransactionID &value)
{
   // overload btUnknownType_t to mean const TransactionID & .
   m_Order.push_back(TracksParamOrder(name, btUnknownType_t));

   std::string var(name);
   std::string key;

   stTransactionID_t tidStruct = (stTransactionID_t)value;

   key = var + std::string(".m_Context");
   m_NVS.Add(key.c_str(), reinterpret_cast<btObjectType>(tidStruct.m_Context));

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
   return (btUnsignedInt)m_Order.size();
}

std::string MethodCallLogEntry::ParamName(unsigned i) const
{
   std::list<TracksParamOrder>::const_iterator iter = m_Order.begin();
   while ( i > 0 ) {
      ++iter;
      --i;
   }
   return (*iter).m_ParamName;
}

eBasicTypes MethodCallLogEntry::ParamType(unsigned i) const
{
   std::list<TracksParamOrder>::const_iterator iter = m_Order.begin();
   while ( i > 0 ) {
      ++iter;
      --i;
   }
   return (*iter).m_Type;
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

   key = var + std::string(".m_Context");
   m_NVS.Get(key.c_str(), reinterpret_cast<btObjectType *>(&tidStruct.m_Context));

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

MethodCallLogEntry * MethodCallLog::AddToLog(btcString method) const
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
   return (unsigned)m_LogList.size();
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

std::ostream & operator << (std::ostream &os, const MethodCallLog &l)
{
#define TYPE_CASE(x) case x##_t : os << #x << " "; break

   unsigned i;
   unsigned param;

   const unsigned E = l.LogEntries();
   for ( i = 0 ; i < E ; ++i ) {
      const MethodCallLogEntry &e(l.Entry(i));

      os << e.MethodName() << "(";

      const unsigned P = e.Params();
      for ( param = 0 ; param < P ; ++param ) {
         switch ( e.ParamType(param) ) {
            TYPE_CASE(btBool);
            TYPE_CASE(btByte);
            TYPE_CASE(bt32bitInt);
            TYPE_CASE(btInt);
            TYPE_CASE(btUnsigned32bitInt);
            TYPE_CASE(btUnsignedInt);
            TYPE_CASE(bt64bitInt);
            TYPE_CASE(btUnsigned64bitInt);
            TYPE_CASE(btFloat);
            TYPE_CASE(btString);
            case btNamedValueSet_t : os << "const NamedValueSet &"; break;
            TYPE_CASE(bt32bitIntArray);
            TYPE_CASE(btUnsigned32bitIntArray);
            TYPE_CASE(bt64bitIntArray);
            TYPE_CASE(btUnsigned64bitIntArray);
            TYPE_CASE(btObjectType);
            TYPE_CASE(btFloatArray);
            TYPE_CASE(btStringArray);
            TYPE_CASE(btObjectArray);
            TYPE_CASE(btByteArray);
            case btUnknownType_t : os << "const TransactionID &"; break;
         }
         os << e.ParamName(param);

         if ( P - 1 > param ) {
            os << ", ";
         }
      }

      os << ")" << std::endl;
   }

   return os;
#undef TYPE_CASE
}

////////////////////////////////////////////////////////////////////////////////
// IAALTransport

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

////////////////////////////////////////////////////////////////////////////////
// IAALMarshaller

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

////////////////////////////////////////////////////////////////////////////////
// IAALUnMarshaller

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

////////////////////////////////////////////////////////////////////////////////
// IServiceClient

EmptyIServiceClient::EmptyIServiceClient()
{
   if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ) {
      m_bIsOK = false;
   }
}

CallTrackingIServiceClient::CallTrackingIServiceClient()
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

SynchronizingIServiceClient::SynchronizingIServiceClient()
{
   m_Bar.Create(1, false);
}

void SynchronizingIServiceClient::serviceAllocated(IBase               *pBase,
                                                   TransactionID const &tid)
{
   CallTrackingIServiceClient::serviceAllocated(pBase, tid);
   Post();
}

void SynchronizingIServiceClient::serviceAllocateFailed(const IEvent &e)
{
   CallTrackingIServiceClient::serviceAllocateFailed(e);
   Post();
}

void SynchronizingIServiceClient::serviceReleased(TransactionID const &tid)
{
   CallTrackingIServiceClient::serviceReleased(tid);
   Post();
}

void SynchronizingIServiceClient::serviceReleaseFailed(const IEvent &e)
{
   CallTrackingIServiceClient::serviceReleaseFailed(e);
   Post();
}

void SynchronizingIServiceClient::serviceEvent(const IEvent &e)
{
   CallTrackingIServiceClient::serviceEvent(e);
   Post();
}

btBool SynchronizingIServiceClient::Wait(btTime Timeout)
{
   btBool resWait  = m_Bar.Wait(Timeout);
   btBool resReset = m_Bar.Reset();
   return resWait && resReset;
}

btBool SynchronizingIServiceClient::Post(btUnsignedInt Count) { return m_Bar.Post(Count); }

////////////////////////////////////////////////////////////////////////////////
// IRuntimeClient

EmptyIRuntimeClient::EmptyIRuntimeClient()
{
   if ( EObjOK != SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this)) ) {
      m_bIsOK = false;
   }
}

CallTrackingIRuntimeClient::CallTrackingIRuntimeClient()
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
   l->AddParam("nvs",      dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(nvs)));
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

SynchronizingIRuntimeClient::SynchronizingIRuntimeClient()
{
   m_Bar.Create(1, false);
}

void SynchronizingIRuntimeClient::runtimeCreateOrGetProxyFailed(IEvent const &e)
{
   CallTrackingIRuntimeClient::runtimeCreateOrGetProxyFailed(e);
   Post();
}

void SynchronizingIRuntimeClient::runtimeStarted(IRuntime            *pRuntime,
                                                 const NamedValueSet &nvs)
{
   CallTrackingIRuntimeClient::runtimeStarted(pRuntime, nvs);
   Post();
}

void SynchronizingIRuntimeClient::runtimeStopped(IRuntime *pRuntime)
{
   CallTrackingIRuntimeClient::runtimeStopped(pRuntime);
   Post();
}

void SynchronizingIRuntimeClient::runtimeStartFailed(const IEvent &e)
{
   CallTrackingIRuntimeClient::runtimeStartFailed(e);
   Post();
}

void SynchronizingIRuntimeClient::runtimeStopFailed(const IEvent &e)
{
   CallTrackingIRuntimeClient::runtimeStopFailed(e);
   Post();
}

void SynchronizingIRuntimeClient::runtimeAllocateServiceFailed(IEvent const &e)
{
   CallTrackingIRuntimeClient::runtimeAllocateServiceFailed(e);
   Post();
}

void SynchronizingIRuntimeClient::runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                                  TransactionID const &tid)
{
   CallTrackingIRuntimeClient::runtimeAllocateServiceSucceeded(pServiceBase, tid);
   Post();
}

void SynchronizingIRuntimeClient::runtimeEvent(const IEvent &e)
{
   CallTrackingIRuntimeClient::runtimeEvent(e);
   Post();
}

btBool SynchronizingIRuntimeClient::Wait(btTime Timeout)
{
   btBool resWait  = m_Bar.Wait(Timeout);
   btBool resReset = m_Bar.Reset();
   return resWait && resReset;
}

btBool SynchronizingIRuntimeClient::Post(btUnsignedInt Count) { return m_Bar.Post(Count); }

////////////////////////////////////////////////////////////////////////////////
// ISvcsFact

EmptyISvcsFact::EmptyISvcsFact() :
   m_CreateServiceObject_returns(NULL),
   m_InitializeService_returns(true)
{}

IBase * EmptyISvcsFact::CreateServiceObject(AALServiceModule * ,
                                            IRuntime         * ) { return m_CreateServiceObject_returns; }

btBool EmptyISvcsFact::InitializeService(IBase               * ,
                                         IBase               * ,
                                         TransactionID const & ,
                                         NamedValueSet const & ) { return m_InitializeService_returns; }

IMPLEMENT_RETVAL_ACCESSORS(EmptyISvcsFact, CreateServiceObject, IBase * , m_CreateServiceObject_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyISvcsFact, InitializeService,   btBool ,  m_InitializeService_returns)

IBase * CallTrackingISvcsFact::CreateServiceObject(AALServiceModule *container,
                                                   IRuntime         *pRuntime)
{
   MethodCallLogEntry *l = AddToLog("ISvcsFact::CreateServiceObject");
   l->AddParam("container", reinterpret_cast<btObjectType>(container));
   l->AddParam("pRuntime",  reinterpret_cast<btObjectType>(pRuntime));
   return EmptyISvcsFact::CreateServiceObject(container, pRuntime);
}

void CallTrackingISvcsFact::DestroyServiceObject(IBase *pServiceBase)
{
   MethodCallLogEntry *l = AddToLog("ISvcsFact::DestroyServiceObject");
   l->AddParam("pServiceBase", reinterpret_cast<btObjectType>(pServiceBase));
}

btBool CallTrackingISvcsFact::InitializeService(IBase               *newService,
                                                IBase               *Client,
                                                TransactionID const &rtid,
                                                NamedValueSet const &optArgs)
{
   MethodCallLogEntry *l = AddToLog("ISvcsFact::InitializeService");
   l->AddParam("newService", reinterpret_cast<btObjectType>(newService));
   l->AddParam("Client",     reinterpret_cast<btObjectType>(Client));
   l->AddParam("rtid",       rtid);
   l->AddParam("optArgs",    dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(optArgs) ));
   return EmptyISvcsFact::InitializeService(newService, Client, rtid, optArgs);
}

////////////////////////////////////////////////////////////////////////////////
// IRuntime

EmptyIRuntime::EmptyIRuntime() :
   m_start_returns(true),
   m_schedDispatchable_returns(true),
   m_getRuntimeProxy_returns(NULL),
   m_releaseRuntimeProxy_returns(true),
   m_getRuntimeClient_returns(NULL),
   m_IsOK_returns(true)
{
   if ( EObjOK != SetInterface(iidRuntime, dynamic_cast<IRuntime *>(this)) ) {
      m_bIsOK = false;
   }
}

btBool                      EmptyIRuntime::start(const NamedValueSet & ) { return m_start_returns;               }
btBool          EmptyIRuntime::schedDispatchable(IDispatchable * )       { return m_schedDispatchable_returns;   }
IRuntime *        EmptyIRuntime::getRuntimeProxy(IRuntimeClient * )      { return m_getRuntimeProxy_returns;     }
btBool        EmptyIRuntime::releaseRuntimeProxy()                       { return m_releaseRuntimeProxy_returns; }
IRuntimeClient * EmptyIRuntime::getRuntimeClient()                       { return m_getRuntimeClient_returns;    }
btBool                       EmptyIRuntime::IsOK()                       { return m_IsOK_returns;                }

IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, start,               btBool,            m_start_returns              )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, schedDispatchable,   btBool,            m_schedDispatchable_returns  )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, getRuntimeProxy,     IRuntime * ,       m_getRuntimeProxy_returns    )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, releaseRuntimeProxy, btBool,            m_releaseRuntimeProxy_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, getRuntimeClient,    IRuntimeClient * , m_getRuntimeClient_returns   )
IMPLEMENT_RETVAL_ACCESSORS(EmptyIRuntime, IsOK,                btBool,            m_IsOK_returns               )

CallTrackingIRuntime::CallTrackingIRuntime()
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

btBool CallTrackingIRuntime::releaseRuntimeProxy()
{
   AddToLog("IRuntime::releaseRuntimeProxy");
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

////////////////////////////////////////////////////////////////////////////////
// IServiceBase

EmptyIServiceBase::EmptyIServiceBase() :
   m_initComplete_returns(false),
   m_initFailed_returns(false),
   m_ReleaseComplete_returns(false),
   m_getServiceClient_returns(NULL),
   m_getServiceClientBase_returns(NULL),
   m_getRuntime_returns(NULL),
   m_getRuntimeClient_returns(NULL),
   m_getAALServiceModule_returns(NULL)
{
   if ( EObjOK != SetInterface(iidServiceBase, this) ) {
      m_bIsOK = false;
   }
}

btBool                    EmptyIServiceBase::initComplete(TransactionID const & ) { return m_initComplete_returns;         }
btBool                      EmptyIServiceBase::initFailed(IEvent const * )        { return m_initFailed_returns;           }
btBool                 EmptyIServiceBase::ReleaseComplete()                       { return m_ReleaseComplete_returns;      }
NamedValueSet const &          EmptyIServiceBase::OptArgs()                 const { return m_OptArgs_returns;              }
IServiceClient *      EmptyIServiceBase::getServiceClient()                 const { return m_getServiceClient_returns;     }
IBase *           EmptyIServiceBase::getServiceClientBase()                 const { return m_getServiceClientBase_returns; }
IRuntime *                  EmptyIServiceBase::getRuntime()                 const { return m_getRuntime_returns;           }
IRuntimeClient *      EmptyIServiceBase::getRuntimeClient()                 const { return m_getRuntimeClient_returns;     }
AALServiceModule * EmptyIServiceBase::getAALServiceModule()                 const { return m_getAALServiceModule_returns;  }

IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, initComplete,         btBool,                m_initComplete_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, initFailed,           btBool,                m_initFailed_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, ReleaseComplete,      btBool,                m_ReleaseComplete_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, OptArgs,              NamedValueSet const &, m_OptArgs_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, getServiceClient,     IServiceClient *,      m_getServiceClient_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, getServiceClientBase, IBase *,               m_getServiceClientBase_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, getRuntime,           IRuntime *,            m_getRuntime_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, getRuntimeClient,     IRuntimeClient *,      m_getRuntimeClient_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyIServiceBase, getAALServiceModule,  AALServiceModule *,    m_getAALServiceModule_returns)

CallTrackingIServiceBase::CallTrackingIServiceBase()
{}

btBool CallTrackingIServiceBase::initComplete(TransactionID const &rtid)
{
   MethodCallLogEntry *l = AddToLog("IServiceBase::initComplete");
   l->AddParam("rtid", rtid);
   return EmptyIServiceBase::initComplete(rtid);
}

btBool CallTrackingIServiceBase::initFailed(IEvent const *ptheEvent)
{
   MethodCallLogEntry *l = AddToLog("IServiceBase::initFailed");
   l->AddParam("ptheEvent", reinterpret_cast<btObjectType>( const_cast<IEvent *>(ptheEvent) ));
   return EmptyIServiceBase::initFailed(ptheEvent);
}

btBool CallTrackingIServiceBase::ReleaseComplete()
{
   AddToLog("IServiceBase::ReleaseComplete");
   return EmptyIServiceBase::ReleaseComplete();
}

NamedValueSet const & CallTrackingIServiceBase::OptArgs() const
{
   AddToLog("IServiceBase::OptArgs");
   return EmptyIServiceBase::OptArgs();
}

IServiceClient * CallTrackingIServiceBase::getServiceClient() const
{
   AddToLog("IServiceBase::getServiceClient");
   return EmptyIServiceBase::getServiceClient();
}

IBase * CallTrackingIServiceBase::getServiceClientBase() const
{
   AddToLog("IServiceBase::getServiceClientBase");
   return EmptyIServiceBase::getServiceClientBase();
}

IRuntime * CallTrackingIServiceBase::getRuntime() const
{
   AddToLog("IServiceBase::getRuntime");
   return EmptyIServiceBase::getRuntime();
}

IRuntimeClient * CallTrackingIServiceBase::getRuntimeClient() const
{
   AddToLog("IServiceBase::getRuntimeClient");
   return EmptyIServiceBase::getRuntimeClient();
}

AALServiceModule * CallTrackingIServiceBase::getAALServiceModule() const
{
   AddToLog("IServiceBase::getAALServiceModule");
   return EmptyIServiceBase::getAALServiceModule();
}

////////////////////////////////////////////////////////////////////////////////
// ServiceBase

EmptyServiceBase::EmptyServiceBase(AALServiceModule *container,
                                   IRuntime         *pAALRUNTIME,
                                   IAALTransport    *ptransport,
                                   IAALMarshaller   *marshaller,
                                   IAALUnMarshaller *unmarshaller) :
   ServiceBase(container, pAALRUNTIME, ptransport, marshaller, unmarshaller),
   m_init_returns(false)
{}

btBool EmptyServiceBase::init(IBase * ,
                              NamedValueSet const & ,
                              TransactionID const & ) { return m_init_returns; }

IMPLEMENT_RETVAL_ACCESSORS(EmptyServiceBase, init, btBool, m_init_returns)

void EmptyServiceBase::ServiceClient(IServiceClient *pSvcClient)
{
   m_pclient = pSvcClient;
}

void EmptyServiceBase::ServiceClientBase(IBase *pBase)
{
   m_pclientbase = pBase;
}

void EmptyServiceBase::RT(IRuntime *pRT)
{
   m_Runtime = pRT;
}

CallTrackingServiceBase::CallTrackingServiceBase(AALServiceModule *container,
                                                 IRuntime         *pAALRUNTIME,
                                                 IAALTransport    *ptransport,
                                                 IAALMarshaller   *marshaller,
                                                 IAALUnMarshaller *unmarshaller) :
   EmptyServiceBase(container, pAALRUNTIME, ptransport, marshaller, unmarshaller)
{}

btBool CallTrackingServiceBase::init(IBase               *pclientBase,
                                     NamedValueSet const &optArgs,
                                     TransactionID const &rtid)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::init");
   l->AddParam("pclientBase", reinterpret_cast<btObjectType>(pclientBase));
   l->AddParam("optArgs",     dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(optArgs)) );
   l->AddParam("rtid",        rtid);
   return EmptyServiceBase::init(pclientBase, optArgs, rtid);
}

btBool CallTrackingServiceBase::_init(IBase               *pclientBase,
                                      TransactionID const &rtid,
                                      NamedValueSet const &optArgs,
                                      CAALEvent           *pcmpltEvent)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::_init");
   l->AddParam("pclientBase", reinterpret_cast<btObjectType>(pclientBase));
   l->AddParam("rtid",        rtid);
   l->AddParam("optArgs",     dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(optArgs)) );
   l->AddParam("pcmpltEvent", reinterpret_cast<btObjectType>(pcmpltEvent));
   return EmptyServiceBase::_init(pclientBase, rtid, optArgs, pcmpltEvent);
}

btBool CallTrackingServiceBase::Release(TransactionID const &rTranID, btTime timeout)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::Release");
   l->AddParam("rTranID", rTranID);
   l->AddParam("timeout", timeout);
   return EmptyServiceBase::Release(rTranID, timeout);
}

btBool CallTrackingServiceBase::initComplete(TransactionID const &rtid)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::initComplete");
   l->AddParam("rtid", rtid);
   return EmptyServiceBase::initComplete(rtid);
}

btBool CallTrackingServiceBase::initFailed(IEvent const *ptheEvent)
{
   MethodCallLogEntry *l = AddToLog("ServiceBase::initFailed");
   l->AddParam("ptheEvent", reinterpret_cast<btObjectType>( const_cast<IEvent *>(ptheEvent) ));
   return EmptyServiceBase::initFailed(ptheEvent);
}

btBool CallTrackingServiceBase::ReleaseComplete()
{
   AddToLog("ServiceBase::ReleaseComplete");
   return EmptyServiceBase::ReleaseComplete();
}

NamedValueSet const & CallTrackingServiceBase::OptArgs() const
{
   AddToLog("ServiceBase::OptArgs");
   return EmptyServiceBase::OptArgs();
}

IServiceClient * CallTrackingServiceBase::getServiceClient() const
{
   AddToLog("ServiceBase::getServiceClient");
   return EmptyServiceBase::getServiceClient();
}

IBase * CallTrackingServiceBase::getServiceClientBase() const
{
   AddToLog("ServiceBase::getServiceClientBase");
   return EmptyServiceBase::getServiceClientBase();
}

IRuntime * CallTrackingServiceBase::getRuntime() const
{
   AddToLog("ServiceBase::getRuntime");
   return EmptyServiceBase::getRuntime();
}

IRuntimeClient * CallTrackingServiceBase::getRuntimeClient() const
{
   AddToLog("ServiceBase::getRuntimeClient");
   return EmptyServiceBase::getRuntimeClient();
}

AALServiceModule * CallTrackingServiceBase::getAALServiceModule() const
{
   AddToLog("ServiceBase::getAALServiceModule");
   return EmptyServiceBase::getAALServiceModule();
}

////////////////////////////////////////////////////////////////////////////////
// IServiceModule / IServiceModuleCallback

EmptyServiceModule::EmptyServiceModule() :
   m_Construct_returns(NULL),
   m_ServiceInitialized_returns(false),
   m_ServiceInitFailed_returns(false)
{}

btBool EmptyServiceModule::Construct(IRuntime            *pAALRUNTIME,
                                     IBase               *Client,
                                     TransactionID const &tid,
                                     NamedValueSet const &optArgs) { return m_Construct_returns; }
void          EmptyServiceModule::Destroy()         {}
void  EmptyServiceModule::ServiceReleased(IBase * ) {}
btBool EmptyServiceModule::ServiceInitialized(IBase * , TransactionID const & )
{ return m_ServiceInitialized_returns; }
btBool EmptyServiceModule::ServiceInitFailed(IBase * , IEvent const * )
{ return m_ServiceInitFailed_returns; }

IMPLEMENT_RETVAL_ACCESSORS(EmptyServiceModule, Construct,          btBool, m_Construct_returns         )
IMPLEMENT_RETVAL_ACCESSORS(EmptyServiceModule, ServiceInitialized, btBool, m_ServiceInitialized_returns)
IMPLEMENT_RETVAL_ACCESSORS(EmptyServiceModule, ServiceInitFailed,  btBool, m_ServiceInitFailed_returns )

CallTrackingServiceModule::CallTrackingServiceModule() {}

btBool CallTrackingServiceModule::Construct(IRuntime            *pAALRUNTIME,
                                            IBase               *Client,
                                            TransactionID const &tid,
                                            NamedValueSet const &optArgs)
{
   MethodCallLogEntry *l = AddToLog("IServiceModule::Construct");
   l->AddParam("pAALRUNTIME", reinterpret_cast<btObjectType>(pAALRUNTIME));
   l->AddParam("Client", reinterpret_cast<btObjectType>(Client));
   l->AddParam("tid", tid);
   l->AddParam("optArgs", dynamic_cast<INamedValueSet *>( & const_cast<NamedValueSet &>(optArgs)) );

   return EmptyServiceModule::Construct(pAALRUNTIME, Client, tid, optArgs);
}

void CallTrackingServiceModule::Destroy()
{
   AddToLog("IServiceModule::Destroy");
}

void CallTrackingServiceModule::ServiceReleased(IBase *pService)
{
   MethodCallLogEntry *l = AddToLog("IServiceModuleCallback::ServiceReleased");
   l->AddParam("pService", reinterpret_cast<btObjectType>(pService));
}

btBool CallTrackingServiceModule::ServiceInitialized(IBase               *pService,
                                                     TransactionID const &rtid)
{
   MethodCallLogEntry *l = AddToLog("IServiceModuleCallback::ServiceInitialized");
   l->AddParam("pService", reinterpret_cast<btObjectType>(pService));
   l->AddParam("rtid",     rtid);
   return EmptyServiceModule::ServiceInitialized(pService, rtid);
}

btBool CallTrackingServiceModule::ServiceInitFailed(IBase        *pService,
                                                    IEvent const *pEvent)
{
   MethodCallLogEntry *l = AddToLog("IServiceModuleCallback::ServiceInitFailed");
   l->AddParam("pService", reinterpret_cast<btObjectType>(pService));
   l->AddParam("pEvent",   reinterpret_cast<btObjectType>( const_cast<IEvent *>(pEvent) ));
   return EmptyServiceModule::ServiceInitFailed(pService, pEvent);
}

////////////////////////////////////////////////////////////////////////////////

void AllocSwvalMod(AAL::IRuntime            *pRuntime,
                   AAL::IBase               *pClientBase,
                   const AAL::TransactionID &tid)
{
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalmod");
   configrec.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   NamedValueSet manifest;

   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);
   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "swval module");

   pRuntime->allocService(pClientBase, manifest, tid);
}

void AllocSwvalSvcMod(AAL::IRuntime            *pRuntime,
                      AAL::IBase               *pClientBase,
                      const AAL::TransactionID &tid)
{
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalsvcmod");
   configrec.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   NamedValueSet manifest;

   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);
   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "swval service module");

   pRuntime->allocService(pClientBase, manifest, tid);
}


EmptySwvalSvcClient::EmptySwvalSvcClient()
{
   if ( EObjOK != SetInterface(iidSwvalSvcClient, dynamic_cast<ISwvalSvcClient *>(this)) ) {
      m_bIsOK = false;
   }
}

void EmptySwvalSvcClient::DidSomething(const AAL::TransactionID & , int ) {}


CallTrackingSwvalSvcClient::CallTrackingSwvalSvcClient()
{
   if ( EObjOK != SetInterface(iidSwvalSvcClient, dynamic_cast<ISwvalSvcClient *>(this)) ) {
      m_bIsOK = false;
   }
}

void CallTrackingSwvalSvcClient::DidSomething(const AAL::TransactionID &tid, int val)
{
   MethodCallLogEntry *l = AddToLog("ISwvalSvcClient::DidSomething");
   l->AddParam("tid", tid);
   l->AddParam("val", val);
}


SynchronizingSwvalSvcClient::SynchronizingSwvalSvcClient()
{
   if ( EObjOK != SetInterface(iidSwvalSvcClient, dynamic_cast<ISwvalSvcClient *>(this)) ) {
      m_bIsOK = false;
   }
}

void SynchronizingSwvalSvcClient::DidSomething(const AAL::TransactionID &tid, int val)
{
   MethodCallLogEntry *l = AddToLog("ISwvalSvcClient::DidSomething");
   l->AddParam("tid", tid);
   l->AddParam("val", val);

   Post();
}

