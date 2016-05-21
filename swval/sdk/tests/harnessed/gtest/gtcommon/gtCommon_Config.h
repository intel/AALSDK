// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_CONFIG_H__
#define __GTCOMMON_CONFIG_H__

// We need these two prototypes from dbg_oslthread_0.cpp
// The 2nd is Windows-specific.

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)
OSAL_API AAL::btUIntPtr DbgOSLThreadCount();
#ifdef __AAL_WINDOWS__
OSAL_API void DbgOSLThreadDelThr(AAL::btTID );
#endif // __AAL_WINDOWS__
   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)


// A place to keep data about..
//  * app command-line arguments that are relevant to all test cases or to just the app.
//    * seed for RNG, VPATH (for Autotools / LD_LIBRARY_PATH)
//  * general config settings that are relevant to all test cases
//    * max number of yields for YIELD_N() and YIELD_N_FOREACH() macros.
class GTCOMMON_API GlobalTestConfig
{
public:
   static GlobalTestConfig & GetInstance();

   // Certain thread tests require entering a tight loop, yielding the cpu in order
   // to allow other threads to reach some state. Defines the max number of polls
   // for such loops.
   AAL::btUIntPtr MaxCPUYieldPolls() const { return 100; }

   AAL::btUIntPtr CurrentThreads() const
   {
      return AAL::Testing::DbgOSLThreadCount();
   }

   AAL::btUnsigned32bitInt RandSeed() const
   {
      return (AAL::btUnsigned32bitInt) ::testing::UnitTest::GetInstance()->random_seed();
   }

   void        Vpath(const std::string &s) { m_Vpath = s;    }
   std::string Vpath() const               { return m_Vpath; }

protected:
   GlobalTestConfig();
   virtual ~GlobalTestConfig();

   std::string m_Vpath; // Root dir of the configure'd build tree.

   static GlobalTestConfig sm_Instance;
};

// Some Sleep / Yield stuff that doesn't warrant a separate source file..

#if   defined( __AAL_WINDOWS__ )
# define cpu_yield()       ::Sleep(0)
# define sleep_millis(__x) ::Sleep(__x)
# define sleep_sec(__x)    ::Sleep(1000 * (__x))
#elif defined( __AAL_LINUX__ )
# define cpu_yield()       ::usleep(0)
# define sleep_millis(__x) ::usleep((__x) * 1000)
# define sleep_sec(__x)    ::sleep(__x)
#endif // OS

#define MINUTES_IN_TERMS_OF_MILLIS(__x) ( ((AAL::btTime)__x) * ((AAL::btTime)60000) )
#define HOURS_IN_TERMS_OF_MILLIS(__x)   ( ((AAL::btTime)__x) * ((AAL::btTime)60000) * ((AAL::btTime)60) )

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

#endif // __GTCOMMON_CONFIG_H__

