// INTEL CONFIDENTIAL - For Intel Internal Use Only

#define MINUTES_IN_TERMS_OF_MILLIS(__x) ( ((AAL::btTime)__x) * ((AAL::btTime)60000) )
#define HOURS_IN_TERMS_OF_MILLIS(__x)   ( ((AAL::btTime)__x) * ((AAL::btTime)60000) * ((AAL::btTime)60) )

#if   defined( __AAL_WINDOWS__ )
# define cpu_yield()       ::Sleep(0)
# define sleep_millis(__x) ::Sleep(__x)
#elif defined( __AAL_LINUX__ )
# define cpu_yield()       ::usleep(0)
# define sleep_millis(__x) ::usleep((__x) * 1000)
#endif // OS

class GlobalTestConfig
{
public:
   GlobalTestConfig() {}

   // Certain thread tests require entering a tight loop, yielding the cpu in order
   // to allow other threads to reach some state. Defines the max number of polls
   // for such loops.
   AAL::btUIntPtr MaxCPUYieldPolls() const { return 100; }

   // Determine the number of pthread's used in Value-Parameterized test fixtures.
   // Yeah, this is hokey, but ::testing::internal::GetThreadCount() only works for MAC OS.
   AAL::btUnsignedInt   PThreadsUsedInFixtures() const { return 0; }
   AAL::btUnsignedInt PThreadsUsedInVPFixtures() const { return 2; }

} Config;

// Enter a tight loop, yielding the cpu so long as __predicate evaluates to true.
#define YIELD_WHILE(__predicate) \
do                               \
{                                \
   while ( __predicate ) {       \
      cpu_yield();               \
   }                             \
}while(0)

// Yield the cpu Config.MaxCPUYieldPolls() times.
#define YIELD_N()                                        \
do                                                       \
{                                                        \
   AAL::btUIntPtr       __i;                             \
   const AAL::btUIntPtr __N = Config.MaxCPUYieldPolls(); \
   for ( __i = 0 ; __i < __N ; ++__i ) {                 \
      cpu_yield();                                       \
   }                                                     \
}while(0)

// Yield the cpu Config.MaxCPUYieldPolls() times, executing __expr after each yield.
#define YIELD_N_FOREACH(__expr)                          \
do                                                       \
{                                                        \
   AAL::btUIntPtr       __i;                             \
   const AAL::btUIntPtr __N = Config.MaxCPUYieldPolls(); \
   for ( __i = 0 ; __i < __N ; ++__i ) {                 \
      cpu_yield();                                       \
      __expr ;                                           \
   }                                                     \
}while(0)

////////////////////////////////////////////////////////////////////////////////

#define ASSERT_NONNULL(x) ASSERT_NE((void *)NULL, x)
#define ASSERT_NULL(x)    ASSERT_EQ((void *)NULL, x)
#define EXPECT_NONNULL(x) EXPECT_NE((void *)NULL, x)
#define EXPECT_NULL(x)    EXPECT_EQ((void *)NULL, x)

const std::string SampleAFU1ConfigRecord("9 20 ConfigRecordIncluded\n \
      \t10\n \
          \t\t9 17 ServiceExecutable\n \
            \t\t\t9 13 libsampleafu1\n \
         \t\t9 18 _CreateSoftService\n \
         \t\t0 1\n \
   9 29 ---- End of embedded NVS ----\n \
      9999\n");

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
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <signal.h>

class SignalHelper
{
public:
   SignalHelper() {}
   virtual ~SignalHelper();

   typedef void (*handler)(int , siginfo_t * , void * );

   // Does not allow hooking the same signum multiple times.
   // non-zero on error.
   int Install(int signum, handler h, bool oneshot=false);

   static void   EmptySIGIOHandler(int , siginfo_t * , void * );
   static void EmptySIGUSR1Handler(int , siginfo_t * , void * );
   static void EmptySIGUSR2Handler(int , siginfo_t * , void * );

protected:
   typedef std::map<int, struct sigaction> sigmap;
   typedef sigmap::iterator                sigiter;
   typedef sigmap::const_iterator          const_sigiter;

   sigmap m_sigmap;
};

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

#endif // OS

#if   defined( __AAL_WINDOWS__ )
# error TODO implement thread hooks for windows.
#elif defined( __AAL_LINUX__ )
# include "hooksbridge.h"

class PThreadsHooks
{
public:
   PThreadsHooks() {}
   virtual ~PThreadsHooks() {}

   AAL::btUnsignedInt CurrentThreads() const
   {
      // Google Test creates pthread's, too. We need to subtract off the Google Test number of threads here.
      return PThreadsHooks::sm_CurrThreads;
   }

   static PThreadsHooks * Get();
   static void        Release();

protected:

   static pthread_mutex_t sm_Lock;
   static void   Lock();
   static void Unlock();

   static PThreadsHooks  *sm_Obj;

//   static void pre_pthread_create(pthread_t            *pthr,
//                                  const pthread_attr_t *pattr,
//                                  void * (*startfn)(void * ),
//                                  void                 *arg);
   static void post_pthread_create(int                   res,
                                   pthread_t            *pthr,
                                   const pthread_attr_t *pattr,
                                   void * (*startfn)(void * ),
                                   void                 *arg);
//   static void pre_pthread_join(pthread_t thr, void **prtn);
   static void post_pthread_join(int res, pthread_t thr, void **prtn);

   static AAL::btUnsignedInt sm_CurrThreads;
};

pthread_mutex_t PThreadsHooks::sm_Lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
void PThreadsHooks::Lock()   { pthread_mutex_lock(&PThreadsHooks::sm_Lock);   }
void PThreadsHooks::Unlock() { pthread_mutex_unlock(&PThreadsHooks::sm_Lock); }

PThreadsHooks * PThreadsHooks::sm_Obj  = NULL;

PThreadsHooks * PThreadsHooks::Get()
{
   PThreadsHooks::Lock();

   if ( NULL == PThreadsHooks::sm_Obj ) {
      PThreadsHooks::sm_Obj = new (std::nothrow) PThreadsHooks();
      if ( NULL != PThreadsHooks::sm_Obj ) {
         install_post_pthread_create_hook((pthread_create_post_hook) PThreadsHooks::post_pthread_create, true);
         install_post_pthread_join_hook(  (pthread_join_post_hook)   PThreadsHooks::post_pthread_join,   true);
      }
   }

   PThreadsHooks::Unlock();

   return PThreadsHooks::sm_Obj;
}

void PThreadsHooks::Release()
{
   PThreadsHooks::Lock();

   if ( NULL != PThreadsHooks::sm_Obj ) {

      install_post_pthread_create_hook((pthread_create_post_hook) PThreadsHooks::post_pthread_create, false);
      install_post_pthread_join_hook(  (pthread_join_post_hook)   PThreadsHooks::post_pthread_join,   false);

      delete PThreadsHooks::sm_Obj;
      PThreadsHooks::sm_Obj = NULL;
   }

   PThreadsHooks::Unlock();
}

void PThreadsHooks::post_pthread_create(int                   res,
                                        pthread_t            *pthr,
                                        const pthread_attr_t *pattr,
                                        void * (*startfn)(void * ),
                                        void                 *arg)
{
   ASSERT(0 == res);

   if ( 0 == res ) {
      PThreadsHooks::Lock();

      ++PThreadsHooks::sm_CurrThreads;

      PThreadsHooks::Unlock();
   }
}

void PThreadsHooks::post_pthread_join(int res, pthread_t thr, void **prtn)
{
   ASSERT(0 == res);

   if ( 0 == res ) {
      PThreadsHooks::Lock();

      --PThreadsHooks::sm_CurrThreads;

      PThreadsHooks::Unlock();
   }
}

AAL::btUnsignedInt PThreadsHooks::sm_CurrThreads = 0;

#endif // OS

