// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

class CAALEvent_f : public AAL::IEvent,
                    public ::testing::Test
{
public:
   virtual btGenericInterface    Interface(btIID ID)           const { return m_pCAALEvent->Interface(ID);    }
   virtual btBool                      Has(btIID ID)           const { return m_pCAALEvent->Has(ID);          }
   virtual btGenericInterface    ISubClass()                   const { return m_pCAALEvent->ISubClass();      }
   virtual btIID                SubClassID()                   const { return m_pCAALEvent->SubClassID();     }
   virtual btBool             operator != (const IEvent &rhs)  const { return m_pCAALEvent->operator !=(rhs); }
   virtual btBool             operator == (const IEvent &rhs)  const { return m_pCAALEvent->operator ==(rhs); }
   virtual IBase &                  Object()                   const { return m_pCAALEvent->Object();         }
   virtual IBase *                 pObject()                   const { return m_pCAALEvent->pObject();        }
   virtual btBool                     IsOK()                   const { return m_pCAALEvent->IsOK();           }
   virtual btApplicationContext    Context()                   const { return m_pCAALEvent->Context();        }
   virtual btApplicationContext SetContext(btApplicationContext Ctx) { return m_pCAALEvent->SetContext(Ctx);  }

   virtual void setHandler(IServiceClient *p) { m_pCAALEvent->setHandler(p); }
   virtual void setHandler(IRuntimeClient *p) { m_pCAALEvent->setHandler(p); }
   virtual void setHandler(btEventHandler  h) { m_pCAALEvent->setHandler(h); }

   EOBJECT SetSubClassInterface(btIID ID, btGenericInterface Ifc) { return m_pCAALEvent->SetSubClassInterface(ID, Ifc); }
   void               SetObject(IBase *pObj)                      { m_pCAALEvent->SetObject(pObj); }
   virtual void        operator()()                               { m_pCAALEvent->operator()();    }

protected:
   CAALEvent_f(btApplicationContext Ctx) :
      m_CAASBase(Ctx),
      m_pCAALEvent(NULL)
   {}

   virtual void SetUp()
   {
      m_pCAALEvent = new(std::nothrow) CAALEvent(&m_CAASBase);
      ASSERT_NONNULL(m_pCAALEvent);
   }
   virtual void TearDown()
   {
      m_pCAALEvent->Delete();
      m_pCAALEvent = NULL;
   }

   void Reset(IBase *pIBase)
   {
      ASSERT_NONNULL(m_pCAALEvent);
      m_pCAALEvent->Delete();

      m_pCAALEvent = new(std::nothrow) CAALEvent(pIBase);
      ASSERT_NONNULL(m_pCAALEvent);
   }

   void Reset(IBase *pIBase, btIID SubClassID)
   {
      ASSERT_NONNULL(m_pCAALEvent);
      m_pCAALEvent->Delete();

      m_pCAALEvent = new(std::nothrow) CAALEvent(pIBase, SubClassID);
      ASSERT_NONNULL(m_pCAALEvent);
   }

   CAASBase   m_CAASBase;
   CAALEvent *m_pCAALEvent;
};

class CAALEvent_f_0 : public CAALEvent_f
{
protected:
   CAALEvent_f_0() :
      CAALEvent_f((btApplicationContext)55)
   {}
};


class CAALEventProtected : public CAALEvent
{
public:
   CAALEventProtected(IBase *p) :
      CAALEvent(p)
   {}
   CAALEventProtected(IBase *pObject, btIID SubClassID) :
      CAALEvent(pObject, SubClassID)
   {}
   ~CAALEventProtected()
   {
      CAALEventProtected::sm_CallLog.ClearLog();
   }

   void    CallUpdateContext() { UpdateContext(); }
   EOBJECT CallSetInterface(btIID ID, btGenericInterface pIfc) { return SetInterface(ID, pIfc); }
   btBool  CallProcessEventTranID() { return ProcessEventTranID(); }

   static void EventHandler(IEvent const & );
   static MethodCallLog sm_CallLog;
};

MethodCallLog CAALEventProtected::sm_CallLog;
void CAALEventProtected::EventHandler(IEvent const &e)
{
   MethodCallLogEntry *l = CAALEventProtected::sm_CallLog.AddToLog("CAALEventProtected::EventHandler");
   l->AddParam("e", reinterpret_cast<void *>( & const_cast<IEvent &>(e) ));
}


TEST_F(CAALEvent_f_0, aal0622)
{
   // CAALEvent::CAALEvent(IBase *pObject) sets an interface pointer for iidCEvent, and sets a SubClass
   // interface of iidEvent. When object initialization is successful, m_bIsOK is set to true. m_pObject
   // stores the input parameter for later retrieval by Object() and pObject().

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_TRUE(IsOK());

   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), pObject());
   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), &Object());
}

TEST_F(CAALEvent_f_0, aal0623)
{
   // CAALEvent::CAALEvent(IBase *pObject) with NULL input IBase pointer , sets an interface
   // pointer for iidCEvent, and sets a SubClass interface of iidEvent. When object initialization
   // is successful, m_bIsOK is set to true. m_pObject stores the NULL input parameter for later
   // retrieval by Object() and pObject().

   Reset(NULL);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_TRUE(IsOK());

   EXPECT_EQ(NULL, pObject());
}

TEST_F(CAALEvent_f_0, aal0624)
{
   // CAALEvent(IBase *pObject, btIID SubClassID) sets an interface pointer for iidCEvent,
   // iidEvent and sets a SubClass interface of SubClassID. When object initialization is
   // successful, m_bIsOK is set to true. m_pObject stores the input parameter for later
   // retrieval by Object() and pObject().

   const btIID ID = 2300;
   Reset(&m_CAASBase, ID);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(ID, SubClassID());
   
   EXPECT_TRUE(IsOK());

   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), pObject());
   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), &Object());
}

TEST_F(CAALEvent_f_0, aal0625)
{
   // When the input parameter to CAALEvent(IBase *pObject, btIID SubClassID)
   // has a matching subclass interface ID, set SubClass interface returns EObjDuplicateName.
   // CAALEvent object doesn't has a subclass interface, m_bIsOK is set to false,
   // m_SubClassID is 0 and m_ISubClass is NULL.   -

   const btIID ID = iidCEvent; // interface conflict

   Reset(&m_CAASBase, ID);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

   EXPECT_EQ(NULL, ISubClass());
   EXPECT_EQ(0,   SubClassID());

   EXPECT_FALSE(IsOK());
   EXPECT_EQ(NULL, pObject());

   EXPECT_EQ(NULL, Context());
}

TEST_F(CAALEvent_f_0, aal0626)
{
   // CAALEvent::CAALEvent(const CAALEvent &rOther) sets an interface pointer for iidCEvent,
   // and sets a SubClass interface of iidEvent. When object initialization is successful,
   // m_bIsOK , m_pObject and m_Context are assigned from rOther stored input parameter
   // for later retrieval by Object() ,pObject(), Context() and IsOK().

   const btIID ID = 2301;
   Reset(&m_CAASBase, ID);

   CAALEvent *copy = new CAALEvent(*m_pCAALEvent);
   ASSERT_NONNULL(copy);

   EXPECT_TRUE(copy->Has(iidCEvent));
   EXPECT_EQ(copy, reinterpret_cast<CAALEvent *>( copy->Interface(iidCEvent) ));

   EXPECT_TRUE(copy->Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(copy), reinterpret_cast<IEvent *>( copy->Interface(iidEvent) ));

   EXPECT_EQ(dynamic_cast<IEvent *>(copy), reinterpret_cast<IEvent *>(copy->ISubClass()));
   EXPECT_EQ(iidEvent, copy->SubClassID());

   EXPECT_TRUE(copy->IsOK());

   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase),  copy->pObject());
   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), &copy->Object());

   EXPECT_EQ(Context(), copy->Context());

   copy->Delete();
}

TEST_F(CAALEvent_f_0, aal0627)
{
   // When interface corresponding to the input parameter is implemented,
   // CAALEvent::Interface(btIID Interface) returns a pointer to the requested interface.

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));
}

TEST_F(CAALEvent_f_0, aal628)
{
   // When interface corresponding to the input parameter is not implemented,
   // CAALEvent::Interface(btIID Interface) returns NULL.

   const btIID ID = 0x299;
   EXPECT_NULL(Interface(ID));
}

TEST(CAALEventTest, aal0629)
{
   // When the input parameter to SetInterface () are new interface ID and non-NULL
   // interface pointer on the object, CAALEvent::SetInterface() returns EObjOK.

   CAASBase b;
   CAALEventProtected e(&b);

   const btIID ID = 0x299;
   const btGenericInterface pIfc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, e.CallSetInterface(ID, pIfc));

   EXPECT_TRUE(e.Has(ID));
   EXPECT_EQ(pIfc, e.Interface(ID));
}

TEST(CAALEventTest, aal0630)
{
   // When the input parameter to SetInterface() is NULL,
   // CAALEvent::SetInterface() returns EObjBadObject.

   CAASBase b;
   CAALEventProtected e(&b);

   const btIID ID = 0x299;

   EXPECT_FALSE(e.Has(ID));

   EXPECT_EQ(EObjBadObject, e.CallSetInterface(ID, NULL));
   EXPECT_FALSE(e.Has(ID));
}

TEST(CAALEventTest, aal0631)
{
   // When the btIID input parameter to SetInterface(btIID , btGenericInterface ) is
   // already implemented interface, CAALEvent::SetInterface() returns EObjDuplicateName.

   CAASBase b;
   CAALEventProtected e(&b);

   btGenericInterface Ifc = (btGenericInterface)99;

   EXPECT_EQ(EObjDuplicateName, e.CallSetInterface(iidEvent,  Ifc));
   EXPECT_EQ(EObjDuplicateName, e.CallSetInterface(iidCEvent, Ifc));

   const btIID ID = 0x299;

   EXPECT_FALSE(e.Has(ID));
   EXPECT_EQ(EObjOK,            e.CallSetInterface(ID, Ifc));
   EXPECT_TRUE(e.Has(ID));
   Ifc = (btGenericInterface)100;
   EXPECT_EQ(EObjDuplicateName, e.CallSetInterface(ID, Ifc));
}

TEST_F(CAALEvent_f_0, aal0632)
{
   // When set a sub class interface pointer to a non-NULL subclass object ( CAALEvent ::SetSubClassInterface()),
   // m_SubClassID stores subclass id and m_ISubClass stores interface pointer and returns EObjOK.

   const btIID ID = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

   EXPECT_TRUE(Has(ID));
   EXPECT_EQ(ID, SubClassID());
   EXPECT_EQ(Ifc, ISubClass());
}

TEST_F(CAALEvent_f_0, aal0633)
{
   // When the input interface pointer parameter to SetSubClassInterface() is NULL,
   // CAALEvent::SetSubClassInterface() returns EObjBadObject.

   const btIID ID = 988;
   EXPECT_EQ(EObjBadObject, SetSubClassInterface(ID, NULL));
   EXPECT_FALSE(Has(ID));
}

TEST_F(CAALEvent_f_0, aal0634)
{
   // When try to set already implemented sub class interface,
   // CAALEvent::SetSubClassInterface() returns EObjDuplicateName.

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_EQ(EObjDuplicateName, SetSubClassInterface(iidEvent, (btGenericInterface)3));

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(iidEvent, SubClassID());

   const btIID              ID  = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;

   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

   EXPECT_EQ(Ifc, ISubClass());
   EXPECT_EQ(ID, SubClassID());

   EXPECT_EQ(EObjDuplicateName, SetSubClassInterface(iidCEvent, (btGenericInterface)4));

   EXPECT_EQ(Ifc, ISubClass());
   EXPECT_EQ(ID, SubClassID());
}

TEST_F(CAALEvent_f_0, aal0635)
{
   // When the CAALEvent object has an interface matching the given
   // interface ID, CAALEvent::Has() returns true.

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_TRUE(Has(iidCEvent));
}

TEST_F(CAALEvent_f_0, aal0636)
{
   // When no interface matching the given interface ID is found,
   // CAALEvent::Has() returns false.

   const btIID ID = 8675309;
   EXPECT_FALSE(Has(ID));
}

TEST_F(CAALEvent_f_0, aal0637)
{
   // ISubClass() returns the latest/ recent cached subclass pointer
   // which set by SetSubClassInterface().

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
   EXPECT_EQ(iidEvent, SubClassID());

   const btIID ID = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

   EXPECT_EQ(Ifc, ISubClass());
   EXPECT_EQ(ID, SubClassID());
}

TEST_F(CAALEvent_f_0, aal0638)
{
   // SubClassID() returns stored subclass ID in m_SubClassID,
   // set by CAALEvent::SetSubClassInterface()

   EXPECT_EQ(iidEvent, SubClassID());
}

TEST(CAALEventTest, aal0639)
{
   // When both are (lhs and rhs) CAALEvent instances, same number and  type of interfaces, and
   // implement the same SubClass, operator == returns true and != returns false.

   CAASBase base;
   CAALEventProtected a(&base);
   CAALEventProtected b(&base);

   EXPECT_TRUE(a == a);
   EXPECT_FALSE(a != a);

   EXPECT_TRUE(a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE(b == a);
   EXPECT_FALSE(b != a);
}

TEST(CAALEventTest, aal0640)
{
   // When one of lhs and rhs does not implement iidCEvent, CAALEvent::operator ==() returns
   // false and CAALEvent::operator !=() returns true.

   CAASBase base;

   CAALEventProtected lhs(&base);

   class aal0640Event : public IEvent
   {
   public:
      aal0640Event(IBase &b) :
         m_IBase(b),
         m_SubClassIfc( reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(this) ) )
      {}
      btGenericInterface    Interface(btIID Interface)   const { return NULL;     }
      btBool                      Has(btIID Interface)   const { return false;    }
      btGenericInterface    ISubClass()                  const { return m_SubClassIfc; }
      btIID                SubClassID()                  const { return iidEvent; }
      btBool             operator != (const IEvent &rhs) const { return true;     }
      btBool             operator == (const IEvent &rhs) const { return false;    }
      btBool                     IsOK()                  const { return true;     }
      btApplicationContext    Context()                  const { return NULL;     }
      IBase &                  Object()                  const { return m_IBase;  }
      IBase *                 pObject()                  const { return &m_IBase; }
      btApplicationContext SetContext(btApplicationContext Ctx) { return NULL;     }

   protected:
      IBase             &m_IBase;
      btGenericInterface m_SubClassIfc;
   } rhs(base);

   EXPECT_FALSE(lhs == rhs);
   EXPECT_TRUE(lhs != rhs);
}

TEST(CAALEventTest, aal0641)
{
   // When both lhs and rhs implement iidCEvent, but do not implement the same SubClass,
   // CAALEvent::operator ==() returns false and CAALEvent::operator !=() returns true.

   CAASBase base;

   CAALEventProtected a(&base);
   CAALEventProtected b(&base);

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   const btIID ID = 998;
   a.SetSubClassInterface(ID, (btGenericInterface)7);

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}

TEST(CAALEventTest, aal0642)
{
   // When both lhs and rhs are CAALEvent instances, implement the same SubClass, but do not
   // implement the same number of interfaces, CAALEvent::operator ==() returns false and
   // CAALEvent::operator !=() returns true.

   CAASBase base;

   CAALEventProtected a(&base);
   CAALEventProtected b(&base);

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   const btIID ID = 998;
   a.CallSetInterface(ID, (btGenericInterface)7);

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}

TEST(CAALEventTest, aal0643)
{
   // When both lhs and rhs implement iidCEvent, implement the same SubClass, and implement same
   // number of interfaces, but implement different interface types, CAALEvent::operator ==() returns
   // false and CAALEvent::operator !=() returns true.

   CAASBase base;

   CAALEventProtected a(&base);
   CAALEventProtected b(&base);

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   const btGenericInterface ifc = (btGenericInterface)3;

   a.CallSetInterface((btIID)998, ifc);
   b.CallSetInterface((btIID)999, ifc);

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}

TEST_F(CAALEvent_f_0, aal0644)
{
   // SetObject() Updates the object pointed to by the Event,
   // m_pObject stores updated cached object pointer.

   EXPECT_EQ(&m_CAASBase, pObject());
   EXPECT_EQ(&m_CAASBase, &Object());

   CAASBase a;

   SetObject(&a);
   EXPECT_EQ(&a, pObject());
   EXPECT_EQ(&a, &Object());

   SetObject(&m_CAASBase);
   EXPECT_EQ(&m_CAASBase, pObject());
}

TEST_F(CAALEvent_f_0, aal0645)
{
   // UpdateContext() updates application context of based on
   // m_pObject(cache the object context).

   EXPECT_EQ((btApplicationContext)55, Context());

   CAASBase base((btApplicationContext)56);
   Reset(&base);

   EXPECT_EQ((btApplicationContext)56, Context());
}

#if 0
Redmine 542
TEST_F(CAALEvent_f_1, aal0646)
{
   //When m_pObject(cache the object context) is NULL, UpdateContext()
   //doesn't update application context .

   const btApplicationContext appCtx =(btApplicationContext)55;
   m_Context = appCtx;
   // m_pObject is NULL
   EXPECT_NULL(m_pObject);
   UpdateContext();
    // doesn't update ,old application context
   EXPECT_EQ(appCtx,m_Context);
}

TEST_F(CAALEvent_f_0, aal0647)
{
   //When m_pObject.IsOK() is false, UpdateContext() doesn't update application context .

   class aal0647Base : public IBase
   {
   public:
     aal0647Base() {}
     btGenericInterface Interface(btIID Interface) const  { return NULL;  }
     btBool Has(btIID Interface) const                    { return false; }
     btGenericInterface ISubClass() const                 { return NULL;  }
     btIID SubClassID() const                             { return 0;     }
     btBool operator != (IBase const &rother) const       { return true;  }
     btBool operator == (IBase const &rother) const       { return false; }
     btBool IsOK() const                                  { return false; }
     btApplicationContext Context() const                 { return NULL;  }
   } ;
   aal0647Base base;

   SetObject(&base);
   const btApplicationContext appCtx =(btApplicationContext)24;
   SetContext(appCtx);
   EXPECT_FALSE(m_pObject->IsOK());

   //UpdateContext() doesn't update application context .
   UpdateContext();
   EXPECT_EQ(appCtx,m_Context);
   EXPECT_EQ(appCtx,Context());
}
#endif

#if 0
TEST_F(CAALEvent_f_0, aal0649)
{
   //When transaction ID 's Event Handler is NULL , ProcessEventTranID() returns false.
   EXPECT_NULL(m_TranID.Handler());
   EXPECT_FALSE(ProcessEventTranID());
}

TEST_F(CAALEvent_f_0, aal0650)
{
   //When transaction ID 's Event Handler is valid , ProcessEventTranID()
   //invokes event handler callback and returns true.
   class AALEventClient
      {
      public:
      AALEventClient() {}
      static void btEventHandlertest(IEvent const &TheEvent) { }
      };

   AALEventClient eventclient;
   m_TranID.Handler(eventclient.btEventHandlertest);
   EXPECT_TRUE(ProcessEventTranID());
}
#endif

TEST_F(CAALEvent_f_0, aal0651)
{
   // Object() returns a reference to the object associated with the event.
   // pObject() returns a pointer to the associated object.

   EXPECT_EQ(&m_CAASBase, &Object());
   EXPECT_EQ(&m_CAASBase, pObject());

   CAASBase a;
   SetObject(&a);

   EXPECT_EQ(&a, &Object());
   EXPECT_EQ(&a, pObject());

   SetObject(NULL);
   EXPECT_NULL(pObject());
}

#if 0
TEST_F(CAALEvent_f_0, aal0653)
{
   //Context() returns pointer to application context of based on
   //m_pObject(cache the object context).

   EXPECT_EQ(m_Context,Context());
}

TEST_F(CAALEvent_f_0, aal0654)
{
   //SetContext(btApplicationContext Ctx) stores input put context and
   //returns previous stored context.

   btApplicationContext appCtx = m_Context;
   btApplicationContext newappCtx = (btApplicationContext)0xff55;

   EXPECT_EQ(appCtx,SetContext(newappCtx));
   EXPECT_EQ(newappCtx,m_Context);
   EXPECT_EQ(newappCtx,Context());
}
#endif


TEST(CAALEventTest, aal0655)
{
   // CAALEvent::setHandler(IServiceClient * ) updates m_pServiceClient. When m_pServiceClient
   // is non-NULL, subsequent calls to CAALEvent::operator()() invoke IServiceClient::serviceEvent(),
   // passing this as the argument. The event is then deleted.

   CAASBase base;
   // new'ing the event, because operator()() with an IServiceClient deletes it.
   CAALEventProtected *pEvent = new CAALEventProtected(&base);
   const IEvent &cr(*pEvent);
   void *Expect = reinterpret_cast<void *>( & const_cast<IEvent &>(cr) );

   CallTrackingServiceClient client;

   pEvent->setHandler(&client);

   pEvent->operator()(); // pEvent is deleted.

   ASSERT_EQ(1, client.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", client.Entry(0).MethodName().c_str());
   EXPECT_EQ(Expect, client.Entry(0).ParamValue(0));
}

TEST(CAALEventTest, aal0656)
{
   // CAALEvent::setHandler(IRuntimeClient * ) updates m_pRuntimeClient. When m_pRuntimeClient
   // is non-NULL, subsequent calls to CAALEvent::operator()() invoke IRuntimeClient::runtimeEvent(),
   // passing this as the argument. The event is then deleted.

   CAASBase base;
   // new'ing the event, because operator()() with an IRuntimeClient deletes it.
   CAALEventProtected *pEvent = new CAALEventProtected(&base);
   const IEvent &cr(*pEvent);
   void *Expect = reinterpret_cast<void *>( & const_cast<IEvent &>(cr) );

   CallTrackingRuntimeClient client;

   pEvent->setHandler(&client);

   pEvent->operator()(); // pEvent is deleted.

   ASSERT_EQ(1, client.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", client.Entry(0).MethodName().c_str());
   EXPECT_EQ(Expect, client.Entry(0).ParamValue(0));
}

TEST(CAALEventTest, aal0657)
{
   // CAALEvent::setHandler(btEventHandler ) updates m_pEventHandler. When m_pEventHandler is
   // non-NULL, subsequent calls to CAALEvent::operator()() invoke the event handler callback.

   CAASBase base;
   CAALEventProtected e(&base);
   IEvent const &cr(e);
   void *Expect = reinterpret_cast<void *>( & const_cast<IEvent &>(cr) );

   e.setHandler(CAALEventProtected::EventHandler);
   e.operator()();

   ASSERT_EQ(1, CAALEventProtected::sm_CallLog.LogEntries());
   EXPECT_STREQ("CAALEventProtected::EventHandler", CAALEventProtected::sm_CallLog.Entry(0).MethodName().c_str());
   EXPECT_EQ(Expect, CAALEventProtected::sm_CallLog.Entry(0).ParamValue(0));
}

