// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "dbg_csemaphore.h"

TEST(OSAL_Sem, aal0031)
{
   // CSemaphore::~CSemaphore() should behave robustly when no call to CSemaphore::Create() was made.
   CSemaphore *pSem = new CSemaphore();
   delete pSem;
}

TEST(OSAL_Sem, aal0032)
{
   // CSemaphore::Destroy() should behave robustly when no call to CSemaphore::Create() was made.
   CSemaphore *pSem = new CSemaphore();
   EXPECT_FALSE(pSem->Destroy());
   delete pSem;
}

TEST(OSAL_Sem, aal0036)
{
   // CSemaphore::Reset(nCount) (created=true, waiters=0, nCount<=Max) is successful.

   CSemaphore *pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(-1, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(0, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(1, 0));
   EXPECT_TRUE(pSem->Reset(1));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;


   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(-1, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(0, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;

   pSem = new CSemaphore();
   EXPECT_TRUE(pSem->Create(1, 2));
   EXPECT_TRUE(pSem->Reset(2));
   EXPECT_TRUE(pSem->Reset(-1));
   EXPECT_TRUE(pSem->Reset(0));
   delete pSem;
}

#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_0 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_Sem_vp_tuple_0() {}
   virtual ~OSAL_Sem_vp_tuple_0() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      m_Sem.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread  *m_pThrs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Sem;

   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_0 *pTC = static_cast<OSAL_Sem_vp_tuple_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;
   AAL::btUnsignedInt       u;

   // we should be able to consume nMaxCount's from the sem without blocking.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_0, aal0049)
{
   // Calling CSemaphore::Create() with nInitialCount > nMaxCount and nMaxCount > 0
   //  results in an initial count of nMaxCount.

   std::tr1::tie(m_nInitialCount, m_nMaxCount) = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);
   EXPECT_EQ(i, m);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[1]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_0,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)5,
                                                             (AAL::btInt)10,
                                                             (AAL::btInt)100
                                                            ),
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)2,
                                                             (AAL::btUnsignedInt)3
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE


#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_1 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btUnsignedInt > >
{
protected:
   OSAL_Sem_vp_tuple_1() {}
   virtual ~OSAL_Sem_vp_tuple_1() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      m_Sem.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread  *m_pThrs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Sem;

   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_1::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_1 *pTC = static_cast<OSAL_Sem_vp_tuple_1 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;
   AAL::btUnsignedInt       u;

   pTC->m_Scratch[0] = 1;   // signal that we're ready.

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // We should be able to make nMaxCount Posts()'s.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Post(1));
   }

   // Attempting to Post() again should fail.
   EXPECT_FALSE(sem.Post(1));
   pTC->m_Scratch[2] = 1;

   // Wait for the other thread to reset the sem.
   YIELD_WHILE(0 == pTC->m_Scratch[3]);
   pTC->m_Scratch[4] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.

   // We should be able to make nMaxCount Posts()'s.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Post(1));
   }

   // Attempting to Post() again should fail.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[5] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_1, aal0054)
{
   // Calling CSemaphore::Create() with nInitialCount < 0 and
   //  nMaxCount > 0 creates a count up semaphore.

   std::tr1::tie(m_nInitialCount, m_nMaxCount) = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   const AAL::btInt N = -m_nInitialCount;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_1::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // We should be required to Post() abs(m_nInitialCount) times before unblocking A.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Reset the semaphore and repeat.
   ASSERT_TRUE(m_Sem.Reset(m_nInitialCount)) << "m_nInitialCount = " << m_nInitialCount << " m_nMaxCount = " << m_nMaxCount;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_Scratch[3] = 1;

   YIELD_WHILE(0 == m_Scratch[4]);

   // We should be required to Post() abs(m_nInitialCount) times before unblocking A.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   // Give A plenty of opportunity to have set scratch[5]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[5]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_1,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)-1,
                                                             (AAL::btInt)-5,
                                                             (AAL::btInt)-10,
                                                             (AAL::btInt)-25
                                                            ),
                                           ::testing::Values((AAL::btUnsignedInt)1,
                                                             (AAL::btUnsignedInt)5,
                                                             (AAL::btUnsignedInt)10,
                                                             (AAL::btUnsignedInt)25
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE


#if GTEST_HAS_TR1_TUPLE

// Value-parameterized test fixture (tuple)
class OSAL_Sem_vp_tuple_2 : public ::testing::TestWithParam< std::tr1::tuple< AAL::btInt, AAL::btInt > >
{
protected:
   OSAL_Sem_vp_tuple_2() {}
   virtual ~OSAL_Sem_vp_tuple_2() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      m_Sem.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread  *m_pThrs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Sem;

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_2::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_vp_tuple_2::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_vp_tuple_2::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_2 *pTC = static_cast<OSAL_Sem_vp_tuple_2 *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   ASSERT_TRUE(pTC->m_Sem.Wait()); // (count <= 0) should block.
   pTC->m_Scratch[5] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_2, aal0059)
{
   // For a created sem, CSemaphore::Post() allows at least one blocking thread
   //  to wake when the semaphore count becomes greater than 0.

   AAL::btInt NumToPost = 0;

   std::tr1::tie(m_nInitialCount, NumToPost) = GetParam();

   m_nMaxCount = 10000;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_vp_tuple_2::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   AAL::btInt i;
   AAL::btInt N = m_nInitialCount;
   if ( N < 0 ) {
      N = -N;
   }

   // We need to Post() the sem N times before any waiters will unblock.
   for ( i = 0 ; i < N - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3] + m_Scratch[4] + m_Scratch[5]));

   // Now, this should wake at least one thread.
   EXPECT_TRUE(m_Sem.Post(NumToPost));

   YIELD_WHILE(0 == m_Scratch[3] + m_Scratch[4] + m_Scratch[5]);

   // Give all child threads an opportunity to wake before we continue.
   YIELD_N();

   AAL::btInt woke = 0;

   // Join() the threads that woke.
   for ( i = 0 ; i < 3 ; ++i ) {
      if ( 1 == m_Scratch[3 + i] ) {
         m_pThrs[i]->Join();
         ++m_Scratch[3 + i]; // increment to 2.
         ++woke;
      }
   }

   // The number of waiters that woke must be <= the number posted to the sem.
   ASSERT_LE(woke, NumToPost);

   while ( woke < 3 ) {

      EXPECT_TRUE(m_Sem.Post(NumToPost));

      // Give all child threads an opportunity to wake before we continue.
      YIELD_N();

      for ( i = 0 ; i < 3 ; ++i ) {
         if ( 1 == m_Scratch[3 + i] ) {
            m_pThrs[i]->Join();
            ++m_Scratch[3 + i]; // increment to 2.
            ++woke;
         }
      }

   }
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
// ::testing::Combine(generator1, generator2, generator3) [requires tr1/tuple]
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_tuple_2,
                        ::testing::Combine(
                                           ::testing::Values(
                                                             (AAL::btInt)-1,
                                                             (AAL::btInt)0
                                                            ),
                                           ::testing::Values((AAL::btInt)1,
                                                             (AAL::btInt)2,
                                                             (AAL::btInt)3
                                                            )
                                          ));

#endif // GTEST_HAS_TR1_TUPLE



// Value-parameterized test fixture
template <typename T>
class OSAL_Sem_vp : public ::testing::TestWithParam<T>
{
public:
   OSAL_Sem_vp() {}
   virtual ~OSAL_Sem_vp() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      m_Sem.UserDefined(NULL);

      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      m_Sem.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread  *m_pThrs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Sem;
};

class OSAL_Sem_vp_int_0 : public OSAL_Sem_vp< AAL::btInt >
{
public:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_int_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_int_0 *pTC = static_cast<OSAL_Sem_vp_int_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;
   AAL::btInt               i;

   // we should be able to consume nInitialCount's from the sem without blocking.
   for ( i = 0 ; i < nInitialCount ; ++i ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // Adjust the count to max.
   ASSERT_TRUE(sem.Post(nInitialCount));

   // Trying to Post() any more should fail.
   ASSERT_FALSE(sem.Post(1));

   // we should be able to consume nInitialCount's from the sem without blocking.
   for ( i = 0 ; i < nInitialCount ; ++i ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[2] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_P(OSAL_Sem_vp_int_0, aal0050)
{
   // Calling CSemaphore::Create() with nInitialCount > 0 and nMaxCount = 0 results in nMaxCount = nInitialCount.

   m_nInitialCount = GetParam();
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m_nInitialCount, i);
   EXPECT_EQ(m, i);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_int_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set scratch[3]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_int_0, ::testing::Values(1, 10, 100, 250));


class OSAL_Sem_vp_int_1 : public OSAL_Sem_vp< AAL::btInt >
{
public:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_int_1::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_int_1 *pTC = static_cast<OSAL_Sem_vp_int_1 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

//   pTC->m_Scratch[0] = 1;

   EXPECT_TRUE(sem.Wait());
   pTC->m_Scratch[1] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[2] = 1;

   // Reset and repeat the test.
   EXPECT_TRUE(sem.Reset(nInitialCount));

   EXPECT_TRUE(sem.Wait());
   pTC->m_Scratch[3] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[4] = 1;
}

TEST_P(OSAL_Sem_vp_int_1, aal0052)
{
   // Calling CSemaphore::Create() with nInitialCount < 0 and nMaxCount = 0 results in
   //  nInitialCount = nInitialCount + 1 and nMaxCount = 1.

   class aal0052AfterCSemaphoreAutoLock : public AAL::Testing::EmptyAfterCSemaphoreAutoLock
   {
   public:
      aal0052AfterCSemaphoreAutoLock(OSAL_Sem_vp_int_1 *pTC) :
         m_pTC(pTC),
         m_Count(0)
      {}

      virtual void OnWait()
      {
         if ( 0 == m_Count ) {
            // Thr0
            m_pTC->m_Scratch[0] = 1;
         }
         ++m_Count;
      }

   protected:
      OSAL_Sem_vp_int_1 *m_pTC;
      btInt              m_Count;
   } AfterAutoLock(this);


   m_nInitialCount = GetParam();
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   m_Sem.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(i, m_nInitialCount + 1);
   EXPECT_EQ(1, m);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_int_1::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of time to have set m_Scratch[1].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // We should be able to Post() the semaphore abs(m_nInitialCount) times before waking A.
   m = -m_nInitialCount;
   for ( i = 0 ; i < m - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1)); // This should wake A.

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of time to have set m_Scratch[3].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // We should be able to Post() the semaphore abs(m_nInitialCount) times before waking A.
   m = -m_nInitialCount;
   for ( i = 0 ; i < m - 1 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
   }
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   EXPECT_TRUE(m_Sem.Post(1)); // This should wake A.

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[4]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_int_1, ::testing::Values(-1, -10, -100, -250));


class OSAL_Sem_vp_uint_0 : public OSAL_Sem_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_uint_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_uint_0 *pTC = static_cast<OSAL_Sem_vp_uint_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;
   AAL::btUnsignedInt       u;

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   // Adjust the count to max.
   ASSERT_TRUE(sem.Post((AAL::btInt)nMaxCount));

   // Trying to Post() any more should fail.
   ASSERT_FALSE(sem.Post(1));

   // we should now be able to consume nMaxCount's from the sem without blocking.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[2] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_P(OSAL_Sem_vp_uint_0, aal0053)
{
   // Calling CSemaphore::Create() with nInitialCount = 0 and nMaxCount > 0 creates
   //  a traditional count down semaphore that is locked.

   m_nInitialCount = 0;
   m_nMaxCount     = GetParam();

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(i, m));
   EXPECT_EQ(m_nInitialCount, i);
   EXPECT_EQ(m, (AAL::btInt)m_nMaxCount);

   m_pThrs[0] = new OSLThread(OSAL_Sem_vp_uint_0::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set scratch[1]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set scratch[3]
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   // This will allow A to grab a count and unblock.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_uint_0, ::testing::Values(1, 10, 100, 250));


// Simple test fixture
class OSAL_Sem_f : public ::testing::Test
{
public:
   OSAL_Sem_f() {}
   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      m_Sem.UserDefined(NULL);

      YIELD_WHILE(CurrentThreads() > 0);

      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }

      m_Sem.Destroy();
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread  *m_pThrs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Sem;

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );
   static void Thr2(OSLThread * , void * );

   static void Thr3(OSLThread * , void * );
   static void Thr4(OSLThread * , void * );
   static void Thr5(OSLThread * , void * );

   static void Thr6(OSLThread * , void * );
   static void Thr7(OSLThread * , void * );
   static void Thr8(OSLThread * , void * );

   static void Thr9(OSLThread * , void * );
   static void Thr10(OSLThread * , void * );
   static void Thr11(OSLThread * , void * );

   static void Thr12(OSLThread * , void * );

   static void Thr13(OSLThread * , void * );

   static void Thr14(OSLThread * , void * );
   static void Thr15(OSLThread * , void * );
   static void Thr16(OSLThread * , void * );
};

void OSAL_Sem_f::Thr2(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   AAL::btInt i = 0;
   AAL::btInt m = 0;

   CSemaphore &sem = pTC->m_Sem;

   EXPECT_TRUE(sem.CurrCounts(i, m));
   ASSERT_EQ(0, i);
   ASSERT_EQ(1, m);

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait());  // (0 == count) should block.
   pTC->m_Scratch[1] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   EXPECT_TRUE(sem.Reset(0));

   pTC->m_Scratch[2] = 1;
   ASSERT_TRUE(sem.Wait());  // (0 == count) should block.

   pTC->m_Scratch[3] = 1;

   EXPECT_TRUE(sem.Post(1)); // (1 == count) at max.
   EXPECT_FALSE(sem.Post(1));

   pTC->m_Scratch[4] = 1;
}

TEST_F(OSAL_Sem_f, aal0051)
{
   // Calling CSemaphore::Create() with nInitialCount = 0 and nMaxCount = 0 results in nInitialCount = 0 and nMaxCount = 1.

   m_nInitialCount = 0;
   m_nMaxCount     = 0;

   ASSERT_TRUE(m_Sem.Create(m_nInitialCount, m_nMaxCount));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr2,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give A plenty of opportunity to have set m_Scratch[1].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[1]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give A plenty of opportunity to have set m_Scratch[3].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));

   EXPECT_TRUE(m_Sem.Post(1));

   YIELD_WHILE(0 == m_Scratch[3]);

   m_pThrs[0]->Join();

   EXPECT_EQ(1, m_Scratch[4]);
}

TEST_F(OSAL_Sem_f, aal0055)
{
   // CSemaphore::Reset(nCount) (created=false, waiters=X, nCount<=X) fails.

   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));

   EXPECT_TRUE(m_Sem.Create(1, 1));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
}

TEST_F(OSAL_Sem_f, aal0056)
{
   // CSemaphore::Reset(nCount) (created=true, waiters=0, nCount>Max) fails.

   EXPECT_TRUE(m_Sem.Create(-1, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(0, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(1, 0));
   EXPECT_TRUE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(-1, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(0, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());

   EXPECT_TRUE(m_Sem.Create(1, 1));
   EXPECT_FALSE(m_Sem.Reset(2));
   EXPECT_TRUE(m_Sem.Destroy());
}

void OSAL_Sem_f::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   CSemaphore &sem = pTC->m_Sem;

   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[2] = 1;
}

void OSAL_Sem_f::Thr1(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   CSemaphore &sem = pTC->m_Sem;

   pTC->m_Scratch[1] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[3] = 1;
}

TEST_F(OSAL_Sem_f, aal0057)
{
   // CSemaphore::Reset(nCount) (created=true, waiters>0, nCount<=Max) fails.

   ASSERT_TRUE(m_Sem.Create(0, 2));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr1,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);

   // Give the child threads plenty of time to have blocked on the sem.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(0, m_Scratch[2]);


   // Try to Reset() the sem. This should fail.
   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));

   // Give the child threads plenty of time to have woken.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(0, m_Scratch[2]);


   // Post() the sem once, allowing one of the child threads to wake and exit.
   EXPECT_TRUE(m_Sem.Post(1));

   // Wait until the child exits.
   YIELD_WHILE(0 == m_Scratch[2] && 0 == m_Scratch[3]);

   // Determine which thread exited.
   AAL::btInt t = (0 == m_Scratch[2]) ? 1 : 0;

   // Thread t exited.
   m_pThrs[t]->Join();
   t = 1 - t;


   // Repeat the tests with the one remaining child thread.

   // Try to Reset() the sem. This should fail.
   EXPECT_FALSE(m_Sem.Reset(-1));
   EXPECT_FALSE(m_Sem.Reset(0));
   EXPECT_FALSE(m_Sem.Reset(1));
   EXPECT_FALSE(m_Sem.Reset(2));

   // Give the child thread plenty of time to have woken.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[t + 2]));

   // Post() the sem once, allowing the last child thread to wake and exit.
   EXPECT_TRUE(m_Sem.Post(1));

   m_pThrs[t]->Join();
   EXPECT_EQ(1, m_Scratch[t + 2]);
}

TEST_F(OSAL_Sem_f, aal0038)
{
   // CSemaphore::CurrCounts() retrieves the values of the current and max counters,
   //  when the sem has been created.

   AAL::btInt         i;
   AAL::btUnsignedInt m;

   AAL::btInt         j;
   AAL::btInt         n;

   i = -1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 0; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 1; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 2; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 2; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 2; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(2, n);
   EXPECT_TRUE(m_Sem.Destroy());
}

TEST_F(OSAL_Sem_f, aal0039)
{
   // CSemaphore::CurrCounts() returns false when sem not initialized.
   AAL::btInt cur;
   AAL::btInt max;
   EXPECT_FALSE(m_Sem.CurrCounts(cur, max));
}

TEST_F(OSAL_Sem_f, aal0058)
{
   // CSemaphore::Post() returns false when sem not initialized.
   EXPECT_FALSE(m_Sem.Post(1));
}

TEST_F(OSAL_Sem_f, aal0065)
{
   // CSemaphore::Post() does not affect the current count and
   // returns false when (current + postval) > maxcount.

   AAL::btInt         i;
   AAL::btUnsignedInt m;

   AAL::btInt         j;
   AAL::btInt         n;

   i = -1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 0; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 0; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());


   i = -1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 0; m = 1; j = 1; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(0, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());

   i = 1; m = 1; j = 0; n = 0;

   EXPECT_TRUE(m_Sem.Create(i, m));
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_FALSE(m_Sem.Post(2));
   j = n = -1;
   EXPECT_TRUE(m_Sem.CurrCounts(j, n));
   EXPECT_EQ(1, j);
   EXPECT_EQ(1, n);
   EXPECT_TRUE(m_Sem.Destroy());
}

TEST_F(OSAL_Sem_f, aal0061)
{
   // CSemaphore::UnblockAll() returns false when sem not initialized.
   EXPECT_FALSE(m_Sem.UnblockAll());
}

void OSAL_Sem_f::Thr3(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   //pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait()); // (0 == count) this will block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr4(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   //pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT)); // (0 == count) this will block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr5(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   //pTC->m_Scratch[2] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1))); // (0 == count) this will block.
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0062)
{
   // When the sem has been initialized, CSemaphore::UnblockAll() causes all
   // threads currently blocked in Wait() calls for the sem to wake, returning
   // false from CSemaphore::Wait(). The CSemaphore::UnblockAll() call itself returns true.

   class aal0062AfterCSemaphoreAutoLock : public AAL::Testing::EmptyAfterCSemaphoreAutoLock
   {
   public:
      aal0062AfterCSemaphoreAutoLock(OSAL_Sem_f *pTC) :
         m_pTC(pTC),
         m_Count(0)
      {}

      virtual void OnWait()
      {
         if ( 0 == m_Count ) {
            // Thr3
            m_pTC->m_Scratch[0] = 1;
         } else if (1 == m_Count) {
            // Thr4
            m_pTC->m_Scratch[1] = 1;
         }
         ++m_Count;
      }

      virtual void OnWait(btTime t)
      {
         // Thr5
         m_pTC->m_Scratch[2] = 1;
      }

   protected:
      OSAL_Sem_f *m_pTC;
      btInt       m_Count;
   } AfterAutoLock(this);

   ASSERT_TRUE(m_Sem.Create(0, 1));

   m_Sem.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr3,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr4,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());
   YIELD_WHILE(0 == m_Scratch[1]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr5,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());
   YIELD_WHILE(0 == m_Scratch[2]);

   //YIELD_X(10);

   EXPECT_TRUE(m_Sem.UnblockAll());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

void OSAL_Sem_f::Thr6(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait()); // (0 == count) this will block.
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr7(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT)); // (0 == count) this will block.
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr8(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[2] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1))); // (0 == count) this will block.
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0066)
{
   // Once all of the threads that were blocked on a CSemaphore::Wait() call have woken
   // and returned false from Wait(), the next call to CSemaphore::Wait() blocks,
   // because the CSemaphore is no longer unblocking. When the CSemaphore is next Post()'ed,
   // the Wait() call returns true.

   class aal0066AfterCSemaphoreAutoLock : public AAL::Testing::EmptyAfterCSemaphoreAutoLock
   {
   public:
      aal0066AfterCSemaphoreAutoLock(OSAL_Sem_f *pTC) :
         m_pTC(pTC),
         m_Count(0)
      {}

      virtual void OnWait()
      {
         if (0 == m_Count) {
            // Thr6
            m_pTC->m_Scratch[0] = 1;
         }
         else if (1 == m_Count) {
            // Thr7
            m_pTC->m_Scratch[1] = 1;
         }
         ++m_Count;
      }

      virtual void OnWait(btTime t)
      {
         // Thr8
         m_pTC->m_Scratch[2] = 1;
      }

   protected:
      OSAL_Sem_f *m_pTC;
      btInt       m_Count;
   } AfterAutoLock(this);

   ASSERT_TRUE(m_Sem.Create(0, 1));

   m_Sem.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr6,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr7,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);

   //YIELD_X(25);

   EXPECT_TRUE(m_Sem.UnblockAll());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr8,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give the thread plenty of opportunity to have set m_Scratch[5].
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Post(1)); // will wake the thread, allowing it to exit.

   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[5]);
}

void OSAL_Sem_f::Thr9(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait());
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr10(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[1] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(AAL_INFINITE_WAIT));
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr11(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[2] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait(HOURS_IN_TERMS_OF_MILLIS(1)));
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0063)
{
   // CSemaphore::NumWaiters() returns the number of threads currently blocked in CSemaphore::Wait() calls.

   ASSERT_TRUE(m_Sem.Create(0, 3));
   EXPECT_EQ(0, m_Sem.NumWaiters());

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr9,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3]));
   EXPECT_EQ(1, m_Sem.NumWaiters());


   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr10,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());

   YIELD_WHILE(0 == m_Scratch[1]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[4]));
   EXPECT_EQ(2, m_Sem.NumWaiters());


   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr11,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());

   YIELD_WHILE(0 == m_Scratch[2]);

   // Give the thread plenty of time to have blocked.
   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[5]));
   EXPECT_EQ(3, m_Sem.NumWaiters());

   AAL::btInt i;
   AAL::btInt j;

   m_Scratch[0] = 0;
   m_Scratch[1] = 0;
   m_Scratch[2] = 0;

   for ( i = 0 ; i < 3 ; ++i ) {
      EXPECT_TRUE(m_Sem.Post(1));
      YIELD_WHILE(i == m_Scratch[3] + m_Scratch[4] + m_Scratch[5]);

      EXPECT_EQ(3 - (i + 1), m_Sem.NumWaiters()) << " i=" << i;

      for ( j = 0 ; j < 3 ; ++j ) {
         if ( ( 0 == m_Scratch[j] ) && ( 1 == m_Scratch[3 + j] ) ) {
            m_Scratch[j] = 1;
            m_pThrs[j]->Join();
         }
      }
   }

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

TEST_F(OSAL_Sem_f, aal0064)
{
   // CSemaphore::Wait(btTime ) returns false when the sem is not initialized.
   EXPECT_FALSE(m_Sem.Wait(1000));
}

TEST_F(OSAL_Sem_f, aal0067)
{
   // When the sem is initialized and the time duration given to CSemaphore::Wait(btTime )
   // expires, Wait() returns false.

   EXPECT_TRUE(m_Sem.Create(0, 1));
   EXPECT_FALSE(m_Sem.Wait(1));
}

#ifdef __AAL_LINUX__
// Linux-only. The only candidates for user-defined signals in Windows are SIGILL and SIGTERM, per:
// https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
//
// Both SIGILL and SIGTERM cause the process to terminate, as determined by running this test case in Windows.

void OSAL_Sem_f::Thr12(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   SignalHelper &sig = SignalHelper::GetInstance();

   btUnsignedInt idx = sig.ThreadRegister(GetThreadID());

   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait(pTC->m_Scratch[2]));

   pTC->m_Scratch[1] = 1;

#if   defined( __AAL_LINUX__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGIO, idx) < pTC->m_Scratch[3]);
#elif defined( __AAL_WINDOWS__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) < pTC->m_Scratch[3]);
#endif // OS
}

TEST_F(OSAL_Sem_f, aal0068)
{
   // When the sem is initialized and a thread blocks in CSemaphore::Wait(btTime X),
   // given that no call to CSemaphore::Post() nor CSemaphore::UnblockAll() is made by
   // another thread, the blocked thread waits at least X milliseconds before resuming,
   // even in the presence of signals.

   SignalHelper &sig = SignalHelper::GetInstance();

   sig.RegistryReset();
   sig.ThreadRegister(GetThreadID());

   m_Scratch[2] = 250; // timeout (millis) for the Wait() call in m_Scratch[2].
   m_Scratch[3] = 20;

   btTime       slept     = 0;
   const btTime sleepeach = 5;
   const btTime thresh    = m_Scratch[2] - ( m_Scratch[2] / 20 ); // within 5%

   EXPECT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr12,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   btUnsignedInt idx = sig.ThreadLookup(m_pThrs[0]->tid());

   int    k;
   btBool Done = false;

   while ( !Done ) {
      sleep_millis(sleepeach);
      slept += sleepeach;

      if ( slept < thresh ) {
         // we still expect the thread to be blocked.
         EXPECT_EQ(0, m_Scratch[1]);
      }

#if   defined( __AAL_LINUX__ )
      Done = sig.GetCount(SignalHelper::IDX_SIGIO, idx) >= m_Scratch[3];
#elif defined( __AAL_WINDOWS__ )
      Done = sig.GetCount(SignalHelper::IDX_SIGUSR1, idx) >= m_Scratch[3];
#endif // OS

      if ( !Done ) {

#if   defined( __AAL_LINUX__ )

         k = sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[0]->tid());
         Done = (ESRCH == k);

#elif defined( __AAL_WINDOWS__ )

         k = sig.Raise(SignalHelper::IDX_SIGUSR1, m_pThrs[0]->tid());
         Done = (0 != k);

#endif // OS

      }

   }

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[1]);

   sig.RegistryReset();
}
#endif // __AAL_LINUX__

TEST_F(OSAL_Sem_f, aal0069)
{
   // CSemaphore::Wait(void) returns false when the sem is not initialized.
   EXPECT_FALSE(m_Sem.Wait());
}

#ifdef __AAL_LINUX__
// Linux-only. The only candidates for user-defined signals in Windows are SIGILL and SIGTERM, per:
// https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
//
// Both SIGILL and SIGTERM cause the process to terminate, as determined by running this test case in Windows.

void OSAL_Sem_f::Thr13(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

   SignalHelper &sig = SignalHelper::GetInstance();

   btUnsignedInt idx = sig.ThreadRegister(GetThreadID());

   pTC->m_Scratch[0] = 1;
   EXPECT_TRUE(pTC->m_Sem.Wait());

   pTC->m_Scratch[1] = 1;

#if   defined( __AAL_WINDOWS__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGUSR2, idx) < pTC->m_Scratch[3]);
#elif defined( __AAL_LINUX__ )
   YIELD_WHILE(sig.GetCount(SignalHelper::IDX_SIGIO, idx) < pTC->m_Scratch[3]);
#endif // __AAL_LINUX__
}

TEST_F(OSAL_Sem_f, aal0070)
{
   // When the sem is initialized and a thread blocks in CSemaphore::Wait(void),
   // given that no call to CSemaphore::Post() nor CSemaphore::UnblockAll() is made
   // by another thread, the blocked thread waits infinitely, even in the presence of signals.

   SignalHelper &sig = SignalHelper::GetInstance();

   sig.RegistryReset();
   sig.ThreadRegister(GetThreadID());

   m_Scratch[3] = 2;

   EXPECT_TRUE(m_Sem.Create(0, 1));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr13,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   YIELD_WHILE(0 == m_Scratch[0]);

   btUnsignedInt idx = sig.ThreadLookup(m_pThrs[0]->tid());

#if   defined( __AAL_WINDOWS__ )
   EXPECT_EQ(0, sig.Raise(SignalHelper::IDX_SIGUSR2, m_pThrs[0]->tid()));
#elif defined( __AAL_LINUX__ )
   EXPECT_EQ(0, sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[0]->tid()));
#endif // OS

   EXPECT_EQ(0, m_Scratch[1]);

   EXPECT_TRUE(m_Sem.Post(1));

   int    k;
   btBool Done = false;

   while ( !Done ) {
      sleep_millis(1);

#if   defined( __AAL_WINDOWS__ )
      Done = sig.GetCount(SignalHelper::IDX_SIGUSR2, idx) >= m_Scratch[3];
#elif defined( __AAL_LINUX__ )
      Done = sig.GetCount(SignalHelper::IDX_SIGIO, idx) >= m_Scratch[3];
#endif // OS

      if ( !Done ) {

#if   defined( __AAL_WINDOWS__ )

         k = sig.Raise(SignalHelper::IDX_SIGUSR2, m_pThrs[0]->tid());
         Done = (0 != k);

#elif defined( __AAL_LINUX__ )

         k = sig.Raise(SignalHelper::IDX_SIGIO, m_pThrs[0]->tid());
         Done = ( ESRCH == k );

#endif // OS

      }
   }

   m_pThrs[0]->Join();
   EXPECT_EQ(1, m_Scratch[1]);

   sig.RegistryReset();
}
#endif // __AAL_LINUX__

void OSAL_Sem_f::Thr14(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[0] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[3] = 1;
}

void OSAL_Sem_f::Thr15(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[1] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[4] = 1;
}

void OSAL_Sem_f::Thr16(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_f *pTC = static_cast<OSAL_Sem_f *>(pContext);
   ASSERT_NONNULL(pTC);

//   pTC->m_Scratch[2] = 1;
   EXPECT_FALSE(pTC->m_Sem.Wait());
   pTC->m_Scratch[5] = 1;
}

TEST_F(OSAL_Sem_f, aal0071)
{
   // CSemaphore::Destroy() should unblock all current waiters and prevent new waiters from blocking.

   class aal0071AfterCSemaphoreAutoLock : public AAL::Testing::EmptyAfterCSemaphoreAutoLock
   {
   public:
      aal0071AfterCSemaphoreAutoLock(OSAL_Sem_f *pTC) :
         m_pTC(pTC),
         m_Count(0)
      {}

      virtual void OnWait()
      {
         if ( 0 == m_Count ) {
            // Thr14
            m_pTC->m_Scratch[0] = 1;
         } else if ( 1 == m_Count ) {
            // Thr15
            m_pTC->m_Scratch[1] = 1;
         } else {
            // Thr16
            m_pTC->m_Scratch[2] = 1;
         }
         ++m_Count;
      }

   protected:
      OSAL_Sem_f *m_pTC;
      btInt       m_Count;
   } AfterAutoLock(this);

   EXPECT_TRUE(m_Sem.Create(0, 3));

   m_Sem.UserDefined(reinterpret_cast<btObjectType>(&AfterAutoLock));

   m_pThrs[0] = new OSLThread(OSAL_Sem_f::Thr14,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[0]->IsOK());
   YIELD_WHILE(0 == m_Scratch[0]);

   m_pThrs[1] = new OSLThread(OSAL_Sem_f::Thr15,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[1]->IsOK());
   YIELD_WHILE(0 == m_Scratch[1]);

   m_pThrs[2] = new OSLThread(OSAL_Sem_f::Thr16,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this);

   EXPECT_TRUE(m_pThrs[2]->IsOK());
   YIELD_WHILE(0 == m_Scratch[2]);

   YIELD_N_FOREACH(EXPECT_EQ(0, m_Scratch[3] + m_Scratch[4] + m_Scratch[5]));

   EXPECT_TRUE(m_Sem.Destroy());

   m_pThrs[0]->Join();
   m_pThrs[1]->Join();
   m_pThrs[2]->Join();

   EXPECT_EQ(1, m_Scratch[3]);
   EXPECT_EQ(1, m_Scratch[4]);
   EXPECT_EQ(1, m_Scratch[5]);
}

/////////////////////////////

class SemBasic : public ::testing::Test
{
protected:
   SemBasic() {}
   virtual ~SemBasic() {}

// virtual void SetUp() { }
   virtual void TearDown()
   {
      m_Sem.Destroy();
   }

   CSemaphore m_Sem;
};

TEST_F(SemBasic, TwoCreates)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
   EXPECT_FALSE(m_Sem.Create(1, INT_MAX)) << "two Create's without an intervening Destroy";
}

TEST_F(SemBasic, DestroyBeforeCreate)
{
   EXPECT_FALSE(m_Sem.Destroy()) << "Destroy before Create";
   EXPECT_FALSE(m_Sem.Reset(1)) << "Reset before Create";

   btInt cur = 0, max = 0;
   EXPECT_FALSE(m_Sem.CurrCounts(cur, max));
   EXPECT_EQ(0, cur);
   EXPECT_EQ(0, max);

   EXPECT_FALSE(m_Sem.Post(1)) << "Post before Create";
   EXPECT_FALSE(m_Sem.Wait(1)) << "Timed Wait before Create";
   EXPECT_FALSE(m_Sem.Wait())  << "Wait before Create";
}

////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
class SemWait : public ::testing::TestWithParam< btInt >
{
protected:
   SemWait() {}
   virtual ~SemWait() {}

   virtual void SetUp()
   {
      m_pThr  = NULL;
      m_Count = 0;
   }

   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);

      if ( NULL != m_pThr ) {
         delete m_pThr;
      }

      EXPECT_TRUE(m_Sem.Destroy());
   }

   AAL::btUnsignedInt CurrentThreads() const { return (AAL::btUnsignedInt) GlobalTestConfig::GetInstance().CurrentThreads(); }

   static void ThrLoop(OSLThread * , void * );
   static void ThrOnce(OSLThread * , void * );

   CSemaphore m_Sem;
   OSLThread *m_pThr;
   btInt      m_Count;
};

void SemWait::ThrLoop(OSLThread * /* unused */, void *arg)
{
   SemWait *pFixture = reinterpret_cast<SemWait *>(arg);

   btInt       i;
   const btInt count = pFixture->m_Count;

   pFixture->m_Count = 0;
   for ( i = 0 ; i < count ; ++i ) {
      //SleepMilli(1);
      ++pFixture->m_Count;
      pFixture->m_Sem.Post(1);
   }
}

void SemWait::ThrOnce(OSLThread * /* unused */, void *arg)
{
   SemWait *pFixture = reinterpret_cast<SemWait *>(arg);

   pFixture->m_Sem.Post(pFixture->m_Count);
}

TEST_P(SemWait, InfiniteWait)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));

   btInt       i;
   const btInt count = GetParam();

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrLoop,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_TRUE(m_Sem.Wait()) << "Infinite wait loop";
   }

   EXPECT_EQ(0, m_Sem.NumWaiters());

   EXPECT_EQ(count, m_Count);

   btInt CurCount = 0;
   btInt MaxCount = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(CurCount, MaxCount));

   EXPECT_EQ(0, CurCount);
   EXPECT_EQ(INT_MAX, MaxCount);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";
   m_pThr->Join();

   EXPECT_TRUE(m_Sem.Destroy());
   delete m_pThr;
   m_pThr = NULL;


   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));

   m_Count = count;

   m_pThr = new OSLThread(SemWait::ThrOnce,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   for ( i = 0 ; i < count ; ++i ) {
      EXPECT_TRUE(m_Sem.Wait()) << "Infinite wait once";
   }
   EXPECT_EQ(count, m_Count);

   EXPECT_EQ(0, m_Sem.NumWaiters());

   CurCount = 0;
   MaxCount = 0;

   EXPECT_TRUE(m_Sem.CurrCounts(CurCount, MaxCount));

   EXPECT_EQ(0, CurCount);
   EXPECT_EQ(INT_MAX, MaxCount);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();
   delete m_pThr;
   m_pThr = NULL;
}

TEST_P(SemWait, CountUp)
{
   const btInt count = GetParam();

   EXPECT_TRUE(m_Sem.Create(-count, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrLoop,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   EXPECT_TRUE(m_Sem.Wait(10000)) << "Count up timed out loop";
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();

   EXPECT_TRUE(m_Sem.Destroy());
   delete m_pThr;
   m_pThr = NULL;


   EXPECT_TRUE(m_Sem.Create(-count, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(SemWait::ThrOnce,
                          OSLThread::THREADPRIORITY_NORMAL,
                          this);
   ASSERT_NONNULL(m_pThr);

   EXPECT_TRUE(m_Sem.Wait(10000)) << "Count up timed out loop";
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();
   delete m_pThr;
   m_pThr = NULL;
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MySem, SemWait,
                        ::testing::Range(1, 100, 5));

