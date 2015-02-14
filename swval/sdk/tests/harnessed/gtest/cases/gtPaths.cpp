// INTEL CONFIDENTIAL - For Intel Internal Use Only

#if 0

#if __AAL_LINUX__

// Simple test fixture
class batPathsStartup : public batStartup
{
protected:
batPathsStartup() :
   m_pSaveLD_LIBRARY_PATH(NULL)
{}

virtual void SetUp()
{
   batStartup::SetUp();

   // if LD_LIBRARY_PATH exists, save it's value to reset it in TearDown(), then clear it.
   char *p = getenv("LD_LIBRARY_PATH");
   if ( NULL != p ) {
      m_pSaveLD_LIBRARY_PATH = strdup(p);
      ASSERT_EQ(0, setenv("LD_LIBRARY_PATH", "", 1));
   }
}
virtual void TearDown()
{
   if ( NULL != m_pSaveLD_LIBRARY_PATH ) {
      ASSERT_EQ(0, setenv("LD_LIBRARY_PATH", m_pSaveLD_LIBRARY_PATH, 1));
      free(m_pSaveLD_LIBRARY_PATH);
   }
   batStartup::TearDown();
}

   char *m_pSaveLD_LIBRARY_PATH;
};

#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

// Tests AAL runtime start, with the bare minimum LD_LIBRARY_PATH.
TEST_F(batPathsStartup, NOKERNEL_Minimum_LD_LIBRARY_PATH)
{
   ASSERT_EQ(0, Require_NOKERNEL_Min_LD_LIBRARY_PATH());
   MSG("LD_LIBRARY_PATH = " << LD_LIBRARY_PATH)

   Runtime *pRT = new(std::nothrow) Runtime();
   ASSERT_NONNULL(pRT);

   NamedValueSet args;
   args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);

   pRT->start(this, args);
   WaitSem(); // for runtimeStarted()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   EXPECT_EQ(0, m_IRuntimesFromStopped.size());

   EXPECT_EQ(0, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   m_IRuntimesFromStarted.front()->stop();
   WaitSem(); // for runtimeStopped()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   ASSERT_EQ(1, m_IRuntimesFromStopped.size());
   EXPECT_NONNULL(m_IRuntimesFromStopped.front());

   EXPECT_EQ(0, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   EXPECT_EQ(m_IRuntimesFromStarted.front(), m_IRuntimesFromStopped.front());

   delete pRT;
}

#undef MSG

#endif // __AAL_LINUX__

////////////////////////////////////////////////////////////////////////////////

#if __AAL_LINUX__

// Simple test fixture
class batPathsServiceAlloc : public batServiceAlloc
{
protected:
batPathsServiceAlloc() :
   m_pSaveLD_LIBRARY_PATH(NULL)
{}

virtual void SetUp()
{
   batServiceAlloc::SetUp();

   // if LD_LIBRARY_PATH exists, save it's value to reset it in TearDown(), then clear it.
   char *p = getenv("LD_LIBRARY_PATH");
   if ( NULL != p ) {
      m_pSaveLD_LIBRARY_PATH = strdup(p);
      ASSERT_EQ(0, setenv("LD_LIBRARY_PATH", "", 1));
   }
}
virtual void TearDown()
{
   if ( NULL != m_pSaveLD_LIBRARY_PATH ) {
      ASSERT_EQ(0, setenv("LD_LIBRARY_PATH", m_pSaveLD_LIBRARY_PATH, 1));
      free(m_pSaveLD_LIBRARY_PATH);
   }
   batServiceAlloc::TearDown();
}

   char *m_pSaveLD_LIBRARY_PATH;
};

#if 0
# define MSG(x) std::cerr << std::endl << x << std::endl;
#else
# define MSG(x)
#endif

// Tests service allocate, with the bare minimum LD_LIBRARY_PATH.
TEST_F(batPathsServiceAlloc, NOKERNEL_Minimum_LD_LIBRARY_PATH)
{
   ASSERT_EQ(0, Require_NOKERNEL_Min_LD_LIBRARY_PATH());
   ASSERT_EQ(0, RequireLD_LIBRARY_PATH(SAMPLE_AFU1_LIBDIR));

   MSG("LD_LIBRARY_PATH = " << LD_LIBRARY_PATH)

   Runtime *pRT = new(std::nothrow) Runtime();
   ASSERT_NONNULL(pRT);

   NamedValueSet args;
   args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);

   pRT->start(this, args);
   WaitSem(); // for runtimeStarted()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());
   EXPECT_EQ(1, m_NVSFromStarted.size());
   EXPECT_EQ(0, m_IRuntimesFromStopped.size());
   EXPECT_EQ(0, m_RTStartFailed);
   EXPECT_EQ(0, m_RTMsgs);

   EXPECT_EQ(0, m_ServiceQ.size());
   EXPECT_EQ(0, m_serviceAllocatedTranIDs.size());
   EXPECT_EQ(0, m_serviceAllocateFailedTranIDs.size());
   EXPECT_EQ(0, m_serviceFreedTranIDs.size());
   EXPECT_EQ(0, m_pingTranIDs.size());

   // Request a Sample AFU 1.
   NamedValueSet Manifest(SampleAFU1ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AFU 1");

   TransactionID tid((bt32bitInt)7);

   m_IRuntimesFromStarted.front()->allocService(dynamic_cast<IBase *>(this), Manifest, tid);
   WaitSem(); // for serviceAllocated()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());
   EXPECT_EQ(1, m_NVSFromStarted.size());
   EXPECT_EQ(0, m_IRuntimesFromStopped.size());
   EXPECT_EQ(0, m_RTStartFailed);
   EXPECT_EQ(0, m_RTMsgs);

   ASSERT_EQ(1, m_ServiceQ.size());
   ASSERT_NONNULL(m_ServiceQ.front());
   ASSERT_EQ(1, m_serviceAllocatedTranIDs.size());
   EXPECT_EQ(7, m_serviceAllocatedTranIDs.front().ID());
   EXPECT_EQ(0, m_serviceAllocateFailedTranIDs.size());
   EXPECT_EQ(0, m_serviceFreedTranIDs.size());

   TransactionID releasetid((bt32bitInt)13);

   EXPECT_TRUE(m_ServiceQ.front()->Release(releasetid));
   WaitSem(); // for freed

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());
   EXPECT_EQ(1, m_NVSFromStarted.size());
   EXPECT_EQ(0, m_IRuntimesFromStopped.size());
   EXPECT_EQ(0, m_RTStartFailed);
   EXPECT_EQ(0, m_RTMsgs);

   ASSERT_EQ(1, m_ServiceQ.size());
   ASSERT_NONNULL(m_ServiceQ.front());
   ASSERT_EQ(1, m_serviceAllocatedTranIDs.size());
   EXPECT_EQ(7, m_serviceAllocatedTranIDs.front().ID());
   EXPECT_EQ(0, m_serviceAllocateFailedTranIDs.size());
   EXPECT_EQ(1, m_serviceFreedTranIDs.size());
   EXPECT_EQ(13, m_serviceFreedTranIDs.front().ID());

   m_IRuntimesFromStarted.front()->stop();
   WaitSem(); // for runtimeStopped()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());
   EXPECT_EQ(1, m_NVSFromStarted.size());
   ASSERT_EQ(1, m_IRuntimesFromStopped.size());
   EXPECT_NONNULL(m_IRuntimesFromStopped.front());
   EXPECT_EQ(m_IRuntimesFromStarted.front(), m_IRuntimesFromStopped.front());
   EXPECT_EQ(0, m_RTStartFailed);
   EXPECT_EQ(0, m_RTMsgs);

   ASSERT_EQ(1, m_ServiceQ.size());
   ASSERT_NONNULL(m_ServiceQ.front());
   ASSERT_EQ(1, m_serviceAllocatedTranIDs.size());
   EXPECT_EQ(7, m_serviceAllocatedTranIDs.front().ID());
   EXPECT_EQ(0, m_serviceAllocateFailedTranIDs.size());
   EXPECT_EQ(1, m_serviceFreedTranIDs.size());
   EXPECT_EQ(13, m_serviceFreedTranIDs.front().ID());

   delete pRT;
}

#undef MSG

#endif // __AAL_LINUX__

#endif // if 0

