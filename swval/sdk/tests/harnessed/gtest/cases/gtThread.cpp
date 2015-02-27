// INTEL CONFIDENTIAL - For Intel Internal Use Only

TEST(OSAL_Thread, aal0000) {
   // GetProcessID() returns the pid of the current process.

   btPID pid = (btPID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentProcessId();
#elif defined( __AAL_LINUX__ )
   getpid();
#endif

   ASSERT_EQ(pid, GetProcessID());
}

TEST(OSAL_Thread, aal0001) {
   // GetThreadID() returns the tid of the current thread.

   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   ASSERT_EQ(tid, GetThreadID());
}



TEST(OSAL_Thread, aal0003) {
   // GetRand() returns a pseudo-random 32-bit integer.

/*
In general, this is a very difficult problem. We don't want to dive into the domain of
verifying the underlying implementation of the RNG. We just want a basic sanity check to
ensure that we're calling the RNG correctly.
*/
   const int          count = 50;
   btUnsigned32bitInt randoms[count];
   btUnsigned32bitInt seed = 1234;

   // We should have no repeated values in count calls to the RNG.
   int i;
   int j;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);

      for ( j = 0 ; j < i ; ++j ) {
         EXPECT_NE(r, randoms[j]);
      }

      randoms[i] = r;
   }
}


TEST(OSAL_Thread, aal0005) {
   // The GetRand() sequence is repeatable for a given input seed value.

/*
Another difficult problem to solve completely. Looking for a basic sanity check here.
*/

   const int          count = 50;
   btUnsigned32bitInt randoms[count];
   btUnsigned32bitInt seed = 1234;
   btUnsigned32bitInt save_seed = seed;

   int i;
   int j;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);

      for ( j = 0 ; j < i ; ++j ) {
         EXPECT_NE(r, randoms[j]);
      }

      randoms[i] = r;
   }

   seed = save_seed;
   for ( i = 0 ; i < count ; ++i ) {
      btUnsigned32bitInt r = GetRand(&seed);
      EXPECT_EQ(r, randoms[i]);
   }
}

TEST(OSAL_Thread, aal0006) {
   // CurrentThreadID::getTID() returns the tid of the current thread.
   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   CurrentThreadID thrid;
   ASSERT_EQ(tid, thrid.getTID());
}

TEST(OSAL_Thread, aal0007) {
   // CurrentThreadID::operator btTID () returns the tid of the current thread.
   btTID tid = (btTID)

#if   defined( __AAL_WINDOWS__ )
   GetCurrentThreadId();
#elif defined( __AAL_LINUX__ )
   pthread_self();
#endif

   CurrentThreadID thrid;
   ASSERT_EQ(tid, (AAL::btTID)thrid);
   ASSERT_EQ(tid, thrid.operator AAL::btTID());
}


// Value-parameterized test fixture
class OSAL_Thread_vp_bool : public ::testing::TestWithParam< AAL::btBool >
{
protected:
   OSAL_Thread_vp_bool() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i]   = NULL;
         m_Scratch[i] = 0;
      }
   }
   // virtual void TearDown() { }

   OSLThread *m_pThrs[3];
   btUIntPtr  m_Scratch[3];

   static void Thr0(OSLThread *pThread, void *pContext);

};

void OSAL_Thread_vp_bool::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_vp_bool *pTC = static_cast<OSAL_Thread_vp_bool *>(pContext);
   ASSERT_NONNULL(pTC);

   EXPECT_NONNULL(pThread);
   EXPECT_EQ(pThread, pTC->m_pThrs[0]);

   ++(pTC->m_Scratch[0]);
}

TEST_P(OSAL_Thread_vp_bool, aal0009)
{
   // OSLThread::OSLThread() when given an invalid nPriority.
   AAL::btBool ThisThread = GetParam();

   OSLThread::ThreadPriority invalid;

   invalid = (OSLThread::ThreadPriority) (OSLThread::THREADPRIORITY_COUNT + 100);

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr0,
                              invalid,
                              this,
                              ThisThread);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[0]);

   delete m_pThrs[0];


   invalid = OSLThread::THREADPRIORITY_INVALID;

   m_pThrs[0] = new OSLThread(OSAL_Thread_vp_bool::Thr0,
                              invalid,
                              this,
                              ThisThread);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(2, m_Scratch[0]);

   delete m_pThrs[0];
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Thread_vp_bool,
                        ::testing::Bool());


// Simple test fixture
class OSAL_Thread_f : public ::testing::Test
{
protected:
   OSAL_Thread_f() {}

   virtual void SetUp()
   {

   }

   OSLThread *m_pThrs[3];
   btUIntPtr  m_Scratch[3];

   static void Thr0(OSLThread *pThread, void *pContext);
};

void OSAL_Thread_f::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Thread_f *pTC = static_cast<OSAL_Thread_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = GetThreadID();
}

TEST_F(OSAL_Thread_f, aal0010)
{
   // OSLThread - verify that pProc is executed in the current thread when ThisThread == true.

   m_pThrs[0] = new OSLThread(OSAL_Thread_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              true);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   m_pThrs[0]->Join();
   EXPECT_EQ(m_Scratch[0], GetThreadID());

   delete m_pThrs[0];
}



