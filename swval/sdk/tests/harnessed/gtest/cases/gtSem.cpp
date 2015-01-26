// INTEL CONFIDENTIAL - For Intel Internal Use Only

// Simple test fixture
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

   AAL::btInt cur = 0, max = 0;
   EXPECT_FALSE(m_Sem.CurrCounts(cur, max));
   EXPECT_EQ(0, cur);
   EXPECT_EQ(0, max);

   EXPECT_FALSE(m_Sem.Post(1)) << "Post before Create";
   EXPECT_FALSE(m_Sem.Wait(1)) << "Timed Wait before Create";
   EXPECT_FALSE(m_Sem.Wait())  << "Wait before Create";
}

////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
class SemWait : public ::testing::TestWithParam< AAL::btInt >
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
   AAL::btInt m_Count;
};

void SemWait::ThrLoop(OSLThread * /* unused */, void *arg)
{
   SemWait *pFixture = reinterpret_cast<SemWait *>(arg);

   AAL::btInt       i;
   const AAL::btInt count = pFixture->m_Count;

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

   AAL::btInt       i;
   const AAL::btInt count = GetParam();

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
   const AAL::btInt count = GetParam();

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

