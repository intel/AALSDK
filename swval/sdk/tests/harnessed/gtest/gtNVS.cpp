// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "aalsdk/AALNamedValueSet.h"

// Define structure that will be initialized per eBasicType
struct eBasicType_Data
{
   eBasicType_Data(eBasicTypes   EBT               =btUnknownType_t,
                   btBool        FISARRAY          =false,
                   btBool        FISVALID          =false,
                   btcString     SNAMEOFEBASICTYPE ="",
                   btcString     SNAMEOFNV         ="",
                   btUnsignedInt INAMEOFNV         =0,
                   void              *PDATA             =NULL,
                   btUnsignedInt UARRAYELEMENTS    =0) :
      ebt(EBT),
      fIsArray(FISARRAY),
      fIsValid(FISVALID),
      sNameOfeBasicType(SNAMEOFEBASICTYPE),
      sNameOfNV(SNAMEOFNV),
      iNameOfNV(INAMEOFNV),
      pData(PDATA),
      uArrayElements(UARRAYELEMENTS)
   {}

   btBool SanityCheck() const { return fIsValid; }


   eBasicTypes   ebt;                 // Redundant, but potentially makes tracking everything easier
   btBool        fIsArray;            // True if this is an array type, Implies uArrayElements is valid
   btBool        fIsValid;            // There are types that are not valid for inclusion in an NVS. This is TRUE if this one is.
   btcString     sNameOfeBasicType;   // Human-readable name of the eBasicType
   btStringKey   sNameOfNV;           // "NONAME" is reserved to mean no name
   btNumberKey   iNameOfNV;           // uNX is reserved to mean NO integer name, leaving 0 open for testing
   void         *pData;               // Pointer to data, if there is any
   btUnsignedInt uArrayElements;      // number of array elements, if there are any, 0 is valid
};

// Simple test fixture
class NVSSimple : public ::testing::Test
{
protected:
NVSSimple() :
   m_bTest(false),
   m_u8Test(0x20), // 'X'
   m_i32Test(-1),
   m_iTest(-1),
   m_u32Test((btUnsigned32bitInt)-1),
   m_uTest((btUnsignedInt)-1),
   m_i64Test(-1),
   m_u64Test((btUnsigned64bitInt)-1),
   m_fTest(3.14159f),
   m_sTest("Single String Data Value"),
   m_sDataForEmbeddedNewLine("Single String Data Value\nwith\nembedded and terminating newlines\n"),
   m_sNameForNewLineInName("Single String Name with\nEmbedded newline"),
   m_sDataForNewLineInName("Single String Data Value with a name that has embedded newline"),
   m_rgb8TestTemp(NULL),
   m_rgb8Test(NULL),
   m_rgi32TestTemp(NULL),
   m_rgi32Test(NULL),
   m_rgu32TestTemp(NULL),
   m_rgu32Test(NULL),
   m_rgi64TestTemp(NULL),
   m_rgi64Test(NULL),
   m_rgu64TestTemp(NULL),
   m_rgu64Test(NULL),
   m_pTest(NULL),
   m_rgfTestTemp(NULL),
   m_rgfTest(NULL),
   m_rgszTest(m_rgszTestTemp),
   m_rgp(m_rgpTemp),
   m_u8TestE('Y'),
   m_rgi64TestTempE(NULL),
   m_rgi64TestE(NULL),
   m_rgszTestE(NULL),
   m_nvsTestDeleted(false)
{
   m_rgb8TestTemp   = new(std::nothrow) btByte[10];
   m_rgb8Test       = m_rgb8TestTemp;

   m_rgi32TestTemp  = new(std::nothrow) bt32bitInt[5];
   m_rgi32Test      = m_rgi32TestTemp;

   m_rgu32TestTemp  = new(std::nothrow) btUnsigned32bitInt[5];
   m_rgu32Test      = m_rgu32TestTemp;

   m_rgi64TestTemp  = new(std::nothrow) bt64bitInt[5];
   m_rgi64Test      = m_rgi64TestTemp;

   m_rgu64TestTemp  = new(std::nothrow) btUnsigned64bitInt[5];
   m_rgu64Test      = m_rgu64TestTemp;

   m_rgfTestTemp    = new(std::nothrow) btFloat[5];
   m_rgfTest        = m_rgfTestTemp;

   m_rgi64TestTempE = new(std::nothrow) bt64bitInt[3];
   m_rgi64TestE     = m_rgi64TestTempE;

   m_rgszTestE      = m_rgszTestTempE;

   // note that the btXXXX are NOT contiguous and not appropriate array indices, in general
   // HOWEVER, the array MUST be kept in the order of the btXXXX up until btUnknownType_t.
   m_mapEBT[ 0] = eBasicType_Data(btBool_t,                  false, true, "btBool_t",                  "Bool",                 5,    &m_bTest,     0);
   m_mapEBT[ 1] = eBasicType_Data(btByte_t,                  false, true, "btByte_t",                  "Byte of X",            15,   &m_u8Test,    0);
   m_mapEBT[ 2] = eBasicType_Data(bt32bitInt_t,              false, true, "bt32bitInt_t",              "Int 32",               25,   &m_i32Test,   0);
   m_mapEBT[ 3] = eBasicType_Data(btInt_t,                   false, true, "btInt_t",                   "Int no size",          35,   &m_iTest,     0);
   m_mapEBT[ 4] = eBasicType_Data(btUnsigned32bitInt_t,      false, true, "btUnsigned32bitInt_t",      "Unsigned 32",          45,   &m_u32Test,   0);
   m_mapEBT[ 5] = eBasicType_Data(btUnsignedInt_t,           false, true, "btUnsignedInt_t",           "Unsigned no size",     55,   &m_uTest,     0);
   m_mapEBT[ 6] = eBasicType_Data(bt64bitInt_t,              false, true, "bt64bitInt_t",              "Int 64",               65,   &m_i64Test,   0);
   m_mapEBT[ 7] = eBasicType_Data(btUnsigned64bitInt_t,      false, true, "btUnsigned64bitInt_t",      "Unsigned 64",          75,   &m_u64Test,   0);
   m_mapEBT[ 8] = eBasicType_Data(btFloat_t,                 false, true, "btFloat_t",                 "Float",                85,   &m_fTest,     0);
   m_mapEBT[ 9] = eBasicType_Data(btString_t,                false, true, "btString_t",                "String",               95,   &m_sTest,     0);
   m_mapEBT[10] = eBasicType_Data(btNamedValueSet_t,         false, true, "btNamedValueSet_t",         "Embedded NVS",         105,  &m_nvsEmbed,  0);
   m_mapEBT[11] = eBasicType_Data(bt32bitIntArray_t,         true,  true, "bt32bitIntArray_t",         "Int 32 Array",         125,  &m_rgi32Test, 5);
   m_mapEBT[12] = eBasicType_Data(btUnsigned32bitIntArray_t, true,  true, "btUnsigned32bitIntArray_t", "Unsigned 32 Array",    135,  &m_rgu32Test, 5);
   m_mapEBT[13] = eBasicType_Data(bt64bitIntArray_t,         true,  true, "bt64bitIntArray_t",         "Int 64 Array",         145,  &m_rgi64Test, 5);
   m_mapEBT[14] = eBasicType_Data(btUnsigned64bitIntArray_t, true,  true, "btUnsigned64bitIntArray_t", "Unsigned 64 Array",    155,  &m_rgu64Test, 5);
   m_mapEBT[15] = eBasicType_Data(btObjectType_t,            false, true, "btObjectType_t",            "Object void*",         165,  &m_pTest,     0);
   m_mapEBT[16] = eBasicType_Data(btFloatArray_t,            true,  true, "btFloatArray_t",            "Float Array",          195,  &m_rgfTest,   5);
   m_mapEBT[17] = eBasicType_Data(btStringArray_t,           true,  true, "btStringArray_t",           "String Array",         205,  &m_rgszTest,  5);
   m_mapEBT[18] = eBasicType_Data(btObjectArray_t,           true,  true, "btObjectArray_t",           "Object void* Array",   215,  &m_rgp,       3);
   m_mapEBT[19] = eBasicType_Data(btByteArray_t,             true,  true, "btByteArray_t",             "Byte Array",           115,  &m_rgb8Test,  10);
   m_mapEBT[20] = eBasicType_Data(btUnknownType_t,           false, false,"btUnknownType_t",           "Unknown Type - ERROR", 235,   NULL,        0);
   // Insert other Entries for the Big NVS here, before the End of NVS;
   // and be sure to insert each one in the Create, Delete, and Compare routines
#define SPECIAL_CASE_1 21
   m_mapEBT[21] = eBasicType_Data(btString_t,                false, true, "btString_t",                "Single String with Embedded newline",
                                                                                                       995,
                                                                                                       (void *)&m_sDataForEmbeddedNewLine,
                                                                                                       0);
#define SPECIAL_CASE_2 22
   m_mapEBT[22] = eBasicType_Data(btString_t,                false, true, "btString_t",                m_sNameForNewLineInName,
                                                                                                       NVSSimple::sm_uNX,
                                                                                                       (void *)&m_sDataForNewLineInName,
                                                                                                       0);
   // --------------------------------------------------------------------------------------------------------------------------------------------
   m_mapEBT[23] = eBasicType_Data(btEndOfNVS_t,              false, false,"btEndOfNVS_t",              "EndofNVS - ERROR",
                                                                                                       245,
                                                                                                       NULL,
                                                                                                        0);

   m_mapEmbedNVS1[0] = eBasicType_Data(btByte_t,             false,   true,"btByte_t",        "Embedded Byte of Y",     915, &m_u8TestE,    0);
   m_mapEmbedNVS1[1] = eBasicType_Data(bt64bitInt_t,         true,    true,"bt64bitInt_t",    "Embedded Int 64 Array", 1035, &m_rgi64TestE, 3);
   m_mapEmbedNVS1[2] = eBasicType_Data(btStringArray_t,      true,    true,"btStringArray_t", "Embedded String Array", 1095, &m_rgszTestE,  2);


}
~NVSSimple()
{
   delete[] m_rgi64TestTempE;
   delete[] m_rgfTestTemp;
   delete[] m_rgu64TestTemp;
   delete[] m_rgi64TestTemp;
   delete[] m_rgu32TestTemp;
   delete[] m_rgi32TestTemp;
   delete[] m_rgb8TestTemp;
}

virtual void SetUp()
{
   ASSERT_NONNULL(m_rgb8TestTemp);
   m_rgb8TestTemp[0] = ' ';
   m_rgb8TestTemp[1] = 'B';
   m_rgb8TestTemp[2] = 'Y';
   m_rgb8TestTemp[3] = 'T';
   m_rgb8TestTemp[4] = 'E';
   m_rgb8TestTemp[5] = 'S';
   m_rgb8TestTemp[6] = 0;
   m_rgb8TestTemp[7] = 'M';
   m_rgb8TestTemp[8] = 'O';
   m_rgb8TestTemp[9] = 0;
   ASSERT_EQ(m_rgb8Test, m_rgb8TestTemp);

   ASSERT_NONNULL(m_rgi32TestTemp);
   m_rgi32TestTemp[0] = (bt32bitInt)0x80000000;
   m_rgi32TestTemp[1] = (bt32bitInt)-1;
   m_rgi32TestTemp[2] = 0;
   m_rgi32TestTemp[3] = 1;
   m_rgi32TestTemp[4] = (bt32bitInt)0x7FFFFFFF;
   ASSERT_EQ(m_rgi32Test, m_rgi32TestTemp);

   ASSERT_NONNULL(m_rgu32TestTemp);
   m_rgu32TestTemp[0] = (btUnsigned32bitInt)0x80000000;
   m_rgu32TestTemp[1] = (btUnsigned32bitInt)-1;
   m_rgu32TestTemp[2] = 0;
   m_rgu32TestTemp[3] = 1;
   m_rgu32TestTemp[4] = (btUnsigned32bitInt)0x7FFFFFFF;
   ASSERT_EQ(m_rgu32Test, m_rgu32TestTemp);

   ASSERT_NONNULL(m_rgi64TestTemp);
   m_rgi64TestTemp[0] = (bt64bitInt)0x8000000000000000LL;
   m_rgi64TestTemp[1] = -1;
   m_rgi64TestTemp[2] = 0;
   m_rgi64TestTemp[3] = 1;
   m_rgi64TestTemp[4] = (bt64bitInt)0x7FFFFFFFFFFFFFFFLL;
   ASSERT_EQ(m_rgi64Test, m_rgi64TestTemp);

   ASSERT_NONNULL(m_rgu64TestTemp);
   m_rgu64TestTemp[0] = (btUnsigned64bitInt)0x8000000000000000LL;
   m_rgu64TestTemp[1] = (btUnsigned64bitInt)-1;
   m_rgu64TestTemp[2] = 0;
   m_rgu64TestTemp[3] = 1;
   m_rgu64TestTemp[4] = (btUnsigned64bitInt)0x7FFFFFFFFFFFFFFFLL;
   ASSERT_EQ(m_rgu64Test, m_rgu64TestTemp);

   ASSERT_NONNULL(m_rgfTestTemp);
   m_rgfTestTemp[0] = -3.14159f;
   m_rgfTestTemp[1] = -1.0f;
   m_rgfTestTemp[2] = 0.0f;
   m_rgfTestTemp[3] = 1.0f;
   m_rgfTestTemp[4] = 3.14159f;
   ASSERT_EQ(m_rgfTest, m_rgfTestTemp);

   m_rgszTestTemp[0] = "First data string";
   m_rgszTestTemp[1] = "Second data string";
   m_rgszTestTemp[2] = "";
   m_rgszTestTemp[3] = "Fourth string, after a null string";
   m_rgszTestTemp[4] = "Fifth and last data string";
   ASSERT_EQ(m_rgszTest, m_rgszTestTemp);

   m_rgpTemp[0] = reinterpret_cast<btObjectType>(0L);
   m_rgpTemp[1] = reinterpret_cast<btObjectType>(4L);
   m_rgpTemp[2] = reinterpret_cast<btObjectType>(-1L);
   ASSERT_EQ(m_rgp, m_rgpTemp);

   ASSERT_NONNULL(m_rgi64TestTempE);
   m_rgi64TestTempE[0] = -1;
   m_rgi64TestTempE[1] = 0;
   m_rgi64TestTempE[2] = 1;
   ASSERT_EQ(m_rgi64TestE, m_rgi64TestTempE);

   m_rgszTestTempE[0] = "First embedded data string";
   m_rgszTestTempE[1] = "Second and last embedded data string";
   ASSERT_EQ(m_rgszTestE, m_rgszTestTempE);

   m_Map_eBasicType_toData[ btBool_t ]                  = m_mapEBT[ btBool_t ];
   m_Map_eBasicType_toData[ btByte_t ]                  = m_mapEBT[ btByte_t ];
   m_Map_eBasicType_toData[ bt32bitInt_t ]              = m_mapEBT[ bt32bitInt_t ];
   m_Map_eBasicType_toData[ btInt_t ]                   = m_mapEBT[ btInt_t ];
   m_Map_eBasicType_toData[ btUnsigned32bitInt_t ]      = m_mapEBT[ btUnsigned32bitInt_t ];
   m_Map_eBasicType_toData[ btUnsignedInt_t ]           = m_mapEBT[ btUnsignedInt_t ];
   m_Map_eBasicType_toData[ bt64bitInt_t ]              = m_mapEBT[ bt64bitInt_t ];
   m_Map_eBasicType_toData[ btUnsigned64bitInt_t ]      = m_mapEBT[ btUnsigned64bitInt_t ];
   m_Map_eBasicType_toData[ btFloat_t ]                 = m_mapEBT[ btFloat_t ];
   m_Map_eBasicType_toData[ btString_t ]                = m_mapEBT[ btString_t ];
   m_Map_eBasicType_toData[ btNamedValueSet_t ]         = m_mapEBT[ btNamedValueSet_t ];
   m_Map_eBasicType_toData[ btByteArray_t ]             = m_mapEBT[ btByteArray_t ];
   m_Map_eBasicType_toData[ bt32bitIntArray_t ]         = m_mapEBT[ bt32bitIntArray_t ];
   m_Map_eBasicType_toData[ btUnsigned32bitIntArray_t ] = m_mapEBT[ btUnsigned32bitIntArray_t ];
   m_Map_eBasicType_toData[ bt64bitIntArray_t ]         = m_mapEBT[ bt64bitIntArray_t ];
   m_Map_eBasicType_toData[ btUnsigned64bitIntArray_t ] = m_mapEBT[ btUnsigned64bitIntArray_t ];
   m_Map_eBasicType_toData[ btObjectType_t ]            = m_mapEBT[ btObjectType_t ];
   m_Map_eBasicType_toData[ btFloatArray_t ]            = m_mapEBT[ btFloatArray_t ];
   m_Map_eBasicType_toData[ btStringArray_t ]           = m_mapEBT[ btStringArray_t ];
   m_Map_eBasicType_toData[ btObjectArray_t ]           = m_mapEBT[ btObjectArray_t ];
   m_Map_eBasicType_toData[ btUnknownType_t ]           = m_mapEBT[ btUnknownType_t ];
   m_Map_eBasicType_toData[ btEndOfNVS_t ]              = m_mapEBT[ (sizeof(m_mapEBT) / sizeof(m_mapEBT[0])) - 1 ];

   m_sMapRetToName[ ENamedValuesOK ] =                                 "ENamedValuesOK";
   m_sMapRetToName[ ENamedValuesNameNotFound ] =                       "ENamedValuesNameNotFound";
   m_sMapRetToName[ ENamedValuesDuplicateName ] =                      "ENamedValuesDuplicateName";
   m_sMapRetToName[ ENamedValuesBadType ] =                            "ENamedValuesBadType";
   m_sMapRetToName[ ENamedValuesNotSupported ] =                       "ENamedValuesNotSupported";
   m_sMapRetToName[ ENamedValuesIndexOutOfRange ] =                    "ENamedValuesIndexOutOfRange";
   m_sMapRetToName[ ENamedValuesRecursiveAdd ] =                       "ENamedValuesRecursiveAdd";
   m_sMapRetToName[ ENamedValuesInternalError_InvalidNameFormat ] =    "ENamedValuesInternalError_InvalidNameFormat";
   m_sMapRetToName[ ENamedValuesInternalError_UnexpectedEndOfFile ] =  "ENamedValuesInternalError_UnexpectedEndOfFile";
   m_sMapRetToName[ ENamedValuesOutOfMemory ] =                        "ENamedValuesOutOfMemory";
   m_sMapRetToName[ ENamedValuesEndOfFile ] =                          "ENamedValuesEndOfFile";

   CreateSmallNVS(&m_nvsEmbed);
   CreateBigNVS(&m_nvsTest);

}
virtual void TearDown()
{
   DeleteBigNVS(&m_nvsTest);
}

#define ADD_NAME_AND_SINGLE_TYPE_TO_NVS(__type)                                                     \
btBool AddNameAndSingleTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                      btStringKey name, __type data)                           \
{                                                                                                   \
   ENamedValues e = pnvs->Add(name, data);                                                     \
   EXPECT_EQ(ENamedValuesOK, e)  <<                                                            \
      "Error on ADD of type\t" << pebt->sNameOfeBasicType <<                                        \
      "\tName\t" << name <<                                                                         \
      "\tReturn Value\t" << m_sMapRetToName[e] <<                                                   \
      "\tData\t" << data << "\n";                                                                   \
   return ENamedValuesOK == e;                                                                 \
}                                                                                                   \
btBool AddNameAndSingleTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                      btNumberKey name, __type data)                           \
{                                                                                                   \
   ENamedValues e = pnvs->Add(name, data);                                                     \
   EXPECT_EQ(ENamedValuesOK, e)  <<                                                            \
      "Error on ADD of type\t" << pebt->sNameOfeBasicType <<                                        \
      "\tName\t" << name <<                                                                         \
      "\tReturn Value\t" << m_sMapRetToName[e] <<                                                   \
      "\tData\t" << data << "\n";                                                                   \
   return ENamedValuesOK == e;                                                                 \
}

ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btBool)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btByte)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(bt32bitInt)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btUnsigned32bitInt)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(bt64bitInt)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btUnsigned64bitInt)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btFloat)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btcString)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(const INamedValueSet *)
ADD_NAME_AND_SINGLE_TYPE_TO_NVS(btObjectType)

#define ADD_NAME_AND_ARRAY_TYPE_TO_NVS(__type)                                                     \
btBool AddNameAndArrayTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                     btStringKey name, __type data)                           \
{                                                                                                  \
   ENamedValues e = pnvs->Add(name, data, pebt->uArrayElements);                              \
   EXPECT_EQ(ENamedValuesOK, e) <<                                                            \
      "Error on ADD of type\t" << pebt->sNameOfeBasicType <<                                       \
      "\tName\t" << name <<                                                                        \
      "\tReturn Value\t" << m_sMapRetToName[e] <<                                                  \
      "\tNumber of elements\t" << pebt->uArrayElements << "\n";                                    \
   return ENamedValuesOK == e;                                                                \
}                                                                                                  \
btBool AddNameAndArrayTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                     btNumberKey name, __type data)                           \
{                                                                                                  \
   ENamedValues e = pnvs->Add(name, data, pebt->uArrayElements);                              \
   EXPECT_EQ(ENamedValuesOK, e) <<                                                            \
      "Error on ADD of type\t" << pebt->sNameOfeBasicType <<                                       \
      "\tName\t" << name <<                                                                        \
      "\tReturn Value\t" << m_sMapRetToName[e] <<                                                  \
      "\tNumber of elements\t" << pebt->uArrayElements << "\n";                                    \
   return ENamedValuesOK == e;                                                                \
}

ADD_NAME_AND_ARRAY_TYPE_TO_NVS(bt32bitIntArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btUnsigned32bitIntArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(bt64bitIntArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btUnsigned64bitIntArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btFloatArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btStringArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btObjectArray)
ADD_NAME_AND_ARRAY_TYPE_TO_NVS(btByteArray)

// Instantiate one for every SINGLE data type
#define ADD_SINGLE_TYPE_TO_NVS(__type)                                                             \
void AddSingleTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, __type data) \
{  ASSERT_TRUE(pebt->SanityCheck());                                                               \
   if ( pebt->SanityCheck() ) { /* Sanity error check */                                           \
      if ( NVSSimple::sm_uNX != pebt->iNameOfNV ) { /* Do not use this particular number */        \
         AddNameAndSingleTypeToNVS(pnvs, pebt, pebt->iNameOfNV, data); /* Add numeric name */      \
      }                                                                                            \
      AddNameAndSingleTypeToNVS(pnvs, pebt, pebt->sNameOfNV, data); /* Add string name */          \
   }                                                                                               \
}

ADD_SINGLE_TYPE_TO_NVS(btBool)
ADD_SINGLE_TYPE_TO_NVS(btByte)
ADD_SINGLE_TYPE_TO_NVS(bt32bitInt)
ADD_SINGLE_TYPE_TO_NVS(btUnsigned32bitInt)
ADD_SINGLE_TYPE_TO_NVS(bt64bitInt)
ADD_SINGLE_TYPE_TO_NVS(btUnsigned64bitInt)
ADD_SINGLE_TYPE_TO_NVS(btFloat)
ADD_SINGLE_TYPE_TO_NVS(btcString)
ADD_SINGLE_TYPE_TO_NVS(const INamedValueSet *)
ADD_SINGLE_TYPE_TO_NVS(btObjectType)


// Instantiate one for every ARRAY data type
#define ADD_ARRAY_TYPE_TO_NVS(__type)                                                              \
void AddArrayTypeToNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, __type data)  \
{  ASSERT_TRUE(pebt->SanityCheck());                                                               \
   if ( pebt->SanityCheck() ) { /* Sanity error check */                                           \
      if ( NVSSimple::sm_uNX != pebt->iNameOfNV ) { /* Do not use this particular number */        \
         AddNameAndArrayTypeToNVS(pnvs, pebt, pebt->iNameOfNV, data); /* Add numeric name */       \
      }                                                                                            \
      AddNameAndArrayTypeToNVS(pnvs, pebt, pebt->sNameOfNV, data); /* Add string name */           \
   }                                                                                               \
}

ADD_ARRAY_TYPE_TO_NVS(bt32bitIntArray)
ADD_ARRAY_TYPE_TO_NVS(btUnsigned32bitIntArray)
ADD_ARRAY_TYPE_TO_NVS(bt64bitIntArray)
ADD_ARRAY_TYPE_TO_NVS(btUnsigned64bitIntArray)
ADD_ARRAY_TYPE_TO_NVS(btFloatArray)
ADD_ARRAY_TYPE_TO_NVS(btStringArray)
ADD_ARRAY_TYPE_TO_NVS(btObjectArray)
ADD_ARRAY_TYPE_TO_NVS(btByteArray)


#define DELETE_NAME_AND_TYPE_FROM_NVS(__type)                                                                                   \
void DeleteNameAndTypeFromNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, btStringKey name, __type data) \
{                                                                                                                               \
   ASSERT_TRUE(pnvs->Has(name)) << "Error on DELETE of type\t" << pebt->sNameOfeBasicType <<                                    \
               "\tName\t" << name <<  "Not found\n";                                                                            \
   ENamedValues e = pnvs->Delete(name);                                                                                         \
   ASSERT_EQ(ENamedValuesOK, e) <<  "Error on DELETE of type\t" << pebt->sNameOfeBasicType <<                                   \
               "\tName\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] << "\n";                                          \
}                                                                                                                               \
void DeleteNameAndTypeFromNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, btNumberKey name, __type data) \
{                                                                                                                               \
   ASSERT_TRUE(pnvs->Has(name)) << "Error on DELETE of type\t" << pebt->sNameOfeBasicType <<                                    \
               "\tName\t" << name <<  "Not found\n";                                                                            \
   ENamedValues e = pnvs->Delete(name);                                                                                         \
   ASSERT_EQ(ENamedValuesOK, e) <<  "Error on DELETE of type\t" << pebt->sNameOfeBasicType <<                                   \
               "\tName\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] << "\n";                                          \
}

DELETE_NAME_AND_TYPE_FROM_NVS(btBool)
DELETE_NAME_AND_TYPE_FROM_NVS(btByte)
DELETE_NAME_AND_TYPE_FROM_NVS(bt32bitInt)
DELETE_NAME_AND_TYPE_FROM_NVS(btUnsigned32bitInt)
DELETE_NAME_AND_TYPE_FROM_NVS(bt64bitInt)
DELETE_NAME_AND_TYPE_FROM_NVS(btUnsigned64bitInt)
DELETE_NAME_AND_TYPE_FROM_NVS(btFloat)
DELETE_NAME_AND_TYPE_FROM_NVS(btcString)
DELETE_NAME_AND_TYPE_FROM_NVS(const INamedValueSet *)
DELETE_NAME_AND_TYPE_FROM_NVS(btObjectType)


#define DELETE_TYPE_FROM_NVS(__type)                                                              \
void DeleteTypeFromNVS(INamedValueSet *pnvs, const struct eBasicType_Data *pebt, __type data) \
{                                                                                                 \
   ASSERT_TRUE(pebt->SanityCheck());                                                              \
   if ( pebt->SanityCheck() ) {                                                                   \
      if ( NVSSimple::sm_uNX != pebt->iNameOfNV ) { /* Do not use this particular number */       \
         DeleteNameAndTypeFromNVS(pnvs, pebt, pebt->iNameOfNV, data); /* Delete numeric name */   \
      }                                                                                           \
      DeleteNameAndTypeFromNVS(pnvs, pebt, pebt->sNameOfNV, data); /* Delete string name */       \
   }                                                                                              \
}

DELETE_TYPE_FROM_NVS(btBool)
DELETE_TYPE_FROM_NVS(btByte)
DELETE_TYPE_FROM_NVS(bt32bitInt)
DELETE_TYPE_FROM_NVS(btUnsigned32bitInt)
DELETE_TYPE_FROM_NVS(bt64bitInt)
DELETE_TYPE_FROM_NVS(btUnsigned64bitInt)
DELETE_TYPE_FROM_NVS(btFloat)
DELETE_TYPE_FROM_NVS(btcString)
DELETE_TYPE_FROM_NVS(const INamedValueSet *)
DELETE_TYPE_FROM_NVS(btObjectType)

#define COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(__type)                                                    \
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                   btStringKey name, __type data)                                 \
{                                                                                                      \
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<          \
               "\tName\t" << name <<  "Not found\n";                                                   \
                                                                                                       \
   __type dataNVS; /* Get the data from the NVS */                                                     \
   ENamedValues e = pnvs->Get(name, &dataNVS);                                                         \
                                                                                                       \
   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<     \
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<            \
               "\tData\t" << data << "\n";                                                             \
                                                                                                       \
   ASSERT_EQ(dataNVS, data) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<                 \
               ". Name \t" << name << "\tOriginal Data ='" << data <<                                  \
               "'\tNVS Data='" << dataNVS << "'\n";                                                    \
}                                                                                                      \
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                   btNumberKey name, __type data)                                 \
{                                                                                                      \
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<          \
               "\tName\t" << name <<  "Not found\n";                                                   \
                                                                                                       \
   __type dataNVS; /* Get the data from the NVS */                                                     \
   ENamedValues e = pnvs->Get(name, &dataNVS);                                                         \
                                                                                                       \
   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<     \
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<            \
               "\tData\t" << data << "\n";                                                             \
                                                                                                       \
   ASSERT_EQ(dataNVS, data) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<                 \
               ". Name \t" << name << "\tOriginal Data ='" << data <<                                  \
               "'\tNVS Data='" << dataNVS << "'\n";                                                    \
}

COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btBool)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btByte)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(bt32bitInt)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btUnsigned32bitInt)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(bt64bitInt)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btUnsigned64bitInt)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btFloat)
//COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btcString)
//COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(NamedValueSet)
COMPARE_NAME_AND_SINGLE_TYPE_TO_NVS(btObjectType)

// special case for btcString
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                   btStringKey name, btcString data)
{
   ASSERT_TRUE(pnvs->Has(name)) <<  "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   btcString dataNVS; // Get the data from the NVS
   ENamedValues e = pnvs->Get(name, &dataNVS);

   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<
               "\tData\t" << data << "\n";

   // Now compare the values
   ASSERT_EQ(0, strcmp(dataNVS, data)) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<
               ". Name \t" << name << "\tOriginal Data ='" << data <<
               "'\tNVS Data='" << dataNVS << "'\n";
}
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                   btNumberKey name, btcString data)
{
   ASSERT_TRUE(pnvs->Has(name)) <<  "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   btcString dataNVS; // Get the data from the NVS
   ENamedValues e = pnvs->Get(name, &dataNVS);

   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<
               "\tData\t" << data << "\n";

   // Now compare the values
   ASSERT_EQ(0, strcmp(dataNVS, data)) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<
               ". Name \t" << name << "\tOriginal Data ='" << data <<
               "'\tNVS Data='" << dataNVS << "'\n";
}

// NamedValueSet-specific version, assumes copy constructor works correctly
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                   btStringKey name, const INamedValueSet *NVSData)
{
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   INamedValueSet const *pNVSTemp = NULL; // Get pointer to embedded NVS (the small one)
   ENamedValues e = pnvs->Get(name, &pNVSTemp);

   ASSERT_EQ(ENamedValuesOK, e) <<  "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] << "\n";

   // Now compare the values, assumes operator== works correctly
//   int iRetValHolder = iRetVal;

   CompareEmbeddedNVS(pNVSTemp);

/*
   if (iRetValHolder != iRetVal) {
      cout <<  "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<
               ". Name \t" << name << "\n";
      cout << "Original reference NVS:\n";
      cout << nvsEmbed << endl;
//      NVSWriteNVS( stderr, nvsEmbed, 1);
      cout << "Modified Embedded NVS:\n";
//      NVSWriteNVS( stderr, *pNVSTemp, 1);
      cout << *pNVSTemp << endl;
      return;
   }
*/
}
void CompareNameAndSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                   btNumberKey name, const INamedValueSet *NVSData)
{
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   INamedValueSet const *pNVSTemp = NULL; // Get pointer to embedded NVS (the small one)
   ENamedValues e = pnvs->Get(name, &pNVSTemp);

   ASSERT_EQ(ENamedValuesOK, e) <<  "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] << "\n";

   // Now compare the values, assumes operator== works correctly
   CompareEmbeddedNVS(pNVSTemp);
}


#define COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(__type)                                                    \
void CompareNameAndArrayTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                  btStringKey name, __type data)                                 \
{                                                                                                     \
   ASSERT_TRUE(pnvs->Has(name)) <<  "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<        \
               "\tName\t" << name <<  "Not found\n";                                                  \
                                                                                                      \
   __type dataNVS; /* Get the data from the NVS */                                                    \
   ENamedValues e = pnvs->Get(name, &dataNVS);                                                        \
                                                                                                      \
   ASSERT_EQ(ENamedValuesOK, e) <<  "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<   \
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<           \
               "\tData\t" << data << "\n";                                                            \
                                                                                                      \
   /* Now compare the values */                                                                       \
   btUnsignedInt u;                                                                              \
   for ( u = 0 ; u < pebt->uArrayElements ; ++u ) {                                                   \
      EXPECT_EQ(dataNVS[u], data[u]) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<       \
                  ". Name \t" << name << "\tOriginal Data Element[" << u << "] ='" << data <<         \
                  "'\tNVS Data='" << dataNVS << "'\n";                                                \
   }                                                                                                  \
}                                                                                                     \
void CompareNameAndArrayTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, \
                                  btNumberKey name, __type data)                                 \
{                                                                                                     \
   ASSERT_TRUE(pnvs->Has(name)) <<  "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<        \
               "\tName\t" << name <<  "Not found\n";                                                  \
                                                                                                      \
   __type dataNVS; /* Get the data from the NVS */                                                    \
   ENamedValues e = pnvs->Get(name, &dataNVS);                                                        \
                                                                                                      \
   ASSERT_EQ(ENamedValuesOK, e) <<  "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<   \
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<           \
               "\tData\t" << data << "\n";                                                            \
                                                                                                      \
   /* Now compare the values */                                                                       \
   btUnsignedInt u;                                                                              \
   for ( u = 0 ; u < pebt->uArrayElements ; ++u ) {                                                   \
      EXPECT_EQ(dataNVS[u], data[u]) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<       \
                  ". Name \t" << name << "\tOriginal Data Element[" << u << "] ='" << data <<         \
                  "'\tNVS Data='" << dataNVS << "'\n";                                                \
   }                                                                                                  \
}

COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(bt32bitIntArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btUnsigned32bitIntArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(bt64bitIntArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btUnsigned64bitIntArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btFloatArray)
//COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btStringArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btObjectArray)
COMPARE_NAME_AND_ARRAY_TYPE_TO_NVS(btByteArray)

// btStringArray-specific version
void CompareNameAndArrayTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                  btStringKey name, btStringArray data)
{
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   btStringArray dataNVS; // Get the data from the NVS
   ENamedValues e = pnvs->Get(name, &dataNVS);

   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<
               "\tData\t" << data << "\n";

   // Now compare the values
   btUnsignedInt u;
   for ( u = 0 ; u < pebt->uArrayElements ; ++u ) {
      EXPECT_EQ(0, strcmp(dataNVS[u], data[u])) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<
                  ". Name \t" << name << "\tOriginal Data Element[" << u << "] ='" << data <<
                  "'\tNVS Data='" << dataNVS << "'\n";
   }
}
void CompareNameAndArrayTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt,
                                  btNumberKey name, btStringArray data)
{
   ASSERT_TRUE(pnvs->Has(name)) << "Error in COMPARE of type\t" << pebt->sNameOfeBasicType <<
               "\tName\t" << name <<  "Not found\n";

   btStringArray dataNVS; // Get the data from the NVS
   ENamedValues e = pnvs->Get(name, &dataNVS);

   ASSERT_EQ(ENamedValuesOK, e) << "Error in COMPARE: GET of type\t" << pebt->sNameOfeBasicType <<
               "\tfailed for Name\t" << name << "\tReturn Value\t" << m_sMapRetToName[e] <<
               "\tData\t" << data << "\n";

   // Now compare the values
   btUnsignedInt u;
   for ( u = 0 ; u < pebt->uArrayElements ; ++u ) {
      EXPECT_EQ(0, strcmp(dataNVS[u], data[u])) << "Error in COMPARE type\t" << pebt->sNameOfeBasicType <<
                  ". Name \t" << name << "\tOriginal Data Element[" << u << "] ='" << data <<
                  "'\tNVS Data='" << dataNVS << "'\n";
   }
}


#define COMPARE_SINGLE_TYPE_TO_NVS(__type)                                                                   \
void CompareSingleTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, __type data) \
{                                                                                                            \
   ASSERT_TRUE(pebt->SanityCheck());                                                                         \
   if ( pebt->SanityCheck() ) { /* Sanity error check */                                                     \
      if ( NVSSimple::sm_uNX != pebt->iNameOfNV ) { /* Do not use this particular number */                  \
         CompareNameAndSingleTypeToNVS(pnvs, pebt, pebt->iNameOfNV, data); /* Compare numeric name */        \
      }                                                                                                      \
      CompareNameAndSingleTypeToNVS(pnvs, pebt, pebt->sNameOfNV, data); /* Compare string name */            \
   }                                                                                                         \
}

COMPARE_SINGLE_TYPE_TO_NVS(btBool)
COMPARE_SINGLE_TYPE_TO_NVS(btByte)
COMPARE_SINGLE_TYPE_TO_NVS(bt32bitInt)
COMPARE_SINGLE_TYPE_TO_NVS(btUnsigned32bitInt)
COMPARE_SINGLE_TYPE_TO_NVS(bt64bitInt)
COMPARE_SINGLE_TYPE_TO_NVS(btUnsigned64bitInt)
COMPARE_SINGLE_TYPE_TO_NVS(btFloat)
COMPARE_SINGLE_TYPE_TO_NVS(btcString)
COMPARE_SINGLE_TYPE_TO_NVS(const INamedValueSet *)
COMPARE_SINGLE_TYPE_TO_NVS(btObjectType)


#define COMPARE_ARRAY_TYPE_TO_NVS(__type)                                                                   \
void CompareArrayTypeToNVS(const INamedValueSet *pnvs, const struct eBasicType_Data *pebt, __type data) \
{                                                                                                           \
   ASSERT_TRUE(pebt->SanityCheck());                                                                        \
   if ( pebt->SanityCheck() ) { /* Sanity error check */                                                    \
      if ( NVSSimple::sm_uNX != pebt->iNameOfNV ) { /* Do not use this particular number */                 \
         CompareNameAndArrayTypeToNVS(pnvs, pebt, pebt->iNameOfNV, data); /* Compare numeric name */        \
      }                                                                                                     \
      CompareNameAndArrayTypeToNVS(pnvs, pebt, pebt->sNameOfNV, data); /* Compare string name */            \
   }                                                                                                        \
}

COMPARE_ARRAY_TYPE_TO_NVS(bt32bitIntArray)
COMPARE_ARRAY_TYPE_TO_NVS(btUnsigned32bitIntArray)
COMPARE_ARRAY_TYPE_TO_NVS(bt64bitIntArray)
COMPARE_ARRAY_TYPE_TO_NVS(btUnsigned64bitIntArray)
COMPARE_ARRAY_TYPE_TO_NVS(btFloatArray)
COMPARE_ARRAY_TYPE_TO_NVS(btStringArray)
COMPARE_ARRAY_TYPE_TO_NVS(btObjectArray)
COMPARE_ARRAY_TYPE_TO_NVS(btByteArray)


void CompareEmbeddedNVS(const INamedValueSet *pnvs)
{
   // Before comparing, ensure that the number of elements match
   ASSERT_TRUE(CompareValidNames(pnvs, m_mapEmbedNVS1, sizeof(m_mapEmbedNVS1) / sizeof(m_mapEmbedNVS1[0]), "mapEmbedNVS1"));

   // btByte
   CompareSingleTypeToNVS(pnvs, &m_mapEmbedNVS1[0],
                          *reinterpret_cast<btByte *>(m_mapEmbedNVS1[0].pData));
   //bt64bitIntArray
   CompareArrayTypeToNVS(pnvs, &m_mapEmbedNVS1[1],
                         *reinterpret_cast<bt64bitIntArray *>(m_mapEmbedNVS1[1].pData));
   //btStringArray
   CompareArrayTypeToNVS(pnvs, &m_mapEmbedNVS1[2],
                         *reinterpret_cast<btStringArray *>(m_mapEmbedNVS1[2].pData));
}



// Utility routine to return the number of valid entries in an array of eBasicType_Data
btUnsignedInt GetNumValidNames(const struct eBasicType_Data *pebt,
                                    btUnsignedInt            uNumElements);

// Utility routine to compare the number of NVs defined by a table with the number
//    actually instantiated in the NVS created from that table
btBool CompareValidNames(const INamedValueSet     *pnvs,
                              const struct eBasicType_Data *pebt,
                              btUnsignedInt            uNumElements,
                              std::string                   sTableName);

void CreateSmallNVS(INamedValueSet *pnvs);

void   CreateBigNVS(INamedValueSet *pnvs);
void   DeleteBigNVS(INamedValueSet *pnvs);
void  CompareBigNVS(const INamedValueSet *pnvs);

FILE * F_OPEN_FOR_READ(const char *file)
{
   return fopen(file, "r+b");
}
FILE * F_OPEN_FOR_WRITE(const char *file)
{
   return fopen(file, "w+b");
}
btBool F_CLOSE(FILE *fp)
{
   return 0 == fclose(fp);
}

btBool S_OPEN_FOR_READ(std::fstream &fs, const char *file)
{
   fs.open(file, ios_base::binary | ios_base::in);
   return fs.good();
}
btBool S_OPEN_FOR_WRITE(std::fstream &fs, const char *file)
{
   fs.open(file, ios_base::binary | ios_base::out);
   return fs.good();
}
void S_CLOSE(std::fstream &fs)
{
   fs.clear();
   fs.close();
}

   btBool                   m_bTest;
   btByte                   m_u8Test;
   bt32bitInt               m_i32Test;
   btInt                    m_iTest;
   btUnsigned32bitInt       m_u32Test;
   btUnsignedInt            m_uTest;
   bt64bitInt               m_i64Test;
   btUnsigned64bitInt       m_u64Test;
   btFloat                  m_fTest;
   btcString                m_sTest;
   btcString                m_sDataForEmbeddedNewLine;
   btcString                m_sNameForNewLineInName;
   btcString                m_sDataForNewLineInName;

   btByte                  *m_rgb8TestTemp;
   btByteArray              m_rgb8Test;
   bt32bitInt              *m_rgi32TestTemp;
   bt32bitIntArray          m_rgi32Test;
   btUnsigned32bitInt      *m_rgu32TestTemp;
   btUnsigned32bitIntArray  m_rgu32Test;
   bt64bitInt              *m_rgi64TestTemp;
   bt64bitIntArray          m_rgi64Test;
   btUnsigned64bitInt      *m_rgu64TestTemp;
   btUnsigned64bitIntArray  m_rgu64Test;
   btObjectType             m_pTest;
   btFloat                 *m_rgfTestTemp;
   btFloatArray             m_rgfTest;
   btcStringArray           m_rgszTest;
   btObjectArray            m_rgp;

   // for Embedded NVS tests..
   btByte                   m_u8TestE;
   bt64bitInt              *m_rgi64TestTempE;
   bt64bitIntArray          m_rgi64TestE;
   btcStringArray           m_rgszTestE;

   btBool                   m_nvsTestDeleted;
   NamedValueSet            m_nvsEmbed;
   NamedValueSet            m_nvsTest;
   btcString                m_rgszTestTemp[5];
   btObjectType             m_rgpTemp[3];
   btcString                m_rgszTestTempE[2];

   struct eBasicType_Data        m_mapEBT[24];
   struct eBasicType_Data        m_mapEmbedNVS1[3];



   std::map<eBasicTypes,  struct eBasicType_Data> m_Map_eBasicType_toData; // Mapping of eBasicTypes to lots of information
   std::map<ENamedValues, std::string>            m_sMapRetToName;         // Mapping of eNamedValues to a printable value


   // Flag value indicating the iNameOfNV is not to be used
   static const btUnsignedInt sm_uNX;
};

const btUnsignedInt NVSSimple::sm_uNX = 314159;

btUnsignedInt NVSSimple::GetNumValidNames(const struct eBasicType_Data *pebt,
                                               btUnsignedInt            uNumElements)
{
   btUnsignedInt uNumValid = 0;
   btUnsignedInt u;

   for ( u = 0 ; u < uNumElements ; ++u ) {
      if ( pebt[u].fIsValid ) {
         ++uNumValid;                        // One NV for the string name
         if ( NVSSimple::sm_uNX != pebt[u].iNameOfNV ) {
            ++uNumValid;                     // Another for the integer name, unless the magic value
         }
      }
   }
   return uNumValid;
}

btBool NVSSimple::CompareValidNames(const INamedValueSet     *pnvs,
                                         const struct eBasicType_Data *pebt,
                                         btUnsignedInt            uNumElements,
                                         std::string                   /*sTableName*/)
{
   // Number of valid names in the defining table
   btUnsignedInt uNumNVs = GetNumValidNames(pebt, uNumElements);
   // Number of NVs in the NVS at this level
   btUnsignedInt uNumNames = 0;
   pnvs->GetNumNames(&uNumNames);

   EXPECT_EQ(uNumNVs, uNumNames);
   return uNumNVs == uNumNames;
}

void NVSSimple::CreateSmallNVS(INamedValueSet *pnvs)
{
   //----------------------------------------------------------------------------
   //WARNING: Be sure to keep CompareEmbeddedNVS() up to date with this function
   //----------------------------------------------------------------------------

   // btByte
   AddSingleTypeToNVS(pnvs, &m_mapEmbedNVS1[0],
                      *reinterpret_cast<btByte *>(m_mapEmbedNVS1[0].pData));
   //bt64bitIntArray
   AddArrayTypeToNVS(pnvs, &m_mapEmbedNVS1[1],
                      *reinterpret_cast<bt64bitIntArray *>(m_mapEmbedNVS1[1].pData));
   //btStringArray
   AddArrayTypeToNVS(pnvs, &m_mapEmbedNVS1[2],
                     *reinterpret_cast<btStringArray *>(m_mapEmbedNVS1[2].pData));
}

//=============================================================================
// Create an NVS that contains every defined type of element
// The ordering comes from the eBasicTypes enum in AALTypes.h (from AAL.h)
//=============================================================================
void NVSSimple::CreateBigNVS(INamedValueSet *pnvs)
{
   // btBool
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btBool_t],
                      *reinterpret_cast<btBool *>(m_Map_eBasicType_toData[btBool_t].pData));

   // btByte
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btByte_t],
                      *reinterpret_cast<btByte *>(m_Map_eBasicType_toData[btByte_t].pData));

   // bt32bitInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt32bitInt_t],
                      *reinterpret_cast<bt32bitInt *>(m_Map_eBasicType_toData[bt32bitInt_t].pData));

   // btInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btInt_t],
                      *reinterpret_cast<btInt *>(m_Map_eBasicType_toData[btInt_t].pData));

   // btUnsigned32bitInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitInt_t],
                      *reinterpret_cast<btUnsigned32bitInt *>(m_Map_eBasicType_toData[btUnsigned32bitInt_t].pData));

   // btUnsignedInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsignedInt_t],
                      *reinterpret_cast<btUnsignedInt *>(m_Map_eBasicType_toData[btUnsignedInt_t].pData));

   // bt64bitInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt64bitInt_t],
                      *reinterpret_cast<bt64bitInt *>(m_Map_eBasicType_toData[bt64bitInt_t].pData));

   // btUnsigned64bitInt
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitInt_t],
                      *reinterpret_cast<btUnsigned64bitInt *>(m_Map_eBasicType_toData[btUnsigned64bitInt_t].pData));

   // btFloat
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btFloat_t],
                      *reinterpret_cast<btFloat *>(m_Map_eBasicType_toData[btFloat_t].pData));

   // btString
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btString_t],
                      *reinterpret_cast<btString *>(m_Map_eBasicType_toData[btString_t].pData));

   // NamedValueSet
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btNamedValueSet_t],
                      reinterpret_cast<const INamedValueSet *>(m_Map_eBasicType_toData[btNamedValueSet_t].pData));

   // btByteArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btByteArray_t],
                     *reinterpret_cast<btByteArray *>(m_Map_eBasicType_toData[btByteArray_t].pData));

   // bt32bitIntArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt32bitIntArray_t],
                     *reinterpret_cast<bt32bitIntArray *>(m_Map_eBasicType_toData[bt32bitIntArray_t].pData));

   //btUnsigned32bitIntArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitIntArray_t],
                     *reinterpret_cast<btUnsigned32bitIntArray *>(m_Map_eBasicType_toData[btUnsigned32bitIntArray_t].pData));

   //bt64bitIntArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt64bitIntArray_t],
                     *reinterpret_cast<bt64bitIntArray *>(m_Map_eBasicType_toData[bt64bitIntArray_t].pData));

   //btUnsigned64bitIntArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitIntArray_t],
                     *reinterpret_cast<btUnsigned64bitIntArray *>(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].pData));

   // btObjectType
   AddSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btObjectType_t],
                      *reinterpret_cast<btObjectType *>(m_Map_eBasicType_toData[btObjectType_t].pData));

   // bt RuntimeControl_t deprecated and not tested
   // bt RuntimeControlpArray_t deprecated and not tested

   //btFloatArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btFloatArray_t],
                     *reinterpret_cast<btFloatArray *>(m_Map_eBasicType_toData[btFloatArray_t].pData));

   //btStringArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btStringArray_t],
                     *reinterpret_cast<btStringArray *>(m_Map_eBasicType_toData[btStringArray_t].pData));

   //btObjectArray
   AddArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btObjectArray_t],
                     *reinterpret_cast<btObjectArray *>(m_Map_eBasicType_toData[btObjectArray_t].pData));

   // SPECIAL CASES OF INTEREST

   // btString - Where the string contains embedded newlines
   AddSingleTypeToNVS(pnvs, &m_mapEBT[SPECIAL_CASE_1],
                      *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_1].pData));

   // btString - Where the string NAME contains embedded newlines
   AddSingleTypeToNVS(pnvs, &m_mapEBT[SPECIAL_CASE_2],
                      *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_2].pData));

}

//=============================================================================
// Delete each item in the passed in NVS (assuming it exactly matches the definition
// Checking for errors along the way
//=============================================================================
void NVSSimple::DeleteBigNVS(INamedValueSet *pnvs)
{
   if ( m_nvsTestDeleted ) {
      return;
   }

   // btBool
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btBool_t],
                     *reinterpret_cast<btBool *>(m_Map_eBasicType_toData[btBool_t].pData));

   // btByte
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btByte_t],
                     *reinterpret_cast<btByte *>(m_Map_eBasicType_toData[btByte_t].pData));

   // bt32bitInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[bt32bitInt_t],
                     *reinterpret_cast<bt32bitInt *>(m_Map_eBasicType_toData[bt32bitInt_t].pData));

   // btInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btInt_t],
                     *reinterpret_cast<btInt *>(m_Map_eBasicType_toData[btInt_t].pData));

   // btUnsigned32bitInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitInt_t],
                     *reinterpret_cast<btUnsigned32bitInt *>(m_Map_eBasicType_toData[btUnsigned32bitInt_t].pData));

   // btUnsignedInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btUnsignedInt_t],
                     *reinterpret_cast<btUnsignedInt *>(m_Map_eBasicType_toData[btUnsignedInt_t].pData));

   // bt64bitInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[bt64bitInt_t],
                     *reinterpret_cast<bt64bitInt *>(m_Map_eBasicType_toData[bt64bitInt_t].pData));

   // btUnsigned64bitInt
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitInt_t],
                     *reinterpret_cast<btUnsigned64bitInt *>(m_Map_eBasicType_toData[btUnsigned64bitInt_t].pData));

   // btFloat
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btFloat_t],
                     *reinterpret_cast<btFloat *>(m_Map_eBasicType_toData[btFloat_t].pData));

   // btString
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btString_t],
                     *reinterpret_cast<btString *>(m_Map_eBasicType_toData[btString_t].pData));

   // NamedValueSet
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btNamedValueSet_t],
                     reinterpret_cast<const INamedValueSet *>(m_Map_eBasicType_toData[btNamedValueSet_t].pData));

   // btByteArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btByteArray_t],
                     *reinterpret_cast<btByteArray *>(m_Map_eBasicType_toData[btByteArray_t].pData));

   // bt32bitIntArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[bt32bitIntArray_t],
                     *reinterpret_cast<bt32bitIntArray *>(m_Map_eBasicType_toData[bt32bitIntArray_t].pData));

   //btUnsigned32bitIntArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitIntArray_t],
                     *reinterpret_cast<btUnsigned32bitIntArray *>(m_Map_eBasicType_toData[btUnsigned32bitIntArray_t].pData));

   //bt64bitIntArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[bt64bitIntArray_t],
                     *reinterpret_cast<bt64bitIntArray *>(m_Map_eBasicType_toData[bt64bitIntArray_t].pData));

   //btUnsigned64bitIntArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitIntArray_t],
                     *reinterpret_cast<btUnsigned64bitIntArray *>(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].pData));

   // btObjectType
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btObjectType_t],
                     *reinterpret_cast<btObjectType *>(m_Map_eBasicType_toData[btObjectType_t].pData));

   // bt RuntimeControl_t deprecated and not tested
   // bt RuntimeControlpArray_t deprecated and not tested

   //btFloatArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btFloatArray_t],
                     *reinterpret_cast<btFloatArray *>(m_Map_eBasicType_toData[btFloatArray_t].pData));

   //btStringArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btStringArray_t],
                     *reinterpret_cast<btStringArray *>(m_Map_eBasicType_toData[btStringArray_t].pData));

   //btObjectArray
   DeleteTypeFromNVS(pnvs, &m_Map_eBasicType_toData[btObjectArray_t],
                     *reinterpret_cast<btObjectArray *>(m_Map_eBasicType_toData[btObjectArray_t].pData));

   // SPECIAL CASES OF INTEREST

   // btString - Where the string contains embedded newlines
   DeleteTypeFromNVS(pnvs, &m_mapEBT[SPECIAL_CASE_1],
                     *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_1].pData));

   // btString - Where the string NAME contains embedded newlines
   DeleteTypeFromNVS(pnvs, &m_mapEBT[SPECIAL_CASE_2],
                     *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_2].pData));

   m_nvsTestDeleted = true;
}

//=============================================================================
// Compare each item in the passed in NVS (assuming it exactly matches the definition)
// Checking for errors along the way
//=============================================================================
void NVSSimple::CompareBigNVS(const INamedValueSet *pnvs)
{
   // Before comparing, ensure that the number of elements match
   ASSERT_TRUE(CompareValidNames(pnvs, m_mapEBT, sizeof(m_mapEBT) / sizeof(m_mapEBT[0]), "mapEBT"));

   // btBool
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btBool_t],
                          *reinterpret_cast<btBool *>(m_Map_eBasicType_toData[btBool_t].pData));

   // btByte
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btByte_t],
                          *reinterpret_cast<btByte *>(m_Map_eBasicType_toData[btByte_t].pData));

   // bt32bitInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt32bitInt_t],
                          *reinterpret_cast<bt32bitInt *>(m_Map_eBasicType_toData[bt32bitInt_t].pData));

   // btInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btInt_t],
                          *reinterpret_cast<btInt *>(m_Map_eBasicType_toData[btInt_t].pData));

   // btUnsigned32bitInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitInt_t],
                          *reinterpret_cast<btUnsigned32bitInt *>(m_Map_eBasicType_toData[btUnsigned32bitInt_t].pData));

   // btUnsignedInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsignedInt_t],
                          *reinterpret_cast<btUnsignedInt *>(m_Map_eBasicType_toData[btUnsignedInt_t].pData));

   // bt64bitInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt64bitInt_t],
                          *reinterpret_cast<bt64bitInt *>(m_Map_eBasicType_toData[bt64bitInt_t].pData));

   // btUnsigned64bitInt
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitInt_t],
                          *reinterpret_cast<btUnsigned64bitInt *>(m_Map_eBasicType_toData[btUnsigned64bitInt_t].pData));

   // btFloat
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btFloat_t],
                          *reinterpret_cast<btFloat *>(m_Map_eBasicType_toData[btFloat_t].pData));

   // btString
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btString_t],
                          *reinterpret_cast<btString *>(m_Map_eBasicType_toData[btString_t].pData));

   // NamedValueSet
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btNamedValueSet_t],
                          reinterpret_cast<const INamedValueSet *>(m_Map_eBasicType_toData[btNamedValueSet_t].pData));
//    Will work once get ostream for NamedValueSet working, but for now add it later

   // btByteArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btByteArray_t],
                         *reinterpret_cast<btByteArray *>(m_Map_eBasicType_toData[btByteArray_t].pData));

   // bt32bitIntArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt32bitIntArray_t],
                         *reinterpret_cast<bt32bitIntArray *>(m_Map_eBasicType_toData[bt32bitIntArray_t].pData));

   //btUnsigned32bitIntArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned32bitIntArray_t],
                         *reinterpret_cast<btUnsigned32bitIntArray *>(m_Map_eBasicType_toData[btUnsigned32bitIntArray_t].pData));

   //bt64bitIntArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[bt64bitIntArray_t],
                         *reinterpret_cast<bt64bitIntArray *>(m_Map_eBasicType_toData[bt64bitIntArray_t].pData));

   //btUnsigned64bitIntArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btUnsigned64bitIntArray_t],
                         *reinterpret_cast<btUnsigned64bitIntArray *>(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].pData));

   // btObjectType
   CompareSingleTypeToNVS(pnvs, &m_Map_eBasicType_toData[btObjectType_t],
                          *reinterpret_cast<btObjectType *>(m_Map_eBasicType_toData[btObjectType_t].pData));

   // bt RuntimeControl_t deprecated and not tested
   // bt RuntimeControlpArray_t deprecated and not tested

   //btFloatArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btFloatArray_t],
                         *reinterpret_cast<btFloatArray *>(m_Map_eBasicType_toData[btFloatArray_t].pData));

   //btStringArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btStringArray_t],
                         *reinterpret_cast<btStringArray *>(m_Map_eBasicType_toData[btStringArray_t].pData));

   //btObjectArray
   CompareArrayTypeToNVS(pnvs, &m_Map_eBasicType_toData[btObjectArray_t],
                         *reinterpret_cast<btObjectArray *>(m_Map_eBasicType_toData[btObjectArray_t].pData));

   // SPECIAL CASES OF INTEREST

   // btString - Where the string contains embedded newlines
   CompareSingleTypeToNVS(pnvs, &m_mapEBT[SPECIAL_CASE_1],
                          *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_1].pData));

   // btString - Where the string NAME contains embedded newlines
   CompareSingleTypeToNVS(pnvs, &m_mapEBT[SPECIAL_CASE_2],
                          *reinterpret_cast<btString *>(m_mapEBT[SPECIAL_CASE_2].pData));

//   pnvs->Compare("Single String Name with\n Embedded newline","Single String Data Value with a name that has embedded newline");

}  // End of CompareBigNVS

#ifdef NVSFileIO

TEST_F(NVSSimple, DISABLED_NonEmptyToFromFILE)
{
   FILE *fp = F_OPEN_FOR_WRITE("file1");
   ASSERT_NONNULL(fp);

   ENamedValues e = m_nvsTest.Write(fp, 0);         // Write out a test version
   EXPECT_EQ(ENamedValuesOK, e) << "ERROR: Write(fp, 0) returned error " << m_sMapRetToName[e] << "\n";

   EXPECT_TRUE(F_CLOSE(fp));


   fp = F_OPEN_FOR_READ("file1");
   ASSERT_NONNULL(fp) << "ERROR: Could not open file1 for input";

   NamedValueSet nvs;

   e = nvs.Read(fp);  // Read in the full test version
   EXPECT_EQ(ENamedValuesOK, e) << "ERROR: Read(fp) returned error " << m_sMapRetToName[e] << "\n";

   EXPECT_TRUE(F_CLOSE(fp));


   CompareBigNVS(&nvs);
   EXPECT_EQ(nvs, m_nvsTest);


   // Write nvs back to disk, using std::fstream.
   std::fstream f;
   ASSERT_TRUE(S_OPEN_FOR_WRITE(f, "file6")) << "Could not open file6 for output";
   e = m_nvsTest.Write(f, 0);
   EXPECT_EQ(ENamedValuesOK, e) << "ERROR: Write(ostream & , 0) returned error " << m_sMapRetToName[e] << "\n";

   S_CLOSE(f);
}

#endif // NVSFileIO

TEST_F(NVSSimple, NonEmptyToFromfstream)
{
   std::fstream f;
   ASSERT_TRUE(S_OPEN_FOR_WRITE(f, "file4")) << "Could not open file4 for output";

   ENamedValues e = m_nvsTest.Write(f, 0);         // Write out a test version
   EXPECT_EQ(ENamedValuesOK, e) << "ERROR: write(ostream & , 0) returned error " << m_sMapRetToName[e] << "\n";

   S_CLOSE(f);
}

#ifdef NVSFileIO

TEST_F(NVSSimple, EmptyToFromFILE)
{
   DeleteBigNVS(&m_nvsTest); // Clear each item

   btUnsignedInt uNumNames = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, m_nvsTest.GetNumNames(&uNumNames));
   EXPECT_EQ(0, uNumNames) << "ERROR: Test NVS contained " << uNumNames <<
            " elements after expected complete removal\n";

   FILE *fp = F_OPEN_FOR_WRITE("file2");
   ASSERT_NONNULL(fp);

   ENamedValues e = m_nvsTest.Write(fp, 0);         // Write out a test version
   ASSERT_EQ(ENamedValuesOK, e) << "ERROR: Write(fp, 0) returned error " << m_sMapRetToName[e] << "\n";

   EXPECT_TRUE(F_CLOSE(fp));


   fp = F_OPEN_FOR_READ("file2");
   ASSERT_NONNULL(fp) << "ERROR: Could not open file2 for input\n";

   NamedValueSet empty;
   e = empty.Read(fp); // Read in the empty test

   EXPECT_TRUE(F_CLOSE(fp));
   ASSERT_EQ(ENamedValuesEndOfFile, e) << "ERROR: Read(fp) returned error " << m_sMapRetToName[e] << "\n";

   ASSERT_EQ(m_nvsTest, empty);

   uNumNames = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, empty.GetNumNames(&uNumNames));
   EXPECT_EQ(0, uNumNames);
}

#endif // NFSFileIO

TEST_F(NVSSimple, EmptyToFromfstream)
{
   DeleteBigNVS(&m_nvsTest); // Clear each item

   btUnsignedInt uNumNames = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, m_nvsTest.GetNumNames(&uNumNames));
   EXPECT_EQ(0, uNumNames) << "ERROR: Test NVS contained " << uNumNames <<
            " elements after expected complete removal\n";

   std::fstream f;
   ASSERT_TRUE(S_OPEN_FOR_WRITE(f, "file5")) << "Could not open file5 for output";

   ENamedValues e = m_nvsTest.Write(f, 0);          // Write out a test version
   EXPECT_EQ(ENamedValuesOK, e) << "ERROR: NVSWriteNVS returned error " <<
            m_Map_eBasicType_toData[btNamedValueSet_t].sNameOfeBasicType << "\n";

   S_CLOSE(f);


   ASSERT_TRUE(S_OPEN_FOR_READ(f, "file5")) << "Could not open file5 for input";

   NamedValueSet empty;
   e = empty.Read(f); // Read in the empty test
   ASSERT_EQ(ENamedValuesEndOfFile, e) << "ERROR: Read(istream & ) returned error " << m_sMapRetToName[e] << "\n";

   ASSERT_EQ(m_nvsTest, empty);

   uNumNames = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, empty.GetNumNames(&uNumNames));
   EXPECT_EQ(0, uNumNames);
}

TEST_F(NVSSimple, Empty)
{
   ENamedValues e = m_nvsTest.Empty();
   EXPECT_EQ(ENamedValuesOK, e) << "error during Empty()";
   m_nvsTestDeleted = true;

   btUnsignedInt uNumNames = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, m_nvsTest.GetNumNames(&uNumNames));
   EXPECT_EQ(0, uNumNames) << "names remain after Empty()";
}

TEST_F(NVSSimple, CopyConstructor)
{
   NamedValueSet copy(m_nvsTest);
   CompareBigNVS(&copy);
   ASSERT_TRUE(copy == m_nvsTest) << "equality operator\n";
}

TEST_F(NVSSimple, AssignmentOperator)
{
   NamedValueSet empty;

   empty = m_nvsTest;
   CompareBigNVS(&empty);
   ASSERT_TRUE(empty == m_nvsTest) << "equality operator\n";

   NamedValueSet nonempty;

   EXPECT_EQ(ENamedValuesOK, nonempty.Add("Name", (btUnsigned32bitInt)3));
   EXPECT_TRUE(nonempty.Has("Name"));

   btUnsigned32bitInt val = 0xdeadbeef;
   EXPECT_EQ(ENamedValuesOK, nonempty.Get("Name", &val));
   EXPECT_EQ(3, val);

   nonempty = m_nvsTest;
   CompareBigNVS(&nonempty);
   ASSERT_TRUE(nonempty == m_nvsTest) << "equality operator\n";

   btUnsignedInt n1 = 0xdeadbeef;
   btUnsignedInt n2 = 0xdeadbeef;

   EXPECT_EQ(ENamedValuesOK, m_nvsTest.GetNumNames(&n1));
   EXPECT_EQ(ENamedValuesOK, nonempty.GetNumNames(&n2));
   EXPECT_EQ(n1, n2);
}

TEST_F(NVSSimple, Subset)
{
   NamedValueSet copy(m_nvsTest);
   ASSERT_TRUE(copy == m_nvsTest);

   ASSERT_TRUE(m_nvsTest.Subset(copy)) << "failed to detect improper subset\n";


   // Create an enhanced embedded NVS with which to modify nvsCopy
   NamedValueSet nvsEmbedModified(m_nvsEmbed);
   // modify the copy
   nvsEmbedModified.Add("ADDED STRING NAME", "ADDED STRING VALUE");
   // modify copy to use the modified embedded NVS
   ASSERT_EQ(ENamedValuesOK, copy.Delete(m_Map_eBasicType_toData[btNamedValueSet_t].sNameOfNV));
   ASSERT_EQ(ENamedValuesOK, copy.Add(m_Map_eBasicType_toData[btNamedValueSet_t].sNameOfNV, &nvsEmbedModified));


   ASSERT_FALSE(copy == m_nvsTest) << "copy should be a superset\n";

   ASSERT_TRUE(m_nvsTest.Subset(copy)) << "m_nvsTest should be a true subset of copy\n";
   ASSERT_FALSE(copy.Subset(m_nvsTest)) << "copy should be a superset of m_nvsTest\n";

   ASSERT_EQ(ENamedValuesOK, copy.Delete(m_Map_eBasicType_toData[btNamedValueSet_t].sNameOfNV));
   ASSERT_EQ(ENamedValuesOK, copy.Add(m_Map_eBasicType_toData[btNamedValueSet_t].sNameOfNV, &m_nvsEmbed));

   ASSERT_TRUE(copy == m_nvsTest) << "failed to remove embedded NVS superset element and recover original\n";
}

TEST_F(NVSSimple, SubsetArrays)
{
   NamedValueSet copy(m_nvsTest);
   ASSERT_TRUE(copy == m_nvsTest);

   // Map_eBasicType_toData[btUnsigned64bitIntArray_t].pData is a pointer to an array of 64-bit unsigned ints
   // Map_eBasicType_toData[btUnsigned64bitIntArray_t].uArrayElements is the number of elements
   // Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV is the name of that NVP
   // That array has values like this: A B C D E
   // Create a new superset array with values like this A A B C D A B C D E A B
   // Add it to copy

   ASSERT_EQ(5, m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].uArrayElements);

   btUnsigned64bitIntArray rgu64Old = NULL;
   btUnsigned64bitIntArray rgu64New;

   EXPECT_EQ(ENamedValuesOK, copy.Get(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV, &rgu64Old));

   rgu64New = new(std::nothrow) btUnsigned64bitInt[12];
   ASSERT_NONNULL(rgu64New);

   rgu64New[ 0] = rgu64Old[0];   // A
   rgu64New[ 1] = rgu64Old[0];   // A
   rgu64New[ 2] = rgu64Old[1];   // B
   rgu64New[ 3] = rgu64Old[2];   // C
   rgu64New[ 4] = rgu64Old[3];   // D
   rgu64New[ 5] = rgu64Old[0];   // A
   rgu64New[ 6] = rgu64Old[1];   // B
   rgu64New[ 7] = rgu64Old[2];   // C
   rgu64New[ 8] = rgu64Old[3];   // D
   rgu64New[ 9] = rgu64Old[4];   // E
   rgu64New[10] = rgu64Old[0];   // A
   rgu64New[11] = rgu64Old[1];   // B

   // Delete the old array
   EXPECT_EQ(ENamedValuesOK, copy.Delete(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV));

   // Add the new array in its place
   EXPECT_EQ(ENamedValuesOK, copy.Add(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV, rgu64New, 12));

   delete[] rgu64New;

   EXPECT_TRUE(m_nvsTest.Subset(copy)) << "Subset fails to detect array Subset\n";


   EXPECT_EQ(ENamedValuesOK, copy.Delete(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV));
   EXPECT_EQ(ENamedValuesOK, copy.Add(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].sNameOfNV,
                                      *reinterpret_cast<btUnsigned64bitIntArray *>(m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].pData),
                                      m_Map_eBasicType_toData[btUnsigned64bitIntArray_t].uArrayElements));

   EXPECT_TRUE(copy == m_nvsTest) << "Failed to revert array subset back to original\n";
}

TEST_F(NVSSimple, SubsetStringArrays)
{
   NamedValueSet copy(m_nvsTest);
   ASSERT_TRUE(copy == m_nvsTest);

   // Map_eBasicType_toData[btStringArray_t].pData is a pointer to an array of strings
   // Map_eBasicType_toData[btStringArray_t].uArrayElements is the number of elements
   // Map_eBasicType_toData[btStringArray_t].sNameOfNV is the name of that NVP
   // That array has values like this: A B C D E
   // Create a new superset array with values like this A A B C D A B C D E A B
   // Add it to copy

   ASSERT_EQ(5, m_Map_eBasicType_toData[btStringArray_t].uArrayElements);

   // Create the new array, really char**, which btStringArray_t
   btcStringArray rgszNew = new(std::nothrow) btcString[12];
   ASSERT_NONNULL(rgszNew);

   rgszNew[ 0] = m_rgszTestTemp[0];   // A
   rgszNew[ 1] = m_rgszTestTemp[0];   // A
   rgszNew[ 2] = m_rgszTestTemp[1];   // B
   rgszNew[ 3] = m_rgszTestTemp[2];   // C
   rgszNew[ 4] = m_rgszTestTemp[3];   // D
   rgszNew[ 5] = m_rgszTestTemp[0];   // A
   rgszNew[ 6] = m_rgszTestTemp[1];   // B
   rgszNew[ 7] = m_rgszTestTemp[2];   // C
   rgszNew[ 8] = m_rgszTestTemp[3];   // D
   rgszNew[ 9] = m_rgszTestTemp[4];   // E
   rgszNew[10] = m_rgszTestTemp[0];   // A
   rgszNew[11] = m_rgszTestTemp[1];   // B

   // Delete the old array
   ASSERT_EQ(ENamedValuesOK, copy.Delete(m_Map_eBasicType_toData[btStringArray_t].sNameOfNV));

   // Add the new array in its place
   ASSERT_EQ(ENamedValuesOK, copy.Add(m_Map_eBasicType_toData[btStringArray_t].sNameOfNV,
                                      const_cast<btStringArray>(rgszNew), 12));

   delete[] rgszNew;

   ASSERT_TRUE(m_nvsTest.Subset(copy)) << "m_nvsTest should be a subset of copy\n";
}

TEST_F(NVSSimple, RecursiveAddSafeguard)
{
   ENamedValues e = m_nvsTest.Add("Recursive NVS", &m_nvsTest);
   EXPECT_EQ(ENamedValuesRecursiveAdd, e);
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btBool_f : public ::testing::Test
{
protected:
   NamedValueSet_btBool_f() {}
   virtual ~NamedValueSet_btBool_f() {}

   virtual void SetUp()
   {
      m_iNames[0] = 0;
      m_iNames[1] = 1;

      m_sNames[0] = "false";
      m_sNames[1] = "true";

      btUnsignedInt i;
      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   NamedValueSet m_NVS;
   btNumberKey   m_iNames[2];
   btStringKey   m_sNames[2];
};

TEST_F(NamedValueSet_btBool_f, test_case1)
{
   // Add() / Get() btBool, btNumberKey

   const btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);

   btUnsignedInt i;
   btNumberKey   name = 0;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btStringKey_t;

   btBool value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = (btBool) name;

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btBool_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = (btBool) name;

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btBool queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

TEST_F(NamedValueSet_btBool_f, test_case2)
{
   // Add() / Get() btBool, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btBool value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = (btBool) i;

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btBool_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = (btBool) i;

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btBool queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btFloat_f : public ::testing::Test
{
protected:
   NamedValueSet_btFloat_f() : m_R(0) {}
   virtual ~NamedValueSet_btFloat_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      m_Values[0] = 1.0;
      m_Values[1] = 3.14;
      m_Values[2] = 5.0;
      m_Values[3] = 10.4321;
      m_Values[4] = 25.125;
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btFloat            m_Values[5];
};

TEST_F(NamedValueSet_btFloat_f, test_case1)
{
   // Add() / Get() btFloat, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btFloat value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btFloat_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btFloat queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

TEST_F(NamedValueSet_btFloat_f, test_case2)
{
   // Add() / Get() btFloat, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btFloat value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btFloat_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btFloat queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btFloatArray_f : public ::testing::Test
{
protected:
   NamedValueSet_btFloatArray_f() : m_R(0) {}
   virtual ~NamedValueSet_btFloatArray_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      btUnsignedInt j;

                          N = sizeof(m_Values) / sizeof(m_Values[0]);
      const btUnsignedInt M = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);
      for ( i = 0 ; i < N ; ++i ) {
         for ( j = 0 ; j < M ; ++j ) {
            m_Values[i][j] = 0.0 + ((i + 1) * (j + 1));
         }
      }
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btFloat            m_Values[5][5];
};

TEST_F(NamedValueSet_btFloatArray_f, test_case1)
{
   // Add() / Get() btFloatArray, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btFloatArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btFloatArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btFloatArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_EQ(m_Values[i][0], queried_value[0]);
      EXPECT_EQ(m_Values[i][1], queried_value[1]);
      EXPECT_EQ(m_Values[i][2], queried_value[2]);
      EXPECT_EQ(m_Values[i][3], queried_value[3]);
      EXPECT_EQ(m_Values[i][4], queried_value[4]);
   }
}

TEST_F(NamedValueSet_btFloatArray_f, test_case2)
{
   // Add() / Get() btFloatArray, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btFloatArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btFloatArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btFloatArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_EQ(m_Values[i][0], queried_value[0]);
      EXPECT_EQ(m_Values[i][1], queried_value[1]);
      EXPECT_EQ(m_Values[i][2], queried_value[2]);
      EXPECT_EQ(m_Values[i][3], queried_value[3]);
      EXPECT_EQ(m_Values[i][4], queried_value[4]);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btcString_f : public ::testing::Test
{
protected:
   NamedValueSet_btcString_f() : m_R(0) {}
   virtual ~NamedValueSet_btcString_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      m_Values[0] = "4";
      m_Values[1] = "1";
      m_Values[2] = "3";
      m_Values[3] = "2";
      m_Values[4] = "0";
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btcString          m_Values[5];
};

TEST_F(NamedValueSet_btcString_f, test_case1)
{
   // Add() / Get() btcString, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btcString value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btString_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btcString queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_STREQ(value, queried_value);
   }
}

TEST_F(NamedValueSet_btcString_f, test_case2)
{
   // Add() / Get() btcString, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btcString value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btString_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btcString queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_STREQ(value, queried_value);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btStringArray_f : public ::testing::Test
{
protected:
   NamedValueSet_btStringArray_f() : m_R(0) {}
   virtual ~NamedValueSet_btStringArray_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      m_Values[0][0] = "04";
      m_Values[0][1] = "01";
      m_Values[0][2] = "03";
      m_Values[0][3] = "02";
      m_Values[0][4] = "00";

      m_Values[1][0] = "14";
      m_Values[1][1] = "11";
      m_Values[1][2] = "13";
      m_Values[1][3] = "12";
      m_Values[1][4] = "10";

      m_Values[2][0] = "24";
      m_Values[2][1] = "21";
      m_Values[2][2] = "23";
      m_Values[2][3] = "22";
      m_Values[2][4] = "20";

      m_Values[3][0] = "34";
      m_Values[3][1] = "31";
      m_Values[3][2] = "33";
      m_Values[3][3] = "32";
      m_Values[3][4] = "30";

      m_Values[4][0] = "44";
      m_Values[4][1] = "41";
      m_Values[4][2] = "43";
      m_Values[4][3] = "42";
      m_Values[4][4] = "40";
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btcString          m_Values[5][5];
};

TEST_F(NamedValueSet_btStringArray_f, test_case1)
{
   // Add() / Get() btcString, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btStringArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = const_cast<btStringArray>(m_Values[i]);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btStringArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btStringArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_STREQ(m_Values[i][0], queried_value[0]);
      EXPECT_STREQ(m_Values[i][1], queried_value[1]);
      EXPECT_STREQ(m_Values[i][2], queried_value[2]);
      EXPECT_STREQ(m_Values[i][3], queried_value[3]);
      EXPECT_STREQ(m_Values[i][4], queried_value[4]);
   }
}

TEST_F(NamedValueSet_btStringArray_f, test_case2)
{
   // Add() / Get() btcString, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btStringArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = const_cast<btStringArray>(m_Values[i]);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btStringArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btStringArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_STREQ(m_Values[i][0], queried_value[0]);
      EXPECT_STREQ(m_Values[i][1], queried_value[1]);
      EXPECT_STREQ(m_Values[i][2], queried_value[2]);
      EXPECT_STREQ(m_Values[i][3], queried_value[3]);
      EXPECT_STREQ(m_Values[i][4], queried_value[4]);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btObjectType_f : public ::testing::Test
{
protected:
   NamedValueSet_btObjectType_f() : m_R(0) {}
   virtual ~NamedValueSet_btObjectType_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      m_Values[0] = (btObjectType) &m_iNames[4];
      m_Values[1] = (btObjectType) &m_sNames[1];
      m_Values[2] = (btObjectType) &m_iNames[3];
      m_Values[3] = (btObjectType) &m_sNames[2];
      m_Values[4] = (btObjectType) &m_iNames[0];
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btObjectType       m_Values[5];
};

TEST_F(NamedValueSet_btObjectType_f, test_case1)
{
   // Add() / Get() btObjectType, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btObjectType value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btObjectType_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btObjectType queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

TEST_F(NamedValueSet_btObjectType_f, test_case2)
{
   // Add() / Get() btObjectType, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btObjectType value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btObjectType_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btObjectType queried_value;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btObjectArray_f : public ::testing::Test
{
protected:
   NamedValueSet_btObjectArray_f() : m_R(0) {}
   virtual ~NamedValueSet_btObjectArray_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }

      m_Values[0][0] = (btObjectType) &m_iNames[4];
      m_Values[0][1] = (btObjectType) &m_sNames[1];
      m_Values[0][2] = (btObjectType) &m_iNames[3];
      m_Values[0][3] = (btObjectType) &m_sNames[2];
      m_Values[0][4] = (btObjectType) &m_iNames[0];

      m_Values[1][0] = (btObjectType) &m_iNames[4];
      m_Values[1][1] = (btObjectType) &m_sNames[1];
      m_Values[1][2] = (btObjectType) &m_iNames[3];
      m_Values[1][3] = (btObjectType) &m_sNames[2];
      m_Values[1][4] = (btObjectType) &m_iNames[0];

      m_Values[2][0] = (btObjectType) &m_iNames[4];
      m_Values[2][1] = (btObjectType) &m_sNames[1];
      m_Values[2][2] = (btObjectType) &m_iNames[3];
      m_Values[2][3] = (btObjectType) &m_sNames[2];
      m_Values[2][4] = (btObjectType) &m_iNames[0];

      m_Values[3][0] = (btObjectType) &m_iNames[4];
      m_Values[3][1] = (btObjectType) &m_sNames[1];
      m_Values[3][2] = (btObjectType) &m_iNames[3];
      m_Values[3][3] = (btObjectType) &m_sNames[2];
      m_Values[3][4] = (btObjectType) &m_iNames[0];

      m_Values[4][0] = (btObjectType) &m_iNames[4];
      m_Values[4][1] = (btObjectType) &m_sNames[1];
      m_Values[4][2] = (btObjectType) &m_iNames[3];
      m_Values[4][3] = (btObjectType) &m_sNames[2];
      m_Values[4][4] = (btObjectType) &m_iNames[0];
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   btObjectType       m_Values[5][5];
};

TEST_F(NamedValueSet_btObjectArray_f, test_case1)
{
   // Add() / Get() btObjectType, btNumberKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   btObjectArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btObjectArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      btObjectArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_EQ(m_Values[i][0], queried_value[0]);
      EXPECT_EQ(m_Values[i][1], queried_value[1]);
      EXPECT_EQ(m_Values[i][2], queried_value[2]);
      EXPECT_EQ(m_Values[i][3], queried_value[3]);
      EXPECT_EQ(m_Values[i][4], queried_value[4]);
   }
}

TEST_F(NamedValueSet_btObjectArray_f, test_case2)
{
   // Add() / Get() btObjectType, btStringKey

   const btUnsignedInt N = sizeof(m_sNames) / sizeof(m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   btObjectArray value;
   const btUnsigned32bitInt NumElements = sizeof(m_Values[0]) / sizeof(m_Values[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value, NumElements));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btObjectArray_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_sNames[i];

      btStringKey queried_name = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      btObjectArray queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));

      EXPECT_EQ(m_Values[i][0], queried_value[0]);
      EXPECT_EQ(m_Values[i][1], queried_value[1]);
      EXPECT_EQ(m_Values[i][2], queried_value[2]);
      EXPECT_EQ(m_Values[i][3], queried_value[3]);
      EXPECT_EQ(m_Values[i][4], queried_value[4]);
   }
}

////////////////////////////////////////////////////////////////////////////////

// Type-parameterized test fixture
// btByte, bt32bitInt, btUnsigned32bitInt, bt64bitInt, btUnsigned64bitInt
template <typename T>
class NamedValueSet_tp_0 : public ::testing::Test
{
protected:
   NamedValueSet_tp_0() : m_R(0) {}
   virtual ~NamedValueSet_tp_0() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)128, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
};

TYPED_TEST_CASE_P(NamedValueSet_tp_0);

TYPED_TEST_P(NamedValueSet_tp_0, test_case1)
{
   // Add() / Get() integer types, btNumberKey

   const btUnsignedInt N = sizeof(this->m_iNames) / sizeof(this->m_iNames[0]);

   btUnsignedInt i;
   btNumberKey   name = 0;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btStringKey_t;

   // Inside a test, refer to TypeParam to get the type parameter.
   TypeParam value;

   eBasicTypes ExpectType = btUnknownType_t;
   if ( typeid(AAL::btByte) == typeid(value) ) {
      ExpectType = btByte_t;
   } else if ( typeid(AAL::bt32bitInt) == typeid(value) ) {
      ExpectType = bt32bitInt_t;
   } else if ( typeid(AAL::btUnsigned32bitInt) == typeid(value) ) {
      ExpectType = btUnsigned32bitInt_t;
   } else if ( typeid(AAL::bt64bitInt) == typeid(value) ) {
      ExpectType = bt64bitInt_t;
   } else if ( typeid(AAL::btUnsigned64bitInt) == typeid(value) ) {
      ExpectType = btUnsigned64bitInt_t;
   }
   EXPECT_NE(btUnknownType_t, ExpectType);

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_iNames[i];
      value = (TypeParam) name;

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Type(name, &t));
      EXPECT_EQ(ExpectType, t);

      EXPECT_TRUE(this->m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_iNames[i];
      value = (TypeParam) name;

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name) << "names: { "
               << this->m_iNames[0] << ' '
               << this->m_iNames[1] << ' '
               << this->m_iNames[2] << ' '
               << this->m_iNames[3] << ' '
               << this->m_iNames[4] << " }";

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      TypeParam queried_value(0);
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

TYPED_TEST_P(NamedValueSet_tp_0, test_case2)
{
   // Add() / Get() integer types, btStringKey

   const btUnsignedInt N = sizeof(this->m_sNames) / sizeof(this->m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   // Inside a test, refer to TypeParam to get the type parameter.
   TypeParam value;

   eBasicTypes ExpectType = btUnknownType_t;
   if ( typeid(AAL::btByte) == typeid(value) ) {
      ExpectType = btByte_t;
   } else if ( typeid(AAL::bt32bitInt) == typeid(value) ) {
      ExpectType = bt32bitInt_t;
   } else if ( typeid(AAL::btUnsigned32bitInt) == typeid(value) ) {
      ExpectType = btUnsigned32bitInt_t;
   } else if ( typeid(AAL::bt64bitInt) == typeid(value) ) {
      ExpectType = bt64bitInt_t;
   } else if ( typeid(AAL::btUnsigned64bitInt) == typeid(value) ) {
      ExpectType = btUnsigned64bitInt_t;
   }
   EXPECT_NE(btUnknownType_t, ExpectType);

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_sNames[i];
      value = (TypeParam) i;

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Type(name, &t));
      EXPECT_EQ(ExpectType, t);

      EXPECT_TRUE(this->m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_sNames[i];
      value = (TypeParam) i;

      btStringKey queried_name;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name) << "names: { "
               << this->m_sNames[0] << ' '
               << this->m_sNames[1] << ' '
               << this->m_sNames[2] << ' '
               << this->m_sNames[3] << ' '
               << this->m_sNames[4] << " }";

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      TypeParam queried_value(0);
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Get(name, &queried_value));
      EXPECT_EQ(value, queried_value);
   }
}

TYPED_TEST_P(NamedValueSet_tp_0, test_case3)
{
   // Add() / Get() integer Array types, btNumberKey

   const btUnsignedInt N = sizeof(this->m_iNames) / sizeof(this->m_iNames[0]);

   btUnsignedInt i;
   btNumberKey   name = 0;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btStringKey_t;

   // Inside a test, refer to TypeParam to get the type parameter.
   TypeParam value;

   eBasicTypes ExpectType = btUnknownType_t;
   if ( typeid(AAL::btByte) == typeid(value) ) {
      ExpectType = btByteArray_t;
   } else if ( typeid(AAL::bt32bitInt) == typeid(value) ) {
      ExpectType = bt32bitIntArray_t;
   } else if ( typeid(AAL::btUnsigned32bitInt) == typeid(value) ) {
      ExpectType = btUnsigned32bitIntArray_t;
   } else if ( typeid(AAL::bt64bitInt) == typeid(value) ) {
      ExpectType = bt64bitIntArray_t;
   } else if ( typeid(AAL::btUnsigned64bitInt) == typeid(value) ) {
      ExpectType = btUnsigned64bitIntArray_t;
   }
   EXPECT_NE(btUnknownType_t, ExpectType);

   TypeParam ValArrays[5][5] =
   {
      {  0,  1,  2,  3,  4 },
      {  5,  6,  7,  8,  9 },
      { 10, 11, 12, 13, 14 },
      { 15, 16, 17, 18, 19 },
      { 20, 21, 22, 23, 24 }
   };
   const btUnsigned32bitInt NumElements = sizeof(ValArrays[0]) / sizeof(ValArrays[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_iNames[i];

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Add(name, ValArrays[i], NumElements));

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Type(name, &t));
      EXPECT_EQ(ExpectType, t);

      EXPECT_TRUE(this->m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_iNames[i];

      btNumberKey queried_name = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetName(i, &queried_name));
      EXPECT_EQ(name, queried_name) << "names: { "
               << this->m_iNames[0] << ' '
               << this->m_iNames[1] << ' '
               << this->m_iNames[2] << ' '
               << this->m_iNames[3] << ' '
               << this->m_iNames[4] << " }";

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btNumberKey_t, k);

      TypeParam *queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Get(name, &queried_value));

      EXPECT_EQ(ValArrays[i][0], queried_value[0]);
      EXPECT_EQ(ValArrays[i][1], queried_value[1]);
      EXPECT_EQ(ValArrays[i][2], queried_value[2]);
      EXPECT_EQ(ValArrays[i][3], queried_value[3]);
      EXPECT_EQ(ValArrays[i][4], queried_value[4]);
   }
}

TYPED_TEST_P(NamedValueSet_tp_0, test_case4)
{
   // Add() / Get() integer Array types, btStringKey

   const btUnsignedInt N = sizeof(this->m_sNames) / sizeof(this->m_sNames[0]);

   btUnsignedInt i;
   btStringKey   name;
   btWSSize      sz   = 0;
   eBasicTypes   t    = btUnknownType_t;
   btUnsignedInt n    = 99;
   eNameTypes    k    = btNumberKey_t;

   // Inside a test, refer to TypeParam to get the type parameter.
   TypeParam value;

   eBasicTypes ExpectType = btUnknownType_t;
   if ( typeid(AAL::btByte) == typeid(value) ) {
      ExpectType = btByteArray_t;
   } else if ( typeid(AAL::bt32bitInt) == typeid(value) ) {
      ExpectType = bt32bitIntArray_t;
   } else if ( typeid(AAL::btUnsigned32bitInt) == typeid(value) ) {
      ExpectType = btUnsigned32bitIntArray_t;
   } else if ( typeid(AAL::bt64bitInt) == typeid(value) ) {
      ExpectType = bt64bitIntArray_t;
   } else if ( typeid(AAL::btUnsigned64bitInt) == typeid(value) ) {
      ExpectType = btUnsigned64bitIntArray_t;
   }
   EXPECT_NE(btUnknownType_t, ExpectType);

   TypeParam ValArrays[5][5] =
   {
      {  0,  1,  2,  3,  4 },
      {  5,  6,  7,  8,  9 },
      { 10, 11, 12, 13, 14 },
      { 15, 16, 17, 18, 19 },
      { 20, 21, 22, 23, 24 }
   };
   const btUnsigned32bitInt NumElements = sizeof(ValArrays[0]) / sizeof(ValArrays[0][0]);

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_sNames[i];

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Add(name, ValArrays[i], NumElements));

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetSize(name, &sz));
      EXPECT_EQ((btWSSize)NumElements, sz);

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Type(name, &t));
      EXPECT_EQ(ExpectType, t);

      EXPECT_TRUE(this->m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   for ( i = 0 ; i < N ; ++i ) {
      name  = this->m_sNames[i];

      btStringKey queried_name;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetName(i, &queried_name));
      EXPECT_STREQ(name, queried_name) << "names: { "
               << this->m_sNames[0] << ' '
               << this->m_sNames[1] << ' '
               << this->m_sNames[2] << ' '
               << this->m_sNames[3] << ' '
               << this->m_sNames[4] << " }";

      EXPECT_EQ(ENamedValuesOK, this->m_NVS.GetNameType(i, &k));
      EXPECT_EQ(btStringKey_t, k);

      TypeParam *queried_value = NULL;
      EXPECT_EQ(ENamedValuesOK, this->m_NVS.Get(name, &queried_value));

      EXPECT_EQ(ValArrays[i][0], queried_value[0]);
      EXPECT_EQ(ValArrays[i][1], queried_value[1]);
      EXPECT_EQ(ValArrays[i][2], queried_value[2]);
      EXPECT_EQ(ValArrays[i][3], queried_value[3]);
      EXPECT_EQ(ValArrays[i][4], queried_value[4]);
   }
}

REGISTER_TYPED_TEST_CASE_P(NamedValueSet_tp_0,
                           test_case1,
                           test_case2,
                           test_case3,
                           test_case4);

typedef ::testing::Types< btByte,
                          bt32bitInt,
                          btUnsigned32bitInt,
                          bt64bitInt,
                          btUnsigned64bitInt > NamedValueSet_tp_0_Types;
INSTANTIATE_TYPED_TEST_CASE_P(My, NamedValueSet_tp_0, NamedValueSet_tp_0_Types);

////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class NamedValueSet_btNamedValueSet_f : public ::testing::Test
{
protected:
   NamedValueSet_btNamedValueSet_f() : m_R(0) {}
   virtual ~NamedValueSet_btNamedValueSet_f() {}

   virtual void SetUp()
   {
      btUnsignedInt i;
      btUnsignedInt N = sizeof(m_iNames) / sizeof(m_iNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         m_iNames[i] = ::UniqueIntRand(m_iNames, i, (btNumberKey)0, &m_R);
      }
      ::MySort(m_iNames, N);

      m_sNames[0] = "four";
      m_sNames[1] = "one";
      m_sNames[2] = "three";
      m_sNames[3] = "two";
      m_sNames[4] = "zero";

      btWSSize      sz = 0;
      eBasicTypes   t  = btUnknownType_t;
      btUnsignedInt n  = 99;
      eNameTypes    k  = btStringKey_t;

      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);

      // ASSERT_EQ(ENamedValuesBadType, m_NVS.GetName(0, &name));

      // ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetNameType(0, &k));
      // ASSERT_EQ(btStringKey_t, k);

      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_iNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_iNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_iNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_iNames[i]));
      }

      N = sizeof(m_sNames) / sizeof(m_sNames[0]);
      for ( i = 0 ; i < N ; ++i ) {
         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Delete(m_sNames[i]));

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.GetSize(m_sNames[i], &sz));
         ASSERT_EQ(0, sz);

         ASSERT_EQ(ENamedValuesNameNotFound, m_NVS.Type(m_sNames[i], &t));
         ASSERT_EQ(btUnknownType_t, t);

         ASSERT_FALSE(m_NVS.Has(m_sNames[i]));
      }
   }
   virtual void TearDown()
   {
      btUnsignedInt i;
      for ( i = 0 ; i < sizeof(m_iNames) / sizeof(m_iNames[0]) ; ++i ) {
         m_NVS.Delete(m_iNames[i]);
      }

      for ( i = 0 ; i < sizeof(m_sNames) / sizeof(m_sNames[0]) ; ++i ) {
         m_NVS.Delete(m_sNames[i]);
      }

      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   void One(INamedValueSet *p) const
   {
      p->Add((btNumberKey)0, false);
      p->Add("key",          (btByte)3);
   }
   void VerifyOne(const INamedValueSet *p) const
   {
      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, p->GetNumNames(&n));
      ASSERT_EQ(2, n);

      btNumberKey ikey = 99;
      btStringKey skey = NULL;

      eNameTypes k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(0, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(0, &ikey));
      EXPECT_EQ(0, ikey);

      eBasicTypes t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btBool_t, t);

      btBool b = true;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &b));
      EXPECT_FALSE(b);


      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(1, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(1, &skey));
      EXPECT_STREQ("key", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btByte_t, t);

      btByte by = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &by));
      EXPECT_EQ(3, by);
   }

   void Two(INamedValueSet *p) const
   {
      p->Add((btNumberKey)1, true);
      p->Add((btNumberKey)2, (btByte)5);
      p->Add((btNumberKey)3, (bt32bitInt)6);
      p->Add((btNumberKey)4, (btUnsigned32bitInt)7);
      p->Add((btNumberKey)5, (bt64bitInt)8);
      p->Add((btNumberKey)6, (btUnsigned64bitInt)9);
      p->Add((btNumberKey)7, (btFloat)10.0);
      p->Add((btNumberKey)8, "value");
      p->Add((btNumberKey)9, (btObjectType)NULL);

      p->Add("one",   true);
      p->Add("two",   (btByte)11);
      p->Add("three", (bt32bitInt)12);
      p->Add("four",  (btUnsigned32bitInt)13);
      p->Add("five",  (bt64bitInt)14);
      p->Add("six",   (btUnsigned64bitInt)15);
      p->Add("seven", (btFloat)16.0);
      p->Add("eight", "val2");
      p->Add("nine",  (btObjectType)1);
   }
   void VerifyTwo(const INamedValueSet *p) const
   {
      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, p->GetNumNames(&n));
      ASSERT_EQ(18, n);

      btNumberKey ikey = 99;
      btStringKey skey = NULL;

      //////////
      // 0
      eNameTypes k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(0, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(0, &ikey));
      EXPECT_EQ(1, ikey);

      eBasicTypes t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btBool_t, t);

      btBool b = false;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &b));
      EXPECT_TRUE(b);

      //////////
      // 1
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(1, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(1, &ikey));
      EXPECT_EQ(2, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btByte_t, t);

      btByte by = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &by));
      EXPECT_EQ(5, by);

      //////////
      // 2
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(2, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(2, &ikey));
      EXPECT_EQ(3, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(bt32bitInt_t, t);

      bt32bitInt s32 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &s32));
      EXPECT_EQ(6, s32);

      //////////
      // 3
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(3, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(3, &ikey));
      EXPECT_EQ(4, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btUnsigned32bitInt_t, t);

      btUnsigned32bitInt u32 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &u32));
      EXPECT_EQ(7, u32);

      //////////
      // 4
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(4, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(4, &ikey));
      EXPECT_EQ(5, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(bt64bitInt_t, t);

      bt64bitInt s64 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &s64));
      EXPECT_EQ(8, s64);

      //////////
      // 5
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(5, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(5, &ikey));
      EXPECT_EQ(6, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btUnsigned64bitInt_t, t);

      btUnsigned64bitInt u64 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &u64));
      EXPECT_EQ(9, u64);

      //////////
      // 6
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(6, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(6, &ikey));
      EXPECT_EQ(7, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btFloat_t, t);

      btFloat f = 0.0;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &f));
      EXPECT_EQ(10.0, f);

      //////////
      // 7
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(7, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(7, &ikey));
      EXPECT_EQ(8, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btString_t, t);

      btcString str = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &str));
      EXPECT_STREQ("value", str);

      //////////
      // 8
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(8, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(8, &ikey));
      EXPECT_EQ(9, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btObjectType_t, t);

      btObjectType o = (btObjectType)3;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &o));
      EXPECT_NULL(o);


      //////////
      // 9
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(9, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(9, &skey));
      EXPECT_STREQ("eight", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btString_t, t);

      str = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &str));
      EXPECT_STREQ("val2", str);

      //////////
      // 10
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(10, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(10, &skey));
      EXPECT_STREQ("five", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(bt64bitInt_t, t);

      s64 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &s64));
      EXPECT_EQ(14, s64);

      //////////
      // 11
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(11, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(11, &skey));
      EXPECT_STREQ("four", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btUnsigned32bitInt_t, t);

      u32 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &u32));
      EXPECT_EQ(13, u32);

      //////////
      // 12
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(12, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(12, &skey));
      EXPECT_STREQ("nine", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btObjectType_t, t);

      o = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &o));
      EXPECT_EQ((btObjectType)1, o);

      //////////
      // 13
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(13, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(13, &skey));
      EXPECT_STREQ("one", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btBool_t, t);

      b = false;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &b));
      EXPECT_TRUE(b);

      //////////
      // 14
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(14, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(14, &skey));
      EXPECT_STREQ("seven", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btFloat_t, t);

      f = 0.0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &f));
      EXPECT_EQ(16.0, f);

      //////////
      // 15
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(15, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(15, &skey));
      EXPECT_STREQ("six", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btUnsigned64bitInt_t, t);

      u64 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &u64));
      EXPECT_EQ(15, u64);

      //////////
      // 16
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(16, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(16, &skey));
      EXPECT_STREQ("three", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(bt32bitInt_t, t);

      s32 = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &s32));
      EXPECT_EQ(12, s32);

      //////////
      // 17
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(17, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(17, &skey));
      EXPECT_STREQ("two", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(skey, &t));
      EXPECT_EQ(btByte_t, t);

      by = 0;
      EXPECT_EQ(ENamedValuesOK, p->Get(skey, &by));
      EXPECT_EQ(11, by);
   }

   void Three(INamedValueSet *p) const
   {
      btByte bA[] = { 3, 2, 1 };
      p->Add((btNumberKey)0, bA, sizeof(bA) / sizeof(bA[0]));

      bt32bitInt s32A[] = { 4, 5, 6 };
      p->Add((btNumberKey)1, s32A, sizeof(s32A) / sizeof(s32A[0]));

      btUnsigned32bitInt u32A[] = { 7, 8, 9 };
      p->Add((btNumberKey)2, u32A, sizeof(u32A) / sizeof(u32A[0]));

      bt64bitInt s64A[] = { 10, 11, 12 };
      p->Add((btNumberKey)3, s64A, sizeof(s64A) / sizeof(s64A[0]));

      btUnsigned64bitInt u64A[] = { 13, 14, 15 };
      p->Add((btNumberKey)4, u64A, sizeof(u64A) / sizeof(u64A[0]));

      btFloat fA[] = { 16.0, 17.0, 18.0 };
      p->Add((btNumberKey)5, fA, sizeof(fA) / sizeof(fA[0]));

      btcString strA[] = { "nineteen", "twenty", "twenty-one" };
      p->Add((btNumberKey)6, const_cast<btStringArray>(strA), sizeof(strA) / sizeof(strA[0]));

      btObjectType oA[] = { (btObjectType)3, (btObjectType)2, (btObjectType)5 };
      p->Add((btNumberKey)7, oA, sizeof(oA) / sizeof(oA[0]));
   }
   void VerifyThree(const INamedValueSet *p) const
   {
      btUnsignedInt n = 99;
      ASSERT_EQ(ENamedValuesOK, p->GetNumNames(&n));
      ASSERT_EQ(8, n);

      btNumberKey ikey = 99;
      btStringKey skey = NULL;

      //////////
      // 0
      eNameTypes k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(0, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(0, &ikey));
      EXPECT_EQ(0, ikey);

      eBasicTypes t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btByteArray_t, t);

      btByteArray by = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &by));
      EXPECT_EQ(3, *(by + 0));
      EXPECT_EQ(2, *(by + 1));
      EXPECT_EQ(1, *(by + 2));

      //////////
      // 1
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(1, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(1, &ikey));
      EXPECT_EQ(1, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(bt32bitIntArray_t, t);

      bt32bitIntArray s32 = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &s32));
      EXPECT_EQ(4, *(s32 + 0));
      EXPECT_EQ(5, *(s32 + 1));
      EXPECT_EQ(6, *(s32 + 2));

      //////////
      // 2
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(2, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(2, &ikey));
      EXPECT_EQ(2, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btUnsigned32bitIntArray_t, t);

      btUnsigned32bitIntArray u32 = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &u32));
      EXPECT_EQ(7, *(u32 + 0));
      EXPECT_EQ(8, *(u32 + 1));
      EXPECT_EQ(9, *(u32 + 2));

      //////////
      // 3
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(3, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(3, &ikey));
      EXPECT_EQ(3, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(bt64bitIntArray_t, t);

      bt64bitIntArray s64 = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &s64));
      EXPECT_EQ(10, *(s64 + 0));
      EXPECT_EQ(11, *(s64 + 1));
      EXPECT_EQ(12, *(s64 + 2));

      //////////
      // 4
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(4, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(4, &ikey));
      EXPECT_EQ(4, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btUnsigned64bitIntArray_t, t);

      btUnsigned64bitIntArray u64 = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &u64));
      EXPECT_EQ(13, *(u64 + 0));
      EXPECT_EQ(14, *(u64 + 1));
      EXPECT_EQ(15, *(u64 + 2));

      //////////
      // 5
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(5, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(5, &ikey));
      EXPECT_EQ(5, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btFloatArray_t, t);

      btFloatArray f = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &f));
      EXPECT_EQ(16.0, *(f + 0));
      EXPECT_EQ(17.0, *(f + 1));
      EXPECT_EQ(18.0, *(f + 2));

      //////////
      // 6
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(6, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(6, &ikey));
      EXPECT_EQ(6, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btStringArray_t, t);

      btStringArray str = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &str));
      EXPECT_STREQ("nineteen",   *(str + 0));
      EXPECT_STREQ("twenty",     *(str + 1));
      EXPECT_STREQ("twenty-one", *(str + 2));

      //////////
      // 7
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, p->GetNameType(7, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, p->GetName(7, &ikey));
      EXPECT_EQ(7, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, p->Type(ikey, &t));
      EXPECT_EQ(btObjectArray_t, t);

      btObjectArray o = NULL;
      EXPECT_EQ(ENamedValuesOK, p->Get(ikey, &o));
      EXPECT_EQ((btObjectType)3, *(o + 0));
      EXPECT_EQ((btObjectType)2, *(o + 1));
      EXPECT_EQ((btObjectType)5, *(o + 2));
   }

   btUnsigned32bitInt m_R;
   NamedValueSet      m_NVS;
   btNumberKey        m_iNames[5];
   btStringKey        m_sNames[5];
   INamedValueSet    *m_Values[5];
};

TEST_F(NamedValueSet_btNamedValueSet_f, test_case1)
{
   // Add() / Get() INamedValueSet *, btNumberKey & btStringKey

   btUnsignedInt       i;
   const btUnsignedInt N    = sizeof(m_iNames) / sizeof(m_iNames[0]);
   btNumberKey         name = 0;
   btWSSize            sz   = 0;
   eBasicTypes         t    = btUnknownType_t;
   btUnsignedInt       n    = 99;
   eNameTypes          k    = btStringKey_t;

   NamedValueSet zero;

   NamedValueSet one;
   One(&one);

   NamedValueSet two;
   Two(&two);

   NamedValueSet three;
   Three(&three);

   NamedValueSet four;

   four.Add((btNumberKey)0, &zero);
   four.Add("one",          &one);
   four.Add((btNumberKey)2, &two);
   four.Add("three",        &three);

   m_Values[0] = &zero;
   m_Values[1] = &one;
   m_Values[2] = &two;
   m_Values[3] = &three;
   m_Values[4] = &four;

   INamedValueSet *value;

   for ( i = 0 ; i < N ; ++i ) {
      name  = m_iNames[i];
      value = m_Values[i];

      EXPECT_EQ(ENamedValuesOK, m_NVS.Add(name, value));

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetSize(name, &sz));
      EXPECT_EQ(1, sz);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Type(name, &t));
      EXPECT_EQ(btNamedValueSet_t, t);

      EXPECT_TRUE(m_NVS.Has(name));

      n = 99;
      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNumNames(&n));
      EXPECT_EQ(n, i+1);
   }

   btNumberKey           queried_name;
   INamedValueSet const *queried_value;

   {
      queried_name = 99;
      k = btStringKey_t;
      queried_value = NULL;
      name = m_iNames[0];

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(0, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(0, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      ASSERT_NONNULL(queried_value);
      ASSERT_NE(&zero, queried_value);

      // zero is empty
      n = 99;
      ASSERT_EQ(ENamedValuesOK, queried_value->GetNumNames(&n));
      ASSERT_EQ(0, n);
   }

   {
      queried_name = 99;
      k = btStringKey_t;
      queried_value = NULL;
      name = m_iNames[1];

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(1, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(1, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      ASSERT_NONNULL(queried_value);
      ASSERT_NE(&one, queried_value);

      VerifyOne(queried_value);
   }

   {
      queried_name = 99;
      k = btStringKey_t;
      queried_value = NULL;
      name = m_iNames[2];

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(2, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(2, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      ASSERT_NONNULL(queried_value);
      ASSERT_NE(&two, queried_value);

      VerifyTwo(queried_value);
   }

   {
      queried_name = 99;
      k = btStringKey_t;
      queried_value = NULL;
      name = m_iNames[3];

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(3, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(3, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      ASSERT_NONNULL(queried_value);
      ASSERT_NE(&three, queried_value);

      VerifyThree(queried_value);
   }

   {
      queried_name = 99;
      k = btStringKey_t;
      queried_value = NULL;
      name = m_iNames[4];

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetName(4, &queried_name));
      EXPECT_EQ(name, queried_name);

      EXPECT_EQ(ENamedValuesOK, m_NVS.GetNameType(4, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, m_NVS.Get(name, &queried_value));
      ASSERT_NONNULL(queried_value);
      ASSERT_NE(&four, queried_value);

      // four.Add((btNumberKey)0, &zero);
      // four.Add((btNumberKey)2, &two);
      // four.Add("one",          &one);
      // four.Add("three",        &three);

      n = 99;
      ASSERT_EQ(ENamedValuesOK, queried_value->GetNumNames(&n));
      ASSERT_EQ(4, n);

      btNumberKey ikey = 99;
      btStringKey skey = NULL;

      ///////
      // 0
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->GetNameType(0, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, queried_value->GetName(0, &ikey));
      EXPECT_EQ(0, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->Type(ikey, &t));
      EXPECT_EQ(btNamedValueSet_t, t);

      INamedValueSet const *nvs = NULL;
      EXPECT_EQ(ENamedValuesOK, queried_value->Get(ikey, &nvs));
      ASSERT_NONNULL(nvs);
      ASSERT_NE(&zero, nvs);

      n = 99;
      ASSERT_EQ(ENamedValuesOK, nvs->GetNumNames(&n));
      ASSERT_EQ(0, n);

      ///////
      // 1
      k = btStringKey_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->GetNameType(1, &k));
      EXPECT_EQ(btNumberKey_t, k);

      EXPECT_EQ(ENamedValuesOK, queried_value->GetName(1, &ikey));
      EXPECT_EQ(2, ikey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->Type(ikey, &t));
      EXPECT_EQ(btNamedValueSet_t, t);

      nvs = NULL;
      EXPECT_EQ(ENamedValuesOK, queried_value->Get(ikey, &nvs));
      ASSERT_NONNULL(nvs);
      ASSERT_NE(&two, nvs);

      VerifyTwo(nvs);

      ///////
      // 2
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->GetNameType(2, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, queried_value->GetName(2, &skey));
      EXPECT_STREQ("one", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->Type(skey, &t));
      EXPECT_EQ(btNamedValueSet_t, t);

      nvs = NULL;
      EXPECT_EQ(ENamedValuesOK, queried_value->Get(skey, &nvs));
      ASSERT_NONNULL(nvs);
      ASSERT_NE(&one, nvs);

      VerifyOne(nvs);

      ///////
      // 3
      k = btNumberKey_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->GetNameType(3, &k));
      EXPECT_EQ(btStringKey_t, k);

      EXPECT_EQ(ENamedValuesOK, queried_value->GetName(3, &skey));
      EXPECT_STREQ("three", skey);

      t = btUnknownType_t;
      EXPECT_EQ(ENamedValuesOK, queried_value->Type(skey, &t));
      EXPECT_EQ(btNamedValueSet_t, t);

      nvs = NULL;
      EXPECT_EQ(ENamedValuesOK, queried_value->Get(skey, &nvs));
      ASSERT_NONNULL(nvs);
      ASSERT_NE(&three, nvs);

      VerifyThree(nvs);
   }

   for ( i = 0 ; i < sizeof(m_Values) / sizeof(m_Values[0]) ; ++i ) {
      m_Values[i] = NULL;
   }
}

////////////////////////////////////////////////////////////////////////////////
















////////////////////////////////////////////////////////////////////////////////

// Simple test fixture
class AAS_CValue_f : public ::testing::Test
{
protected:
   AAS_CValue_f() {}
   virtual void SetUp()
   {

   }
   virtual void TearDown()
   {
   }

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



