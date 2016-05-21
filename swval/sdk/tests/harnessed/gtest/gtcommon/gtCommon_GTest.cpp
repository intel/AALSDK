// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

// Retrieve the current test and test case name from gtest.
// Must be called within the context of a test case/fixture.
GTCOMMON_API void TestCaseName(std::string &TestCase, std::string &Test)
{
   const ::testing::TestInfo * const pInfo =
      ::testing::UnitTest::GetInstance()->current_test_info();

   TestCase = std::string(pInfo->test_case_name());
   Test     = std::string(pInfo->name());
}


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

