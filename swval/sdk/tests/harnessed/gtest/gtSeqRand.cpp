// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "gtSeqRand.h"

////////////////////////////////////////////////////////////////////////////////

CbtNumberKeySequencer::CbtNumberKeySequencer() :
   TValueSequencer< btNumberKey, TIntCompare<btNumberKey> >(CbtNumberKeySequencer::sm_Vals,
                                                            CbtNumberKeySequencer::sm_Count)
{}

const btNumberKey CbtNumberKeySequencer::sm_Vals[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
const btUnsigned32bitInt CbtNumberKeySequencer::sm_Count =
   sizeof(CbtNumberKeySequencer::sm_Vals) / sizeof(CbtNumberKeySequencer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtStringKeySequencer::CbtStringKeySequencer() :
   TValueSequencer< btStringKey, TStrCompare<btStringKey> >(CbtStringKeySequencer::sm_Vals,
                                                            CbtStringKeySequencer::sm_Count)
{}

const btStringKey CbtStringKeySequencer::sm_Vals[] = {
   "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15",
   "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"
};
const btUnsigned32bitInt CbtStringKeySequencer::sm_Count =
   sizeof(CbtStringKeySequencer::sm_Vals) / sizeof(CbtStringKeySequencer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtBoolRandomizer::CbtBoolRandomizer() :
   TValueRandomizer< btBool, TIntCompare<btBool> >(CbtBoolRandomizer::sm_Vals,
                                                   CbtBoolRandomizer::sm_Count)
{}

const btBool CbtBoolRandomizer::sm_Vals[] = { false, true };
const btUnsigned32bitInt CbtBoolRandomizer::sm_Count =
   sizeof(CbtBoolRandomizer::sm_Vals) / sizeof(CbtBoolRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtByteRandomizer::CbtByteRandomizer() :
   TValueRandomizer< btByte, TIntCompare<btByte> >(CbtByteRandomizer::sm_Vals,
                                                   CbtByteRandomizer::sm_Count)
{}

const btByte CbtByteRandomizer::sm_Vals[] = {
   std::numeric_limits<btByte>::min(),
   std::numeric_limits<btByte>::min() + 1,
   0,
   std::numeric_limits<btByte>::max() - 1,
   std::numeric_limits<btByte>::max()
};
const btUnsigned32bitInt CbtByteRandomizer::sm_Count =
   sizeof(CbtByteRandomizer::sm_Vals) / sizeof(CbtByteRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

Cbt32bitIntRandomizer::Cbt32bitIntRandomizer() :
   TValueRandomizer< bt32bitInt, TIntCompare<bt32bitInt> >(Cbt32bitIntRandomizer::sm_Vals,
                                                           Cbt32bitIntRandomizer::sm_Count)
{}

const bt32bitInt Cbt32bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<bt32bitInt>::min(),
   std::numeric_limits<bt32bitInt>::min() + 1,
   0,
   std::numeric_limits<bt32bitInt>::max() - 1,
   std::numeric_limits<bt32bitInt>::max()
};
const btUnsigned32bitInt Cbt32bitIntRandomizer::sm_Count =
   sizeof(Cbt32bitIntRandomizer::sm_Vals) / sizeof(Cbt32bitIntRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtUnsigned32bitIntRandomizer::CbtUnsigned32bitIntRandomizer() :
   TValueRandomizer< btUnsigned32bitInt, TIntCompare<btUnsigned32bitInt> >(CbtUnsigned32bitIntRandomizer::sm_Vals,
                                                                           CbtUnsigned32bitIntRandomizer::sm_Count)
{}

const btUnsigned32bitInt CbtUnsigned32bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<btUnsigned32bitInt>::min(),
   std::numeric_limits<btUnsigned32bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned32bitInt>::max() - 1,
   std::numeric_limits<btUnsigned32bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned32bitIntRandomizer::sm_Count =
   sizeof(CbtUnsigned32bitIntRandomizer::sm_Vals) / sizeof(CbtUnsigned32bitIntRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

Cbt64bitIntRandomizer::Cbt64bitIntRandomizer() :
   TValueRandomizer< bt64bitInt, TIntCompare<bt64bitInt> >(Cbt64bitIntRandomizer::sm_Vals,
                                                           Cbt64bitIntRandomizer::sm_Count)
{}

const bt64bitInt Cbt64bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<bt64bitInt>::min(),
   std::numeric_limits<bt64bitInt>::min() + 1,
   0,
   std::numeric_limits<bt64bitInt>::max() - 1,
   std::numeric_limits<bt64bitInt>::max()
};
const btUnsigned64bitInt Cbt64bitIntRandomizer::sm_Count =
   sizeof(Cbt64bitIntRandomizer::sm_Vals) / sizeof(Cbt64bitIntRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtUnsigned64bitIntRandomizer::CbtUnsigned64bitIntRandomizer() :
   TValueRandomizer< btUnsigned64bitInt, TIntCompare<btUnsigned64bitInt> >(CbtUnsigned64bitIntRandomizer::sm_Vals,
                                                                           CbtUnsigned64bitIntRandomizer::sm_Count)
{}

const btUnsigned64bitInt CbtUnsigned64bitIntRandomizer::sm_Vals[] = {
   std::numeric_limits<btUnsigned64bitInt>::min(),
   std::numeric_limits<btUnsigned64bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned64bitInt>::max() - 1,
   std::numeric_limits<btUnsigned64bitInt>::max()
};
const btUnsigned64bitInt CbtUnsigned64bitIntRandomizer::sm_Count =
   sizeof(CbtUnsigned64bitIntRandomizer::sm_Vals) / sizeof(CbtUnsigned64bitIntRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtFloatRandomizer::CbtFloatRandomizer() :
   TValueRandomizer< btFloat, TFltCompare<btFloat> >(CbtFloatRandomizer::sm_Vals,
                                                     CbtFloatRandomizer::sm_Count)
{}

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

CbtcStringRandomizer::CbtcStringRandomizer() :
   TValueRandomizer< btcString, TStrCompare<btcString> >(CbtcStringRandomizer::sm_Vals,
                                                         CbtcStringRandomizer::sm_Count)
{}

const btcString CbtcStringRandomizer::sm_Vals[] = {
   "a",
   "b",
   "c",
   "d",
   "e"
};
const btUnsigned32bitInt CbtcStringRandomizer::sm_Count =
   sizeof(CbtcStringRandomizer::sm_Vals) / sizeof(CbtcStringRandomizer::sm_Vals[0]);


CbtcStringRandomizer_WithZeroLengthStrings::CbtcStringRandomizer_WithZeroLengthStrings() :
   TValueRandomizer< btcString, TStrCompare<btcString> >(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals,
                                                         CbtcStringRandomizer_WithZeroLengthStrings::sm_Count)
{}

const btcString CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals[] = {
   "",
   "not",
   "",
   "good",
   ""
};
const btUnsigned32bitInt CbtcStringRandomizer_WithZeroLengthStrings::sm_Count =
   sizeof(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals) / sizeof(CbtcStringRandomizer_WithZeroLengthStrings::sm_Vals[0]);


CbtcStringRandomizer_WithLongStrings::CbtcStringRandomizer_WithLongStrings() :
   TValueRandomizer< btcString, TStrCompare<btcString> >(CbtcStringRandomizer_WithLongStrings::sm_Vals,
                                                         CbtcStringRandomizer_WithLongStrings::sm_Count)
{}

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

CbtObjectTypeRandomizer::CbtObjectTypeRandomizer() :
   TValueRandomizer< btObjectType, TIntCompare<btObjectType> >(CbtObjectTypeRandomizer::sm_Vals,
                                                               CbtObjectTypeRandomizer::sm_Count)
{}

const btObjectType CbtObjectTypeRandomizer::sm_Vals[] = {
   (btObjectType)NULL,
   (btObjectType)1,
   (btObjectType)2,
   (btObjectType)3,
   (btObjectType)4,
};
const btUnsigned32bitInt CbtObjectTypeRandomizer::sm_Count =
   sizeof(CbtObjectTypeRandomizer::sm_Vals) / sizeof(CbtObjectTypeRandomizer::sm_Vals[0]);

////////////////////////////////////////////////////////////////////////////////

CbtByteArrayProvider::CbtByteArrayProvider() :
   TArrayProvider< btByte, TIntCompare<btByte>, btByteArray >(const_cast<const btByteArray>(CbtByteArrayProvider::sm_Array),
                                                              CbtByteArrayProvider::sm_Count)
{}

const btByte CbtByteArrayProvider::sm_Array[] = {
   std::numeric_limits<btByte>::min(),
   std::numeric_limits<btByte>::min() + 1,
   0,
   std::numeric_limits<btByte>::max() - 1,
   std::numeric_limits<btByte>::max()
};
const btUnsigned32bitInt CbtByteArrayProvider::sm_Count =
   sizeof(CbtByteArrayProvider::sm_Array) / sizeof(CbtByteArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

Cbt32bitIntArrayProvider::Cbt32bitIntArrayProvider() :
   TArrayProvider< bt32bitInt, TIntCompare<bt32bitInt>, bt32bitIntArray >(
         const_cast<const bt32bitIntArray>(Cbt32bitIntArrayProvider::sm_Array),
         Cbt32bitIntArrayProvider::sm_Count)
{}

const bt32bitInt Cbt32bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<bt32bitInt>::min(),
   std::numeric_limits<bt32bitInt>::min() + 1,
   0,
   std::numeric_limits<bt32bitInt>::max() - 1,
   std::numeric_limits<bt32bitInt>::max()
};
const btUnsigned32bitInt Cbt32bitIntArrayProvider::sm_Count =
   sizeof(Cbt32bitIntArrayProvider::sm_Array) / sizeof(Cbt32bitIntArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

CbtUnsigned32bitIntArrayProvider::CbtUnsigned32bitIntArrayProvider() :
   TArrayProvider< btUnsigned32bitInt, TIntCompare<btUnsigned32bitInt>, btUnsigned32bitIntArray >(
               const_cast<const btUnsigned32bitIntArray>(CbtUnsigned32bitIntArrayProvider::sm_Array),
               CbtUnsigned32bitIntArrayProvider::sm_Count)
{}

const btUnsigned32bitInt CbtUnsigned32bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<btUnsigned32bitInt>::min(),
   std::numeric_limits<btUnsigned32bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned32bitInt>::max() - 1,
   std::numeric_limits<btUnsigned32bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned32bitIntArrayProvider::sm_Count =
   sizeof(CbtUnsigned32bitIntArrayProvider::sm_Array) / sizeof(CbtUnsigned32bitIntArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

Cbt64bitIntArrayProvider::Cbt64bitIntArrayProvider() :
   TArrayProvider< bt64bitInt, TIntCompare<bt64bitInt>, bt64bitIntArray >(
            const_cast<const bt64bitIntArray>(Cbt64bitIntArrayProvider::sm_Array),
            Cbt64bitIntArrayProvider::sm_Count)
{}

const bt64bitInt Cbt64bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<bt64bitInt>::min(),
   std::numeric_limits<bt64bitInt>::min() + 1,
   0,
   std::numeric_limits<bt64bitInt>::max() - 1,
   std::numeric_limits<bt64bitInt>::max()
};
const btUnsigned32bitInt Cbt64bitIntArrayProvider::sm_Count =
   sizeof(Cbt64bitIntArrayProvider::sm_Array) / sizeof(Cbt64bitIntArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

CbtUnsigned64bitIntArrayProvider::CbtUnsigned64bitIntArrayProvider() :
   TArrayProvider< btUnsigned64bitInt, TIntCompare<btUnsigned64bitInt>, btUnsigned64bitIntArray >(
            const_cast<const btUnsigned64bitIntArray>(CbtUnsigned64bitIntArrayProvider::sm_Array),
            CbtUnsigned64bitIntArrayProvider::sm_Count)
{}

const btUnsigned64bitInt CbtUnsigned64bitIntArrayProvider::sm_Array[] = {
   std::numeric_limits<btUnsigned64bitInt>::min(),
   std::numeric_limits<btUnsigned64bitInt>::min() + 1,
   0,
   std::numeric_limits<btUnsigned64bitInt>::max() - 1,
   std::numeric_limits<btUnsigned64bitInt>::max()
};
const btUnsigned32bitInt CbtUnsigned64bitIntArrayProvider::sm_Count =
   sizeof(CbtUnsigned64bitIntArrayProvider::sm_Array) / sizeof(CbtUnsigned64bitIntArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

CbtFloatArrayProvider::CbtFloatArrayProvider() :
   TArrayProvider< btFloat, TFltCompare<btFloat>, btFloatArray >(
            const_cast<const btFloatArray>(CbtFloatArrayProvider::sm_Array),
            CbtFloatArrayProvider::sm_Count)
{}

const btFloat CbtFloatArrayProvider::sm_Array[] = {
   std::numeric_limits<btFloat>::min(),
   std::numeric_limits<btFloat>::min() + 1.0,
   0.0,
   std::numeric_limits<btFloat>::max() - 1.0,
   std::numeric_limits<btFloat>::max()
};
const btUnsigned32bitInt CbtFloatArrayProvider::sm_Count =
   sizeof(CbtFloatArrayProvider::sm_Array) / sizeof(CbtFloatArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

CbtStringArrayProvider::CbtStringArrayProvider() :
   TArrayProvider< btString, TStrCompare<btString>, btStringArray >(
            const_cast<const btStringArray>(CbtStringArrayProvider::sm_Array),
            CbtStringArrayProvider::sm_Count)
{}

const btString CbtStringArrayProvider::sm_Array[] = {
   const_cast<const btString>("a"),
   const_cast<const btString>("b"),
   const_cast<const btString>("c"),
   const_cast<const btString>("d"),
   const_cast<const btString>("e")
};
const btUnsigned32bitInt CbtStringArrayProvider::sm_Count =
   sizeof(CbtStringArrayProvider::sm_Array) / sizeof(CbtStringArrayProvider::sm_Array[0]);


CbtStringArrayProvider_WithZeroLengthStrings::CbtStringArrayProvider_WithZeroLengthStrings() :
   TArrayProvider< btString, TStrCompare<btString>, btStringArray >(
            const_cast<const btStringArray>(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array),
            CbtStringArrayProvider_WithZeroLengthStrings::sm_Count)
{}

const btString CbtStringArrayProvider_WithZeroLengthStrings::sm_Array[] = {
   const_cast<const btString>(""),
   const_cast<const btString>("break"),
   const_cast<const btString>(""),
   const_cast<const btString>("stuff"),
   const_cast<const btString>("")
};
const btUnsigned32bitInt CbtStringArrayProvider_WithZeroLengthStrings::sm_Count =
   sizeof(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array) /
      sizeof(CbtStringArrayProvider_WithZeroLengthStrings::sm_Array[0]);


CbtStringArrayProvider_WithLongStrings::CbtStringArrayProvider_WithLongStrings() :
   TArrayProvider< btString, TStrCompare<btString>, btStringArray >(
            const_cast<const btStringArray>(CbtStringArrayProvider_WithLongStrings::sm_Array),
            CbtStringArrayProvider_WithLongStrings::sm_Count)
{}

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
   sizeof(CbtStringArrayProvider_WithLongStrings::sm_Array) /
      sizeof(CbtStringArrayProvider_WithLongStrings::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

CbtObjectArrayProvider::CbtObjectArrayProvider() :
   TArrayProvider< btObjectType, TIntCompare<btObjectType>, btObjectArray >(
            const_cast<const btObjectArray>(CbtObjectArrayProvider::sm_Array),
            CbtObjectArrayProvider::sm_Count)
{}

const btObjectType CbtObjectArrayProvider::sm_Array[] = {
   (btObjectType)NULL,
   (btObjectType)1,
   (btObjectType)2,
   (btObjectType)3,
   (btObjectType)4,
};
const btUnsigned32bitInt CbtObjectArrayProvider::sm_Count =
   sizeof(CbtObjectArrayProvider::sm_Array) / sizeof(CbtObjectArrayProvider::sm_Array[0]);

////////////////////////////////////////////////////////////////////////////////

