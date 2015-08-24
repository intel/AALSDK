// Copyright (c) 2008-2015, Intel Corporation
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
/// @file ResMgrUtilities.h
/// @brief Utilities used when working with the Resource Manager. String,
/// NVS Name, and GUID inter-conversion operators.
/// @ingroup RMUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:       WHO:     WHAT:
/// 11/08/2008  HM       Initial version started
/// 11/13/2008  HM       Moved code to .cpp module in AASLib
/// 01/04/2009  HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_RESMGRUTILITIES_H__
#define __AALSDK_UTILS_RESMGRUTILITIES_H__
#include <aalsdk/AALTypes.h>         // bt types
#include <aalsdk/kernel/aaldevice.h> // aal_device_addr

BEGIN_NAMESPACE(AAL)

/// @addtogroup RMUtils
/// @{

      //=============================================================================
      // Standard Strings for use by and with the Resource Manager
      //=============================================================================
      /// NVS key name for AIA.
      static const char    KeyNameForAIA[] = "AIA_ID_";
      /// NVS key name for AFU ID.
      static const char    KeyNameForAFU[] = "AFU_ID_";

      //=============================================================================
      // Type-specific name to data conversions
      //=============================================================================

      // AFU_ID Name <--> GUID, knows the correct prefix and real data type

      /// Returns an NVP Name for an AFU_ID GUID, given the GUID in string form, called a GUID String.
AASLIB_API std::string          AFU_IDNameFromGUID         (AAL_GUID_t guidStruct);
      /// Returns the Device Address that was coded into an integer using IntKeyNameFromDeviceAddress.
AASLIB_API btBool               GUIDFromAFU_IDName         (const std::string &guidString,
                                                            AAL_GUID_t        *pguidStruct);

      /// Get a standard representation of an AFU_ID from a config struct, handling 64- vs. 128-bit GUID issues.
AASLIB_API std::string          AFU_IDNameFromConfigStruct (const aalrms_configUpDateEvent &cfgUpdate);

      // AIA_ID <--> StringName, knows the correct prefix
      /// Returns a string Name suitable for use in a Name-Value for an AIA_ID.
AASLIB_API std::string          StringNameFromAIA_ID       (btUnsigned64bitInt AIA_ID);
      /// Decodes an AIA_ID from the string returned by KeyNameFromAIA_ID.
AASLIB_API btUnsigned64bitInt   AIA_IDFromStringName       (const std::string &sKeyName);

      //=============================================================================
      // Structure-specific inter-conversions
      //=============================================================================

      // GUID Struct <--- uint64 until the HW catches up and uses 128-bits
      /// Fakes a GUID Structure from a Unsigned 64-bit.
AASLIB_API AAL_GUID_t           GUIDStructFromU64(const btUnsigned64bitInt &fakeGUID);

/// Description: Create a GUID Structure from 2 Unsigned 64-bit quantities.
AASLIB_API AAL_GUID_t           GUIDStructFrom2xU64(const btUnsigned64bitInt &u64High,
                                                    const btUnsigned64bitInt &u64Low);


      //=============================================================================
      // Structure-specific name to data conversions
      //=============================================================================

      // GUID Struct <--> String
      /// Returns a standard format GUID string from an input struct.
AASLIB_API std::string          GUIDStringFromStruct       (AAL_GUID_t guidStruct);
      /// Inverse of GUIDStringFromStruct.
      /// @retval  false  There was a format error in the string. If false, those
      /// fields will be filled in with zero, so it is still feasible to use the
      /// struct in a non-critical situation.
AASLIB_API btBool               GUIDStructFromString       (const std::string &guidString,
                                                            AAL_GUID_t        *pguidStruct);

      // DeviceAddress <--> Numeric Key
      /// Returns an integer Name suitable for use in a Name-Value for a Device Address.
AASLIB_API btNumberKey          IntNameFromDeviceAddress   (const aal_device_addr *paddr);
      /// Returns a the Device Address that was coded into an integer using IntKeyNameFromDeviceAddress.
AASLIB_API void                 DeviceAddressFromIntName   (const btNumberKey &inKey,
                                                            aal_device_addr   *paddr);

      // Generic 64-bit <--> StringName worker functions, takes a prefix
      /// Returns a u64 decoded from the string returned by function KeyNameFromUID.
AASLIB_API btUnsigned64bitInt   UIDFromStringName          (const std::string &sPrefix,
                                                            const std::string &sKeyName);
      /// Returns a string Name suitable for use in a Name-Value.
      ///
      /// Format is sPrefix followed by hex representation of u64.
      /// This is worker function, so the prefix is passed in.
AASLIB_API std::string          StringNameFromUID          (const std::string &sPrefix,
                                                            btUnsigned64bitInt u64);

      //=============================================================================
      // String-based worker functions
      //=============================================================================

      // Generic string <--> StringName worker functions, takes a prefix
      /// Given a string name and a prefix, return just the embedded string.
      ///
      /// Example:
      /// - String Name is "Foo_Record_0x00000000"
      /// - Embedded string is "0x00000000"
      /// - Prefix is "Foo_Record_"
AASLIB_API std::string          StringFromStringName       (const std::string &sPrefix,
                                                            const std::string &sKeyName);
      /// Given a string and a name prefix, return just the string name.
      /// Example:
      /// - String Name is "Foo_Record_0x00000000"
      /// - Embedded string is "0x00000000"
      /// - Prefix is "Foo_Record_"
AASLIB_API std::string          StringNameFromString       (const std::string &sPrefix,
                                                            const std::string &sInput);

      // char and hex worker routines

      /// Converts a binary char to a hex string.
      /// Example:
      /// - Input 255 and get back "FF"
AASLIB_API std::string          HexFromUChar               (unsigned char uc);
      /// Converts a buffer of binary chars to a hex string.
AASLIB_API std::string          HexFromUCharBuf            (unsigned char *puc, unsigned len);
      /// Converts a Hex character (e.g. "F") to its binary representation, e.g. 15
AASLIB_API int                  UCharFromHexNybble         (unsigned char Nybble);
      /// Returns a binary char value from two input hex digits.
AASLIB_API int                  UCharFromHex               (unsigned char upperNybble,
                                                            unsigned char lowerNybble);

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_UTILS_RESMGRUTILITIES_H__

