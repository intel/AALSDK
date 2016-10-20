// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

// ServiceBase::ServiceBase(AALServiceModule *container,
//                          IRuntime         *pAALRuntime,
//                          IAALTransport    *ptransport,
//                          IAALMarshaller   *marshaller,
//                          IAALUnMarshaller *unmarshaller) :

class AAL0821Surrogate : public IVisitingWorker,
                         public IServiceBuilder,
                         public EmptyIAALTransport,
                         public EmptyIAALMarshaller,
                         public EmptyIAALUnMarshaller
{
public:
   NamedValueSet parms;
   /// ================================================================
   ///   TEST CODE STARTS HERE
   void doContractWork(ServiceBase* pWorkContainer)
   {
      // do work for test case aal0821
      EXPECT_FALSE(pWorkContainer->sendmsg());
      EXPECT_FALSE(pWorkContainer->getmsg());
    
      // Do we exit the process (line 384 in AALService.cpp)
      // because getmsg() returns false

      // Seems I'm not able to use a death-test for this exit call.
      // ::testing::FLAGS_gtest_death_test_style = "threadsafe";
      // EXPECT_DEATH(pWorkContainer->startMDS(), ",*");
   }

   INamedValueSet const* getValues()
   {
      return static_cast<INamedValueSet const*>(&parms);
   }
};
/// ===================================================================
/// @brief        Fixture class to plug surrogate (visitor/builder) service
///               implementations into the existing mock services foundation
///               classes.
///
class MockDoWorkerApp_f : public ::testing::Test
{
protected:
   MockDoWorkerApp_f()
      : m_pSurrogate(NULL)
      , m_pDoWrkSvc(NULL)
      , m_pLock(NULL)
      , m_pRCA(NULL)
      , m_pWrkClnt(NULL)
      , m_pListener(NULL)
   {
      m_pSurrogate = new (std::nothrow) AAL0821Surrogate();
      NULLCHECKDBG(m_pSurrogate);
      m_pLock = new (std::nothrow) CListenerLock();
      NULLCHECKDBG(m_pLock);
      m_pRCA = new (std::nothrow) CRuntimeClientAdapter(m_pLock);
      NULLCHECKDBG(m_pRCA);
      m_pLock->wait();   // wait for runtime start
      m_pWrkClnt = new (std::nothrow) CMockWorkClient(m_pRCA);
      NULLCHECKDBG(m_pWrkClnt);
      m_pListener = new (std::nothrow)
         CServiceListener(m_pRCA, m_pLock, "aal0821");
      NULLCHECKDBG(m_pListener);
   }

   void SetUp()
   {
      m_pWrkClnt->setListener(m_pListener);
      GetServiceFactory()->acceptBuilder(dynamic_cast
                                         <IServiceBuilder*>(m_pSurrogate));
      ASSERT(SUCCESS == m_pWrkClnt->aquireServiceResource());
      m_pLock->wait();
      m_pDoWrkSvc = dynamic_cast<CMockDoWorker*>(m_pListener->getService());
      m_pDoWrkSvc->acceptVisitor(dynamic_cast<IVisitingWorker*>(m_pSurrogate));
   }

   void TearDown()
   {
      dynamic_cast<ServiceBase*>(m_pListener->getService())->serviceRevoke();
      m_pLock->wait();   // wait for service release
      (m_pRCA)->getRuntime()->releaseRuntimeProxy();
      m_pLock->wait();   // wait for runtime release.
   }

   ~MockDoWorkerApp_f()
   {
      delete m_pLock;
      delete m_pRCA;
      delete m_pWrkClnt;
      delete m_pListener;
      delete m_pSurrogate;
   }

   AAL0821Surrogate* m_pSurrogate;
   CMockDoWorker* m_pDoWrkSvc;
   CListenerLock* m_pLock;
   CRuntimeClientAdapter* m_pRCA;
   CMockWorkClient* m_pWrkClnt;
   CServiceListener* m_pListener;
};

TEST_F(MockDoWorkerApp_f, aal0821)
{
   m_pDoWrkSvc->doWork();
   m_pLock->wait();
}
