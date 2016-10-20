// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H

#ifndef HAVE_COMMON_H
#define HAVE_COMMON_H
#include "gtCommon.h"
#endif

TEST(SMocks, aal0820)
{
   /// ========================================================================
   /// @test         Base line test case for the in-module mock service, client
   ///               adapters and listeners, and the in-module service factory.
   ///
   /// @brief        Functional test of some new mock classes that have been adapted
   ///               from the user-mode samples.
   ///
   /// @details      No real work being done by the service.  Tests callbacks
   ///               and delegation, synchronization and cleanup.
   ///
   ///               Uses a service factory that obviates the need for a
   ///               separate service module.
   ///

   CListenerLock localLock;

   CRuntimeClientAdapter* pRCA = new (std::nothrow) CRuntimeClientAdapter(&localLock);
   EXPECT_NONNULL(pRCA);
   localLock.wait();   // wait for runtime start

   EXPECT_NONNULL(dynamic_cast<IBase*>(pRCA));
   EXPECT_NONNULL(dynamic_cast<IRuntimeClient*>(pRCA));
   // Did we have success on a call to SetInterface in the CAASBase parent
   // class?
   //
   EXPECT_TRUE((pRCA)->Has(iidBase));
   // Did we have success on a call SetInterface in the custom constructor?
   //
   EXPECT_TRUE((pRCA)->Has(iidRuntimeClient));
   // Was m_bIsOK set to true in CAASBase()?
   //
   EXPECT_TRUE((pRCA)->IsOK());

   CMockWorkClient* pMSC = new (std::nothrow) CMockWorkClient(pRCA);
   EXPECT_NONNULL(pMSC);

   EXPECT_NONNULL(dynamic_cast<IBase*>(pMSC));
   EXPECT_NONNULL(dynamic_cast<IServiceClient*>(pMSC));

   EXPECT_TRUE(pMSC->Has(iidBase));
   // Did we have success on a call to SetInterface in the CAASBase parent
   // class?
   EXPECT_TRUE(pMSC->Has(iidServiceClient));
   // Did we have success on a call to SetInterface for IServiceClient?
   //
   EXPECT_TRUE(pMSC->Has(iidMockWorkClient));
   // Did we have success on a call to SetInterface for IMockWorkClient?

   EXPECT_TRUE(pMSC->IsOK());

   CServiceListener* listener = new (std::nothrow) CServiceListener(pRCA, &localLock, "aal0820");
   EXPECT_NONNULL(listener);

   pMSC->setListener(listener);

   // Returns greater-than 0 on serviceAllocateFailed event.
   // SUCCESS == 0
   EXPECT_EQ(pMSC->aquireServiceResource(), SUCCESS);
   localLock.wait();

   // Test accessors to variables that are set in OnServiceAllocated of
   // CServiceListener.
   EXPECT_NONNULL(listener->getService());
   EXPECT_NONNULL(listener->getService()->getServiceClient());

   CMockDoWorker* pMS = dynamic_cast<CMockDoWorker*>(listener->getService());
   EXPECT_NONNULL(pMS);

   pMS->doWork();

   localLock.wait();   // wait for work completion

   // dynamic_cast<ServiceBase*>(listener->getService())->Release(TransactionID(), AAL_INFINITE_WAIT);
   dynamic_cast<ServiceBase*>(listener->getService())->serviceRevoke();
   localLock.wait();   // wait for service release

   (pRCA)->getRuntime()->releaseRuntimeProxy();
   localLock.wait();   // wait for runtime release.

   delete pRCA;
   delete pMSC;
   delete listener;
}

