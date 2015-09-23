// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

#if 0
TEST(CbtNumberKeySequencer, verification)
{
   CbtNumberKeySequencer s;

   EXPECT_EQ(0, s.Value());
   EXPECT_EQ(1, s.Value());

   s.Snapshot();

   EXPECT_EQ(2, s.Value());
   EXPECT_EQ(3, s.Value());
   EXPECT_EQ(4, s.Value());
   EXPECT_EQ(0, s.Value());

   s.Replay();

   EXPECT_EQ(2, s.Value());
   EXPECT_EQ(3, s.Value());
   EXPECT_EQ(4, s.Value());
   EXPECT_EQ(0, s.Value());
}

TEST(CbtStringKeySequencer, verification)
{
   CbtStringKeySequencer s;

   EXPECT_STREQ("0", s.Value());
   EXPECT_STREQ("1", s.Value());

   s.Snapshot();

   EXPECT_STREQ("2", s.Value());
   EXPECT_STREQ("3", s.Value());
   EXPECT_STREQ("4", s.Value());
   EXPECT_STREQ("0", s.Value());

   s.Replay();

   EXPECT_STREQ("2", s.Value());
   EXPECT_STREQ("3", s.Value());
   EXPECT_STREQ("4", s.Value());
   EXPECT_STREQ("0", s.Value());
}

TEST(Cbt32bitIntRandomizer, verification)
{
   Cbt32bitIntRandomizer r;

   bt32bitInt v[5];

   v[0] = r.Value();

   r.Snapshot();

   v[1] = r.Value();
   v[2] = r.Value();
   v[3] = r.Value();
   v[4] = r.Value();

   r.Replay();

   EXPECT_EQ(v[1], r.Value());
   EXPECT_EQ(v[2], r.Value());
   EXPECT_EQ(v[3], r.Value());
   EXPECT_EQ(v[4], r.Value());

   unsigned i;
   for ( i = 0 ; i < 100 ; ++i ) {
      EXPECT_NE(v[0], r.ValueOtherThan(v[0]));
   }
}

TEST(CbtcStringRandomizer_WithLongStrings, verification)
{
   CbtcStringRandomizer_WithLongStrings r;

   btcString v[5];

   v[0] = r.Value();

   r.Snapshot();

   v[1] = r.Value();
   v[2] = r.Value();
   v[3] = r.Value();
   v[4] = r.Value();

   r.Replay();

   EXPECT_STREQ(v[1], r.Value());
   EXPECT_STREQ(v[2], r.Value());
   EXPECT_STREQ(v[3], r.Value());
   EXPECT_STREQ(v[4], r.Value());

   unsigned i;
   for ( i = 0 ; i < 100 ; ++i ) {
      EXPECT_STRNE(v[0], r.ValueOtherThan(v[0]));
   }
}
#endif // 0

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btBool,
                   TIntVerifier<btBool>,
                   CbtBoolRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btBoolNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btBool_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btBool_tp_0() {}
   virtual ~NamedValueSet_btBool_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btBool_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0260, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0261, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0262, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0263, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0264, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0265, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0266, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0267, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0268, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0269, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0440, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0441, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0442, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0443, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0508, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0531, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0553, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0554, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0555, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btBool_tp_0, aal0556, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btBool_tp_0,
                           aal0260,
                           aal0261,
                           aal0262,
                           aal0263,
                           aal0264,
                           aal0265,
                           aal0266,
                           aal0267,
                           aal0268,
                           aal0269,
                           aal0440,
                           aal0441,
                           aal0442,
                           aal0443,
                           aal0508,
                           aal0531,
                           aal0553,
                           aal0554,
                           aal0555,
                           aal0556);

typedef ::testing::Types< btBoolNVSTester > NamedValueSet_btBool_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btBool_tp_0, NamedValueSet_btBool_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btByte,
                   TIntVerifier<btByte>,
                   CbtByteRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btByteNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btByte_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btByte_tp_0() {}
   virtual ~NamedValueSet_btByte_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btByte_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0270, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0271, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0272, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0273, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0274, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0275, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0276, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0277, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0278, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0279, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0444, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0445, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0446, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0447, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0509, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0532, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0557, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0558, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0559, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByte_tp_0, aal0560, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btByte_tp_0,
                           aal0270,
                           aal0271,
                           aal0272,
                           aal0273,
                           aal0274,
                           aal0275,
                           aal0276,
                           aal0277,
                           aal0278,
                           aal0279,
                           aal0444,
                           aal0445,
                           aal0446,
                           aal0447,
                           aal0509,
                           aal0532,
                           aal0557,
                           aal0558,
                           aal0559,
                           aal0560);

typedef ::testing::Types< btByteNVSTester > NamedValueSet_btByte_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btByte_tp_0, NamedValueSet_btByte_tp_0_Types);


typedef TArrayNVSTester<btByte,
                        TIntVerifier<btByte>,
                        CbtByteArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btByteArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btByteArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btByteArray_tp_0() {}
   virtual ~NamedValueSet_btByteArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btByteArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0350, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0351, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0352, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0353, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0354, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0355, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0448, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0449, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0510, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0533, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0561, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btByteArray_tp_0, aal0562, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btByteArray_tp_0,
                           aal0350,
                           aal0351,
                           aal0352,
                           aal0353,
                           aal0354,
                           aal0355,
                           aal0448,
                           aal0449,
                           aal0510,
                           aal0533,
                           aal0561,
                           aal0562);

typedef ::testing::Types< btByteArrayNVSTester > NamedValueSet_btByteArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btByteArray_tp_0, NamedValueSet_btByteArray_tp_0_Types);

