// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "swvalsvcmod.h"

#include <aalsdk/aas/ServiceHost.h>
#include <aalsdk/Dispatchables.h>
#include <_ServiceBroker.h>

////////////////////////////////////////////////////////////////////////////////

class Test_ServiceBroker : public _ServiceBroker
{
public:
   Test_ServiceBroker(AALServiceModule *pSvcMod,
                      IRuntime         *pRuntime,
                      IAALTransport    *pTransport,
                      IAALMarshaller   *pMarshaller,
                      IAALUnMarshaller *pUnMarshaller) :
      _ServiceBroker(pSvcMod, pRuntime, pTransport, pMarshaller, pUnMarshaller)
   {}

   ServiceHost * Host(std::string const &sName) { return findServiceHost(sName); }
};

////////////////////////////////////////////////////////////////////////////////

template <typename SvcsFact, // EmptyISvcsFact, CallTrackingISvcsFact
          typename RT,       // EmptyIRuntime, CallTrackingIRuntime
          typename Transport = EmptyIAALTransport,
          typename Marsh     = EmptyIAALMarshaller,
          typename UnMarsh   = EmptyIAALUnMarshaller>
class ServiceBroker_f : public ::testing::Test
{ // simple fixture
public:
   ServiceBroker_f() :
      m_pSvcBroker(NULL),
      m_pTransport(NULL),
      m_pMarshaller(NULL),
      m_pUnMarshaller(NULL),
      m_SvcModule(m_SvcsFactory)
   {}

   virtual void SetUp()
   {
      // We allocate m_pTransport, m_pMarshaller, and m_pUnMarshaller because ~ServiceBase()
      // deletes them.
      m_pTransport = new(std::nothrow) Transport();
      ASSERT_NONNULL(m_pTransport);

      m_pMarshaller = new(std::nothrow) Marsh();
      ASSERT_NONNULL(m_pMarshaller);

      m_pUnMarshaller = new (std::nothrow) UnMarsh();
      ASSERT_NONNULL(m_pUnMarshaller);

      m_Runtime.getRuntimeProxyReturnsThisValue(&m_Runtime);

      m_pSvcBroker = new(std::nothrow) Test_ServiceBroker(&m_SvcModule,
                                                          &m_Runtime,
                                                          m_pTransport,
                                                          m_pMarshaller,
                                                          m_pUnMarshaller);
      ASSERT_NONNULL(m_pSvcBroker);
   }
   virtual void TearDown()
   {
      if ( NULL != m_pSvcBroker ) {
         delete m_pSvcBroker;
      }
   }

   Test_ServiceBroker *m_pSvcBroker;
   Transport          *m_pTransport;
   Marsh              *m_pMarshaller;
   UnMarsh            *m_pUnMarshaller;
   AALServiceModule    m_SvcModule;
   SvcsFact            m_SvcsFactory;
   RT                  m_Runtime;
};

typedef ServiceBroker_f<EmptyISvcsFact, EmptyIRuntime> ServiceBroker_f_0;

class ServiceBroker_f_1 : public ServiceBroker_f<EmptyISvcsFact, CallTrackingIRuntime>
{
protected:
   ServiceBroker_f_1() {}

   virtual void TearDown()
   {
      m_Runtime.ClearLog();
      ServiceBroker_f<EmptyISvcsFact, CallTrackingIRuntime>::TearDown();
   }
};

class ServiceBroker_f_2 : public ServiceBroker_f<CallTrackingISvcsFact, CallTrackingIRuntime>
{
protected:
   ServiceBroker_f_2() {}

   virtual void TearDown()
   {
      m_SvcsFactory.ClearLog();
      m_Runtime.ClearLog();
      ServiceBroker_f<CallTrackingISvcsFact, CallTrackingIRuntime>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceBroker_f_0, aal0709)
{
   // When successful, _ServiceBroker::_ServiceBroker() sets an interface of
   // iidServiceBroker / IServiceBroker *. _ServiceBroker::IsOK() returns true, indicating success.

#if DEPRECATED
   EXPECT_EQ(iidServiceBroker, m_pSvcBroker->SubClassID());
   EXPECT_EQ(dynamic_cast<IServiceBroker *>(m_pSvcBroker), m_pSvcBroker->ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(m_pSvcBroker->Has(iidServiceBroker));
   EXPECT_EQ(dynamic_cast<IServiceBroker *>(m_pSvcBroker), m_pSvcBroker->Interface(iidServiceBroker));

   EXPECT_TRUE(m_pSvcBroker->IsOK());
}

TEST_F(ServiceBroker_f_1, aal0710)
{
   // _ServiceBroker::init() call IServiceBase::initComplete(), passing the given TransactionID.
   // As a result, IServiceModuleCallback::ServiceInitialized() is called, which dispatches a
   // ServiceClientCallback(ServiceClientCallback::Allocated ...) to the IServiceClient.

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIServiceClient SvcClient;
   NamedValueSet nvs;
   TransactionID tid;
   tid.ID(4);

   EXPECT_TRUE(m_pSvcBroker->init(&SvcClient, nvs, tid));

   EXPECT_EQ(2, m_Runtime.LogEntries());
   EXPECT_STREQ("IRuntime::getRuntimeProxy", m_Runtime.Entry(0).MethodName());

   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(1).MethodName());

   btObjectType x = NULL;
   m_Runtime.Entry(1).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);

   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

   //delete pCB;
   pCB->operator() ();
}

TEST_F(ServiceBroker_f_1, aal0711)
{
   // When the pServiceClientBase parameter to _ServiceBroker::allocService() does not
   // implement iidServiceClient, allocService() dispatches a
   // ServiceClientCallback(ServiceClientCallback::AllocateFailed ...) and returns immediately.

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIRuntimeClient rtc;
   CAASBase            base;
   NamedValueSet       manifest;
   TransactionID       tid;
   tid.ID(4);

   m_pSvcBroker->allocService(&m_Runtime,
                              &rtc,
                              &base, // doesn't implement iidServiceClient.
                              manifest,
                              tid);

   EXPECT_EQ(2, m_Runtime.LogEntries());
   EXPECT_STREQ("IRuntime::getRuntimeProxy", m_Runtime.Entry(0).MethodName());

   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(1).MethodName());
   btObjectType x = NULL;
   m_Runtime.Entry(1).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);
   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

   //delete pCB;
   pCB->operator() ();
}

TEST_F(ServiceBroker_f_1, aal0712)
{
   // When the NamedValueSet manifest passed to _ServiceBroker::allocService() has no
   // AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED key, allocService() dispatches a
   // ServiceClientCallback(ServiceClientCallback::AllocateFailed ...) and returns immediately.

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIRuntimeClient rtc;
   EmptyIServiceClient SvcClient;
   NamedValueSet       manifest; // empty (no config record)
   TransactionID       tid;
   tid.ID(5);

   m_pSvcBroker->allocService(&m_Runtime,
                              &rtc,
                              &SvcClient,
                              manifest,
                              tid);

   EXPECT_EQ(2, m_Runtime.LogEntries());
   EXPECT_STREQ("IRuntime::getRuntimeProxy", m_Runtime.Entry(0).MethodName());

   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(1).MethodName());
   btObjectType x = NULL;
   m_Runtime.Entry(1).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);
   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

//   delete pCB;

   pCB->operator() ();
}

TEST_F(ServiceBroker_f_1, aal0713)
{
   // When the NamedValueSet manifest passed to _ServiceBroker::allocService() has an
   // AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED key, but the associated config record has
   // no AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME key, allocService() dispatches a
   // ServiceClientCallback(ServiceClientCallback::AllocateFailed ...) and returns immediately.

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIRuntimeClient rtc;
   EmptyIServiceClient SvcClient;

   NamedValueSet       manifest;
   NamedValueSet       configrecord; // empty (no service name)
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrecord);

   TransactionID       tid;
   tid.ID(6);

   m_pSvcBroker->allocService(&m_Runtime,
                              &rtc,
                              &SvcClient,
                              manifest,
                              tid);

   EXPECT_EQ(2, m_Runtime.LogEntries());
   EXPECT_STREQ("IRuntime::getRuntimeProxy", m_Runtime.Entry(0).MethodName());

   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(1).MethodName());
   btObjectType x = NULL;
   m_Runtime.Entry(1).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);

   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

   //delete pCB;
   pCB->operator() ();
}

TEST_F(ServiceBroker_f_1, aal0714)
{
   // When the required manifest, complete with config record and service name are given,
   // _ServiceBroker::allocService() creates a ServiceHost for the given service. If the
   // given service could not be loaded, allocService() deletes the ServiceHost, then dispatches
   // a ServiceClientCallback(ServiceClientCallback::AllocateFailed ...) and returns.

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIRuntimeClient rtc;
   EmptyIServiceClient SvcClient;

   NamedValueSet       manifest;
   NamedValueSet       configrecord;

   configrecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libThisServiceNotFound");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrecord);

   TransactionID       tid;
   tid.ID(7);

   m_pSvcBroker->allocService(&m_Runtime,
                              &rtc,
                              &SvcClient,
                              manifest,
                              tid);

   EXPECT_EQ(2, m_Runtime.LogEntries());
   EXPECT_STREQ("IRuntime::getRuntimeProxy", m_Runtime.Entry(0).MethodName());

   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(1).MethodName());
   btObjectType x = NULL;
   m_Runtime.Entry(1).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);

   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

   //delete pCB;
   pCB->operator() ();
}

TEST_F(ServiceBroker_f_2, aal0715)
{
   // When the required manifest, complete with config record and service name are given,
   // _ServiceBroker::allocService() creates a ServiceHost for the given service. When the
   // given service is loaded successfully, allocService() maps the service name to its
   // service host object, and invokes ServiceHost::InstantiateService() on the object.

   class aal0715SvcClient : public ISwvalSvcClient,
                            public CAASBase
   {
   public:
      aal0715SvcClient()
      {
         if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ) {
            m_bIsOK = false;
            return;
         }
         if ( EObjOK != SetInterface(iidSwvalSvcClient, dynamic_cast<ISwvalSvcClient *>(this)) ) {
            m_bIsOK = false;
            return;
         }
      }

      virtual void DidSomething(const AAL::TransactionID & , int ) {}

      // <IServiceClient>
      virtual void      serviceAllocated(AAL::IBase               *pServiceBase,
                                         AAL::TransactionID const &rTranID = AAL::TransactionID())
      {
         ASSERT_NONNULL(pServiceBase);
         delete pServiceBase;
      }
      virtual void serviceAllocateFailed(const AAL::IEvent &rEvent)                                {}
      virtual void       serviceReleased(AAL::TransactionID const &rTranID = AAL::TransactionID()) {}
      virtual void  serviceReleaseFailed(const AAL::IEvent &rEvent)                                {}
      virtual void          serviceEvent(const AAL::IEvent &rEvent)                                {}
      // </IServiceClient>
   };

   EXPECT_TRUE(m_pSvcBroker->IsOK());

   EmptyIRuntimeClient rtc;
   aal0715SvcClient    SvcClient;

   NamedValueSet       manifest;
   NamedValueSet       configrecord;

   configrecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libswvalsvcmod");
   manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configrecord);

   TransactionID       tid;
   tid.ID(8);

   m_Runtime.getRuntimeClientReturnsThisValue(&rtc);

   m_pSvcBroker->allocService(&m_Runtime,
                              &rtc,
                              &SvcClient,
                              manifest,
                              tid);

   EXPECT_EQ(4, m_Runtime.LogEntries()) << m_Runtime;
/*
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::getRuntimeProxy(btObjectType pClient)
IRuntime::getRuntimeClient()
IRuntime::schedDispatchable(btObjectType pDisp)
*/
   EXPECT_STREQ("IRuntime::schedDispatchable", m_Runtime.Entry(3).MethodName());

   btObjectType x = NULL;
   m_Runtime.Entry(3).GetParam("pDisp", &x);
   ASSERT_NONNULL(x);

   IDispatchable *pDisp = reinterpret_cast<IDispatchable *>(x);
   ServiceClientCallback *pCB = dynamic_cast<ServiceClientCallback *>(pDisp);
   ASSERT_NONNULL(pCB);

   //delete pCB;
   pCB->operator() ();

   ServiceHost *pSvcHost = m_pSvcBroker->Host("libswvalsvcmod");
   ASSERT_NONNULL(pSvcHost);
   delete pSvcHost;
}
