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
