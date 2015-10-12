// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#if 0
TEST(Runtime, Runtime_start)
{
   CallTrackingIRuntimeClient rtc;
   Runtime                    rt(&rtc);

   rt.start(NamedValueSet());
}
#endif

TEST(Runtime, aal0693)
{
   // When constructed with a NULL IRuntimeClient *, Runtime::Runtime() aborts construction.
   // Runtime::IsOK() returns false, indicating the error.
   Runtime rt(NULL);
   EXPECT_FALSE(rt.IsOK());
}

TEST(Runtime, aal0694)
{
   // When constructed with a valid IRuntimeClient *, Runtime::Runtime() sets an interface for
   // iidRuntime / IRuntime *. Runtime::IsOK() returns true, indicating success.

   CallTrackingIRuntimeClient rtc;
   Runtime                    rt(&rtc);

   EXPECT_TRUE(rt.Has(iidRuntime));
   EXPECT_EQ(dynamic_cast<IRuntime *>(&rt), rt.Interface(iidRuntime));

   EXPECT_TRUE(rt.IsOK());

   EXPECT_EQ(0, rtc.LogEntries());
}

TEST(Runtime, aal0695)
{
   // Attempts to construct more than one Runtime instance in an application process fail
   // with an exception event dispatched to IRuntimeClient::runtimeCreateOrGetProxyFailed().

   CallTrackingIRuntimeClient rtc;

   Runtime *pRT[2];

   pRT[0] = new(std::nothrow) Runtime(&rtc);
   ASSERT_NONNULL(pRT[0]);
   EXPECT_TRUE(pRT[0]->IsOK());

   EXPECT_EQ(0, rtc.LogEntries());

   pRT[1] = new(std::nothrow) Runtime(&rtc);
   ASSERT_NONNULL(pRT[1]);
   EXPECT_FALSE(pRT[1]->IsOK());

   delete pRT[1];

   EXPECT_EQ(1, rtc.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeCreateOrGetProxyFailed", rtc.Entry(0).MethodName());

   delete pRT[0];

   pRT[1] = new(std::nothrow) Runtime(&rtc);
   ASSERT_NONNULL(pRT[1]);
   EXPECT_TRUE(pRT[1]->IsOK());

   delete pRT[1];

   EXPECT_EQ(1, rtc.LogEntries());
}

////////////////////////////////////////////////////////////////////////////////

template <typename RTClient,     // EmptyIRuntimeClient, CallTrackingIRuntimeClient
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

typedef TRuntime_f<EmptyIRuntimeClient>        Runtime_f_0;
typedef TRuntime_f<CallTrackingIRuntimeClient> Runtime_f_1;

////////////////////////////////////////////////////////////////////////////////

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

