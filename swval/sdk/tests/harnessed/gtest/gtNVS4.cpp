// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btFloat,
                   TFltVerifier<btFloat>,
                   CbtFloatRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btFloatNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btFloat_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btFloat_tp_0() {}
   virtual ~NamedValueSet_btFloat_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btFloat_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0320, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0321, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0322, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0323, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0324, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0325, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0326, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0327, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0328, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0329, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0474, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0475, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0476, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0477, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0519, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0542, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0587, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0588, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0589, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, aal0590, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btFloat_tp_0,
                           aal0320,
                           aal0321,
                           aal0322,
                           aal0323,
                           aal0324,
                           aal0325,
                           aal0326,
                           aal0327,
                           aal0328,
                           aal0329,
                           aal0474,
                           aal0475,
                           aal0476,
                           aal0477,
                           aal0519,
                           aal0542,
                           aal0587,
                           aal0588,
                           aal0589,
                           aal0590);

typedef ::testing::Types< btFloatNVSTester > NamedValueSet_btFloat_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btFloat_tp_0, NamedValueSet_btFloat_tp_0_Types);


typedef TArrayNVSTester<btFloat,
                        TFltVerifier<btFloat>,
                        CbtFloatArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btFloatArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btFloatArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btFloatArray_tp_0() {}
   virtual ~NamedValueSet_btFloatArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btFloatArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0380, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0381, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0382, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0383, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0384, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0385, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0478, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0479, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0520, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0543, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0591, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, aal0592, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btFloatArray_tp_0,
                           aal0380,
                           aal0381,
                           aal0382,
                           aal0383,
                           aal0384,
                           aal0385,
                           aal0478,
                           aal0479,
                           aal0520,
                           aal0543,
                           aal0591,
                           aal0592);

typedef ::testing::Types< btFloatArrayNVSTester > NamedValueSet_btFloatArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btFloatArray_tp_0, NamedValueSet_btFloatArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btcString,
                   TStrVerifier<btcString>,
                   CbtcStringRandomizer,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btcStringNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btcString_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btcString_tp_0() {}
   virtual ~NamedValueSet_btcString_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0330, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0331, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0332, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0333, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0334, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0335, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0336, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0337, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0338, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0339, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0480, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0481, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0482, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0483, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0521, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0544, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0593, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0594, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0595, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_0, aal0596, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_0,
                           aal0330,
                           aal0331,
                           aal0332,
                           aal0333,
                           aal0334,
                           aal0335,
                           aal0336,
                           aal0337,
                           aal0338,
                           aal0339,
                           aal0480,
                           aal0481,
                           aal0482,
                           aal0483,
                           aal0521,
                           aal0544,
                           aal0593,
                           aal0594,
                           aal0595,
                           aal0596);

typedef ::testing::Types< btcStringNVSTester > NamedValueSet_btcString_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_0, NamedValueSet_btcString_tp_0_Types);

///////////////////////
// zero-length strings

typedef TNVSTester<btcString,
                   TStrVerifier<btcString>,
                   CbtcStringRandomizer_WithZeroLengthStrings,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btcStringNVSTester_ZeroLengthStrings;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btcString_tp_1 : public ::testing::Test
{
protected:
   NamedValueSet_btcString_tp_1() {}
   virtual ~NamedValueSet_btcString_tp_1() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_1);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0398, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0399, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0400, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0401, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0402, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0403, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0404, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0405, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0406, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0407, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0484, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0485, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0486, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0487, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0522, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0545, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0597, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0598, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0599, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_1, aal0600, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_1,
                           aal0398,
                           aal0399,
                           aal0400,
                           aal0401,
                           aal0402,
                           aal0403,
                           aal0404,
                           aal0405,
                           aal0406,
                           aal0407,
                           aal0484,
                           aal0485,
                           aal0486,
                           aal0487,
                           aal0522,
                           aal0545,
                           aal0597,
                           aal0598,
                           aal0599,
                           aal0600);

typedef ::testing::Types< btcStringNVSTester_ZeroLengthStrings > NamedValueSet_btcString_tp_1_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_1, NamedValueSet_btcString_tp_1_Types);

///////////////////////
// very long strings

typedef TNVSTester<btcString,
                   TStrVerifier<btcString>,
                   CbtcStringRandomizer_WithLongStrings,
                   CbtNumberKeySequencer,
                   CbtStringKeySequencer> btcStringNVSTester_LongStrings;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btcString_tp_2 : public ::testing::Test
{
protected:
   NamedValueSet_btcString_tp_2() {}
   virtual ~NamedValueSet_btcString_tp_2() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_2);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0414, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0415, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0416, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0417, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0418, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0419, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0420, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0421, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0422, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0423, ChevronsbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0488, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0489, ToFromStrbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0490, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0491, ToFromStrbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0523, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0546, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0601, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0602, WriteOneReadbtNumberKeyTest_FILE_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0603, WriteOneReadbtStringKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btcString_tp_2, aal0604, WriteOneReadbtStringKeyTest_FILE_B)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btcString_tp_2,
                           aal0414,
                           aal0415,
                           aal0416,
                           aal0417,
                           aal0418,
                           aal0419,
                           aal0420,
                           aal0421,
                           aal0422,
                           aal0423,
                           aal0488,
                           aal0489,
                           aal0490,
                           aal0491,
                           aal0523,
                           aal0546,
                           aal0601,
                           aal0602,
                           aal0603,
                           aal0604);

typedef ::testing::Types< btcStringNVSTester_LongStrings > NamedValueSet_btcString_tp_2_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_2, NamedValueSet_btcString_tp_2_Types);




typedef TArrayNVSTester<btString,
                        TStrVerifier<btString>,
                        CbtStringArrayProvider,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btStringArrayNVSTester;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btStringArray_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_btStringArray_tp_0() {}
   virtual ~NamedValueSet_btStringArray_tp_0() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_0);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0386, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0387, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0388, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0389, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0390, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0391, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0492, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0493, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0524, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0547, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0605, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_0, aal0606, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_0,
                           aal0386,
                           aal0387,
                           aal0388,
                           aal0389,
                           aal0390,
                           aal0391,
                           aal0492,
                           aal0493,
                           aal0524,
                           aal0547,
                           aal0605,
                           aal0606);

typedef ::testing::Types< btStringArrayNVSTester > NamedValueSet_btStringArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_0, NamedValueSet_btStringArray_tp_0_Types);

///////////////////////////////
// zero-length strings in array

typedef TArrayNVSTester<btString,
                        TStrVerifier<btString>,
                        CbtStringArrayProvider_WithZeroLengthStrings,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btStringArrayNVSTester_ZeroLengthStrings;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btStringArray_tp_1 : public ::testing::Test
{
protected:
   NamedValueSet_btStringArray_tp_1() {}
   virtual ~NamedValueSet_btStringArray_tp_1() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_1);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0408, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0409, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0410, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0411, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0412, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0413, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0494, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0495, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0525, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0548, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0607, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_1, aal0608, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_1,
                           aal0408,
                           aal0409,
                           aal0410,
                           aal0411,
                           aal0412,
                           aal0413,
                           aal0494,
                           aal0495,
                           aal0525,
                           aal0548,
                           aal0607,
                           aal0608);

typedef ::testing::Types< btStringArrayNVSTester_ZeroLengthStrings > NamedValueSet_btStringArray_tp_1_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_1, NamedValueSet_btStringArray_tp_1_Types);

/////////////////////////////
// very long strings in array

typedef TArrayNVSTester<btString,
                        TStrVerifier<btString>,
                        CbtStringArrayProvider_WithLongStrings,
                        CbtNumberKeySequencer,
                        CbtStringKeySequencer> btStringArrayNVSTester_LongStrings;

// Type-parameterized test fixture
template <typename T>
class NamedValueSet_btStringArray_tp_2 : public ::testing::Test
{
protected:
   NamedValueSet_btStringArray_tp_2() {}
   virtual ~NamedValueSet_btStringArray_tp_2() {}
   NamedValueSet m_NVS;
};

TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_2);

MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0424, AddGetbtNumberKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0425, AddGetbtStringKeyTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0426, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0427, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0428, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0429, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0496, ToFromStrbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0497, ToFromStrbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0526, EqualityAndSubsetTest)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0549, CopyConstructorTest)
#ifdef NVSFileIO
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0609, WriteOneReadbtNumberKeyTest_FILE_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btStringArray_tp_2, aal0610, WriteOneReadbtStringKeyTest_FILE_A)
#endif // NVSFileIO

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_2,
                           aal0424,
                           aal0425,
                           aal0426,
                           aal0427,
                           aal0428,
                           aal0429,
                           aal0496,
                           aal0497,
                           aal0526,
                           aal0549,
                           aal0609,
                           aal0610);

typedef ::testing::Types< btStringArrayNVSTester_LongStrings > NamedValueSet_btStringArray_tp_2_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_2, NamedValueSet_btStringArray_tp_2_Types);

