// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include <aalsdk/aas/ServiceHost.h>

////////////////////////////////////////////////////////////////////////////////

template <typename X>
class ServiceHost_f : public ::testing::Test
{ // simple fixture
public:
   ServiceHost_f() :
      m_pSvcHost(NULL)
   {}

   virtual void SetUp()
   {
      m_pSvcHost = new(std::nothrow) ServiceHost(m_ConstructWith);
      ASSERT_NONNULL(m_pSvcHost);
   }
   virtual void TearDown()
   {
      if ( NULL != m_pSvcHost ) {
         delete m_pSvcHost;
      }
   }

   btBool InstantiateService(IRuntime            *pRuntime,
                             IBase               *pClient,
                             NamedValueSet const &rManifest,
                             TransactionID const &rTranID)
   { return m_pSvcHost->InstantiateService(pRuntime, pClient, rManifest, rTranID); }

   btBool                  IsOK() const { return m_pSvcHost->IsOK();                  }
   std::string const &  getName() const { return m_pSvcHost->getName();               }
   IBase *             getIBase() const { return m_pSvcHost->getIBase();              }
              operator IBase * () const { return m_pSvcHost->operator AAL::IBase *(); }
   IServiceModule * getProvider() const { return m_pSvcHost->getProvider();           }
   void            freeProvider()       { m_pSvcHost->freeProvider();                 }

   ServiceHost *m_pSvcHost;
   X            m_ConstructWith;
};

class ServiceHost_f_0 : public ServiceHost_f<btcString>
{
protected:
   ServiceHost_f_0() { m_ConstructWith = (btcString)"libswvalsvcmod"; }
};

class ServiceHost_f_1 : public ServiceHost_f<AALSvcEntryPoint>
{
protected:
   ServiceHost_f_1() { m_ConstructWith = NULL; }
};

class ServiceHost_f_2 : public ServiceHost_f<AALSvcEntryPoint>
{
public:
   virtual void TearDown()
   {
      ServiceHost_f_2::sm_SvcModule.ClearLog();
      ServiceHost_f<AALSvcEntryPoint>::TearDown();
   }

protected:
   ServiceHost_f_2() { m_ConstructWith = ServiceHost_f_2::EntryPoint; }

   static CallTrackingServiceModule sm_SvcModule;

   static bt32bitInt EntryPoint(btUnsigned32bitInt , btAny );
};

CallTrackingServiceModule ServiceHost_f_2::sm_SvcModule;

bt32bitInt ServiceHost_f_2::EntryPoint(btUnsigned32bitInt cmd, btAny arg)
{
   switch ( cmd ) {
      case AAL_SVC_CMD_GET_PROVIDER  : {
         *(IServiceModule **)arg = &ServiceHost_f_2::sm_SvcModule;
      } break;
      case AAL_SVC_CMD_FREE_PROVIDER : break;
      default : return 1;
   }

   return 0;
}

class ServiceHost_f_3 : public ServiceHost_f<AALSvcEntryPoint>
{
protected:
   ServiceHost_f_3() { m_ConstructWith = ServiceHost_f_3::EntryPoint; }

   static bt32bitInt EntryPoint(btUnsigned32bitInt , btAny );
};

bt32bitInt ServiceHost_f_3::EntryPoint(btUnsigned32bitInt cmd, btAny arg)
{
   switch ( cmd ) {
      case AAL_SVC_CMD_GET_PROVIDER  : break;
      case AAL_SVC_CMD_FREE_PROVIDER : break;
      default : return 1;
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceHost_f_0, aal0700)
{
   // When constructed with the root name of a valid AAL Service Module,
   // ServiceHost::ServiceHost(btcString ) loads the service module library, obtains the service
   // entry point, and uses the entry point to query the IServiceModule * for the service.
   // The IServiceModule * is made available by ServiceHost::getProvider().

   EXPECT_TRUE(IsOK());

   EXPECT_EQ(0, getName().compare(m_ConstructWith));
   EXPECT_NULL(getIBase());
   EXPECT_NULL(operator AAL::IBase *());
   EXPECT_NONNULL(getProvider());
}

TEST_F(ServiceHost_f_1, aal0701)
{
   // When constructed with NULL, ServiceHost::ServiceHost(AALSvcEntryPoint ) returns immediately.
   // ServiceHost::IsOK() returns false, indicating the error.

   EXPECT_FALSE(IsOK());
}

TEST_F(ServiceHost_f_2, aal0702)
{
   // When constructed with a non-NULL entry point, ServiceHost::ServiceHost(AALSvcEntryPoint )
   // invokes the entry point with command AAL_SVC_CMD_GET_PROVIDER to retrieve the
   // IServiceModule *. When the entry point acquires a non-NULL IServiceModule *,
   // ServiceHost::IsOK() returns true, indicating success. ServiceHost::getProvider() returns
   // the acquired IServiceModule *.

   EXPECT_TRUE(IsOK());
   EXPECT_EQ(&ServiceHost_f_2::sm_SvcModule, getProvider());
   EXPECT_EQ(0, ServiceHost_f_2::sm_SvcModule.LogEntries());
}

TEST_F(ServiceHost_f_3, aal0703)
{
   // When constructed with a non-NULL entry point, ServiceHost::ServiceHost(AALSvcEntryPoint )
   // invokes the entry point with command AAL_SVC_CMD_GET_PROVIDER to retrieve the
   // IServiceModule *. When the entry point returns a NULL IServiceModule *,
   // ServiceHost::IsOK() returns false, indicating the error. ServiceHost::getProvider() returns
   // NULL.

   EXPECT_FALSE(IsOK());
   EXPECT_NULL(getProvider());
}

TEST_F(ServiceHost_f_2, aal0704)
{
   // When passed a NULL IRuntime *, ServiceHost::InstantiateService() returns false,
   // indicating the error.

   CAASBase      base;
   NamedValueSet nvs;
   TransactionID tid;

   EXPECT_FALSE(InstantiateService(NULL, &base, nvs, tid));
   EXPECT_EQ(0, ServiceHost_f_2::sm_SvcModule.LogEntries());
}

TEST_F(ServiceHost_f_2, aal0705)
{
   // When the ServiceHost is valid and the given IRuntime * is non-NULL,
   // ServiceHost::InstantiateService() invokes IServiceModule::Construct() on m_pProvider.
   // The resulting service IBase * is available via ServiceHost::getIBase() and
   // ServiceHost::operator IBase *(). InstantiateService() returns true if the allocated
   // service is non-NULL, and false otherwise.

   EmptyIRuntime rt;
   CAASBase      client;

   CAASBase      base;
   NamedValueSet nvs;
   nvs.Add(5, (btByte)27);
   TransactionID tid;
   tid.ID(3);

   ServiceHost_f_2::sm_SvcModule.ConstructReturnsThisValue(&base);
   EXPECT_TRUE(InstantiateService(&rt, &client, nvs, tid));
#if 0
   EXPECT_EQ(dynamic_cast<IBase *>(&base), getIBase());
   EXPECT_EQ(dynamic_cast<IBase *>(&base), this->operator AAL::IBase *());
#endif

   EXPECT_EQ(1, ServiceHost_f_2::sm_SvcModule.LogEntries());
   EXPECT_STREQ("IServiceModule::Construct", ServiceHost_f_2::sm_SvcModule.Entry(0).MethodName());

   btObjectType x = NULL;

   ServiceHost_f_2::sm_SvcModule.Entry(0).GetParam("pAALRUNTIME", &x);
   EXPECT_EQ(dynamic_cast<IRuntime *>(&rt), reinterpret_cast<IRuntime *>(x));

   ServiceHost_f_2::sm_SvcModule.Entry(0).GetParam("Client", &x);
   EXPECT_EQ(dynamic_cast<IBase *>(&client), reinterpret_cast<IBase *>(x));

   INamedValueSet const *pNVS = NULL;
   ServiceHost_f_2::sm_SvcModule.Entry(0).GetParam("optArgs", &pNVS);
   ASSERT_NONNULL(pNVS);
   EXPECT_TRUE(pNVS->operator == (nvs));

   TransactionID tid2;
   ServiceHost_f_2::sm_SvcModule.Entry(0).GetParam("tid", tid2);
   EXPECT_EQ(3, tid2.ID());


   ServiceHost_f_2::sm_SvcModule.ConstructReturnsThisValue(NULL);
   EXPECT_FALSE(InstantiateService(&rt, &client, nvs, tid));
#if 0
   EXPECT_EQ(NULL, getIBase());
   EXPECT_EQ(NULL, this->operator AAL::IBase *());
#endif
}

