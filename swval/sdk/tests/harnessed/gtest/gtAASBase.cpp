// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"


//=============================================================================
// Name: AASBase::CAASBase
// Comments: For CAASBase Test cases
//=============================================================================

class AASBase : public CAASBase ,
                public ::testing::Test
{
protected:
	virtual void SetUp() { }
	virtual void TearDown() { }
	void TestBody() {}

public :
	AASBase(): CAASBase() { }
	AASBase(btApplicationContext Context): CAASBase(Context) { }
	virtual ~AASBase() { }
};

//=============================================================================
// Name: CAALIBase
// Comments: CAALIBase derived from IBase for CAASBase operator == and !=
// unit test cases
//=============================================================================
class CAALIBase : public IBase
{

public:
	CAALIBase() {};
	btGenericInterface Interface(btIID Interface) const  { return NULL;  }
	btBool Has(btIID Interface) const                    { return false; }
	btGenericInterface ISubClass() const                 { return NULL;  }
	btIID SubClassID() const                             { return 0;     }
	btBool operator != (IBase const &rother) const       { return false; }
	btBool operator == (IBase const &rother) const       { return true;  }
	btBool IsOK() const                                  { return true;  }
	btApplicationContext Context() const                 { return NULL;  }
};



//=============================================================================
// Name: AASBase
// Comments: CAASBase create and delete
//=============================================================================
TEST(CAASBase_Test,aal0262)
{
	CAASBase *pCAASBase = new(std::nothrow) CAASBase();
	ASSERT_NONNULL(pCAASBase);
	EXPECT_TRUE(pCAASBase->IsOK());
	EXPECT_EQ(pCAASBase->Has(iidCBase),true);
	delete pCAASBase;
}

//=============================================================================
// Name: AASBase
// Comments: CAASBase constructor with Context
//=============================================================================
TEST(CAASBase_Test,aal0263)
{
	// set context
	int* ptr = new (std::nothrow) int(88888);
	if(NULL != ptr)
	{
		CAASBase *pCAASBase = new(std::nothrow) CAASBase(ptr);
		ASSERT_NONNULL(pCAASBase);

		EXPECT_TRUE(pCAASBase->IsOK());
		EXPECT_EQ(static_cast<int *>(pCAASBase->Context()) ,ptr);
		EXPECT_EQ(*(static_cast<int *>(pCAASBase->Context())) ,*(ptr));
		delete pCAASBase;
	}
	if(ptr)
	   delete ptr;

	//Set CAASBase context NULL
	CAASBase *pCAASBase = new(std::nothrow) CAASBase(NULL);
	ASSERT_NONNULL(pCAASBase);

	EXPECT_TRUE(pCAASBase->IsOK());
	EXPECT_NULL(pCAASBase->Context());
	delete pCAASBase;

}

//=============================================================================
// Name: AASBase
// Comments: CAASBase Set Context and get Context
//=============================================================================
TEST_F(AASBase,aal0264)
{
	SetContext(this);
	EXPECT_EQ(static_cast<AASBase *>(Context()),this);
}

//=============================================================================
// Name: AASBase
// Comments: CAASBase Set Interface and get Interface
//=============================================================================
TEST_F(AASBase,aal0265)
{
	// interface with id 9999
	btIID interfaceID =9999;

	EXPECT_EQ( SetInterface(interfaceID, dynamic_cast<AASBase *>(this)) ,EObjOK) ;
	EXPECT_TRUE( IsOK());
	EXPECT_EQ(Has(interfaceID),true);
	EXPECT_EQ(static_cast<AASBase *>(Interface(interfaceID)),this);

	// constructor set IBase interface , set interface returns duplicate
	EXPECT_EQ( SetInterface(iidCBase, dynamic_cast<CAASBase *>(this)) ,EObjDuplicateName) ;
	EXPECT_TRUE(IsOK());
	EXPECT_EQ(Has(iidCBase),true);
	EXPECT_EQ(static_cast<AASBase *>(Interface(iidCBase)),this);

	// interface with NULL pointer
	interfaceID =9000;
	EXPECT_EQ( SetInterface(interfaceID, NULL) ,EObjBadObject) ;
	EXPECT_TRUE( IsOK());
	EXPECT_EQ(Has(interfaceID),false);

	// get NULL interface  with wrong interface
	ASSERT_NULL(Interface(iidServiceClient));
	ASSERT_NULL(Interface(interfaceID));
}

//=============================================================================
// Name: AASBase
// Comments: CAASBase Set SubClass interface and get sub Interface
//=============================================================================
TEST_F(AASBase,aal0266)
{
	// compare  default sub class interface
	EXPECT_EQ(static_cast<CAASBase *>(ISubClass()),dynamic_cast<CAASBase *>(this));

	//set interface  id 988 and compare context
	btIID interfaceID =988;
	EXPECT_EQ( SetSubClassInterface(interfaceID, dynamic_cast<AASBase *>(this)) ,EObjOK) ;
	EXPECT_EQ(SubClassID(),interfaceID);
	EXPECT_EQ(Has(interfaceID),true);

	EXPECT_EQ(static_cast<AASBase *>(ISubClass()),this);

	// set interface id 9990 with NULL
	interfaceID =9990;
	EXPECT_EQ( SetSubClassInterface(interfaceID, NULL) ,EObjBadObject) ;
	EXPECT_TRUE(IsOK());
	EXPECT_EQ(Has(interfaceID),false);

	// set IEvent interface
	EXPECT_EQ( SetSubClassInterface(iidEvent, dynamic_cast<IEvent *>(this)) ,EObjBadObject) ;
	EXPECT_EQ(Has(iidEvent),false);

	// constructor set IBase interface
	EXPECT_EQ( SetSubClassInterface(iidCBase, dynamic_cast<CAASBase *>(this)) ,EObjDuplicateName) ;
	EXPECT_TRUE(IsOK());
	EXPECT_EQ(Has(iidCBase),true);

	EXPECT_EQ(static_cast<CAASBase *>(ISubClass()),this);

}

//=============================================================================
// Name: AASBase
// Comments: CAASBase  verify operator != , ==
//             btBool  operator != (IBase const &rother) const;
//             btBool  operator == (IBase const &rother) const;
//=============================================================================
TEST_F(AASBase,aal0267)
{
	// Compare class object with subclass object
	AASBase* pAASBase = static_cast<AASBase *>(ISubClass());
	EXPECT_TRUE(*this == *pAASBase);

	AASBase aasBaseA;
	AASBase aasBaseB;
	CAALIBase AALIBase;

	// Compare class AASBase object with CAALIBase object
	EXPECT_FALSE(aasBaseA == AALIBase);
	EXPECT_TRUE(aasBaseA != AALIBase);

	// Compare class object with AASBase object
	EXPECT_TRUE(*this == aasBaseA);
	EXPECT_FALSE(aasBaseB != aasBaseA);

}

//=============================================================================
// Name: AASBase
// Comments: CAASBase set interface and get interface with different interface IDs
//=============================================================================
TEST_F(AASBase,aal0268)
{
	btIID interfaceID=00000;
	EXPECT_EQ( SetInterface(interfaceID, dynamic_cast<AASBase *>(this)) ,EObjOK) ;
	EXPECT_EQ(static_cast<AASBase *>(Interface(interfaceID)),this);
	EXPECT_EQ(Has(interfaceID),true);

	interfaceID = -1;
	EXPECT_EQ( SetInterface(interfaceID, dynamic_cast<AASBase *>(this)) ,EObjOK) ;
	EXPECT_EQ(static_cast<AASBase *>(Interface(interfaceID)),this);
	EXPECT_EQ(Has(interfaceID),true);

}

