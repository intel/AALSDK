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

   btBool ProcessEventTranID() { return false; }

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
   // CAALEvent::CAALEvent(IBase *pObject) sets an interface pointer for iidCEvent and
   // iidEvent. When object initialization is successful, m_bIsOK is set to true. m_pObject
   // stores the input parameter for later retrieval by Object() and pObject().

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

#if DEPRECATED
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
#endif // DEPRECATED
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_TRUE(IsOK());

   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), pObject());
   EXPECT_EQ(dynamic_cast<IBase *>(&m_CAASBase), &Object());
}

TEST_F(CAALEvent_f_0, aal0623)
{
   // CAALEvent::CAALEvent(IBase *pObject) with NULL input IBase pointer sets interface
   // pointers for iidCEvent and iidEvent. When object initialization
   // is successful, m_bIsOK is set to true. m_pObject stores the NULL input parameter for later
   // retrieval by Object() and pObject().

   Reset(NULL);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

#if DEPRECATED
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
#endif // DEPRECATED
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_TRUE(IsOK());

   EXPECT_EQ(NULL, pObject());
}

TEST_F(CAALEvent_f_0, aal0624)
{
   // CAALEvent(IBase *pObject, btIID SubClassID) sets interface pointers for iidCEvent,
   // iidEvent, and SubClassID. When object initialization is
   // successful, m_bIsOK is set to true. m_pObject stores the input parameter for later
   // retrieval by Object() and pObject().

   const btIID ID = 2300;
   Reset(&m_CAASBase, ID);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(ID) ));
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
   // m_SubClassID is 0.   -

   const btIID ID = iidCEvent; // interface conflict

   Reset(&m_CAASBase, ID);

   EXPECT_TRUE(Has(iidCEvent));
   EXPECT_EQ(m_pCAALEvent, reinterpret_cast<CAALEvent *>( Interface(iidCEvent) ));

   EXPECT_TRUE(Has(iidEvent));
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>( Interface(iidEvent) ));

#if DEPRECATED
   EXPECT_EQ(NULL, ISubClass());
#endif // DEPRECATED
   EXPECT_EQ(0,   SubClassID());

   EXPECT_FALSE(IsOK());
   EXPECT_EQ(NULL, pObject());

   EXPECT_EQ(NULL, Context());
}

#if 0
Copies were de-featured.
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
#endif

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
   // when set a sub class interface pointer to a non-NULL subclass object (CAALEvent ::SetSubClassInterface()),
   // m_SubClassID stores subclass id and an interface entry is made for the parameters in the
   // interface map. The function returns EObjOK.

   const btIID ID = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

   EXPECT_TRUE(Has(ID));
   EXPECT_EQ(ID, SubClassID());
#if DEPRECATED
   EXPECT_EQ(Ifc, ISubClass());
#endif // DEPRECATED
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

#if DEPRECATED
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
#endif // DEPRECATED
   EXPECT_EQ(iidEvent, SubClassID());

   EXPECT_EQ(EObjDuplicateName, SetSubClassInterface(iidEvent, (btGenericInterface)3));

#if DEPRECATED
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
#endif // DEPRECATED
   EXPECT_EQ(iidEvent, SubClassID());

   const btIID              ID  = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;

   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

#if DEPRECATED
   EXPECT_EQ(Ifc, ISubClass());
#endif // DEPRECATED
   EXPECT_EQ(ID, SubClassID());

   EXPECT_EQ(EObjDuplicateName, SetSubClassInterface(iidCEvent, (btGenericInterface)4));

#if DEPRECATED
   EXPECT_EQ(Ifc, ISubClass());
#endif // DEPRECATED
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

#if DEPRECATED
TEST_F(CAALEvent_f_0, aal0637)
{
   // ISubClass() returns the latest/ recent cached subclass pointer
   // which set by SetSubClassInterface().

#if DEPRECATED
   EXPECT_EQ(dynamic_cast<IEvent *>(m_pCAALEvent), reinterpret_cast<IEvent *>(ISubClass()));
#endif // DEPRECATED
   EXPECT_EQ(iidEvent, SubClassID());

   const btIID ID = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

#if DEPRECATED
   EXPECT_EQ(Ifc, ISubClass());
#endif // DEPRECATED
   EXPECT_EQ(ID, SubClassID());
}
#endif // DEPRECATED

TEST_F(CAALEvent_f_0, aal0638)
{
   // SubClassID() returns stored subclass ID in m_SubClassID,
   // set by CAALEvent::SetSubClassInterface()

   EXPECT_EQ(iidEvent, SubClassID());

   const btIID ID = 988;
   const btGenericInterface Ifc = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetSubClassInterface(ID, Ifc));

   EXPECT_EQ(ID, SubClassID());
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
      btGenericInterface    Interface(btIID Interface)   const  { return NULL;     }
      btBool                      Has(btIID Interface)   const  { return false;    }
      btIID                SubClassID()                  const  { return iidEvent; }
      btBool             operator != (const IEvent &rhs) const  { return true;     }
      btBool             operator == (const IEvent &rhs) const  { return false;    }
      btBool                     IsOK()                  const  { return true;     }
      btApplicationContext    Context()                  const  { return NULL;     }
      IBase &                  Object()                  const  { return m_IBase;  }
      IBase *                 pObject()                  const  { return &m_IBase; }
      btApplicationContext SetContext(btApplicationContext Ctx) { return NULL;     }

   protected:
      btBool       ProcessEventTranID()                         { return false;    }

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

   CallTrackingIServiceClient client;

   pEvent->setHandler(&client);

   pEvent->operator()(); // pEvent is deleted.

   ASSERT_EQ(1, client.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", client.Entry(0).MethodName());

   btObjectType x = NULL;
   client.Entry(0).GetParam("e", &x);
   EXPECT_EQ(Expect, x);
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

   CallTrackingIRuntimeClient client;

   pEvent->setHandler(&client);

   pEvent->operator()(); // pEvent is deleted.

   ASSERT_EQ(1, client.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", client.Entry(0).MethodName());

   btObjectType x = NULL;
   client.Entry(0).GetParam("e", &x);
   EXPECT_EQ(Expect, x);
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
   EXPECT_STREQ("CAALEventProtected::EventHandler",
                CAALEventProtected::sm_CallLog.Entry(0).MethodName());

   btObjectType x = NULL;
   CAALEventProtected::sm_CallLog.Entry(0).GetParam("e", &x);
   EXPECT_EQ(Expect, x);
}

////////////////////////////////////////////////////////////////////////////////

TEST(CTransactionEventTest, aal0662)
{
   // CTransactionEvent(IBase * , TransactionID const & ) constructs CAALEvent with the IBase *,
   // and stores the TransactionID for later retrieval by TranID(). On success, a SubClass of
   // iidTranEvent/ITransactionEvent * is set.

   CAASBase      base;
   TransactionID tid;

   tid.ID(7);

   CTransactionEvent e(&base, tid);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ(7, e.TranID().ID());

   EXPECT_EQ(iidTranEvent, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.IsOK());
}

TEST(CTransactionEventTest, aal0663)
{
   // CTransactionEvent(IBase * , btIID ID, TransactionID const & ) constructs CAALEvent with the
   // IBase *, and stores the TransactionID for later retrieval by TranID(). On success, a SubClass
   // of ID/ITransactionEvent * is set. Interface iidTranEvent/ITransactionEvent * is also added.

   CAASBase      base;
   TransactionID tid;

   tid.ID(7);

   const btIID ID = 998;

   CTransactionEvent e(&base, ID, tid);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ(7, e.TranID().ID());

   EXPECT_EQ(ID, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&e) ), e.Interface(iidTranEvent));

   EXPECT_TRUE(e.IsOK());
}

#if 0
Copies were de-featured.
TEST(CTransactionEventTest, aal0664)
{
   // CTransactionEvent(const CTransactionEvent & ) (copy constructor) creates an exact copy
   // of the given CTransactionEvent.

   CAASBase      baseA((btApplicationContext)71);
   TransactionID tidA;

   tidA.ID(8);

   CTransactionEvent a(&baseA, tidA);

   a.setHandler(CAALEventProtected::EventHandler);
   EXPECT_TRUE(a.IsOK());

   CTransactionEvent copyA(PassReturnByValue(a));

   IEvent const &crA(copyA);
   void *ExpectA = reinterpret_cast<void *>( & const_cast<IEvent &>(crA) );

   EXPECT_TRUE(copyA.Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(&copyA) ), copyA.Interface(iidEvent));

   EXPECT_TRUE(copyA.Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(&copyA) ), copyA.Interface(iidCEvent));

   EXPECT_TRUE(copyA.Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&copyA) ), copyA.Interface(iidTranEvent));

   EXPECT_EQ(iidTranEvent, copyA.SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&copyA) ), copyA.ISubClass());

   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), &copyA.Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), copyA.pObject());

   EXPECT_TRUE(copyA.IsOK());
   EXPECT_EQ((btApplicationContext)71, copyA.Context());

   EXPECT_EQ(8, copyA.TranID().ID());

   EXPECT_TRUE(a == copyA);
   EXPECT_FALSE(a != copyA);

   EXPECT_TRUE(copyA == a);
   EXPECT_FALSE(copyA != a);

   ASSERT_EQ(0, CAALEventProtected::sm_CallLog.LogEntries());

   copyA.operator()();

   ASSERT_EQ(1, CAALEventProtected::sm_CallLog.LogEntries());
   EXPECT_STREQ("CAALEventProtected::EventHandler", CAALEventProtected::sm_CallLog.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectA, CAALEventProtected::sm_CallLog.Entry(0).ParamValue(0));
   CAALEventProtected::sm_CallLog.ClearLog();



   CAASBase      baseB((btApplicationContext)72);
   TransactionID tidB;
   const btIID   IDB = 998;

   tidB.ID(9);

   CTransactionEvent b(&baseB, IDB, tidB);

   CallTrackingServiceClient clientB;

   b.setHandler(&clientB);
   EXPECT_TRUE(b.IsOK());

   // new'ing the event, because operator()() with IServiceClient will delete it.
   CTransactionEvent *copyB = new CTransactionEvent(PassReturnByValue(b));

   IEvent const &crB(*copyB);
   void *ExpectB = reinterpret_cast<void *>( & const_cast<IEvent &>(crB) );

   EXPECT_TRUE(copyB->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyB) ), copyB->Interface(iidEvent));

   EXPECT_TRUE(copyB->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyB) ), copyB->Interface(iidCEvent));

   EXPECT_TRUE(copyB->Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyB) ), copyB->Interface(iidTranEvent));

#if 0
   EXPECT_TRUE(copyB->Has(IDB));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyB) ),
             copyB->Interface(IDB));

   EXPECT_EQ(IDB, copyB->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyB) ),
             copyB->ISubClass());

   EXPECT_TRUE(b == *copyB);
   EXPECT_FALSE(b != *copyB);

   EXPECT_TRUE(*copyB == b);
   EXPECT_FALSE(*copyB != b);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), &copyB->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), copyB->pObject());

   EXPECT_TRUE(copyB->IsOK());
   EXPECT_EQ((btApplicationContext)72, copyB->Context());

   EXPECT_EQ(9, copyB->TranID().ID());

   ASSERT_EQ(0, clientB.LogEntries());

   copyB->operator()(); // Delete's copyB.

   ASSERT_EQ(1, clientB.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", clientB.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectB, clientB.Entry(0).ParamValue(0));



   CAASBase      baseC((btApplicationContext)73);
   TransactionID tidC;
   const btIID   IDC = 999;

   tidC.ID(10);

   CTransactionEvent c(&baseC, IDC, tidC);

   CallTrackingRuntimeClient clientC;

   c.setHandler(&clientC);
   EXPECT_TRUE(c.IsOK());

   // new'ing the event, because operator()() with IRuntimeClient will delete it.
   CTransactionEvent *copyC = new CTransactionEvent(PassReturnByValue(c));

   IEvent const &crC(*copyC);
   void *ExpectC = reinterpret_cast<void *>( & const_cast<IEvent &>(crC) );

   EXPECT_TRUE(copyC->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyC) ), copyC->Interface(iidEvent));

   EXPECT_TRUE(copyC->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyC) ), copyC->Interface(iidCEvent));

   EXPECT_TRUE(copyC->Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyC) ), copyC->Interface(iidTranEvent));

#if 0
   EXPECT_TRUE(copyC->Has(IDC));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyC) ),
             copyC->Interface(IDC));

   EXPECT_EQ(IDC, copyC->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(copyC) ),
             copyC->ISubClass());

   EXPECT_TRUE(c == *copyC);
   EXPECT_FALSE(c != *copyC);

   EXPECT_TRUE(*copyC == c);
   EXPECT_FALSE(*copyC != c);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), &copyC->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), copyC->pObject());

   EXPECT_TRUE(copyC->IsOK());
   EXPECT_EQ((btApplicationContext)73, copyC->Context());

   EXPECT_EQ(10, copyC->TranID().ID());

   ASSERT_EQ(0, clientC.LogEntries());

   copyC->operator()(); // Delete's copyC.

   ASSERT_EQ(1, clientC.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", clientC.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectC, clientC.Entry(0).ParamValue(0));
}
#endif

TEST(CTransactionEventTest, aal0665)
{
   // CTransactionEvent::SetTranID() mutates the TransactionID that is accessed by
   // CTransactionEvent::TranID().

   CAASBase      baseA((btApplicationContext)71);
   TransactionID tidA;

   tidA.ID(8);

   CTransactionEvent a(&baseA, tidA);

   EXPECT_EQ(8, a.TranID().ID());

   TransactionID tidB;

   tidB.ID(9);

   a.SetTranID(tidB);

   EXPECT_EQ(9, a.TranID().ID());


   CAASBase baseB((btApplicationContext)72);
   const btIID IDB = 998;

   tidB.ID(10);

   CTransactionEvent b(&baseB, IDB, tidB);

   EXPECT_EQ(10, b.TranID().ID());

   tidB.ID(11);

   b.SetTranID(tidB);
   EXPECT_EQ(11, b.TranID().ID());
}

////////////////////////////////////////////////////////////////////////////////

TEST(CExceptionEventTest, aal0666)
{
   // CExceptionEvent(IBase * , btID , btID , btcString ) constructs CAALEvent with the IBase *,
   // and stores the ExceptionNumber, Reason, and Description for later retrieval by data accessor
   // member fn's of the same name. On success, a SubClass if iidExEvent/IExceptionEvent * is set.

   CAASBase base((btApplicationContext)101);

   const btID ExNum  = 12;
   const btID Reason = 13;
   btcString  Descr  = "aal0666";

   CExceptionEvent e(&base, ExNum, Reason, Descr);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ((btApplicationContext)101, e.Context());

   EXPECT_EQ(ExNum,    e.ExceptionNumber());
   EXPECT_EQ(Reason,   e.Reason());
   EXPECT_STREQ(Descr, e.Description());

   EXPECT_EQ(iidExEvent, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.IsOK());
}

TEST(CExceptionEventTest, aal0667)
{
   // CExceptionEvent(IBase * , btIID , btID , btID , btcString ) constructs CAALEvent with the
   // IBase *, and stores the ExceptionNumber, Reason, and Description for later retrieval by data
   // accessor member fn's of the same name. On success, a SubClass of ID/IExceptionEvent * is set.
   // Interface iidExEvent/IExceptionEvent * is also added.

   CAASBase base((btApplicationContext)102);

   const btIID ID     = 14;
   const btID  ExNum  = 15;
   const btID  Reason = 16;
   btcString   Descr  = "aal0667";

   CExceptionEvent e(&base, ID, ExNum, Reason, Descr);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ((btApplicationContext)102, e.Context());

   EXPECT_EQ(ExNum,    e.ExceptionNumber());
   EXPECT_EQ(Reason,   e.Reason());
   EXPECT_STREQ(Descr, e.Description());

   EXPECT_EQ(ID, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.Has(iidExEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(&e) ), e.Interface(iidExEvent));

   EXPECT_TRUE(e.IsOK());
}

#if 0
Copies were de-featured.
TEST(CExceptionEventTest, aal0668)
{
   // CExceptionEvent(const CExceptionEvent& ) (copy constructor) creates an exact copy of the
   // given CExceptionEvent.

   CAASBase   baseA((btApplicationContext)71);
   const btID ExNumA  = 10;
   const btID ReasonA = 11;
   btcString  DescrA  = "aal0668A";

   CExceptionEvent a(&baseA, ExNumA, ReasonA, DescrA);

   a.setHandler(CAALEventProtected::EventHandler);
   EXPECT_TRUE(a.IsOK());

   CExceptionEvent copyA(PassReturnByValue(a));

   IEvent const &crA(copyA);
   void *ExpectA = reinterpret_cast<void *>( & const_cast<IEvent &>(crA) );

   EXPECT_TRUE(copyA.Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(&copyA) ), copyA.Interface(iidEvent));

   EXPECT_TRUE(copyA.Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(&copyA) ), copyA.Interface(iidCEvent));

   EXPECT_TRUE(copyA.Has(iidExEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(&copyA) ), copyA.Interface(iidExEvent));

   EXPECT_EQ(iidExEvent, copyA.SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(&copyA) ), copyA.ISubClass());

   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), &copyA.Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), copyA.pObject());

   EXPECT_TRUE(copyA.IsOK());
   EXPECT_EQ((btApplicationContext)71, copyA.Context());

   EXPECT_EQ(ExNumA,    copyA.ExceptionNumber());
   EXPECT_EQ(ReasonA,   copyA.Reason());
   EXPECT_STREQ(DescrA, copyA.Description());

   EXPECT_TRUE(a == copyA);
   EXPECT_FALSE(a != copyA);

   EXPECT_TRUE(copyA == a);
   EXPECT_FALSE(copyA != a);

   ASSERT_EQ(0, CAALEventProtected::sm_CallLog.LogEntries());

   copyA.operator()();

   ASSERT_EQ(1, CAALEventProtected::sm_CallLog.LogEntries());
   EXPECT_STREQ("CAALEventProtected::EventHandler", CAALEventProtected::sm_CallLog.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectA, CAALEventProtected::sm_CallLog.Entry(0).ParamValue(0));
   CAALEventProtected::sm_CallLog.ClearLog();


   CAASBase    baseB((btApplicationContext)72);
   const btIID IDB     = 998;
   const btID  ExNumB  = 10;
   const btID  ReasonB = 11;
   btcString   DescrB  = "aal0668B";

   CExceptionEvent b(&baseB, IDB, ExNumB, ReasonB, DescrB);

   CallTrackingServiceClient clientB;

   b.setHandler(&clientB);
   EXPECT_TRUE(b.IsOK());

   // new'ing the event, because operator()() with IServiceClient will delete it.
   CExceptionEvent *copyB = new CExceptionEvent(PassReturnByValue(b));

   IEvent const &crB(*copyB);
   void *ExpectB = reinterpret_cast<void *>( & const_cast<IEvent &>(crB) );

   EXPECT_TRUE(copyB->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyB) ), copyB->Interface(iidEvent));

   EXPECT_TRUE(copyB->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyB) ), copyB->Interface(iidCEvent));

   EXPECT_TRUE(copyB->Has(iidExEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyB) ), copyB->Interface(iidExEvent));

#if 0
   EXPECT_TRUE(copyB->Has(IDB));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyB) ),
             copyB->Interface(IDB));

   EXPECT_EQ(IDB, copyB->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyB) ),
             copyB->ISubClass());

   EXPECT_TRUE(b == *copyB);
   EXPECT_FALSE(b != *copyB);

   EXPECT_TRUE(*copyB == b);
   EXPECT_FALSE(*copyB != b);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), &copyB->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), copyB->pObject());

   EXPECT_TRUE(copyB->IsOK());
   EXPECT_EQ((btApplicationContext)72, copyB->Context());

   EXPECT_EQ(ExNumB,    copyB->ExceptionNumber());
   EXPECT_EQ(ReasonB,   copyB->Reason());
   EXPECT_STREQ(DescrB, copyB->Description());

   ASSERT_EQ(0, clientB.LogEntries());

   copyB->operator()(); // Delete's copyB.

   ASSERT_EQ(1, clientB.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", clientB.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectB, clientB.Entry(0).ParamValue(0));


   CAASBase    baseC((btApplicationContext)73);
   const btIID IDC     = 999;
   const btID  ExNumC  = 12;
   const btID  ReasonC = 13;
   btcString   DescrC  = "aal0668C";

   CExceptionEvent c(&baseC, IDC, ExNumC, ReasonC, DescrC);

   CallTrackingRuntimeClient clientC;

   c.setHandler(&clientC);
   EXPECT_TRUE(c.IsOK());

   // new'ing the event, because operator()() with IRuntimeClient will delete it.
   CExceptionEvent *copyC = new CExceptionEvent(PassReturnByValue(c));

   IEvent const &crC(*copyC);
   void *ExpectC = reinterpret_cast<void *>( & const_cast<IEvent &>(crC) );

   EXPECT_TRUE(copyC->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyC) ), copyC->Interface(iidEvent));

   EXPECT_TRUE(copyC->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyC) ), copyC->Interface(iidCEvent));

   EXPECT_TRUE(copyC->Has(iidExEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyC) ), copyC->Interface(iidExEvent));

#if 0
   EXPECT_TRUE(copyC->Has(IDC));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyC) ),
             copyC->Interface(IDC));

   EXPECT_EQ(IDC, copyC->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyC) ), copyC->ISubClass());

   EXPECT_TRUE(c == *copyC);
   EXPECT_FALSE(c != *copyC);

   EXPECT_TRUE(*copyC == c);
   EXPECT_FALSE(*copyC != c);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), &copyC->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), copyC->pObject());

   EXPECT_TRUE(copyC->IsOK());
   EXPECT_EQ((btApplicationContext)73, copyC->Context());

   EXPECT_EQ(ExNumC,    copyC->ExceptionNumber());
   EXPECT_EQ(ReasonC,   copyC->Reason());
   EXPECT_STREQ(DescrC, copyC->Description());

   ASSERT_EQ(0, clientC.LogEntries());

   copyC->operator()(); // Delete's copyC.

   ASSERT_EQ(1, clientC.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", clientC.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectC, clientC.Entry(0).ParamValue(0));
}
#endif

////////////////////////////////////////////////////////////////////////////////

TEST(CExceptionTransactionEventTest, aal0669)
{
   // CExceptionTransactionEvent(IBase * , TransactionID const & , btID , btID , btcString )
   // constructs CAALEvent with the IBase *, and stores the ExceptionNumber, Reason, and
   // Description for later retrieval by data accessor member fn's of the same name. The
   // TransactionID is saved for later retrieval by TranID(). On success, a SubClass of
   // iidExTranEvent/IExceptionTransactionEvent * is set, and an interface of
   // iidTranEvent/ITransactionEvent * is available.

   CAASBase base((btApplicationContext)101);

   TransactionID tid;
   tid.ID(105);

   const btID ExNum  = 12;
   const btID Reason = 13;
   btcString  Descr  = "aal0669";

   CExceptionTransactionEvent e(&base, tid, ExNum, Reason, Descr);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ((btApplicationContext)101, e.Context());

   EXPECT_EQ(ExNum,    e.ExceptionNumber());
   EXPECT_EQ(Reason,   e.Reason());
   EXPECT_STREQ(Descr, e.Description());

   EXPECT_EQ(105, e.TranID().ID());

   EXPECT_EQ(iidExTranEvent, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&e) ), e.Interface(iidTranEvent));

   EXPECT_TRUE(e.IsOK());
}

TEST(CExceptionTransactionEventTest, aal0670)
{
   // CExceptionTransactionEvent(IBase * , btIID , TransactionID const & , btID , btID , btcString )
   // constructs CAALEvent with the IBase *, and stores the ExceptionNumber, Reason, and Description
   // for later retrieval by data accessor member fn's of the same name. The TransactionID is saved
   // for later retrieval by TranID(). On success, a SubClass of ID/IExceptionTransactionEvent* is
   // set. Interfaces iidTranEvent/ITransactionEvent * and
   // iidExTranEvent/IExceptionTransactionEvent * are also added.

   CAASBase base((btApplicationContext)102);

   const btIID   ID     = 17;
   TransactionID tid;
   tid.ID(107);
   const btID    ExNum  = 18;
   const btID    Reason = 19;
   btcString     Descr  = "aal0670";

   CExceptionTransactionEvent e(&base, ID, tid, ExNum, Reason, Descr);

   EXPECT_EQ(dynamic_cast<IBase *>(&base), e.pObject());
   EXPECT_EQ((btApplicationContext)102, e.Context());

   EXPECT_EQ(ExNum,    e.ExceptionNumber());
   EXPECT_EQ(Reason,   e.Reason());
   EXPECT_STREQ(Descr, e.Description());

   EXPECT_EQ(107, e.TranID().ID());

   EXPECT_EQ(ID, e.SubClassID());
#if DEPRECATED
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(&e) ), e.ISubClass());
#endif // DEPRECATED

   EXPECT_TRUE(e.Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(&e) ), e.Interface(iidTranEvent));

   EXPECT_TRUE(e.Has(iidExTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(&e) ), e.Interface(iidExTranEvent));

   EXPECT_TRUE(e.IsOK());
}

#if 0
Copies were de-featured.
TEST(CExceptionTransactionEventTest, aal0671)
{
   // CExceptionTransactionEvent(const CExceptionTransactionEvent& ) (copy constructor) creates
   // an exact copy of the given CExceptionTransactionEvent.

   CAASBase      baseA((btApplicationContext)71);
   TransactionID tidA;
   tidA.ID(75);
   const btID    ExNumA  = 10;
   const btID    ReasonA = 11;
   btcString     DescrA  = "aal0671A";

   CExceptionTransactionEvent a(&baseA, tidA, ExNumA, ReasonA, DescrA);

   a.setHandler(CAALEventProtected::EventHandler);
   EXPECT_TRUE(a.IsOK());

   CExceptionTransactionEvent copyA(PassReturnByValue(a));

   IEvent const &crA(copyA);
   void *ExpectA = reinterpret_cast<void *>( & const_cast<IEvent &>(crA) );

   EXPECT_TRUE(copyA.Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(&copyA) ), copyA.Interface(iidEvent));

   EXPECT_TRUE(copyA.Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(&copyA) ), copyA.Interface(iidCEvent));

   EXPECT_TRUE(copyA.Has(iidExTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(&copyA) ),
             copyA.Interface(iidExTranEvent));

   EXPECT_EQ(iidExTranEvent, copyA.SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(&copyA) ),
             copyA.ISubClass());

   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), &copyA.Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseA), copyA.pObject());

   EXPECT_TRUE(copyA.IsOK());
   EXPECT_EQ((btApplicationContext)71, copyA.Context());

   EXPECT_EQ(ExNumA,    copyA.ExceptionNumber());
   EXPECT_EQ(ReasonA,   copyA.Reason());
   EXPECT_STREQ(DescrA, copyA.Description());
   EXPECT_EQ(75,        copyA.TranID().ID());

   EXPECT_TRUE(a == copyA);
   EXPECT_FALSE(a != copyA);

   EXPECT_TRUE(copyA == a);
   EXPECT_FALSE(copyA != a);

   ASSERT_EQ(0, CAALEventProtected::sm_CallLog.LogEntries());

   copyA.operator()();

   ASSERT_EQ(1, CAALEventProtected::sm_CallLog.LogEntries());
   EXPECT_STREQ("CAALEventProtected::EventHandler", CAALEventProtected::sm_CallLog.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectA, CAALEventProtected::sm_CallLog.Entry(0).ParamValue(0));
   CAALEventProtected::sm_CallLog.ClearLog();


   CAASBase      baseB((btApplicationContext)72);
   const btIID   IDB     = 998;
   TransactionID tidB;
   tidB.ID(76);
   const btID    ExNumB  = 10;
   const btID    ReasonB = 11;
   btcString     DescrB  = "aal0671B";

   CExceptionTransactionEvent b(&baseB, IDB, tidB, ExNumB, ReasonB, DescrB);

   CallTrackingServiceClient clientB;

   b.setHandler(&clientB);
   EXPECT_TRUE(b.IsOK());

   // new'ing the event, because operator()() with IServiceClient will delete it.
   CExceptionTransactionEvent *copyB = new CExceptionTransactionEvent(PassReturnByValue(b));

   IEvent const &crB(*copyB);
   void *ExpectB = reinterpret_cast<void *>( & const_cast<IEvent &>(crB) );

   EXPECT_TRUE(copyB->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyB) ), copyB->Interface(iidEvent));

   EXPECT_TRUE(copyB->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyB) ), copyB->Interface(iidCEvent));

   EXPECT_TRUE(copyB->Has(iidExTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(copyB) ),
             copyB->Interface(iidExTranEvent));

#if 0
   EXPECT_TRUE(copyB->Has(IDB));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(copyB) ),
             copyB->Interface(IDB));

   EXPECT_EQ(IDB, copyB->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionEvent *>(copyB) ),
             copyB->ISubClass());

   EXPECT_TRUE(b == *copyB);
   EXPECT_FALSE(b != *copyB);

   EXPECT_TRUE(*copyB == b);
   EXPECT_FALSE(*copyB != b);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), &copyB->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseB), copyB->pObject());

   EXPECT_TRUE(copyB->IsOK());
   EXPECT_EQ((btApplicationContext)72, copyB->Context());

   EXPECT_EQ(ExNumB,    copyB->ExceptionNumber());
   EXPECT_EQ(ReasonB,   copyB->Reason());
   EXPECT_STREQ(DescrB, copyB->Description());
   EXPECT_EQ(76,        copyB->TranID().ID());

   ASSERT_EQ(0, clientB.LogEntries());

   copyB->operator()(); // Delete's copyB.

   ASSERT_EQ(1, clientB.LogEntries());
   EXPECT_STREQ("IServiceClient::serviceEvent", clientB.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectB, clientB.Entry(0).ParamValue(0));


   CAASBase      baseC((btApplicationContext)73);
   const btIID   IDC     = 999;
   TransactionID tidC;
   tidC.ID(77);
   const btID    ExNumC  = 12;
   const btID    ReasonC = 13;
   btcString     DescrC  = "aal0671C";

   CExceptionTransactionEvent c(&baseC, IDC, tidC, ExNumC, ReasonC, DescrC);

   CallTrackingRuntimeClient clientC;

   c.setHandler(&clientC);
   EXPECT_TRUE(c.IsOK());

   // new'ing the event, because operator()() with IRuntimeClient will delete it.
   CExceptionTransactionEvent *copyC = new CExceptionTransactionEvent(PassReturnByValue(c));

   IEvent const &crC(*copyC);
   void *ExpectC = reinterpret_cast<void *>( & const_cast<IEvent &>(crC) );

   EXPECT_TRUE(copyC->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(copyC) ), copyC->Interface(iidEvent));

   EXPECT_TRUE(copyC->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(copyC) ), copyC->Interface(iidCEvent));

   EXPECT_TRUE(copyC->Has(iidExTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(copyC) ),
             copyC->Interface(iidExTranEvent));

#if 0
   EXPECT_TRUE(copyC->Has(IDC));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(copyC) ),
             copyC->Interface(IDC));

   EXPECT_EQ(IDC, copyC->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IExceptionTransactionEvent *>(copyC) ),
             copyC->ISubClass());

   EXPECT_TRUE(c == *copyC);
   EXPECT_FALSE(c != *copyC);

   EXPECT_TRUE(*copyC == c);
   EXPECT_FALSE(*copyC != c);
#endif

   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), &copyC->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&baseC), copyC->pObject());

   EXPECT_TRUE(copyC->IsOK());
   EXPECT_EQ((btApplicationContext)73, copyC->Context());

   EXPECT_EQ(ExNumC,    copyC->ExceptionNumber());
   EXPECT_EQ(ReasonC,   copyC->Reason());
   EXPECT_STREQ(DescrC, copyC->Description());
   EXPECT_EQ(77,        copyC->TranID().ID());

   ASSERT_EQ(0, clientC.LogEntries());

   copyC->operator()(); // Delete's copyC.

   ASSERT_EQ(1, clientC.LogEntries());
   EXPECT_STREQ("IRuntimeClient::runtimeEvent", clientC.Entry(0).MethodName().c_str());
   EXPECT_EQ(ExpectC, clientC.Entry(0).ParamValue(0));
}
#endif

////////////////////////////////////////////////////////////////////////////////
#if DEPRECATED
TEST(ObjectCreatedEventTest, aal0672)
{
   // ObjectCreatedEvent(IRuntimeClient * , IServiceClient * , IBase * , TransactionID ,
   // const NamedValueSet & ) constructs CTransactionEvent with the IBase * and TransactionID,
   // stores the IRuntimeClient * and IServiceClient * for later use by operator()(), and stores
   // the NamedValueSet for later retrieval by GetOptArgs(). On success, a SubClass of
   // tranevtFactoryCreate/IObjectCreatedEvent * is set.

   CallTrackingIRuntimeClient rtc;
   CallTrackingIServiceClient svc;

   CAASBase      base((btApplicationContext)71);
   TransactionID tid;
   tid.ID(32);

   NamedValueSet nvs;
   nvs.Add((btNumberKey)5, (btcString)"hello");

   // new'ing the event because operator()() will delete it.
   ObjectCreatedEvent *e = new ObjectCreatedEvent(&rtc, &svc, &base, tid, nvs);

   EXPECT_TRUE(e->IsOK());

   EXPECT_EQ(dynamic_cast<IBase *>(&base), &e->Object());
   EXPECT_EQ(dynamic_cast<IBase *>(&base), e->pObject());

   EXPECT_EQ((btApplicationContext)71, e->Context());

   EXPECT_EQ(32, e->TranID().ID());

   EXPECT_TRUE(e->Has(iidEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IEvent *>(e) ),
             e->Interface(iidEvent));

   EXPECT_TRUE(e->Has(iidCEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<CAALEvent *>(e) ),
             e->Interface(iidCEvent));

   EXPECT_TRUE(e->Has(iidTranEvent));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<ITransactionEvent *>(e) ),
             e->Interface(iidTranEvent));

   EXPECT_TRUE(e->Has(tranevtFactoryCreate));
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IObjectCreatedEvent *>(e) ),
             e->Interface(tranevtFactoryCreate));

   EXPECT_EQ(tranevtFactoryCreate, e->SubClassID());
   EXPECT_EQ(reinterpret_cast<btGenericInterface>( dynamic_cast<IObjectCreatedEvent *>(e) ),
             e->ISubClass());

   EXPECT_EQ(nvs, e->GetOptArgs());

   // Deletes e.
   e->operator()();

   ASSERT_EQ(1, rtc.LogEntries());
   ASSERT_STREQ("IRuntimeClient::runtimeAllocateServiceSucceeded", rtc.Entry(0).MethodName());

   btObjectType x = NULL;
   rtc.Entry(0).GetParam("pServiceBase", &x);
   ASSERT_EQ(dynamic_cast<IBase *>(&base), reinterpret_cast<IBase *>(x));

   TransactionID tid2;
   rtc.Entry(0).GetParam("tid", tid2);
   EXPECT_EQ(tid.ID(), tid2.ID());


   ASSERT_EQ(1, svc.LogEntries());
   ASSERT_STREQ("IServiceClient::serviceAllocated", svc.Entry(0).MethodName());

   x = NULL;
   svc.Entry(0).GetParam("pBase", &x);
   ASSERT_EQ(dynamic_cast<IBase *>(&base), reinterpret_cast<IBase *>(x));

   TransactionID tid3;
   svc.Entry(0).GetParam("tid", tid3);
   EXPECT_EQ(tid.ID(), tid3.ID());
}
#endif // DEPRECATED

