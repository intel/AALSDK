// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

template <typename S>
class MockDoWorkerApp_f : public ::testing::Test
{
protected:
   MockDoWorkerApp_f()
      : m_pLock(NULL)
      , m_pRCA(NULL)
      , m_pWrkClnt(NULL)
      , m_pListener(NULL)
      , m_pDoWrkSvc(NULL)
   {
   }

   // required ingredients for typed tests
   typedef std::list<S> List;
   static S shared_;
   S value_;

   ~MockDoWorkerApp_f()
   {
      delete m_pLock;
      m_pLock = NULL;
      delete m_pWrkClnt;
      m_pWrkClnt = NULL;
      delete m_pRCA;
      m_pRCA = NULL;
      delete m_pListener;
      m_pListener = NULL;
   }

   void SetUp()
   {
      value_.acceptBuilder(&value_);

      // pass the builder to the factory 
      InModuleSvcsFact
         <CMockDoWorker>::GetServiceFactory()->acceptBuilder(&value_);

      m_pLock = new (std::nothrow) CListenerLock();
      EXPECT_FALSE(NULL == m_pLock);

      m_pRCA = new (std::nothrow) CRuntimeClientAdapter(m_pLock);
      EXPECT_FALSE(NULL == m_pRCA);

      m_pLock->wait();   // wait for runtime start

      m_pWrkClnt = new (std::nothrow) CMockWorkClient(m_pRCA);
      EXPECT_FALSE(NULL == m_pWrkClnt);

      m_pListener = new (std::nothrow)
         CServiceListener(m_pRCA, m_pLock, "aal0821");
      EXPECT_FALSE(NULL == m_pListener);

      m_pWrkClnt->setListener(m_pListener);

      EXPECT_TRUE(SUCCESS == m_pWrkClnt->aquireServiceResource());
      m_pLock->wait();

      m_pDoWrkSvc = dynamic_cast
         <CMockDoWorker*>(m_pListener->getService());

      // pass the visitor to the service the builder and the visitor are
      // the same surrogate object, defined below
      EXPECT_FALSE(NULL == m_pDoWrkSvc);
      m_pDoWrkSvc->acceptVisitor(dynamic_cast
                                       <IVisitingWorker*>(&value_));
   }

   void TearDown()
   {
      dynamic_cast<ServiceBase*>(m_pListener->getService())->serviceRevoke();
      m_pLock->wait();   // wait for service release
      (m_pRCA)->getRuntime()->releaseRuntimeProxy();
      m_pLock->wait();   // wait for runtime release.
   }

   CListenerLock*         m_pLock;
   CRuntimeClientAdapter* m_pRCA;
   CMockWorkClient*       m_pWrkClnt;
   CServiceListener*      m_pListener;
   CMockDoWorker*         m_pDoWrkSvc;
};


/// ===================================================================
/// @brief        Combination builder / visitor for test case aal0821
///
/// @details      The surrogate adapter provides empty implementions
///               from gtCommon_Mocks
///
class AAL0821Surrogate : public CSurrogateAdapter
{
public:
   /// ================================================================
   ///   TEST CODE STARTS HERE
   void doContractWork(ServiceBase* pWorkContainer)
   {
      // do work for test case aal0821
      MSG("doing work in aal0821");
      pWorkContainer->sendmsg();
   }

   ~AAL0821Surrogate() {}
};

// declare the parameter type here
typedef testing::Types<AAL0821Surrogate> S;
TYPED_TEST_CASE(MockDoWorkerApp_f, S);

// must use this pointer to reference the fixture object
TYPED_TEST(MockDoWorkerApp_f, aal0821)
{
   this->m_pDoWrkSvc->doWork();
   this->m_pLock->wait();
}


