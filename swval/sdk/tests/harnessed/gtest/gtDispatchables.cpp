// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/Dispatchables.h>

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



