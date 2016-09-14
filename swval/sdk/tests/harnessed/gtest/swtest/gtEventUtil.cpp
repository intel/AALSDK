// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtCommon_Mocks.h"
#include <iostream>
#include "aalsdk/utils/AALEventUtilities.h"

static btInt events_handled = 0;

class CCustomBase : public CAALBase
{
public:
   static void handleEvents(IEvent const& theEvent)
   {
      ASSERT_TRUE(PrintExceptionDescription(theEvent));
      events_handled++;
   }

   virtual void Destroy(const TransactionID& id)
   {
   }

   CCustomBase() : CAALBase(&CCustomBase::handleEvents)
   {
   }
};

class TestRuntime : public IRuntime
{
public:
   virtual btBool start(const NamedValueSet& rconfigParms)
   {
      return false;
   }
   virtual void stop()
   {
   }
   virtual void allocService(IBase* pClient,
                             NamedValueSet const& rManifest,
                             TransactionID const& rTranID = TransactionID())
   {
   }
   virtual btBool schedDispatchable(IDispatchable* pDispatchable)
   {
      pDispatchable->operator()();
      return false;
   }
   virtual IRuntime* getRuntimeProxy(IRuntimeClient* pClient)
   {
      return NULL;
   }
   virtual btBool releaseRuntimeProxy()
   {
      return false;
   }
   virtual IRuntimeClient* getRuntimeClient()
   {
      return NULL;
   }
   virtual btBool IsOK()
   {
      return false;
   }
   virtual ~TestRuntime()
   {
   }
};

class CAALEventUtilities_f : public ::testing::Test
{
   static const btIID custom_id
      = AAL_ExceptionEvent(AAL_vendAAL, AAL_sysAny, 0X0B0BFEAD);

public:
   CAALEventUtilities_f()
      : m_Base()
      , sEHandler(&CCustomBase::handleEvents)
   {
   }

   ~CAALEventUtilities_f()
   {
   }

   static const btIID GetCustomIID()
   {
      return custom_id;
   }

protected:
   CCustomBase m_Base;
   btEventHandler sEHandler;
};

TEST_F(CAALEventUtilities_f, aal0807)
{
   /// ========================================================================
   /// @test         Pass a CEvent to PrintExceptionDescription.
   ///
   /// @brief        Expect false return because the event object is not an
   ///               exception.

   CAALEvent* pEvent = new (std::nothrow) CAALEvent(&m_Base);
   EXPECT_FALSE(pEvent->Has(iidExEvent));
   EXPECT_FALSE(pEvent->Has(iidExTranEvent));
   EXPECT_FALSE(AAL_IS_EXCEPTION(pEvent->SubClassID()));
   EXPECT_FALSE(PrintExceptionDescription(*pEvent));
   pEvent->Delete();
}

TEST_F(CAALEventUtilities_f, aal0808)
{
   /// ========================================================================
   /// @test         Pass a CExceptionEvent to PrintExceptionDescription.
   ///
   /// @brief        Expect true return, after logging the exception.

   CExceptionEvent theEvent(&m_Base, 0, 0, "basic exception event");

   EXPECT_TRUE(theEvent.Has(iidExEvent));
   EXPECT_TRUE(AAL_IS_EXCEPTION(theEvent.SubClassID()));
   EXPECT_TRUE(PrintExceptionDescription(theEvent));
}

TEST_F(CAALEventUtilities_f, aal0809)
{
   /// ========================================================================
   /// @test         Pass a CTransactionExceptionEvent object to
   ///               PrintExceptionDescription.
   ///
   /// @brief        Expect true return, after logging the exception.

   const TransactionID& tid = TransactionID();

   CExceptionTransactionEvent theEvent(
      &m_Base, tid, 0, 0, "basic exception transaction event");

   EXPECT_FALSE(theEvent.Has(iidExEvent));
   EXPECT_TRUE(theEvent.Has(iidExTranEvent));
   EXPECT_TRUE(PrintExceptionDescription(theEvent));
}

TEST_F(CAALEventUtilities_f, aal0810)
{
   /// ================================================================
   /// @test         Pass a CAALEvent with a custom interface ID to
   ///               PrintExceptionDescription.
   ///
   /// @brief        Expect true return, following log of an unknown
   ///               exception event message.
   ///
   /// @details      Event has no transaction ID, but it is an event
   ///               (i.e. derives from IEvent) and is an exception
   ///               (i.e. has the exception bit set and returns true
   ///               from the macro: AAL_IS_EXCEPTION(btIID)).

   CAALEvent* pEvent = new (std::nothrow) CAALEvent(&m_Base);

   EXPECT_EQ(EObjOK, pEvent->SetSubClassInterface(GetCustomIID(), &pEvent));

   EXPECT_FALSE(pEvent->Has(iidExEvent));
   EXPECT_FALSE(pEvent->Has(iidExTranEvent));
   EXPECT_TRUE(AAL_IS_EXCEPTION(pEvent->SubClassID()));

   IExceptionTransactionEvent* trycast_p;
   trycast_p = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, *pEvent);
   EXPECT_NULL(trycast_p);

   EXPECT_TRUE(PrintExceptionDescription(*pEvent));
   pEvent->Delete();
}

TEST_F(CAALEventUtilities_f, aal0812)
{
   /// ========================================================================
   /// @test         Pass a CExceptionTransactionEvent object to the ReThrow and
   ///               UnWrapAndReThrow event utility functions.
   ///
   /// @brief        Expect events_handled to be incremented by two, step one.
   ///
   /// @details      Re-throw event to a static handler in the base object,
   ///               using a dummy runtime implementation.

   const TransactionID& tid = TransactionID();

   CExceptionTransactionEvent theEvent(
      &m_Base, tid, 0, 0, "basic exception transaction event");

   // put the original TransacdtionID in the context slot
   TransactionID wrappedTID = WrapTransactionID(tid);
   EXPECT_NE(wrappedTID.ID(), tid.ID());

   TransactionID* pContext = reinterpret_cast
      <TransactionID*>(wrappedTID.Context());

   // exercise the equals (==) operator
   EXPECT_EQ(pContext->ID(), tid.ID());
   EXPECT_EQ(*pContext, tid);

   theEvent.SetTranID(wrappedTID);

   TestRuntime* tr = new (std::nothrow) TestRuntime();
   IRuntime* rt = reinterpret_cast<IRuntime*>(tr);

   ASSERT_NONNULL(sEHandler);

   ReThrow(&m_Base, theEvent, rt, sEHandler, &wrappedTID);
   EXPECT_EQ(1, events_handled);
   UnWrapAndReThrow(&m_Base, theEvent, rt, sEHandler);
   EXPECT_EQ(2, events_handled);
   delete tr;
}
