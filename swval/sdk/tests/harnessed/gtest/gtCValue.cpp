// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "aalsdk/AALNamedValueSet.h"

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class AAS_CValue_f : public ::testing::Test
{
protected:
   AAS_CValue_f() {}
   //virtual void SetUp() { }
   //virtual void TearDown() { }

   CValue m_Val[4]; // AALCNamedValueSet.h
};

TEST_F(AAS_CValue_f, aal0215)
{
   // The CValue assignment operator leaks no memory (btByteArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btByte      A[4] = { 'a', 'a', 'a', 0 };
   btByte      B[4] = { 'b', 'b', 'b', 0 };
   btByteArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btByteArray_t, m_Val[2].Type());
   EXPECT_EQ(4,             m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(btByte)));

   EXPECT_EQ(btByteArray_t, m_Val[3].Type());
   EXPECT_EQ(4,             m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(btByte)));


   m_Val[0] = m_Val[2]; // scalar      -> btByteArray
   m_Val[2] = m_Val[1]; // btByteArray -> scalar
   m_Val[3] = m_Val[0]; // btByteArray -> btByteArray


   EXPECT_EQ(btByteArray_t, m_Val[0].Type());
   EXPECT_EQ(4,             m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btByteArray_t, m_Val[3].Type());
   EXPECT_EQ(4,             m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0216)
{
   // The CValue assignment operator leaks no memory (bt32bitIntArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   bt32bitInt      A[4] = { 'a', 'a', 'a', 0 };
   bt32bitInt      B[4] = { 'b', 'b', 'b', 0 };
   bt32bitIntArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(bt32bitIntArray_t, m_Val[2].Type());
   EXPECT_EQ(4,                 m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(bt32bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(bt32bitIntArray_t, m_Val[0].Type());
   EXPECT_EQ(4,                 m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(bt32bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0217)
{
   // The CValue assignment operator leaks no memory (btUnsigned32bitIntArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btUnsigned32bitInt      A[4] = { 'a', 'a', 'a', 0 };
   btUnsigned32bitInt      B[4] = { 'b', 'b', 'b', 0 };
   btUnsigned32bitIntArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btUnsigned32bitIntArray_t, m_Val[2].Type());
   EXPECT_EQ(4,                         m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btUnsigned32bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                         m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(btUnsigned32bitIntArray_t, m_Val[0].Type());
   EXPECT_EQ(4,                         m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btUnsigned32bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                         m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0218)
{
   // The CValue assignment operator leaks no memory (bt64bitIntArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   bt64bitInt      A[4] = { 'a', 'a', 'a', 0 };
   bt64bitInt      B[4] = { 'b', 'b', 'b', 0 };
   bt64bitIntArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(bt64bitIntArray_t, m_Val[2].Type());
   EXPECT_EQ(4,                 m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(bt64bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(bt64bitIntArray_t, m_Val[0].Type());
   EXPECT_EQ(4,                 m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(bt64bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0219)
{
   // The CValue assignment operator leaks no memory (btUnsigned64bitIntArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btUnsigned64bitInt      A[4] = { 'a', 'a', 'a', 0 };
   btUnsigned64bitInt      B[4] = { 'b', 'b', 'b', 0 };
   btUnsigned64bitIntArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btUnsigned64bitIntArray_t, m_Val[2].Type());
   EXPECT_EQ(4,                         m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btUnsigned64bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                         m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(btUnsigned64bitIntArray_t, m_Val[0].Type());
   EXPECT_EQ(4,                         m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btUnsigned64bitIntArray_t, m_Val[3].Type());
   EXPECT_EQ(4,                         m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0220)
{
   // The CValue assignment operator leaks no memory (btFloatArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btFloat      A[4] = { 0.0, 1.0, 2.0, 3.0 };
   btFloat      B[4] = { 4.0, 5.0, 6.0, 7.0 };
   btFloatArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btFloatArray_t, m_Val[2].Type());
   EXPECT_EQ(4,              m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btFloatArray_t, m_Val[3].Type());
   EXPECT_EQ(4,              m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(btFloatArray_t, m_Val[0].Type());
   EXPECT_EQ(4,              m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btFloatArray_t, m_Val[3].Type());
   EXPECT_EQ(4,              m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0221)
{
   // The CValue assignment operator leaks no memory (btObjectArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btObjectType  A[4] = { (btObjectType)0, (btObjectType)1, (btObjectType)2, (btObjectType)3 };
   btObjectType  B[4] = { (btObjectType)4, (btObjectType)5, (btObjectType)6, (btObjectType)7 };
   btObjectArray C;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btObjectArray_t, m_Val[2].Type());
   EXPECT_EQ(4,               m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btObjectArray_t, m_Val[3].Type());
   EXPECT_EQ(4,               m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, B, 4 * sizeof(A[0])));


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(btObjectArray_t, m_Val[0].Type());
   EXPECT_EQ(4,               m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btObjectArray_t, m_Val[3].Type());
   EXPECT_EQ(4,               m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(0, memcmp(C, A, 4 * sizeof(A[0])));
}

TEST_F(AAS_CValue_f, aal0222)
{
   // The CValue assignment operator leaks no memory (btStringArray_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btString      A[4] = { (btString)"A", (btString)"B", (btString)"C", (btString)"D" };
   btString      B[4] = { (btString)"E", (btString)"F", (btString)"G", (btString)"H" };
   btStringArray C = NULL;

   m_Val[2].Put(A, 4);
   m_Val[3].Put(B, 4);

   EXPECT_EQ(btStringArray_t, m_Val[2].Type());
   EXPECT_EQ(4,               m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));

   unsigned i;
   for ( i = 0 ; i < sizeof(A) / sizeof(A[0]) ; ++i ) {
      EXPECT_STREQ(C[i], A[i]);
   }

   EXPECT_EQ(btStringArray_t, m_Val[3].Type());
   EXPECT_EQ(4,               m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));

   for ( i = 0 ; i < sizeof(B) / sizeof(B[0]) ; ++i ) {
      EXPECT_STREQ(C[i], B[i]);
   }


   m_Val[0] = m_Val[2]; // scalar -> Array
   m_Val[2] = m_Val[1]; // Array  -> scalar
   m_Val[3] = m_Val[0]; // Array  -> Array


   EXPECT_EQ(btStringArray_t, m_Val[0].Type());
   EXPECT_EQ(4,               m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));

   for ( i = 0 ; i < sizeof(A) / sizeof(A[0]) ; ++i ) {
      EXPECT_STREQ(C[i], A[i]);
   }

   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btStringArray_t, m_Val[3].Type());
   EXPECT_EQ(4,               m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));

   for ( i = 0 ; i < sizeof(A) / sizeof(A[0]) ; ++i ) {
      EXPECT_STREQ(C[i], A[i]);
   }
}

TEST_F(AAS_CValue_f, aal0223)
{
   // The CValue assignment operator leaks no memory (btString_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   btcString A = "A";
   btcString B = "B";
   btcString C;

   m_Val[2].Put(A);
   m_Val[3].Put(B);

   EXPECT_EQ(btString_t, m_Val[2].Type());
   EXPECT_EQ(1,          m_Val[2].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_STREQ(C, A);

   EXPECT_EQ(btString_t, m_Val[3].Type());
   EXPECT_EQ(1,          m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_STREQ(C, B);

   m_Val[0] = m_Val[2]; // scalar -> String
   m_Val[2] = m_Val[1]; // String -> scalar
   m_Val[3] = m_Val[0]; // String -> String


   EXPECT_EQ(btString_t, m_Val[0].Type());
   EXPECT_EQ(1,          m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_STREQ(C, A);


   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btString_t, m_Val[3].Type());
   EXPECT_EQ(1,          m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_STREQ(C, A);
}

TEST_F(AAS_CValue_f, aal0224)
{
   // The CValue assignment operator leaks no memory (btNamedValueSet_t).

   btBool b[2] = { true, false };
   btBool c;
   m_Val[0].Put(b[0]);
   m_Val[1].Put(b[1]);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   c = false;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);

   EXPECT_EQ(btBool_t, m_Val[1].Type());
   EXPECT_EQ(1,        m_Val[1].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[1].Get(&c));
   EXPECT_FALSE(c);


   NamedValueSet A;
   EXPECT_EQ(ENamedValuesOK, A.Add((btNumberKey)0,     (btInt)1));
   EXPECT_EQ(ENamedValuesOK, A.Add((btStringKey)"val", (btFloat)1.0));

   NamedValueSet B;
   EXPECT_EQ(ENamedValuesOK, B.Add((btNumberKey)0,     (btInt)2));
   EXPECT_EQ(ENamedValuesOK, B.Add((btStringKey)"val", (btFloat)2.0));

   const INamedValueSet *C;

   m_Val[2].Put(&A);
   m_Val[3].Put(&B);

   EXPECT_EQ(btNamedValueSet_t, m_Val[2].Type());
   EXPECT_EQ(1,                 m_Val[2].Size());

   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&C));
   EXPECT_EQ(*C, A);

   EXPECT_EQ(btNamedValueSet_t, m_Val[3].Type());
   EXPECT_EQ(1,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(*C, B);

   m_Val[0] = m_Val[2]; // scalar -> NVS
   m_Val[2] = m_Val[1]; // NVS    -> scalar
   m_Val[3] = m_Val[0]; // NVS    -> NVS


   EXPECT_EQ(btNamedValueSet_t, m_Val[0].Type());
   EXPECT_EQ(1,                 m_Val[0].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_EQ(*C, A);


   EXPECT_EQ(btBool_t,      m_Val[1].Type());
   EXPECT_EQ(1,             m_Val[1].Size());

   EXPECT_EQ(btBool_t,      m_Val[2].Type());
   EXPECT_EQ(1,             m_Val[2].Size());
   c = true;
   EXPECT_EQ(ENamedValuesOK, m_Val[2].Get(&c));
   EXPECT_FALSE(c);

   EXPECT_EQ(btNamedValueSet_t, m_Val[3].Type());
   EXPECT_EQ(1,                 m_Val[3].Size());
   C = NULL;
   EXPECT_EQ(ENamedValuesOK, m_Val[3].Get(&C));
   EXPECT_EQ(*C, A);
}

TEST_F(AAS_CValue_f, aal0250)
{
   // The CValue assignment operator implements a safeguard against self-assign.

   btcString A = "A";
   btcString C = NULL;

   m_Val[0].Put(A);

   CValue *p = &m_Val[0];

   m_Val[0] = *p;

   EXPECT_EQ(btString_t, m_Val[0].Type());
   EXPECT_EQ(1,          m_Val[0].Size());
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&C));
   EXPECT_STREQ(C, A);
}

TEST_F(AAS_CValue_f, aal0251)
{
   // The CValue::Get() accessor's return ENamedValuesBadType when the requested type conflicts with the contained type.   -

   btBool b = true;
   btBool c = false;
   m_Val[0].Put(b);

   EXPECT_EQ(btBool_t, m_Val[0].Type());
   EXPECT_EQ(1,        m_Val[0].Size());
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&c));
   EXPECT_TRUE(c);


   btByte by;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&by));

   bt32bitInt s32;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&s32));

   btUnsigned32bitInt u32;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&u32));

   bt64bitInt s64;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&s64));

   btUnsigned64bitInt u64;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&u64));

   btFloat f;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&f));

   btcString str;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&str));

   INamedValueSet const *nvs;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&nvs));

   btObjectType o;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&o));


   btByteArray byA;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&byA));

   bt32bitIntArray s32A;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&s32A));

   btUnsigned32bitIntArray u32A;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&u32A));

   bt64bitIntArray s64A;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&s64A));

   btUnsigned64bitIntArray u64A;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&u64A));

   btFloatArray fA;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&fA));

   btStringArray strA;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&strA));

   btObjectArray oA;
   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&oA));


   s32 = 3;
   m_Val[0].Put(s32);

   EXPECT_EQ(bt32bitInt_t, m_Val[0].Type());
   EXPECT_EQ(1,            m_Val[0].Size());
   s32 = 0;
   EXPECT_EQ(ENamedValuesOK, m_Val[0].Get(&s32));
   EXPECT_EQ(3, s32);

   EXPECT_EQ(ENamedValuesBadType, m_Val[0].Get(&b));
}

