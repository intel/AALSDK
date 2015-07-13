// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "aalsdk/AALNamedValueSet.h"

#define CHECK_REMAINING_INPUT_BYTES 0

#define NVS_STREAM_WORKAROUND 1

TEST(NVS, aal0252)
{
   // TNamedValueSet::Add() returns ENamedValuesDuplicateName when the given name conflicts with a contained name.

   NamedValueSet     n;
         btStringKey j = "3";
   const btNumberKey k = 3;
   btBool            b = true;

   EXPECT_EQ(ENamedValuesOK, n.Add(j, b));
   EXPECT_EQ(ENamedValuesOK, n.Add(k, b));

   btByte by;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, by));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, by));

   bt32bitInt s32;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, s32));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, s32));

   btUnsigned32bitInt u32;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, u32));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, u32));

   bt64bitInt s64;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, s64));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, s64));

   btUnsigned64bitInt u64;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, u64));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, u64));

   btFloat f;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, f));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, f));

   btcString str = "abc";
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, str));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, str));

   btObjectType o = &u64;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, o));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, o));

   NamedValueSet m;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, &m));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, &m));


   btByteArray byA = &by;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, byA, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, byA, 1));

   bt32bitIntArray s32A = &s32;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, s32A, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, s32A, 1));

   btUnsigned32bitIntArray u32A = &u32;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, u32A, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, u32A, 1));

   bt64bitIntArray s64A = &s64;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, s64A, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, s64A, 1));

   btUnsigned64bitIntArray u64A = &u64;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, u64A, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, u64A, 1));

   btFloatArray fA = &f;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, fA, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, fA, 1));

   btStringArray strA = const_cast<btStringArray>(&str);
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, strA, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, strA, 1));

   btObjectArray oA = &o;
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, oA, 1));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, oA, 1));


   btUnsignedInt names = 0;
   EXPECT_EQ(ENamedValuesOK, n.GetNumNames(&names));
   EXPECT_EQ(2, names);

   eNameTypes ntype = btStringKey_t;
   EXPECT_EQ(ENamedValuesOK, n.GetNameType(0, &ntype));
   EXPECT_EQ(btNumberKey_t, ntype);

   EXPECT_EQ(ENamedValuesOK, n.GetNameType(1, &ntype));
   EXPECT_EQ(btStringKey_t, ntype);


   EXPECT_EQ(ENamedValuesOK, n.Delete(j));
   EXPECT_EQ(ENamedValuesOK, n.Delete(k));

   EXPECT_EQ(ENamedValuesOK, n.GetNumNames(&names));
   EXPECT_EQ(0, names);


   u32 = 5;
   EXPECT_EQ(ENamedValuesOK, n.Add(j, u32));
   EXPECT_EQ(ENamedValuesOK, n.Add(k, u32));

   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(j, b));
   EXPECT_EQ(ENamedValuesDuplicateName, n.Add(k, b));
}

TEST(NVS, aal0253)
{
   // TNamedValueSet::Get() returns ENamedValuesNameNotFound when the given name is not contained.

   NamedValueSet     n;
         btStringKey j = "3";
   const btNumberKey k = 3;
   btBool            b = true;

   EXPECT_EQ(ENamedValuesOK, n.Add((btStringKey)"2", b));
   EXPECT_EQ(ENamedValuesOK, n.Add((btNumberKey)2,   b));

   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &b));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &b));

   btByte by;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &by));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &by));

   bt32bitInt s32;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &s32));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &s32));

   btUnsigned32bitInt u32;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &u32));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &u32));

   bt64bitInt s64;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &s64));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &s64));

   btUnsigned64bitInt u64;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &u64));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &u64));

   btFloat f;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &f));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &f));

   btcString str;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &str));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &str));

   btObjectType o;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &o));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &o));

   INamedValueSet const *nvs;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &nvs));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &nvs));


   btByteArray byA;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &byA));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &byA));

   bt32bitIntArray s32A;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &s32A));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &s32A));

   btUnsigned32bitIntArray u32A;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &u32A));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &u32A));

   bt64bitIntArray s64A;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &s64A));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &s64A));

   btUnsigned64bitIntArray u64A;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &u64A));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &u64A));

   btObjectArray oA;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &oA));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &oA));

   btFloatArray fA;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &fA));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &fA));

   btStringArray strA;
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(j, &strA));
   EXPECT_EQ(ENamedValuesNameNotFound, n.Get(k, &strA));
}

TEST(NVS, aal0254)
{
   // When the NamedValueSet::Add(x, array, count) are given a count of 0,
   // ENamedValuesZeroSizedArray is returned indicating an error.

   NamedValueSet n;
   const btUnsigned32bitInt zero = 0;

   btByte by[3] = { 'a', 'b', 0 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add((btNumberKey)0, by, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("0", by, zero));

   bt32bitInt s32[3] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(1,   s32, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("1", s32, zero));

   btUnsigned32bitInt u32[3] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(2,   u32, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("2", u32, zero));

   bt64bitInt s64[3] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(3,   s64, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("3", s64, zero));

   btUnsigned64bitInt u64[3] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(4,   u64, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("4", u64, zero));

   btFloat f[3] = { 3.0, 4.0, 5.0 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(5,   f, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("5", f, zero));

   btcString str[3] = { "one", "two", "three" };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(6,   (btStringArray)str, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("6", (btStringArray)str, zero));

   btObjectType o[3] = { (btObjectType)1, (btObjectType)2, (btObjectType)3 };
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add(7,   o, zero));
   EXPECT_EQ(ENamedValuesZeroSizedArray, n.Add("7", o, zero));
}

TEST(NVS, aal0255)
{
   // When a NamedValueSet contains i btNumberKey entries and s btStringKey entries, such that
   // i > 0 and s > 0, GetName(btUnsignedInt n, btNumberKey *) returns ENamedValuesBadType when
   // i <= n < i+s. ENamedValuesIndexOutOfRange is returned when i+s <= n.

   NamedValueSet n;

   btNumberKey i = 2;
   btStringKey s = "two";

   bt32bitInt s32 = -3;

   EXPECT_EQ(ENamedValuesOK, n.Add(i, s32));
   EXPECT_EQ(ENamedValuesOK, n.Add(s, s32));

   btUnsignedInt u = 1;
   EXPECT_EQ(ENamedValuesBadType, n.GetName(u, &i));

   u = 2;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));
}

TEST(NVS, aal0256)
{
   // When a NamedValueSet contains i btNumberKey entries and s btStringKey entries, such that
   // i > 0 and s > 0, GetName(btUnsignedInt n, btStringKey *) returns ENamedValuesBadType when
   // n < i. ENamedValuesIndexOutOfRange is returned when i+s <= n.

   NamedValueSet n;

   btNumberKey i = 3;
   btStringKey s = "three";

   bt32bitInt s32 = 7;

   EXPECT_EQ(ENamedValuesOK, n.Add(i, s32));
   EXPECT_EQ(ENamedValuesOK, n.Add(s, s32));

   btUnsignedInt u = 0;
   EXPECT_EQ(ENamedValuesBadType, n.GetName(u, &s));

   u = 2;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));
}

TEST(NVS, aal0257)
{
   // When a NamedValueSet contains i btNumberKey entries and s btStringKey entries, such that
   // i > 0 and s = 0, GetName(btUnsignedInt n, btNumberKey *) returns ENamedValuesIndexOutOfRange
   // when i <= n. GetName(btUnsignedInt n, btStringKey *) returns ENamedValuesBadType when n < i
   // and ENamedValuesIndexOutOfRange when i <= n.

   NamedValueSet n;

   btNumberKey i   = 3;
   btStringKey s;
   bt32bitInt  s32 = 7;

   EXPECT_EQ(ENamedValuesOK, n.Add(i, s32));

   btUnsignedInt u = 0;

   i = 99;
   EXPECT_EQ(ENamedValuesOK, n.GetName(u, &i));
   EXPECT_EQ(3, i);

   u = 1;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));

   u = 0;
   EXPECT_EQ(ENamedValuesBadType, n.GetName(u, &s));

   u = 1;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));
}

TEST(NVS, aal0258)
{
   // When a NamedValueSet contains i btNumberKey entries and s btStringKey entries, such that
   // i = 0 and s > 0, GetName(btUnsignedInt n, btStringKey *) returns ENamedValuesIndexOutOfRange
   // when s <= n. GetName(btUnsignedInt n, btNumberKey *) returns ENamedValuesBadType when n < s
   // and ENamedValuesIndexOutOfRange when s <= n.

   NamedValueSet n;

   btNumberKey i;
   btStringKey s = "six";
   bt32bitInt  s32 = 7;

   EXPECT_EQ(ENamedValuesOK, n.Add(s, s32));

   btUnsignedInt u = 0;

   s = "seven";
   EXPECT_EQ(ENamedValuesOK, n.GetName(u, &s));
   EXPECT_STREQ("six", s);

   u = 1;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));

   u = 0;
   EXPECT_EQ(ENamedValuesBadType, n.GetName(u, &i));

   u = 1;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));
}

TEST(NVS, aal0259)
{
   // When a NamedValueSet contains no entries GetName(btUnsignedInt n, btStringKey *) returns
   // ENamedValuesIndexOutOfRange and GetName(btUnsignedInt n, btNumberKey *) returns
   // ENamedValuesIndexOutOfRange for all n.

   NamedValueSet n;

   btNumberKey i;
   btStringKey s;

   btUnsignedInt u = 0;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));

   u = 1;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));

   u = 2;
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &i));
   EXPECT_EQ(ENamedValuesIndexOutOfRange, n.GetName(u, &s));
}

////////////////////////////////////////////////////////////////////////////////

class CbtNumberKeySequencer : public TValueSequencer<btNumberKey>
{
public:
   CbtNumberKeySequencer() :
      TValueSequencer<btNumberKey>(CbtNumberKeySequencer::sm_Vals, CbtNumberKeySequencer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btUnsigned64bitInt_t; }

   static const btNumberKey        sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btNumberKey CbtNumberKeySequencer::sm_Vals[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
const btUnsigned32bitInt CbtNumberKeySequencer::sm_Count =
   sizeof(CbtNumberKeySequencer::sm_Vals) / sizeof(CbtNumberKeySequencer::sm_Vals[0]);
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
#endif // 0

class CbtStringKeySequencer : public TValueSequencer<btStringKey>
{
public:
   CbtStringKeySequencer() :
      TValueSequencer<btStringKey>(CbtStringKeySequencer::sm_Vals, CbtStringKeySequencer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btStringKey        sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btStringKey CbtStringKeySequencer::sm_Vals[] = {
   "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15",
   "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"
};
const btUnsigned32bitInt CbtStringKeySequencer::sm_Count =
   sizeof(CbtStringKeySequencer::sm_Vals) / sizeof(CbtStringKeySequencer::sm_Vals[0]);
#if 0
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
#endif // 0

class CbtBoolRandomizer : public TValueRandomizer<btBool>
{
public:
   CbtBoolRandomizer() :
      TValueRandomizer<btBool>(CbtBoolRandomizer::sm_Vals, CbtBoolRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btBool_t; }

   static const btBool             sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btBool CbtBoolRandomizer::sm_Vals[] = { false, true };
const btUnsigned32bitInt CbtBoolRandomizer::sm_Count =
   sizeof(CbtBoolRandomizer::sm_Vals) / sizeof(CbtBoolRandomizer::sm_Vals[0]);

class CbtByteRandomizer : public TValueRandomizer<btByte>
{
public:
   CbtByteRandomizer() :
      TValueRandomizer<btByte>(CbtByteRandomizer::sm_Vals, CbtByteRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btByte_t; }

   static const btByte             sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btByte CbtByteRandomizer::sm_Vals[] = {
   std::numeric_limits<btByte>::min(),
   std::numeric_limits<btByte>::min() + 1,
   0,
   std::numeric_limits<btByte>::max() - 1,
   std::numeric_limits<btByte>::max()
};
const btUnsigned32bitInt CbtByteRandomizer::sm_Count =
   sizeof(CbtByteRandomizer::sm_Vals) / sizeof(CbtByteRandomizer::sm_Vals[0]);

class Cbt32bitIntRandomizer : public TValueRandomizer<bt32bitInt>
{
public:
   Cbt32bitIntRandomizer() :
      TValueRandomizer<bt32bitInt>(Cbt32bitIntRandomizer::sm_Vals, Cbt32bitIntRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return bt32bitInt_t; }

   static const bt32bitInt         sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const bt32bitInt Cbt32bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<bt32bitInt>::min(),
   std::numeric_limits<bt32bitInt>::min() + 1,
   0,
   std::numeric_limits<bt32bitInt>::max() - 1,
   std::numeric_limits<bt32bitInt>::max()
};
const btUnsigned32bitInt Cbt32bitIntRandomizer::sm_Count =
   sizeof(Cbt32bitIntRandomizer::sm_Vals) / sizeof(Cbt32bitIntRandomizer::sm_Vals[0]);
#if 0
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
}
#endif // 0
class CbtUnsigned32bitIntRandomizer : public TValueRandomizer<btUnsigned32bitInt>
{
public:
   CbtUnsigned32bitIntRandomizer() :
      TValueRandomizer<btUnsigned32bitInt>(CbtUnsigned32bitIntRandomizer::sm_Vals, CbtUnsigned32bitIntRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btUnsigned32bitInt_t; }

   static const btUnsigned32bitInt sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btUnsigned32bitInt CbtUnsigned32bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<btUnsigned32bitInt>::min(),
   std::numeric_limits<btUnsigned32bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned32bitInt>::max() - 1,
   std::numeric_limits<btUnsigned32bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned32bitIntRandomizer::sm_Count =
   sizeof(CbtUnsigned32bitIntRandomizer::sm_Vals) / sizeof(CbtUnsigned32bitIntRandomizer::sm_Vals[0]);


class Cbt64bitIntRandomizer : public TValueRandomizer<bt64bitInt>
{
public:
   Cbt64bitIntRandomizer() :
      TValueRandomizer<bt64bitInt>(Cbt64bitIntRandomizer::sm_Vals, Cbt64bitIntRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return bt64bitInt_t; }

   static const bt64bitInt         sm_Vals[];
   static const btUnsigned64bitInt sm_Count;
};
const bt64bitInt Cbt64bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<bt64bitInt>::min(),
   std::numeric_limits<bt64bitInt>::min() + 1,
   0,
   std::numeric_limits<bt64bitInt>::max() - 1,
   std::numeric_limits<bt64bitInt>::max()
};
const btUnsigned64bitInt Cbt64bitIntRandomizer::sm_Count =
   sizeof(Cbt64bitIntRandomizer::sm_Vals) / sizeof(Cbt64bitIntRandomizer::sm_Vals[0]);

class CbtUnsigned64bitIntRandomizer : public TValueRandomizer<btUnsigned64bitInt>
{
public:
   CbtUnsigned64bitIntRandomizer() :
      TValueRandomizer<btUnsigned64bitInt>(CbtUnsigned64bitIntRandomizer::sm_Vals, CbtUnsigned64bitIntRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btUnsigned64bitInt_t; }

   static const btUnsigned64bitInt sm_Vals[];
   static const btUnsigned64bitInt sm_Count;
};
const btUnsigned64bitInt CbtUnsigned64bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<btUnsigned64bitInt>::min(),
   std::numeric_limits<btUnsigned64bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned64bitInt>::max() - 1,
   std::numeric_limits<btUnsigned64bitInt>::max()
};
const btUnsigned64bitInt CbtUnsigned64bitIntRandomizer::sm_Count =
   sizeof(CbtUnsigned64bitIntRandomizer::sm_Vals) / sizeof(CbtUnsigned64bitIntRandomizer::sm_Vals[0]);


class CbtFloatRandomizer : public TValueRandomizer<btFloat>
{
public:
   CbtFloatRandomizer() :
      TValueRandomizer<btFloat>(CbtFloatRandomizer::sm_Vals, CbtFloatRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btFloat_t; }

   static const btFloat            sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btFloat CbtFloatRandomizer::sm_Vals[] = {
   std::numeric_limits<btFloat>::min(),
   std::numeric_limits<btFloat>::min() + 1.0,
   0.0,
   std::numeric_limits<btFloat>::max() - 1.0,
   std::numeric_limits<btFloat>::max()
};
const btUnsigned32bitInt CbtFloatRandomizer::sm_Count =
   sizeof(CbtFloatRandomizer::sm_Vals) / sizeof(CbtFloatRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

class CbtcStringRandomizer : public TValueRandomizer<btcString>
{
public:
   CbtcStringRandomizer() :
      TValueRandomizer<btcString>(CbtcStringRandomizer::sm_Vals, CbtcStringRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};

#if 1
const btcString CbtcStringRandomizer::sm_Vals[] = {
   "a",
   "b",
   "c",
   "d",
   "e"
};
#else
//   NULL,
const btcString CbtcStringRandomizer::sm_Vals[] = {
   "a",
"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
};
#endif
const btUnsigned32bitInt CbtcStringRandomizer::sm_Count =
   sizeof(CbtcStringRandomizer::sm_Vals) / sizeof(CbtcStringRandomizer::sm_Vals[0]);


class CbtcStringRandomizer_WithZeroLengthStrings : public TValueRandomizer<btcString>
{
public:
   CbtcStringRandomizer_WithZeroLengthStrings() :
      TValueRandomizer<btcString>(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals,
                                  CbtcStringRandomizer_WithZeroLengthStrings::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btcString CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals[] = {
   "",
   "not",
   "",
   "good",
   ""
};
const btUnsigned32bitInt CbtcStringRandomizer_WithZeroLengthStrings::sm_Count =
   sizeof(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals) / sizeof(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals[0]);



class CbtcStringRandomizer_WithLongStrings : public TValueRandomizer<btcString>
{
public:
   CbtcStringRandomizer_WithLongStrings() :
      TValueRandomizer<btcString>(CbtcStringRandomizer_WithLongStrings::sm_Vals,
                                  CbtcStringRandomizer_WithLongStrings::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btString_t; }

   static const btcString          sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btcString CbtcStringRandomizer_WithLongStrings::sm_Vals[] = {
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", // 128 bytes

   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", // 192 bytes

   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd",  // 254 bytes

   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde", // 255 bytes

   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" // 256 bytes
};
const btUnsigned32bitInt CbtcStringRandomizer_WithLongStrings::sm_Count =
   sizeof(CbtcStringRandomizer_WithLongStrings::sm_Vals) / sizeof(CbtcStringRandomizer_WithLongStrings::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

class CbtObjectTypeRandomizer : public TValueRandomizer<btObjectType>
{
public:
   CbtObjectTypeRandomizer() :
      TValueRandomizer<btObjectType>(CbtObjectTypeRandomizer::sm_Vals, CbtObjectTypeRandomizer::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btObjectType_t; }

   static const btObjectType       sm_Vals[];
   static const btUnsigned32bitInt sm_Count;
};
const btObjectType CbtObjectTypeRandomizer::sm_Vals[] = {
   (btObjectType)NULL,
   (btObjectType)1,
   (btObjectType)2,
   (btObjectType)3,
   (btObjectType)4,
};
const btUnsigned32bitInt CbtObjectTypeRandomizer::sm_Count =
   sizeof(CbtObjectTypeRandomizer::sm_Vals) / sizeof(CbtObjectTypeRandomizer::sm_Vals[0]);




class CbtByteArrayProvider : public TArrayProvider<btByteArray>
{
public:
   CbtByteArrayProvider() :
      TArrayProvider<btByteArray>(const_cast<const btByteArray>(CbtByteArrayProvider::sm_Array), CbtByteArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btByteArray_t; }

   static const btByte             sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btByte CbtByteArrayProvider::sm_Array[] = {
   std::numeric_limits<btByte>::min(),
   std::numeric_limits<btByte>::min() + 1,
   0,
   std::numeric_limits<btByte>::max() - 1,
   std::numeric_limits<btByte>::max()
};
const btUnsigned32bitInt CbtByteArrayProvider::sm_Count =
   sizeof(CbtByteArrayProvider::sm_Array) / sizeof(CbtByteArrayProvider::sm_Array[0]);


class Cbt32bitIntArrayProvider : public TArrayProvider<bt32bitIntArray>
{
public:
   Cbt32bitIntArrayProvider() :
      TArrayProvider<bt32bitIntArray>(const_cast<const bt32bitIntArray>(Cbt32bitIntArrayProvider::sm_Array), Cbt32bitIntArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return bt32bitIntArray_t; }

   static const bt32bitInt         sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const bt32bitInt Cbt32bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<bt32bitInt>::min(),
   std::numeric_limits<bt32bitInt>::min() + 1,
   0,
   std::numeric_limits<bt32bitInt>::max() - 1,
   std::numeric_limits<bt32bitInt>::max()
};
const btUnsigned32bitInt Cbt32bitIntArrayProvider::sm_Count =
   sizeof(Cbt32bitIntArrayProvider::sm_Array) / sizeof(Cbt32bitIntArrayProvider::sm_Array[0]);


class CbtUnsigned32bitIntArrayProvider : public TArrayProvider<btUnsigned32bitIntArray>
{
public:
   CbtUnsigned32bitIntArrayProvider() :
      TArrayProvider<btUnsigned32bitIntArray>(const_cast<const btUnsigned32bitIntArray>(CbtUnsigned32bitIntArrayProvider::sm_Array), CbtUnsigned32bitIntArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btUnsigned32bitIntArray_t; }

   static const btUnsigned32bitInt sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btUnsigned32bitInt CbtUnsigned32bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<btUnsigned32bitInt>::min(),
   std::numeric_limits<btUnsigned32bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned32bitInt>::max() - 1,
   std::numeric_limits<btUnsigned32bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned32bitIntArrayProvider::sm_Count =
   sizeof(CbtUnsigned32bitIntArrayProvider::sm_Array) / sizeof(CbtUnsigned32bitIntArrayProvider::sm_Array[0]);


class Cbt64bitIntArrayProvider : public TArrayProvider<bt64bitIntArray>
{
public:
   Cbt64bitIntArrayProvider() :
      TArrayProvider<bt64bitIntArray>(const_cast<const bt64bitIntArray>(Cbt64bitIntArrayProvider::sm_Array), Cbt64bitIntArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return bt64bitIntArray_t; }

   static const bt64bitInt         sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const bt64bitInt Cbt64bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<bt64bitInt>::min(),
   std::numeric_limits<bt64bitInt>::min() + 1,
   0,
   std::numeric_limits<bt64bitInt>::max() - 1,
   std::numeric_limits<bt64bitInt>::max()
};
const btUnsigned32bitInt Cbt64bitIntArrayProvider::sm_Count =
   sizeof(Cbt64bitIntArrayProvider::sm_Array) / sizeof(Cbt64bitIntArrayProvider::sm_Array[0]);


class CbtUnsigned64bitIntArrayProvider : public TArrayProvider<btUnsigned64bitIntArray>
{
public:
   CbtUnsigned64bitIntArrayProvider() :
      TArrayProvider<btUnsigned64bitIntArray>(const_cast<const btUnsigned64bitIntArray>(CbtUnsigned64bitIntArrayProvider::sm_Array), CbtUnsigned64bitIntArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btUnsigned64bitIntArray_t; }

   static const btUnsigned64bitInt sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btUnsigned64bitInt CbtUnsigned64bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<btUnsigned64bitInt>::min(),
   std::numeric_limits<btUnsigned64bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned64bitInt>::max() - 1,
   std::numeric_limits<btUnsigned64bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned64bitIntArrayProvider::sm_Count =
   sizeof(CbtUnsigned64bitIntArrayProvider::sm_Array) / sizeof(CbtUnsigned64bitIntArrayProvider::sm_Array[0]);


class CbtFloatArrayProvider : public TArrayProvider<btFloatArray>
{
public:
   CbtFloatArrayProvider() :
      TArrayProvider<btFloatArray>(const_cast<const btFloatArray>(CbtFloatArrayProvider::sm_Array), CbtFloatArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btFloatArray_t; }

   static const btFloat            sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btFloat CbtFloatArrayProvider::sm_Array[] = {
   std::numeric_limits<btFloat>::min(),
   std::numeric_limits<btFloat>::min() + 1.0,
   0.0,
   std::numeric_limits<btFloat>::max() - 1.0,
   std::numeric_limits<btFloat>::max()
};
const btUnsigned32bitInt CbtFloatArrayProvider::sm_Count =
   sizeof(CbtFloatArrayProvider::sm_Array) / sizeof(CbtFloatArrayProvider::sm_Array[0]);





class CbtStringArrayProvider : public TArrayProvider<btStringArray>
{
public:
   CbtStringArrayProvider() :
      TArrayProvider<btStringArray>(const_cast<const btStringArray>(CbtStringArrayProvider::sm_Array), CbtStringArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btString CbtStringArrayProvider::sm_Array[] = {
   const_cast<const btString>("a"),
   const_cast<const btString>("b"),
   const_cast<const btString>("c"),
   const_cast<const btString>("d"),
   const_cast<const btString>("e")
};
const btUnsigned32bitInt CbtStringArrayProvider::sm_Count =
   sizeof(CbtStringArrayProvider::sm_Array) / sizeof(CbtStringArrayProvider::sm_Array[0]);


class CbtStringArrayProvider_WithZeroLengthStrings : public TArrayProvider<btStringArray>
{
public:
   CbtStringArrayProvider_WithZeroLengthStrings() :
      TArrayProvider<btStringArray>(const_cast<const btStringArray>(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array),
                                    CbtStringArrayProvider_WithZeroLengthStrings::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btString CbtStringArrayProvider_WithZeroLengthStrings::sm_Array[] = {
   const_cast<const btString>(""),
   const_cast<const btString>("break"),
   const_cast<const btString>(""),
   const_cast<const btString>("stuff"),
   const_cast<const btString>("")
};
const btUnsigned32bitInt CbtStringArrayProvider_WithZeroLengthStrings::sm_Count =
   sizeof(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array) / sizeof(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array[0]);


class CbtStringArrayProvider_WithLongStrings : public TArrayProvider<btStringArray>
{
public:
   CbtStringArrayProvider_WithLongStrings() :
      TArrayProvider<btStringArray>(const_cast<const btStringArray>(CbtStringArrayProvider_WithLongStrings::sm_Array),
                                    CbtStringArrayProvider_WithLongStrings::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btStringArray_t; }

   static const btString           sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btString CbtStringArrayProvider_WithLongStrings::sm_Array[] = {

   const_cast<const btString>("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"),  // 254 bytes

   const_cast<const btString>("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"), // 192 bytes

   const_cast<const btString>("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"), // 256 bytes

   const_cast<const btString>("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"), // 128 bytes

   const_cast<const btString>("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" \
   "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde")  // 255 bytes

};
const btUnsigned32bitInt CbtStringArrayProvider_WithLongStrings::sm_Count =
   sizeof(CbtStringArrayProvider_WithLongStrings::sm_Array) / sizeof(CbtStringArrayProvider_WithLongStrings::sm_Array[0]);



class CbtObjectArrayProvider : public TArrayProvider<btObjectArray>
{
public:
   CbtObjectArrayProvider() :
      TArrayProvider<btObjectArray>(const_cast<const btObjectArray>(CbtObjectArrayProvider::sm_Array), CbtObjectArrayProvider::sm_Count)
   {}

   virtual eBasicTypes BasicType() const { return btObjectArray_t; }

   static const btObjectType       sm_Array[];
   static const btUnsigned32bitInt sm_Count;
};
const btObjectType CbtObjectArrayProvider::sm_Array[] = {
   (btObjectType)NULL,
   (btObjectType)1,
   (btObjectType)2,
   (btObjectType)3,
   (btObjectType)4,
};
const btUnsigned32bitInt CbtObjectArrayProvider::sm_Count =
   sizeof(CbtObjectArrayProvider::sm_Array) / sizeof(CbtObjectArrayProvider::sm_Array[0]);



template <typename V, typename VCmp>
class TTalksToNVS
{
public:
   TTalksToNVS() {}

   void Add(INamedValueSet *nvs, eBasicTypes expect_type, btNumberKey name, V value) const
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
   void Add(INamedValueSet *nvs, eBasicTypes expect_type, btStringKey name, V value) const
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

   void Verify(const INamedValueSet *nvs, btUnsignedInt index, btNumberKey expect_name, V expect_value)
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
         VCmp vcompare;
         vcompare.expect_eq(expect_value, value);
      }
   }
   void Verify(const INamedValueSet *nvs, btUnsignedInt index, btStringKey expect_name, V expect_value)
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
         VCmp vcompare;
         vcompare.expect_eq(expect_value, value);
      }
   }

   void Add(INamedValueSet    *nvs,
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
   void Add(INamedValueSet    *nvs,
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

   void Verify(const INamedValueSet *nvs,
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
         VCmp vcompare;

         btUnsigned32bitInt i;
         for ( i = 0 ; i < num ; ++i ) {
            vcompare.expect_eq(*(expect_values + i), *(values + i));
         }
      }
   }
   void Verify(const INamedValueSet *nvs,
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
         VCmp vcompare;

         btUnsigned32bitInt i;
         for ( i = 0 ; i < num ; ++i ) {
            vcompare.expect_eq(*(expect_values + i), *(values + i));
         }
      }
   }
};

////////////////////////////////////////////////////////////////////////////////

template <typename    V,               // Value type, btBool, btByte, etc.
          typename    VCmp,            // Value comparison object. Instantiation of TIntCmp<>, TFltCmp<>, TStrCmp<>.
          typename    GivesValues,     // Value generator. Instantiation of TValueSequencer<>, TValueRandomizer<>.
          typename    GivesNumKeys,    // btNumberKey generator. CbtNumberKeySequencer.
          typename    GivesStringKeys> // btStringKey generator. CbtStringKeySequencer.
class TNVSTester : public IOStreamMixin< std::stringstream >,
                   public TTalksToNVS< V, VCmp >
{
public:
   TNVSTester()
   {
      m_iCount = std::min(m_GivesNumKeys.Count(), m_GivesValues.Count());
      m_sCount = std::min(m_GivesStrKeys.Count(), m_GivesValues.Count());
   }

   void SetUp(INamedValueSet *nvs)
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

   void TearDown(INamedValueSet *nvs)
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

   void AddGetbtNumberKeyTest(INamedValueSet *nvs)
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

         TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), name, value);

         n = 99;
         EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
         EXPECT_EQ(n, i+1);
      }

      m_GivesNumKeys.Replay();
      m_GivesValues.Replay();

      for ( i = 0 ; i < m_iCount ; ++i ) {
         name  = m_GivesNumKeys.Value();
         value = m_GivesValues.Value();
         TTalksToNVS< V, VCmp >::Verify(nvs, i, name, value);
      }
   }
   void AddGetbtStringKeyTest(INamedValueSet *nvs)
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

         TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), name, value);

         n = 99;
         EXPECT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
         EXPECT_EQ(n, i+1);
      }

      m_GivesStrKeys.Replay();
      m_GivesValues.Replay();

      for ( i = 0 ; i < m_sCount ; ++i ) {
         name  = m_GivesStrKeys.Value();
         value = m_GivesValues.Value();
         TTalksToNVS< V, VCmp >::Verify(nvs, i, name, value);
      }
   }

   void WriteOneReadbtNumberKeyTest_A(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifyiA(&a, 0);
   }
   void WriteOneReadbtNumberKeyTest_B(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
      EXPECT_EQ(m_iCount, n);

      VerifyiB(&b, 0);
   }

   void WriteOneReadbtStringKeyTest_A(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifysA(&a, 0);
   }
   void WriteOneReadbtStringKeyTest_B(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
      EXPECT_EQ(m_sCount, n);

      VerifysB(&b, 0);
   }

   void ChevronsbtNumberKeyTest_A(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifyiA(&a, 0);
   }
   void ChevronsbtNumberKeyTest_B(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
      EXPECT_EQ(m_iCount, n);

      VerifyiB(&b, 0);
   }

   void ChevronsbtStringKeyTest_A(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifysA(&a, 0);
   }
   void ChevronsbtStringKeyTest_B(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, b.GetNumNames(&n));
      EXPECT_EQ(m_sCount, n);

      VerifysB(&b, 0);
   }

protected:
   GivesValues     m_GivesValues;
   GivesNumKeys    m_GivesNumKeys;
   GivesStringKeys m_GivesStrKeys;
   btUnsignedInt   m_iCount;
   btUnsignedInt   m_sCount;

   // one btNumberKey
   void iA(INamedValueSet *nvs)
   {
      m_GivesNumKeys.Snapshot();
      m_GivesValues.Snapshot();
      TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), m_GivesNumKeys.Value(), m_GivesValues.Value());
   }
   void VerifyiA(const INamedValueSet *nvs, btUnsignedInt index)
   {
      m_GivesNumKeys.Replay();
      m_GivesValues.Replay();
      TTalksToNVS< V, VCmp >::Verify(nvs, index, m_GivesNumKeys.Value(), m_GivesValues.Value());
   }

   // all btNumberKey's
   void iB(INamedValueSet *nvs)
   {
      btUnsignedInt i;
      m_GivesNumKeys.Snapshot();
      m_GivesValues.Snapshot();
      for ( i = 0 ; i < m_iCount ; ++i ) {
         TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), m_GivesNumKeys.Value(), m_GivesValues.Value());
      }
   }
   void VerifyiB(const INamedValueSet *nvs, btUnsignedInt index)
   {
      btUnsignedInt i;
      m_GivesNumKeys.Replay();
      m_GivesValues.Replay();
      for ( i = 0 ; i < m_iCount ; ++i ) {
         TTalksToNVS< V, VCmp >::Verify(nvs, i + index, m_GivesNumKeys.Value(), m_GivesValues.Value());
      }
   }

   // one btStringKey
   void sA(INamedValueSet *nvs)
   {
      m_GivesStrKeys.Snapshot();
      m_GivesValues.Snapshot();
      TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), m_GivesStrKeys.Value(), m_GivesValues.Value());
   }
   void VerifysA(const INamedValueSet *nvs, btUnsignedInt index)
   {
      m_GivesStrKeys.Replay();
      m_GivesValues.Replay();
      TTalksToNVS< V, VCmp >::Verify(nvs, index, m_GivesStrKeys.Value(), m_GivesValues.Value());
   }

   // all btStringKey's
   void sB(INamedValueSet *nvs)
   {
      btUnsignedInt i;
      m_GivesStrKeys.Snapshot();
      m_GivesValues.Snapshot();
      for ( i = 0 ; i < m_sCount ; ++i ) {
         TTalksToNVS< V, VCmp >::Add(nvs, m_GivesValues.BasicType(), m_GivesStrKeys.Value(), m_GivesValues.Value());
      }
   }
   void VerifysB(const INamedValueSet *nvs, btUnsignedInt index)
   {
      btUnsignedInt i;
      m_GivesStrKeys.Replay();
      m_GivesValues.Replay();
      for ( i = 0 ; i < m_sCount ; ++i ) {
         TTalksToNVS< V, VCmp >::Verify(nvs, i + index, m_GivesStrKeys.Value(), m_GivesValues.Value());
      }
   }
};

template <typename    V,               // Value type, btBool, btByte, etc.
          typename    VCmp,            // Value comparison object. Instantiation of TIntCmp<>, TFltCmp<>, TStrCmp<>.
          typename    GivesArrays,     // Array Provider. Instantiation of TArrayProvider<>.
          typename    GivesNumKeys,    // btNumberKey generator. CbtNumberKeySequencer.
          typename    GivesStringKeys> // btStringKey generator. CbtStringKeySequencer.
class TArrayNVSTester : public IOStreamMixin< std::stringstream >,
                        public TTalksToNVS< V, VCmp >
{
public:
   TArrayNVSTester()
   {
      m_iCount = m_GivesNumKeys.Count();
      m_sCount = m_GivesStrKeys.Count();
   }

   void SetUp(INamedValueSet *nvs)
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

   void TearDown(INamedValueSet *nvs)
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

   void AddGetbtNumberKeyTest(INamedValueSet *nvs)
   {
      btUnsignedInt i;
      btNumberKey   name;
      btUnsignedInt n;
      V            *values;

      m_GivesNumKeys.Snapshot();

      for ( i = 0 ; i < m_iCount ; ++i ) {
         name  = m_GivesNumKeys.Value();

         TTalksToNVS< V, VCmp >::Add(nvs,
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

         TTalksToNVS< V, VCmp >::Verify(nvs,
                                        i,
                                        name,
                                        m_GivesArrays.Array(),
                                        m_GivesArrays.Count());
      }
   }
   void AddGetbtStringKeyTest(INamedValueSet *nvs)
   {
      btUnsignedInt i;
      btStringKey   name;
      btUnsignedInt n;
      V            *values;

      m_GivesStrKeys.Snapshot();

      for ( i = 0 ; i < m_sCount ; ++i ) {
         name  = m_GivesStrKeys.Value();

         TTalksToNVS< V, VCmp >::Add(nvs,
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

         TTalksToNVS< V, VCmp >::Verify(nvs,
                                        i,
                                        name,
                                        m_GivesArrays.Array(),
                                        m_GivesArrays.Count());
      }
   }

   void WriteOneReadbtNumberKeyTest_A(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifyiA(&a, 0);
   }
   void WriteOneReadbtStringKeyTest_A(INamedValueSet *nvs)
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

#if CHECK_REMAINING_INPUT_BYTES
      EXPECT_EQ(0, InputBytesRemaining());
#endif // CHECK_REMAINING_INPUT_BYTES

      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifysA(&a, 0);
   }

   void ChevronsbtNumberKeyTest_A(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifyiA(&a, 0);
   }
   void ChevronsbtStringKeyTest_A(INamedValueSet *nvs)
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

#if NVS_STREAM_WORKAROUND
      // fail will be on here. Turn it off so that the following check passes.
      ClearFail();
#endif // NVS_STREAM_WORKAROUND
      CheckI(I);

      n = 99;
      EXPECT_EQ(ENamedValuesOK, a.GetNumNames(&n));
      EXPECT_EQ(1, n);

      VerifysA(&a, 0);
   }

protected:
   GivesArrays     m_GivesArrays;
   GivesNumKeys    m_GivesNumKeys;
   GivesStringKeys m_GivesStrKeys;
   btUnsignedInt   m_iCount;
   btUnsignedInt   m_sCount;

   // one Array, btNumberKey
   void iA(INamedValueSet *nvs)
   {
      m_GivesNumKeys.Snapshot();
      TTalksToNVS< V, VCmp >::Add(nvs,
                                  m_GivesArrays.BasicType(),
                                  m_GivesNumKeys.Value(),
                                  m_GivesArrays.Array(),
                                  m_GivesArrays.Count());
   }
   void VerifyiA(const INamedValueSet *nvs, btUnsignedInt index)
   {
      m_GivesNumKeys.Replay();
      TTalksToNVS< V, VCmp >::Verify(nvs,
                                     index,
                                     m_GivesNumKeys.Value(),
                                     m_GivesArrays.Array(),
                                     m_GivesArrays.Count());
   }

   // one Array, btStringKey
   void sA(INamedValueSet *nvs)
   {
      m_GivesStrKeys.Snapshot();
      TTalksToNVS< V, VCmp >::Add(nvs,
                                  m_GivesArrays.BasicType(),
                                  m_GivesStrKeys.Value(),
                                  m_GivesArrays.Array(),
                                  m_GivesArrays.Count());
   }
   void VerifysA(const INamedValueSet *nvs, btUnsignedInt index)
   {
      m_GivesStrKeys.Replay();
      TTalksToNVS< V, VCmp >::Verify(nvs,
                                     index,
                                     m_GivesStrKeys.Value(),
                                     m_GivesArrays.Array(),
                                     m_GivesArrays.Count());
   }
};

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

typedef TNVSTester<btBool,
                   TIntCmp<btBool>,
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
                           aal0269);

typedef ::testing::Types< btBoolNVSTester > NamedValueSet_btBool_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btBool_tp_0, NamedValueSet_btBool_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btByte,
                   TIntCmp<btByte>,
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
                           aal0279);

typedef ::testing::Types< btByteNVSTester > NamedValueSet_btByte_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btByte_tp_0, NamedValueSet_btByte_tp_0_Types);


typedef TArrayNVSTester<btByte,
                        TIntCmp<btByte>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btByteArray_tp_0,
                           aal0350,
                           aal0351,
                           aal0352,
                           aal0353,
                           aal0354,
                           aal0355);

typedef ::testing::Types< btByteArrayNVSTester > NamedValueSet_btByteArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btByteArray_tp_0, NamedValueSet_btByteArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<bt32bitInt,
                   TIntCmp<bt32bitInt>,
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
                           aal0289);

typedef ::testing::Types< bt32bitIntNVSTester > NamedValueSet_bt32bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt32bitInt_tp_0, NamedValueSet_bt32bitInt_tp_0_Types);


typedef TArrayNVSTester<bt32bitInt,
                        TIntCmp<bt32bitInt>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt32bitIntArray_tp_0,
                           aal0356,
                           aal0357,
                           aal0358,
                           aal0359,
                           aal0360,
                           aal0361);

typedef ::testing::Types< bt32bitIntArrayNVSTester > NamedValueSet_bt32bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt32bitIntArray_tp_0, NamedValueSet_bt32bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btUnsigned32bitInt,
                   TIntCmp<btUnsigned32bitInt>,
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
                           aal0299);

typedef ::testing::Types< btUnsigned32bitIntNVSTester > NamedValueSet_btUnsigned32bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned32bitInt_tp_0, NamedValueSet_btUnsigned32bitInt_tp_0_Types);


typedef TArrayNVSTester<btUnsigned32bitInt,
                        TIntCmp<btUnsigned32bitInt>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned32bitIntArray_tp_0,
                           aal0362,
                           aal0363,
                           aal0364,
                           aal0365,
                           aal0366,
                           aal0367);

typedef ::testing::Types< btUnsigned32bitIntArrayNVSTester > NamedValueSet_btUnsigned32bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned32bitIntArray_tp_0, NamedValueSet_btUnsigned32bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<bt64bitInt,
                   TIntCmp<bt64bitInt>,
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
                           aal0309);

typedef ::testing::Types< bt64bitIntNVSTester > NamedValueSet_bt64bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt64bitInt_tp_0, NamedValueSet_bt64bitInt_tp_0_Types);


typedef TArrayNVSTester<bt64bitInt,
                        TIntCmp<bt64bitInt>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_bt64bitIntArray_tp_0,
                           aal0368,
                           aal0369,
                           aal0370,
                           aal0371,
                           aal0372,
                           aal0373);

typedef ::testing::Types< bt64bitIntArrayNVSTester > NamedValueSet_bt64bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_bt64bitIntArray_tp_0, NamedValueSet_bt64bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btUnsigned64bitInt,
                   TIntCmp<btUnsigned64bitInt>,
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
                           aal0319);

typedef ::testing::Types< btUnsigned64bitIntNVSTester > NamedValueSet_btUnsigned64bitInt_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned64bitInt_tp_0, NamedValueSet_btUnsigned64bitInt_tp_0_Types);


typedef TArrayNVSTester<btUnsigned64bitInt,
                        TIntCmp<btUnsigned64bitInt>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btUnsigned64bitIntArray_tp_0,
                           aal0374,
                           aal0375,
                           aal0376,
                           aal0377,
                           aal0378,
                           aal0379);

typedef ::testing::Types< btUnsigned64bitIntArrayNVSTester > NamedValueSet_btUnsigned64bitIntArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btUnsigned64bitIntArray_tp_0, NamedValueSet_btUnsigned64bitIntArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btFloat,
                   TFltCmp<btFloat>,
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
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0322, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0323, WriteOneReadbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0324, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0325, WriteOneReadbtStringKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0326, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0327, ChevronsbtNumberKeyTest_B)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0328, ChevronsbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloat_tp_0, DISABLED_aal0329, ChevronsbtStringKeyTest_B)

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btFloat_tp_0,
                           aal0320,
                           aal0321,
                           DISABLED_aal0322,
                           DISABLED_aal0323,
                           DISABLED_aal0324,
                           DISABLED_aal0325,
                           DISABLED_aal0326,
                           DISABLED_aal0327,
                           DISABLED_aal0328,
                           DISABLED_aal0329);

typedef ::testing::Types< btFloatNVSTester > NamedValueSet_btFloat_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btFloat_tp_0, NamedValueSet_btFloat_tp_0_Types);


typedef TArrayNVSTester<btFloat,
                        TFltCmp<btFloat>,
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
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, DISABLED_aal0382, WriteOneReadbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, DISABLED_aal0383, WriteOneReadbtStringKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, DISABLED_aal0384, ChevronsbtNumberKeyTest_A)
MY_TYPE_PARAMETERIZED_TEST(NamedValueSet_btFloatArray_tp_0, DISABLED_aal0385, ChevronsbtStringKeyTest_A)

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btFloatArray_tp_0,
                           aal0380,
                           aal0381,
                           DISABLED_aal0382,
                           DISABLED_aal0383,
                           DISABLED_aal0384,
                           DISABLED_aal0385);

typedef ::testing::Types< btFloatArrayNVSTester > NamedValueSet_btFloatArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btFloatArray_tp_0, NamedValueSet_btFloatArray_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btcString,
                   TStrCmp<btcString>,
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
                           aal0339);

typedef ::testing::Types< btcStringNVSTester > NamedValueSet_btcString_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_0, NamedValueSet_btcString_tp_0_Types);

///////////////////////
// zero-length strings

typedef TNVSTester<btcString,
                   TStrCmp<btcString>,
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
                           aal0407);

typedef ::testing::Types< btcStringNVSTester_ZeroLengthStrings > NamedValueSet_btcString_tp_1_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_1, NamedValueSet_btcString_tp_1_Types);

///////////////////////
// very long strings

typedef TNVSTester<btcString,
                   TStrCmp<btcString>,
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
                           aal0423);

typedef ::testing::Types< btcStringNVSTester_LongStrings > NamedValueSet_btcString_tp_2_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btcString_tp_2, NamedValueSet_btcString_tp_2_Types);




typedef TArrayNVSTester<btString,
                        TStrCmp<btString>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_0,
                           aal0386,
                           aal0387,
                           aal0388,
                           aal0389,
                           aal0390,
                           aal0391);

typedef ::testing::Types< btStringArrayNVSTester > NamedValueSet_btStringArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_0, NamedValueSet_btStringArray_tp_0_Types);

///////////////////////////////
// zero-length strings in array

typedef TArrayNVSTester<btString,
                        TStrCmp<btString>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_1,
                           aal0408,
                           aal0409,
                           aal0410,
                           aal0411,
                           aal0412,
                           aal0413);

typedef ::testing::Types< btStringArrayNVSTester_ZeroLengthStrings > NamedValueSet_btStringArray_tp_1_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_1, NamedValueSet_btStringArray_tp_1_Types);

/////////////////////////////
// very long strings in array

typedef TArrayNVSTester<btString,
                        TStrCmp<btString>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btStringArray_tp_2,
                           aal0424,
                           aal0425,
                           aal0426,
                           aal0427,
                           aal0428,
                           aal0429);

typedef ::testing::Types< btStringArrayNVSTester_LongStrings > NamedValueSet_btStringArray_tp_2_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btStringArray_tp_2, NamedValueSet_btStringArray_tp_2_Types);

////////////////////////////////////////////////////////////////////////////////

typedef TNVSTester<btObjectType,
                   TIntCmp<btObjectType>,
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
                           aal0349);

typedef ::testing::Types< btObjectTypeNVSTester > NamedValueSet_btObjectType_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btObjectType_tp_0, NamedValueSet_btObjectType_tp_0_Types);


typedef TArrayNVSTester<btObjectType,
                        TIntCmp<btObjectType>,
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

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_btObjectArray_tp_0,
                           aal0392,
                           aal0393,
                           aal0394,
                           aal0395,
                           aal0396,
                           aal0397);

typedef ::testing::Types< btObjectArrayNVSTester > NamedValueSet_btObjectArray_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_btObjectArray_tp_0, NamedValueSet_btObjectArray_tp_0_Types);

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

