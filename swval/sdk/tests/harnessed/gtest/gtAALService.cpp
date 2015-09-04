// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

TEST(AALServiceModuleTest, aal0673)
{
   // AALServiceModule(ISvcsFact & ) sets a SubClass interface of iidServiceProvider/IServiceModule *.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_TRUE(mod.IsOK());

   EXPECT_TRUE(mod.Has(iidServiceProvider));
   EXPECT_EQ(dynamic_cast<IServiceModule *>(&mod),
             reinterpret_cast<IServiceModule *>( mod.Interface(iidServiceProvider) ));

   EXPECT_EQ(iidServiceProvider, mod.SubClassID());
   EXPECT_EQ(dynamic_cast<IServiceModule *>(&mod), mod.ISubClass());
}

TEST(AALServiceModuleTest, aal0674)
{
   // AALServiceModule::Construct() requests an AAL Service (returned as an IBase *) from the
   // ISvcsFact provided in the AALServiceModule constructor. When ISvcsFact::CreateServiceObject()
   // is successful, as signified by a non-NULL return value, the IBase * is added to the list of
   // services tracked by the AALServiceModule, and ISvcsFact::InitializeService() is called.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   CAASBase TheService;
   factory.CreateServiceObjectReturnsThisValue(dynamic_cast<IBase *>(&TheService));

   EXPECT_TRUE(mod.IsOK());

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   tid.ID(3);
   NamedValueSet optArgs;
   optArgs.Add((btStringKey)"A", false);

   IBase *pSvc = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_EQ(dynamic_cast<IBase *>(&TheService), pSvc);

   ASSERT_EQ(2, factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&mod), x);

   x = NULL;
   factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);


   EXPECT_STREQ("ISvcsFact::InitializeService", factory.Entry(1).MethodName());

   x = NULL;
   factory.Entry(1).GetParam("Client", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&client), x);

   TransactionID tid2;
   factory.Entry(1).GetParam("rtid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());

   INamedValueSet const *poptArgs = NULL;
   factory.Entry(1).GetParam("optArgs", &poptArgs);
   ASSERT_NONNULL(poptArgs);
   EXPECT_EQ(optArgs, *poptArgs);
}

TEST(AALServiceModuleTest, aal0675)
{
   // During AALServiceModule::Construct(), when ISvcsFact::CreateServiceObject() returns a NULL
   // IBase *, Construct() returns NULL immediately, indicating the error.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_TRUE(mod.IsOK());

   factory.CreateServiceObjectReturnsThisValue(NULL);

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   NamedValueSet optArgs;

   IBase *pSvc = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_NULL(pSvc);

   ASSERT_EQ(1, factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&mod), x);

   x = NULL;
   factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);
}

TEST(AALServiceModuleTest, aal0676)
{
   // During AALServiceModule::Construct(), when ISvcsFact::InitializeService() fails, Construct()
   // returns NULL indicating the error.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_TRUE(mod.IsOK());

   CAASBase TheService;
   factory.CreateServiceObjectReturnsThisValue(dynamic_cast<IBase *>(&TheService));
   factory.InitializeServiceReturnsThisValue(false);

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   tid.ID(3);
   NamedValueSet optArgs;
   optArgs.Add((btStringKey)"A", false);

   IBase *pSvc = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_NULL(pSvc);

   ASSERT_EQ(2, factory.LogEntries());

   EXPECT_STREQ("ISvcsFact::CreateServiceObject", factory.Entry(0).MethodName());
   btObjectType x;

   x = NULL;
   factory.Entry(0).GetParam("container", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&mod), x);

   x = NULL;
   factory.Entry(0).GetParam("pRuntime", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&irt), x);


   EXPECT_STREQ("ISvcsFact::InitializeService", factory.Entry(1).MethodName());

   x = NULL;
   factory.Entry(1).GetParam("Client", &x);
   EXPECT_EQ(reinterpret_cast<btObjectType>(&client), x);

   TransactionID tid2;
   factory.Entry(1).GetParam("rtid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());

   INamedValueSet const *poptArgs = NULL;
   factory.Entry(1).GetParam("optArgs", &poptArgs);
   ASSERT_NONNULL(poptArgs);
   EXPECT_EQ(optArgs, *poptArgs);
}

TEST(AALServiceModuleTest, aal0677)
{
   // AALServiceModule::Destroy() releases (via IAALService::Release(btTime )) each service being
   // tracked by the AALServiceModule, waiting until each has been released prior to returning.

   CallTrackingISvcsFact factory;
   AALServiceModule      mod(factory);

   EXPECT_TRUE(mod.IsOK());

   EmptyIRuntime irt;
   CAASBase      client;
   TransactionID tid;
   tid.ID(3);
   NamedValueSet optArgs;
   optArgs.Add((btStringKey)"A", false);

   class aal0677Service : public CAASBase,
                          public AAL::IAALService
   {
   public:
      aal0677Service(IThreadGroup           *pGrp,
                     IServiceModuleCallback *pSvcModCB,
                     int                    &Counter) :
         m_pGrp(pGrp),
         m_pSvcModCB(pSvcModCB),
         m_Counter(Counter)
      {
         SetSubClassInterface(iidService, dynamic_cast<IAALService *>(this));
      }

      virtual btBool Release(TransactionID const & ,
                             btTime                ) { return false; }

      // We need the IServiceModuleCallback::ServiceReleased() notification to happen
      // from another thread so that AALServiceModule's CriticalSection prevents manipulation
      // of AALServiceModule::m_serviceList while the list is being walked by
      // AALServiceModule::SendReleaseToAll().
      class DoServiceReleased : public IDispatchable
      {
      public:
         DoServiceReleased(IServiceModuleCallback *pSMC, IBase *pSvc) :
            m_pSMC(pSMC),
            m_pSvc(pSvc)
         {}
         virtual void operator()()
         {
            m_pSMC->ServiceReleased(m_pSvc);
            delete this;
         }
      protected:
         IServiceModuleCallback *m_pSMC;
         IBase                  *m_pSvc;
      };

      virtual btBool Release(btTime timeout)
      {
         ++m_Counter;
         return m_pGrp->Add( new DoServiceReleased(m_pSvcModCB, dynamic_ptr<IBase>(iidBase, this)) );
      }

      virtual btBool SetParms(NamedValueSet const & ) { return false; }

   protected:
      IThreadGroup           *m_pGrp;
      IServiceModuleCallback *m_pSvcModCB;
      int                    &m_Counter;
   };

   OSLThreadGroup grp;
   int            Count = 0;

   aal0677Service ServiceA(&grp, &mod, Count);
   aal0677Service ServiceB(&grp, &mod, Count);
   IBase         *pServices[2];

   factory.CreateServiceObjectReturnsThisValue( dynamic_ptr<IBase>(iidBase, &ServiceA) );
   pServices[0] = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_EQ(dynamic_ptr<IBase>(iidBase, &ServiceA), pServices[0]);

   factory.CreateServiceObjectReturnsThisValue( dynamic_ptr<IBase>(iidBase, &ServiceB) );
   pServices[1] = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_EQ(dynamic_ptr<IBase>(iidBase, &ServiceB), pServices[1]);

   mod.Destroy();

   EXPECT_TRUE(grp.Join(AAL_INFINITE_WAIT));
   EXPECT_EQ(2, Count);
}

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

////////////////////////////////////////////////////////////////////////////////

TEST(ServiceBaseTest, aal0680)
{
   // The ServiceBase constructor sets a SubClass interface of iidService/IAALService * and adds
   // an interface for iidServiceBase/IServiceBase *. The AALServiceModule * that is passed to the
   // constructor is available via pAALServiceModule(), a proxy for the IRuntime * via getRuntime(),
   // the IAALTransport * via recvr() and sender(), the IAALMarshaller via marshall(), and the
   // IAALUnMarshallar via unmarshal().

   EmptyISvcsFact   factory;
   AALServiceModule mod(factory);

   EmptyIRuntime    rt;
   rt.getRuntimeProxyReturnsThisValue(dynamic_cast<IRuntime *>(&rt));

   // new'ing these because ~ServiceBase delete's them.
   EmptyIAALTransport    *tp      = new EmptyIAALTransport();
   EmptyIAALMarshaller   *marsh   = new EmptyIAALMarshaller();
   EmptyIAALUnMarshaller *unmarsh = new EmptyIAALUnMarshaller();

   EmptyServiceBase sb(&mod, &rt, tp, marsh, unmarsh);

   EXPECT_TRUE(sb.IsOK());

   EXPECT_TRUE(sb.Has(iidService));
   EXPECT_EQ(dynamic_cast<IAALService *>(&sb), sb.Interface(iidService));

   EXPECT_EQ(iidService, sb.SubClassID());
   EXPECT_EQ(dynamic_cast<IAALService *>(&sb), sb.ISubClass());

   EXPECT_TRUE(sb.Has(iidServiceBase));
   EXPECT_EQ(dynamic_cast<IServiceBase *>(&sb), sb.Interface(iidServiceBase));

   EXPECT_EQ(&mod, sb.pAALServiceModule());
   EXPECT_EQ(dynamic_cast<IRuntime *>(&rt), sb.getRuntime());

   EXPECT_TRUE(sb.HasMarshaller());
   EXPECT_TRUE(sb.HasUnMarshaller());
   EXPECT_TRUE(sb.HasTransport());

   EXPECT_EQ(dynamic_cast<IAALTransport *>(tp),         &sb.recvr());
   EXPECT_EQ(dynamic_cast<IAALTransport *>(tp),         &sb.sender());
   EXPECT_EQ(dynamic_cast<IAALMarshaller *>(marsh),     &sb.marshall());
   EXPECT_EQ(dynamic_cast<IAALUnMarshaller *>(unmarsh), &sb.unmarshall());

   EXPECT_NULL(sb.getRuntimeClient());
   EXPECT_NULL(sb.Client());
   EXPECT_NULL(sb.ClientBase());
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
   AALServiceModule mod(factory);

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


