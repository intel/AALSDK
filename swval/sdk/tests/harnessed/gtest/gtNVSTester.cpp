// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"
#include "gtNVSTester.h"

////////////////////////////////////////////////////////////////////////////////

CNVSRandomizer::CNVSRandomizer() :
   m_Count(5),
   m_Seed(0),
   m_SaveSeed(0)
{}

CNVSRandomizer::~CNVSRandomizer()
{
   ClearList();
}

void CNVSRandomizer::Snapshot()
{
   ClearList();
   m_SaveSeed = m_Seed;
}

void CNVSRandomizer::Replay()
{
   ClearList();
   m_Seed = m_SaveSeed;
}

const INamedValueSet * CNVSRandomizer::Value()
{
   btUnsigned32bitInt i = ::GetRand(&m_Seed) % m_Count;
   switch ( i ) {
      case 0 : return Zero();
      case 1 : return One();
      case 2 : return Two();
      case 3 : return Three();
      case 4 : return Four();
   }
}

const INamedValueSet * CNVSRandomizer::ValueOtherThan(const INamedValueSet *nvs)
{
   const INamedValueSet *p;
   do
   {
      p = Value();
   }while ( p->operator == (*nvs) );
   return p;
}

btUnsigned32bitInt CNVSRandomizer::Seed(btUnsigned32bitInt s)
{
   btUnsigned32bitInt prev = m_Seed;
   m_Seed = s;
   return prev;
}

INamedValueSet * CNVSRandomizer::Alloc()
{
   INamedValueSet *p = AAL::NewNVS();
   m_NVSList.push_back(p);
   return p;
}

void CNVSRandomizer::ClearList()
{
   std::list<INamedValueSet *>::const_iterator iter;
   for ( iter = m_NVSList.begin() ; iter != m_NVSList.end() ; ++iter ) {
      AAL::DeleteNVS(*iter);
   }
   m_NVSList.clear();
}

INamedValueSet * CNVSRandomizer::Zero()
{
   // An empty NVS.
   return Alloc();
}

INamedValueSet * CNVSRandomizer::One()
{
   INamedValueSet *nvs = Alloc();

   btNumberKey iname;

   iname = 0;
   btBool b = true;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, b));

   iname = 1;
   btByte by = 10;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, by));

   iname = 2;
   bt32bitInt s32 = 15;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s32));

   iname = 3;
   btUnsigned32bitInt u32 = std::numeric_limits<btUnsigned32bitInt>::max();
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, u32));

   iname = 4;
   bt64bitInt s64 = std::numeric_limits<bt64bitInt>::max();
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s64));

   iname = 5;
   btUnsigned64bitInt u64 = 0;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, u64));

   iname = 6;
   btFloat f = 3.14;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, f));

   iname = 7;
   btcString str = "abc";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, str));

   iname = 8;
   btObjectType o = (btObjectType)7;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, o));

   iname = 9;
   btByteArray byA = &by;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, byA, 1));

   iname = 10;
   bt32bitInt s32A[] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s32A, 3));

   iname = 11;
   btUnsigned32bitInt u32A[] = { 25, std::numeric_limits<btUnsigned32bitInt>::max() - 1, 32 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, u32A, 3));

   iname = 12;
   bt64bitInt s64A[] = { 0, 1, 2 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s64A, 3));

   iname = 13;
   btUnsigned64bitInt u64A[] = { 25, 75, std::numeric_limits<btUnsigned64bitInt>::max() - 1 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, u64A, 3));

   iname = 14;
   btFloat fA[] = { 25.678, 50.4, 123.982, 43234872.97 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, fA, 4));

   iname = 15;
   btcString strA[] = { "A", "B", "C" };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, const_cast<btStringArray>(strA), 3));

   iname = 16;
   btObjectType oA[] = { (btObjectType)5, (btObjectType)4, (btObjectType)NULL, (btObjectType)3 };
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, oA, 4));


   btStringKey sname;

   sname = "00";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, oA, 4));

   sname = "01";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, const_cast<btStringArray>(strA), 3));

   sname = "02";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, fA, 4));

   sname = "03";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, u64A, 3));

   sname = "04";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, s64A, 3));

   sname = "05";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, u32A, 3));

   sname = "06";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, s32A, 3));

   sname = "07";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, byA, 1));

   sname = "08";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, o));

   sname = "09";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, str));

   sname = "10";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, f));

   sname = "11";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, u64));

   sname = "12";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, s64));

   sname = "13";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, u32));

   sname = "14";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, s32));

   sname = "15";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, by));

   sname = "16";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, b));

   return nvs;
}

INamedValueSet * CNVSRandomizer::Two()
{
   INamedValueSet *nvs = Alloc();

   btNumberKey iname = 5;

   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, Zero()));

   btStringKey sname = "25";

   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, One()));

   iname = 30;
   btByte by = 5;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, by));

   return nvs;
}

INamedValueSet * CNVSRandomizer::Three()
{
   INamedValueSet *nvs = Alloc();
   INamedValueSet *two = Two();

   btStringKey sname = "nvs_One";
   EXPECT_EQ(ENamedValuesOK, two->Add(sname, One()));

   sname = "nvs_Two";
   EXPECT_EQ(ENamedValuesOK, nvs->Add(sname, two));

   return nvs;
}

INamedValueSet * CNVSRandomizer::Four()
{
   INamedValueSet *nvs = Alloc();

   EXPECT_EQ(ENamedValuesOK, nvs->Add("nvs_Three", Three()));

   btNumberKey iname = 5;
   bt32bitInt s32 = 27;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s32));

   EXPECT_EQ(ENamedValuesOK, nvs->Add("nvs_Two", Two()));

   iname = 17;
   bt64bitInt s64 = 27;
   EXPECT_EQ(ENamedValuesOK, nvs->Add(iname, s64));

   EXPECT_EQ(ENamedValuesOK, nvs->Add("nvs_One", One()));

   return nvs;
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////





