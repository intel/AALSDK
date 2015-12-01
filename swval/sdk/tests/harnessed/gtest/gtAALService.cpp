// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/aas/Dispatchables.h>

template <typename Factory, // EmptyISvcsFact, CallTrackingISvcsFact
          typename Module=AALServiceModule>
class TAALServiceModule_f_0 : public ::testing::Test
{ // simple fixture
protected:
   TAALServiceModule_f_0() :
      m_pModule(NULL)
   {}
   ~TAALServiceModule_f_0() {}

   virtual void SetUp()
   {
      m_pModule = new(std::nothrow) Module(&m_Factory);
      ASSERT_NONNULL(m_pModule);

      EXPECT_TRUE(m_pModule->IsOK());
   }
   virtual void TearDown()
   {
      if ( NULL != m_pModule ) {
         delete m_pModule;
         m_pModule = NULL;
      }
   }

   btBool Construct(IRuntime            *pRuntime,
                    IBase               *Client,
                    TransactionID const &tid,
                    NamedValueSet const &optArgs)
   { return m_pModule->Construct(pRuntime, Client, tid, optArgs); }

   void Destroy() { m_pModule->Destroy(); }

   void      ServiceReleased(IBase *pService) { m_pModule->ServiceReleased(pService); }
   btBool ServiceInitialized(IBase *pService, TransactionID const &rtid)
   { return m_pModule->ServiceInitialized(pService, rtid); }
   btBool  ServiceInitFailed(IBase *pService, IEvent const *pEvent)
   { return m_pModule->ServiceInitFailed(pService, pEvent); }

   btBool          AddToServiceList(IBase *pService) { return m_pModule->AddToServiceList(pService);          }
   btBool     RemovefromServiceList(IBase *pService) { return m_pModule->RemovefromServiceList(pService);     }
   btBool ServiceInstanceRegistered(IBase *pService) { return m_pModule->ServiceInstanceRegistered(pService); }

   Module  *m_pModule;
   Factory  m_Factory;
};

typedef TAALServiceModule_f_0<EmptyISvcsFact> AALServiceModule_f_0;

class AALServiceModule_f_1 : public TAALServiceModule_f_0<CallTrackingISvcsFact>
{
protected:
   AALServiceModule_f_1() {}
   ~AALServiceModule_f_1() {}

   virtual void TearDown()
   {
      m_Factory.ClearLog();
      TAALServiceModule_f_0<CallTrackingISvcsFact>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Factory,  // EmptyISvcsFact, CallTrackingISvcsFact
          typename RT,       // EmptyIRuntime, CallTrackingIRuntime
          typename RTClient, // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename SB,       // EmptyServiceBase, CallTrackingServiceBase
          typename SClient,  // EmptyIServiceClient, CallTrackingIServiceClient
          typename Module=AALServiceModule,
          typename Transport=EmptyIAALTransport,
          typename Marshaller=EmptyIAALMarshaller,
          typename UnMarshaller=EmptyIAALUnMarshaller>
class TAALServiceModule_f_1 : public ::testing::Test
{ // simple fixture
protected:
   TAALServiceModule_f_1() :
      m_pModule(NULL),
      m_pTransport(NULL),
      m_pMarshaller(NULL),
      m_pUnMarshaller(NULL),
      m_pSvcBase(NULL)
   {
      m_Runtime.getRuntimeProxyReturnsThisValue(&m_Runtime);
      m_Runtime.getRuntimeClientReturnsThisValue(&m_RTClient);
   }
   ~TAALServiceModule_f_1() {}

   virtual void SetUp()
   {
      m_pModule = new(std::nothrow) Module(&m_Factory);
      ASSERT_NONNULL(m_pModule);

      EXPECT_TRUE(m_pModule->IsOK());

      // allocate, because ~ServiceBase() will delete.
      m_pTransport = new(std::nothrow) Transport();
      ASSERT_NONNULL(m_pTransport);

      // allocate, because ~ServiceBase() will delete.
      m_pMarshaller = new(std::nothrow) Marshaller();
      ASSERT_NONNULL(m_pMarshaller);

      // allocate, because ~ServiceBase() will delete.
      m_pUnMarshaller = new(std::nothrow) UnMarshaller();
      ASSERT_NONNULL(m_pUnMarshaller);

      m_pSvcBase = new(std::nothrow) SB(m_pModule,
                                        &m_Runtime,
                                        m_pTransport,
                                        m_pMarshaller,
                                        m_pUnMarshaller);
      ASSERT_NONNULL(m_pSvcBase);

      m_pSvcBase->ServiceClient(&m_SvcClient);
      m_pSvcBase->ServiceClientBase(dynamic_cast<IBase *>(&m_SvcClient));
   }
   virtual void TearDown()
   {
      if ( NULL != m_pSvcBase ) {
         delete m_pSvcBase;
         m_pSvcBase = NULL;
      }
      if ( NULL != m_pModule ) {
         delete m_pModule;
         m_pModule = NULL;
      }
   }

   btBool Construct(IRuntime            *pRuntime,
                    IBase               *Client,
                    TransactionID const &tid,
                    NamedValueSet const &optArgs)
   { return m_pModule->Construct(pRuntime, Client, tid, optArgs); }

   void Destroy() { m_pModule->Destroy(); }

   void      ServiceReleased(IBase *pService) { m_pModule->ServiceReleased(pService); }
   btBool ServiceInitialized(IBase *pService, TransactionID const &rtid)
   { return m_pModule->ServiceInitialized(pService, rtid); }
   btBool  ServiceInitFailed(IBase *pService, IEvent const *pEvent)
   { return m_pModule->ServiceInitFailed(pService, pEvent); }

   btBool          AddToServiceList(IBase *pService) { return m_pModule->AddToServiceList(pService);          }
   btBool     RemovefromServiceList(IBase *pService) { return m_pModule->RemovefromServiceList(pService);     }
   btBool ServiceInstanceRegistered(IBase *pService) { return m_pModule->ServiceInstanceRegistered(pService); }

   Module       *m_pModule;
   Transport    *m_pTransport;
   Marshaller   *m_pMarshaller;
   UnMarshaller *m_pUnMarshaller;
   SB           *m_pSvcBase;
   Factory       m_Factory;
   RT            m_Runtime;
   RTClient      m_RTClient;
   SClient       m_SvcClient;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Factory,  // EmptyISvcsFact, CallTrackingISvcsFact
          unsigned SvcCount,
          typename RT,       // EmptyIRuntime, CallTrackingIRuntime
          typename RTClient, // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename SB,       // EmptyServiceBase, CallTrackingServiceBase
          typename SClient,  // EmptyIServiceClient, CallTrackingIServiceClient
          typename Module=AALServiceModule,
          typename Transport=EmptyIAALTransport,
          typename Marshaller=EmptyIAALMarshaller,
          typename UnMarshaller=EmptyIAALUnMarshaller>
class TAALServiceModule_f_2 : public ::testing::Test
{ // simple fixture
protected:
   TAALServiceModule_f_2() :
      m_pModule(NULL)
   {
      unsigned i;
      for ( i = 0 ; i < SvcCount ; ++i ) {
         m_pTransport[i]    = NULL;
         m_pMarshaller[i]   = NULL;
         m_pUnMarshaller[i] = NULL;
         m_pSvcBase[i]      = NULL;

         m_Runtime[i].getRuntimeProxyReturnsThisValue(&m_Runtime[i]);
         m_Runtime[i].getRuntimeClientReturnsThisValue(&m_RTClient[i]);
      }
   }
   ~TAALServiceModule_f_2() {}

   virtual void SetUp()
   {
      m_pModule = new(std::nothrow) Module(&m_Factory);
      ASSERT_NONNULL(m_pModule);

      EXPECT_TRUE(m_pModule->IsOK());

      unsigned i;
      for ( i = 0 ; i < SvcCount ; ++i ) {
         // allocate, because ~ServiceBase() will delete.
         m_pTransport[i] = new(std::nothrow) Transport();
         ASSERT_NONNULL(m_pTransport[i]);

         // allocate, because ~ServiceBase() will delete.
         m_pMarshaller[i] = new(std::nothrow) Marshaller();
         ASSERT_NONNULL(m_pMarshaller[i]);

         // allocate, because ~ServiceBase() will delete.
         m_pUnMarshaller[i] = new(std::nothrow) UnMarshaller();
         ASSERT_NONNULL(m_pUnMarshaller[i]);

         m_pSvcBase[i] = new(std::nothrow) SB(m_pModule,
                                              &m_Runtime[i],
                                              m_pTransport[i],
                                              m_pMarshaller[i],
                                              m_pUnMarshaller[i]);
         ASSERT_NONNULL(m_pSvcBase[i]);

         m_pSvcBase[i]->ServiceClient(&m_SvcClient[i]);
         m_pSvcBase[i]->ServiceClientBase(dynamic_cast<IBase *>(&m_SvcClient[i]));

         m_pSvcBase[i]->RT(&m_Runtime[i]);
      }
   }
   virtual void TearDown()
   {
      unsigned i;
      for ( i = 0 ; i < SvcCount ; ++i ) {
         if ( NULL != m_pSvcBase[i] ) {
            delete m_pSvcBase[i];
            m_pSvcBase[i] = NULL;
         }
      }

      if ( NULL != m_pModule ) {
         delete m_pModule;
         m_pModule = NULL;
      }
   }

   btBool Construct(IRuntime            *pRuntime,
                    IBase               *Client,
                    TransactionID const &tid,
                    NamedValueSet const &optArgs)
   { return m_pModule->Construct(pRuntime, Client, tid, optArgs); }

   void Destroy() { m_pModule->Destroy(); }

   void      ServiceReleased(IBase *pService) { m_pModule->ServiceReleased(pService); }
   btBool ServiceInitialized(IBase *pService, TransactionID const &rtid)
   { return m_pModule->ServiceInitialized(pService, rtid); }
   btBool  ServiceInitFailed(IBase *pService, IEvent const *pEvent)
   { return m_pModule->ServiceInitFailed(pService, pEvent); }

   btBool          AddToServiceList(IBase *pService) { return m_pModule->AddToServiceList(pService);          }
   btBool     RemovefromServiceList(IBase *pService) { return m_pModule->RemovefromServiceList(pService);     }
   btBool ServiceInstanceRegistered(IBase *pService) { return m_pModule->ServiceInstanceRegistered(pService); }

   Module       *m_pModule;
   Transport    *m_pTransport[SvcCount];
   Marshaller   *m_pMarshaller[SvcCount];
   UnMarshaller *m_pUnMarshaller[SvcCount];
   SB           *m_pSvcBase[SvcCount];
   Factory       m_Factory;
   RT            m_Runtime[SvcCount];
   RTClient      m_RTClient[SvcCount];
   SClient       m_SvcClient[SvcCount];
};

////////////////////////////////////////////////////////////////////////////////

class AALServiceModule_f_2 : public TAALServiceModule_f_1<EmptyISvcsFact,
                                                          CallTrackingIRuntime,
                                                          EmptyIRuntimeClient,
                                                          EmptyServiceBase,
                                                          EmptyIServiceClient>
{
protected:
   AALServiceModule_f_2() {}
   ~AALServiceModule_f_2() {}

   virtual void TearDown()
   {
      m_Runtime.ClearLog();
      TAALServiceModule_f_1<EmptyISvcsFact,
                            CallTrackingIRuntime,
                            EmptyIRuntimeClient,
                            EmptyServiceBase,
                            EmptyIServiceClient>::TearDown();
   }
};

class AALServiceModule_f_3 : public TAALServiceModule_f_1<CallTrackingISvcsFact,
                                                          EmptyIRuntime,
                                                          CallTrackingIRuntimeClient,
                                                          EmptyServiceBase,
                                                          CallTrackingIServiceClient>
{
public:
   AALServiceModule_f_3() {}
   ~AALServiceModule_f_3() {}

   virtual void TearDown()
   {
      m_Factory.ClearLog();
      m_RTClient.ClearLog();
      m_SvcClient.ClearLog();
      TAALServiceModule_f_1<CallTrackingISvcsFact,
                            EmptyIRuntime,
                            CallTrackingIRuntimeClient,
                            EmptyServiceBase,
                            CallTrackingIServiceClient>::TearDown();
   }
};

class AALServiceModule_f_4 : public TAALServiceModule_f_1<EmptyISvcsFact,
                                                          CallTrackingIRuntime,
                                                          EmptyIRuntimeClient,
                                                          CallTrackingServiceBase,
                                                          EmptyIServiceClient>
{
protected:
   AALServiceModule_f_4() {}
   ~AALServiceModule_f_4() {}

   virtual void TearDown()
   {
      m_Runtime.ClearLog();
      if ( NULL != m_pSvcBase ) {
         m_pSvcBase->ClearLog();
      }
      TAALServiceModule_f_1<EmptyISvcsFact,
                            CallTrackingIRuntime,
                            EmptyIRuntimeClient,
                            CallTrackingServiceBase,
                            EmptyIServiceClient>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(AALServiceModule_f_0, aal0673)
{
   // AALServiceModule(ISvcsFact & ) sets an interface of iidServiceProvider/IServiceModule *.

   EXPECT_TRUE(m_pModule->Has(iidServiceProvider));
   EXPECT_EQ(dynamic_cast<IServiceModule *>(m_pModule),
             reinterpret_cast<IServiceModule *>( m_pModule->Interface(iidServiceProvider) ));
}

TEST_F(AALServiceModule_f_1, aal0674)
{
   // AALServiceModule::Construct() requests an AAL Service (returned as an IBase *) from the
   // ISvcsFact provided in the AALServiceModule constructor. When ISvcsFact::CreateServiceObject()
   // is successful, as signified by a non-NULL return value ISvcsFact::InitializeService() is called.

   CAASBase TheService;
   m_Factory.CreateServiceObjectReturnsThisValue(dynamic_cast<IBase *>(&TheService));

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   tid.ID(3);
   NamedValueSet optArgs;
   optArgs.Add((btStringKey)"A", false);

   EXPECT_TRUE(Construct(&irt, &client, tid, optArgs));

   ASSERT_EQ(2, m_Factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", m_Factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   m_Factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(m_pModule), x);

   x = NULL;
   m_Factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);


   EXPECT_STREQ("ISvcsFact::InitializeService", m_Factory.Entry(1).MethodName());

   x = NULL;
   m_Factory.Entry(1).GetParam("Client", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&client), x);

   TransactionID tid2;
   m_Factory.Entry(1).GetParam("rtid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());

   INamedValueSet const *poptArgs = NULL;
   m_Factory.Entry(1).GetParam("optArgs", &poptArgs);
   ASSERT_NONNULL(poptArgs);
   EXPECT_EQ(optArgs, *poptArgs);
}

TEST_F(AALServiceModule_f_1, aal0675)
{
   // During AALServiceModule::Construct(), when ISvcsFact::CreateServiceObject() returns a NULL
   // IBase *, Construct() returns false immediately, indicating the error.

   m_Factory.CreateServiceObjectReturnsThisValue(NULL);

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   NamedValueSet optArgs;

   EXPECT_FALSE(Construct(&irt, &client, tid, optArgs));

   ASSERT_EQ(1, m_Factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", m_Factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   m_Factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(m_pModule), x);

   x = NULL;
   m_Factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);
}

TEST_F(AALServiceModule_f_1, aal0676)
{
   // During AALServiceModule::Construct(), when ISvcsFact::InitializeService() fails,
   // the newly-created service object is passed to ISvcsFact::DestroyServiceObject(),
   // and Construct() returns false indicating the error.

   CAASBase TheService;
   m_Factory.CreateServiceObjectReturnsThisValue(dynamic_cast<IBase *>(&TheService));
   m_Factory.InitializeServiceReturnsThisValue(false);

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   tid.ID(3);
   NamedValueSet optArgs;
   optArgs.Add((btStringKey)"A", false);

   EXPECT_FALSE(Construct(&irt, &client, tid, optArgs));

   ASSERT_EQ(3, m_Factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", m_Factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   m_Factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(m_pModule), x);

   x = NULL;
   m_Factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);


   EXPECT_STREQ("ISvcsFact::InitializeService", m_Factory.Entry(1).MethodName());

   x = NULL;
   m_Factory.Entry(1).GetParam("Client", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&client), x);

   TransactionID tid2;
   m_Factory.Entry(1).GetParam("rtid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());

   INamedValueSet const *poptArgs = NULL;
   m_Factory.Entry(1).GetParam("optArgs", &poptArgs);
   ASSERT_NONNULL(poptArgs);
   EXPECT_EQ(optArgs, *poptArgs);


   EXPECT_STREQ("ISvcsFact::DestroyServiceObject", m_Factory.Entry(2).MethodName());

   x = NULL;
   m_Factory.Entry(2).GetParam("pServiceBase", &x);
   EXPECT_EQ(dynamic_cast<IBase *>(&TheService), reinterpret_cast<IBase *>(x));
}

class aal0677Service : public CallTrackingServiceBase
{
public:
   aal0677Service(AALServiceModule *container,
                  IRuntime         *pAALRUNTIME,
                  IAALTransport    *ptransport,
                  IAALMarshaller   *marshaller,
                  IAALUnMarshaller *unmarshaller) :
      CallTrackingServiceBase(container, pAALRUNTIME, ptransport, marshaller, unmarshaller)
   {}
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT)
   {
      btBool res = CallTrackingServiceBase::Release(rTranID, timeout);

      // For this test case, we automatically invoke Released(), which would otherwise
      // result from the ServiceClientCallback::Released.

      Released();

      return res;
   }
};

class AALServiceModule_f_5 : public TAALServiceModule_f_2<EmptyISvcsFact,
                                                          3,
                                                          CallTrackingIRuntime,
                                                          EmptyIRuntimeClient,
                                                          aal0677Service,
                                                          EmptyIServiceClient>
{
protected:
   AALServiceModule_f_5() {}
   ~AALServiceModule_f_5() {}

   virtual void TearDown()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pSvcBase) / sizeof(m_pSvcBase[0]) ; ++i ) {
         m_Runtime[i].ClearLog();

         if ( NULL != m_pSvcBase[i] ) {
            m_pSvcBase[i]->ClearLog();
         }
      }
      TAALServiceModule_f_2<EmptyISvcsFact,
                            sizeof(m_pSvcBase) / sizeof(m_pSvcBase[0]),
                            CallTrackingIRuntime,
                            EmptyIRuntimeClient,
                            aal0677Service,
                            EmptyIServiceClient>::TearDown();
   }
};

TEST_F(AALServiceModule_f_5, aal0677)
{
   // AALServiceModule::Destroy() releases (via IAALService::Release(TransactionID const & , btTime ))
   // each service being tracked by the AALServiceModule, waiting until each has been released prior
   // to returning. As a result of IAALService::Release(), a
   // ServiceReleased notification is sent to each IServiceClient.

   // A) Make sure we don't hang when there are no services in AALServiceModule::m_serviceList.
   Destroy();


   // B) Some services in the list.

   unsigned i;
   for ( i = 0 ; i < sizeof(m_pSvcBase) / sizeof(m_pSvcBase[0]) ; ++i ) {
      EXPECT_TRUE(AddToServiceList(m_pSvcBase[i]));
      EXPECT_TRUE(ServiceInstanceRegistered(m_pSvcBase[i]));
   }

   Destroy();

   for ( i = 0 ; i < sizeof(m_pSvcBase) / sizeof(m_pSvcBase[0]) ; ++i ) {
      EXPECT_FALSE(ServiceInstanceRegistered(m_pSvcBase[i]));

      ASSERT_LE(1, m_pSvcBase[i]->LogEntries()) << *m_pSvcBase[i];
      EXPECT_STREQ("ServiceBase::Release", m_pSvcBase[i]->Entry(0).MethodName());
/*
IRuntime::getRuntimeClient()
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::schedDispatchable(btObjectType pDisp)
*/
      ASSERT_EQ(3, m_Runtime[i].LogEntries()) << m_Runtime[i];
      EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime[i].Entry(2).MethodName());

      btObjectType x = NULL;
      m_Runtime[i].Entry(2).GetParam("pDisp", &x);

      ASSERT_NONNULL(x);
      IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);

      AAL::ServiceReleased *pCB = dynamic_cast<AAL::ServiceReleased *>(pDisp);
      ASSERT_NONNULL(pCB);

      delete pCB;
   }
}

#if DEPRECATED
TEST(AALServiceModuleTest, aal0678)
{
   // AALServiceModule::getRuntime() retrieves the IRuntime * that
   // AALServiceModule::setRuntime() stores.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_NULL(mod.getRuntime());

   IRuntime *pRt = (IRuntime *)3;

   mod.setRuntime(pRt);
   EXPECT_EQ(pRt, mod.getRuntime());
}
#endif // DEPRECATED

#if DEPRECATED
TEST(AALServiceModuleTest, aal0679)
{
   // AALServiceModule::getRuntimeClient() retrieves the IRuntimeClient *
   // that AALServiceModule::setRuntimeClient() stores.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_NULL(mod.getRuntimeClient());

   IRuntimeClient *pRt = (IRuntimeClient *)4;

   mod.setRuntimeClient(pRt);
   EXPECT_EQ(pRt, mod.getRuntimeClient());
}
#endif // DEPRECATED

TEST_F(AALServiceModule_f_1, aal0719)
{
   // When the IBase * parameter to AALServiceModule::ServiceInitialized() is NULL, the
   // function returns false, indicating the error.

   IBase * const pService = NULL;

   TransactionID tid;
   EXPECT_FALSE(ServiceInitialized(pService, tid));
   EXPECT_FALSE(ServiceInstanceRegistered(pService));
}

TEST_F(AALServiceModule_f_1, aal0720)
{
   // When the IBase * parameter to AALServiceModule::ServiceInitialized() doesn't implement
   // iidServiceBase, the function returns false, indicating the error.

   CAASBase      base; // no iidServiceBase
   TransactionID tid;

   EXPECT_FALSE(ServiceInitialized(&base, tid));
   EXPECT_FALSE(ServiceInstanceRegistered(&base));
}

TEST_F(AALServiceModule_f_2, aal0721)
{
   // When AALServiceModule::ServiceInitialized() is successful, pService is added to the list
   // of services tracked by the module, and a ServiceAllocated notification
   // is sent to pService's IServiceClient.

   TransactionID tid;
   tid.ID(3);

   EXPECT_TRUE(ServiceInitialized(m_pSvcBase, tid));

   EXPECT_TRUE(ServiceInstanceRegistered(m_pSvcBase));
/*
IRuntime::getRuntimeClient()
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::schedDispatchable(btObjectType pDisp)
*/
   ASSERT_EQ(3, m_Runtime.LogEntries()) << m_Runtime;

   EXPECT_STREQ("IRuntime::getRuntimeClient",  m_Runtime.Entry(0).MethodName());
   EXPECT_STREQ("IRuntime::getRuntimeProxy",   m_Runtime.Entry(1).MethodName());
   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(2).MethodName());
   btObjectType x = NULL;
   m_Runtime.Entry(2).GetParam("pDisp", &x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);
   ASSERT_NONNULL(pDisp);

   ServiceAllocated *pCB = dynamic_cast<ServiceAllocated *>(pDisp);
   ASSERT_NONNULL(pCB);

   delete pCB;
}

TEST_F(AALServiceModule_f_1, aal0722)
{
   // When the IBase * parameter to AALServiceModule::ServiceInitFailed() is NULL, the
   // function returns false, indicating the error.

   IBase * const pService = NULL;
   IEvent       *pEvent   = NULL;

   EXPECT_FALSE(ServiceInitFailed(pService, pEvent));
}

TEST_F(AALServiceModule_f_1, aal0723)
{
   // When the IBase * parameter to AALServiceModule::ServiceInitFailed() doesn't implement
   // iidServiceBase, the function returns false, indicating the error.

   CAASBase base; // no iidServiceBase
   IEvent  *pEvent = NULL;

   EXPECT_FALSE(ServiceInitFailed(&base, pEvent));
}

#if DEPRECATED
TEST_F(AALServiceModule_f_3, aal0724)
{
   // When the IBase * parameter to AALServiceModule::ServiceInitFailed() is valid, the same is
   // sent to ISvcsFact::DestroyServiceObject(), and a
   // ServiceClientCallback(ServiceClientCallback::AllocateFailed ...) is sent to the
   // IServiceClient.

   const btUIntPtr threads = GlobalTestConfig::GetInstance().CurrentThreads();

   m_pSvcBase->RT(&m_Runtime);

   EXPECT_TRUE(ServiceInitFailed(m_pSvcBase, NULL));

   ASSERT_EQ(1, m_Factory.LogEntries()) << m_Factory;
   EXPECT_STREQ("ISvcsFact::DestroyServiceObject", m_Factory.Entry(0).MethodName());

   btObjectType x = NULL;
   m_Factory.Entry(0).GetParam("pServiceBase", &x);
   EXPECT_NONNULL(x);
   EXPECT_EQ(reinterpret_cast<IBase *>(x), dynamic_cast<IBase *>(m_pSvcBase));

   YIELD_WHILE( GlobalTestConfig::GetInstance().CurrentThreads() > threads );

   ASSERT_EQ(1, m_SvcClient.LogEntries()) << m_SvcClient;
   EXPECT_STREQ("IServiceClient::serviceAllocateFailed", m_SvcClient.Entry(0).MethodName());

   x = (btObjectType)1;
   m_SvcClient.Entry(0).GetParam("e", &x);
   EXPECT_NULL(x); // This is about all we can do, because the IDispatchable will delete itself.

/*
   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceFailed", m_RTClient.Entry(0).MethodName());

   x = (btObjectType)1;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_NULL(x); // This is about all we can do, because the IDispatchable will delete itself.
*/
}
#endif
TEST_F(AALServiceModule_f_0, EmptyRoutines)
{
   m_pModule->serviceAllocated(NULL, TransactionID());

   CAALEvent *pEvent = new CAALEvent(NULL);
   m_pModule->serviceAllocateFailed(*pEvent);

   m_pModule->serviceReleased(TransactionID());

   m_pModule->serviceReleaseFailed(*pEvent);

   m_pModule->serviceEvent(*pEvent);

   pEvent->Delete();
}

////////////////////////////////////////////////////////////////////////////////

template <typename Factory,      // EmptyISvcsFact, CallTrackingISvcsFact
          typename RT,           // EmptyIRuntime, CallTrackingIRuntime
          typename RTClient,     // EmptyIRuntimeClient, CallTrackingIRuntimeClient
          typename SB,           // EmptyServiceBase, CallTrackingServiceBase
          typename Transport=EmptyIAALTransport,
          typename Marshaller=EmptyIAALMarshaller,
          typename UnMarshaller=EmptyIAALUnMarshaller>
class TServiceBase_f : public ::testing::Test
{ // simple fixture
protected:
   TServiceBase_f() :
      m_Factory(),
      m_Module(&m_Factory),
      m_IRuntime(),
      m_RTClient(),
      m_pTransport(NULL),
      m_pMarshaller(NULL),
      m_pUnMarshaller(NULL),
      m_pSB(NULL)
   {}
   ~TServiceBase_f() {}

   virtual void SetUp()
   {
      m_IRuntime.getRuntimeProxyReturnsThisValue(dynamic_cast<IRuntime *>(&m_IRuntime));
      m_IRuntime.getRuntimeClientReturnsThisValue(&m_RTClient);

      m_pTransport    = new Transport();
      m_pMarshaller   = new Marshaller();
      m_pUnMarshaller = new UnMarshaller();

      m_pSB = new SB(&m_Module,
                     &m_IRuntime,
                     m_pTransport,
                     m_pMarshaller,
                     m_pUnMarshaller);
      EXPECT_TRUE(m_pSB->IsOK());

      EXPECT_EQ(&m_Module, m_pSB->getAALServiceModule());
      EXPECT_EQ(dynamic_cast<IRuntime *>(&m_IRuntime), m_pSB->getRuntime());

      EXPECT_TRUE(m_pSB->HasMarshaller());
      EXPECT_TRUE(m_pSB->HasUnMarshaller());
      EXPECT_TRUE(m_pSB->HasTransport());

      EXPECT_EQ(dynamic_cast<IAALTransport *>(m_pTransport),       &m_pSB->recvr());
      EXPECT_EQ(dynamic_cast<IAALTransport *>(m_pTransport),       &m_pSB->sender());
      EXPECT_EQ(dynamic_cast<IAALMarshaller *>(m_pMarshaller),     &m_pSB->marshall());
      EXPECT_EQ(dynamic_cast<IAALUnMarshaller *>(m_pUnMarshaller), &m_pSB->unmarshall());

      EXPECT_EQ(dynamic_cast<IRuntimeClient *>(&m_RTClient), m_pSB->getRuntimeClient());
      EXPECT_NULL(m_pSB->getServiceClient());
      EXPECT_NULL(m_pSB->getServiceClientBase());
   }
   virtual void TearDown()
   {
      if ( NULL != m_pSB ) {
         delete m_pSB; // also deletes m_pTransport, m_pMarshaller, m_pUnMarshaller
      }
      m_pSB           = NULL;
      m_pTransport    = NULL;
      m_pMarshaller   = NULL;
      m_pUnMarshaller = NULL;
   }

   Factory          m_Factory;
   AALServiceModule m_Module;

   RT               m_IRuntime;
   RTClient         m_RTClient;
   Transport       *m_pTransport;
   Marshaller      *m_pMarshaller;
   UnMarshaller    *m_pUnMarshaller;

   SB              *m_pSB;
};

typedef TServiceBase_f<EmptyISvcsFact,
                       EmptyIRuntime,
                       EmptyIRuntimeClient,
                       EmptyServiceBase> ServiceBase_f_0;

class ServiceBase_f_1 :
   public TServiceBase_f< EmptyISvcsFact, CallTrackingIRuntime, EmptyIRuntimeClient, EmptyServiceBase >
{
protected:
   virtual void TearDown()
   {
      m_IRuntime.ClearLog();
      TServiceBase_f< EmptyISvcsFact, CallTrackingIRuntime, EmptyIRuntimeClient, EmptyServiceBase >::TearDown();
   }
};

class ServiceBase_f_2 :
   public TServiceBase_f< EmptyISvcsFact, EmptyIRuntime, EmptyIRuntimeClient, CallTrackingServiceBase >
{};

class ServiceBase_f_3 :
   public TServiceBase_f< CallTrackingISvcsFact, EmptyIRuntime, EmptyIRuntimeClient, EmptyServiceBase >
{
protected:
   virtual void TearDown()
   {
      m_Factory.ClearLog();
      TServiceBase_f< CallTrackingISvcsFact, EmptyIRuntime, EmptyIRuntimeClient, EmptyServiceBase >::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceBase_f_0, aal0680)
{
   // The ServiceBase constructor sets a SubClass interface of iidService/IAALService * and adds
   // an interface for iidServiceBase/IServiceBase *. The AALServiceModule * that is passed to the
   // constructor is available via pAALServiceModule(), a proxy for the IRuntime * via getRuntime(),
   // the IAALTransport * via recvr() and sender(), the IAALMarshaller via marshall(), and the
   // IAALUnMarshallar via unmarshal().

   EXPECT_TRUE(m_pSB->Has(iidService));
   EXPECT_EQ(dynamic_cast<IAALService *>(m_pSB), m_pSB->Interface(iidService));

#if DEPRECATED
   EXPECT_EQ(iidService, m_pSB->SubClassID());
   EXPECT_EQ(dynamic_cast<IAALService *>(m_pSB), m_pSB->ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(m_pSB->Has(iidServiceBase));
   EXPECT_EQ(dynamic_cast<IServiceBase *>(m_pSB), m_pSB->Interface(iidServiceBase));
}

TEST(ServiceBaseTest, aal0681)
{
   // When the ServiceBase has an IAALTransport, IAALMarshaller, or IAALUnMarshaller, as
   // determined by HasTransport(), HasMarshaller(), and HasUnMarshaller(), respectively,
   // those data members are deleted by the ServiceBase destructor.

   class aal0681Transport : public EmptyIAALTransport
   {
   public:
      aal0681Transport(btBool &b) :
         m_b(b)
      {}
      ~aal0681Transport() { m_b = true; }

   protected:
      btBool &m_b;
   };

   class aal0681Marshaller : public EmptyIAALMarshaller
   {
   public:
      aal0681Marshaller(btBool &b) :
         m_b(b)
      {}
      ~aal0681Marshaller() { m_b = true; }

   protected:
      btBool &m_b;
   };

   class aal0681UnMarshaller : public EmptyIAALUnMarshaller
   {
   public:
      aal0681UnMarshaller(btBool &b) :
         m_b(b)
      {}
      ~aal0681UnMarshaller() { m_b = true; }

   protected:
      btBool &m_b;
   };

   EmptyISvcsFact   factory;
   AALServiceModule mod(&factory);

   EmptyIRuntime    rt;
   rt.getRuntimeProxyReturnsThisValue(dynamic_cast<IRuntime *>(&rt));

   btBool TransportDeleted    = false;
   btBool MarshallerDeleted   = false;
   btBool UnMarshallerDeleted = false;

   // new'ing these because ~ServiceBase delete's them.
   aal0681Transport    *tp      = new aal0681Transport(TransportDeleted);
   aal0681Marshaller   *marsh   = new aal0681Marshaller(MarshallerDeleted);
   aal0681UnMarshaller *unmarsh = new aal0681UnMarshaller(UnMarshallerDeleted);

   EmptyServiceBase *sb = new EmptyServiceBase(&mod, &rt, tp, marsh, unmarsh);

   EXPECT_TRUE(sb->IsOK());

   EXPECT_TRUE(sb->HasTransport());
   EXPECT_TRUE(sb->HasMarshaller());
   EXPECT_TRUE(sb->HasUnMarshaller());

   EXPECT_FALSE(TransportDeleted);
   EXPECT_FALSE(MarshallerDeleted);
   EXPECT_FALSE(UnMarshallerDeleted);

   delete sb;

   EXPECT_TRUE(TransportDeleted);
   EXPECT_TRUE(MarshallerDeleted);
   EXPECT_TRUE(UnMarshallerDeleted);
}

TEST_F(ServiceBase_f_0, aal0682)
{
   // When the IBase * parameter to ServiceBase::_init() is NULL and when the IBase * has no
   // interface for iidServiceClient, false is returned immediately, indicating an error.

   EXPECT_FALSE(m_pSB->_init(NULL, TransactionID(), NamedValueSet()));

   CAASBase base; // (no iidServiceClient)
   EXPECT_FALSE(m_pSB->_init(&base, TransactionID(), NamedValueSet()));
}

TEST_F(ServiceBase_f_1, aal0683)
{
   // When the IBase * parameter to ServiceBase::_init() is non-NULL and has an iidServiceClient,
   // the NamedValueSet parameter is saved for later retrieval by OptArgs() and the object's
   // IRuntimeClient * (as known by getRuntimeClient()) is initialized. If the CAALEvent * is
   // non-NULL, it is dispatched to the object's IRuntime. A true return value indicates success.

   EmptyIServiceClient SvcClient;
   TransactionID       tid;
   NamedValueSet       args;
   args.Add((btNumberKey)2, (btBool)false);

   CAALEvent *e = new CAALEvent(NULL);

   // non-NULL CAALEvent *
   EXPECT_TRUE(m_pSB->_init(dynamic_cast<IBase *>(&SvcClient), tid, args, e));

   btBool x = true;
   m_pSB->OptArgs().Get((btNumberKey)2, &x);
   EXPECT_FALSE(x);

   EXPECT_EQ(&m_RTClient, m_pSB->getRuntimeClient());

   // getRuntimeProxy() is called from the c'tor.
   // getRuntimeClient() is called from _init().
   // schedDispatchable() is called from _init().
   EXPECT_EQ(3, m_IRuntime.LogEntries());
   EXPECT_STREQ("IRuntime::schedDispatchable", m_IRuntime.Entry(2).MethodName());
   btObjectType o = NULL;
   m_IRuntime.Entry(2).GetParam("pDisp", &o);
   EXPECT_EQ(reinterpret_cast<IDispatchable *>(o), dynamic_cast<IDispatchable *>(e));

   e->Delete();
}

TEST_F(ServiceBase_f_2, aal0684)
{
   // When the IBase * parameter to ServiceBase::_init() is non-NULL and has an iidServiceClient,
   // the NamedValueSet parameter is saved for later retrieval by OptArgs() and the object's
   // IRuntimeClient * (as known by getRuntimeClient()) is initialized. If the CAALEvent * is NULL,
   // ServiceBase::init() is called, passing the TransactionID parameter. this is the fn's return
   // value.

   EmptyIServiceClient SvcClient;
   TransactionID       tid;
   tid.ID(3);
   NamedValueSet       args;

   m_pSB->initReturnsThisValue(true);

   // NULL CAALEvent *
   EXPECT_TRUE(m_pSB->_init(dynamic_cast<IBase *>(&SvcClient), tid, args, NULL));

   EXPECT_EQ(&m_RTClient, m_pSB->getRuntimeClient());
/*
0   ServiceBase::getAALServiceModule()
1   ServiceBase::getRuntime()
2   ServiceBase::getRuntimeClient()
3   ServiceBase::getServiceClient()
4   ServiceBase::getServiceClientBase()
5   ServiceBase::_init(btObjectType pclientBase, const TransactionID &rtid, const NamedValueSet &optArgs, btObjectType pcmpltEvent)
6   ServiceBase::init(btObjectType pclientBase, const NamedValueSet &optArgs, const TransactionID &rtid)
7   ServiceBase::getRuntimeClient()
*/
   EXPECT_EQ(8, m_pSB->LogEntries()) << *m_pSB;
   EXPECT_STREQ("ServiceBase::init", m_pSB->Entry(6).MethodName());

   TransactionID tid2;
   m_pSB->Entry(6).GetParam("rtid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());
}

TEST_F(ServiceBase_f_1, aal0685)
{
   // ServiceBase::Release(TransactionID const & , btTime ) dispatches a ServiceClientCallback(Released)
   // event to the service's IRuntime *. The return value is that of IRuntime::schedDispatchable().

   m_IRuntime.schedDispatchableReturnsThisValue(false);

   TransactionID tid;

   EXPECT_FALSE(m_pSB->Release(tid, AAL_INFINITE_WAIT));
/*
IRuntime::getRuntimeClient()
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::schedDispatchable(btObjectType pDisp)
*/
   ASSERT_EQ(3, m_IRuntime.LogEntries()) << m_IRuntime;

   EXPECT_STREQ("IRuntime::schedDispatchable", m_IRuntime.Entry(2).MethodName());
   btObjectType x = NULL;
   m_IRuntime.Entry(2).GetParam("pDisp", &x);

   ASSERT_NONNULL(x);
   delete reinterpret_cast<IDispatchable *>(x);
}

#if DEPRECATED
TEST_F(ServiceBase_f_1, aal0686)
{
   // ServiceBase::Release(btTime ) sends the 'service released' notification to the service
   // module, then calls IRuntime::releaseRuntimeProxy() on m_Runtime. Finally, the ServiceBase
   // is deleted (delete this;), prior to returning true.

   EXPECT_TRUE(m_pSB->Release(AAL_INFINITE_WAIT));
   // guard against double-delete in TearDown()
   m_pSB = NULL;

   // getRuntimeProxy() is called from the c'tor.
   // releaseRuntimeProxy() is called from Release().
   EXPECT_EQ(2, m_IRuntime.LogEntries());
   EXPECT_STREQ("IRuntime::releaseRuntimeProxy1", m_IRuntime.Entry(1).MethodName());
   EXPECT_EQ(0, m_IRuntime.Entry(1).Params());
}
#endif // DEPRECATED

TEST_F(ServiceBase_f_0, aal0687)
{
   // ServiceBase implements IRuntimeClient as do-nothing routines.

   CAALEvent *e = new CAALEvent(NULL);

   m_pSB->runtimeCreateOrGetProxyFailed(*e);
   m_pSB->runtimeStarted(&m_IRuntime, NamedValueSet());
   m_pSB->runtimeStopped(&m_IRuntime);
   m_pSB->runtimeStartFailed(*e);
   m_pSB->runtimeStopFailed(*e);
   m_pSB->runtimeAllocateServiceFailed(*e);
   m_pSB->runtimeAllocateServiceSucceeded(NULL, TransactionID());
   m_pSB->runtimeEvent(*e);

   e->Delete();
}

TEST_F(ServiceBase_f_0, aal0688)
{
   // ServiceBase implements processmsg() and SetParms() as do-nothing routines.

   m_pSB->processmsg();
   EXPECT_FALSE(m_pSB->SetParms(NamedValueSet()));
}

TEST_F(ServiceBase_f_1, aal0689)
{
   // ServiceBase::allocService() requests the service from the cached IRuntime *.

   CAASBase      base;
   NamedValueSet nvs;
   nvs.Add((btNumberKey)5, (btUnsigned32bitInt)123);
   TransactionID tid;
   tid.ID(4);

   m_pSB->allocService(&base, nvs, tid);

   // getRuntimeClient()
   // getRuntimeProxy() is called from the c'tor.
   // allocService() is called from ServiceBase::allocService().
   ASSERT_EQ(3, m_IRuntime.LogEntries()) << m_IRuntime;
   ASSERT_STREQ("IRuntime::allocService", m_IRuntime.Entry(2).MethodName());

   EXPECT_EQ(3, m_IRuntime.Entry(2).Params());

   btObjectType x = NULL;
   m_IRuntime.Entry(2).GetParam("pClient", &x);
   EXPECT_EQ(reinterpret_cast<IBase *>(x), dynamic_cast<IBase *>(&base));

   INamedValueSet const *pNVS = NULL;
   m_IRuntime.Entry(2).GetParam("rManifest", &pNVS);
   ASSERT_NONNULL(pNVS);
   EXPECT_TRUE(nvs == *pNVS);

   TransactionID tid2;
   m_IRuntime.Entry(2).GetParam("rTranID", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());
}

TEST_F(ServiceBase_f_1, aal0690)
{
   // The ServiceBase::initComplete() notification callback passes the TransactionID parameter
   // to AALServiceModule::ServiceInitialized().

   TransactionID tid;
   tid.ID(4);

   m_pSB->initComplete(tid);

   EXPECT_TRUE(m_Module.ServiceInstanceRegistered(m_pSB));
/*
IRuntime::getRuntimeClient()
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::schedDispatchable(btObjectType pDisp)
*/
   EXPECT_EQ(3, m_IRuntime.LogEntries()) << m_IRuntime;

   EXPECT_STREQ("IRuntime::schedDispatchable", m_IRuntime.Entry(2).MethodName());

   btObjectType x = NULL;
   m_IRuntime.Entry(2).GetParam("pDisp", &x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);

   ServiceAllocated *pCB = dynamic_cast<ServiceAllocated *>(pDisp);
   ASSERT_NONNULL(pCB);

   delete pCB;
}

TEST_F(ServiceBase_f_1, aal0725)
{
   // ServiceBase::ReleaseComplete() releases the runtime proxy object, and calls
   // ServiceBase::Released(), which notifies the AALServiceModule via
   // AALServiceModule::ServiceReleased(). The service is then deleted, and the
   // function returns true.

   m_Module.AddToServiceList(m_pSB);

   EXPECT_TRUE(m_Module.ServiceInstanceRegistered(m_pSB));
   EXPECT_TRUE(m_pSB->ReleaseComplete());
   EXPECT_FALSE(m_Module.ServiceInstanceRegistered(m_pSB));
/*
IRuntime::getRuntimeClient()
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::releaseRuntimeProxy()
*/
   ASSERT_EQ(3, m_IRuntime.LogEntries()) << m_IRuntime;

   EXPECT_STREQ("IRuntime::releaseRuntimeProxy", m_IRuntime.Entry(2).MethodName());

   m_pSB = NULL;
}

TEST_F(ServiceBase_f_3, aal0726)
{
   // ServiceBase::initFailed() notifies the AALServiceModule via
   // AALServiceModule::ServiceInitFailed().

   const btUIntPtr threads = GlobalTestConfig::GetInstance().CurrentThreads();

   EXPECT_TRUE(m_pSB->initFailed(NULL));

   YIELD_WHILE( GlobalTestConfig::GetInstance().CurrentThreads() > threads );

   EXPECT_EQ(1, m_Factory.LogEntries());
   EXPECT_STREQ("ISvcsFact::DestroyServiceObject", m_Factory.Entry(0).MethodName());

   btObjectType x = NULL;
   m_Factory.Entry(0).GetParam("pServiceBase", &x);

   EXPECT_EQ(dynamic_cast<IBase *>(m_pSB), reinterpret_cast<IBase *>(x));
}

