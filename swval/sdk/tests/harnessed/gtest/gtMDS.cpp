// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtThreadGroup.h"

#include <_MessageDelivery.h>

////////////////////////////////////////////////////////////////////////////////

class MessageDelivery_f : public ::testing::Test
{ // simple fixture
protected:
   MessageDelivery_f() :
      m_pMDS(NULL)
   {}

   virtual void SetUp()
   {
      m_pMDS = new(std::nothrow) _MessageDelivery();
      ASSERT_NONNULL(m_pMDS);
   }
   virtual void TearDown()
   {
      if ( NULL != m_pMDS ) {
         delete m_pMDS;
      }
   }

   void StartMessageDelivery() { m_pMDS->StartMessageDelivery(); }
   void  StopMessageDelivery() { m_pMDS->StopMessageDelivery();  }
   btBool    scheduleMessage(IDispatchable *pDisp) { return m_pMDS->scheduleMessage(pDisp); }

   _MessageDelivery *m_pMDS;
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(MessageDelivery_f, aal0706)
{
   // When successful, _MessageDelivery::_MessageDelivery() sets an interface of
   // iidMDS / IMessageDeliveryService *. _MessageDelivery::IsOK() returns true,
   // indicating success.

#if DEPRECATED
   EXPECT_EQ(iidMDS, m_pMDS->SubClassID());
   EXPECT_EQ(dynamic_cast<IMessageDeliveryService *>(m_pMDS), m_pMDS->ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(m_pMDS->Has(iidMDS));
   EXPECT_EQ(dynamic_cast<IMessageDeliveryService *>(m_pMDS), m_pMDS->Interface(iidMDS));

   EXPECT_TRUE(m_pMDS->IsOK());
}

TEST_F(MessageDelivery_f, aal0707)
{
   // A _MessageDelivery is immediately capable of accepting and dispatching
   // IDispatchable's upon construction.

   btInt i = 0;

   UnsafeCountUpD d(i);

   EXPECT_TRUE(scheduleMessage(&d));

   YIELD_WHILE(0 == i);
}

TEST_F(MessageDelivery_f, aal0708)
{
   // _MessageDelivery::StopMessageDelivery() waits for all the current items to
   // dispatch, then stops accepting new items. Subsequent attempts at placing
   // items into the object via scheduleMessage() will fail with false. Message
   // delivery can be resumed by calling StartMessageDelivery().

   btInt i = 0;

   UnsafeCountUpD d(i);

   EXPECT_TRUE(scheduleMessage(&d));

   StopMessageDelivery();
   YIELD_WHILE(0 == i);

   EXPECT_FALSE(scheduleMessage(&d));
   EXPECT_EQ(1, i);


   StartMessageDelivery();
   EXPECT_TRUE(scheduleMessage(&d));

   YIELD_WHILE(1 == i);
}

