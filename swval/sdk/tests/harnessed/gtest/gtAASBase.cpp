// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

//=============================================================================
// Name: CAASBase_f_0
// Comments: CAASBase_f_0 derived from CAASBase and gtest
//=============================================================================
class CAASBase_f_0 : public AAL::CAASBase,
                     public ::testing::Test
{
public:
	CAASBase_f_0() :
      CAASBase((btApplicationContext) 33)
   {}
	virtual ~CAASBase_f_0() {}
};

TEST_F(CAASBase_f_0, aal0225)
{
   // CAASBase::CAASBase(btApplicationContext ) sets interface pointers for iidCBase and iidBase,
   // When object initialization is successful, m_bIsOK is set to true. m_Context stores the
   // input parameter for later retrieval by Context().

   EXPECT_TRUE(Has(iidBase));
   EXPECT_TRUE(Has(iidCBase));

   EXPECT_EQ(dynamic_cast<IBase *>(this),    Interface(iidBase));
   EXPECT_EQ(dynamic_cast<CAASBase *>(this), Interface(iidCBase));

   EXPECT_TRUE(IsOK());

   const btApplicationContext ExpectCtx = (btApplicationContext)33;
   EXPECT_EQ(ExpectCtx, Context());
}

//=============================================================================
// Name: CAASBase_f_1
// Comments: CAASBase_f_1 derived from CAASBase and gtest
//=============================================================================
class CAASBase_f_1 : public AAL::CAASBase,
                     public ::testing::Test
{
public:
	CAASBase_f_1() :
      CAASBase()
   {}
	virtual ~CAASBase_f_1() {}
};

TEST_F(CAASBase_f_1, aal0226)
{
   // When an CAASBase::CAASBase(btApplicationContext ) is created with the default
   // constructor. m_bIsOK is set to true. m_Context stores the NULL application
   // context for later retrieval by Context().

   EXPECT_TRUE(IsOK());
   EXPECT_NULL(Context());
}

TEST_F(CAASBase_f_0, aal0227)
{
   // When the input parameter to CAASBase::SetContext(btApplicationContext )
   // is non-NULL, m_Context stores the btApplicationContext, Subsequent calls
   // CAASBase::Context() return stored app context (m_Context).

   const btApplicationContext appContext = (btApplicationContext)20;

   SetContext(appContext);
   EXPECT_EQ(appContext, Context());
}

TEST_F(CAASBase_f_0, aal0228)
{
   // When the input parameter to CAASBase::SetContext(btApplicationContext ) is NULL,
   // m_Context stores the NULL Context, Subsequent calls to Context() return NULL
   // context (m_Context).

	SetContext(NULL);
	EXPECT_NULL(Context());
}

TEST_F(CAASBase_f_0, aal0229)
{
   // When the input parameter to CAASBase::SetInterface() are new interface ID
   // and non-NULL pointer on the object, CAASBase::SetInterface() returns EObjOK.

   btIID interfaceID = 0x299;
   const btGenericInterface interface = (btGenericInterface)99;
   EXPECT_EQ(EObjOK, SetInterface(interfaceID, interface));
}

TEST_F(CAASBase_f_0, aal0230)
{
   // When the input parameter to CAASBase::SetInterface() is NULL,
   // CAASBase::SetInterface() returns EObjBadObject.

	btIID interfaceID = 0x299;
	EXPECT_EQ(EObjBadObject, SetInterface(interfaceID, NULL));
}

TEST_F(CAASBase_f_0, aal0231)
{
   // When the input parameter to CAASBase::SetInterface() is already
   // implemented interface, CAASBase::SetInterface() returns EObjDuplicateName.

	btIID interfaceID = 0x299;
	const btGenericInterface interface = (btGenericInterface)99;
	EXPECT_EQ(EObjOK, SetInterface(interfaceID, interface));

	const btGenericInterface interfaceA = (btGenericInterface)100;
	EXPECT_EQ(EObjDuplicateName, SetInterface(interfaceID, interfaceA));
}

TEST_F(CAASBase_f_0, aal0232)
{
   // When interface is not implemented, CAASBase::Interface(btIID Interface) returns NULL.

	btIID interfaceID = 0x299;
	EXPECT_NULL(Interface(interfaceID));
}

TEST_F(CAASBase_f_0, aal0233)
{
   // When interface is implemented, CAASBase::Interface(btIID Interface)
   // returns a pointer to the requested interface.

	btIID interfaceID = 0x299;
	EXPECT_NULL(Interface(interfaceID));

	const btGenericInterface pInterface= (btGenericInterface)99;
	EXPECT_EQ(EObjOK, SetInterface(interfaceID, pInterface));
	EXPECT_EQ(pInterface, Interface(interfaceID));

	EXPECT_EQ(pInterface, dynamic_ptr<btGenericInterface>(interfaceID,  this));
	EXPECT_EQ(pInterface, dynamic_ptr<btGenericInterface>(interfaceID, *this));

	btGenericInterface &refA = dynamic_ref<btGenericInterface>(interfaceID, this);
	EXPECT_EQ(&refA, pInterface);

	btGenericInterface &refB = dynamic_ref<btGenericInterface>(interfaceID, *this);
	EXPECT_EQ(&refB, pInterface);
}

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0234)
{
   // When set a sub class interface pointer to subclass object (CAASBase::SetSubClassInterface()),
   // m_SubClassID stores subclass id and m_ISubClass stores interface pointer and returns EObjOK.

	btIID interfaceID = 988;
	const btGenericInterface pinterface = (btGenericInterface)99;
	EXPECT_EQ(EObjOK, SetSubClassInterface(interfaceID, pinterface));

   EXPECT_EQ(pinterface, ISubClass());
   EXPECT_EQ(interfaceID, SubClassID());
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0235)
{
   // When the input parameter to CAASBase::SetSubClassInterface() is NULL,
   // CAASBase::SetSubClassInterface() returns EObjBadObject.

	btIID interfaceID =988;
	EXPECT_EQ(EObjBadObject, SetSubClassInterface(interfaceID, NULL));
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0236)
{
   // When try to set already implemented sub class interface,
   // CAASBase::SetSubClassInterface() returns EObjDuplicateName.

	btIID interfaceID =988;
	const btGenericInterface pinterface= (btGenericInterface)99;
	EXPECT_EQ(EObjOK, SetSubClassInterface(interfaceID, pinterface)) ;
	EXPECT_EQ(EObjDuplicateName, SetSubClassInterface(interfaceID, pinterface));
}
#endif // DEPRECATED

TEST_F(CAASBase_f_0, aal0237)
{
   // When a CAASBase object has an interface matching the given
   // interface ID, CAASBase::Has() returns true.

	EXPECT_TRUE(Has(iidBase));
	EXPECT_TRUE(Has(iidCBase));
}

TEST_F(CAASBase_f_0, aal0238)
{
   // When a CAASBase object doesn’t have an interface matching the given
   // interface ID, CAASBase::Has() returns false.
	btIID interfaceID =988;
	EXPECT_FALSE(Has(interfaceID));
}

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0239)
{
   // CAASBase::CAASBase(btApplicationContext ) sets a default SubClass interface of iidBase.

	EXPECT_EQ(static_cast<CAASBase_f_0 *>(ISubClass()),this);
	EXPECT_EQ(SubClassID(),iidBase);
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0240)
{
   // CAASBase::ISubClass() returns the cached pointer to subclass iidBase object.

	EXPECT_EQ(static_cast<CAASBase_f_0 *>(ISubClass()), this);

	EXPECT_NONNULL(subclass_ptr<IBase>(this));
	EXPECT_NONNULL(subclass_ptr<IBase>(*this));

	EXPECT_EQ(subclass_ref<IBase>(this),  *this);
	EXPECT_EQ(subclass_ref<IBase>(*this), *this);
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0241)
{
   // CAASBase::ISubClass() returns the latest/recent cached subclass
   // pointer which set by SetSubClassInterface().

	btIID interfaceID =988;
	const btGenericInterface pinterface= (btGenericInterface)99;

	EXPECT_EQ(this, static_cast<CAASBase_f_0 *>(ISubClass()));

	EXPECT_EQ(EObjOK, SetSubClassInterface(interfaceID, pinterface));
	EXPECT_EQ(pinterface, ISubClass());
	EXPECT_NE(this, static_cast<CAASBase_f_0 *>(ISubClass()));

	EXPECT_EQ(pinterface, subclass_ptr<btGenericInterface>(this));
	EXPECT_EQ(pinterface, subclass_ptr<btGenericInterface>(*this));

	EXPECT_EQ(pinterface, &subclass_ref<btGenericInterface>(this));
	EXPECT_EQ(pinterface, &subclass_ref<btGenericInterface>(*this));
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0242)
{
   // When constructor CAASBase(btApplicationContext ) sets SubClass ID iidBase by
   // default, ISubClassID() returns subclass ID (iidBase).
	EXPECT_EQ(iidBase, SubClassID());
}
#endif // DEPRECATED

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0243)
{
   // CAASBase::ISubClassID() returns stored subclass ID in m_SubClassID,
   // set by CAASBase::SetSubClassInterface().

	btIID interfaceID =988;
	const btGenericInterface pinterface= (btGenericInterface)99;
	EXPECT_EQ(EObjOK, SetSubClassInterface(interfaceID, pinterface));
	EXPECT_EQ(interfaceID, SubClassID());
}
#endif // DEPRECATED

TEST_F(CAASBase_f_0, aal0244)
{
   // When both are (lhs and rhs) CAASBase instances, same number and
   // type of interfaces, operator == returns true and != returns false.

	CAASBase AASBaseA;
	CAASBase AASBaseB;

	EXPECT_TRUE(AASBaseA == *this);
	EXPECT_FALSE(AASBaseA != *this);

	EXPECT_TRUE(AASBaseA == AASBaseB);
	EXPECT_FALSE(AASBaseA != AASBaseB);

	EXPECT_TRUE(AASBaseB == AASBaseA);
	EXPECT_FALSE(AASBaseB != AASBaseA);

	EXPECT_TRUE(AASBaseA == AASBaseA);
	EXPECT_FALSE(AASBaseA != AASBaseA);
}

TEST_F(CAASBase_f_0, aal0245)
{
   // When either (lhs and rhs) are not CAASBase instances (not derived form CAASBase),
   // operator == returns false and != returns true.

   class aal0245Base : public IBase
   {
   public:
      aal0245Base() {}
      btGenericInterface Interface(btIID Interface)     const { return NULL;  }
      btBool                   Has(btIID Interface)     const { return false; }
      btBool          operator != (IBase const &rother) const { return true;  }
      btBool          operator == (IBase const &rother) const { return false; }
      btBool                  IsOK()                    const { return true;  }
      btApplicationContext Context()                    const { return NULL;  }
   } a;

   CAASBase b;

	EXPECT_FALSE(b == a);
	EXPECT_TRUE( b != a);
}

#if DEPRECATED
TEST_F(CAASBase_f_0, aal0246)
{
   // When both (lhs and rhs) are CAASBase instances but don't implement the same SubClass,
   // operator == returns false and != returns true.

   class aal0246Base : public CAASBase
   {
   public:
      aal0246Base() {}

      void ChangeSubClass()
      {
         EXPECT_EQ(EObjOK, SetSubClassInterface((btIID)998, (btGenericInterface)998));
      }
   } a;

   CAASBase b;

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   a.ChangeSubClass();

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}
#endif // DEPRECATED

TEST_F(CAASBase_f_0, aal0247)
{
   // When both (lhs and rhs) are CAASBase instances but don't implement the same number
   // of interfaces, operator == returns false and != returns true.

   class aal0247Base : public CAASBase
   {
   public:
      aal0247Base() {}

      void AddInterface()
      {
         EXPECT_EQ(EObjOK, SetInterface((btIID)998, (btGenericInterface)998));
      }
   } a;

   CAASBase b;

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   a.AddInterface();

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}

TEST_F(CAASBase_f_0, aal0248)
{
   // When both (lhs and rhs) are CAASBase instances,
   // implement same number of interfaces, but implement different interface types,
   // operator == returns false and != returns true.

   class aal0248Base : public CAASBase
   {
   public:
      aal0248Base() {}

      void AddInterface(btIID id, btGenericInterface ifc)
      {
         EXPECT_EQ(EObjOK, SetInterface(id, ifc));
      }
   } a, b;

   EXPECT_TRUE( a == b);
   EXPECT_FALSE(a != b);

   EXPECT_TRUE( b == a);
   EXPECT_FALSE(b != a);

   const btGenericInterface ifc = (btGenericInterface)3;

   a.AddInterface((btIID)998, ifc);
   b.AddInterface((btIID)999, ifc);

   EXPECT_FALSE(a == b);
   EXPECT_TRUE( a != b);

   EXPECT_FALSE(b == a);
   EXPECT_TRUE( b != a);
}

TEST_F(CAASBase_f_0, aal0716)
{
   // When a CAASBase object does not implement an interface described by the btIID parameter
   // to CAASBase::ReplaceInterface(), the function returns EObjNameNotFound, indicating failure.

   class aal0716Base : public CAASBase
   {
   public:
      aal0716Base() {}

      EOBJECT CallReplaceInterface(btIID id, btGenericInterface ifc)
      {
         return ReplaceInterface(id, ifc);
      }
   } b;

   const btIID         ID = 999;
   btGenericInterface Ifc = (btGenericInterface) 3;

   EXPECT_EQ(EObjNameNotFound, b.CallReplaceInterface(ID, Ifc));
}

TEST_F(CAASBase_f_0, aal0717)
{
   // When a CAASBase object does implement the interface described by the btIID parameter
   // to CAASBase::ReplaceInterface(btIID , btGenericInterface ), and the btGenericInterface
   // parameter is NULL, the function removes the interface entry for the btIID and returns EObjOK.

   class aal0717Base : public CAASBase
   {
   public:
      aal0717Base() {}

      EOBJECT CallSetInterface(btIID id, btGenericInterface ifc)
      { return SetInterface(id, ifc); }

      EOBJECT CallReplaceInterface(btIID id, btGenericInterface ifc)
      { return ReplaceInterface(id, ifc); }
   } b;

   const btIID         ID = 999;
   btGenericInterface Ifc = (btGenericInterface) 3;

   EXPECT_EQ(EObjOK, b.CallSetInterface(ID, Ifc));

   EXPECT_TRUE(b.Has(ID));
   EXPECT_EQ(EObjOK, b.CallReplaceInterface(ID, NULL));
   EXPECT_FALSE(b.Has(ID));
}

TEST_F(CAASBase_f_0, aal0718)
{
   // When a CAASBase object does implement the interface described by the btIID parameter to
   // CAASBase::ReplaceInterface(btIID , btGenericInterface ), and the btGenericInterface
   // parameter is non-NULL, the interface entry is updated and EObjOK is returned.

   class aal0718Base : public CAASBase
   {
   public:
      aal0718Base() {}

      EOBJECT CallSetInterface(btIID id, btGenericInterface ifc)
      { return SetInterface(id, ifc); }

      EOBJECT CallReplaceInterface(btIID id, btGenericInterface ifc)
      { return ReplaceInterface(id, ifc); }
   } b;

   const btIID         ID = 999;
   btGenericInterface Ifc = (btGenericInterface) 3;

   EXPECT_EQ(EObjOK, b.CallSetInterface(ID, Ifc));

   EXPECT_TRUE(b.Has(ID));
   EXPECT_EQ(Ifc, b.Interface(ID));

   btGenericInterface NewIfc = (btGenericInterface) 7;

   EXPECT_EQ(EObjOK, b.CallReplaceInterface(ID, NewIfc));
   EXPECT_TRUE(b.Has(ID));
   EXPECT_EQ(NewIfc, b.Interface(ID));
}

TEST(CAALBaseTest, CAALBaseTest)
{
   class DerivedFromCAALBase : public CAALBase
   {
   public:
      DerivedFromCAALBase(btEventHandler       h,
                          btApplicationContext ctx) : CAALBase(h, ctx) {}
      virtual void Destroy(TransactionID const & ) {}
   };

   DerivedFromCAALBase d0((btEventHandler)NULL, (btApplicationContext)NULL);
   EXPECT_FALSE(d0.IsOK()); // NULL event handler

   DerivedFromCAALBase d1((btEventHandler)3, (btApplicationContext)NULL);
   EXPECT_TRUE(d1.IsOK());
}

