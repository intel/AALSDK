// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/aas/Dispatchables.h>

template <typename RTClient,  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename RT=Runtime>
class TRuntime_f : public ::testing::Test
{ // simple fixture
protected:
   TRuntime_f() :
      m_pRuntime(NULL)
   {}
   ~TRuntime_f() {}

   virtual void SetUp()
   {
      m_pRuntime = new(std::nothrow) RT(&m_RuntimeClient);
      ASSERT_NONNULL(m_pRuntime);

      EXPECT_TRUE(m_pRuntime->Has(iidRuntime));
      EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), m_pRuntime->Interface(iidRuntime));

      EXPECT_TRUE(m_pRuntime->IsOK());
   }
   virtual void TearDown()
   {
      if ( NULL != m_pRuntime ) {
         delete m_pRuntime;
         m_pRuntime = NULL;
      }
   }

   btBool                      start(const NamedValueSet &rconfigParms) { return m_pRuntime->start(rconfigParms); }
   void                         stop()                                  { m_pRuntime->stop();                     }
   void                 allocService(IBase               *pClient,
                                     NamedValueSet const &rManifest = NamedValueSet(),
                                     TransactionID const &rTranID   = TransactionID())
   { m_pRuntime->allocService(pClient, rManifest, rTranID); }

   btBool          schedDispatchable(IDispatchable *pdispatchable) { return m_pRuntime->schedDispatchable(pdispatchable); }
   IRuntime *        getRuntimeProxy(IRuntimeClient *pClient)      { return m_pRuntime->getRuntimeProxy(pClient);         }
   btBool        releaseRuntimeProxy()                             { return m_pRuntime->releaseRuntimeProxy();            }
   IRuntimeClient * getRuntimeClient()                             { return m_pRuntime->getRuntimeClient();               }
   btBool                       IsOK()                             { return m_pRuntime->IsOK();                           }

   RT       *m_pRuntime;
   RTClient  m_RuntimeClient;
};

typedef TRuntime_f<EmptyIRuntimeClient> Runtime_f_0;

class Runtime_f_1 : public TRuntime_f<CallTrackingIRuntimeClient>
{
protected:
   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();
      TRuntime_f<CallTrackingIRuntimeClient>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST(Runtime, DISABLED_aal0716)
{
   // When Runtime::start() then Runtime::stop() are called without any AAL Service allocation,
   // Runtime::stop() will not leak any of the resources acquired by the prior Runtime::start().

   CallTrackingIRuntimeClient rtc;
   Runtime                    rt(&rtc);

   rt.start(NamedValueSet());
   rt.stop();

   YIELD_WHILE(GlobalTestConfig::GetInstance().CurrentThreads() > 0);
}

TEST(Runtime, aal0693)
{
   // When constructed with a NULL IRuntimeClient *, Runtime::Runtime() aborts construction.
   // Runtime::IsOK() returns false, indicating the error.
   Runtime rt(NULL);
   EXPECT_FALSE(rt.IsOK());
}

TEST_F(Runtime_f_1, aal0694)
{
   // When constructed with a valid IRuntimeClient *, Runtime::Runtime() sets an interface for
   // iidRuntime / IRuntime *. Runtime::IsOK() returns true, indicating success.

   EXPECT_TRUE(m_pRuntime->Has(iidRuntime));
   EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), m_pRuntime->Interface(iidRuntime));

   EXPECT_EQ(0, m_RuntimeClient.LogEntries());
}

TEST(Runtime, aal0695)
{
   // Attempts to construct more than one Runtime instance in an application process fail
   // with an exception event dispatched to IRuntimeClient::runtimeCreateOrGetProxyFailed().

   CallTrackingIRuntimeClient rtc[2];
   Runtime                   *pRT[2];

   pRT[0] = new(std::nothrow) Runtime(&rtc[0]);
   ASSERT_NONNULL(pRT[0]);
   EXPECT_TRUE(pRT[0]->IsOK());

   EXPECT_EQ(0, rtc[0].LogEntries());

   pRT[1] = new(std::nothrow) Runtime(&rtc[1]);
   ASSERT_NONNULL(pRT[1]);
   EXPECT_FALSE(pRT[1]->IsOK());

   delete pRT[1];

   EXPECT_EQ(1, rtc[1].LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeCreateOrGetProxyFailed", rtc[1].Entry(0).MethodName());

   rtc[1].ClearLog();

   delete pRT[0];

   pRT[1] = new(std::nothrow) Runtime(&rtc[1]);
   ASSERT_NONNULL(pRT[1]);
   EXPECT_TRUE(pRT[1]->IsOK());

   delete pRT[1];

   EXPECT_EQ(0, rtc[1].LogEntries());
}

TEST_F(Runtime_f_0, aal0696)
{
   // When passed NULL, Runtime::getRuntimeProxy() returns NULL.
   IRuntime *p = getRuntimeProxy(NULL);
   EXPECT_NULL(p);
}

TEST_F(Runtime_f_0, aal0697)
{
   // When passed a non-NULL IRuntimeClient *, Runtime::getRuntimeProxy() returns a valid proxy object.

   CallTrackingIRuntimeClient rtc;

   IRuntime *p = getRuntimeProxy(&rtc);
   ASSERT_NONNULL(p);

   EXPECT_TRUE(p->IsOK());
   EXPECT_TRUE(p->releaseRuntimeProxy());

   EXPECT_EQ(0, rtc.LogEntries());
}

TEST_F(Runtime_f_0, aal0727)
{
   // Runtime::getRuntimeClient() returns the IRuntimeClient * passed to the Runtime constructor.

   EXPECT_EQ(dynamic_cast<IRuntimeClient *>(&m_RuntimeClient), getRuntimeClient());
}

TEST_F(Runtime_f_0, aal0728)
{
   // Runtime::releaseRuntimeProxy() deletes this and returns true.

   EXPECT_TRUE(m_pRuntime->releaseRuntimeProxy());
   m_pRuntime = NULL;
}

TEST_F(Runtime_f_1, DISABLED_aal0729)
{
   // When Runtime::allocService() is called without Runtime::start() having been called first,
   // [Runtime is owner]

   CallTrackingIServiceClient SvcClient;
   NamedValueSet nvs;
   nvs.Add((btNumberKey)3, (btByte)2);
   TransactionID tid;
   tid.ID(3);

   m_pRuntime->allocService(&SvcClient, nvs, tid);

   EXPECT_FALSE(true) << m_RuntimeClient;
   EXPECT_TRUE(false) << SvcClient;
}

TEST_F(Runtime_f_1, DISABLED_aal0730)
{
   // When Runtime::allocService() is called without Runtime::start() having been called first,
   // [Runtime not owner]

   CallTrackingIRuntimeClient RTClient;
   IRuntime *pProxy = getRuntimeProxy(&RTClient);
   ASSERT_NONNULL(pProxy);


   CallTrackingIServiceClient SvcClient;
   NamedValueSet nvs;
   nvs.Add((btNumberKey)3, (btByte)2);
   TransactionID tid;
   tid.ID(3);

   pProxy->allocService(&SvcClient, nvs, tid);

   pProxy->releaseRuntimeProxy();

   EXPECT_FALSE(true) << m_RuntimeClient;
   EXPECT_FALSE(true) << RTClient;
   EXPECT_FALSE(true) << SvcClient;
}

TEST_F(Runtime_f_1, aal0731)
{
   // When Runtime::stop() is called without Runtime::start() having been called first,
   // IRuntimeClient::runtimeStopped() is sent to the IRuntimeClient * passed to the
   // Runtime constructor. [Runtime is owner]

   m_pRuntime->stop();

   ASSERT_EQ(1, m_RuntimeClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStopped", m_RuntimeClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);

   ASSERT_NONNULL(x);
   EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), reinterpret_cast<IRuntime *>(x));
}

TEST_F(Runtime_f_1, DISABLED_aal0732)
{
   // When Runtime::stop() is called without Runtime::start() having been called first
   // [Runtime not the owner], RuntimeCallback(RuntimeCallback::StopFailed ...) is sent
   // to the IRuntimeClient * passed to the Runtime constructor.

   CallTrackingIRuntimeClient RTClient;
   IRuntime *pProxy = getRuntimeProxy(&RTClient);
   ASSERT_NONNULL(pProxy);


   pProxy->stop();

   YIELD_WHILE(m_RuntimeClient.LogEntries() < 1);

   ASSERT_EQ(0, RTClient.LogEntries());

   ASSERT_EQ(1, m_RuntimeClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStopFailed", m_RuntimeClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RuntimeClient.Entry(0).GetParam("e", &x);

   ASSERT_NONNULL(x);
}

TEST_F(Runtime_f_1, aal0733)
{
   // When given a NULL IDispatchable *, Runtime::schedDispatchable() returns false.

   EXPECT_FALSE(m_pRuntime->schedDispatchable(NULL));
   EXPECT_EQ(0, m_RuntimeClient.LogEntries());

   CallTrackingIRuntimeClient RTClient;
   IRuntime *pProxy = getRuntimeProxy(&RTClient);
   ASSERT_NONNULL(pProxy);

   EXPECT_FALSE(pProxy->schedDispatchable(NULL));
   EXPECT_EQ(0, m_RuntimeClient.LogEntries());
   EXPECT_EQ(0, RTClient.LogEntries());

   pProxy->releaseRuntimeProxy();
}

////////////////////////////////////////////////////////////////////////////////

TEST(AALRuntime, aal0802)
{
   // When libaalrt is opened via OSServiceModuleOpen(), the value queried
   // by the standard entry point command C matches the value created by the
   // configure.ac macro M, where {C, M} are {AAL_SVC_CMD_VER_STR, AALRUNTIME_VERSION},
   // {AAL_SVC_CMD_VER_CURRENT, AALRUNTIME_VERSION_CURRENT},
   // {AAL_SVC_CMD_VER_REVISION, AALRUNTIME_VERSION_REVISION},
   // {AAL_SVC_CMD_VER_AGE, AALRUNTIME_VERSION_AGE}.

#if   defined( __AAL_LINUX__ )
   std::string lt_objdir(LT_OBJDIR);
   if ( ( lt_objdir.length() > 0 ) && ( '/' == lt_objdir[lt_objdir.length() - 1] ) ) {
      lt_objdir = lt_objdir.substr(0, lt_objdir.length() - 1);
   }

   std::string path(GlobalTestConfig::GetInstance().Vpath() +
                    std::string("/aas/AALRuntime/")         +
                    lt_objdir);

   //std::cerr << "Before, LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

   EXPECT_EQ(0, RequireLD_LIBRARY_PATH(path.c_str()));
   //std::cerr << "During, LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

#elif defined( __AAL_WINDOWS__ )
# error Implement path resolution for Windows.
#endif // OS

   OSServiceModule mod;

   OSServiceModuleInit(&mod, "libaalrt");

   if ( 0 != OSServiceModuleOpen(&mod) ) {
#if   defined( __AAL_LINUX__ )
      UnRequireLD_LIBRARY_PATH(path.c_str());
#endif // OS
      FAIL() << "couldn't open libaalrt";
   }

   btUnsigned32bitInt cur = 0;
   btUnsigned32bitInt rev = 0;
   btUnsigned32bitInt age = 0;

   char VerStr[AAL_SVC_MOD_VER_STR_MAX + 1] = { 0, };

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_CURRENT, (AAL::btAny)&cur));
   EXPECT_EQ(AALRUNTIME_VERSION_CURRENT, cur);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_REVISION, (AAL::btAny)&rev));
   EXPECT_EQ(AALRUNTIME_VERSION_REVISION, rev);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_AGE, (AAL::btAny)&age));
   EXPECT_EQ(AALRUNTIME_VERSION_AGE, age);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_STR, (AAL::btAny)VerStr));
   EXPECT_STREQ(AALRUNTIME_VERSION, VerStr);

   EXPECT_EQ(0, OSServiceModuleClose(&mod));

#if   defined( __AAL_LINUX__ )

   EXPECT_EQ(0, UnRequireLD_LIBRARY_PATH(path.c_str()));
   //std::cerr << "After,  LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

#elif defined( __AAL_WINDOWS__ )
# error Implement path resolution for Windows.
#endif // OS
}

