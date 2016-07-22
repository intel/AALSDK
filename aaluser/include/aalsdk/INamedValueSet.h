// Copyright(c) 2015-2016, Intel Corporation
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
/// @file INamedValueSet.h
/// @brief Interface for NamedValueSets.
/// @ingroup BasicTypes
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/26/2015     TSW      Split off from AALNamedValueSet.h@endverbatim
//****************************************************************************
#ifndef __AALSDK_INAMEDVALUESET_H__
#define __AALSDK_INAMEDVALUESET_H__
#include <aalsdk/osal/CriticalSection.h>

#define NVSFileIO /* for the time being, leave it in */

BEGIN_NAMESPACE(AAL)

/// @addtogroup BasicTypes
/// @{

/// Return codes for INamedValueSet methods.
typedef enum ENamedValues
{
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
   ENamedValuesInvalidReadToNull,
   ENamedValuesZeroSizedArray,
   ENamedValuesNullPointerArgument,
} ENamedValues;

#ifdef __cplusplus

/// Base public interface for Named Value Sets.
class AASLIB_API INamedValueSet
{
public:
   /// INamedValueSet Destructor.
   virtual ~INamedValueSet() {}

   /// Add btBool, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btBool Value)                          = 0;
   /// Add btByte, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btByte Value)                          = 0;
   /// Add bt32bitInt, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, bt32bitInt Value)                      = 0;
   /// Add btUnsigned32bitInt, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btUnsigned32bitInt Value)              = 0;
   /// Add bt64bitInt, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, bt64bitInt Value)                      = 0;
   /// Add btUnsigned64bitInt, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btUnsigned64bitInt Value)              = 0;
   /// Add btFloat, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btFloat Value)                         = 0;
   /// Add btcString, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, btcString Value)                       = 0;
   /// Add NamedValueSet, keyed by integer.
   virtual ENamedValues Add(btNumberKey Name, const INamedValueSet *Value)           = 0;
   /// Add byte array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of bytes to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            btByteArray        value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add signed 32-bit integer array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of signed 32-bit integers to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            bt32bitIntArray    value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add unsigned 32-bit integer array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of unsigned 32-bit integers to add.
   virtual ENamedValues Add(btNumberKey             Name,
                            btUnsigned32bitIntArray value,
                            btUnsigned32bitInt      NumElements)                     = 0;
   /// Add signed 64-bit integer array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of signed 64-bit integers to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            bt64bitIntArray    value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add unsigned 64-bit integer array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of unsigned 64-bit integers to add.
   virtual ENamedValues Add(btNumberKey             Name,
                            btUnsigned64bitIntArray value,
                            btUnsigned32bitInt      NumElements)                     = 0;
   /// Add btObjectType, keyed by integer.
   virtual ENamedValues Add(btNumberKey  Name, btObjectType value)                   = 0;
   /// Add floating-point array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of floating points to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            btFloatArray       value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add string array, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of strings to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            btStringArray      value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add generic objects, keyed by integer.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of generic objects to add.
   virtual ENamedValues Add(btNumberKey        Name,
                            btObjectArray      value,
                            btUnsigned32bitInt NumElements)                          = 0;

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
   virtual ENamedValues Get(btNumberKey Name, INamedValueSet const **pValue) const   = 0;
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
   virtual ENamedValues Delete(btNumberKey Name)                                     = 0;
   /// Determine the number of elements stored in an entry keyed by integer.
   ///
   /// @param[out]  pSize  Receives the number of elements.
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues GetSize(btNumberKey Name, btWSSize *pSize) const             = 0;
   /// Determine the type of element(s) stored in an entry keyed by integer.
   ///
   /// @param[out]  pType  Receives the type..
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues Type(btNumberKey Name, eBasicTypes *pType) const             = 0;
   /// Determine whether an entry exists for Name, an integer key.
   virtual btBool Has(btNumberKey Name) const                                        = 0;
   /// Retrieve the name (key) of the named-value pair at zero-based index.
   ///
   /// @param[in]   index  Zero-based index into named-value pairs.
   /// @param[out]  pName  Location to receive the name (integer).
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues GetName(btUnsignedInt index, btNumberKey *pName) const       = 0;


   /// Add btBool, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btBool Value)                          = 0;
   /// Add btByte, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btByte Value)                          = 0;
   /// Add bt32bitInt, keyed by string.
   virtual ENamedValues Add(btStringKey Name, bt32bitInt Value)                      = 0;
   /// Add btUnsigned32bitInt, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btUnsigned32bitInt Value)              = 0;
   /// Add bt64bitInt, keyed by string.
   virtual ENamedValues Add(btStringKey Name, bt64bitInt Value)                      = 0;
   /// Add btUnsigned64bitInt, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btUnsigned64bitInt Value)              = 0;
   /// Add btFloat, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btFloat Value)                         = 0;
   /// Add btcString, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btcString Value)                       = 0;
   /// Add NamedValueSet, keyed by string.
   virtual ENamedValues Add(btStringKey Name, const INamedValueSet *Value)           = 0;
   /// Add byte array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of bytes to add.
   virtual ENamedValues Add(btStringKey        Name,
                            btByteArray        value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add signed 32-bit integer array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of signed 32-bit integers to add.
   virtual ENamedValues Add(btStringKey        Name,
                            bt32bitIntArray    value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add unsigned 32-bit integer array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of unsigned 32-bit integers to add.
   virtual ENamedValues Add(btStringKey             Name,
                            btUnsigned32bitIntArray value,
                            btUnsigned32bitInt      NumElements)                     = 0;
   /// Add signed 64-bit integer array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of signed 64-bit integers to add.
   virtual ENamedValues Add(btStringKey        Name,
                            bt64bitIntArray    value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add unsigned 64-bit integer array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of unsigned 64-bit integers to add.
   virtual ENamedValues Add(btStringKey             Name,
                            btUnsigned64bitIntArray value,
                            btUnsigned32bitInt      NumElements)                     = 0;
   /// Add btObjectType, keyed by string.
   virtual ENamedValues Add(btStringKey Name, btObjectType value)                    = 0;
   /// Add floating-point array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of floating points to add.
   virtual ENamedValues Add(btStringKey        Name,
                            btFloatArray       value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add string array, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of strings to add.
   virtual ENamedValues Add(btStringKey        Name,
                            btStringArray      value,
                            btUnsigned32bitInt NumElements)                          = 0;
   /// Add generic objects, keyed by string.
   ///
   /// @param[in]  Name         Key.
   /// @param[in]  value        Pointer to the buffer to be added.
   /// @param[in]  NumElements  Number of generic objects to add.
   virtual ENamedValues Add(btStringKey        Name,
                            btObjectArray      value,
                            btUnsigned32bitInt NumElements)                          = 0;

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
   virtual ENamedValues Get(btStringKey Name, INamedValueSet const **pValue) const   = 0;
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
   virtual ENamedValues Delete(btStringKey Name)                                     = 0;
   /// Determine the number of elements stored in an entry keyed by string.
   ///
   /// @param[out]  pSize  Receives the number of elements.
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues GetSize(btStringKey Name, btWSSize *pSize) const             = 0;
   /// Determine the type of element(s) stored in an entry keyed by string.
   ///
   /// @param[out]  pType  Receives the type..
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues Type(btStringKey Name,eBasicTypes *pType) const              = 0;
   /// Determine whether an entry exists for Name, a string key.
   virtual btBool Has(btStringKey Name) const                                        = 0;
   /// Retrieve the name (key) of the named-value pair at zero-based index.
   ///
   /// @param[in]   index  Zero-based index into named-value pairs.
   /// @param[out]  pName  Location to receive the name (string).
   ///
   /// @retval ENamedValuesNameNotFound  Name not found.
   /// @retval ENamedValuesOK            On success.
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const       = 0;


   /// Force the Named Value Set to delete all its members.
   virtual ENamedValues Empty()                                                      = 0;
   /// Determine whether a Named Value Set is a subset of another.
   virtual btBool Subset(const INamedValueSet &rOther) const                         = 0;
   /// Test for equality.
   virtual btBool operator == (const INamedValueSet &rOther) const                   = 0;
   /// Determine the number of name/value pairs held in the Named Value Set.
   virtual ENamedValues GetNumNames(btUnsignedInt *pNum) const                       = 0;
   /// Determine the key type of the named-value pair at zero-based index.
   virtual ENamedValues GetNameType(btUnsignedInt index,
                                    eNameTypes   *pType) const                       = 0;

   virtual ENamedValues     Read(std::istream & )                                    = 0;
   virtual ENamedValues    Write(std::ostream & )            const                   = 0;
   virtual ENamedValues    Write(std::ostream & , unsigned ) const                   = 0;
   // Serialize NVS with ending demarcation
   virtual ENamedValues WriteOne(std::ostream & , unsigned ) const                   = 0;

#ifdef NVSFileIO

   /// @brief  Reads an open (binary) file and creates an NVS from it.
   virtual ENamedValues  Read(FILE * )                                               = 0;
   virtual ENamedValues Write(FILE * )               const                           = 0;
   virtual ENamedValues Write(FILE * , unsigned )    const                           = 0;
   /// @brief Serialize NVS with ending demarcation
   virtual ENamedValues WriteOne(FILE * , unsigned ) const                           = 0;

#endif // NVSFileIO


   /// @brief Merges one NamedValueSet into another
   ///
   /// @param[in] INamedValueSet nvsInput and nvsOutput will be merged together
   /// @return ENamedValuesOK for success, appropriate value otherwise
   ///
   /// If there are duplicate names, the original value in the
   ///                   nvsOutput takes precedence and there is no error
   virtual ENamedValues Merge(const INamedValueSet & )                               = 0;


   /// @brief Converts a std::string length into an NamedValueSet
   ///
   /// @param[in] std::string & containing the serialized representation of the NamedValueSet
   ///
   /// output nvs is a non-constant reference to the returned NamedValueSet
   virtual ENamedValues FromStr(const std::string & )                                = 0;

   //=============================================================================
   // Name:        NamedValueSetFromCharString
   // Description: Convert a char* + length into an NVS
   // Interface:   public
   // Inputs:      char*, length of char* including a final terminating null.
   //              Note that the string itself may contain embedded nulls; they
   //              do not terminate the string. That is, the char* is not a normal
   //              NULL-terminated string
   // Outputs:     nvs is a non-const reference to the returned NamedValueSet
   //=============================================================================
   /// @brief converts a char* + length into an NVS
   ///
   virtual ENamedValues FromStr(void * , btWSSize )                                  = 0;

   //=============================================================================
   // Name:        StdStringFromNamedValueSet
   // Description: Return a std::string containing the serialized NamedValueSet
   // Interface:   public
   // Inputs:      const NamedValueSet& nvs
   // Returns:     As in description. Note that the returned string.length() is
   //              the correct length to allocate for a buffer in which to store
   //              the buffer.
   // NOTE:        See CharFromString for important usage note
   //=============================================================================
   /// @brief Returns a std::string containing the serialized NamedValueSet
   virtual std::string ToStr() const                                                 = 0;

protected:
   // Make this equal to the given INamedValueSet.
   virtual ENamedValues               Copy(const INamedValueSet & )                  = 0;
   // Create a new identical copy of this object.
   virtual INamedValueSet *          Clone() const                                   = 0;
   // Retrieve a pointer to the concrete object.
   virtual INamedValueSet const * Concrete() const                                   = 0;

   friend class CValue;
   friend class CNamedValueSet;
   friend class NamedValueSet;
};

AASLIB_API std::ostream & operator << (std::ostream & , const INamedValueSet & );
AASLIB_API std::istream & operator >> (std::istream & ,       INamedValueSet & );

// NamedValueSet Factory Methods
AASLIB_API INamedValueSet * NewNVS();
AASLIB_API void DeleteNVS(INamedValueSet * );

//=============================================================================
// Name: CValue
// Description: Container class for values stored by NamedValueSet. Single values
//              are stored directly in a member of union type Val_t.  Complex
//              types like arrays, strings and NVS are stored indirectly
//              through pointers. String Array is particularly involved.
//              The container allocates memory when storing complex types and
//              deletes on destroy.
//     Comment: Better performance might be obtained by making this a counted
//              object and only copying on modify. It is recommended to
//              pass these and the NamedValueSet object that holds them by
//              reference.
//=============================================================================
class AASLIB_API CValue
{
   //Simple types
   typedef union Val_t
   {
      btBool                  _1b;
      btByte                  _8b;
      btByteArray             _8bA;
      bt32bitInt              _32b;
      bt32bitIntArray         _32bA;
      btUnsigned32bitInt      _U32b;
      btUnsigned32bitIntArray _U32bA;
      bt64bitInt              _64b;
      bt64bitIntArray         _64bA;
      btUnsigned64bitInt      _U64b;
      btUnsigned64bitIntArray _U64bA;
      btFloat                 flt;
      btFloatArray            fltA;
      btString                str;
      btStringArray           strA;
      btObjectType            Obj;
      btObjectArray           ObjA;
      INamedValueSet         *pNVS;
   } Val_t;

   btUnsigned32bitInt m_Size;     // 32 bits
   eBasicTypes        m_Type;     // 32 bits
   Val_t              m_Val;      // 64 bits
                                  // Total 16 bytes
public:
   //=======================================================================
   //Constructor
   //=======================================================================
   CValue();

   //=======================================================================
   // Copy Constructor
   //=======================================================================
   CValue(const CValue &rOther);

   //=======================================================================
   //Destructor
   //=======================================================================
   ~CValue();

   //=======================================================================
   //Assignment
   //=======================================================================
   CValue & operator = (const CValue &rOther);

   //=======================================================================
   //Type Accessors
   //=======================================================================
   eBasicTypes        Type() const { return m_Type; }
   btUnsigned32bitInt Size() const { return m_Size; }

   //=======================================================================
   //Single CValue Mutators
   //=======================================================================
   void Put(btBool );
   void Put(btByte );
   void Put(bt32bitInt );
   void Put(btUnsigned32bitInt );
   void Put(bt64bitInt );
   void Put(btUnsigned64bitInt );
   void Put(btFloat );
   void Put(btObjectType );

   //=======================================================================
   // Complex CValue Mutators
   //=======================================================================
   void Put(const INamedValueSet * );
   void Put(btcString );

   //=======================================================================
   //Array CValue Mutators
   //=======================================================================
   void Put(btByteArray ,             btUnsigned32bitInt );
   void Put(bt32bitIntArray ,         btUnsigned32bitInt );
   void Put(btUnsigned32bitIntArray , btUnsigned32bitInt );
   void Put(bt64bitIntArray ,         btUnsigned32bitInt );
   void Put(btUnsigned64bitIntArray , btUnsigned32bitInt );
   void Put(btFloatArray ,            btUnsigned32bitInt );
   void Put(btStringArray ,           btUnsigned32bitInt );
   void Put(btObjectArray ,           btUnsigned32bitInt );

   //=======================================================================
   //Accessors
   //=======================================================================
   ENamedValues Get(bt32bitInt * )              const;
   ENamedValues Get(btBool * )                  const;
   ENamedValues Get(btByte * )                  const;
   ENamedValues Get(btUnsigned32bitInt * )      const;
   ENamedValues Get(bt64bitInt * )              const;
   ENamedValues Get(btUnsigned64bitInt * )      const;
   ENamedValues Get(btFloat * )                 const;
   ENamedValues Get(btcString * )               const;
   ENamedValues Get(INamedValueSet const ** )   const;
   ENamedValues Get(btObjectType * )            const;

   ENamedValues Get(btByteArray * )             const;
   ENamedValues Get(bt32bitIntArray * )         const;
   ENamedValues Get(btUnsigned32bitIntArray * ) const;
   ENamedValues Get(bt64bitIntArray * )         const;
   ENamedValues Get(btUnsigned64bitIntArray * ) const;
   ENamedValues Get(btFloatArray * )            const;
   ENamedValues Get(btStringArray * )           const;
   ENamedValues Get(btObjectArray * )           const;
}; // CValue

#endif // __cplusplus

/// @}

END_NAMESPACE(AAL)

#endif //__AALSDK_INAMEDVALUESET_H__

