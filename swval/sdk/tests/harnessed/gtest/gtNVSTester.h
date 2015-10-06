// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTNVSTESTER_H__
#define __GTNVSTESTER_H__
#include "aalsdk/AALNamedValueSet.h"

////////////////////////////////////////////////////////////////////////////////

#define NVS_IO_WORKAROUND 1

////////////////////////////////////////////////////////////////////////////////

class CNVSRandomizer
{
public:
   CNVSRandomizer();
   virtual ~CNVSRandomizer();

   void Snapshot();
   void Replay();

   const INamedValueSet * Value();
   const INamedValueSet * ValueOtherThan(const INamedValueSet *nvs);

   btUnsigned32bitInt Count() const { return m_Count; }

   btUnsigned32bitInt Seed(btUnsigned32bitInt s);

   virtual eBasicTypes BasicType() const { return btNamedValueSet_t; }

protected:
   btUnsigned32bitInt m_Count;
   btUnsigned32bitInt m_Seed;
   btUnsigned32bitInt m_SaveSeed;

   INamedValueSet *  Zero();
   INamedValueSet *   One();
   INamedValueSet *   Two();
   INamedValueSet * Three();
   INamedValueSet *  Four();

   INamedValueSet * Alloc();
   void         ClearList();

   std::list<INamedValueSet *> m_NVSList;
};

////////////////////////////////////////////////////////////////////////////////

template <typename V, typename Verifier>
class TTalksToNVS
{
public:
   TTalksToNVS();
   // Add single value.
   void Add(INamedValueSet *nvs, eBasicTypes expect_type, btNumberKey name, V value) const;
   void Add(INamedValueSet *nvs, eBasicTypes expect_type, btStringKey name, V value) const;
   // Verify single value.
   void Verify(const INamedValueSet *nvs, btUnsignedInt index, btNumberKey expect_name, V expect_value);
   void Verify(const INamedValueSet *nvs, btUnsignedInt index, btStringKey expect_name, V expect_value);
   // Add array of values.
   void Add(INamedValueSet    *nvs,
            eBasicTypes        expect_type,
            btNumberKey        name,
            V          * const value,
            btUnsigned32bitInt num) const;
   void Add(INamedValueSet    *nvs,
            eBasicTypes        expect_type,
            btStringKey        name,
            V          * const value,
            btUnsigned32bitInt num) const;
   // Verify array of values.
   void Verify(const INamedValueSet *nvs,
               btUnsignedInt         index,
               btNumberKey           expect_name,
               V             * const expect_values,
               btUnsigned32bitInt    num);
   void Verify(const INamedValueSet *nvs,
               btUnsignedInt         index,
               btStringKey           expect_name,
               V             * const expect_values,
               btUnsigned32bitInt    num);
};

template <typename V, typename Verifier>
TTalksToNVS<V, Verifier>::TTalksToNVS() {}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Add(INamedValueSet *nvs, eBasicTypes expect_type, btNumberKey name, V value) const
{
   btWSSize    sz = 0;
   eBasicTypes t  = btUnknownType_t;

   EXPECT_EQ(ENamedValuesOK, nvs->Add(name, value));

   EXPECT_EQ(ENamedValuesOK, nvs->GetSize(name, &sz));
   EXPECT_EQ(1, sz);

   EXPECT_EQ(ENamedValuesOK, nvs->Type(name, &t));
   EXPECT_EQ(expect_type, t);

   EXPECT_TRUE(nvs->Has(name));
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Add(INamedValueSet *nvs, eBasicTypes expect_type, btStringKey name, V value) const
{
   btWSSize    sz = 0;
   eBasicTypes t  = btUnknownType_t;

   EXPECT_EQ(ENamedValuesOK, nvs->Add(name, value));

   EXPECT_EQ(ENamedValuesOK, nvs->GetSize(name, &sz));
   EXPECT_EQ(1, sz);

   EXPECT_EQ(ENamedValuesOK, nvs->Type(name, &t));
   EXPECT_EQ(expect_type, t);

   EXPECT_TRUE(nvs->Has(name));
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Verify(const INamedValueSet *nvs, btUnsignedInt index, btNumberKey expect_name, V expect_value)
{
   const eNameTypes expect_name_type = btNumberKey_t;

   eNameTypes type = btStringKey_t;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNameType(index, &type));
   EXPECT_EQ(expect_name_type, type);

   btNumberKey name;
   EXPECT_EQ(ENamedValuesOK, nvs->GetName(index, &name));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btNumberKey) name");
      EXPECT_EQ(expect_name, name);
   }

   V value;
   EXPECT_EQ(ENamedValuesOK, nvs->Get(expect_name, &value));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btNumberKey) value");
      Verifier verifier;
      verifier.expect_eq(expect_value, value);
   }
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Verify(const INamedValueSet *nvs, btUnsignedInt index, btStringKey expect_name, V expect_value)
{
   const eNameTypes expect_name_type = btStringKey_t;

   eNameTypes type = btNumberKey_t;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNameType(index, &type));
   EXPECT_EQ(expect_name_type, type);

   btStringKey name = NULL;
   EXPECT_EQ(ENamedValuesOK, nvs->GetName(index, &name));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btStringKey) name");
      EXPECT_STREQ(expect_name, name);
   }

   V value;
   EXPECT_EQ(ENamedValuesOK, nvs->Get(expect_name, &value));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btStringKey) value");
      Verifier verifier;
      verifier.expect_eq(expect_value, value);
   }
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Add(INamedValueSet    *nvs,
                                   eBasicTypes        expect_type,
                                   btNumberKey        name,
                                   V          * const value,
                                   btUnsigned32bitInt num) const
{
   btWSSize    sz = 0;
   eBasicTypes t  = btUnknownType_t;

   EXPECT_EQ(ENamedValuesOK, nvs->Add(name, value, num));

   EXPECT_EQ(ENamedValuesOK, nvs->GetSize(name, &sz));
   EXPECT_EQ((btWSSize)num, sz);

   EXPECT_EQ(ENamedValuesOK, nvs->Type(name, &t));
   EXPECT_EQ(expect_type, t);

   EXPECT_TRUE(nvs->Has(name));
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Add(INamedValueSet    *nvs,
                                   eBasicTypes        expect_type,
                                   btStringKey        name,
                                   V          * const value,
                                   btUnsigned32bitInt num) const
{
   btWSSize    sz = 0;
   eBasicTypes t  = btUnknownType_t;

   EXPECT_EQ(ENamedValuesOK, nvs->Add(name, value, num));

   EXPECT_EQ(ENamedValuesOK, nvs->GetSize(name, &sz));
   EXPECT_EQ((btWSSize)num, sz);

   EXPECT_EQ(ENamedValuesOK, nvs->Type(name, &t));
   EXPECT_EQ(expect_type, t);

   EXPECT_TRUE(nvs->Has(name));
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Verify(const INamedValueSet *nvs,
                                      btUnsignedInt         index,
                                      btNumberKey           expect_name,
                                      V             * const expect_values,
                                      btUnsigned32bitInt    num)
{
   const eNameTypes expect_name_type = btNumberKey_t;

   eNameTypes type = btStringKey_t;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNameType(index, &type));
   EXPECT_EQ(expect_name_type, type);

   btNumberKey name;
   EXPECT_EQ(ENamedValuesOK, nvs->GetName(index, &name));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btNumberKey,array) name");
      EXPECT_EQ(expect_name, name);
   }

   V *values = NULL;
   EXPECT_EQ(ENamedValuesOK, nvs->Get(expect_name, &values));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btNumberKey,array) value");
      Verifier verifier;

      btUnsigned32bitInt i;
      for ( i = 0 ; i < num ; ++i ) {
         verifier.expect_eq(*(expect_values + i), *(values + i));
      }
   }
}

template <typename V, typename Verifier>
void TTalksToNVS<V, Verifier>::Verify(const INamedValueSet *nvs,
                                      btUnsignedInt         index,
                                      btStringKey           expect_name,
                                      V             * const expect_values,
                                      btUnsigned32bitInt    num)
{
   const eNameTypes expect_name_type = btStringKey_t;

   eNameTypes type = btNumberKey_t;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNameType(index, &type));
   EXPECT_EQ(expect_name_type, type);

   btStringKey name = NULL;
   EXPECT_EQ(ENamedValuesOK, nvs->GetName(index, &name));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btStringKey,array) name");
      EXPECT_STREQ(expect_name, name);
   }

   V *values = NULL;
   EXPECT_EQ(ENamedValuesOK, nvs->Get(expect_name, &values));

   {
      SCOPED_TRACE("TTalksToNVS::Verify(btStringKey,array) value");
      Verifier verifier;

      btUnsigned32bitInt i;
      for ( i = 0 ; i < num ; ++i ) {
         verifier.expect_eq(*(expect_values + i), *(values + i));
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

template <typename V,               // Value type, btBool, btByte, etc.
          typename Verifier,        // Value verification object. Instantiation of TIntVerifier<>, TFltVerifier<>, TStrVerifier<>.
          typename GivesValues,     // Value generator. Instantiation of TValueSequencer<>, TValueRandomizer<>.
          typename GivesNumKeys,    // btNumberKey generator. CbtNumberKeySequencer.
          typename GivesStringKeys> // btStringKey generator. CbtStringKeySequencer.
class TNVSTester : public IOStreamMixin< std::stringstream >,
#ifdef NVSFileIO
                   public FILEMixin,
#endif // NVSFileIO
                   public TTalksToNVS< V, Verifier >
{
public:
   TNVSTester();

   void    SetUp(INamedValueSet * );
   void TearDown(INamedValueSet * );

   void         AddGetbtNumberKeyTest(INamedValueSet * );
   void         AddGetbtStringKeyTest(INamedValueSet * );
   void WriteOneReadbtNumberKeyTest_A(INamedValueSet * );
   void WriteOneReadbtNumberKeyTest_B(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_A(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_B(INamedValueSet * );
   void     ChevronsbtNumberKeyTest_A(INamedValueSet * );
   void     ChevronsbtNumberKeyTest_B(INamedValueSet * );
   void     ChevronsbtStringKeyTest_A(INamedValueSet * );
   void     ChevronsbtStringKeyTest_B(INamedValueSet * );
   void    ToFromStrbtNumberKeyTest_A(INamedValueSet * );
   void    ToFromStrbtNumberKeyTest_B(INamedValueSet * );
   void    ToFromStrbtStringKeyTest_A(INamedValueSet * );
   void    ToFromStrbtStringKeyTest_B(INamedValueSet * );
   void         EqualityAndSubsetTest(INamedValueSet * );
   void           CopyConstructorTest(INamedValueSet * );

#ifdef NVSFileIO
   void WriteOneReadbtNumberKeyTest_FILE_A(INamedValueSet * );
   void WriteOneReadbtNumberKeyTest_FILE_B(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_FILE_A(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_FILE_B(INamedValueSet * );
#endif // NVSFileIO

protected:
   GivesValues     m_GivesValues;
   GivesNumKeys    m_GivesNumKeys;
   GivesStringKeys m_GivesStrKeys;
   btUnsignedInt   m_iCount;
   btUnsignedInt   m_sCount;

   // one btNumberKey
   void       iA(INamedValueSet * );
   void VerifyiA(const INamedValueSet * , btUnsignedInt );

   // all btNumberKey's
   void       iB(INamedValueSet * );
   void VerifyiB(const INamedValueSet * , btUnsignedInt );

   // one btStringKey
   void       sA(INamedValueSet * );
   void VerifysA(const INamedValueSet * , btUnsignedInt );

   // all btStringKey's
   void       sB(INamedValueSet * );
   void VerifysB(const INamedValueSet * , btUnsignedInt );
};

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::TNVSTester()
{
   m_iCount = std::min(m_GivesNumKeys.Count(), m_GivesValues.Count());
   m_sCount = std::min(m_GivesStrKeys.Count(), m_GivesValues.Count());
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::SetUp(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btWSSize      sz = 0;
   eBasicTypes   t  = btUnknownType_t;
   btUnsignedInt n  = 99;
   eNameTypes    k  = btStringKey_t;
   btNumberKey   iname;
   btStringKey   sname;

   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetName(0, &iname));
   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetName(0, &sname));

   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetNameType(0, &k));

   m_GivesNumKeys.Snapshot();
   m_GivesStrKeys.Snapshot();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      iname = m_GivesNumKeys.Value();

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Delete(iname));

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->GetSize(iname, &sz));
      ASSERT_EQ(0, sz);

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Type(iname, &t));
      ASSERT_EQ(btUnknownType_t, t);

      ASSERT_FALSE(nvs->Has(iname));
   }

   for ( i = 0 ; i < m_sCount ; ++i ) {
      sname = m_GivesStrKeys.Value();

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Delete(sname));

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->GetSize(sname, &sz));
      ASSERT_EQ(0, sz);

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Type(sname, &t));
      ASSERT_EQ(btUnknownType_t, t);

      ASSERT_FALSE(nvs->Has(sname));
   }

   m_GivesNumKeys.Replay();
   m_GivesStrKeys.Replay();
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::TearDown(INamedValueSet *nvs)
{
   m_GivesNumKeys.Replay();
   m_GivesStrKeys.Replay();

   btUnsignedInt i;
   btNumberKey   iname;
   for ( i = 0 ; i < m_iCount ; ++i ) {
      iname = m_GivesNumKeys.Value();
      nvs->Delete(iname);
   }

   btStringKey sname;
   for ( i = 0 ; i < m_sCount ; ++i ) {
      sname = m_GivesStrKeys.Value();
      nvs->Delete(sname);
   }

   btUnsignedInt n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   ClearEOF();
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::AddGetbtNumberKeyTest(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btNumberKey   name;
   btUnsignedInt n;
   V             value;

   m_GivesNumKeys.Snapshot();
   m_GivesValues.Snapshot();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      name  = m_GivesNumKeys.Value();
      value = m_GivesValues.Value();

      TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), name, value);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   m_GivesNumKeys.Replay();
   m_GivesValues.Replay();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      name  = m_GivesNumKeys.Value();
      value = m_GivesValues.Value();
      TTalksToNVS< V, Verifier >::Verify(nvs, i, name, value);
   }
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::AddGetbtStringKeyTest(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btStringKey   name;
   btUnsignedInt n;
   V             value;

   m_GivesStrKeys.Snapshot();
   m_GivesValues.Snapshot();

   for ( i = 0 ; i < m_sCount ; ++i ) {
      name  = m_GivesStrKeys.Value();
      value = m_GivesValues.Value();

      TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), name, value);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   m_GivesStrKeys.Replay();
   m_GivesValues.Replay();

   for ( i = 0 ; i < m_sCount ; ++i ) {
      name  = m_GivesStrKeys.Value();
      value = m_GivesValues.Value();
      TTalksToNVS< V, Verifier >::Verify(nvs, i, name, value);
   }
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_B(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet b;

   EXPECT_EQ(ENamedValuesOK, b.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_B(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet b;

   EXPECT_EQ(ENamedValuesOK, b.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ChevronsbtNumberKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet a;

   is() >> a;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ChevronsbtNumberKeyTest_B(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet b;

   is() >> b;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ChevronsbtStringKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet a;

   is() >> a;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ChevronsbtStringKeyTest_B(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet b;

   is() >> b;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b, 0);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ToFromStrbtNumberKeyTest_A(INamedValueSet *nvs)
{
   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet a0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , a0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a0.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet a1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a1.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet a2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a2.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)a2));
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ToFromStrbtNumberKeyTest_B(INamedValueSet *nvs)
{
   iB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet b0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , b0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b0.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet b1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b1.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet b2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b2.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)b2));
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ToFromStrbtStringKeyTest_A(INamedValueSet *nvs)
{
   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet a0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , a0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a0.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet a1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a1.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet a2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a2.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)a2));
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::ToFromStrbtStringKeyTest_B(INamedValueSet *nvs)
{
   sB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet b0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , b0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b0.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet b1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b1.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet b2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b2.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)b2));
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::EqualityAndSubsetTest(INamedValueSet *nvs)
{
   btUnsignedInt n;

   n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   // Two empty NVS's are equal and are subsets.
   {
      NamedValueSet empty;

      EXPECT_TRUE(*nvs == empty);
      EXPECT_TRUE(empty == *nvs);

      EXPECT_TRUE(nvs->Subset(empty));
      EXPECT_TRUE(empty.Subset(*nvs));
   }

   // Two NVS's with the same content are equal and are subsets.
   {
      iA(nvs);
      VerifyiA(nvs, 0);

      m_GivesNumKeys.Replay();
      m_GivesValues.Replay();

      NamedValueSet a;
      iA(&a);
      VerifyiA(&a, 0);

      EXPECT_TRUE(*nvs == a);
      EXPECT_TRUE(a == *nvs);

      EXPECT_TRUE(nvs->Subset(a));
      EXPECT_TRUE(a.Subset(*nvs));

      sB(nvs);
      VerifysB(nvs, 1); // nvs now contains iA + sB.

      EXPECT_FALSE(*nvs == a);
      EXPECT_FALSE(a == *nvs);

      // nvs is not a subset of a.
      EXPECT_FALSE(nvs->Subset(a));
      // a is a subset of nvs.
      EXPECT_TRUE(a.Subset(*nvs));


      EXPECT_EQ(ENamedValuesOK, a.Merge(*nvs));

      // We should now be equal, again.
      EXPECT_TRUE(*nvs == a);
      EXPECT_TRUE(a == *nvs);

      EXPECT_TRUE(nvs->Subset(a));
      EXPECT_TRUE(a.Subset(*nvs));
   }

   EXPECT_EQ(ENamedValuesOK, nvs->Empty());
   n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   {
      iB(nvs);
      VerifyiB(nvs, 0);

      m_GivesNumKeys.Replay();
      m_GivesValues.Replay();

      NamedValueSet b;
      iB(&b);
      VerifyiB(&b, 0);

      EXPECT_TRUE(*nvs == b);
      EXPECT_TRUE(b == *nvs);

      EXPECT_TRUE(nvs->Subset(b));
      EXPECT_TRUE(b.Subset(*nvs));

      sA(nvs);
      VerifysA(nvs, m_iCount); // nvs now contains iB + sA.

      EXPECT_FALSE(*nvs == b);
      EXPECT_FALSE(b == *nvs);

      // nvs is not a subset of b.
      EXPECT_FALSE(nvs->Subset(b));
      // b is a subset of nvs.
      EXPECT_TRUE(b.Subset(*nvs));


      EXPECT_EQ(ENamedValuesOK, b.Merge(*nvs));

      // We should now be equal, again.
      EXPECT_TRUE(*nvs == b);
      EXPECT_TRUE(b == *nvs);

      EXPECT_TRUE(nvs->Subset(b));
      EXPECT_TRUE(b.Subset(*nvs));
   }

   EXPECT_EQ(ENamedValuesOK, nvs->Empty());
   n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   // Same keys, different values.
   {
      sB(nvs);
      VerifysB(nvs, 0);

      m_GivesStrKeys.Replay();
      m_GivesValues.Replay();

      NamedValueSet c;
      sB(&c);
      VerifysB(&c, 0);

      EXPECT_TRUE(c == *nvs);
      EXPECT_TRUE(c.Subset(*nvs));

      m_GivesStrKeys.Replay();

      btStringKey k = m_GivesStrKeys.Value();

      V v;
      EXPECT_EQ(ENamedValuesOK, c.Get(k, &v));

      V w( m_GivesValues.ValueOtherThan(v) );

      EXPECT_EQ(ENamedValuesOK, c.Delete(k));

      EXPECT_EQ(ENamedValuesOK, c.Add(k, w));

      // c is the same as nvs, except that..
      //  * in nvs, 'k' maps to 'v'.
      //  * in c,   'k' maps to 'w'.

      EXPECT_FALSE(c == *nvs)      << "k=" << k << "\n" << c << "\nvs.\n\n" << *nvs;
      EXPECT_FALSE(c.Subset(*nvs)) << "k=" << k << "\n" << c << "\nvs.\n\n" << *nvs;
      EXPECT_FALSE(nvs->Subset(c)) << "k=" << k << "\n" << c << "\nvs.\n\n" << *nvs;
   }
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::CopyConstructorTest(INamedValueSet *nvs)
{
   sB(nvs);

   // NamedValueSet(const INamedValueSet & )
   NamedValueSet c0(*nvs);

   VerifysB(&c0, 0);

   // NamedValueSet(const NamedValueSet & )
   NamedValueSet c1( PassReturnByValue(c0) );

   VerifysB(&c1, 0);

   EXPECT_TRUE(c0 == c1);
   EXPECT_TRUE(*nvs == c1);
   EXPECT_TRUE(c0 == *nvs);


   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->Empty());

   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);


   iB(nvs);

   // NamedValueSet(const INamedValueSet & )
   NamedValueSet c2(*nvs);

   VerifyiB(&c2, 0);

   // NamedValueSet(const NamedValueSet & )
   NamedValueSet c3( PassReturnByValue(c2) );

   VerifyiB(&c3, 0);

   EXPECT_TRUE(c2 == c3);
   EXPECT_TRUE(*nvs == c3);
   EXPECT_TRUE(c2 == *nvs);
}

#ifdef NVSFileIO
template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_FILE_A(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);

   this->fclose(fp);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_FILE_B(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   iB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet b;

   EXPECT_EQ(ENamedValuesOK, b.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_iCount, n);

   VerifyiB(&b, 0);

   this->fclose(fp);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_FILE_A(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);

   this->fclose(fp);
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_FILE_B(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   sB(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet b;

   EXPECT_EQ(ENamedValuesOK, b.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
   EXPECT_EQ(m_sCount, n);

   VerifysB(&b, 0);

   this->fclose(fp);
}
#endif // NVSFileIO

// one btNumberKey
template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::iA(INamedValueSet *nvs)
{
   m_GivesNumKeys.Snapshot();
   m_GivesValues.Snapshot();
   TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), m_GivesNumKeys.Value(), m_GivesValues.Value());
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::VerifyiA(const INamedValueSet *nvs, btUnsignedInt index)
{
   m_GivesNumKeys.Replay();
   m_GivesValues.Replay();
   TTalksToNVS< V, Verifier >::Verify(nvs, index, m_GivesNumKeys.Value(), m_GivesValues.Value());
}

// all btNumberKey's
template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::iB(INamedValueSet *nvs)
{
   btUnsignedInt i;
   m_GivesNumKeys.Snapshot();
   m_GivesValues.Snapshot();
   for ( i = 0 ; i < m_iCount ; ++i ) {
      TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), m_GivesNumKeys.Value(), m_GivesValues.Value());
   }
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::VerifyiB(const INamedValueSet *nvs, btUnsignedInt index)
{
   btUnsignedInt i;
   m_GivesNumKeys.Replay();
   m_GivesValues.Replay();
   for ( i = 0 ; i < m_iCount ; ++i ) {
      TTalksToNVS< V, Verifier >::Verify(nvs, i + index, m_GivesNumKeys.Value(), m_GivesValues.Value());
   }
}

// one btStringKey
template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::sA(INamedValueSet *nvs)
{
   m_GivesStrKeys.Snapshot();
   m_GivesValues.Snapshot();
   TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), m_GivesStrKeys.Value(), m_GivesValues.Value());
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::VerifysA(const INamedValueSet *nvs, btUnsignedInt index)
{
   m_GivesStrKeys.Replay();
   m_GivesValues.Replay();
   TTalksToNVS< V, Verifier >::Verify(nvs, index, m_GivesStrKeys.Value(), m_GivesValues.Value());
}

// all btStringKey's
template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::sB(INamedValueSet *nvs)
{
   btUnsignedInt i;
   m_GivesStrKeys.Snapshot();
   m_GivesValues.Snapshot();
   for ( i = 0 ; i < m_sCount ; ++i ) {
      TTalksToNVS< V, Verifier >::Add(nvs, m_GivesValues.BasicType(), m_GivesStrKeys.Value(), m_GivesValues.Value());
   }
}

template <typename V,
          typename Verifier,
          typename GivesValues,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TNVSTester<V, Verifier, GivesValues, GivesNumKeys, GivesStringKeys>::VerifysB(const INamedValueSet *nvs, btUnsignedInt index)
{
   btUnsignedInt i;
   m_GivesStrKeys.Replay();
   m_GivesValues.Replay();
   for ( i = 0 ; i < m_sCount ; ++i ) {
      TTalksToNVS< V, Verifier >::Verify(nvs, i + index, m_GivesStrKeys.Value(), m_GivesValues.Value());
   }
}

////////////////////////////////////////////////////////////////////////////////

template <typename V,               // Value type, btBool, btByte, etc.
          typename Verifier,        // Value verification object. Instantiation of TIntVerifier<>, TFltVerifier<>, TStrVerifier<>.
          typename GivesArrays,     // Array Provider. Instantiation of TArrayProvider<>.
          typename GivesNumKeys,    // btNumberKey generator. CbtNumberKeySequencer.
          typename GivesStringKeys> // btStringKey generator. CbtStringKeySequencer.
class TArrayNVSTester : public IOStreamMixin< std::stringstream >,
#ifdef NVSFileIO
                        public FILEMixin,
#endif // NVSFileIO
                        public TTalksToNVS< V, Verifier >
{
public:
   TArrayNVSTester();

   void    SetUp(INamedValueSet * );
   void TearDown(INamedValueSet * );

   void         AddGetbtNumberKeyTest(INamedValueSet * );
   void         AddGetbtStringKeyTest(INamedValueSet * );
   void WriteOneReadbtNumberKeyTest_A(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_A(INamedValueSet * );
   void     ChevronsbtNumberKeyTest_A(INamedValueSet * );
   void     ChevronsbtStringKeyTest_A(INamedValueSet * );
   void    ToFromStrbtNumberKeyTest_A(INamedValueSet * );
   void    ToFromStrbtStringKeyTest_A(INamedValueSet * );
   void         EqualityAndSubsetTest(INamedValueSet * );
   void           CopyConstructorTest(INamedValueSet * );

#ifdef NVSFileIO
   void WriteOneReadbtNumberKeyTest_FILE_A(INamedValueSet * );
   void WriteOneReadbtStringKeyTest_FILE_A(INamedValueSet * );
#endif // NVSFileIO

protected:
   GivesArrays     m_GivesArrays;
   GivesNumKeys    m_GivesNumKeys;
   GivesStringKeys m_GivesStrKeys;
   btUnsignedInt   m_iCount;
   btUnsignedInt   m_sCount;

   // one Array, btNumberKey
   void       iA(INamedValueSet * );
   void VerifyiA(const INamedValueSet * , btUnsignedInt );

   // one Array, btStringKey
   void       sA(INamedValueSet * );
   void VerifysA(const INamedValueSet * , btUnsignedInt );
};

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::TArrayNVSTester()
{
   m_iCount = m_GivesNumKeys.Count();
   m_sCount = m_GivesStrKeys.Count();
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::SetUp(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btWSSize      sz = 0;
   eBasicTypes   t  = btUnknownType_t;
   btUnsignedInt n  = 99;
   eNameTypes    k  = btStringKey_t;
   btNumberKey   iname;
   btStringKey   sname;

   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetName(0, &iname));
   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetName(0, &sname));

   ASSERT_EQ(ENamedValuesIndexOutOfRange, nvs->GetNameType(0, &k));

   m_GivesNumKeys.Snapshot();
   m_GivesStrKeys.Snapshot();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      iname = m_GivesNumKeys.Value();

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Delete(iname));

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->GetSize(iname, &sz));
      ASSERT_EQ(0, sz);

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Type(iname, &t));
      ASSERT_EQ(btUnknownType_t, t);

      ASSERT_FALSE(nvs->Has(iname));
   }

   for ( i = 0 ; i < m_sCount ; ++i ) {
      sname = m_GivesStrKeys.Value();

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Delete(sname));

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->GetSize(sname, &sz));
      ASSERT_EQ(0, sz);

      ASSERT_EQ(ENamedValuesNameNotFound, nvs->Type(sname, &t));
      ASSERT_EQ(btUnknownType_t, t);

      ASSERT_FALSE(nvs->Has(sname));
   }

   m_GivesNumKeys.Replay();
   m_GivesStrKeys.Replay();
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::TearDown(INamedValueSet *nvs)
{
   m_GivesNumKeys.Replay();
   m_GivesStrKeys.Replay();

   btUnsignedInt i;
   btNumberKey   iname;
   for ( i = 0 ; i < m_iCount ; ++i ) {
      iname = m_GivesNumKeys.Value();
      nvs->Delete(iname);
   }

   btStringKey sname;
   for ( i = 0 ; i < m_sCount ; ++i ) {
      sname = m_GivesStrKeys.Value();
      nvs->Delete(sname);
   }

   btUnsignedInt n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   ClearEOF();
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::AddGetbtNumberKeyTest(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btNumberKey   name;
   btUnsignedInt n;
   V            *values;

   m_GivesNumKeys.Snapshot();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      name  = m_GivesNumKeys.Value();

      TTalksToNVS< V, Verifier >::Add(nvs,
                                      m_GivesArrays.BasicType(),
                                      name,
                                      m_GivesArrays.Array(),
                                      m_GivesArrays.Count());

      n = 99;
      EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   m_GivesNumKeys.Replay();

   for ( i = 0 ; i < m_iCount ; ++i ) {
      name  = m_GivesNumKeys.Value();

      TTalksToNVS< V, Verifier >::Verify(nvs,
                                         i,
                                         name,
                                         m_GivesArrays.Array(),
                                         m_GivesArrays.Count());
   }
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::AddGetbtStringKeyTest(INamedValueSet *nvs)
{
   btUnsignedInt i;
   btStringKey   name;
   btUnsignedInt n;
   V            *values;

   m_GivesStrKeys.Snapshot();

   for ( i = 0 ; i < m_sCount ; ++i ) {
      name  = m_GivesStrKeys.Value();

      TTalksToNVS< V, Verifier >::Add(nvs,
                                      m_GivesArrays.BasicType(),
                                      name,
                                      m_GivesArrays.Array(),
                                      m_GivesArrays.Count());

      n = 99;
      EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   m_GivesStrKeys.Replay();

   for ( i = 0 ; i < m_sCount ; ++i ) {
      name  = m_GivesStrKeys.Value();

      TTalksToNVS< V, Verifier >::Verify(nvs,
                                         i,
                                         name,
                                         m_GivesArrays.Array(),
                                         m_GivesArrays.Count());
   }
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(os(), 0));
   CheckO(O);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(is()));
   EXPECT_EQ(0, IOStreamMixin< std::stringstream >::InputBytesRemaining());
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::ChevronsbtNumberKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet a;

   is() >> a;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::ChevronsbtStringKeyTest_A(INamedValueSet *nvs)
{
   const std::ios_base::iostate O = std::ios_base::failbit|std::ios_base::badbit|std::ios_base::eofbit;
   const std::ios_base::iostate I = std::ios_base::failbit|std::ios_base::badbit;

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   os() << *nvs;
   CheckO(O);

   NamedValueSet a;

   is() >> a;
   EXPECT_TRUE(eof());

#if NVS_IO_WORKAROUND
   // fail will be on here. Turn it off so that the following check passes.
   ClearFail();
#endif // NVS_IO_WORKAROUND
   CheckI(I);

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::ToFromStrbtNumberKeyTest_A(INamedValueSet *nvs)
{
   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet a0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , a0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a0.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet a1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a1.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet a2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a2.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)a2));
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::ToFromStrbtStringKeyTest_A(INamedValueSet *nvs)
{
   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   const std::string nvsStr(nvs->ToStr());

   NamedValueSet a0;

   EXPECT_EQ(
#if NVS_IO_WORKAROUND
            ENamedValuesEndOfFile
#else
            ENamedValuesOK
#endif // NVS_IO_WORKAROUND
            , a0.FromStr( nvsStr ));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a0.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a0, 0);


   // NamedValueSet(const std::string &rstr)
   NamedValueSet a1( nvsStr );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a1.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a1, 0);


   // NamedValueSet(void *p, btWSSize sz)
   NamedValueSet a2( const_cast<char *>(nvsStr.c_str()), nvsStr.length() );

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a2.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a2, 0);

   EXPECT_EQ(0, nvsStr.compare((std::string)a2));
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::EqualityAndSubsetTest(INamedValueSet *nvs)
{
   btUnsignedInt n;

   n = 99;
   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);

   // Two NVS's with the same content are equal and are subsets.

   iA(nvs);
   VerifyiA(nvs, 0);

   m_GivesNumKeys.Replay();

   NamedValueSet a;
   iA(&a);
   VerifyiA(&a, 0);

   EXPECT_TRUE(*nvs == a);
   EXPECT_TRUE(a == *nvs);

   EXPECT_TRUE(nvs->Subset(a));
   EXPECT_TRUE(a.Subset(*nvs));

   sA(nvs);
   VerifysA(nvs, 1); // nvs now contains iA + sA.

   EXPECT_FALSE(*nvs == a);
   EXPECT_FALSE(a == *nvs);

   // nvs is not a subset of a.
   EXPECT_FALSE(nvs->Subset(a));
   // a is a subset of nvs.
   EXPECT_TRUE(a.Subset(*nvs));


   EXPECT_EQ(ENamedValuesOK, a.Merge(*nvs));

   // We should now be equal, again.
   EXPECT_TRUE(*nvs == a);
   EXPECT_TRUE(a == *nvs);

   EXPECT_TRUE(nvs->Subset(a));
   EXPECT_TRUE(a.Subset(*nvs));


   // Same keys, different content.
   m_GivesStrKeys.Replay();
   btStringKey k = m_GivesStrKeys.Value();

   V *v = NULL;
   ASSERT_EQ(ENamedValuesOK, a.Get(k, &v));
   btWSSize sz = 0;
   ASSERT_EQ(ENamedValuesOK, a.GetSize(k, &sz));

   if ( sz > 1 ) {
      V save(v[0]);
      v[0] = m_GivesArrays.ValueOtherThan(save);

      EXPECT_FALSE(a == *nvs);
      EXPECT_FALSE(a.Subset(*nvs));
      EXPECT_FALSE(nvs->Subset(a));

      v[0] = save;
   }
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::CopyConstructorTest(INamedValueSet *nvs)
{
   sA(nvs);

   // NamedValueSet(const INamedValueSet & )
   NamedValueSet c0(*nvs);

   VerifysA(&c0, 0);

   // NamedValueSet(const NamedValueSet & )
   NamedValueSet c1( PassReturnByValue(c0) );

   VerifysA(&c1, 0);

   EXPECT_TRUE(c0 == c1);
   EXPECT_TRUE(*nvs == c1);
   EXPECT_TRUE(c0 == *nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->Empty());

   ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   ASSERT_EQ(0, n);


   iA(nvs);

   // NamedValueSet(const INamedValueSet & )
   NamedValueSet c2(*nvs);

   VerifyiA(&c2, 0);

   // NamedValueSet(const NamedValueSet & )
   NamedValueSet c3( PassReturnByValue(c2) );

   VerifyiA(&c3, 0);

   EXPECT_TRUE(c2 == c3);
   EXPECT_TRUE(*nvs == c3);
   EXPECT_TRUE(c2 == *nvs);
}

#ifdef NVSFileIO
template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::WriteOneReadbtNumberKeyTest_FILE_A(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   iA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifyiA(&a, 0);

   this->fclose(fp);
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::WriteOneReadbtStringKeyTest_FILE_A(INamedValueSet *nvs)
{
   FILE *fp = fopen_tmp();
   ASSERT_NONNULL(fp);

   sA(nvs);

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
   EXPECT_EQ(1, n);

   ASSERT_EQ(ENamedValuesOK, nvs->WriteOne(fp, 0));
   EXPECT_EQ(0, this->ferror(fp));

   this->rewind(fp);

   NamedValueSet a;

   EXPECT_EQ(ENamedValuesOK, a.Read(fp));
   EXPECT_EQ(0, FILEMixin::InputBytesRemaining(fp));
   EXPECT_EQ(0, this->ferror(fp));
   EXPECT_EQ(0, this->feof(fp));

   n = 99;
   EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
   EXPECT_EQ(1, n);

   VerifysA(&a, 0);

   this->fclose(fp);
}
#endif // NVSFileIO

// one Array, btNumberKey
template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::iA(INamedValueSet *nvs)
{
   m_GivesNumKeys.Snapshot();
   TTalksToNVS< V, Verifier >::Add(nvs,
                                   m_GivesArrays.BasicType(),
                                   m_GivesNumKeys.Value(),
                                   m_GivesArrays.Array(),
                                   m_GivesArrays.Count());
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::VerifyiA(const INamedValueSet *nvs, btUnsignedInt index)
{
   m_GivesNumKeys.Replay();
   TTalksToNVS< V, Verifier >::Verify(nvs,
                                      index,
                                      m_GivesNumKeys.Value(),
                                      m_GivesArrays.Array(),
                                      m_GivesArrays.Count());
}

// one Array, btStringKey
template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::sA(INamedValueSet *nvs)
{
   m_GivesStrKeys.Snapshot();
   TTalksToNVS< V, Verifier >::Add(nvs,
                                   m_GivesArrays.BasicType(),
                                   m_GivesStrKeys.Value(),
                                   m_GivesArrays.Array(),
                                   m_GivesArrays.Count());
}

template <typename V,
          typename Verifier,
          typename GivesArrays,
          typename GivesNumKeys,
          typename GivesStringKeys>
void TArrayNVSTester<V, Verifier, GivesArrays, GivesNumKeys, GivesStringKeys>::VerifysA(const INamedValueSet *nvs, btUnsignedInt index)
{
   m_GivesStrKeys.Replay();
   TTalksToNVS< V, Verifier >::Verify(nvs,
                                      index,
                                      m_GivesStrKeys.Value(),
                                      m_GivesArrays.Array(),
                                      m_GivesArrays.Count());
}

////////////////////////////////////////////////////////////////////////////////

#define MY_TYPE_PARAMETERIZED_TEST(__fixture, __case, __mbrfn) \
TYPED_TEST_P(__fixture, __case)                                \
{                                                              \
   {                                                           \
      SCOPED_TRACE(#__case);                                   \
      TypeParam tester;                                        \
                                                               \
      tester.SetUp(&this->m_NVS);                              \
      tester.__mbrfn(&this->m_NVS);                            \
      tester.TearDown(&this->m_NVS);                           \
   }                                                           \
}

////////////////////////////////////////////////////////////////////////////////

#endif // __GTNVSTESTER_H__

