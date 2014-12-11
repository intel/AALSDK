// Copyright (c) 2008-2014, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file ResMgrUtilities.cpp
/// @brief Utilities used when working with the Resource Manager. String,
/// NVS Name, and GUID inter-conversion operators.
/// @ingroup RMUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/13/2008     HM       Initial version started
/// 01/04/2009     HM       Updated Copyright
/// 02/17/2009     HM       Changed IntNameFromDeviceAddress to have bus type
///                            high order, subdevicenum low order@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALLogger.h"
#include "aalsdk/utils/ResMgrUtilities.h"

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

/*=============================================================================
===============================================================================
============================== GUID conversion macros =========================
===============================================================================
=============================================================================*/

//=============================================================================
// Name:        HexFromUChar
// Description: Converts a binary char to a hex string.
// Example:     Input 255 and get back "FF"
//=============================================================================
std::string HexFromUChar( unsigned char uc)
{
   static const std::string Hex[] = {"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
   return Hex[(uc & 0xF0) >> 4] + Hex[(uc & 0x0F)];
}

//=============================================================================
// Name:        HexFromUCharBuf
// Description: Converts a buffer of binary chars to a hex string.
// Example:     Input character array holding {255,240} and get back "FFF0"
//=============================================================================
std::string HexFromUCharBuf( unsigned char *puc, unsigned len)
{
   std::string retval("");
   for ( int i = 0; i < len ; ++i ) {
      retval = retval + HexFromUChar( *puc++ );
   }
   return retval;
}

//=============================================================================
// Name:        GUIDStringFromStruct
// Description: Returns a standard format GUID string from an input struct
// Notes:       See http://en.wikipedia.org/wiki/Globally_Unique_Identifier
//              2014: treating GUID as a byte array instead of struct
//=============================================================================
std::string GUIDStringFromStruct(AAL_GUID_t guidStruct)
{
#if 1
   std::string retval("");
   unsigned char *p = reinterpret_cast<unsigned char*>(&guidStruct);

   retval = HexFromUCharBuf( &p[0], 4) +
            "-" +
            HexFromUCharBuf( &p[4], 2 ) +
            "-" +
            HexFromUCharBuf( &p[6], 2 ) +
            "-" +
            HexFromUCharBuf( &p[8], 2 ) +
            "-" +
            HexFromUCharBuf( &p[10], 6 )
            ;
   return retval;
#else
   std::ostringstream oss;
   oss << std::hex << std::setfill('0') << std::uppercase
       << std::setw(8) << guidStruct.Data1
       << "-"
       << std::setw(4) << guidStruct.Data2
       << "-"
       << std::setw(4) << guidStruct.Data3   // includes version as first byte
       << "-"
       << HexFromUChar(guidStruct.Data4[0])
       << HexFromUChar(guidStruct.Data4[1])
       << "-"
       << HexFromUChar(guidStruct.Data4[2])
       << HexFromUChar(guidStruct.Data4[3])
       << HexFromUChar(guidStruct.Data4[4])
       << HexFromUChar(guidStruct.Data4[5])
       << HexFromUChar(guidStruct.Data4[6])
       << HexFromUChar(guidStruct.Data4[7])
       ;
   return oss.str();
#endif

}

//=============================================================================
// Name:        GUIDStructFromU64
// Description: Fakes a GUID Structure from a Unsigned 64-bit.
//              Needed until Hardware catches up to Software, and uses a real
//                128-bit GUID
//=============================================================================
AAL_GUID_t GUIDStructFromU64(const btUnsigned64bitInt &fakeGUID)
{
   AAL_GUID_t afu_id;
   memset(&afu_id, 0, sizeof(afu_id));

   char temp8[sizeof(afu_id.Data4)];

   memcpy(temp8, &fakeGUID, sizeof(afu_id.Data4));

   btUnsigned32bitInt i;
   for ( i = 0 ; i < sizeof(afu_id.Data4) ; ++i ) {  // reverse the bytes
      afu_id.Data4[i] = temp8[sizeof(afu_id.Data4) - 1 - i];
   }

   return afu_id;
}

//=============================================================================
// Name:        GUIDStructFrom2xU64
// Description: Create a GUID Structure from 2 Unsigned 64-bit quantities.
//=============================================================================
AAL_GUID_t GUIDStructFrom2xU64(const btUnsigned64bitInt &u64High, const btUnsigned64bitInt &u64Low)
{
   union {
      AAL_GUID_t afu_id;
      volatile char destination[16];   // prevent "optimizing out"
   } Guid;

   union {
      struct {
         btUnsigned64bitInt u64Low;
         btUnsigned64bitInt u64High;
      };
      char source[16];
   } Init;

   // Initialize source[16]
   Init.u64High = u64High;
   Init.u64Low  = u64Low;

   // DEBUG

   // DEBUG

   // Copy to Destination, reversing as go
   const int len = sizeof(Guid.destination);
   for ( int i = 0; i < len; ++i) {
      Guid.destination[ i ] = Init.source[ len - i -1 ];
   }

   return Guid.afu_id;
}

//=============================================================================
// Name:        UCharFromHexNybble
// Description: Converts a Hex character (e.g. "F") to its binary representation,
//                 e.g. 15
// Notes:       Worker function for internal use
// Returns:     Value, or -1 on error
//=============================================================================
int UCharFromHexNybble (unsigned char Nybble)
{
   if (Nybble >= '0' && Nybble <= '9')
      return Nybble-'0';
   if (Nybble >= 'A' && Nybble <= 'F')
      return Nybble-'A'+10;
   if (Nybble >= 'a' && Nybble <= 'f')
      return Nybble-'a'+10;
   return -1;                  // Error return, invalid hex nybble
}

//=============================================================================
// Name:        UCharFromHex
// Description: Returns a binary char value from two input hex digits
//=============================================================================
int UCharFromHex (unsigned char upperNybble, unsigned char lowerNybble)
{
   int uNyb, lNyb;
   if (  ((uNyb = UCharFromHexNybble(upperNybble)) >= 0) &&
         ((lNyb = UCharFromHexNybble(lowerNybble)) >= 0)) {
      return (uNyb<<4) + lNyb;
   }
   else
      return -1;
}

//=============================================================================
// Name:        GUIDStructFromString
// Description: Inverse of GUIDStringFromStruct
// Notes:       See http://en.wikipedia.org/wiki/Globally_Unique_Identifier
// Returns:     False if there was a format error in the string. If false, those
//                 fields will be filled in with zero, so it is still feasible
//                 to use the struct in a non-critical situation.
//              If you care about the value, then check the return code.
//=============================================================================
btBool GUIDStructFromString(const std::string& guidString, AAL_GUID_t* pguidStruct)
{
   std::istringstream iss;
   btBool             fRetVal = true;

   memset( pguidStruct, 0, sizeof(AAL_GUID_t));

   std::string temp = guidString.substr(0,18);
   iss.str(guidString.substr(0,18));
   iss >> std::hex >> pguidStruct->Data1;
   iss.ignore(1);
   iss >> pguidStruct->Data2;
   iss.ignore(1);
   iss >> pguidStruct->Data3;

   int trychar;
                                       // Data 4[0], positions 19,20
   if ((trychar = UCharFromHex( guidString[19], guidString[20])) >= 0) {
      pguidStruct->Data4[0] = trychar;
   } else {
      pguidStruct->Data4[0] = 0;
      fRetVal = false;
   }
                                       // Data 4[1], positions 21,22
   if ((trychar = UCharFromHex( guidString[21], guidString[22])) >= 0) {
      pguidStruct->Data4[1] = trychar;
   } else {
      pguidStruct->Data4[1] = 0;
      fRetVal = false;
   }

   // Position 23 is '-', then 12 hex digits for last 6 chars
   // First valid hex digit is at location 24
   for (int i=2, index=24; i<8; ++i, index+=2) {
      if ((trychar = UCharFromHex( guidString[index], guidString[index+1])) >= 0) {
         pguidStruct->Data4[i] = trychar;
      } else {
         pguidStruct->Data4[i] = 0;
         fRetVal = false;
      }
   }

   return fRetVal;
} // end of GUIDStructFromString


//=============================================================================
// Name:          AFU_IDNameFromConfigStruct
// Description:   Get a standard representation of an AFU_ID from a config
//                   struct, handling 64- vs. 128-bit GUID issues
// Interface:     public
// Inputs:        pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       String representation of AFU_ID
// Comments:
//=============================================================================
std::string AFU_IDNameFromConfigStruct( const aalrms_configUpDateEvent& cfgUpdate)
{
   // TODO - fix up to use the real m_afuGUID whenever it becomes a struct
//#define USING_64_BIT_GUID
#ifdef USING_64_BIT_GUID   /* the cfgUpdate.devattrs.devid.m_afuGUID field is 64-bit ulong */
   return GUIDStringFromStruct( GUIDStructFromU64( cfgUpdate.devattrs.devid.m_afuGUID)).c_str();
#else
   return AAL::AAS::GUIDStringFromStruct(
                AAL::AAS::GUIDStructFrom2xU64(
                      cfgUpdate.devattrs.devid.m_afuGUIDh,
                      cfgUpdate.devattrs.devid.m_afuGUIDl));
//   return GUIDStringFromStruct( cfgUpdate.devattrs.devid.m_afuGUID).c_str();
#endif

}  // end of AFU_IDNameFromConfigStruct

/*=============================================================================
===============================================================================
============================= 64-bit conversion macros ========================
===============================================================================
=============================================================================*/

//=============================================================================
// Name:        StringNameFromUID
// Description: Returns a string Name suitable for use in a Name-Value
// Notes:       Format is sPrefix followed by hex representation of u64
// Parameters:  This is worker function, so the prefix is passed in
//=============================================================================
std::string StringNameFromUID (const std::string& sPrefix, btUnsigned64bitInt u64)
{
   std::ostringstream oss;
   oss << sPrefix << std::setfill('0') << std::uppercase << std::hex << std::setw(16) << u64;
   return oss.str();
}

//=============================================================================
// Name:        UIDFromStringName
// Description: Returns a u64 decoded from the string returned by function
//                 KeyNameFromUID
// Parameters:  This is worker function, so the prefix is passed in
//=============================================================================
btUnsigned64bitInt UIDFromStringName (const std::string& sPrefix, const std::string& sKeyName)
{
   std::istringstream iss( StringFromStringName( sPrefix, sKeyName));
   btUnsigned64bitInt intkey(0);
   iss >> std::hex >> intkey;
   return intkey;
}

/*=============================================================================
===============================================================================
============================== *_ID conversion macros =========================
===============================================================================
=============================================================================*/

//=============================================================================
// Name:        StringNameFromAIA_ID
// Description: Returns a string Name suitable for use in a Name-Value for an
//              AIA_ID
//=============================================================================
std::string StringNameFromAIA_ID (btUnsigned64bitInt AIA_ID)
{
   return StringNameFromUID(KeyNameForAIA, AIA_ID);
}

//=============================================================================
// Name:        AIA_IDFromStringName
// Description: Decodes an AIA_ID from the string returned by
//                 KeyNameFromAIA_ID
//=============================================================================
btUnsigned64bitInt AIA_IDFromStringName(const std::string &sKeyName)
{
   return UIDFromStringName(KeyNameForAIA, sKeyName);
}

/*=============================================================================
===============================================================================
======================= Device Address conversion macros ======================
===============================================================================
=============================================================================*/

struct INFDA_structure // private to IntNameFromDeviceAddress
{  // This structure will overlay a btNumberKey
   btUnsigned16bitInt subdevnum;
   btUnsigned16bitInt devicenum;
   btUnsigned16bitInt busnum;
   btUnsigned16bitInt bustype;
};
CASSERT(sizeof(struct INFDA_structure) == sizeof(btNumberKey));
union INFDA_union      // private to IntNameFromDeviceAddress
{
   btNumberKey            Key;
   struct INFDA_structure inner;
};

//=============================================================================
// Name:        IntNameFromDeviceAddress
// Description: Returns an integer Name suitable for use in a Name-Value for a
//                Device Address
// WARNING:     if sizeof(btNumberKey) changes, this routine will break
//=============================================================================
btNumberKey IntNameFromDeviceAddress(const aal_device_addr *paddr)
{
   INFDA_union overload;

   overload.inner.bustype   = paddr->m_bustype;
   overload.inner.busnum    = paddr->m_busnum;
   overload.inner.devicenum = paddr->m_devicenum;
   overload.inner.subdevnum = paddr->m_subdevnum;

   return overload.Key;
} // IntNameFromDeviceAddress

//=============================================================================
// Name:        DeviceAddressFromIntName
// Description: Returns a the Device Address that was coded into an integer
//                using IntKeyNameFromDeviceAddress
// WARNING:     if sizeof(btNumberKey) changes, this routine will break
//=============================================================================
void DeviceAddressFromIntName(const btNumberKey &inKey, aal_device_addr *paddr)
{
   INFDA_union overload;

   overload.Key       = inKey;

   paddr->m_bustype   = static_cast<aal_bus_types_e>(overload.inner.bustype);
   paddr->m_busnum    = overload.inner.busnum;
   paddr->m_devicenum = overload.inner.devicenum;
   paddr->m_subdevnum = overload.inner.subdevnum;
}  // DeviceAddressFromIntName

//=============================================================================
// Name:        AFU_IDNameFromGUID
// Description: Returns an NVP Name for an AFU_ID GUID, given the GUID in
//                string form, called a GUID String
//              GUID Name --> GUID String
//=============================================================================
std::string AFU_IDNameFromGUID(AAL_GUID_t guidStruct)
{
   std::string sGUIDName = GUIDStringFromStruct(guidStruct);
   return StringNameFromString( KeyNameForAFU, sGUIDName);
} // End of AFU_IDNameFromGUID

//=============================================================================
// Name:        GUIDFromAFU_IDName
// Description: Returns a the Device Address that was coded into an integer
//                using IntKeyNameFromDeviceAddress
//              GUID Name <-- GUID String
//=============================================================================
btBool GUIDFromAFU_IDName(const std::string &guidString, AAL_GUID_t *pguidStruct)
{
   std::string sGUIDName = StringFromStringName(KeyNameForAFU, guidString);
   return GUIDStructFromString(sGUIDName, pguidStruct);
}  // GUIDFromAFU_IDName

// s, takes a prefix
//=============================================================================
// Name:        StringFromStringName
// Description: Given a string name and a prefix, return just the embedded
//                 string.
// Example:     String Name is "Foo_Record_0x00000000"
//              Embedded string is "0x00000000"
//              Prefix is "Foo_Record_"
//=============================================================================
std::string StringFromStringName(const std::string &sPrefix, const std::string &sKeyName)
{
   return sKeyName.substr(sPrefix.length());
}  // StringFromStringName
//=============================================================================
// Name:        StringNameFromString
// Description: Given a string and a name prefix, return just the string name.
// Example:     String Name is "Foo_Record_0x00000000"
//              Embedded string is "0x00000000"
//              Prefix is "Foo_Record_"
//=============================================================================
std::string StringNameFromString(const std::string &sPrefix, const std::string &sInput)
{
   return sPrefix + sInput;
}  // StringNameFromString


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

