// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/Dispatchables.h>

template <typename RTClient,  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename RT=Runtime>
class TRuntime_Int_f : public ::testing::Test
{ // simple fixture
protected:
   TRuntime_Int_f() :
      m_pRuntime(NULL)
   {}
   ~TRuntime_Int_f() {}

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

typedef TRuntime_Int_f<EmptyIRuntimeClient> Runtime_Int_f_0;

class Runtime_Int_f_1 : public TRuntime_Int_f<CallTrackingIRuntimeClient>
{
protected:
   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();
      TRuntime_Int_f<CallTrackingIRuntimeClient>::TearDown();
   }
};

class Runtime_Int_f_2 : public TRuntime_Int_f<SynchronizingIRuntimeClient>
{
protected:
   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();
      TRuntime_Int_f<SynchronizingIRuntimeClient>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(Runtime_Int_f_2, DISABLED_aal0783)
{

   NamedValueSet args;

   args.Add((btStringKey)"a", (btByte)8);
   args.Add((btNumberKey)3,   (bt32bitInt)16375);

   EXPECT_TRUE(start(args));
   m_RuntimeClient.Wait();

   ASSERT_EQ(1, m_RuntimeClient.LogEntries()) << m_RuntimeClient;

   EXPECT_STREQ("IRuntimeClient::runtimeStarted", m_RuntimeClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);
   ASSERT_NONNULL(x);

   IRuntime *pRT = reinterpret_cast<IRuntime *>(x);
   EXPECT_TRUE(pRT->IsOK());

   INamedValueSet const *pNVS = NULL;
   m_RuntimeClient.Entry(0).GetParam("nvs", &pNVS);
   ASSERT_NONNULL(pNVS);

   EXPECT_TRUE(args == *pNVS);

   stop();
}



