// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTSEQRAND_H__
#define __GTSEQRAND_H__

////////////////////////////////////////////////////////////////////////////////

template <typename I>
class TIntVerifier
{
public:
   TIntVerifier() {}
   void expect_eq(I a, I b) { EXPECT_EQ(a, b); }
   void expect_ne(I a, I b) { EXPECT_NE(a, b); }
};

template <typename F>
class TFltVerifier
{
public:
   TFltVerifier() {}
   void expect_eq(F a, F b) { FAIL(); }
   void expect_ne(F a, F b) { EXPECT_NE(a, b); }
};

// specialization for btFloat.
template <>
class TFltVerifier<btFloat>
{
public:
   TFltVerifier() {}
   void expect_eq(btFloat a, btFloat b) { EXPECT_FLOAT_EQ(a, b); }
};

template <typename S>
class TStrVerifier
{
public:
   TStrVerifier() {}
   void expect_eq(S a, S b) { EXPECT_STREQ(a, b); }
   void expect_ne(S a, S b) { EXPECT_STRNE(a, b); }
};

class NVSVerifier
{
public:
   NVSVerifier() {}
   void expect_eq(const INamedValueSet *a, const INamedValueSet *b)
   {
      EXPECT_TRUE( a->operator == (*b) ) << *a << "\nvs.\n" << *b;
   }
   void expect_ne(const INamedValueSet *a, const INamedValueSet *b)
   {
      EXPECT_FALSE( a->operator == (*b) ) << *a << "\nvs.\n" << *b;
   }
};

////////////////////////////////////////////////////////////////////////////////

template <typename I>
class TIntCompare
{
public:
   TIntCompare() {}
   bool equal(I a, I b) { return a == b; }
};

template <typename F>
class TFltCompare
{
public:
   TFltCompare() {}
   bool equal(F a, F b) { return a == b; }
};

template <typename S>
class TStrCompare
{
public:
   TStrCompare() {}
   bool equal(S a, S b) { return (0 == ::strcmp(a, b)); }
};

class NVSCompare
{
public:
   NVSCompare() {}
   bool equal(const INamedValueSet *a, const INamedValueSet *b) { return a->operator == (*b); }
};

////////////////////////////////////////////////////////////////////////////////

template <typename X, typename Compare>
class TValueSequencer
{
public:
   TValueSequencer(const X *pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count),
      m_i(0),
      m_Savei(0)
   {}
   virtual ~TValueSequencer() {}

   void Snapshot() { m_Savei = m_i; }
   void Replay()   { m_i = m_Savei; }

   const X & Value()
   {
      btUnsigned32bitInt i = m_i;
      m_i = (m_i + 1) % m_Count;
      return *(m_pValues + i);
   }

   const X & ValueOtherThan(const X &x)
   {
      Compare            c;
      btUnsigned32bitInt i;
      do
      {
         i = m_i;
         m_i = (m_i + 1) % m_Count;
      }while ( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

   btUnsigned32bitInt Count() const { return m_Count; }

   btUnsigned32bitInt Seed(btUnsigned32bitInt ) { return 0; }

   virtual eBasicTypes BasicType() const = 0;

protected:
   TValueSequencer() {}

   const X                 *m_pValues;
   const btUnsigned32bitInt m_Count;
   btUnsigned32bitInt       m_i;
   btUnsigned32bitInt       m_Savei;
};

template <typename X, typename Compare>
class TValueRandomizer
{
public:
   TValueRandomizer(const X *pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count),
      m_Seed(0),
      m_SaveSeed(0)
   {}
   virtual ~TValueRandomizer() {}

   void Snapshot() { m_SaveSeed = m_Seed;     }
   void Replay()   { m_Seed     = m_SaveSeed; }

   const X & Value()
   {
      btUnsigned32bitInt i = ::GetRand(&m_Seed) % m_Count;
      return *(m_pValues + i);
   }

   const X & ValueOtherThan(const X &x)
   {
      Compare            c;
      btUnsigned32bitInt i;
      do
      {
         i = ::GetRand(&m_Seed) % m_Count;
      }while ( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

   btUnsigned32bitInt Count() const { return m_Count; }

   btUnsigned32bitInt Seed(btUnsigned32bitInt s)
   {
      btUnsigned32bitInt prev = m_Seed;
      m_Seed = s;
      return prev;
   }

   virtual eBasicTypes BasicType() const = 0;

protected:
   TValueRandomizer() {}

   const X                 *m_pValues;
   const btUnsigned32bitInt m_Count;
   btUnsigned32bitInt       m_Seed;
   btUnsigned32bitInt       m_SaveSeed;
};

template <typename ElemT, typename Compare, typename ArrT>
class TArrayProvider
{
public:
   TArrayProvider(const ArrT pValues, btUnsigned32bitInt Count) :
      m_pValues(pValues),
      m_Count(Count)
   {}
   virtual ~TArrayProvider() {}

                const ArrT Array() const { return m_pValues; }
        btUnsigned32bitInt Count() const { return m_Count;   }
   virtual eBasicTypes BasicType() const = 0;

   const ElemT & ValueOtherThan(const ElemT &x)
   {
      Compare            c;
      btUnsigned32bitInt i = m_Count - 1;
      do
      {
         i = (i + 1) % m_Count;
      }while( c.equal(*(m_pValues + i), x) );
      return *(m_pValues + i);
   }

protected:
   TArrayProvider() {}

   const ArrT               m_pValues;
   const btUnsigned32bitInt m_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtNumberKeySequencer : public TValueSequencer< btNumberKey, TIntCompare<btNumberKey> >
{
public:
   CbtNumberKeySequencer();

   virtual eBasicTypes BasicType() const { return btUnsigned64bitInt_t; }

   static const btNumberKey        sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtStringKeySequencer : public TValueSequencer< btStringKey, TStrCompare<btStringKey> >
{
public:
   CbtStringKeySequencer();

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btStringKey        sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtBoolRandomizer : public TValueRandomizer< btBool, TIntCompare<btBool> >
{
public:
   CbtBoolRandomizer();

   virtual eBasicTypes BasicType() const { return btBool_t; }

   static const btBool             sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtByteRandomizer : public TValueRandomizer< btByte, TIntCompare<btByte> >
{
public:
   CbtByteRandomizer();

   virtual eBasicTypes BasicType() const { return btByte_t; }

   static const btByte             sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class Cbt32bitIntRandomizer : public TValueRandomizer< bt32bitInt, TIntCompare<bt32bitInt> >
{
public:
   Cbt32bitIntRandomizer();

   virtual eBasicTypes BasicType() const { return bt32bitInt_t; }

   static const bt32bitInt         sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtUnsigned32bitIntRandomizer : public TValueRandomizer< btUnsigned32bitInt, TIntCompare<btUnsigned32bitInt> >
{
public:
   CbtUnsigned32bitIntRandomizer();

   virtual eBasicTypes BasicType() const { return btUnsigned32bitInt_t; }

   static const btUnsigned32bitInt sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class Cbt64bitIntRandomizer : public TValueRandomizer< bt64bitInt, TIntCompare<bt64bitInt> >
{
public:
   Cbt64bitIntRandomizer();

   virtual eBasicTypes BasicType() const { return bt64bitInt_t; }

   static const bt64bitInt         sm_Vals[];
   static const btUnsigned64bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtUnsigned64bitIntRandomizer : public TValueRandomizer< btUnsigned64bitInt, TIntCompare<btUnsigned64bitInt> >
{
public:
   CbtUnsigned64bitIntRandomizer();

   virtual eBasicTypes BasicType() const { return btUnsigned64bitInt_t; }

   static const btUnsigned64bitInt sm_Vals[];
   static const btUnsigned64bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtFloatRandomizer : public TValueRandomizer< btFloat, TFltCompare<btFloat> >
{
public:
   CbtFloatRandomizer();

   virtual eBasicTypes BasicType() const { return btFloat_t; }

   static const btFloat            sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtcStringRandomizer : public TValueRandomizer< btcString, TStrCompare<btcString> >
{
public:
   CbtcStringRandomizer();

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

class CbtcStringRandomizer_WithZeroLengthStrings : public TValueRandomizer< btcString, TStrCompare<btcString> >
{
public:
   CbtcStringRandomizer_WithZeroLengthStrings();

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

class CbtcStringRandomizer_WithLongStrings : public TValueRandomizer< btcString, TStrCompare<btcString> >
{
public:
   CbtcStringRandomizer_WithLongStrings();

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtObjectTypeRandomizer : public TValueRandomizer< btObjectType, TIntCompare<btObjectType> >
{
public:
   CbtObjectTypeRandomizer();

   virtual eBasicTypes BasicType() const { return btObjectType_t; }

   static const btObjectType       sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtByteArrayProvider : public TArrayProvider< btByte, TIntCompare<btByte>, btByteArray >
{
public:
   CbtByteArrayProvider();

   virtual eBasicTypes BasicType() const { return btByteArray_t; }

   static const btByte             sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class Cbt32bitIntArrayProvider : public TArrayProvider< bt32bitInt, TIntCompare<bt32bitInt>, bt32bitIntArray >
{
public:
   Cbt32bitIntArrayProvider();

   virtual eBasicTypes BasicType() const { return bt32bitIntArray_t; }

   static const bt32bitInt         sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtUnsigned32bitIntArrayProvider :
   public TArrayProvider< btUnsigned32bitInt, TIntCompare<btUnsigned32bitInt>, btUnsigned32bitIntArray >
{
public:
   CbtUnsigned32bitIntArrayProvider();

   virtual eBasicTypes BasicType() const { return btUnsigned32bitIntArray_t; }

   static const btUnsigned32bitInt sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class Cbt64bitIntArrayProvider : public TArrayProvider< bt64bitInt, TIntCompare<bt64bitInt>, bt64bitIntArray >
{
public:
   Cbt64bitIntArrayProvider();

   virtual eBasicTypes BasicType() const { return bt64bitIntArray_t; }

   static const bt64bitInt         sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtUnsigned64bitIntArrayProvider :
   public TArrayProvider< btUnsigned64bitInt, TIntCompare<btUnsigned64bitInt>, btUnsigned64bitIntArray >
{
public:
   CbtUnsigned64bitIntArrayProvider();

   virtual eBasicTypes BasicType() const { return btUnsigned64bitIntArray_t; }

   static const btUnsigned64bitInt sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtFloatArrayProvider : public TArrayProvider< btFloat, TFltCompare<btFloat>, btFloatArray >
{
public:
   CbtFloatArrayProvider();

   virtual eBasicTypes BasicType() const { return btFloatArray_t; }

   static const btFloat            sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtStringArrayProvider : public TArrayProvider< btString, TStrCompare<btString>, btStringArray >
{
public:
   CbtStringArrayProvider();

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

class CbtStringArrayProvider_WithZeroLengthStrings :
   public TArrayProvider< btString, TStrCompare<btString>, btStringArray >
{
public:
   CbtStringArrayProvider_WithZeroLengthStrings();

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

class CbtStringArrayProvider_WithLongStrings :
   public TArrayProvider< btString, TStrCompare<btString>, btStringArray >
{
public:
   CbtStringArrayProvider_WithLongStrings();

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

class CbtObjectArrayProvider : public TArrayProvider< btObjectType, TIntCompare<btObjectType>, btObjectArray >
{
public:
   CbtObjectArrayProvider();

   virtual eBasicTypes BasicType() const { return btObjectArray_t; }

   static const btObjectType       sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __GTSEQRAND_H__

