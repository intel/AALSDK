// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_H__
#define __GTCOMMON_H__
#include <cstdio>
#include <string>
#include <fstream>
#include <list>
#include <map>

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
using namespace AAL;

#include "gtest/gtest.h"

#define MINUTES_IN_TERMS_OF_MILLIS(__x) ( ((AAL::btTime)__x) * ((AAL::btTime)60000) )
#define HOURS_IN_TERMS_OF_MILLIS(__x)   ( ((AAL::btTime)__x) * ((AAL::btTime)60000) * ((AAL::btTime)60) )

#if   defined( __AAL_WINDOWS__ )
# define cpu_yield()       ::Sleep(0)
# define sleep_millis(__x) ::Sleep(__x)
#elif defined( __AAL_LINUX__ )
# define cpu_yield()       ::usleep(0)
# define sleep_millis(__x) ::usleep((__x) * 1000)
#endif // OS

#if defined( __AAL_LINUX__ )
AAL::btUIntPtr DbgOSLThreadCount();
#endif // __AAL_LINUX__
class GlobalTestConfig
{
public:
   static const GlobalTestConfig & GetInstance();

   // Certain thread tests require entering a tight loop, yielding the cpu in order
   // to allow other threads to reach some state. Defines the max number of polls
   // for such loops.
   AAL::btUIntPtr MaxCPUYieldPolls() const { return 100; }

   AAL::btUIntPtr CurrentThreads() const
   {
#if   defined( __AAL_WINDOWS__ )
# error TODO implement GlobalTestConfig::CurrentThreads() for Windows.
#elif defined( __AAL_LINUX__ )
      return DbgOSLThreadCount();
#endif // OS
   }

protected:
   GlobalTestConfig();
   virtual ~GlobalTestConfig();

   static GlobalTestConfig sm_Instance;
};

// Enter a tight loop, yielding the cpu so long as __predicate evaluates to true.
#define YIELD_WHILE(__predicate) \
do                               \
{                                \
   while ( __predicate ) {       \
      cpu_yield();               \
   }                             \
}while(0)

// Yield the cpu the given number of times.
#define YIELD_X(__x)                                 \
do                                                   \
{                                                    \
   AAL::btUIntPtr       __i;                         \
   const AAL::btUIntPtr __N = (AAL::btUIntPtr)(__x); \
   for ( __i = 0 ; __i < __N ; ++__i ) {             \
      cpu_yield();                                   \
   }                                                 \
}while(0)

// Yield the cpu a fixed number of times.
#define YIELD_N()                                                                 \
do                                                                                \
{                                                                                 \
   AAL::btUIntPtr       __i;                                                      \
   const AAL::btUIntPtr __N = GlobalTestConfig::GetInstance().MaxCPUYieldPolls(); \
   for ( __i = 0 ; __i < __N ; ++__i ) {                                          \
      cpu_yield();                                                                \
   }                                                                              \
}while(0)

// Yield the cpu Config.MaxCPUYieldPolls() times, executing __expr after each yield.
#define YIELD_N_FOREACH(__expr)                                                   \
do                                                                                \
{                                                                                 \
   AAL::btUIntPtr       __i;                                                      \
   const AAL::btUIntPtr __N = GlobalTestConfig::GetInstance().MaxCPUYieldPolls(); \
   for ( __i = 0 ; __i < __N ; ++__i ) {                                          \
      cpu_yield();                                                                \
      __expr ;                                                                    \
   }                                                                              \
}while(0)

////////////////////////////////////////////////////////////////////////////////

class TestStatus
{
public:
   enum Status
   {
      STATUS_PASS,
      STATUS_FAIL,
      STATUS_SEGFAULT,
      STATUS_TERMINATED,
      STATUS_KEEPALIVE_TIMEOUT
   };

   static void Report(Status st);

   static void HaltOnSegFault(bool );
   static void HaltOnKeepaliveTimeout(bool );

protected:
   static void OnPass();
   static void OnFail();
   static void OnSegFault();
   static void OnTerminated();
   static void OnKeepaliveTimeout();

   static const char sm_Red[];
   static const char sm_Green[];
   static const char sm_Blue[];
   static const char sm_Reset[];

   static bool       sm_HaltOnSegFault;
   static bool       sm_HaltOnKeepaliveTimeout;
};

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

// Retrieve the current test case and test name from gtest.
// Must be called within the context of a test case/fixture.
void TestCaseName(std::string &TestCase, std::string &Test);

#if defined( __AAL_LINUX__ )

// Make sure that the given path appears in LD_LIBRARY_PATH, preventing duplication.
// Return non-zero on error.
int RequireLD_LIBRARY_PATH(const char *path);

// Print streamer for LD_LIBRARY_PATH.
// Ex.
//   cout << LD_LIBRARY_PATH << endl;
std::ostream & LD_LIBRARY_PATH(std::ostream &os);

#endif // __AAL_LINUX__


class ThreadRegistry : public CriticalSection
{
public:
   ThreadRegistry();

   btUnsignedInt ThreadRegister(btTID );
   btUnsignedInt ThreadLookup(btTID );
   void          RegistryReset();

protected:
   btUnsignedInt m_NextThread;
   btTID         m_RegisteredThreads[50];
};


#if   defined( __AAL_WINDOWS__ )
# error TODO implement SignalHelper class for windows.
#elif defined( __AAL_LINUX__ )
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <signal.h>

class SignalHelper : public ThreadRegistry
{
public:
   enum SigIndex
   {
      IDX_SIGINT = 0,
      IDX_FIRST  = IDX_SIGINT,
      IDX_SIGSEGV,
      IDX_SIGIO,
      IDX_SIGUSR1,
      IDX_SIGUSR2,

      IDX_COUNT
   };

   static SignalHelper & GetInstance();

   // Does not allow hooking the same signum multiple times.
   // non-zero on error.
   int        Install(SigIndex i);

   // non-zero on error.
   int      Uninstall(SigIndex i);


   btUIntPtr GetCount(SigIndex i, btUnsignedInt thr);

protected:
   SignalHelper();
   virtual ~SignalHelper();

   void PutCount(SigIndex i, btUnsignedInt thr);

   typedef void (*handler)(int , siginfo_t * , void * );

   struct SigTracker
   {
      int              signum;
      handler          h;
      btBool           installed;
      struct sigaction orig;
      btUIntPtr        Counts[50]; // support the max number of threads.
   };

   SigTracker    m_Tracker[IDX_COUNT];

   static SignalHelper sm_Instance;

   static void   SIGIOHandler(int , siginfo_t * , void * );
   static void SIGUSR1Handler(int , siginfo_t * , void * );
   static void SIGUSR2Handler(int , siginfo_t * , void * );

   static void SIGSEGVHandler(int , siginfo_t * , void * );
   static void  SIGINTHandler(int , siginfo_t * , void * );
};

#endif // OS

////////////////////////////////////////////////////////////////////////////////

class KeepAliveTimerEnv : public ::testing::Environment
{
public:
   static KeepAliveTimerEnv * GetInstance();
   virtual ~KeepAliveTimerEnv() {}

   void KeepAlive()
   {
      ++m_KeepAliveCounter;
   }

   void StopThread();

   virtual void SetUp();
   virtual void TearDown();

protected:
   KeepAliveTimerEnv();

   void KeepAliveExpired();

   btBool             m_KeepAliveRunning;
   btUnsigned64bitInt m_KeepAliveCounter;
   btUnsignedInt      m_KeepAliveTimeouts;
#if   defined( __AAL_LINUX__ )
   pthread_t          m_thread;
   pthread_mutex_t    m_mutex;
   pthread_cond_t     m_condition;
#elif defined ( __AAL_WINDOWS__ )
   HANDLE             m_hEvent;
   HANDLE             m_hJoinEvent;
#endif // OS

   static       btTime        sm_KeepAliveFreqMillis;
   static const btUnsignedInt sm_MaxKeepAliveTimeouts;

#if   defined( __AAL_LINUX__ )
   static void * KeepAliveThread(void * );
#elif defined ( __AAL_WINDOWS__ )
   static void   KeepAliveThread(void * );
#endif // OS

   static KeepAliveTimerEnv *sm_pInstance;
};

class KeepAliveTestListener : public ::testing::EmptyTestEventListener
{
public:
   virtual void OnTestEnd(const ::testing::TestInfo & /*test_info*/)
   {
      KeepAliveTimerEnv::GetInstance()->KeepAlive();
   }
};

#endif // __GTCOMMON_H__

