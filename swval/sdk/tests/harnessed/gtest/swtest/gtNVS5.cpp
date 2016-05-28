// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btObjectType,
                   TIntVerifier<btObjectType>,
                   CbtObjectTypeRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btObjectTypeNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btObjectType_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btObjectType_tp_0() {}
   virtual ~NamedValueSet_btObjectType_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btObjectType_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0340, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0341, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0342, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0343, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0344, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0345, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0346, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0347, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0348, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0349, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0498, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0499, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0500, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0501, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0527, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0550, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0611, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0612, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0613, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectType_tp_0, aal0614, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btObjectType_tp_0,
                           aal0340,
                           aal0341,
                           aal0342,
                           aal0343,
                           aal0344,
                           aal0345,
                           aal0346,
                           aal0347,
                           aal0348,
                           aal0349,
                           aal0498,
                           aal0499,
                           aal0500,
                           aal0501,
                           aal0527,
                           aal0550,
                           aal0611,
                           aal0612,
                           aal0613,
                           aal0614);

typedef ::testing::Types< btObjectTypeNVSTester > NamedValueSet_btObjectType_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btObjectType_tp_0, NamedValueSet_btObjectType_tp_0_Types);


typedef TArrayNVSTester<btObjectType,
                        TIntVerifier<btObjectType>,
                        CbtObjectArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btObjectArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btObjectArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btObjectArray_tp_0() {}
   virtual ~NamedValueSet_btObjectArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btObjectArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0392, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0393, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0394, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0395, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0396, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0397, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0502, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0503, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0528, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0551, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0615, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btObjectArray_tp_0, aal0616, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btObjectArray_tp_0,
                           aal0392,
                           aal0393,
                           aal0394,
                           aal0395,
                           aal0396,
                           aal0397,
                           aal0502,
                           aal0503,
                           aal0528,
                           aal0551,
                           aal0615,
                           aal0616);

typedef ::testing::Types< btObjectArrayNVSTester > NamedValueSet_btObjectArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btObjectArray_tp_0, NamedValueSet_btObjectArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<const INamedValueSet *,
                   NVSVerifier,
                   CNVSRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> NestedNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_Nested_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_Nested_tp_0() {}
   virtual ~NamedValueSet_Nested_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_Nested_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0430, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0431, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0432, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0433, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0434, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0435, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0436, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0437, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0438, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0439, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0504, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0505, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0506, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0507, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0529, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0552, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0617, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0618, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0619, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_Nested_tp_0, aal0620, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_Nested_tp_0,
                           aal0430,
                           aal0431,
                           aal0432,
                           aal0433,
                           aal0434,
                           aal0435,
                           aal0436,
                           aal0437,
                           aal0438,
                           aal0439,
                           aal0504,
                           aal0505,
                           aal0506,
                           aal0507,
                           aal0529,
                           aal0552,
                           aal0617,
                           aal0618,
                           aal0619,
                           aal0620);

typedef ::testing::Types< NestedNVSTester > NamedValueSet_Nested_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_Nested_tp_0, NamedValueSet_Nested_tp_0_Types);
