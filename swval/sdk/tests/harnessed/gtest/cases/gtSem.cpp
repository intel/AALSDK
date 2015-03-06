// INTEL CONFIDENTIAL - For Intel Internal Use Only

TEST(OSAL_Sem, aal0031)
{
   // CSemaphore::~CSemaphore() should behave robustly when no call to CSemaphore::Create() was made.
   CSemaphore *pSem = new CSemaphore();
   delete pSem;
}

TEST(OSAL_Sem, DISABLED_aal0032)
{
   // CSemaphore::Destroy() should behave robustly when no call to CSemaphore::Create() was made.
   CSemaphore *pSem = new CSemaphore();
   EXPECT_FALSE(pSem->Destroy());
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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

   static void Thr0(OSLThread * , void * );
};

void OSAL_Sem_vp_tuple_0::Thr0(OSLThread *pThread, void *pContext)
{
   OSAL_Sem_vp_tuple_0 *pTC = static_cast<OSAL_Sem_vp_tuple_0 *>(pContext);
   ASSERT_NONNULL(pTC);

   const AAL::btInt         nInitialCount = pTC->m_nInitialCount;
   const AAL::btUnsignedInt nMaxCount     = pTC->m_nMaxCount;
   CSemaphore              &sem           = pTC->m_Sem;

   AAL::btInt               i;
   AAL::btUnsignedInt       u;

   // we should be able to consume nMaxCount's from the sem without blocking.
   for ( u = 0 ; u < nMaxCount ; ++u ) {
      EXPECT_TRUE(sem.Wait());
   }
   pTC->m_Scratch[0] = 1;

   ASSERT_TRUE(sem.Wait()); // (0 == count) should block.
   pTC->m_Scratch[1] = 1;
}

TEST_P(OSAL_Sem_vp_tuple_0, DISABLED_aal0049)
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



// Value-parameterized test fixture
template <typename T>
class OSAL_Sem_vp : public ::testing::TestWithParam<T>
{
protected:
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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;
};

class OSAL_Sem_vp_int_0 : public OSAL_Sem_vp< AAL::btInt >
{
protected:
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
   AAL::btUnsignedInt       u;

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
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_int_0, ::testing::Values(1, 10, 100, 1000));


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
INSTANTIATE_TEST_CASE_P(My, OSAL_Sem_vp_uint_0, ::testing::Values(1, 10, 100, 1000));


#if 0
// Simple test fixture
class OSAL_Sem_f : public ::testing::Test
{
protected:
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
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         if ( NULL != m_pThrs[i] ) {
            delete m_pThrs[i];
         }
      }
   }

   AAL::btInt          m_nInitialCount;
   AAL::btUnsignedInt  m_nMaxCount;

   OSLThread          *m_pThrs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Sem;

//   static void Thr0(OSLThread * , void * );
//   static void Thr1(OSLThread * , void * );
};
#endif

/////////////////////////////





class SemBasic : public ::testing::Test
{
protected:
SemBasic() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
   CSemaphore m_Sem;
};

TEST_F(SemBasic, TwoCreates)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
   EXPECT_FALSE(m_Sem.Create(1, INT_MAX)) << "two Create's without an intervening Destroy";
}

TEST_F(SemBasic, DestroyBeforeCreate)
{
   EXPECT_TRUE(m_Sem.Destroy()) << "Destroy before Create";
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

virtual void SetUp()
{
   m_pThr  = NULL;
   m_Count = 0;
}

virtual void TearDown()
{
   if ( NULL != m_pThr ) {
      delete m_pThr;
   }

   EXPECT_TRUE(m_Sem.Destroy());
}

static void ThrLoop(OSLThread * , void * );
static void ThrOnce(OSLThread * , void * );

   CSemaphore m_Sem;
   OSLThread *m_pThr;
   btInt m_Count;
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
   EXPECT_EQ(count, m_Count);

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";
   m_pThr->Join();


   EXPECT_TRUE(m_Sem.Destroy());


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

   EXPECT_FALSE(m_Sem.Wait(10)) << "Should time out";

   m_pThr->Join();
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
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MySem, SemWait,
                        ::testing::Range(1, 100, 5));

