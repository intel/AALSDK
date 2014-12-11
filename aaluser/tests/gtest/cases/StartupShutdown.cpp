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

#if 0

#include <aalsdk/service/SampleAFU1Service.h>
#ifdef MSG
# undef MSG
#endif // MSG

// Simple test fixture
class batStartup : public AAL::CAASBase,
                   public ::testing::Test,
                   public AAL::XL::RT::IRuntimeClient
{
public:
   // <begin IRuntimeClient interface>
   void runtimeStarted(AAL::XL::RT::IRuntime    *pRuntime,
                       const AAL::NamedValueSet &rConfigParms);

   void runtimeStopped(AAL::XL::RT::IRuntime *pRuntime);

   void runtimeStartFailed(const AAL::IEvent &rEvent);
   void runtimeAllocateServiceFailed( IEvent const &rEvent){};

   void runtimeAllocateServiceSucceeded( AAL::IBase *pClient,
                                         TransactionID const &rTranID){};
   void runtimeEvent(const AAL::IEvent &rEvent);
   // <end IRuntimeClient interface>

protected:
batStartup() :
   AAL::CAASBase(),
   m_RTStartFailed(0),
   m_RTMsgs(0)
{
   SetSubClassInterface(iidRuntimeClient, dynamic_cast<AAL::XL::RT::IRuntimeClient*>(this));
}

virtual void SetUp()
{
   ASSERT_TRUE(m_Sem.Create(0, INT_MAX));
}
virtual void TearDown()
{
   ASSERT_TRUE(m_Sem.Destroy());
}

   void WaitSem() { m_Sem.Wait();  }
   void PostSem() { m_Sem.Post(1); }

   AAL::NamedValueSet StartupArgs() const
   {
      AAL::NamedValueSet args;

      const ::testing::TestInfo * const pInfo =
         ::testing::UnitTest::GetInstance()->current_test_info();

      args.Add("Test",     ::strdup(pInfo->name()));
      args.Add("TestCase", ::strdup(pInfo->test_case_name()));

      return args;
   }

   typedef std::list<AAL::XL::RT::IRuntime *> irt_q;
   typedef std::list<AAL::NamedValueSet>      nvs_q;

   AAL::btInt      m_RTStartFailed;
   AAL::btInt      m_RTMsgs;

   CSemaphore      m_Sem;
   CriticalSection m_CS;

   irt_q           m_IRuntimesFromStarted;
   nvs_q           m_NVSFromStarted;

   irt_q           m_IRuntimesFromStopped;
};

#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

void batStartup::runtimeStarted(AAL::XL::RT::IRuntime    *pRuntime,
                                const AAL::NamedValueSet &rConfigParms)
{
   AutoLock(&m_CS);

   m_IRuntimesFromStarted.push_back(pRuntime);
   m_NVSFromStarted.push_back(rConfigParms);

   PostSem();
}

void batStartup::runtimeStopped(AAL::XL::RT::IRuntime *pRuntime)
{
   AutoLock(&m_CS);

   m_IRuntimesFromStopped.push_back(pRuntime);

   PostSem();
}

void batStartup::runtimeStartFailed(const AAL::IEvent &rEvent)
{
   AutoLock(&m_CS);

   ++m_RTStartFailed;
   AAL::PrintExceptionDescription(rEvent);

   PostSem();
}

void batStartup::runtimeEvent(const AAL::IEvent &rEvent)
{
   AutoLock(&m_CS);

   ++m_RTMsgs;
   AAL::PrintExceptionDescription(rEvent);

   PostSem();
}

// Tests normal AAL runtime start/stop.
TEST_F(batStartup, StartThenCleanStop)
{
   AAL::XL::RT::Runtime *pRT = new(std::nothrow) AAL::XL::RT::Runtime();
   ASSERT_NONNULL(pRT);

   AAL::NamedValueSet args = StartupArgs();

   pRT->start(this, args);
   WaitSem(); // for runtimeStarted()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());
   EXPECT_EQ(1, m_NVSFromStarted.size());
   EXPECT_EQ(0, m_IRuntimesFromStopped.size());
   EXPECT_EQ(0, m_RTStartFailed);
   EXPECT_EQ(0, m_RTMsgs);

   std::string    Test;
   std::string    TestCase;
   AAL::btcString val;

   TestCaseName(Test, TestCase);

   EXPECT_TRUE(m_NVSFromStarted.front().Has("Test"));
   if ( m_NVSFromStarted.front().Has("Test") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("Test", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, Test.c_str());
   }

   EXPECT_TRUE(m_NVSFromStarted.front().Has("TestCase"));
   if ( m_NVSFromStarted.front().Has("TestCase") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("TestCase", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, TestCase.c_str());
   }

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

#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

// Tests two starts without an intervening stop.
TEST_F(batStartup, TwoStarts)
{
   AAL::XL::RT::Runtime *pRT = new(std::nothrow) AAL::XL::RT::Runtime();
   ASSERT_NONNULL(pRT);

   AAL::NamedValueSet nvs0;
   AAL::NamedValueSet nvs1;

   pRT->start(this, nvs0);
   WaitSem(); // for runtimeStarted()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   EXPECT_EQ(0, m_IRuntimesFromStopped.size());

   EXPECT_EQ(0, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   pRT->start(this, nvs1);
   WaitSem(); // for runtimeStarted()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   EXPECT_EQ(0, m_IRuntimesFromStopped.size());

   EXPECT_EQ(1, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   m_IRuntimesFromStarted.front()->stop();
   WaitSem(); // for runtimeStopped()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   ASSERT_EQ(1, m_IRuntimesFromStopped.size());
   EXPECT_NONNULL(m_IRuntimesFromStopped.front());

   EXPECT_EQ(m_IRuntimesFromStarted.front(), m_IRuntimesFromStopped.front());

   EXPECT_EQ(1, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   delete pRT;
}

#undef MSG

#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

// Tests two stops without an intervening start.
TEST_F(batStartup, TwoStops)
{
   AAL::XL::RT::Runtime *pRT = new(std::nothrow) AAL::XL::RT::Runtime();
   ASSERT_NONNULL(pRT);

   AAL::NamedValueSet nvs;
   pRT->start(this, nvs);
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

   EXPECT_EQ(m_IRuntimesFromStarted.front(), m_IRuntimesFromStopped.front());

   EXPECT_EQ(0, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   m_IRuntimesFromStarted.front()->stop();
   WaitSem(); // for runtimeStopped()

   ASSERT_EQ(1, m_IRuntimesFromStarted.size());
   ASSERT_NONNULL(m_IRuntimesFromStarted.front());

   EXPECT_EQ(1, m_NVSFromStarted.size());

   ASSERT_EQ(2, m_IRuntimesFromStopped.size());
   irt_q::const_iterator iter = m_IRuntimesFromStopped.begin();

   while ( m_IRuntimesFromStopped.end() != iter ) {
      EXPECT_NONNULL(*iter);

      EXPECT_EQ(m_IRuntimesFromStarted.front(), *iter);

      ++iter;
   }

   EXPECT_EQ(0, m_RTStartFailed);

   EXPECT_EQ(0, m_RTMsgs);

   delete pRT;
}

#undef MSG

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class batServiceAlloc : public batStartup,
                        public AAL::AAS::IServiceClient, public AAL::ISampleAFUPingClient
{
public:
   // <begin IServiceClient interface>
   void      serviceAllocated(AAL::IBase          *pServiceBase,
                              AAL::TransactionID const &rTranID = AAL::TransactionID());
   void serviceAllocateFailed(const AAL::IEvent &rEvent);
   void          serviceFreed(AAL::TransactionID const &rTranID = AAL::TransactionID());
   void serviceEvent(const AAL::IEvent & );
   // <end IServiceClient interface>

   // <begin  ISampleAFUPingClient>
   void PingReceived(AAL::TransactionID const &rTranID);
   // <end  ISampleAFUPingClient>

protected:
batServiceAlloc()
{
   SetSubClassInterface(iidServiceClient, dynamic_cast<AAL::AAS::IServiceClient *>(this));
   SetInterface(iidSampleAFUPingClient, dynamic_cast<AAL::ISampleAFUPingClient *>(this));
}

   typedef std::list<AAL::IAALService * > service_q_t;
   typedef std::list<AAL::TransactionID > tranid_q_t;

   service_q_t m_ServiceQ;
   tranid_q_t  m_serviceAllocatedTranIDs;
   tranid_q_t  m_serviceAllocateFailedTranIDs;
   tranid_q_t  m_serviceFreedTranIDs;
   tranid_q_t  m_pingTranIDs;
};

#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

void batServiceAlloc::serviceAllocated(AAL::IBase               *pServiceBase,
                                       AAL::TransactionID const &rTranID)
{
   AutoLock(&m_CS);

   MSG("serviceAllocated")
   m_ServiceQ.push_back( dynamic_ptr<IAALService>(iidService, pServiceBase) );
   m_serviceAllocatedTranIDs.push_back(rTranID);
   PostSem();
}

void batServiceAlloc::serviceAllocateFailed(const AAL::IEvent        &rEvent)
{
   AutoLock(&m_CS);

   MSG("serviceAllocateFailed")
   AAL::TransactionID TranID = dynamic_ref<ITransactionEvent>(iidTranEvent,rEvent).TranID();
   m_serviceAllocateFailedTranIDs.push_back(&TranID);
   PostSem();
}

void batServiceAlloc::serviceFreed(AAL::TransactionID const &rTranID)
{
   AutoLock(&m_CS);

   MSG("serviceFreed")
   m_serviceFreedTranIDs.push_back(rTranID);
   PostSem();
}

void batServiceAlloc::serviceEvent(const AAL::IEvent &rEvent)
{
   AutoLock(&m_CS);

   ++m_RTMsgs;
   AAL::PrintExceptionDescription(rEvent);

   PostSem();
}

void batServiceAlloc::PingReceived(AAL::TransactionID const &rTranID)
{
   AutoLock(&m_CS);

   MSG("Ping")
   m_pingTranIDs.push_back(rTranID);
}

// Tests sw-only AAL service allocation.
TEST_F(batServiceAlloc, SimpleAlloc)
{
#if __AAL_LINUX__
   ASSERT_EQ(0, Require_NOKERNEL_Min_LD_LIBRARY_PATH());
   ASSERT_EQ(0, RequireLD_LIBRARY_PATH(SAMPLE_AFU1_LIBDIR));
#endif // __AAL_LINUX__

   AAL::NamedValueSet args = StartupArgs();

   args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);

   AAL::XL::RT::Runtime *pRT = new(std::nothrow) AAL::XL::RT::Runtime();
   ASSERT_NONNULL(pRT);

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

   std::string    Test;
   std::string    TestCase;
   AAL::btcString val;

   TestCaseName(Test, TestCase);

   EXPECT_TRUE(m_NVSFromStarted.front().Has("Test"));
   if ( m_NVSFromStarted.front().Has("Test") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("Test", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, Test.c_str());
   }

   EXPECT_TRUE(m_NVSFromStarted.front().Has("TestCase"));
   if ( m_NVSFromStarted.front().Has("TestCase") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("TestCase", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, TestCase.c_str());
   }

   // Request a Sample AFU 1.
   NamedValueSet Manifest(SampleAFU1ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AFU 1");

   AAL::TransactionID tid((AAL::bt32bitInt)7);

   m_IRuntimesFromStarted.front()->allocService(dynamic_cast<AAL::IBase *>(this), Manifest, tid);
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

   AAL::TransactionID releasetid((AAL::bt32bitInt)13);

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


#if 0
# define MSG(x) std::cerr << x << std::endl;
#else
# define MSG(x)
#endif

TEST_F(batServiceAlloc, RTStopWithoutServiceRelease)
{
#if __AAL_LINUX__
   ASSERT_EQ(0, Require_NOKERNEL_Min_LD_LIBRARY_PATH());
   ASSERT_EQ(0, RequireLD_LIBRARY_PATH(SAMPLE_AFU1_LIBDIR));
#endif // __AAL_LINUX__

   AAL::NamedValueSet args = StartupArgs();

   args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);

   AAL::XL::RT::Runtime *pRT = new(std::nothrow) AAL::XL::RT::Runtime();
   ASSERT_NONNULL(pRT);

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

   std::string    Test;
   std::string    TestCase;
   AAL::btcString val;

   TestCaseName(Test, TestCase);

   EXPECT_TRUE(m_NVSFromStarted.front().Has("Test"));
   if ( m_NVSFromStarted.front().Has("Test") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("Test", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, Test.c_str());
   }

   EXPECT_TRUE(m_NVSFromStarted.front().Has("TestCase"));
   if ( m_NVSFromStarted.front().Has("TestCase") ) {
      val = NULL;
      EXPECT_EQ(AAL::ENamedValuesOK, m_NVSFromStarted.front().Get("TestCase", &val));
      ASSERT_NONNULL(val);
      EXPECT_STREQ(val, TestCase.c_str());
   }

   // Request a Sample AFU 1.
   NamedValueSet Manifest(SampleAFU1ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AFU 1");

   AAL::TransactionID tid((AAL::bt32bitInt)7);

   m_IRuntimesFromStarted.front()->allocService(dynamic_cast<AAL::IBase *>(this), Manifest, tid);
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
   EXPECT_EQ(0, m_serviceFreedTranIDs.size());

   delete pRT;
}

#undef MSG

#endif // if 0

