// INTEL CONFIDENTIAL - For Intel Internal Use Only

// Special case - when created with 0 threads, a Thread Group creates GetNumProcessors()'s threads.
TEST(ThrGr, ZeroThreads) {
   OSLThreadGroup tg(0, 0);
   // EXPECT_EQ(GetNumProcessors(), tg.GetNumThreads());
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
class ThrGrFAF : public ::testing::TestWithParam< unsigned long >
{
protected:
ThrGrFAF() {}

virtual void SetUp()
{
   EXPECT_TRUE(m_Sem.Create(0, INT_MAX));
}
// virtual void TearDown() { }

   CSemaphore m_Sem;
};

TEST_P(ThrGrFAF, FireAndForget)
{
   (new FireAndForget(&m_Sem, GetParam()))->Fire();

   EXPECT_TRUE(m_Sem.Wait());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrFAF,
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
class ThrGrSingle : public ::testing::TestWithParam< int >
{
protected:
ThrGrSingle() {}
// virtual void SetUp() { }
// virtual void TearDown() { }

   CSemaphore     m_Sem;
   OSLThreadGroup m_ThrGrp;
};

TEST_P(ThrGrSingle, Queuing)
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
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrSingle,
                        ::testing::Values(1, 10, 100, 1000));

////////////////////////////////////////////////////////////////////////////////

// Functor for Multi-threaded Group queuing.
class Multi : public IDispatchable
{
public:
   Multi(CriticalSection &cs, volatile btUnsignedInt &counter) :
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
   CriticalSection        &m_CS;
   volatile btUnsignedInt &m_Counter;
};

// Value-parameterized test fixture
class ThrGrMulti : public ::testing::TestWithParam< btUnsignedInt >
{
protected:
ThrGrMulti() :
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

TEST_P(ThrGrMulti, SharedMemory)
{
   btUnsignedInt thrds = GetParam();

   m_pThrGrp = new(std::nothrow) OSLThreadGroup(thrds);
   ASSERT_NONNULL(m_pThrGrp);

   CriticalSection        cs;         // to synchronize accesses to counter
   volatile btUnsignedInt counter = 0;

   const btUnsignedInt loops = 1000;
   btUnsignedInt       i;
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
INSTANTIATE_TEST_CASE_P(MyThrGr, ThrGrMulti,
                        ::testing::Range<btUnsignedInt>(2, 10));

