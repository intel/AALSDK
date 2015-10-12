// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "aalsdk/AALNamedValueSet.h"

#if 0
TEST(NVS, Redmine529)
{
   NamedValueSet n;

   n.Add("AAL_keyRegAFU_ID",  "00000000-0000-0000-0000-000011100181");
   n.Add("AIAExecutable",     "libAASUAIA");
   n.Add("ServiceExecutable", "libHWSPLAFU");

   std::cout << "begin [" << n << "] end";
}
#endif // Redmine529

#if 0
TEST(NVS, PrintFloat)
{
   NamedValueSet nvs;

   const btFloat pi = 3.14159;

   nvs.Add(3,    pi);
   nvs.Add("pi", pi);

   btFloat A[] = { 0.0, 1.0, 2.0 };
   nvs.Add(4,       A, 3);
   nvs.Add("array", A, 3);

   nvs.WriteOne(std::cout, 0);
}
TEST(NVS, PrintFloat_FILE)
{
   NamedValueSet nvs;

   const btFloat pi = 3.14159;

   nvs.Add(3,    pi);
   nvs.Add("pi", pi);

   btFloat A[] = { 0.0, 1.0, 2.0 };
   nvs.Add(4,       A, 3);
   nvs.Add("array", A, 3);

   nvs.WriteOne(stdout, 0);
}
#endif // 0

TEST(NVS, BufFromString)
{
   char buf[4] = { 1, 2, 3, 4 };

   std::string str("abc");

   BufFromString(buf, str);
   EXPECT_EQ('a', buf[0]);
   EXPECT_EQ('b', buf[1]);
   EXPECT_EQ('c', buf[2]);
   EXPECT_EQ(4,   buf[3]); // BufFromString does not NULL-terminate the string.
}

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

TEST(NVS, aal0530)
{
   // NamedValueSet::operator = () implements a safety check preventing self-assign.

   NamedValueSet nvs;
   NamedValueSet *p = &nvs;

   nvs.Add("str", "abc");

   nvs = *p;

   btUnsignedInt n = 99;
   EXPECT_EQ(ENamedValuesOK, nvs.GetNumNames(&n));
   EXPECT_EQ(1, n);

   btStringKey sName = NULL;

   EXPECT_EQ(ENamedValuesOK, nvs.GetName(0, &sName));
   EXPECT_STREQ("str", sName);

   btcString str = NULL;
   EXPECT_EQ(ENamedValuesOK, nvs.Get(sName, &str));
   EXPECT_STREQ("abc", str);
}

TEST(NVS, aal0621)
{
   // CNamedValueSet implements a safety check preventing recursive Add().

   NamedValueSet nvs;

   btNumberKey iname = 3;
   btStringKey sname = "x";

   EXPECT_EQ(ENamedValuesRecursiveAdd, nvs.Add(iname, &nvs));
   EXPECT_EQ(ENamedValuesRecursiveAdd, nvs.Add(sname, &nvs));
}

TEST(NVS, aal0698)
{
   // An empty NamedValueSet is a subset (NamedValueSet::Subset()) of a non-empty NamedValueSet,
   // but not vice-versa.

   NamedValueSet a;
   NamedValueSet b;

   EXPECT_EQ(ENamedValuesOK, b.Add((btNumberKey)3, "hi"));

   EXPECT_TRUE(a.Subset(b));
   EXPECT_FALSE(b.Subset(a));
}

TEST(NVS, aal0699)
{
   // An empty NamedValueSet is a subset (NamedValueSet::Subset()) of another empty
   // NamedValueSet (and of itself).

   NamedValueSet a;
   NamedValueSet b;

   EXPECT_TRUE(a.Subset(b));
   EXPECT_TRUE(a.Subset(a));
}

