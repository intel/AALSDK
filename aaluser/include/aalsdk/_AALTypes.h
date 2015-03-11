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
/// @file _AALTypes.h
/// @brief Defines the scalar type abstraction used by AAL.
/// @ingroup BasicTypes
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///  AUTHORS: Joseph Grecco, Intel Corporation
///           Henry Mitchel, Intel Corporation
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/15/2005     JG       Initial version started
/// 04/24/2006     JG       Added Byte and NamedValues
/// 03/05/2007     JG       Added Message header type
/// 03/19/2007     JG       Linux Port
/// 07/30/2007     HM       Added new btEndOfNVS_t, not really a
///                            type, needed to support NVSReadNVS
///                            of embedded NVS's
/// 08/11/2007     HM       Modified btEndOfNVS_t to be statically
///                            defined to a high number to avoid
///                            forward compatibility problems
/// 10/04/2007     JG       Renamed to AALTypes.h
/// 11/21/2007     HM       Add btcString type
/// 05/01/2008     JG       Added byByteArray
/// 05/07/2008     HM       Tweaks to btByteArray,
///                            removed Message Header
/// 05/08/2008     HM       Comments & License
/// 05/28/2008     HM       Clean up key types preparatory to making
///                            numeric key 64 bits, finish btcString
/// 05/29/2008     HM       Convert btcString and btUnsignedInt key types into
///                            specific typedefs: btStringKey and btNumberKey
/// 01/04/2009     HM       Updated Copyright
/// 03/19/2012     TSW      Add size-agnostic macros.
/// 10/24/2012     TSW      Cleanup for faplib.@endverbatim
//****************************************************************************
#ifndef __AALSDK__AALTYPES_H__
#define __AALSDK__AALTYPES_H__

#if HAVE_STDINT_H
# include <stdint.h>
#endif // HAVE_STDINT_H

BEGIN_NAMESPACE(AAL)

/// @addtogroup BasicTypes
/// @{


#ifndef __AALSDK_DATA_MODEL_LP32
# define __AALSDK_DATA_MODEL_LP32  0
#endif // __AALSDK_DATA_MODEL_LP32
#ifndef __AALSDK_DATA_MODEL_ILP32 
# define __AALSDK_DATA_MODEL_ILP32 1
#endif // __AALSDK_DATA_MODEL_ILP32
#ifndef __AALSDK_DATA_MODEL_ILP64
# define __AALSDK_DATA_MODEL_ILP64 2
#endif // __AALSDK_DATA_MODEL_ILP64
#ifndef __AALSDK_DATA_MODEL_LLP64 
# define __AALSDK_DATA_MODEL_LLP64 3
#endif // __AALSDK_DATA_MODEL_LLP64
#ifndef __AALSDK_DATA_MODEL_LP64
# define __AALSDK_DATA_MODEL_LP64  4
#endif // __AALSDK_DATA_MODEL_LP64

// The Windows compiler doesn't define these, so we compensate here.
#if defined( __AAL_WINDOWS__ )
# if   defined( _WIN64 )
#    ifndef __LLP64__
#       define __LLP64__ 1
#    endif // __LLP64__
# elif defined( _WIN32 )
#    ifndef __ILP32__
#       define __ILP32__ 1
#    endif // __ILP32__
# endif // _WIN64
#endif // __AAL_WINDOWS__

#ifndef __AALSDK_DATA_MODEL
# if defined( __LP32__ )
#   define __AALSDK_DATA_MODEL __AALSDK_DATA_MODEL_LP32
#   define sizeof_void_ptr 4
# elif defined( __ILP32__ )
#   define __AALSDK_DATA_MODEL __AALSDK_DATA_MODEL_ILP32
#   define sizeof_void_ptr 4
# elif defined( __ILP64__ )
#   define __AALSDK_DATA_MODEL __AALSDK_DATA_MODEL_ILP64
#   define sizeof_void_ptr 8
# elif defined( __LLP64__ )
#   define __AALSDK_DATA_MODEL __AALSDK_DATA_MODEL_LLP64
#   define sizeof_void_ptr 8
# elif defined( __LP64__ )
#   define __AALSDK_DATA_MODEL __AALSDK_DATA_MODEL_LP64
#   define sizeof_void_ptr 8
# endif
#endif // __AALSDK_DATA_MODEL

#ifndef __AALSDK_DATA_MODEL
# error Could not detect data model of current platform. (__AALSDK_DATA_MODEL)
#endif // __AALSDK_DATA_MODEL


#if defined( __AAL_WINDOWS__ )
# ifndef __PHYS_ADDR_DEFINED
# define __PHYS_ADDR_DEFINED 1
// UINT_PTR defined in minwindef.h
     typedef UINT_PTR btPhysAddr;
# endif // __PHYS_ADDR_DEFINED
# ifndef __PHYS_ADDR_CAST
#    define __PHYS_ADDR_CAST(p) ( (UINT_PTR)(p) )
# endif // __PHYS_ADDR_CAST

# ifndef __VIRT_ADDR_DEFINED
# define __VIRT_ADDR_DEFINED 1
// LPBYTE defined in minwindef.h
     typedef LPBYTE btVirtAddr;
# endif // __VIRT_ADDR_DEFINED

# ifdef HAVE_UINTPTR_T
     typedef uintptr_t btUIntPtr;
# else
#    define HAVE_UINTPTR_T 1
// UINT_PTR defined in minwindef.h
     typedef UINT_PTR btUIntPtr;
# endif // HAVE_UINTPTR_T
# ifndef __UINTPTR_T_CAST
#    define __UINTPTR_T_CAST(n) ( (UINT_PTR)(n) )
# endif // __UINTPTR_T_CAST

# ifndef __UINT64_T_DEFINED
# define __UINT64_T_DEFINED 1
// UINT64 defined in basetsd.h
     END_NAMESPACE(AAL)
     typedef UINT64 uint64_t;
     BEGIN_NAMESPACE(AAL)
# endif // __UINT64_T_DEFINED

# ifndef __SIZE_T_CAST
// size_t defined in crtdefs.h
#    define __SIZE_T_CAST(s) ( (size_t)(s) )
# endif // __SIZE_T_CAST

#else
# ifndef __VIRT_ADDR_DEFINED
# define __VIRT_ADDR_DEFINED 1
     typedef unsigned char * btVirtAddr;
# endif // __VIRT_ADDR_DEFINED
#endif // OS
CASSERT(sizeof_void_ptr == sizeof(btVirtAddr));


#if ( __AALSDK_DATA_MODEL_LLP64 == __AALSDK_DATA_MODEL )
# ifndef __UINT64_T_CONST
#    define __UINT64_T_CONST(c) c##ULL
# endif // __UINT64_T_CONST()
# ifndef PRIu64
#    define PRIu64              "llu"
# endif // PRIu64
# ifndef PRIx64
#    define PRIx64              "llx"
# endif // PRIx64
# ifndef PRIX64
#    define PRIX64              "llX"
# endif // PRIX64
#elif defined( __AAL_LINUX__ ) && defined( __x86_64__ )
// Linux deviates from the LP64 model here.
# ifndef __UINT64_T_CONST
#    define __UINT64_T_CONST(c) c##ULL
# endif // __UINT64_T_CONST()
# ifndef PRIu64
#    define PRIu64              "llu"
# endif // PRIu64
# ifndef PRIx64
#    define PRIx64              "llx"
# endif // PRIx64
# ifndef PRIX64
#    define PRIX64              "llX"
# endif // PRIX64
#elif ( __AALSDK_DATA_MODEL_ILP64 == __AALSDK_DATA_MODEL ) || ( __AALSDK_DATA_MODEL_LP64 == __AALSDK_DATA_MODEL )
# ifndef __UINT64_T_CONST
#    define __UINT64_T_CONST(c) c##UL
# endif // __UINT64_T_CONST()
# ifndef PRIu64
#    define PRIu64              "lu"
# endif // PRIu64
# ifndef PRIx64
#    define PRIx64              "lx"
# endif // PRIx64
# ifndef PRIX64
#    define PRIX64              "lX"
# endif // PRIX64
#else
// Default to unsigned long long for the rest
# ifndef __UINT64_T_CONST
#    define __UINT64_T_CONST(c) c##ULL
# endif // __UINT64_T_CONST()
# ifndef PRIu64
#    define PRIu64              "llu"
# endif // PRIu64
# ifndef PRIx64
#    define PRIx64              "llx"
# endif // PRIx64
# ifndef PRIX64
#    define PRIX64              "llX"
# endif // PRIX64
#endif // __AALSDK_DATA_MODEL


#if ( __AALSDK_DATA_MODEL_LLP64 == __AALSDK_DATA_MODEL )
// (long long's and pointers are the same size)

# ifndef __PHYS_ADDR_DEFINED
# define __PHYS_ADDR_DEFINED 1
     typedef unsigned long long int btPhysAddr;
# endif // __PHYS_ADDR_DEFINED
# ifndef __PHYS_ADDR_CAST
#    define __PHYS_ADDR_CAST(p) ( (unsigned long long int)(p) )
# endif // __PHYS_ADDR_CAST
# ifndef __PHYS_ADDR_CONST
#    define __PHYS_ADDR_CONST(c) c##ULL
# endif // __PHYS_ADDR_CONST
# ifndef PRIuPHYS_ADDR
#    define PRIuPHYS_ADDR        "llu"
# endif // PRIuPHYS_ADDR
# ifndef PRIxPHYS_ADDR
#    define PRIxPHYS_ADDR        "llx"
# endif // PRIxPHYS_ADDR
# ifndef PRIXPHYS_ADDR
#    define PRIXPHYS_ADDR        "llX"
# endif // PRIXPHYS_ADDR

# ifdef HAVE_UINTPTR_T
     typedef uintptr_t btUIntPtr;
# else
#    define HAVE_UINTPTR_T 1
     typedef unsigned long long int btUIntPtr;
# endif // HAVE_UINTPTR_T
# ifndef __UINTPTR_T_CAST
#    define __UINTPTR_T_CAST(n) ( (unsigned long long int)(n) )
# endif // __UINTPTR_T_CAST
# ifndef __UINTPTR_T_CONST
#    define __UINTPTR_T_CONST(c) c##ULL
# endif // __UINTPTR_T_CONST
# ifndef PRIuUINTPTR_T
#    define PRIuUINTPTR_T        "llu"
# endif // PRIuUINTPTR_T
# ifndef PRIxUINTPTR_T
#    define PRIxUINTPTR_T        "llx"
# endif // PRIxUINTPTR_T
# ifndef PRIXUINTPTR_T
#    define PRIXUINTPTR_T        "llX"
# endif // PRIXUINTPTR_T

# ifndef __SIZE_T_CAST
#    define __SIZE_T_CAST(s) ( (unsigned long long int)(s) )
# endif // __SIZE_T_CAST
# ifndef __SIZE_T_CONST
#    define __SIZE_T_CONST(c) c##ULL
# endif // __SIZE_T_CONST
# ifndef PRIuSIZE_T
#    define PRIuSIZE_T        "llu"
# endif // PRIuSIZE_T
# ifndef PRIxSIZE_T
#    define PRIxSIZE_T        "llx"
# endif // PRIxSIZE_T
# ifndef PRIXSIZE_T
#    define PRIXSIZE_T        "llX"
# endif // PRIXSIZE_T
// (long long's and pointers are the same size)

#else

// __AALSDK_DATA_MODEL_LP32
// __AALSDK_DATA_MODEL_ILP32
// __AALSDK_DATA_MODEL_ILP64
// __AALSDK_DATA_MODEL_LP64
// (long's and pointers are the same size)
# ifndef __PHYS_ADDR_DEFINED
# define __PHYS_ADDR_DEFINED 1
     typedef unsigned long int btPhysAddr;
# endif // __PHYS_ADDR_DEFINED
# ifndef __PHYS_ADDR_CAST
#    define __PHYS_ADDR_CAST(p)  ( (unsigned long int)(p) )
# endif // __PHYS_ADDR_CAST
# ifndef __PHYS_ADDR_CONST
#    define __PHYS_ADDR_CONST(c) c##UL
# endif // __PHYS_ADDR_CONST
# ifndef PRIuPHYS_ADDR
#    define PRIuPHYS_ADDR        "lu"
# endif // PRIuPHYS_ADDR
# ifndef PRIxPHYS_ADDR
#    define PRIxPHYS_ADDR        "lx"
# endif // PRIxPHYS_ADDR
# ifndef PRIXPHYS_ADDR
#    define PRIXPHYS_ADDR        "lX"
# endif // PRIXPHYS_ADDR

# ifdef HAVE_UINTPTR_T
     typedef uintptr_t btUIntPtr;
# else
#    define HAVE_UINTPTR_T 1
     typedef unsigned long int btUIntPtr;
# endif // HAVE_UINTPTR_T
# ifndef __UINTPTR_T_CAST
#    define __UINTPTR_T_CAST(n)  ( (unsigned long int)(n) )
# endif // __UINTPTR_T_CAST
# ifndef __UINTPTR_T_CONST
#    define __UINTPTR_T_CONST(c) c##UL
# endif // __UINTPTR_T_CONST
# ifndef PRIuUINTPTR_T
#    define PRIuUINTPTR_T        "lu"
# endif // PRIuUINTPTR_T
# ifndef PRIxUINTPTR_T
#    define PRIxUINTPTR_T        "lx"
# endif // PRIxUINTPTR_T
# ifndef PRIXUINTPTR_T
#    define PRIXUINTPTR_T        "lX"
# endif // PRIXUINTPTR_T

# ifndef __SIZE_T_CAST
#    define __SIZE_T_CAST(s)  ( (unsigned long int)(s) )
# endif // __SIZE_T_CAST
# ifndef __SIZE_T_CONST
#    define __SIZE_T_CONST(c) c##UL
# endif // __SIZE_T_CONST
# ifndef PRIuSIZE_T
#    define PRIuSIZE_T        "lu"
# endif // PRIuSIZE_T
# ifndef PRIxSIZE_T
#    define PRIxSIZE_T        "lx"
# endif // PRIxSIZE_T
# ifndef PRIXSIZE_T
#    define PRIXSIZE_T        "lX"
# endif // PRIXSIZE_T

// (long's and pointers are the same size)
#endif // __AALSDK_DATA_MODEL
CASSERT(sizeof_void_ptr == sizeof(btPhysAddr));
CASSERT(sizeof_void_ptr == sizeof(btUIntPtr));


# ifndef PRIuID
#    define PRIuID  "llu"
# endif // PRIuID
# ifndef PRIxID
#    define PRIxID  "llx"
# endif // PRIxID
# ifndef PRIXID
#    define PRIXID  "llX"
# endif // PRIXID

# ifndef PRIuIID
#    define PRIuIID "llu"
# endif // PRIuIID
# ifndef PRIxIID
#    define PRIxIID "llx"
# endif // PRIxIID
# ifndef PRIXIID
#    define PRIXIID "llX"
# endif // PRIXIID


#ifndef __AALSDK_BASIC_TYPES_DEFINED
#define __AALSDK_BASIC_TYPES_DEFINED

/// Definition of basic types recognized by AAL.
typedef enum eBasicTypes {
   // Allows use as an array index, if needed
   btBool_t = 0,              ///< btBool
   btByte_t,                  ///< btByte
   bt32bitInt_t,              ///< bt32bitInt
   btInt_t,                   ///< btInt
   btUnsigned32bitInt_t,      ///< btUnsigned32bitInt
   btUnsignedInt_t,           ///< btUnsignedInt
   bt64bitInt_t,              ///< bt64bitInt
   btUnsigned64bitInt_t,      ///< btUnsigned64bitInt
   btFloat_t,                 ///< btFloat
   btString_t,                ///< btString
   btNamedValueSet_t,         ///< NamedValueSet
   bt32bitIntArray_t,         ///< bt32bitIntArray
   btUnsigned32bitIntArray_t, ///< btUnsigned32bitIntArray
   bt64bitIntArray_t,         ///< bt64bitIntArray
   btUnsigned64bitIntArray_t, ///< btUnsigned64bitIntArray
   btObjectType_t,            ///< btObjectType
   btFloatArray_t,            ///< btFloatArray
   btStringArray_t,           ///< btStringArray
   btObjectArray_t,           ///< btObjectArray
   btByteArray_t,             ///< btByteArray
   // Always insert new types above this comment
   btUnknownType_t,           ///< Type is unknown.
   btEndOfNVS_t = 9999        ///< Special value for end of NamedValueSet.
} eBasicTypes;

/// Name types for NamedValueSet's
typedef enum eNameTypes {
   // Numbering provides backward compatibility for now
   btNumberKey_t = 5,         ///< btNumberKey
   btStringKey_t = 9          ///< btStringKey
} eNameTypes;


/// @brief UUID identifier for AAL objects, events, and error codes.
///
/// btID is an integer type which encapsulates a hierarchical representation of an ID space. The ID space
/// is used to represent objects, events, and error codes in a uniform manner that is efficient, definitive in
/// code (i.e. codes can be switched on), and unique in the binary space (so that if seen out of context they
/// can still be identified).
///
/// A btID incorporates 5 fields:
/// - Vendor: 16 bits that encode the PCI-SIG Vendor ID for a vendor.
///   - Intel’s value is 0x8086.
///   - The AAL system itself is 0x0000, as AAL may not always be completely Intel specific.
///   - 0xFFFF is currently used as a special value for those wishing to test their code without requiring a PCI-SIG
/// vendor ID.
/// - Sub-System: 12 bits that encode a vendor specific system or subsystem. This address space belongs to the
/// vendor.
///   - AAL uses the sub-system field to identify AAL subsystems, such as the Factory, the Registrar, etc.
/// - Exception: 1 bit that is set if an object represents an exception.
/// - Type: 15 bits that represents an object’s type, or in the special case of events, also whether the event is
/// just an event, is a Transaction Event, is an exception event, or is an exception Transaction Event.
/// - Item: 16 bits enumerating specific details about the ID.
///
/// For example, the specific Transaction Event returned after the SystemInit() call has executed is represented
/// uniquely by the symbol tranevtSystemInit, which is an AAL_ID composed of:
/// - Vendor:    AAL
/// - Subsystem: sysAAS
/// - Exception: No
/// - Type:      Transaction Event
/// - Item:      1 (Init)
///
/// It is expected that this hierarchical type system will be extended by vendors based upon their PCI-SIG
/// Vendor ID.
#if defined( __AAL_WINDOWS__ )
   typedef ULONGLONG   btID;
   typedef LPVOID      btGenericInterface;   ///< Application casts to proper interface pointer.
   typedef LPVOID      btApplicationContext; ///< Generic context type. Application casts to proper implementation-specific pointer.
// these Windows types defined in ntdef.h
# ifdef __cplusplus
   typedef bool        btBool;
   typedef bool       *btBoolArray;
# else
   typedef BYTE        btBool;
   typedef LPBYTE     *btBoolArray;
# endif // __cplusplus
   typedef LPSTR       btString;
   typedef LPSTR      *btStringArray;
   typedef CHAR        btByte;
   typedef PCHAR       btByteArray;
   typedef LPVOID      btObjectType;
   typedef LPVOID      btAny;
   typedef LPVOID     *btObjectArray;
   typedef LPCVOID     btcObjectType;
   typedef LPCSTR      btcString;
   typedef LPCSTR     *btcStringArray;
// these Windows types (plus ULONGLONG) defined in ntintsafe.h
# ifndef __AAL_INT32_DEFINED
# define __AAL_INT32_DEFINED 1
     typedef INT32     bt32bitInt;                ///< Signed 32-bit integer.
     typedef INT32    *bt32bitIntArray;           ///< Signed 32-bit integer array.
     typedef UINT32    btUnsigned32bitInt;        ///< Unsigned 32-bit integer.
     typedef UINT32   *btUnsigned32bitIntArray;   ///< Unsigned 32-bit integer array.
# endif // __AAL_INT32_DEFINED
# ifndef __AAL_INT64_DEFINED
# define __AAL_INT64_DEFINED 1
     typedef INT64     bt64bitInt;                ///< Signed 64-bit integer.
     typedef INT64    *bt64bitIntArray;           ///< Signed 64-bit integer array.
     typedef UINT64    btUnsigned64bitInt;        ///< Unsigned 64-bit integer.
     typedef UINT64   *btUnsigned64bitIntArray;   ///< Unsigned 64-bit integer array.
# endif // __AAL_INT64_DEFINED
   typedef HANDLE      btPID;                     ///< Process Identifier
   typedef HANDLE      btTID;                     ///< Thread Identifier
   typedef HANDLE      btHANDLE;
#else // ! __AAL_WINDOWS__
   typedef unsigned long long btID;
   typedef void              *btGenericInterface;   ///< Application casts to proper interface pointer.
   typedef void              *btApplicationContext; ///< Generic context type. Application casts to proper implementation-specific pointer.
# ifdef __cplusplus
     typedef bool             btBool;
     typedef bool            *btBoolArray;
# else
     typedef unsigned char    btBool;
     typedef unsigned char   *btBoolArray;
# endif // __cplusplus
   typedef char              *btString;
   typedef char             **btStringArray;
   typedef char               btByte;
   typedef char              *btByteArray;
   typedef void              *btObjectType;
   typedef void              *btAny;
   typedef void             **btObjectArray;
   typedef const void        *btcObjectType;
   typedef const char        *btcString;
   typedef const char       **btcStringArray;
   typedef void              *btHANDLE;
   typedef void              *HMODULE;
#endif // OS
typedef btID                  btIID;              ///< Integer interface identifier.
typedef int                   btInt;
typedef unsigned int          btUnsignedInt;
typedef float                 btFloat;
typedef float                *btFloatArray;
typedef unsigned short int    btUnsigned16bitInt; ///< Unsigned 16-bit integer.

#ifndef __AAL_INT32_DEFINED
#define __AAL_INT32_DEFINED 1
# if ( __AALSDK_DATA_MODEL_LP32 == __AALSDK_DATA_MODEL )
     typedef long int                bt32bitInt;              ///< Signed 32-bit integer.
     typedef long int               *bt32bitIntArray;         ///< Signed 32-bit integer array.
     typedef unsigned long int       btUnsigned32bitInt;      ///< Unsigned 32-bit integer.
     typedef unsigned long int      *btUnsigned32bitIntArray; ///< Unsigned 32-bit integer array.
# elif ( __AALSDK_DATA_MODEL_ILP32 == __AALSDK_DATA_MODEL ) || ( __AALSDK_DATA_MODEL_LLP64 == __AALSDK_DATA_MODEL ) || ( __AALSDK_DATA_MODEL_LP64 == __AALSDK_DATA_MODEL )
     typedef int                     bt32bitInt;              ///< Signed 32-bit integer.
     typedef int                    *bt32bitIntArray;         ///< Signed 32-bit integer array.
     typedef unsigned int            btUnsigned32bitInt;      ///< Unsigned 32-bit integer.
     typedef unsigned int           *btUnsigned32bitIntArray; ///< Unsigned 32-bit integer array.
# else
// __AALSDK_DATA_MODEL_ILP64 requires int32 extension
     typedef int32                   bt32bitInt;              ///< Signed 32-bit integer.
     typedef int32                  *bt32bitIntArray;         ///< Signed 32-bit integer array.
     typedef uint32                  btUnsigned32bitInt;      ///< Unsigned 32-bit integer.
     typedef uint32                 *btUnsigned32bitIntArray; ///< Unsigned 32-bit integer array.
# endif // __AALSDK_DATA_MODEL
#endif // __AAL_INT32_DEFINED

#ifndef __AAL_INT64_DEFINED
#define __AAL_INT64_DEFINED 1
# if ( __AALSDK_DATA_MODEL_LLP64 == __AALSDK_DATA_MODEL ) || ( defined( __AAL_LINUX__ ) && defined( __x86_64__ ) )
     typedef long long int           bt64bitInt;              ///< Signed 64-bit integer.
     typedef long long int          *bt64bitIntArray;         ///< Signed 64-bit integer array.
     typedef unsigned long long int  btUnsigned64bitInt;      ///< Unsigned 64-bit integer.
     typedef unsigned long long int *btUnsigned64bitIntArray; ///< Unsigned 64-bit integer array.
# elif ( __AALSDK_DATA_MODEL_ILP64 == __AALSDK_DATA_MODEL ) || ( __AALSDK_DATA_MODEL_LP64 == __AALSDK_DATA_MODEL )
     typedef long int                bt64bitInt;              ///< Signed 64-bit integer.
     typedef long int               *bt64bitIntArray;         ///< Signed 64-bit integer array.
     typedef unsigned long int       btUnsigned64bitInt;      ///< Unsigned 64-bit integer.
     typedef unsigned long int      *btUnsigned64bitIntArray; ///< Unsigned 64-bit integer array.
# else
// Default to unsigned long long for the rest
     typedef long long int           bt64bitInt;              ///< Signed 64-bit integer.
     typedef long long int          *bt64bitIntArray;         ///< Signed 64-bit integer array.
     typedef unsigned long long int  btUnsigned64bitInt;      ///< Unsigned 64-bit integer.
     typedef unsigned long long int *btUnsigned64bitIntArray; ///< Unsigned 64-bit integer array.
# endif // __AALSDK_DATA_MODEL
#endif // __AAL_INT64_DEFINED

# ifdef __AAL_LINUX__
   typedef pid_t                     btPID;                   ///< Process Identifier
// pthreads may define pthread_t as a 64-bit value.
   typedef btUnsigned64bitInt        btTID;                   ///< Thread Identifier
# endif // __AAL_LINUX__

// Key types for NVS's
typedef btcString          btStringKey;
typedef btString           btmStringKey; // Modifiable version of btStringKey, no associated eNameTypes
typedef btUnsigned64bitInt btNumberKey;  // WARNING: sizeof(btNumberKey) tracked in ResMgrUtilities::KeyNameFromDeviceAddress()

typedef btUnsigned64bitInt btWSID;       ///< Workspace ID type.
typedef btUnsigned64bitInt btWSSize;     ///< Workspace size type.

typedef btUnsigned32bitInt btCSROffset;  ///< Configuration & Status Register offset.
typedef btUnsigned64bitInt btCSRValue;   ///< Configuration & Status Register value.
typedef btUnsigned32bitInt bt32bitCSR;   ///< 32-bit Configuration & Status Register value.
typedef btUnsigned64bitInt bt64bitCSR;   ///< 64-bit Configuration & Status Register value.
#ifndef AAL_INVALID_CSR_VALUE
# define AAL_INVALID_CSR_VALUE ( (btCSRValue)-1 )
#endif // AAL_INVALID_CSR_VALUE

typedef btUnsigned64bitInt btTime;       ///< Generic time value.
#ifndef AAL_INFINITE_WAIT
# ifdef __cplusplus
/// Signifies that a call which accepts a timeout value is to become a blocking call.
#    define AAL_INFINITE_WAIT ( (AAL::btTime)-1 )
# else
#    define AAL_INFINITE_WAIT ( (btTime)-1 )
# endif // __cplusplus
#endif // AAL_INFINITE_WAIT

#ifdef __cplusplus
class IEvent;
/// @brief btEventHandler is a user-provided callback routine used for dispatching events from the
///   Event Delivery Service.
typedef void (*btEventHandler)(IEvent const &TheEvent);
#else
typedef void (*btEventHandler)(void *TheEvent);
#endif // __cplusplus

#endif // __AALSDK_BASIC_TYPES_DEFINED


/// @} group BasicTypes


CASSERT(true);
CASSERT(!false);
CASSERT(1 == sizeof(btBool)            );
CASSERT(1 == sizeof(btByte)            );
CASSERT(2 == sizeof(btUnsigned16bitInt));
CASSERT(4 == sizeof(bt32bitInt)        );
CASSERT(4 == sizeof(btUnsigned32bitInt));
CASSERT(4 == sizeof(btFloat)           );
CASSERT(8 == sizeof(bt64bitInt)        );
CASSERT(8 == sizeof(btUnsigned64bitInt));

CASSERT(sizeof_void_ptr == sizeof(btBoolArray)         );
CASSERT(sizeof_void_ptr == sizeof(btFloatArray)        );
CASSERT(sizeof_void_ptr == sizeof(btString)            );
CASSERT(sizeof_void_ptr == sizeof(btStringArray)       );
CASSERT(sizeof_void_ptr == sizeof(btByteArray)         );
CASSERT(sizeof_void_ptr == sizeof(btObjectType)        );
CASSERT(sizeof_void_ptr == sizeof(btObjectArray)       );
CASSERT(sizeof_void_ptr == sizeof(btAny)               );
CASSERT(sizeof_void_ptr == sizeof(btcObjectType)       );
CASSERT(sizeof_void_ptr == sizeof(btcString)           );
CASSERT(sizeof_void_ptr == sizeof(btcStringArray)      );
CASSERT(sizeof_void_ptr == sizeof(btGenericInterface)  );
CASSERT(sizeof_void_ptr == sizeof(btApplicationContext));

#if   defined( __AAL_WINDOWS__ )

// processthreadsapi.h: WINBASEAPI DWORD WINAPI GetProcessId(_In_ HANDLE Process);
CASSERT(sizeof_void_ptr == sizeof(btPID));
// processthreadsapi.h: WINBASEAPI DWORD WINAPI GetCurrentThreadId(VOID);
CASSERT(sizeof_void_ptr == sizeof(btTID));

#elif defined( __AAL_LINUX__ )

// bits/typesizes.h:    #define __PID_T_TYPE    __S32_TYPE
// bits/types.h:        __STD_TYPE __PID_T_TYPE __pid_t; /* Type of process identifications.  */
// unistd.h:            typedef __pid_t pid_t;
CASSERT(sizeof(bt32bitInt) == sizeof(btPID));
// bits/pthreadtypes.h: typedef unsigned long int pthread_t;
CASSERT(sizeof(btUnsigned64bitInt) == sizeof(btTID));

#endif // OS


END_NAMESPACE(AAL)

#endif //__AALSDK__AALTYPES_H__

