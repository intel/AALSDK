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

// Special case - when created with 0 threads, a Thread Group creates GetNumProcessors()'s threads.
TEST(batThrGr, ZeroThreads) {
   OSLThreadGroup tg(0, 0);
   EXPECT_EQ(GetNumProcessors(), tg.GetNumThreads());
   EXPECT_EQ(0, tg.GetNumWorkItems());
   EXPECT_FALSE(tg.Start()) << "Thread groups are created in the Running state";
}

// Functor for Fire-and-Forget test.
class FireAndForget : public IDispatchable
{
public:
   FireAndForget(CSemaphore   *psem,
                 unsigned long delay_in_micros) :
      m_psem(psem),
      m_micros(delay_in_micros),
      m_tg()
   {}

   void Fire()
   {
      m_tg.Add(this);
   }

   void operator() ()
   {
      SleepMicro(m_micros);
      m_psem->Post(1);
      delete this;
   }

protected:
   CSemaphore    *m_psem;
   unsigned long  m_micros;
   OSLThreadGroup m_tg;
};


// Value-parameterized test fixture
class batThreadGroupFAF : public ::testing::TestWithParam< unsigned long >
{
protected:
batThreadGroupFAF() {}

virtual void SetUp()
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
}
// virtual void TearDown() { }

   CSemaphore m_Sem;
};

TEST_P(batThreadGroupFAF, FireAndForget)
{
   (new FireAndForget(&m_Sem, GetParam()))->Fire();

   EXPECT_TRUE(m_Sem.Wait());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, batThreadGroupFAF,
                        ::testing::Values(1, 10, 100, 1000));

////////////////////////////////////////////////////////////////////////////////

// Functor for single Thread Group queuing.
class Numbered : public IDispatchable
{
public:
   Numbered(CSemaphore *psem, int x) :
      m_psem(psem),
      m_x(x)
   {}

   void operator() ()
   {
      m_psem->Post(1);
      delete this;
   }

protected:
  CSemaphore *m_psem;
  int         m_x;
};

// Value-parameterized test fixture
class batThreadGroupSingle : public ::testing::TestWithParam< int >
{
protected:
batThreadGroupSingle() {}
// virtual void SetUp() { }
// virtual void TearDown() { }

   CSemaphore     m_Sem;
   OSLThreadGroup m_ThrGrp;
};

TEST_P(batThreadGroupSingle, Queuing)
{
   const int count = GetParam();

   ASSERT_TRUE(m_Sem.Create(-count, INT_MAX));

   int i;
   for ( i = 0 ; i < count ; ++i ) {
      m_ThrGrp.Add( new Numbered(&m_Sem, i) );
   }

   EXPECT_TRUE(m_Sem.Wait());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, batThreadGroupSingle,
                        ::testing::Values(1, 10, 100, 1000));

////////////////////////////////////////////////////////////////////////////////

// Functor for Multi-threaded Group queuing.
class Multi : public IDispatchable
{
public:
   Multi(CriticalSection &cs, volatile AAL::btUnsignedInt &counter) :
      m_CS(cs),
      m_Counter(counter)
   {}

   void operator() ()
   {
      m_CS.Lock();
      ++m_Counter;
      m_CS.Unlock();
      delete this;
   }

protected:
   CriticalSection             &m_CS;
   volatile AAL::btUnsignedInt &m_Counter;
};

// Value-parameterized test fixture
class batThreadGroupMulti : public ::testing::TestWithParam< AAL::btUnsignedInt >
{
protected:
batThreadGroupMulti() :
   m_pThrGrp(NULL)
{}

// virtual void SetUp() { }
virtual void TearDown()
{
   if ( NULL != m_pThrGrp ) {
      delete m_pThrGrp;
      m_pThrGrp = NULL;
   }
}

   OSLThreadGroup *m_pThrGrp;
};

TEST_P(batThreadGroupMulti, SharedMemory)
{
   AAL::btUnsignedInt thrds = GetParam();

   m_pThrGrp = new(std::nothrow) OSLThreadGroup(thrds);
   ASSERT_NONNULL(m_pThrGrp);

   CriticalSection             cs;         // to synchronize accesses to counter
   volatile AAL::btUnsignedInt counter = 0;

   const AAL::btUnsignedInt loops = 1000;
   AAL::btUnsignedInt       i;
   for ( i = 0 ; i < loops ; ++i ) {
      ASSERT_TRUE( m_pThrGrp->Add( new(std::nothrow) Multi(cs, counter) ) );
   }

   m_pThrGrp->Drain();

   while ( counter != loops ) {
      SleepMilli(10);
   }

   EXPECT_EQ(counter, loops);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, batThreadGroupMulti,
                        ::testing::Range<AAL::btUnsignedInt>(2, 10));

