// INTEL CONFIDENTIAL - For Intel Internal Use Only

// Simple test fixture
class CritSectFixt : public ::testing::Test
{
protected:
   CritSectFixt() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
   CriticalSection m_CS;
};

TEST_F(CritSectFixt, TrylockSameThread)
{
   // What's the expected behavior when a thread locks a critical section,
   // and then calls Trylock on the same?

   // The man page for pthread_mutex_trylock() says:
   //
   // the mutex type is PTHREAD_MUTEX_RECURSIVE and the mutex is currently owned by the calling
   // thread, the mutex lock count shall be incremented by one and the pthread_mutex_trylock()
   // function shall immediately return success.

   m_CS.Lock();

   EXPECT_TRUE(m_CS.TryLock());

   // now, 2 unlocks are required

   m_CS.Unlock();
   m_CS.Unlock();

   // **We need to verify that critical sections work this way in all supported OS's.
}

TEST_F(CritSectFixt, DestroyWhileLocked)
{
   // What happens when a critical section is destroyed with a non-zero lock count?
   m_CS.Lock();
}

