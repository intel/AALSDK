// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<bt32bitInt,
                   TIntVerifier<bt32bitInt>,
                   Cbt32bitIntRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> bt32bitIntNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_bt32bitInt_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_bt32bitInt_tp_0() {}
   virtual ~NamedValueSet_bt32bitInt_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_bt32bitInt_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0280, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0281, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0282, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0283, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0284, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0285, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0286, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0287, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0288, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0289, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0450, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0451, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0452, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0453, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0511, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0534, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0563, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0564, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0565, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitInt_tp_0, aal0566, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt32bitInt_tp_0,
                           aal0280,
                           aal0281,
                           aal0282,
                           aal0283,
                           aal0284,
                           aal0285,
                           aal0286,
                           aal0287,
                           aal0288,
                           aal0289,
                           aal0450,
                           aal0451,
                           aal0452,
                           aal0453,
                           aal0511,
                           aal0534,
                           aal0563,
                           aal0564,
                           aal0565,
                           aal0566);

typedef ::testing::Types< bt32bitIntNVSTester > NamedValueSet_bt32bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt32bitInt_tp_0, NamedValueSet_bt32bitInt_tp_0_Types);


typedef TArrayNVSTester<bt32bitInt,
                        TIntVerifier<bt32bitInt>,
                        Cbt32bitIntArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> bt32bitIntArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_bt32bitIntArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_bt32bitIntArray_tp_0() {}
   virtual ~NamedValueSet_bt32bitIntArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_bt32bitIntArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0356, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0357, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0358, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0359, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0360, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0361, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0454, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0455, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0512, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0535, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0567, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_bt32bitIntArray_tp_0, aal0568, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt32bitIntArray_tp_0,
                           aal0356,
                           aal0357,
                           aal0358,
                           aal0359,
                           aal0360,
                           aal0361,
                           aal0454,
                           aal0455,
                           aal0512,
                           aal0535,
                           aal0567,
                           aal0568);

typedef ::testing::Types< bt32bitIntArrayNVSTester > NamedValueSet_bt32bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt32bitIntArray_tp_0, NamedValueSet_bt32bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btUnsigned32bitInt,
                   TIntVerifier<btUnsigned32bitInt>,
                   CbtUnsigned32bitIntRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btUnsigned32bitIntNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btUnsigned32bitInt_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btUnsigned32bitInt_tp_0() {}
   virtual ~NamedValueSet_btUnsigned32bitInt_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btUnsigned32bitInt_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0290, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0291, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0292, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0293, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0294, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0295, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0296, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0297, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0298, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0299, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0456, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0457, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0458, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0459, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0513, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0536, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0569, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0570, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0571, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitInt_tp_0, aal0572, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned32bitInt_tp_0,
                           aal0290,
                           aal0291,
                           aal0292,
                           aal0293,
                           aal0294,
                           aal0295,
                           aal0296,
                           aal0297,
                           aal0298,
                           aal0299,
                           aal0456,
                           aal0457,
                           aal0458,
                           aal0459,
                           aal0513,
                           aal0536,
                           aal0569,
                           aal0570,
                           aal0571,
                           aal0572);

typedef ::testing::Types< btUnsigned32bitIntNVSTester > NamedValueSet_btUnsigned32bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned32bitInt_tp_0, NamedValueSet_btUnsigned32bitInt_tp_0_Types);


typedef TArrayNVSTester<btUnsigned32bitInt,
                        TIntVerifier<btUnsigned32bitInt>,
                        CbtUnsigned32bitIntArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btUnsigned32bitIntArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btUnsigned32bitIntArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btUnsigned32bitIntArray_tp_0() {}
   virtual ~NamedValueSet_btUnsigned32bitIntArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btUnsigned32bitIntArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0362, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0363, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0364, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0365, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0366, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0367, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0460, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0461, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0514, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0537, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0573, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btUnsigned32bitIntArray_tp_0, aal0574, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned32bitIntArray_tp_0,
                           aal0362,
                           aal0363,
                           aal0364,
                           aal0365,
                           aal0366,
                           aal0367,
                           aal0460,
                           aal0461,
                           aal0514,
                           aal0537,
                           aal0573,
                           aal0574);

typedef ::testing::Types< btUnsigned32bitIntArrayNVSTester > NamedValueSet_btUnsigned32bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned32bitIntArray_tp_0, NamedValueSet_btUnsigned32bitIntArray_tp_0_Types);

