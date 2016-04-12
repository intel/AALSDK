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

#include "swvalmod.h"
#include "swvalsvcmod.h"

#include "gtest/gtest.h"

#define MINUTES_IN_TERMS_OF_MILLIS(__x) ( ((AAL::btTime)__x) * ((AAL::btTime)60000) )
#define HOURS_IN_TERMS_OF_MILLIS(__x)   ( ((AAL::btTime)__x) * ((AAL::btTime)60000) * ((AAL::btTime)60) )

#if   defined( __AAL_WINDOWS__ )
# define cpu_yield()       ::Sleep(0)
# define sleep_millis(__x) ::Sleep(__x)
# define sleep_sec(__x)    ::Sleep(1000 * (__x))
#elif defined( __AAL_LINUX__ )
# define cpu_yield()       ::usleep(0)
# define sleep_millis(__x) ::usleep((__x) * 1000)
# define sleep_sec(__x)    ::sleep(__x)
#endif // OS

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)
OSAL_API AAL::btUIntPtr DbgOSLThreadCount();
OSAL_API void DbgOSLThreadDelThr(AAL::btTID );
   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)

class GlobalTestConfig
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

class ConsoleColorizer
{
public:
   enum Stream
   {
      STD_COUT = 1,
      STD_CERR
   };

   static ConsoleColorizer & GetInstance();

   bool HasColors(Stream );

   void   Red(Stream );
   void Green(Stream );
   void  Blue(Stream );
   void Reset(Stream );

protected:
   ConsoleColorizer();

   static ConsoleColorizer sm_Instance;

#if   defined( __AAL_LINUX__ )
   static const char sm_Red[];
   static const char sm_Green[];
   static const char sm_Blue[];
   static const char sm_Reset[];
#elif defined( __AAL_WINDOWS__ )
   WORD m_OldStdoutAttrs;
   WORD m_OldStderrAttrs;
#endif // OS
};

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

   static bool sm_HaltOnSegFault;
   static bool sm_HaltOnKeepaliveTimeout;
};

////////////////////////////////////////////////////////////////////////////////

#if defined( __AAL_LINUX__ )

// Make sure that the given path appears in LD_LIBRARY_PATH, preventing duplication.
// Return non-zero on error.
int RequireLD_LIBRARY_PATH(const char *path);

// Remove the given path from LD_LIBRARY_PATH.
// Return non-zero on error.
int UnRequireLD_LIBRARY_PATH(const char *path);

// Print streamer for LD_LIBRARY_PATH.
// Ex.
//   cout << LD_LIBRARY_PATH << endl;
std::ostream & LD_LIBRARY_PATH(std::ostream &os);

#endif // __AAL_LINUX__

////////////////////////////////////////////////////////////////////////////////

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
# include <signal.h>
#elif defined( __AAL_LINUX__ )
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <signal.h>
#endif // OS

class SignalHelper : public ThreadRegistry
{
public:
   enum SigIndex
   {
#if   defined( __AAL_WINDOWS__ )
      IDX_SIGINT = 0,
      IDX_FIRST = IDX_SIGINT,
      IDX_SIGSEGV,
      IDX_SIGUSR1,
      IDX_SIGUSR2,
#elif defined( __AAL_LINUX__ )
      IDX_SIGINT = 0,
      IDX_FIRST = IDX_SIGINT,
      IDX_SIGSEGV,
      IDX_SIGIO,
      IDX_SIGUSR1,
      IDX_SIGUSR2,
#endif // OS

      IDX_COUNT
   };

   static SignalHelper & GetInstance();

   // Does not allow hooking the same signum multiple times.
   // non-zero on error.
   int        Install(SigIndex i);

   // non-zero on error.
   int      Uninstall(SigIndex i);

   btUIntPtr GetCount(SigIndex i, btUnsignedInt thr);

   // non-zero on error.
   int          Raise(SigIndex i, btTID tid);

protected:
   SignalHelper();
   virtual ~SignalHelper();

   void PutCount(SigIndex i, btUnsignedInt thr);

#if   defined( __AAL_WINDOWS__ )
   typedef void(*handler)(int );
#elif defined( __AAL_LINUX__ )
   typedef void(*handler)(int, siginfo_t *, void *);
#endif // OS

   struct SigTracker
   {
      int              signum;
      handler          h;
      btBool           installed;
#if   defined( __AAL_WINDOWS__ )
      handler          orig;
#elif defined ( __AAL_LINUX__ )
      struct sigaction orig;
#endif // OS
      btUIntPtr        Counts[50]; // support the max number of threads.
   };

   SigTracker m_Tracker[IDX_COUNT];

   static SignalHelper sm_Instance;

#if   defined( __AAL_WINDOWS__ )
   static void  SIGINTHandler(int );
   static void SIGSEGVHandler(int );
   static void SIGUSR1Handler(int );
   static void SIGUSR2Handler(int );
#elif defined( __AAL_LINUX__ )
   static void  SIGINTHandler(int, siginfo_t *, void *);
   static void SIGSEGVHandler(int, siginfo_t *, void *);
   static void   SIGIOHandler(int, siginfo_t *, void *);
   static void SIGUSR1Handler(int, siginfo_t *, void *);
   static void SIGUSR2Handler(int, siginfo_t *, void *);
#endif // OS
};

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
   HANDLE             m_hThread;
   HANDLE             m_hEvent;
#endif // OS

   static       btTime        sm_KeepAliveFreqMillis;
   static const btUnsignedInt sm_MaxKeepAliveTimeouts;

#if   defined( __AAL_LINUX__ )
//   static void  KeepAliveCleanup(void * );
   static void * KeepAliveThread(void * );
#elif defined ( __AAL_WINDOWS__ )
   static DWORD WINAPI KeepAliveThread(LPVOID );
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
   MethodCallLogEntry(btcString method, Timer timestamp=Timer());

   btcString MethodName() const;

   void AddParam(btcString , btBool                );
   void AddParam(btcString , btByte                );
   void AddParam(btcString , bt32bitInt            );
   void AddParam(btcString , btUnsigned32bitInt    );
   void AddParam(btcString , bt64bitInt            );
   void AddParam(btcString , btUnsigned64bitInt    );
   void AddParam(btcString , btFloat               );
   void AddParam(btcString , btcString             );
   void AddParam(btcString , btObjectType          );
   void AddParam(btcString , INamedValueSet *      );
   void AddParam(btcString , const TransactionID & );

   unsigned       Params()           const;
   std::string ParamName(unsigned i) const;
   eBasicTypes ParamType(unsigned i) const;

   void GetParam(btcString , btBool                * ) const;
   void GetParam(btcString , btByte                * ) const;
   void GetParam(btcString , bt32bitInt            * ) const;
   void GetParam(btcString , btUnsigned32bitInt    * ) const;
   void GetParam(btcString , bt64bitInt            * ) const;
   void GetParam(btcString , btUnsigned64bitInt    * ) const;
   void GetParam(btcString , btFloat               * ) const;
   void GetParam(btcString , btcString             * ) const;
   void GetParam(btcString , btObjectType          * ) const;
   void GetParam(btcString , INamedValueSet const ** ) const;
   void GetParam(btcString , TransactionID &         ) const;

protected:
   Timer         m_TimeStamp;
   NamedValueSet m_NVS;

   struct TracksParamOrder
   {
      TracksParamOrder(btcString ParamName, eBasicTypes Type) :
         m_ParamName(ParamName),
         m_Type(Type)
      {}
      std::string m_ParamName;
      eBasicTypes m_Type;
   };

   std::list<TracksParamOrder> m_Order;
};

class MethodCallLog : public CriticalSection
{
public:
   MethodCallLog() {}

   MethodCallLogEntry *    AddToLog(btcString method) const;
   unsigned              LogEntries()                 const;
   const MethodCallLogEntry & Entry(unsigned i)       const;
   void                    ClearLog();

protected:
   typedef std::list<MethodCallLogEntry> LogList;
   typedef LogList::iterator             iterator;
   typedef LogList::const_iterator       const_iterator;

   mutable LogList m_LogList;
};

std::ostream & operator << (std::ostream & , const MethodCallLog & );

////////////////////////////////////////////////////////////////////////////////

#define DECLARE_RETVAL_ACCESSORS(__membfn, __rettype) \
__rettype __membfn##ReturnsThisValue() const;         \
void __membfn##ReturnsThisValue(__rettype );

#define IMPLEMENT_RETVAL_ACCESSORS(__cls, __membfn, __rettype, __membvar) \
__rettype __cls::__membfn##ReturnsThisValue() const { return __membvar; } \
void __cls::__membfn##ReturnsThisValue(__rettype x) { __membvar = x;    }

////////////////////////////////////////////////////////////////////////////////
// IAALTransport

class EmptyIAALTransport : public AAL::IAALTransport
{
public:
   EmptyIAALTransport();

   virtual btBool  connectremote(NamedValueSet const & );
   virtual btBool waitforconnect(NamedValueSet const & );
   virtual btBool     disconnect();
   virtual btcString      getmsg(btWSSize * );
   virtual int            putmsg(btcString , btWSSize );

   DECLARE_RETVAL_ACCESSORS(connectremote,  btBool    )
   DECLARE_RETVAL_ACCESSORS(waitforconnect, btBool    )
   DECLARE_RETVAL_ACCESSORS(disconnect,     btBool    )
   DECLARE_RETVAL_ACCESSORS(getmsg,         btcString )
   DECLARE_RETVAL_ACCESSORS(putmsg,         int       )

protected:
   btBool    m_connectremote_returns;
   btBool    m_waitforconnect_returns;
   btBool    m_disconnect_returns;
   btcString m_getmsg_returns;
   int       m_putmsg_returns;
};

////////////////////////////////////////////////////////////////////////////////
// IAALMarshaller

class EmptyIAALMarshaller : public AAL::IAALMarshaller
{
public:
   EmptyIAALMarshaller();

   virtual ENamedValues   Empty();
   virtual btBool           Has(btStringKey )                   const;
   virtual ENamedValues  Delete(btStringKey );
   virtual ENamedValues GetSize(btStringKey   , btWSSize    * ) const;
   virtual ENamedValues    Type(btStringKey   , eBasicTypes * ) const;
   virtual ENamedValues GetName(btUnsignedInt , btStringKey * ) const;

   virtual ENamedValues Add(btNumberKey Name, btBool                Value);
   virtual ENamedValues Add(btNumberKey Name, btByte                Value);
   virtual ENamedValues Add(btNumberKey Name, bt32bitInt            Value);
   virtual ENamedValues Add(btNumberKey Name, btUnsigned32bitInt    Value);
   virtual ENamedValues Add(btNumberKey Name, bt64bitInt            Value);
   virtual ENamedValues Add(btNumberKey Name, btUnsigned64bitInt    Value);
   virtual ENamedValues Add(btNumberKey Name, btFloat               Value);
   virtual ENamedValues Add(btNumberKey Name, btcString             Value);
   virtual ENamedValues Add(btNumberKey Name, btObjectType          Value);
   virtual ENamedValues Add(btNumberKey Name, const INamedValueSet *Value);

   virtual ENamedValues Add(btNumberKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements);

   virtual ENamedValues Add(btStringKey Name, btBool                Value);
   virtual ENamedValues Add(btStringKey Name, btByte                Value);
   virtual ENamedValues Add(btStringKey Name, bt32bitInt            Value);
   virtual ENamedValues Add(btStringKey Name, btUnsigned32bitInt    Value);
   virtual ENamedValues Add(btStringKey Name, bt64bitInt            Value);
   virtual ENamedValues Add(btStringKey Name, btUnsigned64bitInt    Value);
   virtual ENamedValues Add(btStringKey Name, btFloat               Value);
   virtual ENamedValues Add(btStringKey Name, btcString             Value);
   virtual ENamedValues Add(btStringKey Name, btObjectType          Value);
   virtual ENamedValues Add(btStringKey Name, const INamedValueSet *Value);

   virtual ENamedValues Add(btStringKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements);

   virtual char const * pmsgp(btWSSize *len);

   DECLARE_RETVAL_ACCESSORS(pmsgp, char const *)

protected:
   const char   *m_pmsgp_returns;
   NamedValueSet m_NVS;
};

////////////////////////////////////////////////////////////////////////////////
// IAALUnMarshaller

class EmptyIAALUnMarshaller : public AAL::IAALUnMarshaller
{
public:
   EmptyIAALUnMarshaller();

   virtual ENamedValues   Empty();
   virtual btBool           Has(btStringKey )                   const;
   virtual ENamedValues  Delete(btStringKey );
   virtual ENamedValues GetSize(btStringKey   , btWSSize    * ) const;
   virtual ENamedValues    Type(btStringKey   , eBasicTypes * ) const;
   virtual ENamedValues GetName(btUnsignedInt , btStringKey * ) const;

   virtual ENamedValues Get(btNumberKey Name, btBool                   *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btByte                   *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt32bitInt               *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitInt       *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt64bitInt               *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitInt       *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btFloat                  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btcString                *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btObjectType             *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, INamedValueSet const    **pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btByteArray              *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt32bitIntArray          *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt64bitIntArray          *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btFloatArray             *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btStringArray            *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btObjectArray            *pValue) const;

   virtual ENamedValues Get(btStringKey Name, btBool                   *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btByte                   *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt32bitInt               *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitInt       *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt64bitInt               *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitInt       *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btFloat                  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btcString                *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btObjectType             *pValue) const;
   virtual ENamedValues Get(btStringKey Name, INamedValueSet const    **pValue) const;
   virtual ENamedValues Get(btStringKey Name, btByteArray              *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt32bitIntArray          *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt64bitIntArray          *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btFloatArray             *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btStringArray            *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btObjectArray            *pValue) const;

   virtual void   importmsg(char const *pmsg, btWSSize len);

protected:
   NamedValueSet m_NVS;
};

////////////////////////////////////////////////////////////////////////////////
// IServiceClient

class EmptyIServiceClient : public AAL::IServiceClient,
                            public AAL::CAASBase
{
public:
   EmptyIServiceClient();
   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & )    {}
   virtual void serviceAllocateFailed(const IEvent & )           {}
   virtual void       serviceReleased(TransactionID const & )    {}
   virtual void serviceReleaseRequest(IBase *, const IEvent & )  {} 
   virtual void  serviceReleaseFailed(const IEvent & )           {}
   virtual void          serviceEvent(const IEvent & )           {}
};

class CallTrackingIServiceClient : public EmptyIServiceClient,
                                   public MethodCallLog
{
public:
   CallTrackingIServiceClient();
   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & );
   virtual void serviceAllocateFailed(const IEvent & );
   virtual void       serviceReleased(TransactionID const & );
   virtual void  serviceReleaseFailed(const IEvent & );
   virtual void          serviceEvent(const IEvent & );
};

class SynchronizingIServiceClient : public CallTrackingIServiceClient
{
public:
   SynchronizingIServiceClient();

   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & );
   virtual void serviceAllocateFailed(const IEvent & );
   virtual void       serviceReleased(TransactionID const & );
   virtual void  serviceReleaseFailed(const IEvent & );
   virtual void          serviceEvent(const IEvent & );

   btBool Wait(btTime        Timeout=AAL_INFINITE_WAIT);
   btBool Post(btUnsignedInt Count=1);

protected:
   Barrier m_Bar;
};

////////////////////////////////////////////////////////////////////////////////
// IRuntimeClient

class EmptyIRuntimeClient : public AAL::IRuntimeClient,
                            public AAL::CAASBase
{
public:
   EmptyIRuntimeClient();
   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & )        {}
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & ) {}
   virtual void                  runtimeStopped(IRuntime * )            {}
   virtual void              runtimeStartFailed(const IEvent & )        {}
   virtual void               runtimeStopFailed(const IEvent & )        {}
   virtual void    runtimeAllocateServiceFailed(IEvent const & )        {}
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & ) {}
   virtual void                    runtimeEvent(const IEvent & )        {}
};

class CallTrackingIRuntimeClient : public EmptyIRuntimeClient,
                                   public MethodCallLog
{
public:
   CallTrackingIRuntimeClient();
   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & );
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & );
   virtual void                  runtimeStopped(IRuntime * );
   virtual void              runtimeStartFailed(const IEvent & );
   virtual void               runtimeStopFailed(const IEvent & );
   virtual void    runtimeAllocateServiceFailed(IEvent const & );
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & );
   virtual void                    runtimeEvent(const IEvent & );
};

class SynchronizingIRuntimeClient : public CallTrackingIRuntimeClient
{
public:
   SynchronizingIRuntimeClient();

   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & );
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & );
   virtual void                  runtimeStopped(IRuntime * );
   virtual void              runtimeStartFailed(const IEvent & );
   virtual void               runtimeStopFailed(const IEvent & );
   virtual void    runtimeAllocateServiceFailed(IEvent const & );
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & );
   virtual void                    runtimeEvent(const IEvent & );

   btBool Wait(btTime        Timeout=AAL_INFINITE_WAIT);
   btBool Post(btUnsignedInt Count=1);

protected:
   Barrier m_Bar;
};

////////////////////////////////////////////////////////////////////////////////
// ISvcsFact

class EmptyISvcsFact : public AAL::ISvcsFact
{
public:
   EmptyISvcsFact();

   virtual IBase * CreateServiceObject(AALServiceModule * ,
                                       IRuntime         * );
   virtual void   DestroyServiceObject(IBase * ) { }
   virtual btBool    InitializeService(IBase               * ,
                                       IBase               * ,
                                       TransactionID const & ,
                                       NamedValueSet const & );
   DECLARE_RETVAL_ACCESSORS(CreateServiceObject, IBase *)
   DECLARE_RETVAL_ACCESSORS(InitializeService,   btBool)
protected:
   IBase  *m_CreateServiceObject_returns;
   btBool  m_InitializeService_returns;
};

class CallTrackingISvcsFact : public EmptyISvcsFact,
                              public MethodCallLog
{
public:
   CallTrackingISvcsFact() {}
   virtual IBase * CreateServiceObject(AALServiceModule * ,
                                       IRuntime         * );
   virtual void   DestroyServiceObject(IBase * );
   virtual btBool    InitializeService(IBase               * ,
                                       IBase               * ,
                                       TransactionID const & ,
                                       NamedValueSet const & );
};

////////////////////////////////////////////////////////////////////////////////
// IRuntime

class EmptyIRuntime : public AAL::IRuntime,
                      public AAL::CAASBase
{
public:
   EmptyIRuntime();

   virtual btBool                     start(const NamedValueSet & );
   virtual void                        stop() {}
   virtual void                allocService(IBase               * ,
                                            NamedValueSet const & ,
                                            TransactionID const & = TransactionID()) {}
   virtual btBool         schedDispatchable(IDispatchable * );
   virtual IRuntime *       getRuntimeProxy(IRuntimeClient * );
   virtual btBool       releaseRuntimeProxy();
   virtual IRuntimeClient *getRuntimeClient();
   virtual btBool                      IsOK();

   DECLARE_RETVAL_ACCESSORS(start,               btBool           )
   DECLARE_RETVAL_ACCESSORS(schedDispatchable,   btBool           )
   DECLARE_RETVAL_ACCESSORS(getRuntimeProxy,     IRuntime *       )
   DECLARE_RETVAL_ACCESSORS(releaseRuntimeProxy, btBool           )
   DECLARE_RETVAL_ACCESSORS(getRuntimeClient,    IRuntimeClient * )
   DECLARE_RETVAL_ACCESSORS(IsOK,                btBool           )

protected:
   btBool          m_start_returns;
   btBool          m_schedDispatchable_returns;
   IRuntime       *m_getRuntimeProxy_returns;
   btBool          m_releaseRuntimeProxy_returns;
   IRuntimeClient *m_getRuntimeClient_returns;
   btBool          m_IsOK_returns;
};

class CallTrackingIRuntime : public EmptyIRuntime,
                             public MethodCallLog
{
public:
   CallTrackingIRuntime();
   virtual btBool                     start(const NamedValueSet & );
   virtual void                        stop();
   virtual void                allocService(IBase                * ,
                                            NamedValueSet const &rManifest,
                                            TransactionID const &rTranID = TransactionID());
   virtual btBool         schedDispatchable(IDispatchable * );
   virtual IRuntime *       getRuntimeProxy(IRuntimeClient * );
   virtual btBool       releaseRuntimeProxy();
   virtual IRuntimeClient *getRuntimeClient();
   virtual btBool                      IsOK();
};

////////////////////////////////////////////////////////////////////////////////
// IServiceBase

class EmptyIServiceBase : public AAL::IServiceBase,
                          public AAL::CAASBase
{
public:
   EmptyIServiceBase();

   virtual btBool                     initComplete(TransactionID const &rtid);
   virtual btBool                       initFailed(IEvent const *ptheEvent);
   virtual btBool                  ReleaseComplete();
   virtual NamedValueSet const &           OptArgs() const;
   virtual IServiceClient *       getServiceClient() const;
   virtual IBase *            getServiceClientBase() const;
   virtual IRuntime *                   getRuntime() const;
   virtual IRuntimeClient *       getRuntimeClient() const;
   virtual AALServiceModule *  getAALServiceModule() const;

   DECLARE_RETVAL_ACCESSORS(initComplete,         btBool)
   DECLARE_RETVAL_ACCESSORS(initFailed,           btBool)
   DECLARE_RETVAL_ACCESSORS(ReleaseComplete,      btBool)
   DECLARE_RETVAL_ACCESSORS(OptArgs,              NamedValueSet const &)
   DECLARE_RETVAL_ACCESSORS(getServiceClient,     IServiceClient *)
   DECLARE_RETVAL_ACCESSORS(getServiceClientBase, IBase *)
   DECLARE_RETVAL_ACCESSORS(getRuntime,           IRuntime *)
   DECLARE_RETVAL_ACCESSORS(getRuntimeClient,     IRuntimeClient *)
   DECLARE_RETVAL_ACCESSORS(getAALServiceModule,  AALServiceModule *)

protected:
   btBool            m_initComplete_returns;
   btBool            m_initFailed_returns;
   btBool            m_ReleaseComplete_returns;
   IServiceClient   *m_getServiceClient_returns;
   IBase            *m_getServiceClientBase_returns;
   IRuntime         *m_getRuntime_returns;
   IRuntimeClient   *m_getRuntimeClient_returns;
   AALServiceModule *m_getAALServiceModule_returns;
   NamedValueSet     m_OptArgs_returns;
};

class CallTrackingIServiceBase : public EmptyIServiceBase,
                                 public MethodCallLog
{
public:
   CallTrackingIServiceBase();

   virtual btBool                     initComplete(TransactionID const &rtid);
   virtual btBool                       initFailed(IEvent const *ptheEvent);
   virtual btBool                  ReleaseComplete();
   virtual NamedValueSet const &           OptArgs() const;
   virtual IServiceClient *       getServiceClient() const;
   virtual IBase *            getServiceClientBase() const;
   virtual IRuntime *                   getRuntime() const;
   virtual IRuntimeClient *       getRuntimeClient() const;
   virtual AALServiceModule *  getAALServiceModule() const;
};

////////////////////////////////////////////////////////////////////////////////
// ServiceBase

class EmptyServiceBase : public AAL::ServiceBase
{
public:
   EmptyServiceBase(AALServiceModule *container,
                    IRuntime         *pAALRUNTIME,
                    IAALTransport    *ptransport,
                    IAALMarshaller   *marshaller,
                    IAALUnMarshaller *unmarshaller);

   virtual btBool init(IBase               * ,
                       NamedValueSet const & ,
                       TransactionID const & );

   DECLARE_RETVAL_ACCESSORS(init, btBool)

   void     ServiceClient(IServiceClient * );
   void ServiceClientBase(IBase * );
   void                RT(IRuntime * );

protected:
   btBool m_init_returns;
};

class CallTrackingServiceBase : public EmptyServiceBase,
                                public MethodCallLog
{
public:
   CallTrackingServiceBase(AALServiceModule *container,
                           IRuntime         *pAALRUNTIME,
                           IAALTransport    *ptransport,
                           IAALMarshaller   *marshaller,
                           IAALUnMarshaller *unmarshaller);

   virtual btBool  init(IBase               *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &rtid);
   virtual btBool _init(IBase               *pclientBase,
                        TransactionID const &rtid,
                        NamedValueSet const &optArgs,
                        CAALEvent           *pcmpltEvent=NULL);

   // <IAALService>
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);
   // </IAALService>

   // <IServiceBase>
   virtual btBool                    initComplete(TransactionID const &rtid);
   virtual btBool                      initFailed(IEvent const        *ptheEvent);
   virtual btBool                 ReleaseComplete();
   virtual NamedValueSet const &          OptArgs() const;
   virtual IServiceClient *      getServiceClient() const;
   virtual IBase *           getServiceClientBase() const;
   virtual IRuntime *                  getRuntime() const;
   virtual IRuntimeClient *      getRuntimeClient() const;
   virtual AALServiceModule * getAALServiceModule() const;
   // </IServiceBase>
};

////////////////////////////////////////////////////////////////////////////////
// IServiceModule / IServiceModuleCallback

class EmptyServiceModule : public AAL::IServiceModule,
                           public AAL::IServiceModuleCallback
{
public:
   EmptyServiceModule();

   // <IServiceModule>
   virtual btBool Construct(IRuntime            *pAALRUNTIME,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet());
   virtual void     Destroy();
   // </IServiceModule>

   // <IServiceModuleCallback>

   virtual void      ServiceReleased(IBase * );
   virtual btBool ServiceInitialized(IBase * , TransactionID const & );
   virtual btBool  ServiceInitFailed(IBase * , IEvent        const * );

   // </IServiceModuleCallback>

   DECLARE_RETVAL_ACCESSORS(Construct,          btBool)
   DECLARE_RETVAL_ACCESSORS(ServiceInitialized, btBool)
   DECLARE_RETVAL_ACCESSORS(ServiceInitFailed,  btBool)

protected:
   btBool m_Construct_returns;
   btBool m_ServiceInitialized_returns;
   btBool m_ServiceInitFailed_returns;
};

class CallTrackingServiceModule : public EmptyServiceModule,
                                  public MethodCallLog
{
public:
   CallTrackingServiceModule();

   // <IServiceModule>
   virtual btBool Construct(IRuntime            *pAALRUNTIME,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet());
   virtual void     Destroy();
   // </IServiceModule>

   // <IServiceModuleCallback>

   virtual void      ServiceReleased(IBase * );
   virtual btBool ServiceInitialized(IBase * , TransactionID const & );
   virtual btBool  ServiceInitFailed(IBase * , IEvent        const * );

   // </IServiceModuleCallback>
};

////////////////////////////////////////////////////////////////////////////////
// sw validation module / service module

void    AllocSwvalMod(AAL::IRuntime * ,
                      AAL::IBase    * ,
                      const AAL::TransactionID & );

void AllocSwvalSvcMod(AAL::IRuntime * ,
                      AAL::IBase    * ,
                      const AAL::TransactionID & );

class EmptySwvalSvcClient : public ISwvalSvcClient,
                            public EmptyIServiceClient
{
public:
   EmptySwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

class CallTrackingSwvalSvcClient : public ISwvalSvcClient,
                                   public CallTrackingIServiceClient
{
public:
   CallTrackingSwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

class SynchronizingSwvalSvcClient : public ISwvalSvcClient,
                                    public SynchronizingIServiceClient
{
public:
   SynchronizingSwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

#endif // __GTCOMMON_H__

