// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_GTEST_H__
#define __GTCOMMON_GTEST_H__

// Not quite sure why Google Test doesn't define these.
#define ASSERT_NONNULL(x) ASSERT_NE((void *)NULL, x)
#define ASSERT_NULL(x)    ASSERT_EQ((void *)NULL, x)
#define EXPECT_NONNULL(x) EXPECT_NE((void *)NULL, x)
#define EXPECT_NULL(x)    EXPECT_EQ((void *)NULL, x)

// Retrieve the current test case and test name from Google Test.
// Must be called within the context of a test case/fixture.
GTCOMMON_API void TestCaseName(std::string &TestCase, std::string &Test);

// A custom Google Test Environment that implements our application keep-alive timer.
// For example, main() would include the following code:
//
//   ::testing::AddGlobalTestEnvironment(KeepAliveTimerEnv::GetInstance());
//
class GTCOMMON_API KeepAliveTimerEnv : public ::testing::Environment
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

// A custom Google Test Event Listener that works in conjunction with our application keep-alive
// timer. This is the portion that pulses the keep-alive signal after each test is complete.
// For example, main() would include the following code:
//
//   ::testing::UnitTest::GetInstance()->listeners().Append(new KeepAliveTestListener());
//
class GTCOMMON_API KeepAliveTestListener : public ::testing::EmptyTestEventListener
{
public:
   virtual void OnTestEnd(const ::testing::TestInfo & /*test_info*/)
   {
      KeepAliveTimerEnv::GetInstance()->KeepAlive();
   }
};

#endif // __GTCOMMON_GTEST_H__

