// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/aas/Dispatchables.h>

////////////////////////////////////////////////////////////////////////////////

template <typename Disp>
class TRuntimeDispatchables_f_0 : public ::testing::Test
{ // simple fixture
public:
   TRuntimeDispatchables_f_0() :
      m_pRTDisp(NULL),
      m_pEvent(NULL)
   {}

   virtual void SetUp()
   {
      m_pEvent = new(std::nothrow) CExceptionTransactionEvent(&m_Runtime,
                                                              extranevtRuntimeCreateorProxy,
                                                              TransactionID(),
                                                              errSysSystemStarted,
                                                              reasInitError,
                                                              "RuntimeDispatchables_f_0");
      ASSERT_NONNULL(m_pEvent);

      m_pRTDisp = new(std::nothrow) Disp(&m_RTClient, m_pEvent);
      ASSERT_NONNULL(m_pRTDisp);
   }

   virtual void TearDown()
   {
      m_RTClient.ClearLog();
   }

protected:
   Disp                      *m_pRTDisp;
   const IEvent              *m_pEvent;
   EmptyIRuntime              m_Runtime;
   CallTrackingIRuntimeClient m_RTClient;
};

typedef TRuntimeDispatchables_f_0<RuntimeCreateOrGetProxyFailed> RuntimeDispatchables_f_0;

template <typename Disp>
class TRuntimeDispatchables_f_1 : public ::testing::Test
{ // simple fixture
public:
   TRuntimeDispatchables_f_1() :
      m_pRTDisp(NULL)
   {}

   virtual void SetUp()
   {
      m_ConfigParms.Add((btNumberKey)3, (btByte)5);

      m_pRTDisp = new(std::nothrow) Disp(&m_RTClient, &m_Runtime, m_ConfigParms);
      ASSERT_NONNULL(m_pRTDisp);
   }

   virtual void TearDown()
   {
      m_RTClient.ClearLog();
   }

protected:
   Disp                      *m_pRTDisp;
   EmptyIRuntime              m_Runtime;
   CallTrackingIRuntimeClient m_RTClient;
   NamedValueSet              m_ConfigParms;
};

typedef TRuntimeDispatchables_f_1<RuntimeStarted> RuntimeDispatchables_f_1;

typedef TRuntimeDispatchables_f_0<RuntimeStartFailed> RuntimeDispatchables_f_2;

template <typename Disp>
class TRuntimeDispatchables_f_2 : public ::testing::Test
{ // simple fixture
public:
   TRuntimeDispatchables_f_2() :
      m_pRTDisp(NULL)
   {}

   virtual void SetUp()
   {
      m_pRTDisp = new(std::nothrow) Disp(&m_RTClient, &m_Runtime);
      ASSERT_NONNULL(m_pRTDisp);
   }

   virtual void TearDown()
   {
      m_RTClient.ClearLog();
   }

protected:
   Disp                      *m_pRTDisp;
   const IEvent              *m_pEvent;
   EmptyIRuntime              m_Runtime;
   CallTrackingIRuntimeClient m_RTClient;
};

typedef TRuntimeDispatchables_f_2<RuntimeStopped> RuntimeDispatchables_f_3;

typedef TRuntimeDispatchables_f_0<RuntimeStopFailed> RuntimeDispatchables_f_4;

template <typename Disp>
class TRuntimeDispatchables_f_3 : public ::testing::Test
{ // simple fixture
public:
   TRuntimeDispatchables_f_3() :
      m_pRTDisp(NULL)
   {}

   virtual void SetUp()
   {
      m_TranID.ID(3);

      m_pRTDisp = new(std::nothrow) Disp(&m_RTClient, &m_CAASBase, m_TranID);
      ASSERT_NONNULL(m_pRTDisp);
   }

   virtual void TearDown()
   {
      m_RTClient.ClearLog();
   }

protected:
   Disp                      *m_pRTDisp;
   EmptyIRuntime              m_Runtime;
   CallTrackingIRuntimeClient m_RTClient;
   CAASBase                   m_CAASBase;
   TransactionID              m_TranID;
};

typedef TRuntimeDispatchables_f_3<RuntimeAllocateServiceSucceeded> RuntimeDispatchables_f_5;

typedef TRuntimeDispatchables_f_0<RuntimeAllocateServiceFailed>    RuntimeDispatchables_f_6;
typedef TRuntimeDispatchables_f_0<RuntimeEvent>                    RuntimeDispatchables_f_7;

////////////////////////////////////////////////////////////////////////////////

TEST_F(RuntimeDispatchables_f_0, aal0734)
{
   // Dispatchable RuntimeCreateOrGetProxyFailed calls
   // IRuntimeClient::runtimeCreateOrGetProxyFailed() for the given runtime client.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeCreateOrGetProxyFailed", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_EQ(reinterpret_cast<IEvent *>(x), m_pEvent);
}

TEST_F(RuntimeDispatchables_f_1, aal0735)
{
   // Dispatchable RuntimeStarted calls IRuntimeClient::runtimeStarted() for the given
   // runtime client, runtime, and NamedValueSet.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStarted", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("pRuntime", &x);

   ASSERT_NONNULL(x);
   IRuntime *pRT = reinterpret_cast<IRuntime *>(x);

   EXPECT_EQ(pRT, dynamic_cast<IRuntime *>(&m_Runtime));

   INamedValueSet const *pNVS = NULL;
   m_RTClient.Entry(0).GetParam("nvs", &pNVS);
   ASSERT_NONNULL(pNVS);

   btByte b = 0;
   EXPECT_EQ(ENamedValuesOK, pNVS->Get((btNumberKey)3, &b));
   EXPECT_EQ((btByte)5, b);
}

TEST_F(RuntimeDispatchables_f_2, aal0736)
{
   // Dispatchable RuntimeStartFailed calls IRuntimeClient::runtimeStartFailed() for the given
   // runtime client.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStartFailed", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_EQ(reinterpret_cast<IEvent *>(x), m_pEvent);
}

TEST_F(RuntimeDispatchables_f_3, aal0737)
{
   // Dispatchable RuntimeStopped calls IRuntimeClient::runtimeStopped() for the given runtime
   // client and runtime.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStopped", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<IRuntime *>(x), dynamic_cast<IRuntime *>(&m_Runtime));
}

TEST_F(RuntimeDispatchables_f_4, aal0738)
{
   // Dispatchable RuntimeStopFailed calls IRuntimeClient::runtimeStopFailed() for the given
   // runtime client.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeStopFailed", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_EQ(reinterpret_cast<IEvent *>(x), m_pEvent);
}

TEST_F(RuntimeDispatchables_f_5, aal0739)
{
   // Dispatchable RuntimeAllocateServiceSucceeded calls
   // IRuntimeClient::runtimeAllocateServiceSucceeded() for the given runtime client,
   // service base, and TransactionID.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("pServiceBase", &x);
   EXPECT_EQ(reinterpret_cast<IBase *>(x), dynamic_cast<IBase *>(&m_CAASBase));

   TransactionID tid;
   m_RTClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(3, tid.ID());
}

TEST_F(RuntimeDispatchables_f_6, aal0740)
{
   // Dispatchable RuntimeAllocateServiceFailed calls IRuntimeClient::runtimeAllocateServiceFailed()
   // for the given runtime client.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceFailed", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_EQ(reinterpret_cast<IEvent *>(x), m_pEvent);
}

TEST_F(RuntimeDispatchables_f_7, aal0741)
{
   // Dispatchable RuntimeEvent calls IRuntimeClient::runtimeEvent() for the given runtime client.

   m_pRTDisp->operator() ();

   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", m_RTClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   EXPECT_EQ(reinterpret_cast<IEvent *>(x), m_pEvent);
}

////////////////////////////////////////////////////////////////////////////////

TEST(DispatchableGroup, aal0742)
{
   // The DispatchableGroup c'tor that takes 1 IDispatchable * executes that IDispatchable from
   // its operator()().

   class LightSwitch : public IDispatchable
   {
   public:
      LightSwitch(btBool &Switch) :
         m_Switch(Switch)
      {}

      virtual void operator() ()
      {
         m_Switch = !m_Switch;
         delete this;
      }

   protected:
      btBool &m_Switch;
   };

   btBool b = true;
   DispatchableGroup *pGrp = new DispatchableGroup( new LightSwitch(b) );

   EXPECT_TRUE(b);
   pGrp->operator() ();
   EXPECT_FALSE(b);
}

TEST(DispatchableGroup, aal0743)
{
   // The DispatchableGroup c'tor that takes 2 IDispatchable *'s executes those IDispatchable from
   // its operator()() in the order in which they were provided.

   class LightSwitchID : public IDispatchable
   {
   public:
      LightSwitchID(btBool &Switch, btInt &SharedID, btInt MyID) :
         m_Switch(Switch),
         m_SharedID(SharedID),
         m_MyID(MyID)
      {}

      virtual void operator() ()
      {
         m_Switch   = !m_Switch;
         m_SharedID = m_MyID;
         delete this;
      }

   protected:
      btBool &m_Switch;
      btInt  &m_SharedID;
      btInt   m_MyID;
   };


   btBool b[2] = { true, true };
   btInt  id   = 0;

   DispatchableGroup *pGrp = new DispatchableGroup(new LightSwitchID(b[0], id, 1),
                                                   new LightSwitchID(b[1], id, 2)
                                                  );

   EXPECT_TRUE(b[0]);
   EXPECT_TRUE(b[1]);

   pGrp->operator() ();

   EXPECT_FALSE(b[0]);
   EXPECT_FALSE(b[1]);

   EXPECT_EQ(2, id);
}

TEST(DispatchableGroup, aal0744)
{
   // The DispatchableGroup c'tor that takes 3 IDispatchable *'s executes those IDispatchable from
   // its operator()() in the order in which they were provided.

   class LightSwitchID : public IDispatchable
   {
   public:
      LightSwitchID(btBool &Switch, btInt &SharedID, btInt MyID) :
         m_Switch(Switch),
         m_SharedID(SharedID),
         m_MyID(MyID)
      {}

      virtual void operator() ()
      {
         m_Switch   = !m_Switch;
         m_SharedID = m_MyID;
         delete this;
      }

   protected:
      btBool &m_Switch;
      btInt  &m_SharedID;
      btInt   m_MyID;
   };


   btBool b[3] = { true, true, true };
   btInt  id   = 0;

   DispatchableGroup *pGrp = new DispatchableGroup(new LightSwitchID(b[0], id, 1),
                                                   new LightSwitchID(b[1], id, 2),
                                                   new LightSwitchID(b[2], id, 3)
                                                  );

   EXPECT_TRUE(b[0]);
   EXPECT_TRUE(b[1]);
   EXPECT_TRUE(b[2]);

   pGrp->operator() ();

   EXPECT_FALSE(b[0]);
   EXPECT_FALSE(b[1]);
   EXPECT_FALSE(b[2]);

   EXPECT_EQ(3, id);
}

TEST(DispatchableGroup, aal0745)
{
   // The DispatchableGroup c'tor that takes 4 IDispatchable *'s executes those IDispatchable from
   // its operator()() in the order in which they were provided.

   class LightSwitchID : public IDispatchable
   {
   public:
      LightSwitchID(btBool &Switch, btInt &SharedID, btInt MyID) :
         m_Switch(Switch),
         m_SharedID(SharedID),
         m_MyID(MyID)
      {}

      virtual void operator() ()
      {
         m_Switch   = !m_Switch;
         m_SharedID = m_MyID;
         delete this;
      }

   protected:
      btBool &m_Switch;
      btInt  &m_SharedID;
      btInt   m_MyID;
   };


   btBool b[4] = { true, true, true, true };
   btInt  id   = 0;

   DispatchableGroup *pGrp = new DispatchableGroup(new LightSwitchID(b[0], id, 1),
                                                   new LightSwitchID(b[1], id, 2),
                                                   new LightSwitchID(b[2], id, 3),
                                                   new LightSwitchID(b[3], id, 4)
                                                  );

   EXPECT_TRUE(b[0]);
   EXPECT_TRUE(b[1]);
   EXPECT_TRUE(b[2]);
   EXPECT_TRUE(b[3]);

   pGrp->operator() ();

   EXPECT_FALSE(b[0]);
   EXPECT_FALSE(b[1]);
   EXPECT_FALSE(b[2]);
   EXPECT_FALSE(b[3]);

   EXPECT_EQ(4, id);
}

TEST(DispatchableGroup, aal0746)
{
   // The DispatchableGroup c'tor that takes 5 IDispatchable *'s executes those IDispatchable from
   // its operator()() in the order in which they were provided.

   class LightSwitchID : public IDispatchable
   {
   public:
      LightSwitchID(btBool &Switch, btInt &SharedID, btInt MyID) :
         m_Switch(Switch),
         m_SharedID(SharedID),
         m_MyID(MyID)
      {}

      virtual void operator() ()
      {
         m_Switch   = !m_Switch;
         m_SharedID = m_MyID;
         delete this;
      }

   protected:
      btBool &m_Switch;
      btInt  &m_SharedID;
      btInt   m_MyID;
   };


   btBool b[5] = { true, true, true, true, true };
   btInt  id   = 0;

   DispatchableGroup *pGrp = new DispatchableGroup(new LightSwitchID(b[0], id, 1),
                                                   new LightSwitchID(b[1], id, 2),
                                                   new LightSwitchID(b[2], id, 3),
                                                   new LightSwitchID(b[3], id, 4),
                                                   new LightSwitchID(b[4], id, 5)
                                                  );

   EXPECT_TRUE(b[0]);
   EXPECT_TRUE(b[1]);
   EXPECT_TRUE(b[2]);
   EXPECT_TRUE(b[3]);
   EXPECT_TRUE(b[4]);

   pGrp->operator() ();

   EXPECT_FALSE(b[0]);
   EXPECT_FALSE(b[1]);
   EXPECT_FALSE(b[2]);
   EXPECT_FALSE(b[3]);
   EXPECT_FALSE(b[4]);

   EXPECT_EQ(5, id);
}

////////////////////////////////////////////////////////////////////////////////

class DispTestService : public CallTrackingIServiceBase
{
public:
   DispTestService(IServiceClient *pSvcClient=NULL,
                   IRuntimeClient *pRTClient=NULL)
   {
      if ( NULL != pSvcClient ) {
         SetInterface(iidServiceClient, pSvcClient);
      }
      if ( NULL != pRTClient ) {
         SetInterface(iidRuntimeClient, pRTClient);
      }
   }
};

template <typename SvcClient, // EmptyIServiceClient, CallTrackingIServiceClient
          typename RTClient>  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
class TServiceAllocated_f_0 : public ::testing::Test
{ // simple fixture
public:
   TServiceAllocated_f_0() :
      m_pDisp(NULL),
      m_Service(NULL, NULL)
   {}

   virtual void SetUp()
   {
      TransactionID tid;
      tid.ID(3);

      tid.Filter(true); // filter has no effect because Ibase() is NULL.

      m_pDisp = new(std::nothrow) ServiceAllocated(&m_SvcClient,
                                                   &m_RTClient,
                                                   &m_Service,
                                                   tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceAllocated *m_pDisp;
   DispTestService   m_Service;
   SvcClient         m_SvcClient;
   RTClient          m_RTClient;
};

class ServiceAllocated_f_0 : public TServiceAllocated_f_0<CallTrackingIServiceClient,
                                                          CallTrackingIRuntimeClient>
{
protected:
   ServiceAllocated_f_0() {}
   virtual ~ServiceAllocated_f_0() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_RTClient.ClearLog();
      TServiceAllocated_f_0<CallTrackingIServiceClient,
                            CallTrackingIRuntimeClient>::TearDown();
   }
};


template <typename SvcClient, // EmptyIServiceClient, CallTrackingIServiceClient
          typename RTClient>  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
class TServiceAllocated_f_1 : public ::testing::Test
{ // simple fixture
public:
   TServiceAllocated_f_1() :
      m_pDisp(NULL),
      m_Service(&m_TIDSvcClient, &m_TIDRTClient)
   {}

   virtual void SetUp()
   {
      TransactionID tid(4, &m_Service);

      tid.Filter(false);

      m_pDisp = new(std::nothrow) ServiceAllocated(&m_SvcClient,
                                                   &m_RTClient,
                                                   &m_Service,
                                                   tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceAllocated *m_pDisp;
   DispTestService   m_Service;
   SvcClient         m_SvcClient;
   RTClient          m_RTClient;
   SvcClient         m_TIDSvcClient;
   RTClient          m_TIDRTClient;
};

class ServiceAllocated_f_1 : public TServiceAllocated_f_1<CallTrackingIServiceClient,
                                                          CallTrackingIRuntimeClient>
{
protected:
   ServiceAllocated_f_1() {}
   virtual ~ServiceAllocated_f_1() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_RTClient.ClearLog();
      m_TIDSvcClient.ClearLog();
      m_TIDRTClient.ClearLog();
      TServiceAllocated_f_1<CallTrackingIServiceClient,
                            CallTrackingIRuntimeClient>::TearDown();
   }
};


template <typename SvcClient, // EmptyIServiceClient, CallTrackingIServiceClient
          typename RTClient>  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
class TServiceAllocated_f_2 : public ::testing::Test
{ // simple fixture
public:
   TServiceAllocated_f_2() :
      m_pDisp(NULL),
      m_Service(&m_TIDSvcClient, &m_TIDRTClient)
   {}

   virtual void SetUp()
   {
      TransactionID tid(5, &m_Service);

      tid.Filter(true);

      m_pDisp = new(std::nothrow) ServiceAllocated(&m_SvcClient,
                                                   &m_RTClient,
                                                   &m_Service,
                                                   tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceAllocated *m_pDisp;
   DispTestService   m_Service;
   SvcClient         m_SvcClient;
   RTClient          m_RTClient;
   SvcClient         m_TIDSvcClient;
   RTClient          m_TIDRTClient;
};

class ServiceAllocated_f_2 : public TServiceAllocated_f_2<CallTrackingIServiceClient,
                                                          CallTrackingIRuntimeClient>
{
protected:
   ServiceAllocated_f_2() {}
   virtual ~ServiceAllocated_f_2() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_RTClient.ClearLog();
      m_TIDSvcClient.ClearLog();
      m_TIDRTClient.ClearLog();
      TServiceAllocated_f_2<CallTrackingIServiceClient,
                            CallTrackingIRuntimeClient>::TearDown();
   }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceAllocated_f_0, aal0752)
{
   // When the TransactionID parameter to ServiceAllocated has no IBase * (as reported by
   // TransactionID::Ibase()) ServiceAllocated::operator() invokes IServiceClient::serviceAllocated() on
   // a non-NULL IServiceClient and IRuntimeClient::runtimeAllocateServiceSucceeded() on a non-NULL
   // IRuntimeClient.

   const bt32bitInt id = 3;

   m_pDisp->operator() ();


   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocated", m_SvcClient.Entry(0).MethodName());

   btObjectType x = NULL;
   m_SvcClient.Entry(0).GetParam("pBase", &x);
   ASSERT_NONNULL(x);

   IBase *pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   TransactionID tid;
   m_SvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_RTClient.Entry(0).MethodName());

   x = NULL;
   m_RTClient.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_RTClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());
}

TEST_F(ServiceAllocated_f_1, aal0753)
{
   // When the TransactionID parameter to ServiceAllocated has a non-NULL IBase * (as reported by
   // TransactionID::Ibase()) ServiceAllocated::operator() () queries the IBase for iidServiceClient
   // and iidRuntimeClient. When present, operator() invokes IServiceClient::serviceAllocated() on
   // the iidServiceClient found in the TransactionID's IBase. When present, operator() invokes
   // IRuntimeClient::runtimeAllocateServiceSucceeded() on the iidRuntimeClient found in the
   // TransactionID's IBase. If the TransactionID's filter flag (as reported by TransactionID::Filter())
   // is clear, ServiceAllocated::operator() () also invokes IServiceClient::serviceAllocated() on
   // a non-NULL IServiceClient passed to the c'tor and IRuntimeClient::runtimeAllocateServiceSucceeded()
   // on a non-NULL IRuntimeClient passed to the c'tor.

   const bt32bitInt id = 4;

   m_pDisp->operator() ();

   btObjectType  x = NULL;
   TransactionID tid;
   IBase        *pSvc;

   ASSERT_EQ(1, m_TIDSvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocated", m_TIDSvcClient.Entry(0).MethodName());


   m_TIDSvcClient.Entry(0).GetParam("pBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_TIDSvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_TIDRTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_TIDRTClient.Entry(0).MethodName());

   x = NULL;
   m_TIDRTClient.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_TIDRTClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocated", m_SvcClient.Entry(0).MethodName());

   x = NULL;
   m_SvcClient.Entry(0).GetParam("pBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_SvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_RTClient.Entry(0).MethodName());

   x = NULL;
   m_RTClient.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_RTClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());
}

TEST_F(ServiceAllocated_f_2, aal0754)
{
   // When the TransactionID parameter to ServiceAllocated has a non-NULL IBase * (as reported by
   // TransactionID::Ibase()) ServiceAllocated::operator() () queries the IBase for iidServiceClient
   // and iidRuntimeClient. When present, operator() invokes IServiceClient::serviceAllocated() on
   // the iidServiceClient found in the TransactionID's IBase. When present, operator() invokes
   // IRuntimeClient::runtimeAllocateServiceSucceeded() on the iidRuntimeClient found in the
   // TransactionID's IBase. If the TransactionID's filter flag (as reported by TransactionID::Filter())
   // is set, the IServiceClient * and IRuntimeClient * passed to the ServiceAllocated c'tor are
   // ignored.

   const bt32bitInt id = 5;

   m_pDisp->operator() ();

   btObjectType  x = NULL;
   TransactionID tid;
   IBase        *pSvc;

   ASSERT_EQ(1, m_TIDSvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocated", m_TIDSvcClient.Entry(0).MethodName());


   m_TIDSvcClient.Entry(0).GetParam("pBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_TIDSvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_TIDRTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", m_TIDRTClient.Entry(0).MethodName());

   x = NULL;
   m_TIDRTClient.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_NONNULL(x);

   pSvc = reinterpret_cast<IBase *>(x);
   EXPECT_EQ(dynamic_cast<IBase *>(&m_Service), pSvc);

   m_TIDRTClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(0, m_SvcClient.LogEntries());
   ASSERT_EQ(0, m_RTClient.LogEntries());
}

////////////////////////////////////////////////////////////////////////////////

template <typename SvcClient, // EmptyIServiceClient, CallTrackingIServiceClient
          typename RTClient>  // EmptyIRuntimeClient, CallTrackingIRuntimeClient
class TServiceAllocateFailed_f_0 : public ::testing::Test
{ // simple fixture
public:
   TServiceAllocateFailed_f_0() :
      m_pDisp(NULL),
      m_pEvent(NULL)
   {}

   virtual void SetUp()
   {
      m_pEvent = new(std::nothrow) CAALEvent(NULL);
      ASSERT_NONNULL(m_pEvent);

      m_pDisp = new(std::nothrow) ServiceAllocateFailed(&m_SvcClient,
                                                        &m_RTClient,
                                                        m_pEvent);
      ASSERT_NONNULL(m_pDisp);
   }

protected:
   ServiceAllocateFailed *m_pDisp;
   IEvent                *m_pEvent;
   SvcClient              m_SvcClient;
   RTClient               m_RTClient;
};

class ServiceAllocateFailed_f_0 : public TServiceAllocateFailed_f_0<CallTrackingIServiceClient,
                                                                    CallTrackingIRuntimeClient>
{
protected:
   ServiceAllocateFailed_f_0() {}
   virtual ~ServiceAllocateFailed_f_0() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_RTClient.ClearLog();
   }
};

TEST_F(ServiceAllocateFailed_f_0, aal0755)
{
   // ServiceAllocateFailed::operator() invokes IServiceClient::serviceAllocateFailed() on
   // a non-NULL IServiceClient and IRuntimeClient::runtimeAllocateServiceFailed() on a non-NULL
   // IRuntimeClient. The IEvent * parameter to the ServiceAllocateFailed() c'tor is destroyed
   // along with the ServiceAllocateFailed object.

   m_pDisp->operator() ();

   btObjectType x = NULL;
   IEvent      *pEvt;

   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceAllocateFailed", m_SvcClient.Entry(0).MethodName());

   m_SvcClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);

   pEvt = reinterpret_cast<IEvent *>(x);
   EXPECT_EQ(m_pEvent, pEvt);


   ASSERT_EQ(1, m_RTClient.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeAllocateServiceFailed", m_RTClient.Entry(0).MethodName());

   x = NULL;
   m_RTClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);

   pEvt = reinterpret_cast<IEvent *>(x);
   EXPECT_EQ(m_pEvent, pEvt);
}

////////////////////////////////////////////////////////////////////////////////

template <typename SvcClient> // EmptyIServiceClient, CallTrackingIServiceClient
class TServiceReleased_f_0 : public ::testing::Test
{ // simple fixture
public:
   TServiceReleased_f_0() :
      m_pDisp(NULL),
      m_Service(NULL, NULL)
   {}

   virtual void SetUp()
   {
      TransactionID tid;
      tid.ID(6);

      tid.Filter(true); // filter has no effect because Ibase() is NULL.

      m_pDisp = new(std::nothrow) ServiceReleased(&m_SvcClient,
                                                  &m_Service,
                                                  tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceReleased *m_pDisp;
   DispTestService  m_Service;
   SvcClient        m_SvcClient;
};

class ServiceReleased_f_0 : public TServiceReleased_f_0<CallTrackingIServiceClient>
{
protected:
   ServiceReleased_f_0() {}
   virtual ~ServiceReleased_f_0() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      TServiceReleased_f_0<CallTrackingIServiceClient>::TearDown();
   }
};

TEST_F(ServiceReleased_f_0, aal0756)
{
   // When the IBase * parameter to ServiceReleased implements iidServiceBase,
   // ServiceReleased::operator() invokes IServiceBase::ReleaseComplete() on
   // the iidServiceBase interface.

   const bt32bitInt id = 6;

   m_pDisp->operator() ();


   ASSERT_EQ(1, m_Service.LogEntries());
   EXPECT_STREQ("IServiceBase::ReleaseComplete", m_Service.Entry(0).MethodName());


   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_SvcClient.Entry(0).MethodName());

   TransactionID tid;
   m_SvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());
}

TEST_F(ServiceReleased_f_0, aal0757)
{
   // When the TransactionID parameter to ServiceReleased has no IBase * (as reported by
   // TransactionID::Ibase()) ServiceAllocated::operator() invokes IServiceClient::serviceReleased() on
   // a non-NULL IServiceClient.

   const bt32bitInt id = 6;

   m_pDisp->operator() ();


   ASSERT_EQ(1, m_Service.LogEntries());
   EXPECT_STREQ("IServiceBase::ReleaseComplete", m_Service.Entry(0).MethodName());


   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_SvcClient.Entry(0).MethodName());

   TransactionID tid;
   m_SvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());
}

template <typename SvcClient> // EmptyIServiceClient, CallTrackingIServiceClient
class TServiceReleased_f_1 : public ::testing::Test
{ // simple fixture
public:
   TServiceReleased_f_1() :
      m_pDisp(NULL),
      m_Service(&m_TIDSvcClient, NULL)
   {}

   virtual void SetUp()
   {
      TransactionID tid(7, &m_Service);

      tid.Filter(false);

      m_pDisp = new(std::nothrow) ServiceReleased(&m_SvcClient,
                                                  &m_Service,
                                                  tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceReleased *m_pDisp;
   DispTestService  m_Service;
   SvcClient        m_SvcClient;
   SvcClient        m_TIDSvcClient;
};

class ServiceReleased_f_1 : public TServiceReleased_f_1<CallTrackingIServiceClient>
{
protected:
   ServiceReleased_f_1() {}
   virtual ~ServiceReleased_f_1() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_TIDSvcClient.ClearLog();
      TServiceReleased_f_1<CallTrackingIServiceClient>::TearDown();
   }
};

TEST_F(ServiceReleased_f_1, aal0758)
{
   // When the TransactionID parameter to ServiceReleased has a non-NULL IBase * (as reported by
   // TransactionID::Ibase()) ServiceReleased::operator() () queries the IBase for iidServiceClient.
   // When present, operator() invokes IServiceClient::serviceReleased() on
   // the iidServiceClient found in the TransactionID's IBase. If the TransactionID's filter flag
   // (as reported by TransactionID::Filter()) is clear, ServiceReleased::operator() () also invokes
   // IServiceClient::serviceReleased() on a non-NULL IServiceClient passed to the c'tor.

   const bt32bitInt id = 7;

   m_pDisp->operator() ();

   btObjectType  x = NULL;
   TransactionID tid;
   IBase        *pSvc;

   ASSERT_EQ(1, m_TIDSvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_TIDSvcClient.Entry(0).MethodName());


   m_TIDSvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_SvcClient.Entry(0).MethodName());

   m_SvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());
}

template <typename SvcClient> // EmptyIServiceClient, CallTrackingIServiceClient
class TServiceReleased_f_2 : public ::testing::Test
{ // simple fixture
public:
   TServiceReleased_f_2() :
      m_pDisp(NULL),
      m_Service(&m_TIDSvcClient, NULL)
   {}

   virtual void SetUp()
   {
      TransactionID tid(8, &m_Service);

      tid.Filter(true);

      m_pDisp = new(std::nothrow) ServiceReleased(&m_SvcClient,
                                                  &m_Service,
                                                  tid);
      ASSERT_NONNULL(m_pDisp);
   }

   virtual void TearDown()
   {
      m_Service.ClearLog();
   }

protected:
   ServiceReleased *m_pDisp;
   DispTestService  m_Service;
   SvcClient        m_SvcClient;
   SvcClient        m_TIDSvcClient;
};

class ServiceReleased_f_2 : public TServiceReleased_f_2<CallTrackingIServiceClient>
{
protected:
   ServiceReleased_f_2() {}
   virtual ~ServiceReleased_f_2() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
      m_TIDSvcClient.ClearLog();
      TServiceReleased_f_2<CallTrackingIServiceClient>::TearDown();
   }
};

TEST_F(ServiceReleased_f_2, aal0759)
{
   // When the TransactionID parameter to ServiceReleased has a non-NULL IBase * (as reported by
   // TransactionID::Ibase()) ServiceReleased::operator() () queries the IBase for iidServiceClient.
   // When present, operator() invokes IServiceClient::serviceReleased() on
   // the iidServiceClient found in the TransactionID's IBase. If the TransactionID's filter flag
   // (as reported by TransactionID::Filter()) is set, the IServiceClient * passed to the
   // ServiceReleased c'tor is ignored.

   const bt32bitInt id = 8;

   m_pDisp->operator() ();

   btObjectType  x = NULL;
   TransactionID tid;
   IBase        *pSvc;

   ASSERT_EQ(1, m_TIDSvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleased", m_TIDSvcClient.Entry(0).MethodName());


   m_TIDSvcClient.Entry(0).GetParam("tid", tid);
   EXPECT_EQ(id, tid.ID());


   EXPECT_EQ(0, m_SvcClient.LogEntries());
}

////////////////////////////////////////////////////////////////////////////////

template <typename SvcClient> // EmptyIServiceClient, CallTrackingIServiceClient
class TServiceReleaseFailed_f_0 : public ::testing::Test
{ // simple fixture
public:
   TServiceReleaseFailed_f_0() :
      m_pDisp(NULL),
      m_pEvent(NULL)
   {}

   virtual void SetUp()
   {
      m_pEvent = new(std::nothrow) CAALEvent(NULL);
      ASSERT_NONNULL(m_pEvent);

      m_pDisp = new(std::nothrow) ServiceReleaseFailed(&m_SvcClient,
                                                       m_pEvent);
      ASSERT_NONNULL(m_pDisp);
   }

protected:
   ServiceReleaseFailed *m_pDisp;
   IEvent               *m_pEvent;
   SvcClient             m_SvcClient;
};

class ServiceReleaseFailed_f_0 : public TServiceReleaseFailed_f_0<CallTrackingIServiceClient>
{
protected:
   ServiceReleaseFailed_f_0() {}
   virtual ~ServiceReleaseFailed_f_0() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
   }
};

TEST_F(ServiceReleaseFailed_f_0, aal0760)
{
   // ServiceReleaseFailed::operator() invokes IServiceClient::serviceReleaseFailed() on
   // a non-NULL IServiceClient. The IEvent * parameter to the ServiceReleaseFailed() c'tor
   // is destroyed along with the ServiceReleaseFailed object.

   m_pDisp->operator() ();

   btObjectType x = NULL;
   IEvent      *pEvt;

   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceReleaseFailed", m_SvcClient.Entry(0).MethodName());

   m_SvcClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);

   pEvt = reinterpret_cast<IEvent *>(x);
   EXPECT_EQ(m_pEvent, pEvt);
}

////////////////////////////////////////////////////////////////////////////////

template <typename SvcClient> // EmptyIServiceClient, CallTrackingIServiceClient
class TServiceEvent_f_0 : public ::testing::Test
{ // simple fixture
public:
   TServiceEvent_f_0() :
      m_pDisp(NULL),
      m_pEvent(NULL)
   {}

   virtual void SetUp()
   {
      m_pEvent = new(std::nothrow) CAALEvent(NULL);
      ASSERT_NONNULL(m_pEvent);

      m_pDisp = new(std::nothrow) ServiceEvent(&m_SvcClient,
                                               m_pEvent);
      ASSERT_NONNULL(m_pDisp);
   }

protected:
   ServiceEvent *m_pDisp;
   IEvent       *m_pEvent;
   SvcClient     m_SvcClient;
};

class ServiceEvent_f_0 : public TServiceEvent_f_0<CallTrackingIServiceClient>
{
protected:
   ServiceEvent_f_0() {}
   virtual ~ServiceEvent_f_0() {}

   virtual void TearDown()
   {
      m_SvcClient.ClearLog();
   }
};

TEST_F(ServiceEvent_f_0, aal0761)
{
   // ServiceEvent::operator() invokes IServiceClient::serviceEvent() on
   // a non-NULL IServiceClient. The IEvent * parameter to the ServiceEvent() c'tor
   // is destroyed along with the ServiceEvent object.

   m_pDisp->operator() ();

   btObjectType x = NULL;
   IEvent      *pEvt;

   ASSERT_EQ(1, m_SvcClient.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", m_SvcClient.Entry(0).MethodName());

   m_SvcClient.Entry(0).GetParam("e", &x);
   ASSERT_NONNULL(x);

   pEvt = reinterpret_cast<IEvent *>(x);
   EXPECT_EQ(m_pEvent, pEvt);
}

