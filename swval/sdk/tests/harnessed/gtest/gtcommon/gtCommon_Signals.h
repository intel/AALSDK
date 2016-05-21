// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_SIGNALS_H__
#define __GTCOMMON_SIGNALS_H__

// Unlike Windows, signals are delivered to specific threads in Linux. For some of the test
// cases, we need to know that certain signals and how many of them have been delivered to a
// specific thread.
// The idea is that each thread participating in signal passing will register their tid in a
// common location. This registration thereafter allows a tid to be converted to a zero-based
// index.
// After all such threads are registered, signal passing may begin. When a signal handler is
// entered, the thread uses its tid to rediscover its unique index. The index is then used to
// update a count of the number of that signal type that has been received for the thread.
// In this way, examiner threads may query the number of signals received by other threads, and
// all of this is done in a thread-safe way.
//
// The ThreadRegistry is the piece of this mechanism that allows converting tid's to zero-based
// indexes and that provides synchronization around the registration process.
class GTCOMMON_API ThreadRegistry : public CriticalSection
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

// SignalHelper provides a means to..
//   * install/uninstall pre-baked signal handlers for a small set of signal types.
//   * raise signals (the tid parameter is ignored in Windows)
//   * query a count of the signals of a particular type received by a thread (by index).
class GTCOMMON_API SignalHelper : public ThreadRegistry
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

#endif // __GTCOMMON_SIGNALS_H__

