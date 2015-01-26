// Copyright (c) 2005-2015, Intel Corporation
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
/// @file AALNamedValueSet.h
/// @brief Definitions for NamedValueSets.
/// @ingroup BasicTypes
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/15/2005     JG       Initial version started
/// 03/06/2007     JG       Added MessageHeader Type
/// 03/19/2007     JG       Linux Port
/// 05/29/2007     JG       File changed to AALNamedValueSet.h
///                         Modified to handle multiple key types
/// 07/13/2007     JG       Fixed a bug in assigment that would
///                         cause incorrect results (could be bigger).
///                JG       Fixed also could leak.
///                JG       Added Empty() method
/// 07/23/2007     JG       Made NVS value on Add NVS a const &
///                JG       Changed NVS get to take a NVS const **
/// 07/30/2007     HM       Added more ENamedValues to handle
///                            NVSRead/WriteNVS error conditions
/// 08/08/2007     HM       Added subset() and operator== to NVS
/// 08/11/2007     HM       Added ENamedValues EndOfFile to indicate
///                            end of a file read operation
/// 09/25/2007     JG       Began refactoring out old code
/// 10/04/2007     JG       Minor interface changes
/// 11/21/2007     HM       changed btString to btcString
/// 03/22/2008     HM       Added ENamedValuesInvalidReadToNull, indicating
///                            an attempt to read to a null nvs
/// 03/22/2008     HM       Moved NVS Read and Write and operator << >> function
///                            headers to here, removing them from
///                            AALCNamedValueSet.h
/// 05/01/2008     JG       Added btByteArray
/// 05/08/2008     HM       Comments & License
/// 05/26/2008     HM       Added Subset(), in preparation for deprecating
///                            subset().
/// 05/28/2008     HM       Begin removing NVS File IO
/// 05/29/2008     HM       Convert btcString and btUnsignedInt key types into
///                            specific typedefs: btStringKey and btNumberKey
/// 11/16/2008     HM       New utilities, for converting between strings and
///                            and NamedValueSets, and merging NVS's
/// 11/20/2008     HM       FormattedNVS output provides tabbing and hook point
///                            for human readable AAL_ID->string conversion
/// 01/04/2009     HM       Updated Copyright
/// 09/21/2011     JG       Added constructor from const std:string@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALNAMEDVALUESET_H__
#define __AALSDK_AALNAMEDVALUESET_H__
#include <aalsdk/AALTypes.h>

#define NVSFileIO /* for the time being, leave it in */


BEGIN_NAMESPACE(AAL)

#ifdef __cplusplus

/// @addtogroup BasicTypes
/// @{

   //=============================================================================
   // Typedefs and Constants
   //=============================================================================

   /// Return codes for INamedValueSet methods.
   typedef enum ENamedValues {
      ENamedValuesOK = 0,                             ///< Operation successful.
      ENamedValuesNameNotFound,                       ///< Key lookup failed.
      ENamedValuesDuplicateName,                      ///< Key name collision.
      ENamedValuesBadType,
      ENamedValuesNotSupported,
      ENamedValuesIndexOutOfRange,                    ///< No such index.
      ENamedValuesRecursiveAdd,
      ENamedValuesInternalError_InvalidNameFormat,
      ENamedValuesInternalError_UnexpectedEndOfFile,
      ENamedValuesOutOfMemory,
      ENamedValuesEndOfFile,
      ENamedValuesInvalidReadToNull
   } ENamedValues;

   class NamedValueSet;

   /// Base public interface for Named Value Sets.
   class AASLIB_API INamedValueSet
   {
   public:
      /// INamedValueSet Destructor.
      virtual ~INamedValueSet();

      /// Add btBool, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name,btBool value)                = 0;
      /// Add btByte, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, btByte value)               = 0;
      /// Add bt32bitInt, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, bt32bitInt value)           = 0;
      /// Add btUnsigned32bitInt, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, btUnsigned32bitInt value)   = 0;
      /// Add bt64bitInt, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, bt64bitInt value)           = 0;
      /// Add btUnsigned64bitInt, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, btUnsigned64bitInt value)   = 0;
      /// Add btFloat, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, btFloat value)              = 0;
      /// Add btcString, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, btcString value)            = 0;
      /// Add NamedValueSet, keyed by integer.
      virtual ENamedValues Add(btNumberKey Name, NamedValueSet const &value) = 0;
      /// Add byte array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of bytes to add.
      virtual ENamedValues Add(btNumberKey        Name,
                               btByteArray        value,
                               btUnsigned32bitInt NumElements)               = 0;
      /// Add signed 32-bit integer array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of signed 32-bit integers to add.
      virtual ENamedValues Add(btNumberKey       Name,
                               bt32bitIntArray   value,
                              btUnsigned32bitInt NumElements)                = 0;
      /// Add unsigned 32-bit integer array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of unsigned 32-bit integers to add.
      virtual ENamedValues Add(btNumberKey             Name,
                               btUnsigned32bitIntArray value,
                               btUnsigned32bitInt      NumElements)          = 0;
      /// Add signed 64-bit integer array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of signed 64-bit integers to add.
      virtual ENamedValues Add(btNumberKey        Name,
                               bt64bitIntArray    value,
                               btUnsigned32bitInt NumElements)               = 0;
      /// Add unsigned 64-bit integer array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of unsigned 64-bit integers to add.
      virtual ENamedValues Add(btNumberKey             Name,
                               btUnsigned64bitIntArray value,
                               btUnsigned32bitInt      NumElements)          = 0;
      /// Add btObjectType, keyed by integer.
      virtual ENamedValues Add(btNumberKey  Name, btObjectType value)        = 0;
      /// Add floating-point array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of floating points to add.
      virtual ENamedValues Add(btNumberKey        Name,
                               btFloatArray       value,
                               btUnsigned32bitInt NumElements)               = 0;
      /// Add string array, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of strings to add.
      virtual ENamedValues Add(btNumberKey        Name,
                               btStringArray      value,
                               btUnsigned32bitInt NumElements)               = 0;
      /// Add generic objects, keyed by integer.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of generic objects to add.
      virtual ENamedValues Add(btNumberKey        Name,
                               btObjectArray      value,
                               btUnsigned32bitInt NumElements)               = 0;

      /// Retrieve btBool by integer key.
      virtual ENamedValues Get(btNumberKey Name, btBool *pValue) const                  = 0;
      /// Retrieve btByte by integer key.
      virtual ENamedValues Get(btNumberKey Name, btByte *pValue) const                  = 0;
      /// Retrieve bt32bitInt by integer key.
      virtual ENamedValues Get(btNumberKey Name, bt32bitInt *pValue) const              = 0;
      /// Retrieve btUnsigned32bitInt by integer key.
      virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitInt *pValue) const      = 0;
      /// Retrieve bt64bitInt by integer key.
      virtual ENamedValues Get(btNumberKey Name, bt64bitInt *pValue) const              = 0;
      /// Retrieve btUnsigned64bitInt by integer key.
      virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitInt *pValue) const      = 0;
      /// Retrieve btFloat by integer key.
      virtual ENamedValues Get(btNumberKey Name, btFloat *pValue) const                 = 0;
      /// Retrieve btcString by integer key.
      virtual ENamedValues Get(btNumberKey Name, btcString *pValue) const               = 0;
      /// Retrieve NamedValueSet * by integer key.
      virtual ENamedValues Get(btNumberKey Name, NamedValueSet const **pValue) const    = 0;
      /// Retrieve btByteArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btByteArray *pValue) const             = 0;
      /// Retrieve bt32bitIntArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, bt32bitIntArray *pValue) const         = 0;
      /// Retrieve btUnsigned32bitIntArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const = 0;
      /// Retrieve bt64bitIntArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, bt64bitIntArray *pValue) const         = 0;
      /// Retrieve btUnsigned64bitIntArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const = 0;
      /// Retrieve btObjectType by integer key.
      virtual ENamedValues Get(btNumberKey Name, btObjectType *pValue) const            = 0;
      /// Retrieve btFloatArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btFloatArray *pValue) const            = 0;
      /// Retrieve btStringArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btStringArray *pValue) const           = 0;
      /// Retrieve btObjectArray by integer key.
      virtual ENamedValues Get(btNumberKey Name, btObjectArray *pValue) const           = 0;

      /// Remove an entry by integer key.
      ///
      /// @retval  ENamedValuesNameNotFound  Name not found.
      /// @retval  ENamedValuesOK            On success.
      virtual ENamedValues Delete(btNumberKey Name)                                   = 0;
      /// Determine the number of elements stored in an entry keyed by integer.
      ///
      /// @param[out]  pSize  Receives the number of elements.
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues GetSize(btNumberKey Name, btWSSize *pSize) const = 0;
      /// Determine the type of element(s) stored in an entry keyed by integer.
      ///
      /// @param[out]  pType  Receives the type..
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues Type(btNumberKey Name, eBasicTypes *pType) const           = 0;
      /// Determine whether an entry exists for Name, an integer key.
      virtual btBool Has(btNumberKey Name) const                                      = 0;
      /// Retrieve the name (key) of the named-value pair at zero-based index.
      ///
      /// @param[in]   index  Zero-based index into named-value pairs.
      /// @param[out]  pName  Location to receive the name (integer).
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues GetName(btUnsignedInt index, btNumberKey *pName) const     = 0;


   #ifndef _NO_STRING_KEYS_
      /// Add btBool, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btBool value)               = 0;
      /// Add btByte, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btByte value)               = 0;
      /// Add bt32bitInt, keyed by string.
      virtual ENamedValues Add(btStringKey Name, bt32bitInt value)           = 0;
      /// Add btUnsigned32bitInt, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btUnsigned32bitInt  value)  = 0;
      /// Add bt64bitInt, keyed by string.
      virtual ENamedValues Add(btStringKey Name, bt64bitInt value)           = 0;
      /// Add btUnsigned64bitInt, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btUnsigned64bitInt value)   = 0;
      /// Add btFloat, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btFloat value)              = 0;
      /// Add btcString, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btcString value)            = 0;
      /// Add NamedValueSet, keyed by string.
      virtual ENamedValues Add(btStringKey Name, NamedValueSet const &value) = 0;
      /// Add byte array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of bytes to add.
      virtual ENamedValues Add(btStringKey        Name,
                               btByteArray        value,
                               btUnsigned32bitInt NumElements)       = 0;
      /// Add signed 32-bit integer array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of signed 32-bit integers to add.
      virtual ENamedValues Add(btStringKey        Name,
                               bt32bitIntArray    value,
                               btUnsigned32bitInt NumElements)       = 0;
      /// Add unsigned 32-bit integer array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of unsigned 32-bit integers to add.
      virtual ENamedValues Add(btStringKey             Name,
                               btUnsigned32bitIntArray value,
                               btUnsigned32bitInt      NumElements)  = 0;
      /// Add signed 64-bit integer array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of signed 64-bit integers to add.
      virtual ENamedValues Add(btStringKey        Name,
                               bt64bitIntArray    value,
                               btUnsigned32bitInt NumElements)       = 0;
      /// Add unsigned 64-bit integer array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of unsigned 64-bit integers to add.
      virtual ENamedValues Add(btStringKey             Name,
                               btUnsigned64bitIntArray value,
                               btUnsigned32bitInt      NumElements)  = 0;
      /// Add btObjectType, keyed by string.
      virtual ENamedValues Add(btStringKey Name, btObjectType value) = 0;
      /// Add floating-point array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of floating points to add.
      virtual ENamedValues Add(btStringKey        Name,
                               btFloatArray       value,
                               btUnsigned32bitInt NumElements)       = 0;
      /// Add string array, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of strings to add.
      virtual ENamedValues Add(btStringKey        Name,
                               btStringArray      value,
                               btUnsigned32bitInt NumElements)       = 0;
      /// Add generic objects, keyed by string.
      ///
      /// @param[in]  Name         Key.
      /// @param[in]  value        Pointer to the buffer to be added.
      /// @param[in]  NumElements  Number of generic objects to add.
      virtual ENamedValues Add(btStringKey        Name,
                               btObjectArray      value,
                               btUnsigned32bitInt NumElements)       = 0;

      /// Retrieve btBool by string key.
      virtual ENamedValues Get(btStringKey Name, btBool *pValue) const                  = 0;
      /// Retrieve btByte by string key.
      virtual ENamedValues Get(btStringKey Name, btByte *pValue) const                  = 0;
      /// Retrieve bt32bitInt by string key.
      virtual ENamedValues Get(btStringKey Name, bt32bitInt *pValue) const              = 0;
      /// Retrieve btUnsigned32bitInt by string key.
      virtual ENamedValues Get(btStringKey Name, btUnsigned32bitInt *pValue) const      = 0;
      /// Retrieve bt64bitInt by string key.
      virtual ENamedValues Get(btStringKey Name, bt64bitInt *pValue) const              = 0;
      /// Retrieve btUnsigned64bitInt by string key.
      virtual ENamedValues Get(btStringKey Name, btUnsigned64bitInt *pValue) const      = 0;
      /// Retrieve btFloat by string key.
      virtual ENamedValues Get(btStringKey Name, btFloat *pValue) const                 = 0;
      /// Retrieve btcString by string key.
      virtual ENamedValues Get(btStringKey Name, btcString *pValue) const               = 0;
      /// Retrieve NamedValueSet * by string key.
      virtual ENamedValues Get(btStringKey Name, NamedValueSet const **pValue) const    = 0;
      /// Retrieve btByteArray by string key.
      virtual ENamedValues Get(btStringKey Name, btByteArray *pValue) const             = 0;
      /// Retrieve bt32bitIntArray by string key.
      virtual ENamedValues Get(btStringKey Name, bt32bitIntArray *pValue) const         = 0;
      /// Retrieve btUnsigned32bitIntArray by string key.
      virtual ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const = 0;
      /// Retrieve bt64bitIntArray by string key.
      virtual ENamedValues Get(btStringKey Name, bt64bitIntArray *pValue) const         = 0;
      /// Retrieve btUnsigned64bitIntArray by string key.
      virtual ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const = 0;
      /// Retrieve btObjectType by string key.
      virtual ENamedValues Get(btStringKey Name, btObjectType *pValue) const            = 0;
      /// Retrieve btFloatArray by string key.
      virtual ENamedValues Get(btStringKey Name, btFloatArray *pValue) const            = 0;
      /// Retrieve btStringArray by string key.
      virtual ENamedValues Get(btStringKey Name, btStringArray *pValue) const           = 0;
      /// Retrieve btObjectArray by string key.
      virtual ENamedValues Get(btStringKey Name, btObjectArray *pValue) const           = 0;

      /// Remove an entry by string key.
      ///
      /// @retval  ENamedValuesNameNotFound  Name not found.
      /// @retval  ENamedValuesOK            On success.
      virtual ENamedValues Delete(btStringKey Name)                                   = 0;
      /// Determine the number of elements stored in an entry keyed by string.
      ///
      /// @param[out]  pSize  Receives the number of elements.
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues GetSize(btStringKey Name, btWSSize *pSize) const = 0;
      /// Determine the type of element(s) stored in an entry keyed by string.
      ///
      /// @param[out]  pType  Receives the type..
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues Type(btStringKey Name,eBasicTypes *pType) const            = 0;
      /// Determine whether an entry exists for Name, a string key.
      virtual btBool Has(btStringKey Name) const                                      = 0;
      /// Retrieve the name (key) of the named-value pair at zero-based index.
      ///
      /// @param[in]   index  Zero-based index into named-value pairs.
      /// @param[out]  pName  Location to receive the name (string).
      ///
      /// @retval ENamedValuesNameNotFound  Name not found.
      /// @retval ENamedValuesOK            On success.
      virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const     = 0;
   #endif

      /// Force the Named Value Set to delete all its members.
      virtual ENamedValues Empty()                                    = 0;
#if DEPRECATED
      /// @deprecated
      virtual btBool subset(const INamedValueSet &rOther) const       = 0;
#endif // DEPRECATED
      /// Determine whether a Named Value Set is a subset of another.
      virtual btBool Subset(const INamedValueSet &rOther) const       = 0;
      /// Test for equality.
      virtual btBool operator == (const INamedValueSet &rOther) const = 0;
      /// Determine the number of name/value pairs held in the Named Value Set.
      virtual ENamedValues GetNumNames(btUnsignedInt *pNum) const     = 0;
      /// Determine the key type of the named-value pair at zero-based index.
      virtual ENamedValues GetNameType(btUnsignedInt index,
                                       eNameTypes   *pType) const     = 0;

   private:
      INamedValueSet & operator = (const INamedValueSet & );
   };


   //=============================================================================
   //   NVS* functions are helper functions for NamedValueSets. They do not access
   //       the internal structure of a NamedValueSet, but perform useful
   //       operations on them.
   //=============================================================================

#ifdef NVSFileIO
   // Reads an open (binary) file and creates an NVS from it.
   AASLIB_API
      ENamedValues NVSReadNVS(FILE          *file,
                              NamedValueSet *nvsToRead);

   // Writes the contents of an NVS to an open (binary) file in a form that can be read by NVSReadNVS.
   //
   // @param[in]  file        An open FILE * to receive the NVS.
   // @param[in]  nvsToWrite  The NVS to write.
   // @param[in]  level       Zero-based level used to identify nested NVS's.
   AASLIB_API
      ENamedValues NVSWriteNVS(FILE                *file,
                               NamedValueSet const &nvsToWrite,
                               unsigned             level);

   AASLIB_API                                // Serialize NVS with ending
      ENamedValues NVSWriteOneNVSToFile(FILE                *file,     //    demarcation
                                        NamedValueSet const &nvsToWrite,
                                        unsigned             level);
#endif

   AASLIB_API                                // Serialize NVS from stream
      ENamedValues NVSReadNVS (std::istream& is,
                               NamedValueSet* nvsToRead);

   AASLIB_API                                // Serialize NVS to file
      ENamedValues NVSWriteNVS (std::ostream& os,
                                NamedValueSet const& nvsToWrite,
                                unsigned level);
   AASLIB_API                                // Serialize NVS with ending
      ENamedValues NVSWriteOneNVSToFile (std::ostream& os,    //    demarcation
                                         NamedValueSet const& nvsToWrite,
                                         unsigned level);

   //=============================================================================
   // Name:        operator << on NamedValueSet
   // Description: serializes a NamedValueSet to a stream
   // Interface:   public
   // Inputs:      stream, and NamedValueSet
   // Outputs:     serialized NamedValueSet
   // Comments:    works for writing to any of cout, ostream, ofstream,
   //                 ostringstream, fstream, stringstream
   //=============================================================================
   AASLIB_API
      std::ostream& operator << (std::ostream& s, const AAL::NamedValueSet& nvs);


   //=============================================================================
   // Name:        FormattedNVS
   // Description: Helper function for ostream::operator<< for a formatted NVS
   // Interface:   public
   // Inputs:      NamedValueSet
   // Outputs:     FormattedNVS object to be passed to ostream::operator<<
   // Comments:    works for writing to any of cout, ostream, ofstream,
   //                 ostringstream, fstream, stringstream
   //=============================================================================
   class AASLIB_API FormattedNVS
   {
   public:
      const NamedValueSet &m_nvs;
      const btInt          m_tab;
      const btBool         m_fDebug;
      FormattedNVS(const NamedValueSet &nvs, btInt tab=0, btBool fDebug=false) :
         m_nvs(nvs), m_tab(tab), m_fDebug(fDebug) {}
      const FormattedNVS & operator = (const FormattedNVS &other ) { return *this; }
   };

   //=============================================================================
   // Name:        operator << on FormattedNVS
   // Description: serializes a NamedValueSet to a stream in a formatted manner
   // Interface:   public
   // Inputs:      stream, FormattedNVS which was constructed from a
   //                 NamedValueSet, a tab value, and a debug value
   // Outputs:     serialized formatted NamedValueSet
   // Comments:    works for writing to any of cout, ostream, ofstream,
   //                 ostringstream, fstream, stringstream
   //=============================================================================
   AASLIB_API
      std::ostream & operator << (std::ostream &s, const AAL::FormattedNVS &fnvs);

   //=============================================================================
   // Name: operator >> on NamedValueSet
   // Description: reads a serialized NamedValueSet from a stream
   // Interface: public
   // Inputs: stream, and NamedValueSet
   // Outputs: none
   // Comments: works for reading from of cin, istream, ifstream,
   //           iostringstream, fstream, stringstream
   // Comments: The passed in NamedValueSet is not emptied first, so if it contains
   //           information already, that will simply be added to.
   //=============================================================================
   AASLIB_API
      std::istream& operator >> (std::istream& s, AAL::NamedValueSet& rnvs);

   //=============================================================================
   // Name:        AAL::NamedValueSetFromStdString
   // Description: Convert a std::string length into an NVS
   // Interface:   public
   // Inputs:      std::string containing the serialized representation of the
   //              NamedValueSet.
   // Outputs:     nvs is a non-const reference to the returned NamedValueSet
   //=============================================================================
   AASLIB_API
      void NamedValueSetFromStdString (const std::string& s, AAL::NamedValueSet& nvs);

   //=============================================================================
   // Name:        AAL::NamedValueSetFromCharString
   // Description: Convert a char* + length into an NVS
   // Interface:   public
   // Inputs:      char*, length of char* including a final terminating null.
   //              Note that the string itself may contain embedded nulls; they
   //              do not terminate the string. That is, the char* is not a normal
   //              NULL-terminated string
   // Outputs:     nvs is a non-const reference to the returned NamedValueSet
   //=============================================================================
   AASLIB_API
      void NamedValueSetFromCharString(void* pv, btWSSize len, AAL::NamedValueSet& nvs);


   //=============================================================================
   // Name:        AAL::StdStringFromNamedValueSet
   // Description: Return a std::string containing the serialized NamedValueSet
   // Interface:   public
   // Inputs:      const NamedValueSet& nvs
   // Returns:     As in description. Note that the returned string.length() is
   //              the correct length to allocate for a buffer in which to store
   //              the buffer.
   // NOTE:        See AAL::CharFromString for important usage note
   //=============================================================================
   AASLIB_API
      std::string StdStringFromNamedValueSet (const AAL::NamedValueSet& nvs);

   //=============================================================================
   // Name:        AAL::BufFromString
   // Description: Copy the contents of a std::string into a char[]. Note that
   //              embedded NULLs are properly handled (that is, they are ignored).
   // Interface:   public
   // Inputs:      const NamedValueSet& nvs
   // Returns:     Buffer must be >= the string's length
   // Use:         To serialize a NVS to a char buffer, do something like this:
   //                 std::string s = StdStringFromNamedValueSet( nvs);
   //                 char* pBuf = new char[s.length()];
   //                 BufFromString( pBuf, s);
   // Efficiency:  This involves at least 3 copies (into the ostringstream,
   //                 the creation of s, and the copy into pBuf). If you need
   //                 to reduce it to two copies, you can eliminate the middle
   //                 string creation, but you'll need to put the new() inside the
   //                 function. See the implementations of the functions for
   //                 more details.
   // Caveat:      This routine assumes that the std::string passed in does have
   //                 an embedded terminating NULL, which is used to final terminate
   //                 buffer (if it is to be used as a string).
   //                 StdStringFromNamedValueSet() adds this terminating NULL.
   //                 Using this routine without such a guarantee and then using
   //                 the buffer as a c-string will result in a non-NULL-terminated
   //                 string. It is unlikely that you will want to do that.
   //=============================================================================
   AASLIB_API
      void BufFromString (void* pBuf, std::string const &s);

   //=============================================================================
   // Name:          AAL::NVSMerge
   // Description:   Merge one NamedValueSet into another
   // Interface:     public
   // Inputs:        nvsInput and nvsOutput will be merged together
   // Outputs:       nvsOutput will contain the merged results
   // Returns:       ENamedValuesOK for success, appropriate value otherwise
   // Comments:      If there are duplicate names, the original value in the
   //                   nvsOutput takes precedence and there is no error
   //=============================================================================
   AASLIB_API
      ENamedValues NVSMerge (NamedValueSet& nvsOutput, const NamedValueSet& nvsInput);

   //=============================================================================
   // Factory and wrapper for application created Parms
   //=============================================================================
   AASLIB_API INamedValueSet * _NewNamedValueSet();
   AASLIB_API void             _DeleteNamedValueSet(INamedValueSet *m_namedvalues);
   AASLIB_API void             _DupNamedValueSet(INamedValueSet *m_namedvalues,
                                                 INamedValueSet *rOther);


   /// Wrapper class for NamedValueSet implementation.
   class AASLIB_API NamedValueSet : public INamedValueSet
   {
   public:
      /// NamedValueSet Default Constructor.
      NamedValueSet() : m_namedvalues(NULL) {
         m_namedvalues =_NewNamedValueSet();
      }
      /// NamedValueSet Destructor.
      virtual ~NamedValueSet() {
         _DeleteNamedValueSet(m_namedvalues);
      }
      /// NamedValueSet Assignment.
      NamedValueSet  & operator =(const NamedValueSet  &rOther) {
         if(&rOther == this) {
            return *this;
         }     //Don't dup yourself

         // Make sure this NVS is empty
         m_namedvalues->Empty();

         //Copy the NamedValueSet
         _DupNamedValueSet(m_namedvalues, rOther.m_namedvalues);
         return *this;
      }
      /// NamedValueSet Copy Constructor.
      NamedValueSet(const NamedValueSet &rOther) : m_namedvalues(NULL) {
         m_namedvalues = _NewNamedValueSet();
         _DupNamedValueSet(m_namedvalues, rOther.m_namedvalues);
      }
      /// NamedValueSet Construct from std::string.
      NamedValueSet (const std::string & rstr) : m_namedvalues(NULL) {
         m_namedvalues =_NewNamedValueSet();
         NamedValueSetFromStdString(rstr, *this);
      }

      /// NamedValueSet equality.
      btBool operator == (const INamedValueSet &rOther) const {
         return *m_namedvalues == *((dynamic_cast<const NamedValueSet*>(&rOther))->m_namedvalues);
      }

#if DEPRECATED
      /// @deprecated
      btBool subset(const INamedValueSet &rOther) const { return Subset(rOther); }
#endif // DEPRECATED
//          return m_namedvalues->subset(*((dynamic_cast<const NamedValueSet*>(&rOther))->m_namedvalues));
//       }

      btBool Subset(const INamedValueSet &rOther) const {
         return m_namedvalues->Subset(*((dynamic_cast<const NamedValueSet*>(&rOther))->m_namedvalues));
      }

      ENamedValues GetNameType(btUnsignedInt index, eNameTypes *pType) const {
         return m_namedvalues->GetNameType( index, pType);
      }

      ENamedValues Add( btNumberKey Name,btBool value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,btByte value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,bt32bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,btUnsigned32bitInt  value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,bt64bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,btUnsigned64bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,btFloat value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name, btcString value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name, NamedValueSet const &value) {
         if(&value == this) {
            return ENamedValuesRecursiveAdd;
         }
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,
                        btByteArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        bt32bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        btUnsigned32bitIntArray  value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        bt64bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        btUnsigned64bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btNumberKey Name,btObjectType value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btNumberKey Name,
                        btFloatArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        btStringArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Add( btNumberKey Name,
                        btObjectArray value,
                        btUnsigned32bitInt  NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Get( btNumberKey Name,
                        btBool *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btByte *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        bt32bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btUnsigned32bitInt  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        bt64bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btUnsigned64bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btFloat *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btcString  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        NamedValueSet const **pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        bt32bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btByteArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btUnsigned32bitIntArray  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        bt64bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btUnsigned64bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btObjectType *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btFloatArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btStringArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btNumberKey Name,
                        btObjectArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Delete( btNumberKey Name) {
         return m_namedvalues->Delete( Name );
      }

      ENamedValues Empty() {
         return m_namedvalues->Empty();
      }

      ENamedValues GetSize(btNumberKey Name,
                           btWSSize   *pSize) const {
         return m_namedvalues->GetSize(Name, pSize);
      }

      ENamedValues Type(btNumberKey  Name,
                        eBasicTypes *pType)const {
         return m_namedvalues->Type(Name, pType);
      }

      ENamedValues GetNumNames(btUnsignedInt *pNum)const {
         return m_namedvalues->GetNumNames(pNum);
      }

      ENamedValues GetName( btUnsignedInt index,
                            btNumberKey *pName)const {
         return m_namedvalues->GetName( index,pName);
      }

      btBool Has( btNumberKey Name )const {
         return m_namedvalues->Has( Name );
      }
#ifndef _NO_STRING_KEYS_
      //String Key methods
      ENamedValues Add( btStringKey Name,btBool value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,btByte value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,bt32bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,btUnsigned32bitInt  value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,bt64bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,btUnsigned64bitInt value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,btFloat value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name, btcString value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name, NamedValueSet const &value) {
         if(&value == this) {
            return ENamedValuesRecursiveAdd;
         }
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,
                        btByteArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        bt32bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        btUnsigned32bitIntArray  value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        bt64bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        btUnsigned64bitIntArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value, NumElements);
      }

      ENamedValues Add( btStringKey Name,btObjectType value) {
         return m_namedvalues->Add( Name,value);
      }

      ENamedValues Add( btStringKey Name,
                        btFloatArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        btStringArray value,
                        btUnsigned32bitInt NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Add( btStringKey Name,
                        btObjectArray value,
                        btUnsigned32bitInt  NumElements) {
         return m_namedvalues->Add( Name,value,NumElements);
      }

      ENamedValues Get( btStringKey Name,
                        btBool *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btByte *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        bt32bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btUnsigned32bitInt  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        bt64bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btUnsigned64bitInt *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btFloat *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btcString  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        NamedValueSet const **pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btByteArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        bt32bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btUnsigned32bitIntArray  *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        bt64bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btUnsigned64bitIntArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btObjectType *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }


      ENamedValues Get( btStringKey Name,
                        btFloatArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btStringArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Get( btStringKey Name,
                        btObjectArray *pValue)const {
         return m_namedvalues->Get( Name,pValue);
      }

      ENamedValues Delete( btStringKey Name) {
         return m_namedvalues->Delete( Name );
      }

      ENamedValues GetSize(btStringKey  Name,
                           btWSSize    *pSize)const {
         return m_namedvalues->GetSize(Name, pSize);
      }

      ENamedValues Type(btStringKey  Name,
                        eBasicTypes *pType)const {
         return m_namedvalues->Type(Name, pType);
      }

      ENamedValues GetName( btUnsignedInt index,
                            btStringKey *pName)const {
         return m_namedvalues->GetName( index,pName);
      }

      btBool Has( btStringKey Name )const {
         return m_namedvalues->Has( Name );
      }
#endif // _NO_STRING_KEYS_

   protected:
      INamedValueSet *m_namedvalues;
   };

#endif //__cplusplus

/// @} group BasicTypes

END_NAMESPACE(AAL)

#endif //__AALSDK_AALNAMEDVALUESET_H__

