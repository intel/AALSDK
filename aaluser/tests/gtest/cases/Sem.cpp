// Copyright (c) 2014, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Simple test fixture
class batSemBasic : public ::testing::Test
{
protected:
batSemBasic() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
   CSemaphore m_Sem;
};

TEST_F(batSemBasic, TwoCreates)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
   EXPECT_FALSE(m_Sem.Create(1, INT_MAX)) << "two Create's without an intervening Destroy";
}

TEST_F(batSemBasic, DestroyBeforeCreate)
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
class batSemWait : public ::testing::TestWithParam< AAL::btInt >
{
protected:
batSemWait() {}

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

void batSemWait::ThrLoop(OSLThread * /* unused */, void *arg)
{
   batSemWait *pFixture = reinterpret_cast<batSemWait *>(arg);

   AAL::btInt       i;
   const AAL::btInt count = pFixture->m_Count;

   pFixture->m_Count = 0;
   for ( i = 0 ; i < count ; ++i ) {
      //SleepMilli(1);
      ++pFixture->m_Count;
      pFixture->m_Sem.Post(1);
   }
}

void batSemWait::ThrOnce(OSLThread * /* unused */, void *arg)
{
   batSemWait *pFixture = reinterpret_cast<batSemWait *>(arg);

   pFixture->m_Sem.Post(pFixture->m_Count);
}

TEST_P(batSemWait, InfiniteWait)
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));

   AAL::btInt       i;
   const AAL::btInt count = GetParam();

   m_Count = count;
   m_pThr = new OSLThread(batSemWait::ThrLoop,
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
   m_pThr = new OSLThread(batSemWait::ThrOnce,
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

TEST_P(batSemWait, CountUp)
{
   const AAL::btInt count = GetParam();

   EXPECT_TRUE(m_Sem.Create(-count, INT_MAX));

   m_Count = count;
   m_pThr = new OSLThread(batSemWait::ThrLoop,
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
   m_pThr = new OSLThread(batSemWait::ThrOnce,
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
INSTANTIATE_TEST_CASE_P(My, batSemWait,
                        ::testing::Range(1, 100, 5));

