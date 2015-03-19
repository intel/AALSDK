// INTEL CONFIDENTIAL - For Intel Internal Use Only

class PostDisp : public IDispatchable
{
public:
   PostDisp(CSemaphore &ToPost,
            AAL::btInt  PostCount=1,
            AAL::btBool ExpPostRes=true) :
      m_ToPost(ToPost),
      m_PostCount(PostCount),
      m_ExpPostRes(ExpPostRes)
   {}

   virtual void operator() () { EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount)); }

protected:
   CSemaphore &m_ToPost;
   AAL::btInt  m_PostCount;
   AAL::btBool m_ExpPostRes;
};

class WaitDisp : public IDispatchable
{
public:
   WaitDisp(CSemaphore &ToWait,
            AAL::btTime WaitTimeout=AAL_INFINITE_WAIT,
            AAL::btBool ExpWaitRes=true) :
      m_ToWait(ToWait),
      m_WaitTimeout(WaitTimeout),
      m_ExpWaitRes(ExpWaitRes)
   {}

   virtual void operator() () { EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout)); }

protected:
   CSemaphore &m_ToWait;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpWaitRes;
};

class PostThenWaitDisp : public IDispatchable
{
public:
   PostThenWaitDisp(CSemaphore &ToPost,
                    CSemaphore &ToWait,
                    AAL::btInt  PostCount=1,
                    AAL::btTime WaitTimeout=AAL_INFINITE_WAIT,
                    AAL::btBool ExpPostRes=true,
                    AAL::btBool ExpWaitRes=true) :
      m_ToPost(ToPost),
      m_ToWait(ToWait),
      m_PostCount(PostCount),
      m_WaitTimeout(WaitTimeout),
      m_ExpPostRes(ExpPostRes),
      m_ExpWaitRes(ExpWaitRes)
   {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
   }

protected:
   CSemaphore &m_ToPost;
   CSemaphore &m_ToWait;
   AAL::btInt  m_PostCount;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpPostRes;
   AAL::btBool m_ExpWaitRes;
};

class UnsafeCountUpDisp : public IDispatchable
{
public:
   UnsafeCountUpDisp(AAL::btInt &i,
                     AAL::btInt incr=1) :
      m_i(i),
      m_incr(incr)
   {}
   virtual void operator() () { m_i += m_incr; }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_incr;
};

class UnsafeCountDownDisp : public IDispatchable
{
public:
   UnsafeCountDownDisp(AAL::btInt &i,
                       AAL::btInt  decr=1) :
      m_i(i),
      m_decr(decr)
   {}
   virtual void operator() () { m_i -= m_decr; }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_decr;
};

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class OSAL_ThreadGroup_f : public ::testing::Test
{
protected:
   OSAL_ThreadGroup_f() :
      m_pGroup(NULL)
   {}
   virtual void SetUp()
   {
      m_MinThreads  = 0;
      m_MaxThreads  = 0;
      m_ThrPriority = OSLThread::THREADPRIORITY_NORMAL;

      unsigned i;

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      Destroy();
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs=0,
                           OSLThread::ThreadPriority nPriority=OSLThread::THREADPRIORITY_NORMAL)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority);
   }

   AAL::btBool Destroy()
   {
      if ( NULL == m_pGroup ) {
         return false;
      }
      delete m_pGroup;
      m_pGroup = NULL;
      return true;
   }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   volatile AAL::btUIntPtr   m_Scratch[10];
};

TEST_F(OSAL_ThreadGroup_f, aal0072)
{
   // When OSLThreadGroup is constructed with 0 == MinThreads and 0 == MaxThreads, min = max = 1.

   OSLThreadGroup *g = Create(0, 0);

   ASSERT_NONNULL(g);
   EXPECT_EQ(1, g->GetNumThreads());
}

TEST_F(OSAL_ThreadGroup_f, aal0075)
{
   // OSLThreadGroup objects are created in a state that immediately
   // accepts IDispatchable's for execution.

   CSemaphore sem;
   ASSERT_TRUE(sem.Create(0, 1));

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);

   EXPECT_TRUE(g->Add( new PostDisp(sem) ));
   EXPECT_TRUE(sem.Wait());
}

TEST_F(OSAL_ThreadGroup_f, aal0076)
{
   // OSLThreadGroup::Add() does not queue / returns false for NULL work items.

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);

   EXPECT_FALSE(g->Add(NULL));
}

////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
template <typename T>
class OSAL_ThreadGroup_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_ThreadGroup_vp() :
      m_pGroup(NULL)
   {}
   virtual ~OSAL_ThreadGroup_vp() {}

   virtual void SetUp()
   {
      m_MinThreads  = 0;
      m_MaxThreads  = 0;
      m_ThrPriority = OSLThread::THREADPRIORITY_NORMAL;

      unsigned i;

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }
   virtual void TearDown()
   {
      Destroy();
   }

   OSLThreadGroup * Create(AAL::btUnsignedInt        MinThrs,
                           AAL::btUnsignedInt        MaxThrs=0,
                           OSLThread::ThreadPriority nPriority=OSLThread::THREADPRIORITY_NORMAL)
   {
      m_MinThreads    = MinThrs;
      m_MaxThreads    = MaxThrs;
      m_ThrPriority   = nPriority;
      return m_pGroup = new OSLThreadGroup(m_MinThreads, m_MaxThreads, m_ThrPriority);
   }

   AAL::btBool Destroy()
   {
      if ( NULL == m_pGroup ) {
         return false;
      }
      delete m_pGroup;
      m_pGroup = NULL;
      return true;
   }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   volatile AAL::btUIntPtr   m_Scratch[10];
};

class OSAL_ThreadGroup_vp_uint_0 : public OSAL_ThreadGroup_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

TEST_P(OSAL_ThreadGroup_vp_uint_0, DISABLED_aal0074)
{
   // OSLThreadGroup::OSLThreadGroup() creates a thread group with uiMinThreads worker threads.

   OSLThreadGroup *g = Create( GetParam() );
   ASSERT_NONNULL(g);

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   CSemaphore WorkerCount;
   CSemaphore BlocksWorkers;

   ASSERT_TRUE(WorkerCount.Create(-w, 1));
   ASSERT_TRUE(BlocksWorkers.Create(0, 1));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitDisp(WorkerCount,
                                               BlocksWorkers,
                                               1,
                                               AAL_INFINITE_WAIT,
                                               true,
                                               false) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(WorkerCount.Wait());

   AAL::btInt Cur;
   AAL::btInt Max;

   EXPECT_TRUE(WorkerCount.CurrCounts(Cur, Max));
   EXPECT_EQ(0, Cur); // Current count must equal 0 (count up sem).

   // Unblock all workers.
   EXPECT_TRUE(BlocksWorkers.UnblockAll());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0079)
{
   // OSLThreadGroup::GetNumWorkItems() returns a snapshot of the number of work items in the queue.

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);

   EXPECT_EQ(0, g->GetNumWorkItems());

   CSemaphore InDispatch;
   CSemaphore BlocksWorkers;

   ASSERT_TRUE(InDispatch.Create(0, 1));
   ASSERT_TRUE(BlocksWorkers.Create(0, 1));

   // Queue a dispatchable that will block the worker thread.
   EXPECT_TRUE(g->Add( new PostThenWaitDisp(InDispatch, BlocksWorkers) ));

   // Wait for the initial work item to be dispatched, guaranteeing that the work queue is empty.
   EXPECT_TRUE(InDispatch.Wait());
   EXPECT_EQ(0, g->GetNumWorkItems());

   // There is one worker thread in the group, and that thread will block, unable to consume
   // more work items.
   // The work queue is currently empty.
   // Adding work items here will cause them to back up in the queue.

   AAL::btInt               counter = 0;
   AAL::btUnsignedInt       i;
   const AAL::btUnsignedInt N = GetParam();

   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(g->Add( new UnsafeCountUpDisp(counter) ));
      EXPECT_EQ(i + 1, g->GetNumWorkItems());
   }

   // none of the work items should have been dispatched.
   EXPECT_EQ(0, counter);

   // free up the worker thread, allowing it to drain the queue.
   EXPECT_TRUE(BlocksWorkers.Post(1));

   // wait for the worker to drain the queue.
   EXPECT_TRUE(g->Add( new PostDisp(InDispatch) ));
   EXPECT_TRUE(InDispatch.Wait());

   EXPECT_EQ(counter, (AAL::btInt)N);
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_0, ::testing::Values(1, 10, 25, 50));






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



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

