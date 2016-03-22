// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

template <typename RTClient,  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename RT=Runtime>
class TRuntime_Int_f_0 : public ::testing::Test
{ // simple fixture
protected:
   TRuntime_Int_f_0() :
      m_pRuntime(NULL)
   {}
   ~TRuntime_Int_f_0() {}

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

typedef TRuntime_Int_f_0<EmptyIRuntimeClient> Runtime_Int_f_0;

class Runtime_Int_f_1 : public TRuntime_Int_f_0<CallTrackingIRuntimeClient>
{
protected:
   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();
      TRuntime_Int_f_0<CallTrackingIRuntimeClient>::TearDown();
   }
};

class Runtime_Int_f_2 : public TRuntime_Int_f_0<SynchronizingIRuntimeClient>
{
protected:
   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();
      TRuntime_Int_f_0<SynchronizingIRuntimeClient>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(Runtime_Int_f_2, DISABLED_aal0783)
{
   // When Runtime::start(), then Runtime::stop() is called with no intervening service allocation.

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

TEST_F(Runtime_Int_f_2, aal0784)
{
   // When Runtime::stop() is called without a prior Runtime::start(),
   // IRuntimeClient::runtimeStopped() is sent to the IRuntimeClient * passed to the Runtime c'tor.
   // The value passed to IRuntimeClient::runtimeStopped() is a pointer to the Runtime instance
   // upon which Runtime::stop() was invoked. Applications must wait for the
   // IRuntimeClient::runtimeStopped() notification, prior to destroying the IRuntimeClient, else
   // they risk segfault caused by the async runtimeStopped() delivery.

   stop();
   m_RuntimeClient.Wait();

   ASSERT_EQ(1, m_RuntimeClient.LogEntries()) << m_RuntimeClient;

   EXPECT_STREQ("IRuntimeClient::runtimeStopped", m_RuntimeClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);

   ASSERT_NONNULL(x);
   IRuntime *pRT = reinterpret_cast<IRuntime *>(x);
   EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), pRT);
}

TEST_F(Runtime_Int_f_2, DISABLED_aal0785)
{
   // When Runtime::allocService() is called without a prior Runtime::start(),

   NamedValueSet nvs;
   TransactionID tid;

   allocService(&m_RuntimeClient, nvs, tid);
   m_RuntimeClient.Wait();

   ASSERT_EQ(1, m_RuntimeClient.LogEntries()) << m_RuntimeClient;


}

TEST_F(Runtime_Int_f_2, aal0786)
{
   // When Runtime::schedDispatchable() is called without a prior Runtime::start(), the dispatchable
   // is accepted, and Runtime::schedDispatchable() returns true.

   class aal0786Disp : public IDispatchable
   {
   public:
      aal0786Disp(int &Count) :
         m_Count(Count)
      {}

      virtual void operator() ()
      {
         ++m_Count;
      }

   protected:
      int &m_Count;
   };

   int         i = 0;
   aal0786Disp disp(i);

   EXPECT_TRUE(schedDispatchable(&disp));

   YIELD_WHILE(0 == i);
   EXPECT_EQ(1, i);
}

TEST_F(Runtime_Int_f_2, aal0787)
{
   // When Runtime::getRuntimeProxy() is called without a prior Runtime::start(), a valid proxy
   // object is returned.

   SynchronizingIRuntimeClient client;

   IRuntime *pRT = getRuntimeProxy(&client);

   ASSERT_NONNULL(pRT);

   EXPECT_TRUE(pRT->releaseRuntimeProxy());
}

TEST_F(Runtime_Int_f_2, DISABLED_aal0788)
{
   // When Runtime::releaseRuntimeProxy() is called on the runtime owner,

   EXPECT_TRUE(releaseRuntimeProxy());

}

TEST_F(Runtime_Int_f_2, aal0789)
{
   // When Runtime::getRuntimeClient() is called without a prior Runtime::start(), the IRuntimeClient *
   // sent to the Runtime c'tor is returned.

   EXPECT_EQ(dynamic_cast<IRuntimeClient *>(&m_RuntimeClient), getRuntimeClient());
}

TEST_F(Runtime_Int_f_2, aal0790)
{
   // When Runtime::IsOK() is called without a prior Runtime::start(), the function returns true.

   EXPECT_TRUE(IsOK());
}

////////////////////////////////////////////////////////////////////////////////

template <typename RTClient,
          typename SvcClient,
          typename RT=Runtime>
class TRuntime_Int_f_1 : public ::testing::Test
{ // simple fixture
protected:
   TRuntime_Int_f_1() :
      m_pRuntime(NULL)
   {}
   ~TRuntime_Int_f_1() {}

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
   SvcClient m_ServiceClient;
};

class Runtime_Int_f_3 : public TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingIServiceClient>
{
protected:
   virtual void SetUp()
   {
      TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingIServiceClient>::SetUp();

      m_StartParams.Add((btStringKey)"A", (btUnsigned32bitInt)7);

      EXPECT_TRUE(start(m_StartParams));

      m_RuntimeClient.Wait(); // for runtimeStarted()

      EXPECT_EQ(1, m_RuntimeClient.LogEntries());
      EXPECT_STREQ("IRuntimeClient::runtimeStarted", m_RuntimeClient.Entry(0).MethodName());

      btObjectType x = NULL;
      m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);
      EXPECT_NONNULL(x);
      EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), reinterpret_cast<IRuntime *>(x));

      INamedValueSet const *pNVS = NULL;
      m_RuntimeClient.Entry(0).GetParam("nvs", &pNVS);

      EXPECT_TRUE(m_StartParams == *pNVS);

      m_RuntimeClient.ClearLog();
   }

   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();

      if ( NULL != m_pRuntime ) {
         m_pRuntime->stop();

         m_RuntimeClient.Wait(); // for runtimeStopped()

         if ( m_RuntimeClient.LogEntries() > 0 ) {

            EXPECT_STREQ("IRuntimeClient::runtimeStopped", m_RuntimeClient.Entry(0).MethodName());

            btObjectType x = NULL;
            m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);

            EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), reinterpret_cast<IRuntime *>(x));
         } else {
            FAIL();
         }

         m_RuntimeClient.ClearLog();
      }

      TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingIServiceClient>::TearDown();
   }

   void VerifyServiceAllocFailed(SynchronizingIRuntimeClient *pRTClient)
   {
      pRTClient->Wait();
      ASSERT_EQ(1, pRTClient->LogEntries());
      EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceFailed", pRTClient->Entry(0).MethodName());
      btObjectType x = NULL;
      pRTClient->Entry(0).GetParam("e", &x);
      ASSERT_NONNULL(x);
   }

   void VerifyServiceAllocFailed(SynchronizingIServiceClient *pSvcClient)
   {
      pSvcClient->Wait();
      ASSERT_EQ(1, pSvcClient->LogEntries());
      EXPECT_STREQ("IServiceClient::serviceAllocateFailed", pSvcClient->Entry(0).MethodName());

      btObjectType x = NULL;
      pSvcClient->Entry(0).GetParam("e", &x);
      ASSERT_NONNULL(x);
   }

   NamedValueSet m_StartParams;
};

class Runtime_Int_f_4 : public TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingSwvalSvcClient>
{
protected:
   virtual void SetUp()
   {
      TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingSwvalSvcClient>::SetUp();

      m_StartParams.Add((btStringKey)"B", (btUnsigned32bitInt)8);

      EXPECT_TRUE(start(m_StartParams));

      m_RuntimeClient.Wait(); // for runtimeStarted()

      EXPECT_EQ(1, m_RuntimeClient.LogEntries());
      EXPECT_STREQ("IRuntimeClient::runtimeStarted", m_RuntimeClient.Entry(0).MethodName());

      btObjectType x = NULL;
      m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);
      EXPECT_NONNULL(x);
      EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), reinterpret_cast<IRuntime *>(x));

      INamedValueSet const *pNVS = NULL;
      m_RuntimeClient.Entry(0).GetParam("nvs", &pNVS);

      EXPECT_TRUE(m_StartParams == *pNVS);

      m_RuntimeClient.ClearLog();
   }

   virtual void TearDown()
   {
      m_RuntimeClient.ClearLog();

      if ( NULL != m_pRuntime ) {
         m_pRuntime->stop();

         m_RuntimeClient.Wait(); // for runtimeStopped()

         if ( m_RuntimeClient.LogEntries() > 0 ) {

            EXPECT_STREQ("IRuntimeClient::runtimeStopped", m_RuntimeClient.Entry(0).MethodName());

            btObjectType x = NULL;
            m_RuntimeClient.Entry(0).GetParam("pRuntime", &x);

            EXPECT_EQ(dynamic_cast<IRuntime *>(m_pRuntime), reinterpret_cast<IRuntime *>(x));
         } else {
            FAIL();
         }

         m_RuntimeClient.ClearLog();
      }

      TRuntime_Int_f_1<SynchronizingIRuntimeClient, SynchronizingSwvalSvcClient>::TearDown();
   }

   std::pair<IBase * , ISwvalSvcMod * > VerifyServiceAllocSuccess(SynchronizingIRuntimeClient *pRTClient,
                                                                  const TransactionID         &tid)
   {
      pRTClient->Wait();
      EXPECT_EQ(1, pRTClient->LogEntries());
      EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", pRTClient->Entry(0).MethodName());

      btObjectType x = NULL;
      pRTClient->Entry(0).GetParam("pServiceBase", &x);
      EXPECT_NONNULL(x);

      IBase *pServiceBase = reinterpret_cast<IBase *>(x);
      ISwvalSvcMod *pService = dynamic_ptr<ISwvalSvcMod>(iidSwvalSvc, pServiceBase);
      EXPECT_NONNULL(pService);

      TransactionID tid2;
      pRTClient->Entry(0).GetParam("tid", tid2);
      EXPECT_TRUE(tid2 == tid);

      return std::make_pair(pServiceBase, pService);
   }

   std::pair<IBase * , ISwvalSvcMod * > VerifyServiceAllocSuccess(SynchronizingSwvalSvcClient *pSvcClient,
                                                                  const TransactionID         &tid)
   {
      pSvcClient->Wait();
      EXPECT_EQ(1, pSvcClient->LogEntries());
      EXPECT_STREQ("IServiceClient::serviceAllocated", pSvcClient->Entry(0).MethodName());

      btObjectType x = NULL;
      pSvcClient->Entry(0).GetParam("pBase", &x);
      EXPECT_NONNULL(x);

      IBase *pServiceBase = reinterpret_cast<IBase *>(x);
      ISwvalSvcMod *pService = dynamic_ptr<ISwvalSvcMod>(iidSwvalSvc, pServiceBase);
      EXPECT_NONNULL(pService);

      TransactionID tid2;
      pSvcClient->Entry(0).GetParam("tid", tid2);
      EXPECT_TRUE(tid2 == tid);

      return std::make_pair(pServiceBase, pService);
   }

   NamedValueSet m_StartParams;
};

// An IBase * which does not implement IServiceClient.
class FauxServiceClient : public CAASBase
{
public:
   FauxServiceClient()
   {}
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(Runtime_Int_f_3, DISABLED_aal0791)
{
   // Allocating a sw-only AAL non-Service module. (AAL_BEGIN_MOD, not AAL_BEGIN_SVC_MOD)

   TransactionID tid;
   tid.ID(791);

   AllocSwvalMod(m_pRuntime, &m_ServiceClient, tid);
   m_ServiceClient.Wait();

   ASSERT_EQ(1, m_ServiceClient.LogEntries());

}

TEST_F(Runtime_Int_f_3, aal0792)
{
   // When the IBase * to Runtime::allocService() does not implement IServiceClient.
   // (empty manifest)

   NamedValueSet manifest;

   TransactionID tid;
   tid.ID(792);

   FauxServiceClient SvcClient;

   m_pRuntime->allocService(&SvcClient, manifest, tid);

   VerifyServiceAllocFailed(&m_RuntimeClient);
}

TEST_F(Runtime_Int_f_3, aal0793)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   // (empty manifest)

   NamedValueSet manifest;

   TransactionID tid;
   tid.ID(793);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   VerifyServiceAllocFailed(&m_ServiceClient);
   VerifyServiceAllocFailed(&m_RuntimeClient);
}

TEST_F(Runtime_Int_f_3, aal0794)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_SERVICENAME

   NamedValueSet manifest;
   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "aal0794");

   TransactionID tid;
   tid.ID(794);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   VerifyServiceAllocFailed(&m_ServiceClient);
   VerifyServiceAllocFailed(&m_RuntimeClient);
}

TEST_F(Runtime_Int_f_3, aal0795)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_SERVICENAME
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED (empty)

   NamedValueSet manifest;
   NamedValueSet configrec;

   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "aal0795");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);

   TransactionID tid;
   tid.ID(795);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   VerifyServiceAllocFailed(&m_ServiceClient);
   VerifyServiceAllocFailed(&m_RuntimeClient);
}

TEST_F(Runtime_Int_f_3, aal0796)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_SERVICENAME
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED
   //
   // config record has..
   //  * AAL_FACTORY_CREATE_SOFTWARE_SERVICE

   NamedValueSet manifest;
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "aal0796");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);

   TransactionID tid;
   tid.ID(796);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   VerifyServiceAllocFailed(&m_ServiceClient);
   VerifyServiceAllocFailed(&m_RuntimeClient);
}

TEST_F(Runtime_Int_f_3, DISABLED_aal0797)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_SERVICENAME
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED
   //
   // config record has..
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME
   //
   // AAL_BEGIN_MOD()

   NamedValueSet manifest;
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalmod");

   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "aal0797");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);

   TransactionID tid;
   tid.ID(797);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   m_ServiceClient.Wait();
   ASSERT_EQ(1, m_ServiceClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocateFailed", m_ServiceClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_ServiceClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);


   m_RuntimeClient.Wait();
   ASSERT_EQ(1, m_RuntimeClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceFailed", m_RuntimeClient.Entry(0).MethodName());
   x = NULL;
   m_RuntimeClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);
}

TEST_F(Runtime_Int_f_4, aal0798)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_SERVICENAME
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED
   //
   // config record has..
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME
   //
   // AAL_BEGIN_SVC_MOD()

   NamedValueSet manifest;
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalsvcmod");

   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "aal0798");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);

   TransactionID tid;
   tid.ID(798);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   std::pair<IBase *, ISwvalSvcMod *> svcres = VerifyServiceAllocSuccess(&m_ServiceClient, tid);
   std::pair<IBase *, ISwvalSvcMod *> rtres  = VerifyServiceAllocSuccess(&m_RuntimeClient, tid);

   EXPECT_EQ(svcres.first,  rtres.first);
   EXPECT_EQ(svcres.second, rtres.second);



   TransactionID tid2;
   TransactionID tid3;
   tid3.ID(999);

   svcres.second->DoSomething(tid3, 0);
   m_ServiceClient.Wait();

   ASSERT_EQ(2, m_ServiceClient.LogEntries());
   EXPECT_STREQ("ISwvalSvcClient::DidSomething", m_ServiceClient.Entry(1).MethodName());

   m_ServiceClient.Entry(1).GetParam("tid", tid2);
   EXPECT_TRUE(tid2 == tid3);

   int i = 0;
   m_ServiceClient.Entry(1).GetParam("val", &i);
   EXPECT_EQ(1, i);

   EXPECT_EQ(1, m_RuntimeClient.LogEntries());


   EXPECT_TRUE(dynamic_ptr<IAALService>(iidService, svcres.first)->Release(tid3));
   m_ServiceClient.Wait();

   ASSERT_EQ(3, m_ServiceClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_ServiceClient.Entry(2).MethodName());

   m_ServiceClient.Entry(2).GetParam("tid", tid);
   EXPECT_TRUE(tid == tid3);

   EXPECT_EQ(1, m_RuntimeClient.LogEntries());

}

TEST_F(Runtime_Int_f_4, aal0799)
{
   // When the IBase * to Runtime::allocService() implements IServiceClient.
   //
   // manifest has..
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED
   //
   // config record has..
   //  * AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME
   //
   // AAL_BEGIN_SVC_MOD()

   NamedValueSet manifest;
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalsvcmod");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);

   TransactionID tid;
   tid.ID(799);

   m_pRuntime->allocService(&m_ServiceClient, manifest, tid);

   m_ServiceClient.Wait();
   ASSERT_EQ(1, m_ServiceClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocated", m_ServiceClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_ServiceClient.Entry(0).GetParam("pBase", &x);
   ASSERT_NONNULL(x);

   IBase *pServiceBase = reinterpret_cast<IBase *>(x);
   ISwvalSvcMod *pService = dynamic_ptr<ISwvalSvcMod>(iidSwvalSvc, pServiceBase);
   ASSERT_NONNULL(pService);

   TransactionID tid2;
   m_ServiceClient.Entry(0).GetParam("tid", tid2);
   EXPECT_TRUE(tid2 == tid);


   m_RuntimeClient.Wait();
   ASSERT_EQ(1, m_RuntimeClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_RuntimeClient.Entry(0).MethodName());

   x = NULL;
   m_RuntimeClient.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_NONNULL(x);

   ASSERT_EQ(pServiceBase, reinterpret_cast<IBase *>(x));
   EXPECT_EQ(pService, dynamic_ptr<ISwvalSvcMod>(iidSwvalSvc, reinterpret_cast<IBase *>(x)));


   TransactionID tid3;
   tid3.ID(999);

   pService->DoSomething(tid3, 0);
   m_ServiceClient.Wait();

   ASSERT_EQ(2, m_ServiceClient.LogEntries());
   EXPECT_STREQ("ISwvalSvcClient::DidSomething", m_ServiceClient.Entry(1).MethodName());


   int i = 0;
   m_ServiceClient.Entry(1).GetParam("val", &i);
   EXPECT_EQ(1, i);

   EXPECT_EQ(1, m_RuntimeClient.LogEntries());


   EXPECT_TRUE(dynamic_ptr<IAALService>(iidService, pServiceBase)->Release(tid3));
   m_ServiceClient.Wait();

   ASSERT_EQ(3, m_ServiceClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_ServiceClient.Entry(2).MethodName());

   m_ServiceClient.Entry(2).GetParam("tid", tid);
   EXPECT_TRUE(tid == tid3);

   EXPECT_EQ(1, m_RuntimeClient.LogEntries());
}




/*
   NamedValueSet configrec;

   configrec.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalmod");
   configrec.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   NamedValueSet manifest;

   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrec);
   manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "swval module");

   pRuntime->allocService(pClientBase, manifest, tid);

*/

