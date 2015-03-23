// INTEL CONFIDENTIAL - For Intel Internal Use Only

class PostD : public IDispatchable
{
public:
   PostD(CSemaphore &ToPost,
         AAL::btInt  PostCount=1,
         AAL::btBool ExpPostRes=true) :
      m_ToPost(ToPost),
      m_PostCount(PostCount),
      m_ExpPostRes(ExpPostRes)
   {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      delete this;
   }

protected:
   CSemaphore &m_ToPost;
   AAL::btInt  m_PostCount;
   AAL::btBool m_ExpPostRes;
};

class WaitD : public IDispatchable
{
public:
   WaitD(CSemaphore &ToWait,
         AAL::btTime WaitTimeout=AAL_INFINITE_WAIT,
         AAL::btBool ExpWaitRes=true) :
      m_ToWait(ToWait),
      m_WaitTimeout(WaitTimeout),
      m_ExpWaitRes(ExpWaitRes)
   {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
      delete this;
   }

protected:
   CSemaphore &m_ToWait;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpWaitRes;
};

class PostThenWaitD : public IDispatchable
{
public:
   PostThenWaitD(CSemaphore &ToPost,
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
      delete this;
   }

protected:
   CSemaphore &m_ToPost;
   CSemaphore &m_ToWait;
   AAL::btInt  m_PostCount;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpPostRes;
   AAL::btBool m_ExpWaitRes;
};

class UnsafeCountUpD : public IDispatchable
{
public:
   UnsafeCountUpD(AAL::btInt &i,
                  AAL::btInt incr=1) :
      m_i(i),
      m_incr(incr)
   {}
   virtual void operator() ()
   {
      m_i += m_incr;
      delete this;
   }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_incr;
};

class UnsafeCountDownD : public IDispatchable
{
public:
   UnsafeCountDownD(AAL::btInt &i,
                    AAL::btInt  decr=1) :
      m_i(i),
      m_decr(decr)
   {}
   virtual void operator() ()
   {
      m_i -= m_decr;
      delete this;
   }

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
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL)
   {
#if defined( __AAL_LINUX__ )
      m_pHooks = PThreadsHooks::Get();
#endif
   }
   virtual ~OSAL_ThreadGroup_f() {}
   virtual void SetUp()
   {
#if defined( __AAL_LINUX__ )
      ASSERT_NONNULL(m_pHooks);
#endif
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
   AAL::btUnsignedInt CurrentThreads() const
   {
#if   defined( __AAL_WINDOWS__ )
# error TODO Threads() for Windows
#elif defined( __AAL_LINUX__ )
      return m_pHooks->CurrentThreads() - Config.PThreadsUsedInFixtures();
#endif // OS
   }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   CSemaphore                m_Sems[5];

#if defined( __AAL_LINUX__ )
   PThreadsHooks            *m_pHooks;
#endif // __AAL_LINUX__
};

TEST_F(OSAL_ThreadGroup_f, aal0072)
{
   // When OSLThreadGroup is constructed with 0 == MinThreads and 0 == MaxThreads, min = max = 1.

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(0, 0);

   EXPECT_EQ(1, CurrentThreads());

   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());
   EXPECT_EQ(1, g->GetNumThreads());

   EXPECT_TRUE(Destroy());
   EXPECT_EQ(0, CurrentThreads());
}

TEST_F(OSAL_ThreadGroup_f, aal0075)
{
   // OSLThreadGroup objects are created in a state that immediately
   // accepts IDispatchable's for execution.

   ASSERT_TRUE(m_Sems[0].Create(0, 1));

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_EQ(1, CurrentThreads());

   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_TRUE(Destroy());
   EXPECT_EQ(0, CurrentThreads());
}

TEST_F(OSAL_ThreadGroup_f, aal0076)
{
   // OSLThreadGroup::Add() does not queue / returns false for NULL work items.

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_FALSE(g->Add(NULL));

   EXPECT_TRUE(Destroy());
   EXPECT_EQ(0, CurrentThreads());
}

////////////////////////////////////////////////////////////////////////////////

// Value-parameterized test fixture
template <typename T>
class OSAL_ThreadGroup_vp : public ::testing::TestWithParam<T>
{
protected:
   OSAL_ThreadGroup_vp() :
      m_pGroup(NULL),
      m_MinThreads(0),
      m_MaxThreads(0),
      m_ThrPriority(OSLThread::THREADPRIORITY_NORMAL)
   {
#if defined( __AAL_LINUX__ )
      m_pHooks = PThreadsHooks::Get();
#endif
   }
   virtual ~OSAL_ThreadGroup_vp() {}
   virtual void SetUp()
   {
#if defined( __AAL_LINUX__ )
      ASSERT_NONNULL(m_pHooks);
#endif
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
   AAL::btUnsignedInt CurrentThreads() const
   {
#if   defined( __AAL_WINDOWS__ )
# error TODO Threads() for Windows
#elif defined( __AAL_LINUX__ )
      return m_pHooks->CurrentThreads() - Config.PThreadsUsedInVPFixtures();
#endif // OS
   }

   OSLThreadGroup           *m_pGroup;
   AAL::btUnsignedInt        m_MinThreads;
   AAL::btUnsignedInt        m_MaxThreads;
   OSLThread::ThreadPriority m_ThrPriority;
   CSemaphore                m_Sems[5];

#if defined( __AAL_LINUX__ )
   PThreadsHooks            *m_pHooks;
#endif // __AAL_LINUX__
};

class OSAL_ThreadGroup_vp_uint_0 : public OSAL_ThreadGroup_vp< AAL::btUnsignedInt >
{
protected:
   static void Thr0(OSLThread * , void * );
};

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0074)
{
   // OSLThreadGroup::OSLThreadGroup() creates a thread group with uiMinThreads worker threads.

   EXPECT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create( GetParam() );
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   // m_Sems[1] - blocks worker threads in their dispatchables.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0],
                                            m_Sems[1],
                                            1,
                                            AAL_INFINITE_WAIT,
                                            true,
                                            true) ));
   }

   EXPECT_EQ(m_MinThreads, CurrentThreads());

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   AAL::btInt Cur;
   AAL::btInt Max;

   EXPECT_TRUE(m_Sems[0].CurrCounts(Cur, Max));
   EXPECT_EQ(0, Cur); // Current count must equal 0 (count up sem).

   EXPECT_EQ(m_MinThreads, g->GetNumThreads());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   EXPECT_TRUE(Destroy());
   EXPECT_EQ(0, CurrentThreads());
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0079)
{
   // OSLThreadGroup::GetNumWorkItems() returns a snapshot of the number of work items in the queue.

   ASSERT_TRUE(m_Sems[0].Create(0, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, 1));

   OSLThreadGroup *g = Create(1);
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   EXPECT_EQ(0, g->GetNumWorkItems());

   // Queue a dispatchable that will block the worker thread.
   EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0], m_Sems[1]) ));

   // Wait for the initial work item to be dispatched, guaranteeing that the work queue is empty (1 worker).
   EXPECT_TRUE(m_Sems[0].Wait());
   EXPECT_EQ(0, g->GetNumWorkItems());

   EXPECT_EQ(1, g->GetNumThreads());

   // There is one worker thread in the group, and that thread will block, unable to consume
   // more work items.
   // The work queue is currently empty.
   // Adding work items here will cause them to back up in the queue.

   AAL::btInt               counter = 0;
   AAL::btUnsignedInt       i;
   const AAL::btUnsignedInt N = GetParam();

   for ( i = 0 ; i < N ; ++i ) {
      EXPECT_TRUE(g->Add( new UnsafeCountUpD(counter) ));
      EXPECT_EQ(i + 1, g->GetNumWorkItems());
   }

   // none of the work items should have been dispatched.
   EXPECT_EQ(0, counter);

   // free up the worker thread, allowing it to drain the queue.
   EXPECT_TRUE(m_Sems[1].Post(1));

   // wait for the worker to drain the queue.
   EXPECT_TRUE(g->Add( new PostD(m_Sems[0]) ));
   EXPECT_TRUE(m_Sems[0].Wait());

   EXPECT_EQ(counter, (AAL::btInt)N);
}

TEST_P(OSAL_ThreadGroup_vp_uint_0, aal0080)
{
   // When constructed with bAutoJoin=true, OSLThreadGroup::~OSLThreadGroup() joins all workers.

   ASSERT_EQ(0, CurrentThreads());

   OSLThreadGroup *g = Create( GetParam() );
   ASSERT_NONNULL(g);
   ASSERT_TRUE(g->IsOK());

   AAL::btInt w = (AAL::btInt)m_MinThreads;

   ASSERT_EQ(m_MinThreads, CurrentThreads());

   // m_Sems[0] - count up sem, Post()'ed by each worker thread.
   ASSERT_TRUE(m_Sems[0].Create(-w, 1));
   ASSERT_TRUE(m_Sems[1].Create(0, INT_MAX));

   AAL::btUnsignedInt i;
   for ( i = 0 ; i < m_MinThreads ; ++i ) {
      EXPECT_TRUE(g->Add( new PostThenWaitD(m_Sems[0],
                                            m_Sems[1],
                                            1,
                                            AAL_INFINITE_WAIT,
                                            true,
                                            true) ));
   }

   // Block until w counts have been Post()'ed to WorkerCount.
   EXPECT_TRUE(m_Sems[0].Wait());

   // Unblock all workers.
   EXPECT_TRUE(m_Sems[1].Post(w));

   // Destroy the thread group now.
   EXPECT_TRUE(Destroy());
   ASSERT_EQ(0, CurrentThreads());
}

// ::testing::Range(begin, end [, step])
// ::testing::Values(v1, v2, v3)
// ::testing::ValuesIn(STL container), ::testing::ValuesIn(STL iter begin, STL iter end)
// ::testing::Bool()
INSTANTIATE_TEST_CASE_P(My, OSAL_ThreadGroup_vp_uint_0, ::testing::Values(1, 5, 10, 25, 50));






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



// Special case - when created with 0 threads, a Thread Group creates GetNumProcessors()'s threads.
TEST(ThrGr, DISABLED_ZeroThreads) {
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

TEST_P(ThrGrFAF, DISABLED_FireAndForget)
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

TEST_P(ThrGrSingle, DISABLED_Queuing)
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

TEST_P(ThrGrMulti, DISABLED_SharedMemory)
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

