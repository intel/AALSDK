// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_H__
#define __GTCOMMON_H__
#include <cstdio>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
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

template <typename X>
void MySort(X *p, btUnsignedInt n)
{
   // Selection Sort
   btUnsignedInt i;
   btUnsignedInt j;
   btUnsignedInt k;

   for ( i = 0 ; i < n ; ++i ) {
      k = i;
      for ( j = i + 1 ; j < n ; ++j ) {
         if ( *(p + j) < *(p + k) ) {
            k = j;
         }
      }
      std::swap(*(p + i), *(p + k));
   }
}

template <typename X>
X UniqueIntRand(X *p, btUnsignedInt n, X mod, btUnsigned32bitInt *R)
{
   X             res;
   btBool        unique;
   btUnsignedInt i;

   do
   {
      res = (X) GetRand(R);
      if ( mod > 0 ) {
         res %= mod;
      }

      unique = true;
      for ( i = 0 ; i < n ; ++i ) {
         if ( res == *(p + i) ) {
            unique = false;
            break;
         }
      }

   }while( !unique );

   return res;
}

////////////////////////////////////////////////////////////////////////////////

template <typename I>
class TIntVerifier
{
public:
   TIntVerifier() {}
   void expect_eq(I a, I b) { EXPECT_EQ(a, b); }
   void expect_ne(I a, I b) { EXPECT_NE(a, b); }
};

template <typename F>
class TFltVerifier
{
public:
   TFltVerifier() {}
   void expect_eq(F a, F b) { FAIL(); }
   void expect_ne(F a, F b) { EXPECT_NE(a, b); }
};

// specialization for btFloat.
template <>
class TFltVerifier<btFloat>
{
public:
   TFltVerifier() {}
   void expect_eq(btFloat a, btFloat b) { EXPECT_FLOAT_EQ(a, b); }
};

template <typename S>
class TStrVerifier
{
public:
   TStrVerifier() {}
   void expect_eq(S a, S b) { EXPECT_STREQ(a, b); }
   void expect_ne(S a, S b) { EXPECT_STRNE(a, b); }
};

class NVSVerifier
{
public:
   NVSVerifier() {}
   void expect_eq(const INamedValueSet *a, const INamedValueSet *b)
   {
      EXPECT_TRUE( a->operator == (*b) ) << *a << "\nvs.\n" << *b;
   }
   void expect_ne(const INamedValueSet *a, const INamedValueSet *b)
   {
      EXPECT_FALSE( a->operator == (*b) ) << *a << "\nvs.\n" << *b;
   }
};

////////////////////////////////////////////////////////////////////////////////

template <typename I>
class TIntCompare
{
public:
   TIntCompare() {}
   bool equal(I a, I b) { return a == b; }
};

template <typename F>
class TFltCompare
{
public:
   TFltCompare() {}
   bool equal(F a, F b) { return a == b; }
};

template <typename S>
class TStrCompare
{
public:
   TStrCompare() {}
   bool equal(S a, S b) { return (0 == ::strcmp(a, b)); }
};

class NVSCompare
{
public:
   NVSCompare() {}
   bool equal(const INamedValueSet *a, const INamedValueSet *b) { return a->operator == (*b); }
};

////////////////////////////////////////////////////////////////////////////////

template <typename X, typename Compare>
class TValueSequencer
{
public:
   TValueSequencer(const X *pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count),
      m_i(0),
      m_Savei(0)
   {}
   virtual ~TValueSequencer() {}

   void Snapshot() { m_Savei = m_i; }
   void Replay()   { m_i = m_Savei; }

   const X & Value()
   {
      btUnsigned32bitInt i = m_i;
      m_i = (m_i + 1) % m_Count;
      return *(m_pValues + i);
   }

   const X & ValueOtherThan(const X &x)
   {
      Compare            c;
      btUnsigned32bitInt i;
      do
      {
         i = m_i;
         m_i = (m_i + 1) % m_Count;
      }while ( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

   btUnsigned32bitInt Count() const { return m_Count; }

   btUnsigned32bitInt Seed(btUnsigned32bitInt ) { return 0; }

   virtual eBasicTypes BasicType() const = 0;

protected:
   TValueSequencer() {}

   const X                 *m_pValues;
   const btUnsigned32bitInt m_Count;
   btUnsigned32bitInt       m_i;
   btUnsigned32bitInt       m_Savei;
};

template <typename X, typename Compare>
class TValueRandomizer
{
public:
   TValueRandomizer(const X *pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count),
      m_Seed(0),
      m_SaveSeed(0)
   {}
   virtual ~TValueRandomizer() {}

   void Snapshot() { m_SaveSeed = m_Seed;     }
   void Replay()   { m_Seed     = m_SaveSeed; }

   const X & Value()
   {
      btUnsigned32bitInt i = ::GetRand(&m_Seed) % m_Count;
      return *(m_pValues + i);
   }

   const X & ValueOtherThan(const X &x)
   {
      Compare            c;
      btUnsigned32bitInt i;
      do
      {
         i = ::GetRand(&m_Seed) % m_Count;
      }while ( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

   btUnsigned32bitInt Count() const { return m_Count; }

   btUnsigned32bitInt Seed(btUnsigned32bitInt s)
   {
      btUnsigned32bitInt prev = m_Seed;
      m_Seed = s;
      return prev;
   }

   virtual eBasicTypes BasicType() const = 0;

protected:
   TValueRandomizer() {}

   const X                 *m_pValues;
   const btUnsigned32bitInt m_Count;
   btUnsigned32bitInt       m_Seed;
   btUnsigned32bitInt       m_SaveSeed;
};

template <typename ElemT, typename Compare, typename ArrT>
class TArrayProvider
{
public:
   TArrayProvider(const ArrT pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count)
   {}
   virtual ~TArrayProvider() {}

                const ArrT Array() const { return m_pValues; }
        btUnsigned32bitInt Count() const { return m_Count;   }
   virtual eBasicTypes BasicType() const = 0;

   const ElemT & ValueOtherThan(const ElemT &x)
   {
      Compare            c;
      btUnsigned32bitInt i = m_Count - 1;
      do
      {
         i = (i + 1) % m_Count;
      }while( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

protected:
   TArrayProvider() {}

   const ArrT               m_pValues;
   const btUnsigned32bitInt m_Count;
};

template <typename S>
class IOStreamMixin
{
public:
   IOStreamMixin() :
      m_IOStream(std::ios_base::in|std::ios_base::out|std::ios_base::binary)
   {}

   std::ostream & os() { return dynamic_cast<std::ostream &>(m_IOStream); }
   std::istream & is() { return dynamic_cast<std::istream &>(m_IOStream); }

   const std::ostream & os() const { return dynamic_cast<const std::ostream &>(m_IOStream); }
   const std::istream & is() const { return dynamic_cast<const std::istream &>(m_IOStream); }

   void CheckO(std::ios_base::iostate check) const
   {
      const std::ios_base::iostate common = os().rdstate() & check;
      if ( 0 != common ) {
         std::string flags;
         if ( common & std::ios_base::eofbit ) {
            flags += "eofbit ";
         }
         if ( common & std::ios_base::failbit ) {
            flags += "failbit ";
         }
         if ( common & std::ios_base::badbit ) {
            flags += "badbit ";
         }
         FAIL() << "ostream state: " << flags;
      }
   }

   void CheckI(std::ios_base::iostate check) const
   {
      const std::ios_base::iostate common = is().rdstate() & check;
      if ( 0 != common ) {
         std::string flags;
         if ( common & std::ios_base::eofbit ) {
            flags += "eofbit ";
         }
         if ( common & std::ios_base::failbit ) {
            flags += "failbit ";
         }
         if ( common & std::ios_base::badbit ) {
            flags += "badbit ";
         }
         FAIL() << "istream state: " << flags;
      }
   }

   bool  eof() const { return is().eof(); }
   bool fail() const { return os().fail() || is().fail(); }
   bool  bad() const { return os().bad()  || is().bad();  }

   void ClearEOF()
   {
      os().clear( os().rdstate() & ~std::ios_base::eofbit );
      is().clear( is().rdstate() & ~std::ios_base::eofbit );
   }
   void ClearFail()
   {
      os().clear( os().rdstate() & ~std::ios_base::failbit );
      is().clear( is().rdstate() & ~std::ios_base::failbit );
   }
   void ClearBad()
   {
      os().clear( os().rdstate() & ~std::ios_base::badbit );
      is().clear( is().rdstate() & ~std::ios_base::badbit );
   }

   std::ios_base::streampos InputBytesRemaining()
   {
      const std::ios_base::streampos curpos = is().tellg();

      is().seekg(0, std::ios_base::end);

      const std::ios_base::streampos endpos = is().tellg();

      is().seekg(curpos, std::ios_base::beg);

      return endpos - curpos;
   }

protected:
   S m_IOStream;
};

class FILEMixin
{
public:
   FILEMixin() {}
   virtual ~FILEMixin();

   FILE * fopen_tmp();
   btBool    fclose(FILE * );

   void rewind(FILE * ) const;
   int    feof(FILE * ) const;
   int  ferror(FILE * ) const;

   long InputBytesRemaining(FILE * ) const;

protected:
   struct FILEInfo
   {
      FILEInfo(std::string fname, int fd) :
         m_fname(fname),
         m_fd(fd)
      {}
      std::string m_fname;
      int         m_fd;
   };

   typedef std::map< FILE * , FILEInfo > map_type;
   typedef map_type::iterator            iterator;
   typedef map_type::const_iterator      const_iterator;

   map_type m_FileMap;
};

////////////////////////////////////////////////////////////////////////////////

template <typename X>
X PassReturnByValue(X x) { return x; }

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

#define ASSERT_NONNULL(x) ASSERT_NE((void *)NULL, x)
#define ASSERT_NULL(x)    ASSERT_EQ((void *)NULL, x)
#define EXPECT_NONNULL(x) EXPECT_NE((void *)NULL, x)
#define EXPECT_NULL(x)    EXPECT_EQ((void *)NULL, x)

// Retrieve the current test case and test name from gtest.
// Must be called within the context of a test case/fixture.
void TestCaseName(std::string &TestCase, std::string &Test);

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
//   static void  KeepAliveCleanup(void * );
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

////////////////////////////////////////////////////////////////////////////////

class MethodCallLogEntry
{
public:
   MethodCallLogEntry(btcString method, Timer timestamp=Timer()) :
      m_TimeStamp(timestamp)
   {
      m_NVS.Add((btStringKey)"__func__", method);
   }

   btcString MethodName() const
   {
      btcString n = NULL;
      m_NVS.Get((btStringKey)"__func__", &n);
      return n;
   }

   void AddParam(btcString name, btBool             value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btByte             value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, bt32bitInt         value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btUnsigned32bitInt value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, bt64bitInt         value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btUnsigned64bitInt value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btFloat            value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btcString          value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, btObjectType       value) { m_NVS.Add(name, value); }
   void AddParam(btcString name, INamedValueSet    *value) { m_NVS.Add(name, value); }

   void AddParam(btcString name, const TransactionID &value)
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

   unsigned Params() const
   {
      btUnsignedInt u = 0;
      m_NVS.GetNumNames(&u);
      return (unsigned)(u - 1);
   }

   void GetParam(btcString name, btBool                *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btByte                *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, bt32bitInt            *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btUnsigned32bitInt    *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, bt64bitInt            *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btUnsigned64bitInt    *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btFloat               *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btcString             *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, btObjectType          *pValue) const { m_NVS.Get(name, pValue); }
   void GetParam(btcString name, INamedValueSet const **pValue) const { m_NVS.Get(name, pValue); }

   void GetParam(btcString name, TransactionID &tid) const
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

protected:
   Timer         m_TimeStamp;
   NamedValueSet m_NVS;
};

class MethodCallLog : public CriticalSection
{
public:
   MethodCallLog() {}

   MethodCallLogEntry * AddToLog(btcString method)
   {
      AutoLock(this);

      m_LogList.push_back(MethodCallLogEntry(method));

      iterator iter = m_LogList.end();
      --iter;
      return &(*iter);
   }

   unsigned LogEntries() const { AutoLock(this); return m_LogList.size(); }

   const MethodCallLogEntry & Entry(unsigned i) const
   {
      AutoLock(this);

      const_iterator iter;
      for ( iter = m_LogList.begin() ; i-- ; ++iter ) { ; /* traverse */ }
      return *iter;
   }

   void ClearLog() { m_LogList.clear(); }

protected:
   typedef std::list<MethodCallLogEntry> LogList;
   typedef LogList::iterator             iterator;
   typedef LogList::const_iterator       const_iterator;

   LogList m_LogList;
};

class CallTrackingServiceClient : public AAL::IServiceClient,
                                  public MethodCallLog
{
public:
   CallTrackingServiceClient() {}

   virtual void serviceAllocated(IBase               *pBase,
                                 TransactionID const &tid= TransactionID())
   {
      MethodCallLogEntry *l = AddToLog("IServiceClient::serviceAllocated");
      l->AddParam("pBase", reinterpret_cast<btObjectType>(pBase));
      l->AddParam("tid",   tid);
   }
   virtual void serviceAllocateFailed(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IServiceClient::serviceAllocateFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void serviceReleased(TransactionID const &tid= TransactionID())
   {
      MethodCallLogEntry *l = AddToLog("IServiceClient::serviceReleased");
      l->AddParam("tid", tid);
   }
   virtual void serviceReleaseFailed(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IServiceClient::serviceReleaseFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void serviceEvent(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IServiceClient::serviceEvent");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
};

class CallTrackingRuntimeClient : public AAL::IRuntimeClient,
                                  public MethodCallLog
{
public:
   CallTrackingRuntimeClient() {}

   virtual void runtimeCreateOrGetProxyFailed(IEvent const &e)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeCreateOrGetProxyFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void runtimeStarted(IRuntime            *pRuntime,
                               const NamedValueSet &nvs)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStarted");
      l->AddParam("pRuntime", reinterpret_cast<btObjectType>(pRuntime));
      l->AddParam("nvs",      &nvs);
   }
   virtual void runtimeStopped(IRuntime *pRuntime)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStopped");
      l->AddParam("pRuntime", reinterpret_cast<btObjectType>(pRuntime));
   }
   virtual void runtimeStartFailed(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStartFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void runtimeStopFailed(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeStopFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void runtimeAllocateServiceFailed(IEvent const &e)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeAllocateServiceFailed");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
   virtual void runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                TransactionID const &tid)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeAllocateServiceSucceeded");
      l->AddParam("pServiceBase", reinterpret_cast<btObjectType>(pServiceBase));
      l->AddParam("tid",          tid);
   }
   virtual void runtimeEvent(const IEvent &e)
   {
      MethodCallLogEntry *l = AddToLog("IRuntimeClient::runtimeEvent");
      l->AddParam("e", reinterpret_cast<btObjectType>( & const_cast<IEvent &>(e) ));
   }
};

#endif // __GTCOMMON_H__

