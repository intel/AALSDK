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
      aal0677Service(IServiceModuleCallback *pSvcModCB, int &Counter) :
         m_pSvcModCB(pSvcModCB),
         m_Counter(Counter)
      {
         SetSubClassInterface(iidService, dynamic_cast<IAALService *>(this));
      }

      virtual btBool Release(TransactionID const & ,
                             btTime                ) { return false; }

      virtual btBool Release(btTime timeout)
      {
         m_pSvcModCB->ServiceReleased( dynamic_ptr<IBase>(iidBase, this) );
         ++m_Counter;
         return true;
      }

      virtual btBool SetParms(NamedValueSet const & ) { return false; }

   protected:
      IServiceModuleCallback *m_pSvcModCB;
      int                    &m_Counter;
   };

   int            Count = 0;
   aal0677Service ServiceA(&mod, Count);
   aal0677Service ServiceB(&mod, Count);
   IBase         *pServices[2];

   factory.CreateServiceObjectReturnsThisValue( dynamic_ptr<IBase>(iidBase, &ServiceA) );
   pServices[0] = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_EQ(dynamic_ptr<IBase>(iidBase, &ServiceA), pServices[0]);

   factory.CreateServiceObjectReturnsThisValue( dynamic_ptr<IBase>(iidBase, &ServiceB) );
   pServices[1] = mod.Construct(&irt, &client, tid, optArgs);

   EXPECT_EQ(dynamic_ptr<IBase>(iidBase, &ServiceB), pServices[1]);

   mod.Destroy();

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

