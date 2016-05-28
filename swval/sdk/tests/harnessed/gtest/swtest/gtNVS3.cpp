// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<bt64bitInt,
                   TIntVerifier<bt64bitInt>,
                   Cbt64bitIntRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> bt64bitIntNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_bt64bitInt_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_bt64bitInt_tp_0() {}
   virtual ~NamedValueSet_bt64bitInt_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_bt64bitInt_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0300, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0301, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0302, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0303, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0304, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0305, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0306, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0307, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0308, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0309, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0462, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0463, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0464, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0465, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0515, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0538, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0575, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0576, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0577, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitInt_tp_0, aal0578, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt64bitInt_tp_0,
                           aal0300,
                           aal0301,
                           aal0302,
                           aal0303,
                           aal0304,
                           aal0305,
                           aal0306,
                           aal0307,
                           aal0308,
                           aal0309,
                           aal0462,
                           aal0463,
                           aal0464,
                           aal0465,
                           aal0515,
                           aal0538,
                           aal0575,
                           aal0576,
                           aal0577,
                           aal0578);

typedef ::testing::Types< bt64bitIntNVSTester > NamedValueSet_bt64bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt64bitInt_tp_0, NamedValueSet_bt64bitInt_tp_0_Types);


typedef TArrayNVSTester<bt64bitInt,
                        TIntVerifier<bt64bitInt>,
                        Cbt64bitIntArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> bt64bitIntArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_bt64bitIntArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_bt64bitIntArray_tp_0() {}
   virtual ~NamedValueSet_bt64bitIntArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_bt64bitIntArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0368, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0369, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0370, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0371, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0372, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0373, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0466, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0467, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0516, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0539, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0579, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt64bitIntArray_tp_0, aal0580, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt64bitIntArray_tp_0,
                           aal0368,
                           aal0369,
                           aal0370,
                           aal0371,
                           aal0372,
                           aal0373,
                           aal0466,
                           aal0467,
                           aal0516,
                           aal0539,
                           aal0579,
                           aal0580);

typedef ::testing::Types< bt64bitIntArrayNVSTester > NamedValueSet_bt64bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt64bitIntArray_tp_0, NamedValueSet_bt64bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btUnsigned64bitInt,
                   TIntVerifier<btUnsigned64bitInt>,
                   CbtUnsigned64bitIntRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btUnsigned64bitIntNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btUnsigned64bitInt_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btUnsigned64bitInt_tp_0() {}
   virtual ~NamedValueSet_btUnsigned64bitInt_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btUnsigned64bitInt_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0310, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0311, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0312, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0313, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0314, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0315, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0316, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0317, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0318, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0319, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0468, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0469, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0470, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0471, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0517, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0540, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0581, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0582, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0583, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitInt_tp_0, aal0584, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned64bitInt_tp_0,
                           aal0310,
                           aal0311,
                           aal0312,
                           aal0313,
                           aal0314,
                           aal0315,
                           aal0316,
                           aal0317,
                           aal0318,
                           aal0319,
                           aal0468,
                           aal0469,
                           aal0470,
                           aal0471,
                           aal0517,
                           aal0540,
                           aal0581,
                           aal0582,
                           aal0583,
                           aal0584);

typedef ::testing::Types< btUnsigned64bitIntNVSTester > NamedValueSet_btUnsigned64bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned64bitInt_tp_0, NamedValueSet_btUnsigned64bitInt_tp_0_Types);


typedef TArrayNVSTester<btUnsigned64bitInt,
                        TIntVerifier<btUnsigned64bitInt>,
                        CbtUnsigned64bitIntArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btUnsigned64bitIntArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btUnsigned64bitIntArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btUnsigned64bitIntArray_tp_0() {}
   virtual ~NamedValueSet_btUnsigned64bitIntArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btUnsigned64bitIntArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0374, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0375, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0376, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0377, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0378, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0379, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0472, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0473, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0518, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0541, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0585, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned64bitIntArray_tp_0, aal0586, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned64bitIntArray_tp_0,
                           aal0374,
                           aal0375,
                           aal0376,
                           aal0377,
                           aal0378,
                           aal0379,
                           aal0472,
                           aal0473,
                           aal0518,
                           aal0541,
                           aal0585,
                           aal0586);

typedef ::testing::Types< btUnsigned64bitIntArrayNVSTester > NamedValueSet_btUnsigned64bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned64bitIntArray_tp_0, NamedValueSet_btUnsigned64bitIntArray_tp_0_Types);

