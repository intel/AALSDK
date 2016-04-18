// Copyright(c) 2005-2016, Intel Corporation
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
/// @file CNamedValueSet.cpp
/// @brief Concrete implementations of the Named Value Set classes.
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
/// 12/15/2005     JG       Initial version started
/// 04/24/2006     JG       Added bool, byte and NamedValues
/// 03/06/2007     JG       Added MessageHeader Type
/// 03/21/2007     JG       Ported to Linux
/// 05/29/2007     JG       Copied file from CNamedValues
///                         Modified to handle multiple key types
/// 07/13/2007     JG       Fixed a bug in assigment that would
///                         cause incorrect results (could be bigger).
///                JG       Fixed also could leak.
///                JG       Added Empty() method
/// 07/23/2007     JG       Fixed bug in some array functions
///                         where Num Elements wasn't passed
/// 07/23/2007     JG       Fixed bug in Dup where type wasn't
///                         copied correctly for int named values
/// 08/01/2007     HM       Added NVSRead/WriteNVS and helpers
/// 08/08/2007     HM       Added subset() and operator== to NVS
/// 08/11/2007     HM       #pragma'd out the fscanf warnings
///                            Using fscanf is inimical to linux
///                            and for all of these cases there is
///                            no difference
/// 08/17/2007     HM       Modified operator== to call subset() with
///                            fEqual flag TRUE, and subset to
///                            take the fEqual parameter for embedded
///                            NVS testing of subset or operator==
/// 09/14/2007     HM       Cleaning up code
/// 10/04/2007     JG       Minor interface change to Has()
/// 10/31/2007     HM       #ifndef __GNUC__ away various #pragmas
/// 11/21/2007     HM       Change signature of NVS to btcString
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 02/25/2008     HM       Noted that this file contains the ICC
///                            #ifdef __ICC master list of pragma warnings
/// 02/26/2008     HM       Fixed TNamedValueSet operator=, it was busted, never
///                            been tested, exercised
///                         Switched _DupNamedValueSet to use the new fixed
///                            TNamedValueSet operator=
/// 03/15/2008     HM       Templatized subset further with template functions
/// 03/16/2008     HM       Added streamio to FILE i/o for NVS's
/// 03/21/2008     HM       Split stream io into stream i and stream o
/// 03/22/2008     HM       Moved implementation of NVS operator << >> here
/// 05/01/2008     JG       Added byByteArray
/// 05/07/2008     HM       Tweaks to btByteArray, including I/O
///                            Fixed btByte I/O
/// 05/08/2008     HM       Comments & License
/// 05/26/2008     HM       Modified subsetIfArrayValuesNotEqual() to handle
///                            array subset as substr, instead of exactly
///                            equal, which is what it was (erroneously)
/// 05/27/2008     HM       Same modification for string arrays
/// 05/28/2008     HM       Begin removing NVS File IO
/// 05/29/2008     HM       Convert btcString and btUnsignedInt key types into
///                            specific typedefs: btStringKey and btNumberKey
/// 11/16/2008     HM       New utilities, for converting between strings and
///                            and NamedValueSets, and merging NVS's
/// 11/20/2008     HM       FormattedNVS output provides tabbing and hook point
///                            for human readable AAL_ID->string conversion
/// 01/04/2009     HM       Updated Copyright
/// 04/25/2012     HM       Disabled hopefully irrelevant warning about export
///                            of template for _WIN32, plus some cleanup.
///                         Warning disabled for template<> GetNumNames()@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#define NVSFileIO          /* for the time being, leave it in */
#include "aalsdk/INamedValueSet.h"


#define MAX_VALID_NVS_ARRAY_ENTRIES (1024 * 1024)

#if defined( __AAL_WINDOWS__ )
# define  strdup _strdup
#endif

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks (MASTER)*/
//   #pragma warning( push)
//   #pragma warning( pop)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
//   #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
//   #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning( pop)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
     #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
//   #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
     #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif


BEGIN_NAMESPACE(AAL)


CValue::CValue() :
   m_Size(0),
   m_Type(btUnknownType_t)
{
   m_Val.Obj = NULL;
}

CValue::CValue(const CValue &rOther) :
   m_Size(0),
   m_Type(btUnknownType_t)
{
   m_Val.Obj = NULL;

   //Assignment operator does the real work
   *this = rOther;
}

CValue::~CValue()
{
   // =======================================
   // If the value is not a normal scaler
   // delete any allocated arrays and objects
   //========================================
   switch ( m_Type ) {
      case btByteArray_t :
         delete [] m_Val._8bA;
      break;
      case bt32bitIntArray_t :
         delete [] m_Val._32bA;
      break;
      case btUnsigned32bitIntArray_t :
         delete [] m_Val._U32bA;
      break;
      case bt64bitIntArray_t :
         delete [] m_Val._64bA;
      break;
      case btUnsigned64bitIntArray_t :
         delete [] m_Val._U64bA;
      break;
      case btFloatArray_t :
         delete [] m_Val.fltA;
      break;
      case btString_t :
         free(m_Val.str);
      break;
      case btStringArray_t : {
         unsigned i;
         for ( i = 0 ; i < m_Size; ++i ) {
            free(m_Val.strA[i]);
         }
         delete [] m_Val.strA;
      } break;
      case btObjectArray_t:
         delete [] m_Val.ObjA;
      break;

      case btNamedValueSet_t:
         AAL::DeleteNVS(m_Val.pNVS);
      break;
      default : break;
   }//End case
}

CValue & CValue::operator = (const CValue &rOther)
{
   if ( &rOther == this ) {
      return *this; // don't duplicate self
   }

   // Put() for complex types allocates memory. We need to examine the type here, freeing
   // memory as required.

   switch ( m_Type ) {
      case btByteArray_t : {
         if ( NULL != m_Val._8bA ) {
            delete[] m_Val._8bA;
         }
      } break;
      case bt32bitIntArray_t : {
         if ( NULL != m_Val._32bA ) {
            delete[] m_Val._32bA;
         }
      } break;
      case btUnsigned32bitIntArray_t : {
         if ( NULL != m_Val._U32bA ) {
            delete[] m_Val._U32bA;
         }
      } break;
      case bt64bitIntArray_t : {
         if ( NULL != m_Val._64bA ) {
            delete[] m_Val._64bA;
         }
      } break;
      case btUnsigned64bitIntArray_t : {
         if ( NULL != m_Val._U64bA ) {
            delete[] m_Val._U64bA;
         }
      } break;
      case btFloatArray_t : {
         if ( NULL != m_Val.fltA ) {
            delete[] m_Val.fltA;
         }
      } break;
      case btStringArray_t : {
         if ( NULL != m_Val.strA ) {
            btUnsigned32bitInt i;
            for ( i = 0 ; i < m_Size ; ++i ) {
               free(m_Val.strA[i]);
            }
            delete[] m_Val.strA;
         }
      } break;
      case btObjectArray_t : {
         if ( NULL != m_Val.ObjA ) {
            delete[] m_Val.ObjA;
         }
      } break;
      case btString_t : {
         if ( NULL != m_Val.str ) {
            free(m_Val.str);
         }
      } break;
      case btNamedValueSet_t : {
         if ( NULL != m_Val.pNVS ) {
            AAL::DeleteNVS(m_Val.pNVS);
         }
      } break;
   }

   m_Type = rOther.m_Type;
   m_Size = rOther.m_Size;

   // Copy array, string, and NVS types using mutator.
   switch ( m_Type ) {
      case btByteArray_t :
         Put(rOther.m_Val._8bA, m_Size);
      break;
      case bt32bitIntArray_t :
         Put(rOther.m_Val._32bA, m_Size);
      break;
      case btUnsigned32bitIntArray_t :
         Put(rOther.m_Val._U32bA, m_Size);
      break;
      case bt64bitIntArray_t :
         Put(rOther.m_Val._64bA, m_Size);
      break;
      case btUnsigned64bitIntArray_t :
         Put(rOther.m_Val._U64bA, m_Size);
      break;
      case btFloatArray_t :
         Put(rOther.m_Val.fltA, m_Size);
      break;
      case btStringArray_t :
         Put(rOther.m_Val.strA, m_Size);
      break;
      case btObjectArray_t :
         Put(rOther.m_Val.ObjA, m_Size);
      break;
      case btString_t :
         Put(rOther.m_Val.str);
      break;
      case btNamedValueSet_t :
         Put(rOther.m_Val.pNVS);
      break;

      default:
         // Otherwise simple value
         m_Val = rOther.m_Val;
      break;
   }

   return *this;
}

void CValue::Put(btBool val)             { m_Type = btBool_t;             m_Val._1b   = val; m_Size = 1; }
void CValue::Put(btByte val)             { m_Type = btByte_t;             m_Val._8b   = val; m_Size = 1; }
void CValue::Put(bt32bitInt val)         { m_Type = bt32bitInt_t;         m_Val._32b  = val; m_Size = 1; }
void CValue::Put(btUnsigned32bitInt val) { m_Type = btUnsigned32bitInt_t; m_Val._U32b = val; m_Size = 1; }
void CValue::Put(bt64bitInt val)         { m_Type = bt64bitInt_t;         m_Val._64b  = val; m_Size = 1; }
void CValue::Put(btUnsigned64bitInt val) { m_Type = btUnsigned64bitInt_t; m_Val._U64b = val; m_Size = 1; }
void CValue::Put(btFloat val)            { m_Type = btFloat_t;            m_Val.flt   = val; m_Size = 1; }
void CValue::Put(btObjectType val)       { m_Type = btObjectType_t;       m_Val.Obj   = val; m_Size = 1; }

void CValue::Put(const INamedValueSet *val)
{
   m_Type     = btNamedValueSet_t;
   m_Val.pNVS = val->Clone();
   m_Size     = 1;
}

void CValue::Put(btcString val)
{
   m_Type    = btString_t;
   m_Val.str = strdup(val);
   m_Size    = 1;
}

void CValue::Put(btByteArray val, btUnsigned32bitInt Num)
{
   m_Type = btByteArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val._8bA = new btByte[Num];
   memcpy(m_Val._8bA, val, (sizeof(btByte)*Num));
}

void CValue::Put(bt32bitIntArray val, btUnsigned32bitInt Num)
{
   m_Type = bt32bitIntArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val._32bA = new bt32bitInt[Num];
   memcpy(m_Val._32bA, val, (sizeof(bt32bitInt)*Num));
}

void CValue::Put(btUnsigned32bitIntArray val, btUnsigned32bitInt Num)
{
   m_Type = btUnsigned32bitIntArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val._U32bA = new btUnsigned32bitInt[Num];
   memcpy(m_Val._U32bA, val, (sizeof(btUnsigned32bitInt)*Num));
}

void CValue::Put(bt64bitIntArray val,btUnsigned32bitInt Num)
{
   m_Type = bt64bitIntArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val._64bA = new bt64bitInt[Num];
   memcpy(m_Val._64bA, val, (sizeof(bt64bitInt)*Num));
}

void CValue::Put(btUnsigned64bitIntArray val,btUnsigned32bitInt Num)
{
   m_Type = btUnsigned64bitIntArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val._64bA = new bt64bitInt[Num];
   memcpy(m_Val._U64bA, val, (sizeof(bt64bitInt)*Num));
}

void CValue::Put(btFloatArray val,btUnsigned32bitInt Num)
{
   m_Type = btFloatArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val.fltA = new btFloat[Num];
   memcpy(m_Val.fltA, val, (sizeof(btFloat)*Num));
}

void CValue::Put(btStringArray val, btUnsigned32bitInt NumElements)
{
   m_Type = btStringArray_t;
   m_Size = NumElements;
   // Copy the array of btStrings and create btString array
   m_Val.strA = new btString[NumElements];
   btUnsigned32bitInt x;
   for ( x = 0 ; x < NumElements ; ++x ) {
      //Copy string
      m_Val.strA[x] = strdup(val[x]);
   }
}

void CValue::Put(btObjectArray val,btUnsigned32bitInt Num)
{
   m_Type = btObjectArray_t;
   m_Size = Num;
   //Allocate space for local array copy
   m_Val.ObjA = new btObjectType[Num];
   memcpy(m_Val.ObjA , val, (sizeof(btObjectType)*Num));
}

ENamedValues CValue::Get(bt32bitInt *pval) const
{
   if ( m_Type != bt32bitInt_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val._32b;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btBool *pval) const
{
   if ( m_Type != btBool_t ) {
      *pval = false;
      return ENamedValuesBadType;
   }
   *pval = m_Val._1b;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btByte *pval) const
{
   if ( m_Type != btByte_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val._8b;
   return ENamedValuesOK;
}


ENamedValues CValue::Get(btUnsigned32bitInt *pval) const
{
   if ( m_Type != btUnsigned32bitInt_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val._U32b;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(bt64bitInt *pval) const
{
   if ( m_Type != bt64bitInt_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val._64b;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btUnsigned64bitInt *pval) const
{
   if ( m_Type != btUnsigned64bitInt_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val._U64b;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btFloat *pval) const
{
   if ( m_Type != btFloat_t ) {
      *pval = 0;
      return ENamedValuesBadType;
   }
   *pval = m_Val.flt;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btcString *pval) const
{
   if ( m_Type != btString_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.str;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(INamedValueSet const **pval) const
{
   if ( m_Type != btNamedValueSet_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.pNVS;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btObjectType *pval) const
{
   if ( m_Type != btObjectType_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.Obj;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btByteArray *pval) const
{
   if ( m_Type != btByteArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val._8bA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(bt32bitIntArray *pval) const
{
   if ( m_Type != bt32bitIntArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val._32bA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btUnsigned32bitIntArray *pval) const
{
   if ( m_Type != btUnsigned32bitIntArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val._U32bA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(bt64bitIntArray *pval) const
{
   if ( m_Type != bt64bitIntArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val._64bA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btUnsigned64bitIntArray *pval) const
{
   if ( m_Type != btUnsigned64bitIntArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val._U64bA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btFloatArray *pval) const
{
   if ( m_Type != btFloatArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.fltA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btStringArray *pval) const
{
   if ( m_Type != btStringArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.strA;
   return ENamedValuesOK;
}

ENamedValues CValue::Get(btObjectArray *pval) const
{
   if ( m_Type != btObjectArray_t ) {
      *pval = NULL;
      return ENamedValuesBadType;
   }
   *pval = m_Val.ObjA;
   return ENamedValuesOK;
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@                    T E M P L A T E                         @@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

//=============================================================================
//=============================================================================
//   This template is used to construct a NVS class specific to a particular
//   key data type. It is used in the CNamevValueSet  container class to hold
//   an NVS instance that is specific to a particular key type.
//   Namely std::string and and btUnsignedInt.  We could have defined a class
//   for each type but this allows us to easily create new NVS for any key
//   type.
//=============================================================================
//=============================================================================

//=============================================================================
// Name: TNamedValueSet
// Description: Template class definition of for NamedValueSets
//=============================================================================
template<typename Kt>
class TNamedValueSet
{
private:
   typedef typename std::map<Kt, AAL::CValue> map_type;
   typedef typename map_type::const_iterator  const_iterator;

   map_type m_NVSet;

public:
   //=============================================================================
   // Name: TNamedValuesSet
   // Description: Constructor
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments:
   //=============================================================================
   TNamedValueSet() {}

   //=============================================================================
   // Name: TNamedValues
   // Description: Assignment
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments:
   //=============================================================================
   TNamedValueSet & operator = (const TNamedValueSet &rOther);

   btBool Subset(const TNamedValueSet &rOther, btBool fEqual=false) const;

   //=============================================================================
   // Name: TNamedValues
   // Description: operator ==
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments: Algorithm is to check if the number of elements is equal, and if
   //           so, then do a subset. If the subset is true and the number of
   //           elements are equal, the NVS's must be equal.
   //=============================================================================
   btBool operator == (const TNamedValueSet &rOther) const
   {
      btUnsignedInt NumNamesThis;
      btUnsignedInt NumNamesOther;

      GetNumNames(&NumNamesThis);
      rOther.GetNumNames(&NumNamesOther);

      if ( NumNamesThis == NumNamesOther ) {
         return Subset(rOther, true); // Is this a subset of rOther?, and
                                      //   are embedded NVS's also ==
      }

      return false;
   }

   //=============================================================================
   // Name: ~TNamedValues
   // Description: Destructor
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments: Iterate through list and free all named value sets.
   //=============================================================================
   virtual ~TNamedValueSet()
   {
      Empty();
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btBool value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btBool Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btByte value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btByte Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a bt32bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, bt32bitInt Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btUnsigned32bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btUnsigned32bitInt Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a bt64bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, bt64bitInt Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btUnsigned64bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btUnsigned64bitInt Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btFloat value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btFloat Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a string value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btcString Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add an INamedValueSet value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, const INamedValueSet *Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }


   //=============================================================================
   // Name: Add
   // Description: Add a ByteArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btByteArray        Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a 32bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, bt32bitIntArray    Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a btUnsigned32bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btUnsigned32bitIntArray Value,
                             btUnsigned32bitInt      NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a 64bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, bt64bitIntArray    Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a Unsigned64bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btUnsigned64bitIntArray Value,
                             btUnsigned32bitInt      NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a ObjectType value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btObjectType Value)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a floatArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btFloatArray       Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a stringArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btStringArray      Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Add
   // Description: Add a ObjectArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   //         Value - Value
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Add(Kt Name, btObjectArray      Value,
                             btUnsigned32bitInt NumElements)
   {
      const_iterator itr = m_NVSet.end();

      //Check for exclusivity
      if ( m_NVSet.find(Name) != itr ) {
         return ENamedValuesDuplicateName;
      }

      //Store the value
      m_NVSet[Name].Put(Value, NumElements);
      return ENamedValuesOK;
   }


   //=============================================================================
   // Name: Get
   // Description: Get a btBool value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btBool *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a btByte value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btByte *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a 32bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, bt32bitInt *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a Unsigned32bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btUnsigned32bitInt *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a 64bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, bt64bitInt *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a Unsigned64bitInt value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btUnsigned64bitInt *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a float value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btFloat *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a string value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btcString *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get an INamedValueSet value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, INamedValueSet const **pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a ByteArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btByteArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a 32bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, bt32bitIntArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a Unsigned32bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btUnsigned32bitIntArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a 64bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, bt64bitIntArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a Unsigned64bitIntArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btUnsigned64bitIntArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a ObjectType value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btObjectType *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a floatArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btFloatArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a stringArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btStringArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Get
   // Description: Get a ObjectArray value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pvalue - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues Get(Kt Name, btObjectArray *pValue) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      return (*itr).second.Get(pValue);
   }

   //=============================================================================
   // Name: Delete
   // Description: Delete a named value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: none.
   // Comments:
   //=============================================================================
   ENamedValues Delete(Kt Name)
   {
      //Find the named value pair
      if ( m_NVSet.end() == m_NVSet.find(Name) ) {
         return ENamedValuesNameNotFound;
      }

      //Remove the entry from the set
      m_NVSet.erase(Name);
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Empty
   // Description: Empties the NVS
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments: Iterate through list and free all named value sets.
   //=============================================================================
   ENamedValues Empty()
   {
      btUnsignedInt NumNames;
      Kt            CurrName;

      GetNumNames(&NumNames);

      while ( NumNames-- ) {
         GetName(NumNames, &CurrName);
         //Remove it
         ENamedValues result;
         result = Delete(CurrName);
         if ( result != ENamedValuesOK ) {
            return result;
         }
      }

      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: GetSize
   // Description: Get the size in objects of a named value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pSize - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues     GetSize(Kt Name, btWSSize    *pSize) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      *pSize = (*itr).second.Size();

      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Type
   // Description: Get the Type of a named value
   // Interface: public
   // Inputs: Name - Parameter name.
   // Outputs: pType - Place to return value.
   // Comments:
   //=============================================================================
   ENamedValues        Type(Kt Name, eBasicTypes *pType) const
   {
      const_iterator itr = m_NVSet.find(Name);

      //Find the named value pair
      if ( m_NVSet.end() == itr ) {
         return ENamedValuesNameNotFound;
      }

      //Return the Type
      *pType = (*itr).second.Type();
      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: GetNumNames
   // Description: Get the number of named value pairs in this object
   // Interface: public
   // Inputs: none.
   // Outputs: pNum - Place to return number of names.
   // Comments:
   //=============================================================================
#if defined( _MSC_VER )
   #pragma warning( push )
   #pragma warning( disable:4251 )  // Cannot export template definitions
#endif // _MSC_VER
   ENamedValues GetNumNames(btUnsignedInt *pNum) const
   {
      *pNum = static_cast<btUnsignedInt>(m_NVSet.size());  // size_t truncation to int possible
      return ENamedValuesOK;
   }
#if defined( _MSC_VER )
   #pragma warning( pop )
#endif // _MSC_VER

   //=============================================================================
   // Name: GetName
   // Description: Gets the name of a named value pair at index number.
   // Interface: public
   // Inputs: index - Zero based index into list of named/value pairs.
   // Outputs: pName - Place to return name.
   // Comments:
   //=============================================================================
   ENamedValues     GetName(btUnsignedInt index, Kt *pName) const
   {
      const_iterator itr;

      //Find the named value pair
      if ( m_NVSet.size() < (typename map_type::size_type) index ) {
         return ENamedValuesNameNotFound;
      }

      //Get the right entry
      for ( itr = m_NVSet.begin() ; index != 0 ; index--, itr++ ) {/*empty*/ ; }

      //Return the name
      *pName = (Kt)(*itr).first;

      return ENamedValuesOK;
   }

   //=============================================================
   // Specialized template function for std:string keys
   // The default GetName implementation will not work for
   // btcString keys. btcString keys are stored as std:strings in
   // the map structure. So the normal return won't work.
   // This specialized template function handles this one
   // exception case.
   //=============================================================
   ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const
   {
      std::map<std::string, CValue>::const_iterator itr = m_NVSet.end();

      // Find the named value pair
      if ( m_NVSet.size() < (std::map<std::string, CValue>::size_type) index ) {
         return ENamedValuesNameNotFound;
      }

      // Get the right entry
      for ( itr = m_NVSet.begin() ; index != 0 ; index--, itr++ ) {/*empty*/ ; }

      // Return the name
      *pName = const_cast<btStringKey>((*itr).first.c_str());

      return ENamedValuesOK;
   }

   //=============================================================================
   // Name: Has
   // Description: Return whether a named value pair exits
   // Interface: public
   // Inputs: Name - Name to check.
   // Outputs: none.
   // Comments: Returns ENamedValuesOK if Name exists
   //=============================================================================
   btBool Has(Kt Name) const
   {
      //Find the named value pair
      return m_NVSet.find(Name) != m_NVSet.end();
   }

}; // End of template<class Kt>  class TNamedValueSet : public CriticalSection

//=============================================================================
//=============================================================================
//   This template is used to construct a NVS class specific to a particular
//   key data type. It is used in the CNamevValueSet  container class to hold
//   an NVS instance that is specific to a particular key type.
//   Namely std::string and and btUnsignedInt.  We could have defined a class
//   for each type but this allows us to easily create new NVS for any key
//   type.
//=============================================================================
//=============================================================================

template<typename Kt>
TNamedValueSet<Kt> & TNamedValueSet<Kt>::operator = (const TNamedValueSet<Kt> &rOther)
{
   btUnsignedInt NumNames;
   Kt CurrName;
   eBasicTypes Type;

   //Ignore assigning self to self
   if ( &rOther == this ) {
      return *this;
   }

   //Make sure the target is empty
   Empty();

   //Get the number of entries in source
   rOther.GetNumNames(&NumNames);

   //Copy one at a time
   for(btUnsignedInt count=0; count < NumNames; count++) {
      rOther.GetName(count,&CurrName);
      rOther.Type(CurrName,&Type);

      switch(Type) {
         case btBool_t: {
            btBool val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btByte_t: {
            btByte val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case bt32bitInt_t: {
            bt32bitInt val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btUnsigned32bitInt_t: {
            btUnsigned32bitInt val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case bt64bitInt_t: {
            bt64bitInt val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btUnsigned64bitInt_t: {
            btUnsigned64bitInt val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btFloat_t: {
            btFloat val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btString_t: {
            btcString val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btNamedValueSet_t: {
            INamedValueSet const *pval = NULL;
            rOther.Get(CurrName, &pval);
            Add(CurrName, pval);
         }
         break;
         case btByteArray_t: {
            btByte  *val;
            btWSSize Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case bt32bitIntArray_t: {
            bt32bitInt *val;
            btWSSize    Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case btUnsigned32bitIntArray_t: {
            btUnsigned32bitInt *val;
            btWSSize            Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case bt64bitIntArray_t: {
            bt64bitInt *val;
            btWSSize    Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case btUnsigned64bitIntArray_t: {
            btUnsigned64bitInt *val;
            btWSSize            Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case btObjectType_t: {
            btObjectType val;
            rOther.Get(CurrName,&val);
            Add(CurrName,val);
         }
         break;
         case btFloatArray_t: {
            btFloat *val;
            btWSSize Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case btStringArray_t: {
            btString *val;
            btWSSize  Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         case btObjectArray_t: {
            btObjectType *val;
            btWSSize      Num = 0;
            rOther.GetSize(CurrName,&Num);
            rOther.Get(CurrName,&val);
            Add(CurrName,val,Num);
         }
         break;
         default: {
            break;
         }

      };
   }
   return( *this );
}  // end of operator = (assignment)

//=============================================================================
// Name: TNamedValues::subsetIfSingleValuesNotEqual
// Description: subset worker function
// Interface: not part of the template
// Inputs: this and rOther NVS's, name of the current NVP
// Outputs: returns false if the values are equal
// Comments: A copy of this is instantiated for each name and data type in an
//    NVS that are single valued
//=============================================================================
template<typename Kt, typename dataT>
btBool subsetIfSingleValuesNotEqual(const TNamedValueSet<Kt> &rThis,
                                    const TNamedValueSet<Kt> &rOther,
                                    const Kt CurrName)
{
   dataT valThis, valOther;
   rThis.Get(CurrName,&valThis);
   rOther.Get(CurrName,&valOther);
   return valThis != valOther;
}

//=============================================================================
// Name: TNamedValues::subsetIfArrayValuesNotEqual
// Description: subset worker function
// Interface: not part of the template
// Inputs: this and rOther NVS's, name of the current NVP
// Outputs: returns false if ALL of the values are equal
// Comments: A copy of this is instantiated for each name and data type in an
//    NVS that are arrays
// Comments: Keep this in sync with the string function as well,
//           subsetIfArrayValuesNotEqualbtStringArray()
// TODO: Try to find a way to do this that is lighter-weight, perhaps passing
//    in an equality function and making it more generic. E.g., two functions,
//    one per keytype but not parameterized on dataType, but instead use a
//    dataType-specific equality functor, or something like that.
//=============================================================================
template<typename Kt, typename dataT>
btBool subsetIfArrayValuesNotEqual(const TNamedValueSet<Kt> &rThis,
                                   const TNamedValueSet<Kt> &rOther,
                                   const Kt CurrName,
                                   btBool fEqual)
{
   btWSSize uThis  = 0;                   // For GetSize call
   btWSSize uOther = 0;                   // "
   int      numThis;                      // number of elements in each array
   int      numOther;                     // "

   rThis.GetSize(CurrName,&uThis);
   numThis = static_cast<int>(uThis);
   rOther.GetSize(CurrName,&uOther);
   numOther = static_cast<int>(uOther);

   // Initial checks
   if (fEqual) {                          // For equality, number of elements must be equal
      if (numThis != numOther) {
         return true;                     // Not a match
      }
   }
   else {
      if (numThis == 0) return true;      // no match, as no test pattern
   }

   dataT valThis, valOther;               // get arrays, always pointers to something
   rThis.Get(CurrName,&valThis);
   rOther.Get(CurrName,&valOther);

   if (fEqual) {
      while (numThis--) {                 // iterate, testinq for equality
         if (*valThis++ != *valOther++) {
            return true;                  // Not a match
         }
      }
      return false;                       // items MATCH
   }
   else {                                 // subset match using substr() semantics
      enum { Initial, Search};
      int State = Initial;
      int OtherIndex, SearchOtherIndex, ThisIndex;

      // Loop through Other, testing the first element of This against each element of Other.
      // If a match is found, see if the rest of This matches.
      // Example This is ABC, Other is AABC, match is on second A in Other.
      // numOther-numThis is the maximum position in Other that one even needs to try.
      //    E.g., in above case, 4-3 = 1, so one only needs to test against Other positions
      //    0 and 1. If it doesn't match there, it never will, as the This pattern is too long.
      for (OtherIndex=0; OtherIndex<=(numOther-numThis); ++OtherIndex) {

         if (State == Initial) {          // scan in this state until hit first match
            if (valThis[0] == valOther[OtherIndex]) {
               State = Search;            // hit first match, outer for loop remembers current
            }                             //    position. Continue testing below.
         }                                // If no match, just continue outer for scan

         // Can hit both above and below clauses in same loop
         // - that is on purpose and is typical of a first match
         if (State == Search) {
            for (ThisIndex=1, SearchOtherIndex=OtherIndex+1;
                 ThisIndex<numThis && SearchOtherIndex<numOther;  // test ThisIndex first as it might kick out without every executin this loop,but final test must match
                 ++ThisIndex, ++SearchOtherIndex) {
               if (valThis[ThisIndex] != valOther[SearchOtherIndex]) {
                  State = Initial;        // found non-match, abort, increment, and try again
                  break;
               }
            }
            // Reached end of search, if matched entire This, then found it; otherwise start over
            if (ThisIndex >= numThis) {   // 'greater-than' should never happen, just defensive programming
               return false;              // items MATCH - at OtherIndex, if anyone should care
            }
         };
      };
      return true;                        // no match found
   }
}  // end of subsetIfArrayValuesNotEqual

//=============================================================================
// Name: TNamedValues::subsetIfArrayValuesNotEqualbtStringArray
// Description: subset worker function
// Interface: not part of the template
// Inputs: this and rOther NVS's, name of the current NVP
// Outputs: returns false if ALL of the values are equal
// Comments: Keep this in sync with the main templatized function,
//           subsetIfArrayValuesNotEqual()
//=============================================================================
template <typename Kt>
btBool subsetIfArrayValuesNotEqualbtStringArray( const TNamedValueSet<Kt> &rThis,
                                                 const TNamedValueSet<Kt> &rOther,
                                                 const Kt CurrName,
                                                 btBool fEqual)
{
   btWSSize uThis  = 0;                   // For GetSize call
   btWSSize uOther = 0;                   // "
   int      numThis;                      // number of elements in each array
   int      numOther;                     // "

   rThis.GetSize( CurrName, &uThis );
   numThis = static_cast<int>( uThis );
   rOther.GetSize( CurrName, &uOther );
   numOther = static_cast<int>( uOther );

            // Initial checks
   if ( fEqual ) {                        // For equality, number of elements must be equal
      if ( numThis != numOther ) {
         return true;                    // Not a match
      }
   } else {
      if ( numThis == 0 )
         return true;                    // no match, as no test pattern
   }

   btStringArray valThis, valOther;       // get arrays, always pointers to something
   rThis.Get( CurrName, &valThis );
   rOther.Get( CurrName, &valOther );

   if ( fEqual ) {
      while ( numThis-- ) {               // iterate, testinq for equality
         if ( strcmp(*valThis++, *valOther++ )) {
            return true;                  // Not a match
         }
      }
      return false;                       // items MATCH
   } else {                               // subset match using substr() semantics
      enum { Initial, Search};
      int State = Initial;
      int OtherIndex, SearchOtherIndex, ThisIndex;

      // Loop through Other, testing the first element of This against each element of Other.
      // If a match is found, see if the rest of This matches.
      // Example This is ABC, Other is AABC, match is on second A in Other.
      // numOther-numThis is the maximum position in Other that one even needs to try.
      //    E.g., in above case, 4-3 = 1, so one only needs to test against Other positions
      //    0 and 1. If it doesn't match there, it never will, as the This pattern is too long.
      for ( OtherIndex = 0; OtherIndex <= ( numOther - numThis ); ++OtherIndex ) {

         if ( State == Initial ) {          // scan in this state until hit first match
            if ( !strcmp(valThis[ 0 ], valOther[ OtherIndex ] )) {
               State = Search;            // hit first match, outer for loop remembers current
            }                             //    position. Continue testing below.
         }                                // If no match, just continue outer for scan

         // Can hit both above and below clauses in same loop
         // - that is on purpose and is typical of a first match
         if ( State == Search ) {
            for ( ThisIndex = 1, SearchOtherIndex = OtherIndex + 1;
                  ThisIndex < numThis && SearchOtherIndex < numOther;  // test ThisIndex first as it might kick out without every executin this loop,but final test must match
                  ++ThisIndex, ++SearchOtherIndex ) {
               if ( strcmp(valThis[ ThisIndex ], valOther[ SearchOtherIndex ] )) {
                  State = Initial;        // found non-match, abort, increment, and try again
                  break;                  // exit this loop, but stay in outer one to try again
               }
            }
            // Reached end of search, if matched entire This, then found it; otherwise start over
            if ( ThisIndex >= numThis ) { // 'greater-than' should never happen, just defensive programming
               return false;              // items MATCH - at OtherIndex, if anyone should care
            }
         };
      };
      return true;                        // no match found
   }
} // end of subsetIfArrayValuesNotEqualbtStringArray


//=============================================================================
// Name: TNamedValues
// Description: subset
// Interface: public
// Inputs:  "other" NVS
//          btBool fEqual true if an equality match (used by ==), false if a
//             subset match.
// Outputs: none.
// Comments: "this" is the pattern, to be tested to see it is a subset of rOther
//          The algorithm is that for each NameValue in 'this', check to see if
//          there is an exact match in rOther.
//
//          An exact match is defined as the same name, same datatype, same
//          datavalue. The type of the name must match, as well, but because
//          the template is parameterized on that, the name types will automatically
//          match.
//=============================================================================
template<typename Kt>
btBool TNamedValueSet<Kt>::Subset(const TNamedValueSet<Kt> &rOther,
                                  btBool fEqual) const
{
   btUnsignedInt NumNames;          // for iterating through 'this'

   //Get the number of entries
   GetNumNames(&NumNames);

   //Check one at a time
   for(btUnsignedInt count=0; count < NumNames; count++) {
      Kt CurrName;
      eBasicTypes typeDataThis, typeDataOther;

      GetName(count,&CurrName);     // get the name at the current index of 'this', which is the pattern
      if (!rOther.Has(CurrName)) {  // If rOther doesn't have the element, 'this' is not a subset
         return false;
      }

      Type(CurrName,&typeDataThis); // get the data type of the pattern and the set
      rOther.Type(CurrName,&typeDataOther);
      if (typeDataThis != typeDataOther) {
         return false;              // data types must match
      }

      switch(typeDataThis)          // data values must match, as well
      {
         case btBool_t:
            if (subsetIfSingleValuesNotEqual<Kt,btBool>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btByte_t:
            if (subsetIfSingleValuesNotEqual<Kt,btByte>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case bt32bitInt_t:
            if (subsetIfSingleValuesNotEqual<Kt,bt32bitInt>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btUnsigned32bitInt_t:
            if (subsetIfSingleValuesNotEqual<Kt,btUnsigned32bitInt>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case bt64bitInt_t:
            if (subsetIfSingleValuesNotEqual<Kt,bt64bitInt>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btUnsigned64bitInt_t:
            if (subsetIfSingleValuesNotEqual<Kt,btUnsigned64bitInt>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btFloat_t:
            if (subsetIfSingleValuesNotEqual<Kt,btFloat>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btString_t: {
            btcString valThis, valOther;
            Get(CurrName,&valThis);
            rOther.Get(CurrName,&valOther);
            if ( (NULL==valThis)&&(NULL==valOther) ){ // if both null, okay
               return true;
            }
            if ( (NULL==valThis)||(NULL==valOther) ){ // if either null, bad
               return false;                          // need to protect strcmp, or it throws exceptions
            }
            if (strcmp(valThis,valOther)){            // if not equal string
               return false;
            }
         }
         break;
         case btNamedValueSet_t: {
            INamedValueSet const *valThis  = NULL;
            INamedValueSet const *valOther = NULL;
            Get(CurrName, &valThis);
            rOther.Get(CurrName, &valOther);
            if (fEqual) {                             // Truly want equality
               if (!(*valThis == *valOther)) {        // Handles equality testing in NVS
                  return false;
               }
            } else {
               if (!valThis->Subset(*valOther)) {     // Handles equality testing in NVS
                  return false;
               }
            }
         }
         break;
         case btByteArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,btByteArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case bt32bitIntArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,bt32bitIntArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case btUnsigned32bitIntArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,btUnsigned32bitIntArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case bt64bitIntArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,bt64bitIntArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case btUnsigned64bitIntArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,btUnsigned64bitIntArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case btObjectType_t:
            if (subsetIfSingleValuesNotEqual<Kt,btObjectType>(*this, rOther, CurrName)) {
               return false;
            }
         break;
         case btFloatArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,btFloatArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         case btStringArray_t:
            if (subsetIfArrayValuesNotEqualbtStringArray<Kt>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
            break;
         case btObjectArray_t:
            if (subsetIfArrayValuesNotEqual<Kt,btObjectArray>(*this, rOther, CurrName, fEqual)) {
               return false;
            }
         break;
         default: {
            break;
         }
      }; // switch (Type)
   }  // for(btUnsignedInt count=0; count < NumNames; count++)

   return true;   // Nothing did not match, so it all matched
}  // end of Subset


//=============================================================================
//=============================================================================
//   The CNamedValuesSet class is a container class for the specific NVS
//   instances.  It holds member for each NVS type it supports. Currently
//   std:string keys and btUnsignedInt keys.  This object is the actual storage
//   for the NVS used by the application. The application only "sees" the proxy
//   object called NamedValueSet which simply calls through to this one.
//=============================================================================
//=============================================================================


/// Concrete implementation of the INamedValueSet interface.
/// @ingroup BasicTypes
class CNamedValueSet : public INamedValueSet,
                       public CriticalSection
{
   // For the purposes of lock safety, we follow the order of..
   //   AutoLock(this);
   //   AutoLock(&Other);
   // throughout CNamedValueSet.

private:
   TNamedValueSet<btNumberKey> m_iNVS;
   TNamedValueSet<std::string> m_sNVS;  // Note that the interface is btStringKey, which is const char*

public:
   /// CNamedValueSet Default Constructor.
   CNamedValueSet() {}
   /// CNamedValueSet Destructor.
   virtual ~CNamedValueSet() {}

   /// Assign Named Value Set to another.
   CNamedValueSet & operator = (const CNamedValueSet &rOther)
   {
      if ( &rOther != this ) {   //Don't duplicate yourself
         AutoLock(this);

         // Make sure this NVS is empty
         m_iNVS.Empty();
         m_sNVS.Empty();

         {
            AutoLock(&rOther);
            m_iNVS = rOther.m_iNVS;
            m_sNVS = rOther.m_sNVS;
         }
      }
      return *this;
   }

   /// CNamedValueSet Copy Constructor.
   CNamedValueSet(const CNamedValueSet &rOther)
   {
      AutoLock(this);
      {
         AutoLock(&rOther);
         // Do the copy
         m_iNVS = rOther.m_iNVS;
         m_sNVS = rOther.m_sNVS;
      }
   }

   ENamedValues Add(btNumberKey Name, btBool Value)                { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btByte Value)                { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, bt32bitInt Value)            { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btUnsigned32bitInt Value)    { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, bt64bitInt Value)            { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btUnsigned64bitInt Value)    { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btFloat Value)               { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btcString Value)             { ASSERT(NULL != Value); AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btObjectType Value)          { AutoLock(this); return m_iNVS.Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, const INamedValueSet *Value)
   {
      ASSERT(NULL != Value);
      AutoLock(this);
      if ( this == Value->Concrete() ) {
         return ENamedValuesRecursiveAdd;
      }
      return m_iNVS.Add(Name, Value->Concrete());
   }

   ENamedValues Add(btNumberKey        Name,
                    btByteArray        Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey        Name,
                    bt32bitIntArray    Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned32bitIntArray Value,
                    btUnsigned32bitInt      NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey        Name,
                    bt64bitIntArray    Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned64bitIntArray Value,
                    btUnsigned32bitInt      NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }

   ENamedValues Add(btNumberKey        Name,
                    btFloatArray       Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey        Name,
                    btStringArray      Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_iNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btNumberKey        Name,
                    btObjectArray      Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this); return m_iNVS.Add(Name, Value, NumElements);
   }

   ENamedValues Get(btNumberKey Name, btBool *pValue) const                  { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByte *pValue) const                  { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitInt *pValue) const              { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitInt  *pValue) const     { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitInt *pValue) const              { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitInt *pValue) const      { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloat *pValue) const                 { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btcString *pValue) const               { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, INamedValueSet const **pValue) const   { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByteArray *pValue) const             { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitIntArray *pValue) const         { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitIntArray *pValue) const         { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectType *pValue) const            { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloatArray *pValue) const            { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btStringArray *pValue) const           { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectArray *pValue) const           { ASSERT(NULL != pValue); AutoLock(this); return m_iNVS.Get(Name, pValue); }

   ENamedValues  Delete(btNumberKey Name)                                    { AutoLock(this); return m_iNVS.Delete(Name);        }
   ENamedValues GetSize(btNumberKey Name, btWSSize *pSize)       const       { ASSERT(NULL != pSize); AutoLock(this); return m_iNVS.GetSize(Name,pSize); }
   ENamedValues    Type(btNumberKey Name, eBasicTypes *pType)    const       { ASSERT(NULL != pType); AutoLock(this); return m_iNVS.Type(Name,pType);    }
   btBool           Has(btNumberKey Name)                        const       { AutoLock(this); return m_iNVS.Has(Name);           }
   ENamedValues GetName(btUnsignedInt index, btNumberKey *pName) const
   {
      eNameTypes   Type = btNumberKey_t;
      ENamedValues res;

      ASSERT(NULL != pName);

      AutoLock(this);

      res = GetNameType(index, &Type);
      if ( ENamedValuesOK != res ) {
         return res;
      }

      if ( Type != btNumberKey_t ) {
         return ENamedValuesBadType;
      }

      return m_iNVS.GetName(index, pName);
   }

   ENamedValues Add(btStringKey Name, btBool Value)                { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btByte Value)                { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, bt32bitInt Value)            { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btUnsigned32bitInt Value)    { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, bt64bitInt Value)            { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btUnsigned64bitInt Value)    { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btFloat Value)               { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btcString Value)             { ASSERT(NULL != Value); AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btObjectType Value)          { AutoLock(this); return m_sNVS.Add(Name, Value); }
   ENamedValues Add(btStringKey Name, const INamedValueSet *Value)
   {
      ASSERT(NULL != Value);
      AutoLock(this);
      if ( this == Value->Concrete() ) {
         return ENamedValuesRecursiveAdd;
      }
      return m_sNVS.Add(Name, Value->Concrete());
   }

   ENamedValues Add(btStringKey        Name,
                    btByteArray        Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey        Name,
                    bt32bitIntArray    Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned32bitIntArray Value,
                    btUnsigned32bitInt      NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey        Name,
                    bt64bitIntArray    Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned64bitIntArray Value,
                    btUnsigned32bitInt      NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey        Name,
                    btFloatArray       Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey        Name,
                    btStringArray      Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }
   ENamedValues Add(btStringKey        Name,
                    btObjectArray      Value,
                    btUnsigned32bitInt NumElements)
   {
      ASSERT(NULL != Value);
      if ( 0 == NumElements ) {
         return ENamedValuesZeroSizedArray;
      }
      AutoLock(this);
      return m_sNVS.Add(Name, Value, NumElements);
   }

   ENamedValues Get(btStringKey Name, btBool *pValue) const                  { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByte *pValue) const                  { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitInt *pValue) const              { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitInt *pValue) const      { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitInt *pValue) const              { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitInt *pValue) const      { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloat *pValue) const                 { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btcString *pValue) const               { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, INamedValueSet const **pValue) const   { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByteArray *pValue) const             { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitIntArray *pValue) const         { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitIntArray *pValue) const         { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectType *pValue) const            { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloatArray *pValue) const            { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btStringArray *pValue) const           { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectArray *pValue) const           { ASSERT(NULL != pValue); AutoLock(this); return m_sNVS.Get(Name, pValue); }

   ENamedValues  Delete(btStringKey Name)                                    { AutoLock(this); return m_sNVS.Delete(Name);         }
   ENamedValues GetSize(btStringKey Name, btWSSize *pSize)    const          { ASSERT(NULL != pSize); AutoLock(this); return m_sNVS.GetSize(Name, pSize); }
   ENamedValues    Type(btStringKey Name, eBasicTypes *pType) const          { ASSERT(NULL != pType); AutoLock(this); return m_sNVS.Type(Name, pType);    }
   btBool           Has(btStringKey Name)                     const          { AutoLock(this); return m_sNVS.Has(Name);            }

   ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const
   {
      eNameTypes    Type = btStringKey_t;
      btUnsignedInt iNum = 0;
      ENamedValues  res;

      ASSERT(NULL != pName);

      AutoLock(this);

      res = GetNameType(index, &Type);
      if ( ENamedValuesOK != res ) {
         return res;
      }

      if ( Type != btStringKey_t ) {
         return ENamedValuesBadType;
      }

      // Adjust the index for non-string keys
      m_iNVS.GetNumNames(&iNum);
      index -= iNum;
      return m_sNVS.GetName(index, pName);
   }


   //--------------------------------------------------------------------------
   // Regular functions, not distinguished by key type
   //--------------------------------------------------------------------------

   ENamedValues Empty()          // Force the NVS to delete all its members
   {
      AutoLock(this);
      ENamedValues res[2] = { ENamedValuesOK, ENamedValuesOK };
      res[0] = m_iNVS.Empty();
      res[1] = m_sNVS.Empty();
      return ( ENamedValuesOK != res[0] ? res[0] : res[1] );
   }

   btBool Subset(const INamedValueSet &rOther) const
   {
      CNamedValueSet const *pCNamedValueSet = dynamic_cast<CNamedValueSet const *>(rOther.Concrete());

      ASSERT(NULL != pCNamedValueSet);
      if ( NULL == pCNamedValueSet ) {
         return false;
      }

      AutoLock(this);
      {
         AutoLock(pCNamedValueSet);
         btBool iRet = m_iNVS.Subset(pCNamedValueSet->m_iNVS);
         btBool sRet = m_sNVS.Subset(pCNamedValueSet->m_sNVS);
         return (iRet && sRet);
      }
   }

   btBool operator == (const INamedValueSet &rOther) const
   {
      CNamedValueSet const *pCNamedValueSet = dynamic_cast<CNamedValueSet const *>(rOther.Concrete());

      ASSERT(NULL != pCNamedValueSet);
      if ( NULL == pCNamedValueSet ) {
         return false;
      }

      AutoLock(this);
      {
         AutoLock(pCNamedValueSet);
         btBool iRet = (m_iNVS == pCNamedValueSet->m_iNVS);
         btBool sRet = (m_sNVS == pCNamedValueSet->m_sNVS);
         return (iRet && sRet);
      }
   }

   ENamedValues GetNumNames(btUnsignedInt *pNum) const
   {
      btUnsignedInt piNum = 0;
      btUnsignedInt psNum = 0;
      ENamedValues  result;

      ASSERT(NULL != pNum);

      AutoLock(this);

      result = m_iNVS.GetNumNames(&piNum);
      if ( result != ENamedValuesOK ) {
         return result;
      }

      result = m_sNVS.GetNumNames(&psNum);
      if ( result != ENamedValuesOK ) {
         return result;
      }

      // Return the sum of both NVS
      *pNum = piNum + psNum;
      return result;
   }

   ENamedValues GetNameType(btUnsignedInt index, eNameTypes *pType) const
   {
      btUnsignedInt piNum    = 0;
      btUnsignedInt NumNames = 0;
      ENamedValues  result;

      ASSERT(NULL != pType);

      AutoLock(this);

      // Get the total number of names
      result = GetNumNames(&NumNames);
      if ( ENamedValuesOK != result ) {
         return result;
      }

      if ( index >= NumNames ) {
         return ENamedValuesIndexOutOfRange;
      }

      // Get the total names that are indexed by integers
      result = m_iNVS.GetNumNames(&piNum);

      // Integers keys start at index zero
      *pType = (index >= piNum ? btStringKey_t : btNumberKey_t);

      return result;
   }

   virtual ENamedValues     Read(std::istream & );
   virtual ENamedValues    Write(std::ostream &os) const { return Write(os, 0); }
   virtual ENamedValues    Write(std::ostream & , unsigned ) const;

   // was ENamedValues NVSWriteOneNVSToFile(std::ostream        &os, NamedValueSet const &nvsToWrite,   unsigned             level);
   // Serialize NVS with ending demarcation
   virtual ENamedValues WriteOne(std::ostream & , unsigned ) const;

#ifdef NVSFileIO

   // Reads an open (binary) file and creates an NVS from it.
   // was NVSReadNVS(FILE * , NamedValueSet * );
   virtual ENamedValues     Read(FILE * );

   // was NVSWriteNVS(FILE * , NamedValueSet * , unsigned );
   virtual ENamedValues    Write(FILE *f) const { return Write(f, 0); }
   virtual ENamedValues    Write(FILE * , unsigned ) const;

   // Serialize NVS with ending demarcation
   // was NVSWriteOneNVSToFile(FILE * , NamedValueSet const & , unsigned );
   virtual ENamedValues WriteOne(FILE * , unsigned ) const;

#endif // NVSFileIO

   virtual ENamedValues   Merge(const INamedValueSet & );

   virtual ENamedValues FromStr(const std::string & );
   virtual ENamedValues FromStr(void * , btWSSize );
   virtual std::string    ToStr() const;

protected:

   // Make this equal to the given INamedValueSet.
   virtual ENamedValues Copy(const INamedValueSet &Other)
   {
      CNamedValueSet const *pCNamedValueSet = dynamic_cast<CNamedValueSet const *>(Other.Concrete());

      ASSERT(NULL != pCNamedValueSet);
      if ( NULL == pCNamedValueSet ) {
         return ENamedValuesBadType;
      }

      AutoLock(this);
      {
         AutoLock(pCNamedValueSet);
         m_iNVS = pCNamedValueSet->m_iNVS;
         m_sNVS = pCNamedValueSet->m_sNVS;
      }

      return ENamedValuesOK;
   }

   // Create a new identical copy of this object.
   virtual INamedValueSet * Clone() const
   {
      INamedValueSet *p = AAL::NewNVS();

      ASSERT(NULL != p);
      if ( NULL == p ) {
         return NULL;
      }

      AutoLock(this);
      p->Copy(*this);

      return p;
   }

   virtual INamedValueSet const * Concrete() const { return this; }

private:

   // was void NVSWriteEndOfNVS(std::ostream &os, unsigned level);
   static void WriteEndOfNVS(std::ostream & , unsigned );

   // was void NVSWriteName(std::ostream &os, eNameTypes typeName, btStringKey pszName, unsigned level);
   static void WriteName(std::ostream & , eNameTypes , btStringKey , unsigned );

   // was void NVSWriteName(std::ostream &os, eNameTypes typeName, btNumberKey uName, unsigned level);
   static void WriteName(std::ostream & , eNameTypes , btNumberKey , unsigned );

   // was NVSWriteLevel(std::ostream &os, unsigned level);
   static void WriteLevel(std::ostream & , unsigned );

   // was void NVSWriteUnsigned(std::ostream &os, unsigned u, unsigned level);
   static void WriteUnsigned(std::ostream & , btUnsignedInt , unsigned );

   // was void NVSWriteString(std::ostream &os, btcString sz, unsigned level);
   static void WriteString(std::ostream & , btcString , unsigned );

   static void WritebtFloat(std::ostream & , btFloat );

   // was btBool  NVSReadUnsigned(std::istream &is, btUnsignedInt *pu);
   static btBool ReadUnsigned(std::istream & , btUnsignedInt * );

   // was char *    NVSReadString(std::istream &is);
   static char * ReadString(std::istream & );

   // was btBool NVSReadNumberKey(std::istream &is, btNumberKey *pu);
   static btBool ReadNumberKey(std::istream & , btNumberKey * );

   // was ENamedValues NVSReadNVSError(btmStringKey sName, ENamedValues error);
   static ENamedValues ReadNVSError(btmStringKey , ENamedValues );

   static ENamedValues ReadbtFloat(std::istream & , btFloat * );

#ifdef NVSFileIO

   // was NVSWriteEndOfNVS(FILE * , unsigned );
   static void WriteEndOfNVS(FILE * , unsigned );

   // was NVSWriteName(FILE *file, eNameTypes typeName, btStringKey pszName, unsigned level);
   static void WriteName(FILE * , eNameTypes , btStringKey , unsigned );

   // was void NVSWriteName(FILE *file, eNameTypes typeName, btNumberKey uName, unsigned level);
   static void WriteName(FILE * , eNameTypes , btNumberKey , unsigned );

   // was NVSWriteLevel(FILE *file, unsigned level);
   static void WriteLevel(FILE * , unsigned );

   // was NVSWriteUnsigned(FILE *file, unsigned u, unsigned level);
   static void WriteUnsigned(FILE * , btUnsignedInt , unsigned );

   // was void NVSWriteString(FILE *file, btcString sz, unsigned level);
   static void WriteString(FILE * , btcString , unsigned );

   static void WritebtFloat(FILE * , btFloat );

   // was int NVSReadUnsigned(FILE *file, btUnsignedInt *pu);
   static int ReadUnsigned(FILE * , btUnsignedInt * );

   // was AASLIB_API btString NVSReadString(FILE *file);
   static btString ReadString(FILE * );

   // was int NVSReadNumberKey(FILE *file, btNumberKey *pu);
   static int ReadNumberKey(FILE * , btNumberKey * );

   static ENamedValues ReadbtFloat(FILE * , btFloat * );

#endif // NVSFileIO

}; // End of class CNamedValueSet

/// NamedValueSet factory.
/// @ingroup BasicTypes
///
/// @return Pointer to newly-allocated INamedValuesSet interface.
INamedValueSet * NewNVS()
{
   return new(std::nothrow) CNamedValueSet();
}

/// Deletes an INamedValueSet object.
/// @ingroup BasicTypes
void DeleteNVS(INamedValueSet *p)
{
   if ( NULL != p ) {
      delete p;
   }
}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@                    G E N E R I C S                         @@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

//=============================================================================
// Name:        NamedValueSetFromStdString
// Description: Convert a std::string length into an NVS
// Interface:   public
// Inputs:      std::string containing the serialized representation of the
//              NamedValueSet.
// Outputs:     nvs is a non-const reference to the returned NamedValueSet
//=============================================================================
ENamedValues CNamedValueSet::FromStr(const std::string &s)
{
   AutoLock(this);
   std::istringstream iss(s); // put the string inside an istringstream
   return Read(iss);          // use Read() to get it out
}

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
ENamedValues CNamedValueSet::FromStr(void *pv, btWSSize len)
{
   ASSERT(NULL != pv);
   std::string s(static_cast<char *>(pv), (size_t)len);   // initializing this way allows embedded nulls
   return FromStr(s);
}

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
std::string CNamedValueSet::ToStr() const
{
   AutoLock(this);
   std::ostringstream oss;
   oss << *this << '\0';  // add a final, ensuring, terminating null
   return oss.str();
}

//=============================================================================
// Name:          NVSMerge
// Description:   Merge one NamedValueSet into another
// Interface:     public
// Inputs:        nvsInput and nvsOutput will be merged together
// Outputs:       nvsOutput will contain the merged results
// Returns:       ENamedValuesOK for success, appropriate value otherwise
// Comments:      If there are duplicate names, the original value in the
//                   nvsOutput takes precedence and there is no error
//=============================================================================
ENamedValues CNamedValueSet::Merge(const INamedValueSet &nvsInput)
{
   AutoLock(this);
   // Write nvsInput to a stringstream
   std::stringstream ss;
   ss << nvsInput;
   ss >> *this;
   return ENamedValuesOK;
}  // NVSMerge



//=============================================================================
// Name:        operator << on NamedValueSet
// Description: serializes a NamedValueSet to a stream
// Interface:   public
// Inputs:      stream, and NamedValueSet
// Outputs:     serialized NamedValueSet
// Comments:    works for writing to any of cout, ostream, ofstream,
//                 ostringstream, fstream, stringstream
//=============================================================================
std::ostream & operator << (std::ostream &s, const INamedValueSet &nvs)
{
   nvs.Write(s);
   return s;
}

//=============================================================================
// Name: operator >> on NamedValueSet
// Description: reads a serialized NamedValueSet from a file
// Interface: public
// Inputs: stream, and NamedValueSet
// Outputs: none
// Comments: works for reading from of cin, istream, ifstream,
//           iostringstream, fstream, stringstream
// Comments: The passed in NamedValueSet is not emptied first, so if it contains
//           information already, that will simply be added to.
//=============================================================================
std::istream & operator >> (std::istream &s, INamedValueSet &rnvs)
{
   ENamedValues eRet = rnvs.Read(s);
   if ( ENamedValuesOK != eRet ) {
      s.setstate(std::ios_base::failbit);
   }
   return s;
}

//=============================================================================
// Name:        BufFromString
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
AASLIB_API void BufFromString(void *pBuf, std::string const &s)
{
   size_t len = s.length();

   #if defined( _MSC_VER )
      #pragma warning( push)
      #pragma warning( disable : 4996) // destination of copy is unsafe
   #endif // _MSC_VER

   s.copy(static_cast<char*>(pBuf), len);

   #if defined( _MSC_VER )
      #pragma warning( pop)      // fscanf
   #endif // _MSC_VER
}

#ifdef NVSFileIO
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@                      F I L E   I / O                       @@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

//=============================================================================
// Name: NVSWriteLevel - write tabs so that entries will be nicely indented
//=============================================================================
void CNamedValueSet::WriteLevel(FILE *file, unsigned level)
{
   while ( level-- ) {
      fputc('\t', file);
   }
}

//=============================================================================
// Name: NVSWriteUnsigned - write a single unsigned, at the level passed in, followed by a space
//=============================================================================
void CNamedValueSet::WriteUnsigned(FILE *file, btUnsignedInt u, unsigned level)
{
   CNamedValueSet::WriteLevel(file, level);
   fprintf(file, "%u ", u);
}

//=============================================================================
// Name: NVSReadUnsigned - read a single unsigned integer, ignoring preceeding whitespace
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
#ifdef _MSC_VER
   #pragma warning( push)                // fscanf
   #pragma warning( disable : 4996)      // fscanf
#endif // _MSC_VER
int CNamedValueSet::ReadUnsigned(FILE *file, btUnsignedInt *pu)
{
   return fscanf(file, "%u", pu);   // eats preceeding whitespace
}

//=============================================================================
//=============================================================================
int CNamedValueSet::ReadNumberKey(FILE *file, btNumberKey *pu)
{
   return fscanf( file, "%llu", pu);   // choose one or the other
// return fscanf( file, "%u", pu);     // depends on type of btNumberKey
}
#ifdef _MSC_VER
   #pragma warning( pop)      // fscanf
#endif // _MSC_VER

//=============================================================================
// Name: WriteString to an open file
//       Format is LEN STR\n, e.g. 10 HelloWorld\n
//       not expecting errors here
//       level preceeds string with an appropriate number of tabs
//=============================================================================
void CNamedValueSet::WriteString(FILE *file, btcString sz, unsigned level)
{
   CNamedValueSet::WriteLevel(file, level);
   fprintf(file, "%u %s\n", (unsigned)strlen(sz), sz);
}

//=============================================================================
// Name: NVSReadString from an open file - must free the result if not NULL
//       Format is LEN STRING, e.g. 5 hello\n
//          optionally preceeded by whitespace
//       Input is open file
//       Returns pointer to malloc'd buffer if successful, NULL if not
//=============================================================================
btString CNamedValueSet::ReadString(FILE *file)
{
   btUnsignedInt u = 0;
   btWSSize      szlen;
   btWSSize      Total;
   btWSSize      ThisTime;

   if ( EOF == CNamedValueSet::ReadUnsigned(file, &u) ) {
      return NULL;                  // Get length of string
   }

   szlen = u;

   ASSERT(szlen <= 256);
   if ( szlen > 256 ) {
      std::cerr << "WARNING: NVSReadString: input length "
                << szlen
                << " greater than 256" << std::endl;
   }

   if ( szlen > MAX_VALID_NVS_ARRAY_ENTRIES ) {
      std::cerr << "ERROR: NVSReadString: input length "
                << szlen
                << " greater than " << MAX_VALID_NVS_ARRAY_ENTRIES << std::endl;
      return NULL;
   }

   // get an input buffer
   btString psz = new(std::nothrow) btByte[szlen + 1];

   if ( NULL == psz ) {
      return NULL;
   }
                                 // malloc worked, read the string
   fgetc(file);                  // eat the preceeding space

   Total = 0;
   do
   {
      ThisTime = fread(psz + (Total * sizeof(btByte)),
                       sizeof(btByte),
                       (szlen+1) - Total, // Be sure to consume the terminating \n from the stream.
                       file);

      Total += ThisTime;

      if ( ferror(file) ) {
         delete[] psz;
         return NULL;
      }

   }while( Total < (szlen+1) );

   psz[szlen] = 0;               // overwrite terminating \n

   return psz;
}


//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void CNamedValueSet::WriteName(FILE *file, eNameTypes typeName, btStringKey pszName, unsigned level)
{
   CNamedValueSet::WriteLevel(file, level);                      // tab in
   fprintf(file, "%u ", static_cast<unsigned>(typeName));
   CNamedValueSet::WriteString(file, pszName, 0);                // write the name
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void CNamedValueSet::WriteName(FILE *file, eNameTypes typeName, btNumberKey uName, unsigned level)
{
   CNamedValueSet::WriteLevel(file, level);            // tab in
   fprintf(file, "%u ", static_cast<unsigned>(typeName));
   fprintf(file, "%llu\n", uName);                     // Depends on underlying type of btNumberKey
//   fprintf (file, "%u\n", uName);                    // No way to get around it in here
}

//=============================================================================
// Name: NVSWriteEndOfNVS
//       Write an EndOfNVS marker to an open file
//       Name is hardcoded and fake - will never actually be read or used, can
//          something already in the NVS without problem
//=============================================================================
void CNamedValueSet::WriteEndOfNVS(FILE *file, unsigned level)
{
   CNamedValueSet::WriteName(file, btStringKey_t, "---- End of NVS ----", level);
   CNamedValueSet::WriteUnsigned(file, btEndOfNVS_t, level+1); // This is the real end marker
   fprintf(file, "\n");
}  // End of NVSWriteEndOfNVS

void CNamedValueSet::WritebtFloat(FILE *file, btFloat f)
{
   unsigned char       *p    = reinterpret_cast<unsigned char *>(&f);
   unsigned char const *pEnd = p + sizeof(btFloat);

   fprintf(file, "{ ");

   while ( p < pEnd ) {
      unsigned u = (unsigned)*p;
      fprintf(file, "%02x ", u);
      ++p;
   }

   fprintf(file, "}");
}

ENamedValues CNamedValueSet::ReadbtFloat(FILE *file, btFloat *pflt)
{
   unsigned char       *p    = reinterpret_cast<unsigned char *>(pflt);
   unsigned char const *pEnd = p + sizeof(btFloat);

   fgetc(file); // discard {
   fgetc(file); // discard space

   if ( feof(file) ) {
      return ENamedValuesInternalError_UnexpectedEndOfFile;
   }

   while ( p < pEnd ) {
      unsigned u = 0;
      if ( EOF == fscanf(file, "%x", &u) ) {
         return ENamedValuesInternalError_UnexpectedEndOfFile;
      }
      fgetc(file); // discard space

      if ( feof(file) ) {
         return ENamedValuesInternalError_UnexpectedEndOfFile;
      }

      *p = (unsigned char)u;
      ++p;
   }

   fgetc(file); // discard }

   if ( feof(file) ) {
      return ENamedValuesInternalError_UnexpectedEndOfFile;
   }

   return ENamedValuesOK;
}

//=============================================================================
// Name: NVSWriteNVS to an open file, opened in binary mode
//       file has to be open, with EOL translation OFF (e.g. binary mode, b), file handle is not checked
//       nvsToWrite is input NVS to write to the file
//       Level is the indentation level at which the NVS is to be written, initially 0, it can be increased
//          for recursion or other special purposes. Due to the formatting conventions of always starting a line
//          with a number, indentations are stripped away as whitespace and do not cause problems
//
//       Format is:
//       NameType Name: e.g. "5 100\n" means name type of unsigned int, name is 100
//                or    e.g. "9 7 foo bar\n" means type of string, name is foo bar, of length 7
//       ValueType Value: e.g. "\t0 1\n" means:
//                   ValueType 0 is btBool
//                   Value 1 is TRUE
//                   the preceeding \t is indentation to make the file more readable
//    or ValueType NumValues Value[s]: e.g. "\t0 3 1 0 1\n" for an array means:
//                   ValueType 0 is btBool
//                   NumValues 3 is one value
//                   Value 1 0 1 is TRUE, FALSE, TRUE
//                or "\t11 3 1 2 3\n" means bt32bitIntArray_t length 3 with values 1, 2, and 3
//                or "\t19 2\n\t7 foo bar\n\t 9 foo2 bar2\n" is
//                   special because it is an array of strings, which are separated by newlines instead of spaces
//=============================================================================
ENamedValues CNamedValueSet::Write(FILE *file, unsigned level) const
{
   AutoLock(this);

   btUnsignedInt uElements;                  // number of elements in the NVS
   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName;                      // to hold an integer name
   btStringKey   sName = NULL;               // to hold a string name
   eBasicTypes   typeData = btUnknownType_t; // type of data


   GetNumNames(&uElements);     // initialize the loop to the number of elements in the NVS

   //Write one at a time
   for ( irg = 0 ; irg < uElements ; irg++ ) {        // walk forward through NVS
      GetNameType(irg, &typeName);// Get name type
      switch ( typeName ) {
         case btStringKey_t:
            GetName(irg,&sName);     // Get the name
            CNamedValueSet::WriteName(file, typeName, sName, level); // Write the name
            Type(sName,&typeData);   // Get the data type
         break;
         case btNumberKey_t:                 // same here, for UINT names
            GetName(irg,&iName);
            CNamedValueSet::WriteName(file, typeName, iName, level);
            Type(iName,&typeData);
         break;
         default:
            ASSERT(false);
            //fprintf(stderr, "ERROR: WriteNVS: name type %u unsupported.\n", static_cast<unsigned>(typeName));
            return ENamedValuesInternalError_InvalidNameFormat;
      };

      // write data
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // In this section, there is a matrix of typeName vs. typeData, with a different function
      //    having to be called for each. Clearly this should be templatized, or something, someday.

      switch ( typeData ) {
         case btBool_t : {
            btBool val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf(file, "%u\n", static_cast<unsigned>(val));
         } break;
         case btByte_t : {
            btByte val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
//            fprintf( file, "%c\n", val);
            fwrite (&val, sizeof(btByte), 1, file);
            fprintf(file, "\n");
         } break;
         case bt32bitInt_t : {
            bt32bitInt val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf(file, "%d\n", val);
         } break;
         case btUnsigned32bitInt_t: {
            btUnsigned32bitInt val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf( file, "%u\n", val);
         }
         break;
         case bt64bitInt_t: {
            bt64bitInt val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf( file, "%lld\n", val);
         }
         break;
         case btUnsigned64bitInt_t: {
            btUnsigned64bitInt val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf(file, "%llu\n", val);
         }
         break;
         case btFloat_t: {
            btFloat val = 0.0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WritebtFloat(file, val);
            fprintf(file, "\n");
         }
         break;
         case btString_t: {
            btcString val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteString(file, val, 0);
         }
         break;
         case btNamedValueSet_t: {
            INamedValueSet const *pval = NULL;
            if ( btStringKey_t == typeName ) {
               Get(sName, &pval);
            } else {
               Get(iName, &pval);
            }
            // NVS Name already written, write the NVS DataType
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fputc('\n',file);
            pval->Write(file, level+2);  // Write the NVS itself
            // Write the trailer - String Name, and DataType 'End of NVS'
            CNamedValueSet::WriteName(file, btStringKey_t, "---- End of embedded NVS ----", level);
            CNamedValueSet::WriteUnsigned(file, btEndOfNVS_t, level+1); // This is the real end marker
            fprintf( file, "\n");
         }
         break;
         case btByteArray_t: {
            btByte  *val;
            btWSSize Num = 0;
            btWSSize NumWritten = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while (Num) {
               NumWritten = fwrite(val, sizeof(btByte), Num, file);
               Num -= NumWritten;
            }
            fprintf(file, "\n");
         }
         break;
         case bt32bitIntArray_t: {
            bt32bitInt *val;
            btWSSize    Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               fprintf(file, "%d ", *val++);
            }
            fprintf(file, "\n");
         }
         break;
         case btUnsigned32bitIntArray_t: {
            btUnsigned32bitInt *val;
            btWSSize            Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               fprintf(file, "%u ", *val++);
            }
            fprintf(file, "\n");
         }
         break;
         case bt64bitIntArray_t: {
            bt64bitInt *val;
            btWSSize    Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               fprintf(file, "%lld ", *val++);
            }
            fprintf(file, "\n");
         }
         break;
         case btUnsigned64bitIntArray_t: {
            btUnsigned64bitInt *val;
            btWSSize            Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               fprintf(file, "%llu ", *val++);
            }
            fprintf(file, "\n");
         }
         break;
         case btObjectType_t:      // Pointer, could be 32 or 64 bit
         {
            btObjectType val;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(val);
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            fprintf(file, "%llu\n", u64temp);
         }
         break;
         case btFloatArray_t: {
            btFloat *val;
            btWSSize Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               CNamedValueSet::WritebtFloat(file, *val++);
            }
            fprintf(file, "\n");
         }
         break;
         case btStringArray_t: {
            btString *val;
            btWSSize  Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            fputs("\n", file);
            while ( Num-- ) {
               CNamedValueSet::WriteString(file, *val++, level+2);
            }
         }
         break;
         case btObjectArray_t: {
            btObjectType *val;     // pointer to array of pointers
            btWSSize      Num = 0; // number of pointers
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(file, typeData, level+1);
            CNamedValueSet::WriteUnsigned(file, Num, 0);
            while ( Num-- ) {
               btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(*val++);
               fprintf(file, "%llu ", u64temp);
            }
            fprintf(file, "\n");
         }
         break;

         default: {
            break;
         }

      }
      ; // switch typeData
   }  // for loop
   return ENamedValuesOK;
}  // NVSWriteNVS

//=============================================================================
// Name: NVSWriteOneNVSToFile
//       Write an NVS + an EndofNVS marker to an open file
//=============================================================================
ENamedValues CNamedValueSet::WriteOne(FILE    *file,
                                      unsigned level) const
{
   ENamedValues retval = Write(file, level);
   CNamedValueSet::WriteEndOfNVS(file, level);
   return retval;
}  // End of NVSWriteOneNVSToFile

//=============================================================================
// Name: NVSReadNVS from an open file, opened in binary mode
//       file has to be open, in binary mode (e.g. b option), file handle is not checked
//       nvsToRead is a passed in already created NVS to which all data will be appended
//          it is not checked for prior emptiness
//
//       for Format see NVSWriteNVS
//
//       Returns on:
//          Error, e.g. unknown values
//          EOF, implying finished with an NVS
//          reading btUnknownType_t, implying finished reading an EMBEDDED NVS
//
//=============================================================================
#if defined( _MSC_VER )
# pragma warning( push)           // fscanf
# pragma warning( disable : 4996) // fscanf
#endif // _MSC_VER
ENamedValues CNamedValueSet::Read(FILE *file)
{
   AutoLock(this);

   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName;                      // to hold an integer name
   btmStringKey  sName    = NULL;            // to hold a string name, returned from NVSReadString
   eBasicTypes   typeData = btUnknownType_t; // type of data

   //Read one at a time, counting as we go
   for ( irg = 0 ; ; irg++ ) {        // walk forward through NVS
      // Read NVS Name
      //       Format is TYPE NAME
      //       Where TYPE must be either btString_t or btUnsignedInt_t
      //       Where NAME is either an INTEGER or a LEN STRING, depending upon the TYPE

      // Get type of the NAME
      btUnsignedInt tempType;

      tempType = 0;
      if ( EOF == CNamedValueSet::ReadUnsigned(file, &tempType) ) {
         return ENamedValuesEndOfFile;  // EOF means end of file, done with this NVS
      }
      if (tempType   != btNumberKey_t &&  // range check
            tempType != btStringKey_t) {
         return ENamedValuesBadType;    //    error return
      }
      typeName = static_cast<eNameTypes>(tempType); // safe, because of range check

      iName = 0;

      switch (typeName)                 // if read of typeName successful, use it
      {
         case btStringKey_t : {         // string name, FREE IT ONCE DONE WITH IT

            sName = CNamedValueSet::ReadString(file);
            if ( NULL == sName ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }

         } break;

         case btNumberKey_t : {         // integer name

            if ( EOF == CNamedValueSet::ReadNumberKey(file, &iName) ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }

         } break;

         default : return ENamedValuesInternalError_InvalidNameFormat;
      }
      // At this point, know that that typeName is either btString_t or btUnsignedInt_t
      //    and corresponding name is in either sName or iName

      // Get type of the DATA - expect an unsigned here
      tempType = 0;
      if (EOF == CNamedValueSet::ReadUnsigned(file, &tempType)) {
         return CNamedValueSet::ReadNVSError( sName, ENamedValuesInternalError_UnexpectedEndOfFile);
      }
      if (tempType  > btUnknownType_t &&   // range check
          tempType != btEndOfNVS_t) {
         return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);
      }
      typeData = static_cast<eBasicTypes>(tempType); // safe, because of range check

      // READ DATA
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // In this section, there is a matrix of typeName vs. typeData, with a different function
      //    having to be called for each. Clearly this should be templatized, or something, someday.

      // If typeName is btString_t, sName must be freed. This is done at the end of the switch.
      //    Error returns or other aborts from the switch need to free sName.

#define NVS_ADD(__x)                  \
do                                    \
{                                     \
   if ( btStringKey_t == typeName ) { \
      Add(sName, __x);                \
   } else {                           \
      Add(iName, __x);                \
   }                                  \
}while(0)

#define NVSREADNVS_FSCANF(__type, __initializer, __fmt)                                          \
case __type##_t : {                                                                              \
   __type val = __initializer;                                                                   \
   if ( EOF == fscanf(file, __fmt, &val) ) { /* what happens if number is too big to fit? */     \
      return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile); \
   }                                                                                             \
   NVS_ADD(val);                                                                                 \
} break

#define NVS_ADD_ARRAY(__A, __len)     \
do                                    \
{                                     \
   if ( btStringKey_t == typeName ) { \
      Add(sName, __A, __len);         \
   } else {                           \
      Add(iName, __A, __len);         \
   }                                  \
}while(0)

#define NVSREADNVS_FSCANF_ARRAY(__type, __fmt)                                                      \
case __type##Array_t : {                                                                            \
   __type##Array      val, p;                                                                       \
   btUnsigned32bitInt i,   Num = 0;                                                                 \
   btUnsigned32bitInt ThisTime;                                                                     \
   if ( EOF == CNamedValueSet::ReadUnsigned(file, &Num) ) { /* read number of elements */           \
      return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);    \
   }                                                                                                \
   if ( ( 0 == Num ) || ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) ) {                                   \
      break; /* end now on bad item count */                                                        \
   }                                                                                                \
   if ( NULL == (val = new __type[Num]) ) {                                                         \
      return CNamedValueSet::ReadNVSError(sName, ENamedValuesOutOfMemory);                          \
   }                                                                                                \
   i = Num;                                                                                         \
   p = val;                                                                                         \
   while ( i-- ) {                             /* read the array */                                 \
      ThisTime = fscanf(file, __fmt, p++);                                                          \
      if ( ( 1 != ThisTime ) || ferror(file) ) {                                                    \
         delete[] val;                                                                              \
         return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile); \
      }                                                                                             \
   }                                                                                                \
   NVS_ADD_ARRAY(val, Num);                                                                         \
   delete[] val;                                                                                    \
} break

      switch ( typeData ) {
         case btBool_t : {                // Read Data
            btBool val   = false;
            int    uTemp = 0;

            if (EOF == fscanf(file, "%d", &uTemp)) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ((uTemp < 0) || (uTemp > 1)) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);
            }

            #if defined( _MSC_VER )
               #pragma warning( push )
               #pragma warning( disable:4800 )  // int to bool performance warning
            #endif // _MSC_VER

            val = static_cast<btBool>(uTemp);         // C4800

            #if defined( _MSC_VER )
               #pragma warning( pop )
            #endif // _MSC_VER

            NVS_ADD(val);
         } break;

         case btByte_t : {                // Read Data
            btByte val = 0;
            if ( EOF == fgetc(file) ) {   // remove preceeding whitespace
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 1 != fread(&val, sizeof(btByte), 1, file) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         case bt32bitInt_t : {            // Read Data
            bt32bitInt val = 0;
            if (EOF == CNamedValueSet::ReadUnsigned(file, reinterpret_cast<btUnsigned32bitInt*>(&val))) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         case btUnsigned32bitInt_t : {    // Read Data
            btUnsigned32bitInt val = 0;
            if ( EOF == CNamedValueSet::ReadUnsigned(file, &val) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         NVSREADNVS_FSCANF(bt64bitInt,         0,   "%lld");
         NVSREADNVS_FSCANF(btUnsigned64bitInt, 0,   "%llu");

         case btFloat_t : {
            btFloat val = 0.0;
            ENamedValues res = CNamedValueSet::ReadbtFloat(file, &val);
            if ( ENamedValuesOK != res ) {
               return CNamedValueSet::ReadNVSError(sName, res);
            }
            NVS_ADD(val);
         } break;

         case btString_t : {              // Read Data
            btString val = CNamedValueSet::ReadString(file);
            if (NULL == val) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
            delete[] val;
         } break;

         case btNamedValueSet_t : {         // Read Data
            CNamedValueSet val;
            ENamedValues   ret;

            ret = val.Read(file);
            if ( ENamedValuesOK == ret ) {
               NVS_ADD(&val);
            } else {
               return CNamedValueSet::ReadNVSError(sName, ret);
            }
         } break;

         // Normal end of embedded NVS, not really an error, but need to free sName
         case btEndOfNVS_t  : {
            fgetc(file);
            fgetc(file);
            return CNamedValueSet::ReadNVSError(sName, ENamedValuesOK);
         }

         case btByteArray_t : {       // Read Data
            btByteArray        val;
            btUnsigned32bitInt Num = 0;
            btUnsigned32bitInt Total;
            btUnsigned32bitInt ThisTime;

            if ( EOF == CNamedValueSet::ReadUnsigned(file, &Num) ) {  // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                            // crazy value, but possible, end now
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesIndexOutOfRange);
            }

            if ( NULL == (val = new btByte[Num + 1]) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesOutOfMemory);
            }

            fgetc(file);                             // eat the preceeding space

            Total = 0;
            do
            {
               ThisTime = fread(val + (Total * sizeof(btByte)),
                                sizeof(btByte),
                                (Num+1) - Total, // Be sure to consume the terminating \n from the stream.
                                file);

               Total += ThisTime;

               if ( ferror(file) ) {
                  delete[] val;
                  return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
               }

            }while( Total < (Num+1) );

            val[Num] = 0;                            // overwrite terminating \n with \0

            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         NVSREADNVS_FSCANF_ARRAY(bt32bitInt,         "%d"  );
         NVSREADNVS_FSCANF_ARRAY(btUnsigned32bitInt, "%u"  );
         NVSREADNVS_FSCANF_ARRAY(bt64bitInt,         "%lld");
         NVSREADNVS_FSCANF_ARRAY(btUnsigned64bitInt, "%llu");

         case btFloatArray_t : {
            btFloatArray       val, p;
            btUnsigned32bitInt i,   Num = 0;
            ENamedValues       res;
            if ( EOF == CNamedValueSet::ReadUnsigned(file, &Num) ) { /* read number of elements */
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( ( 0 == Num ) || ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) ) {
               break; /* end now on bad item count */
            }
            if ( NULL == (val = new btFloat[Num]) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesOutOfMemory);
            }
            i = Num;
            p = val;
            while ( i-- ) { /* read the array */
               res = CNamedValueSet::ReadbtFloat(file, p++);
               if ( ( ENamedValuesOK != res ) || ferror(file) ) {
                  delete[] val;
                  return CNamedValueSet::ReadNVSError(sName, res);
               }
            }
            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         // Read Data
         case btObjectType_t : {             // Pointer, could be 32 or 64 bit
            btObjectType       val;
            btUnsigned64bitInt u64 = 0;

            if ( EOF == fscanf(file, "%llu", &u64) ) {   // what happens if number is too big to fit?
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            val = reinterpret_cast<btObjectType>(u64);

            NVS_ADD(val);
         } break;

         case btStringArray_t : {            // Read Data
            btStringArray      val;
            btUnsigned32bitInt i, Num = 0;

            if ( EOF == CNamedValueSet::ReadUnsigned(file, &Num) ) {  // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesIndexOutOfRange);
            }
            if ( NULL == (val = new btString[Num]) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesOutOfMemory);
            }

            for ( i = 0 ; i < Num ; ++i ) {                     // initialize, helps with final cleanup
               val[i] = NULL;
            }

            for ( i = 0 ; i < Num ; ++i ) {                     // read strings, break if run out of memory
               if ( NULL == (val[i] = CNamedValueSet::ReadString(file)) ) {
                  break;
               }
            }

            if ( i == Num ) {                            // if the loop ended properly, then got all
               NVS_ADD_ARRAY(val, Num);
            }

            for ( i = 0 ; i < Num ; ++i ) {
               if ( NULL == val[i] ) {
                  break;
               }
               delete[] val[i];
            }
            delete[] val;
         } break;

         case btObjectArray_t : {            // Read Data
            btObjectArray      val;
            btUnsigned32bitInt i, Num = 0;
            btUnsigned32bitInt ThisTime;

            if ( EOF == CNamedValueSet::ReadUnsigned(file, &Num) ) {  // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                            // crazy value, but possible, end now
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesIndexOutOfRange);
            }

            if ( NULL == (val = new btObjectType[Num]) ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesOutOfMemory);
            }

            btUnsigned64bitInt u64;
            for ( i = 0 ; i < Num ; ++i ) {                     // read the array
               u64 = 0;
               ThisTime = fscanf(file, "%llu", &u64);
               if ( ( 1 != ThisTime ) || ferror(file) ) {
                  delete[] val;
                  return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
               }
               val[i] = reinterpret_cast<btObjectType>(u64);
            }

            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         case btUnknownType_t : return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);
         default              : break;

      } // switch typeData

      if ( NULL != sName ) {                // Free read in name
         delete[] sName;
         sName = NULL;
      }
   }  // for loop

   return ENamedValuesOK;
}  // NVSReadNVS
#ifdef _MSC_VER
   #pragma warning(pop)      // fscanf
#endif // _MSC_VER
#endif /* #ifdef NVSFileIO */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@                  S T R E A M   I / O                       @@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

//=============================================================================
// Name: NVSWriteLevel - write tabs so that entries will be nicely indented
//=============================================================================
void CNamedValueSet::WriteLevel(std::ostream &os, unsigned level)
{
   while ( level-- ) {
      os << "\t";
   }
}

//=============================================================================
// Name: NVSWriteUnsigned - write a single unsigned, at the level passed in, followed by a space
//=============================================================================
void CNamedValueSet::WriteUnsigned(std::ostream &os, btUnsignedInt u, unsigned level)
{
   CNamedValueSet::WriteLevel(os, level);
   os << u << " ";
}

//=============================================================================
// Name: NVSReadUnsigned - read a single unsigned integer, ignoring preceeding whitespace
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
btBool CNamedValueSet::ReadUnsigned(std::istream &is, btUnsignedInt *pu)
{
   is >> *pu;
   return is.good();    // true implies good health on the stream
}

//=============================================================================
// Name: NVSReadNumberKey - read a number that is supposed to represent a number key
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
btBool CNamedValueSet::ReadNumberKey(std::istream &is, btNumberKey *pu)
{
   is >> *pu;
   return is.good();    // true implies good health on the stream
}

//=============================================================================
// Name: WriteString to an open file
//       Format is LEN STR\n, e.g. 10 HelloWorld\n
//       not expecting errors here
//       level preceeds string with an appropriate number of tabs
//=============================================================================
void CNamedValueSet::WriteString(std::ostream &os, btcString sz, unsigned level)
{
   CNamedValueSet::WriteLevel(os, level);
   os << (unsigned)strlen(sz) << " " << sz << "\n";
}

//=============================================================================
// Name: NVSReadString from an open file - must free the result if not NULL
//       Format is LEN STRING, e.g. 5 hello\n
//          optionally preceeded by whitespace
//       Input is open file
//       Returns pointer to malloc'd buffer if successful, NULL if not
//=============================================================================
char * CNamedValueSet::ReadString(std::istream &is)
{
   btUnsignedInt szlen = 0;

   if ( !CNamedValueSet::ReadUnsigned(is, &szlen) ) {
      return NULL;                  // Get length of string
   }

   if ( szlen > 256 ) {
      std::clog << "WARNING: NVSReadString: input length " << szlen << "greater than 256" << std::endl;
   }

   btString psz = new btByte[szlen + 1]; // get an input buffer, exception if NULL
                                         // valid index is [0] to [szlen]
   if ( NULL != psz ) {                  // new worked, read the string
      is.ignore(1);                      // eat the intervening space
      // incorporate terminating \n
      is.read(psz, szlen+1);
      psz[szlen] = 0;                    // overwrite terminating \n
   } else {                              // should have thrown an exception
      std::cerr << "ERROR: NVSReadString: out of memory\n" << std::endl;
      // leave null psz for return
   }

   return psz;
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void CNamedValueSet::WriteName(std::ostream &os, eNameTypes typeName, btStringKey pszName, unsigned level)
{
   CNamedValueSet::WriteLevel(os, level);        // tab in
   os << static_cast<unsigned>(typeName) << " "; // type of name (string)
   CNamedValueSet::WriteString(os, pszName, 0);  // write the name
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void CNamedValueSet::WriteName(std::ostream &os, eNameTypes typeName, btNumberKey uName, unsigned level)
{
   CNamedValueSet::WriteLevel(os, level);          // tab in
   os << static_cast<unsigned>(typeName) << " ";   // type of name (int)
   os << uName << "\n";                            // write the name
}

//=============================================================================
// Name: NVSWriteEndOfNVS
//       Write an EndOfNVS marker to an open file
//       Name is hardcoded and fake - will never actually be read or used, can
//          something already in the NVS without problem
//=============================================================================
void CNamedValueSet::WriteEndOfNVS(std::ostream &os, unsigned level)
{
   CNamedValueSet::WriteName(os, btStringKey_t, "---- End of NVS ----", level);
   CNamedValueSet::WriteUnsigned(os, btEndOfNVS_t, level+1); // This is the real end marker
   os << "\n";
}  // End of NVSWriteEndOfNVS

void CNamedValueSet::WritebtFloat(std::ostream &os, btFloat f)
{
   unsigned char       *p    = reinterpret_cast<unsigned char *>(&f);
   unsigned char const *pEnd = p + sizeof(btFloat);

   os << "{ ";

   const std::ios_base::fmtflags flags = os.flags();
   const std::streamsize         width = os.width();
   const char                    fill  = os.fill();

   os.flags(std::ios::hex|std::ios::right);
   os.fill('0');

   while ( p < pEnd ) {
      unsigned u = *p;
      os.width(2);
      os << u << " ";
      ++p;
   }

   os.fill(fill);
   os.width(width);
   os.flags(flags);

   os << "}";
}

ENamedValues CNamedValueSet::ReadbtFloat(std::istream &is, btFloat *pflt)
{
   unsigned char *p = reinterpret_cast<unsigned char *>(pflt);

   while ( '{' != is.peek() ) {
      is.ignore(1);
      if ( is.eof() ) {
         return ENamedValuesInternalError_UnexpectedEndOfFile;
      }
   }

   //           11111
   // 012345678901234
   // { xx xx xx xx }
   char buf[(3 * sizeof(btFloat)) + 3];

   is.read(buf, sizeof(buf));
   if ( is.gcount() < sizeof(buf) ) {
      return ENamedValuesInternalError_UnexpectedEndOfFile;
   }

   unsigned u;
   unsigned i;
   for ( i = 0 ; i < sizeof(btFloat) ; ++i ) {
      buf[4 + (3 * i)] = 0;

      u = 0;
      sscanf(&buf[2 + (3 * i)], "%x", &u);

      *p = (unsigned char)u;
      ++p;
   }

   return ENamedValuesOK;
}

//=============================================================================
// Name: NVSWriteNVS to an open file, opened in binary mode
//       file has to be open, with EOL translation OFF (e.g. binary mode, b), file handle is not checked
//       nvsToWrite is input NVS to write to the file
//       Level is the indentation level at which the NVS is to be written, initially 0, it can be increased
//          for recursion or other special purposes. Due to the formatting conventions of always starting a line
//          with a number, indentations are stripped away as whitespace and do not cause problems
//
//       Format is:
//       NameType Name: e.g. "5 100\n" means name type of unsigned int, name is 100
//                or    e.g. "9 7 foo bar\n" means type of string, name is foo bar, of length 7
//       ValueType Value: e.g. "\t0 1\n" means:
//                   ValueType 0 is btBool
//                   Value 1 is TRUE
//                   the preceeding \t is indentation to make the file more readable
//    or ValueType NumValues Value[s]: e.g. "\t0 3 1 0 1\n" for an array means:
//                   ValueType 0 is btBool
//                   NumValues 3 is one value
//                   Value 1 0 1 is TRUE, FALSE, TRUE
//                or "\t12 3 1 2 3\n" means bt32bitIntArray_t length 3 with values 1, 2, and 3
//                or "\t20 2\n\t7 foo bar\n\t 9 foo2 bar2\n" is
//                   special because it is an array of strings, which are separated by newlines instead of spaces
//=============================================================================
ENamedValues CNamedValueSet::Write(std::ostream &os, unsigned level) const
{
   AutoLock(this);

   btUnsignedInt uElements = 0;              // number of elements in the NVS
   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName(0);                   // to hold an integer name
   btStringKey   sName(NULL);                // to hold a string name
   eBasicTypes   typeData = btUnknownType_t; // type of data

   GetNumNames(&uElements);          // initialize the loop to the number of elements in the NVS

   //Write one at a time
   for ( irg = 0 ; irg < uElements ; ++irg ) {        // walk forward through NVS
      GetNameType(irg, &typeName);         // Get name type
      switch ( typeName ) {
         case btStringKey_t : {
            GetName(irg, &sName);          // Get the name
            CNamedValueSet::WriteName(os, typeName, sName, level); // Write the name
            Type(sName, &typeData);        // Get the data type
         } break;

         case btNumberKey_t : {                       // same here, for UINT names
            GetName(irg, &iName);
            CNamedValueSet::WriteName(os, typeName, iName, level);
            Type(iName, &typeData);
         } break;

         default : {
            std::cerr << "ERROR: NVSWriteNVS: name type " << static_cast<unsigned>(typeName) << "unsupported. Aborting." << std::endl;
            return ENamedValuesInternalError_InvalidNameFormat;
         }
      }

      // write data
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // In this section, there is a matrix of typeName vs. typeData, with a different function
      //    having to be called for each. Clearly this should be templatized, or something, someday.

#define NVSWRITENVS_CASE(__t) case __t##_t : {           \
   __t __val;                                            \
   if ( btStringKey_t == typeName ) {                    \
      Get(sName, &__val);                                \
   } else {                                              \
      Get(iName, &__val);                                \
   }                                                     \
   CNamedValueSet::WriteUnsigned(os, typeData, level+1); \
   os << __val << "\n";                                  \
} break

#define NVSWRITENVS_ARRAY_CASE(__t) case __t##Array_t : { \
   __t     *__val = NULL;                                 \
   btWSSize __Num = 0;                                    \
   if ( btStringKey_t == typeName ) {                     \
      Get(sName, &__val);                                 \
      GetSize(sName, &__Num);                             \
   } else {                                               \
      Get(iName, &__val);                                 \
      GetSize(iName, &__Num);                             \
   }                                                      \
   CNamedValueSet::WriteUnsigned(os, typeData, level+1);  \
   CNamedValueSet::WriteUnsigned(os, __Num, 0);           \
   while ( __Num-- ) {                                    \
      os << *__val++ << " ";                              \
   }                                                      \
   os << "\n";                                            \
} break

      switch( typeData ) {

         NVSWRITENVS_CASE(btBool);
         NVSWRITENVS_CASE(btByte);
         NVSWRITENVS_CASE(bt32bitInt);
         NVSWRITENVS_CASE(btUnsigned32bitInt);
         NVSWRITENVS_CASE(bt64bitInt);
         NVSWRITENVS_CASE(btUnsigned64bitInt);

         case btFloat_t : {
            btFloat val = 0.0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WritebtFloat(os, val);
            os << "\n";
         } break;

         case btString_t : {
            btcString val = NULL;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WriteString(os, val, 0);
         } break;

         case btNamedValueSet_t : {
            INamedValueSet const *pval = NULL;
            if ( btStringKey_t == typeName ) {
               Get(sName, &pval);
            } else {
               Get(iName, &pval);
            }
            // NVS Name already written, write the NVS DataType
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            os << "\n";
            pval->Write(os, level+2); // Write the NVS itself
            // Write the trailer - String Name, and DataType 'End of NVS'
            //NVSWriteEndOfNVS( os, level);     // Could use this instead, but the string is
                                                // slightly different, and the archive test files
                                                // would have to change. TODO: go ahead and do this.
            CNamedValueSet::WriteName(os, btStringKey_t, "---- End of embedded NVS ----", level);
            CNamedValueSet::WriteUnsigned(os, btEndOfNVS_t, level+1); // This is the real end marker
            os << "\n";
         } break;

         case btByteArray_t : {
            btByteArray val;
            btWSSize    Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WriteUnsigned(os, Num, 0);
            os.write(val, Num);
            os << "\n";
         } break;

         NVSWRITENVS_ARRAY_CASE(bt32bitInt);
         NVSWRITENVS_ARRAY_CASE(btUnsigned32bitInt);
         NVSWRITENVS_ARRAY_CASE(bt64bitInt);
         NVSWRITENVS_ARRAY_CASE(btUnsigned64bitInt);

         case btFloatArray_t : {
            btFloat *val = NULL;
            btWSSize Num = 0;
            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WriteUnsigned(os, Num, 0);
            while ( Num-- ) {
               CNamedValueSet::WritebtFloat(os, *val++);
            }
            os << "\n";
         } break;

         case btObjectType_t : {      // Pointer, could be 32 or 64 bit
            btObjectType val = NULL;

            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
            } else {
               Get(iName, &val);
            }

            btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(val);
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            os << u64temp << "\n";
         } break;

         case btStringArray_t : {
            btString *val = NULL;
            btWSSize  Num = 0;

            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WriteUnsigned(os, Num, 0);
            os << "\n";
            while ( Num-- ) {
               CNamedValueSet::WriteString(os, *val++, level+2);
            }
         } break;

         case btObjectArray_t : {
            btObjectType *val = NULL; // pointer to array of pointers
            btWSSize      Num = 0;    // number of pointers

            if ( btStringKey_t == typeName ) {
               Get(sName, &val);
               GetSize(sName, &Num);
            } else {
               Get(iName, &val);
               GetSize(iName, &Num);
            }
            CNamedValueSet::WriteUnsigned(os, typeData, level+1);
            CNamedValueSet::WriteUnsigned(os, Num, 0);

            btUnsigned64bitInt u64temp;
            while ( Num-- ) {
               u64temp = reinterpret_cast<btUnsigned64bitInt>(*val++);
               os << u64temp << " ";
            }
            os << "\n";
         } break;

         default : break;
      } // switch typeData
   }  // for loop

   return ENamedValuesOK;
}  // NVSWriteNVS

//=============================================================================
// Name: NVSWriteOneNVSToFile
//       Write an NVS + an EndofNVS marker to an open file
//=============================================================================
ENamedValues CNamedValueSet::WriteOne(std::ostream &os, unsigned level) const
{
   ENamedValues retval = Write(os, level);
   CNamedValueSet::WriteEndOfNVS(os, level);
   return retval;
}  // End of NVSWriteOneNVSToFile

//=============================================================================
// Utility function of NVSReadNVS for handling error returns
//=============================================================================
ENamedValues CNamedValueSet::ReadNVSError(btmStringKey sName, ENamedValues error)
{
   if ( sName ) {
      delete[] sName;     // clean up before error return
   }
   return error;
}

//=============================================================================
// Name: NVSReadNVS from an open file, opened in binary mode
//       file has to be open, in binary mode (e.g. b option), file handle is not checked
//       nvsToRead is a passed in already created NVS to which all data will be appended
//          it is not checked for prior emptiness
//
//       for Format see NVSWriteNVS
//
//       Returns on:
//          Error, e.g. unknown values
//          EOF, implying finished with an NVS
//          reading btUnknownType_t, implying finished reading an EMBEDDED NVS
//=============================================================================
ENamedValues CNamedValueSet::Read(std::istream &is)
{
   AutoLock(this);

   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName    = 0;               // to hold an integer name
   btmStringKey  sName    = NULL;            // to hold a string name, returned from NVSReadString
   eBasicTypes   typeData = btUnknownType_t; // type of data
   ENamedValues  eRetVal  = ENamedValuesOK;

   //if ( NULL == nvsToRead ) {                // if return parameter NULL, abort
   //   return ENamedValuesInvalidReadToNull;
   //}

   //Read one at a time, counting as we go
   for ( irg = 0 ; ; ++irg ) {         // walk forward through NVS
      // Read NVS Name
      //       Format is TYPE NAME
      //       Where TYPE must be either btString_t or btUnsignedInt_t
      //       Where NAME is either an INTEGER or a LEN STRING, depending upon the TYPE

      // Get type of the NAME
      btUnsignedInt tempType = 0;
      if ( !CNamedValueSet::ReadUnsigned(is, &tempType) ) {
         return ENamedValuesEndOfFile;    // False means end of file or error, done with this NVS
      }
      if ( tempType != btNumberKey_t &&   // range check
           tempType != btStringKey_t ) {
         return ENamedValuesBadType;      //    error return
      }
      typeName = static_cast<eNameTypes>(tempType); // safe, because of range check

      iName = 0;

      switch ( typeName ) {                // if read of typeName successful, use it
         case btStringKey_t : {            // string name, FREE IT ONCE DONE WITH IT
            sName = CNamedValueSet::ReadString(is);
            if ( NULL == sName ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }
         } break;

         case btNumberKey_t : {            // integer name
            if ( !CNamedValueSet::ReadNumberKey(is, &iName) ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }
         } break;

         default : return ENamedValuesInternalError_InvalidNameFormat;
      }
      // At this point, know that that typeName is either btString_t or btUnsignedInt_t
      //    and corresponding name is in either sName or iName

      // Get type of the DATA - expect an unsigned here
      if ( !CNamedValueSet::ReadUnsigned(is, &tempType) ) {
         return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
      }
      if ( tempType >  btUnknownType_t &&   // range check
           tempType != btEndOfNVS_t ) {
         return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);
      }
      typeData = static_cast<eBasicTypes>(tempType); // safe, because of range check

      // READ DATA
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // If typeName is btString_t, sName must be freed. This is done at the end of the switch.
      //    Error returns or other aborts from the switch need to free sName.

#define NVSREADNVS_SINGLE(__t)                                             \
case __t##_t : {                                                           \
   __t __val;                                                              \
   is >> __val;                                                            \
   if ( !is.good() ) {                                                     \
      return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);     \
   }                                                                       \
   if ( btStringKey_t == typeName ) {                                      \
      Add(sName, __val);                                                   \
   } else {                                                                \
      Add(iName, __val);                                                   \
   }                                                                       \
} break


#define NVSREADNVS_STREAM_ARRAY(__type)                                                          \
case __type##Array_t : {                                                                         \
   __type##Array      val, p;                                                                    \
   btUnsigned32bitInt i;                                                                         \
   btUnsignedInt      Num = 0;                                                                   \
   if ( !CNamedValueSet::ReadUnsigned(is, &Num) ) { /* read number of elements */                \
      return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile); \
   }                                                                                             \
   if ( 0 == Num ) {                   /* crazy value, but possible, end now */                  \
      break;                                                                                     \
   }                                                                                             \
   val = new __type[Num];                                                                        \
   i = Num;                                                                                      \
   p = val;                                                                                      \
   while ( is && i-- ) {               /* read the array */                                      \
      is >> *p++;                                                                                \
   }                                                                                             \
   NVS_ADD_ARRAY(val, Num);                                                                      \
   delete[] val;                                                                                 \
} break

      switch ( typeData ) {

         NVSREADNVS_SINGLE(btBool);

         case btByte_t : {             // Read Data
            btByte val = 0;            // Don't use standard technique because of char 0x20 (space)
            is.ignore(1);              // remove preceeding whitespace
            is.read(&val, sizeof(btByte));
            if ( !is.good() ) {
               return ENamedValuesInternalError_UnexpectedEndOfFile;
            }
            NVS_ADD(val);
         } break;

         NVSREADNVS_SINGLE(bt32bitInt);
         NVSREADNVS_SINGLE(btUnsigned32bitInt);
         NVSREADNVS_SINGLE(bt64bitInt);
         NVSREADNVS_SINGLE(btUnsigned64bitInt);

         case btFloat_t : {
            btFloat val = 0.0;
            ENamedValues res = CNamedValueSet::ReadbtFloat(is, &val);
            if ( ENamedValuesOK != res ) {
               return CNamedValueSet::ReadNVSError(sName, res);
            }
            is.ignore(1, '\n');
            if ( btStringKey_t == typeName ) {
               Add(sName, val);
            } else {
               Add(iName, val);
            }
         } break;

         case btString_t : {              // Read Data
            btString val = CNamedValueSet::ReadString(is);
            if ( NULL == val ) {
               return CNamedValueSet::ReadNVSError( sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
            delete[] val;
         } break;

         case btNamedValueSet_t : {         // Read Data
            CNamedValueSet val;
            ENamedValues   ret;
            ret = val.Read(is);
            if ( ENamedValuesOK == ret ) {
               NVS_ADD(&val);
            } else {
               return CNamedValueSet::ReadNVSError(sName, ret);
            }
         } break;

         // Normal end of embedded NVS, not really an error, but need to free sName
         case btEndOfNVS_t  : {
            is.ignore(2, '\n');
            return CNamedValueSet::ReadNVSError(sName, ENamedValuesOK);
         }

         case btByteArray_t : {      // Read Data
            btByteArray        val;
            btUnsigned32bitInt Num = 0;

            if ( !CNamedValueSet::ReadUnsigned(is, &Num) ) {          // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                            // crazy value, but possible, end now
               break;
            }
            val = new btByte[Num+1];
            is.ignore(1);                                // eat the preceeding space
            is.read(val, Num+1);                         // load value + terminating \n
            val[Num] = 0;                                // overwrite terminating \n with \0

            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         NVSREADNVS_STREAM_ARRAY(bt32bitInt);
         NVSREADNVS_STREAM_ARRAY(btUnsigned32bitInt);
         NVSREADNVS_STREAM_ARRAY(bt64bitInt);
         NVSREADNVS_STREAM_ARRAY(btUnsigned64bitInt);

         case btFloatArray_t : {
            btFloatArray  val, p;
            btUnsignedInt i;
            btUnsignedInt Num = 0;
            if ( !CNamedValueSet::ReadUnsigned(is, &Num) ) { /* read number of elements */
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) { /* crazy value, but possible, end now */
               break;
            }
            val = new btFloat[Num];
            i = Num;
            p = val;
            while ( is && i-- ) { /* read the array */
               CNamedValueSet::ReadbtFloat(is, p++);
            }
            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         case btObjectType_t : {             // Pointer, could be 32 or 64 bit
            btObjectType       val = NULL;
            btUnsigned64bitInt u64 = 0;
            is >> u64;
            if ( !is.good() ) {
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            val = reinterpret_cast<btObjectType>(u64);
            NVS_ADD(val);
         } break;

         case btStringArray_t : {                       // Read Data
            btStringArray      val;
            btUnsigned32bitInt i, Num = 0;

            if ( !CNamedValueSet::ReadUnsigned(is, &Num) ) {         // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                           // crazy value, but legal?, end now
               break;
            }
            val = new btString[Num];

            for ( i = 0 ; i < Num ; ++i ) {             // initialize, helps with final cleanup
               val[i] = NULL;
            }
            for ( i = 0 ; i < Num ; ++i ) {             // read strings, break if run out of memory
               if ( NULL == (val[i] = CNamedValueSet::ReadString(is)) ) {
                  break;
               }
            }
            if ( i == Num ) {                           // if the loop ended properly, then got all
               NVS_ADD_ARRAY(val, Num);                 //    the strings, load the nvs
            }

            for ( i = 0 ; i < Num ; ++i ) {
               if ( NULL == val[i] ) {
                  break;
               }
               delete[] val[i];                         // clean up properly even if not all elements
            }
            delete[] val;
         } break;

         case btObjectArray_t : {                 // Read Data
            btObjectArray      val;
            btUnsigned32bitInt i;
            btUnsignedInt      Num = 0;

            if ( !CNamedValueSet::ReadUnsigned(is, &Num) ) {   // read number of elements
               return CNamedValueSet::ReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                     // crazy value, but possible, end now
               break;
            }
            val = new btObjectType[Num];

            btUnsigned64bitInt u64;
            for ( i = 0 ; is && i < Num ; ++i ) { // read the array
               u64 = 0;
               is >> u64;
               val[i] = reinterpret_cast<btObjectType>(u64);
            }

            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         case btUnknownType_t : return CNamedValueSet::ReadNVSError(sName, ENamedValuesBadType);
         default              : break;
      } // switch typeData

      if ( NULL != sName ) {                       // Free read in name
         delete[] sName;
         sName = NULL;
      }

   }  // for loop

   return eRetVal;
}  // NVSReadNVS


END_NAMESPACE(AAL)


