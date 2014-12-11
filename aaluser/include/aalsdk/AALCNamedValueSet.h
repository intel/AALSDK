// Copyright (c) 2005-2014, Intel Corporation
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
/// @file AALCNamedValueSet.h
/// @brief Concrete definition of the CNamedValueSet class.
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
/// 03/21/2007     JG       Ported to Linux
/// 05/30/2007     JG       Ported from LRNamedValues
/// 07/13/2007     JG       Fixed a bug in assigment that would
///                            cause incorrect results (could be bigger).
///                JG       Fixed also could leak.
///                JG       Added Empty() method
/// 07/20/2007     JG       Fixed bug in some array functions
///                            where Numelements wasn't passed
/// 08/01/2007     HM       Added declarations for NVSRead/WriteNVS
/// 08/08/2007     HM       Added subset() and operator== to NVS
/// 08/17/2007     HM       Modified template declaration of subset
///                            to include fEqual parameter to be used
///                            by operator== in testing embedded NVS
/// 09/14/2007     JG       Removed extraneous reference to libLongRidgeLibCPP.h
/// 10/04/2007     JG       Minor interfface changes
/// 10/31/2007     HM       #ifndef __GNUC__ away various #pragmas
/// 11/21/2007     HM       changed btString to btcString
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 02/25/2008     HM       Added public copy constructor and operator= to
///                            CNamedValueSet, calling the corrected TNamedValueSet
///                            operator=. Fixed TNamedValueSet operator= declaration.
/// 03/15/2008     HM       Code cleanup
/// 03/16/2008     HM       Reordered members of CValue to optimize spacing and
///                            access, 64-bit quantities first
/// 03/22/2008     HM       Moved NVS Read and Write and operator << >> function
///                            headers to AALNamedValueSet.h
/// 05/01/2008     JG       Added byByteArray
/// 05/07/2008     HM       Tweaks to btByteArray
/// 05/08/2008     HM       Comments & License
/// 05/26/2008     HM       Added Subset(), in preparation for deprecating
///                            subset().
/// 05/29/2008     HM       Convert btcString and btUnsignedInt key types into
///                            specific typedefs: btStringKey and btNumberKey
/// 06/24/2008     HM       Added include of stdlib.h for gcc 4.3.1
/// 06/27/2008     HM       Splitting Registrar from Database
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALCNAMEDVALUESET_H__
#define __AALSDK_AALCNAMEDVALUESET_H__
#include <aalsdk/AALDefs.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/osal/CriticalSection.h>


#ifdef _MSC_VER
   // Disable dll-interface warnings for private interfaces
   #pragma warning( disable : 4251 )
   #pragma warning( disable : 4275 )
   
   //'strdup': The POSIX name for this item is deprecated.
   #pragma warning( disable : 4996) 

   // Disable debug trunction warning due to STL template name length
   #pragma warning( disable : 4786 )
#endif

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
     #pragma warning( push)
//   #pragma warning( pop)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
//   #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif

BEGIN_NAMESPACE(AAL)

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
   class CValue
   {
      //Simple types
      typedef union Val_t {
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
         NamedValueSet          *pNVS;
      } Val_t;
   private:
      Val_t                m_Val;      // 64 bits
      btUnsigned32bitInt   m_Size;     // 32 bits
      eBasicTypes          m_Type;     // 32 bits
                                       // Total 16 bytes
   public:
      //=======================================================================
      //Constructor
      //=======================================================================
      CValue()
      :  m_Size(0),
         m_Type(btUnknownType_t)
      {
         m_Val.Obj=NULL;
      }

      //=======================================================================
      // Copy Constructor
      //=======================================================================
      CValue(const CValue &rOther)
      :  m_Size(0),
         m_Type(btUnknownType_t)
      {
         m_Val.Obj=NULL;

         //Assignment operator does the real work
         *this = rOther;
      }

      //=======================================================================
      //Destructor
      //=======================================================================
//      virtual ~CValue() - not needed as not derived from, and adds overhead to object
      ~CValue()
      {

         // =======================================
         // If the value is not a normal scaler
         // delete any allocated arrays and objects
         //========================================
         switch(m_Type)
         {
         case btByteArray_t:
            delete [] m_Val._8bA;
            break;
         case bt32bitIntArray_t:
            delete [] m_Val._32bA;
            break;
         case btUnsigned32bitIntArray_t:
            delete [] m_Val._U32bA;
            break;
         case bt64bitIntArray_t:
            delete [] m_Val._64bA;
            break;
         case btUnsigned64bitIntArray_t:
            delete [] m_Val._U64bA;
            break;
         case btFloatArray_t:
            delete [] m_Val.fltA;
            break;
         case btString_t:
            free(m_Val.str);
            break;
         case btStringArray_t:
         {
            for ( unsigned i=0; i<m_Size; i++ ) {
               free ( m_Val.strA[i] );
            }
            delete [] m_Val.strA;
            break;
         }
         case btObjectArray_t:
            delete [] m_Val.ObjA;
            break;

         case btNamedValueSet_t:
            delete m_Val.pNVS;
            break;
         default:
            {
               break;
            }
         }//End case
      }

      //=======================================================================
      //Assignment
      //=======================================================================
      CValue & operator = (const CValue &rOther)
      {
         if ( &rOther == this ) {
            return *this; // don't duplicate self
         }

         m_Type = rOther.m_Type;
         m_Size = rOther.m_Size;

         //Copy array types using mutator
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

            default:
               //Otherwise simple value
               m_Val=rOther.m_Val;
            break;
         }

         return *this;
      }

      //=======================================================================
      //Type Accessors
      //=======================================================================
      eBasicTypes        Type() const { return m_Type; }
      btUnsigned32bitInt Size() const { return m_Size; }

      //=======================================================================
      //Single CValue Mutators
      //=======================================================================
      void Put(btBool val)             { m_Type = btBool_t;             m_Val._1b   = val; m_Size = 1; }
      void Put(btByte val)             { m_Type = btByte_t;             m_Val._8b   = val; m_Size = 1; }
      void Put(bt32bitInt val)         { m_Type = bt32bitInt_t;         m_Val._32b  = val; m_Size = 1; }
      void Put(btUnsigned32bitInt val) { m_Type = btUnsigned32bitInt_t; m_Val._U32b = val; m_Size = 1; }
      void Put(bt64bitInt val)         { m_Type = bt64bitInt_t;         m_Val._64b  = val; m_Size = 1; }
      void Put(btUnsigned64bitInt val) { m_Type = btUnsigned64bitInt_t; m_Val._U64b = val; m_Size = 1; }
      void Put(btFloat val)            { m_Type = btFloat_t;            m_Val.flt   = val; m_Size = 1; }
      void Put(btObjectType val)       { m_Type = btObjectType_t;       m_Val.Obj   = val; m_Size = 1; }

      //=======================================================================
      // Complex CValue Mutators
      //=======================================================================
      void Put(NamedValueSet const &val)
      {
         m_Type=btNamedValueSet_t;
         m_Val.pNVS = new NamedValueSet(val);
         m_Size=1;
      }

      void Put(btcString val)
      {
         m_Type    = btString_t;
         ASSERT(NULL == m_Val.str); // Leak, otherwise
         m_Val.str = strdup(val);
         m_Size    = 1;
      }

      //=======================================================================
      //Array CValue Mutators
      //=======================================================================
      void Put(btByteArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val._8bA = new btByte[Num];
         memcpy(m_Val._8bA,val,(sizeof(btByte)*Num));
         m_Type=btByteArray_t;m_Size=Num;
      }

      void Put(bt32bitIntArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val._32bA = new bt32bitInt[Num];
         memcpy(m_Val._32bA,val,(sizeof(bt32bitInt)*Num));
         m_Type=bt32bitIntArray_t;m_Size=Num;
      }

      void Put(btUnsigned32bitIntArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val._U32bA = new btUnsigned32bitInt[Num];
         memcpy(m_Val._U32bA,val,(sizeof(btUnsigned32bitInt)*Num));
         m_Type=btUnsigned32bitIntArray_t;m_Size=Num;
      }

      void Put(bt64bitIntArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val._64bA = new bt64bitInt[Num];
         memcpy(m_Val._64bA,val,(sizeof(bt64bitInt)*Num));
         m_Type=bt64bitIntArray_t;m_Size=Num;
      }

      void Put(btUnsigned64bitIntArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val._64bA = new bt64bitInt[Num];
         memcpy(m_Val._U64bA,val,(sizeof(bt64bitInt)*Num));
         m_Type=btUnsigned64bitIntArray_t;m_Size=Num;
      }

      void Put(btFloatArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val.fltA = new btFloat[Num];
         memcpy(m_Val.fltA,val,(sizeof(btFloat)*Num));
         m_Type=btFloatArray_t;m_Size=Num;
      }

      void Put(btStringArray val, btUnsigned32bitInt NumElements)
      {
         m_Type=btStringArray_t;
         m_Size=NumElements;
         // Copy the array of btStrings and create btString array
         m_Val.strA = new btString[NumElements];
         for(btUnsigned32bitInt x=0; x < NumElements; x++)
         {
            //Copy string
            m_Val.strA[x] = strdup(val[x]);
         }

      }

      void Put(btObjectArray val,btUnsigned32bitInt Num)
      {
         //Allocate space for local array copy
         m_Val.ObjA = new btObjectType[Num];
         memcpy(m_Val.ObjA ,val,(sizeof(btObjectType)*Num));
         m_Type=btObjectArray_t;m_Size=Num;
      }

      //=======================================================================
      //Accessors
      //=======================================================================
      ENamedValues Get(bt32bitInt *pval) const
      {
         if(m_Type!=bt32bitInt_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val._32b;
         return ENamedValuesOK;
      }

      ENamedValues Get(btBool *pval) const
      {
         if(m_Type!=btBool_t)
         {
            *pval=false;
            return ENamedValuesBadType;
         }
         *pval=m_Val._1b;
         return ENamedValuesOK;
      }

      ENamedValues Get(btByte *pval) const
      {
         if(m_Type!=btByte_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val._8b;
         return ENamedValuesOK;
      }


      ENamedValues Get(btUnsigned32bitInt *pval) const
      {
         if(m_Type!=btUnsigned32bitInt_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val._U32b;
         return ENamedValuesOK;
      }

      ENamedValues Get(bt64bitInt *pval) const
      {
         if(m_Type!=bt64bitInt_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val._64b;
         return ENamedValuesOK;
      }

      ENamedValues Get(btUnsigned64bitInt *pval) const
      {
         if(m_Type!=btUnsigned64bitInt_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val._U64b;
         return ENamedValuesOK;
      }

      ENamedValues Get(btFloat *pval) const
      {
         if(m_Type!=btFloat_t)
         {
            *pval=0;
            return ENamedValuesBadType;
         }
         *pval=m_Val.flt;
         return ENamedValuesOK;
      }

      ENamedValues Get( btcString *pval) const
      {
         if(m_Type!=btString_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val.str;
         return ENamedValuesOK;
      }

      ENamedValues Get( NamedValueSet const **pval) const
      {
         if(m_Type!=btNamedValueSet_t)
         {
            return ENamedValuesBadType;
         }
         *pval=m_Val.pNVS;
         return ENamedValuesOK;
      }

      ENamedValues Get(btObjectType *pval) const
      {
         if(m_Type!=btObjectType_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val.Obj;
         return ENamedValuesOK;
      }

      ENamedValues Get(btByteArray *pval) const
      {
         if(m_Type!=btByteArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val._8bA;
         return ENamedValuesOK;
      }

      ENamedValues Get(bt32bitIntArray *pval) const
      {
         if(m_Type!=bt32bitIntArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val._32bA;
         return ENamedValuesOK;
      }

      ENamedValues Get(btUnsigned32bitIntArray *pval) const
      {
         if(m_Type!=btUnsigned32bitIntArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val._U32bA;
         return ENamedValuesOK;
      }

      ENamedValues Get(bt64bitIntArray *pval) const
      {
         if(m_Type!=bt64bitIntArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val._64bA;
         return ENamedValuesOK;
      }

      ENamedValues Get(btUnsigned64bitIntArray *pval) const
      {
         if(m_Type!=btUnsigned64bitIntArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val._U64bA;
         return ENamedValuesOK;
      }

      ENamedValues Get(btFloatArray *pval) const
      {
         if(m_Type!=btFloatArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val.fltA;
         return ENamedValuesOK;
      }

      ENamedValues Get( btStringArray *pval) const
      {
         if(m_Type!=btStringArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val.strA;
         return ENamedValuesOK;
      }

      ENamedValues Get(btObjectArray *pval) const
      {
         if(m_Type!=btObjectArray_t)
         {
            *pval=NULL;
            return ENamedValuesBadType;
         }
         *pval=m_Val.ObjA;
         return ENamedValuesOK;
      }

   }; // End of Class CValue definition

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
   template<class Kt>  class TNamedValueSet : public CriticalSection
   {
   private:
      std::map<Kt,CValue> m_NVSet;

   public:
      TNamedValueSet(void);
      TNamedValueSet(const TNamedValueSet &rOther);
      TNamedValueSet & operator = (const TNamedValueSet &rOther);
      btBool Subset(const TNamedValueSet &rOther, btBool fEqual=false) const;
      btBool subset(const TNamedValueSet &rOther, btBool fEqual=false) const { return Subset(rOther, fEqual); }
      btBool operator==(const TNamedValueSet &rOther) const;
      virtual ~TNamedValueSet(void);

   public:
      ENamedValues Add( Kt Name,btBool CValue);
      ENamedValues Add( Kt Name,btByte CValue);
      ENamedValues Add( Kt Name,bt32bitInt CValue);
      ENamedValues Add( Kt Name,btUnsigned32bitInt  CValue);
      ENamedValues Add( Kt Name,bt64bitInt CValue);
      ENamedValues Add( Kt Name,btUnsigned64bitInt CValue);
      ENamedValues Add( Kt Name,btFloat CValue);
      ENamedValues Add( Kt Name,btcString CValue);
      ENamedValues Add( Kt Name,NamedValueSet const &CValue);
      ENamedValues Add( Kt Name,btByteArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,bt32bitIntArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,btUnsigned32bitIntArray  CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,bt64bitIntArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,btUnsigned64bitIntArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,btObjectType CValue);
      ENamedValues Add( Kt Name,btFloatArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,btStringArray CValue,
                        btUnsigned32bitInt NumElements);
      ENamedValues Add( Kt Name,btObjectArray CValue,
                        btUnsigned32bitInt NumElements);

      ENamedValues Get( Kt Name,btBool *pValue)const;
      ENamedValues Get( Kt Name,btByte *pValue)const;
      ENamedValues Get( Kt Name,bt32bitInt *pValue)const;
      ENamedValues Get( Kt Name,btUnsigned32bitInt  *pValue)const;
      ENamedValues Get( Kt Name,bt64bitInt *pValue)const;
      ENamedValues Get( Kt Name,btUnsigned64bitInt *pValue)const;
      ENamedValues Get( Kt Name,btFloat *pValue)const;
      ENamedValues Get( Kt Name,btcString *pValue)const;
      ENamedValues Get( Kt Name,NamedValueSet const **pValue)const;
      ENamedValues Get( Kt Name,btByteArray *pValue)const;
      ENamedValues Get( Kt Name,bt32bitIntArray *pValue)const;
      ENamedValues Get( Kt Name,btUnsigned32bitIntArray  *pValue)const;
      ENamedValues Get( Kt Name,bt64bitIntArray *pValue)const;
      ENamedValues Get( Kt Name,btUnsigned64bitIntArray *pValue)const;
      ENamedValues Get( Kt Name,btObjectType *pValue)const;
      ENamedValues Get( Kt Name,btFloatArray *pValue)const;
      ENamedValues Get( Kt Name,btStringArray *pValue)const;
      ENamedValues Get( Kt Name,btObjectArray *pValue)const;

      ENamedValues Delete( Kt Name);
      ENamedValues Empty();

      ENamedValues GetSize( Kt Name, btWSSize *pSize)const;
      ENamedValues Type( Kt Name,eBasicTypes *pType)const;

      ENamedValues GetNumNames(btUnsignedInt *pNum)const;
      ENamedValues GetName( btUnsignedInt index,Kt *pName)const;

      //=============================================================
      // Specialized template function for std:string keys
      // The default GetName implementation will not work for
      // btcString keys. btcString keys are stored as std:strings in
      // the map structure. So the normal return won't work.
      // This specialized template function handles this one
      // exception case.
      //=============================================================
      ENamedValues  GetName( btUnsignedInt index, btStringKey *pName)const
      {
         AutoLock(this);
         std::map< std::string,CValue>::const_iterator itr=m_NVSet.end();

         //Find the named value pair
         if(m_NVSet.size() < (unsigned) index ) {
            return ENamedValuesNameNotFound;
         }

         //Get the right entry
         for(itr=m_NVSet.begin(); index!=0; index--,itr++);

         //Return the name
         *pName = const_cast<btStringKey>((*itr).first.c_str());

         return ENamedValuesOK;
      }

      btBool Has(Kt Name) const;

   }; // End of template<class Kt>  class TNamedValueSet : public CriticalSection

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
class CNamedValueSet : public INamedValueSet
{
private:
   TNamedValueSet<btNumberKey> m_iNVS;
//#ifndef _NO_STRING_KEYS_
   TNamedValueSet<std::string> m_sNVS;  // Note that the interface is btStringKey, which is const char*
//#endif

public:
   /// CNamedValueSet Default Constructor.
   CNamedValueSet() {}
   /// CNamedValueSet Destructor.
   virtual ~CNamedValueSet() {}
   /// Assign Named Value Set to another.
   CNamedValueSet & operator = (const CNamedValueSet &rOther) {
      if ( &rOther != this ) {   //Don't duplicate yourself
         // Make sure this NVS is empty
         m_iNVS.Empty();
         m_sNVS.Empty();
         m_iNVS = rOther.m_iNVS;
         m_sNVS = rOther.m_sNVS;
      }
      return *this;
   }
   /// CNamedValueSet Copy Constructor.
   CNamedValueSet(const CNamedValueSet &rOther) {
      // Do the copy
      m_iNVS = rOther.m_iNVS;
      m_sNVS = rOther.m_sNVS;
   }

   ENamedValues Add(btNumberKey Name, btBool value)               { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btByte value)               { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, bt32bitInt value)           { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btUnsigned32bitInt value)   { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, bt64bitInt value)           { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btUnsigned64bitInt value)   { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btFloat value)              { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btcString value)            { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, NamedValueSet const &value) { return m_iNVS.Add(Name, value); }
   ENamedValues Add(btNumberKey        Name,
                    btByteArray        value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey        Name,
                    bt32bitIntArray    value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned32bitIntArray value,
                    btUnsigned32bitInt      NumElements)  { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey        Name,
                    bt64bitIntArray    value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned64bitIntArray value,
                    btUnsigned32bitInt      NumElements)  { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btObjectType value) { return m_iNVS.Add(Name, value);              }
   ENamedValues Add(btNumberKey        Name,
                    btFloatArray       value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey        Name,
                    btStringArray      value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey        Name,
                    btObjectArray      value,
                    btUnsigned32bitInt NumElements)       { return m_iNVS.Add(Name, value, NumElements); }

   ENamedValues Get(btNumberKey Name, btBool *pValue) const                  { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByte *pValue) const                  { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitInt *pValue) const              { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitInt  *pValue) const     { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitInt *pValue) const              { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitInt *pValue) const      { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloat *pValue) const                 { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btcString *pValue) const               { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, NamedValueSet const **pValue) const    { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByteArray *pValue) const             { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitIntArray *pValue) const         { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitIntArray *pValue) const         { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectType *pValue) const            { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloatArray *pValue) const            { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btStringArray *pValue) const           { return m_iNVS.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectArray *pValue) const           { return m_iNVS.Get(Name, pValue); }

   ENamedValues Delete(btNumberKey Name)                                     { return m_iNVS.Delete(Name);        }
   ENamedValues GetSize(btNumberKey Name, btWSSize *pSize) const             { return m_iNVS.GetSize(Name,pSize); }
   ENamedValues Type(btNumberKey Name, eBasicTypes *pType) const             { return m_iNVS.Type(Name,pType);    }
   btBool       Has(btNumberKey Name) const                                  { return m_iNVS.Has(Name);           }
   ENamedValues GetName(btUnsignedInt index, btNumberKey *pName) const
   {
      eNameTypes Type;
      GetNameType(index,&Type);
      if(Type != btNumberKey_t)
         return ENamedValuesBadType;
      //std::string strName;
      return m_iNVS.GetName(index, pName);
   }


#ifndef _NO_STRING_KEYS_
   ENamedValues Add(btStringKey Name, btBool value)               { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btByte value)               { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, bt32bitInt value)           { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btUnsigned32bitInt value)   { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, bt64bitInt value)           { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btUnsigned64bitInt value)   { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btFloat value)              { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btcString value)            { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey Name, NamedValueSet const &value) { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey        Name,
                    btByteArray        value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey        Name,
                    bt32bitIntArray    value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned32bitIntArray value,
                    btUnsigned32bitInt      NumElements)  { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey        Name,
                    bt64bitIntArray    value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned64bitIntArray value,
                    btUnsigned32bitInt      NumElements)  { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btObjectType value) { return m_sNVS.Add(Name, value); }
   ENamedValues Add(btStringKey        Name,
                    btFloatArray       value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey        Name,
                    btStringArray      value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey        Name,
                    btObjectArray      value,
                    btUnsigned32bitInt NumElements)       { return m_sNVS.Add(Name, value, NumElements); }

   ENamedValues Get(btStringKey Name, btBool *pValue) const                  { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByte *pValue) const                  { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitInt *pValue) const              { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitInt *pValue) const      { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitInt *pValue) const              { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitInt *pValue) const      { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloat *pValue) const                 { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btcString *pValue) const               { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, NamedValueSet const **pValue) const    { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByteArray *pValue) const             { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitIntArray *pValue) const         { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitIntArray *pValue) const         { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectType *pValue) const            { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloatArray *pValue) const            { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btStringArray *pValue) const           { return m_sNVS.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectArray *pValue) const           { return m_sNVS.Get(Name, pValue); }

   ENamedValues Delete(btStringKey Name)                                   { return m_sNVS.Delete(Name);        }
   ENamedValues GetSize(btStringKey Name, btWSSize *pSize) const           { return m_sNVS.GetSize(Name,pSize); }
   ENamedValues Type(btStringKey Name, eBasicTypes *pType) const           { return m_sNVS.Type(Name,pType);    }
   btBool Has(btStringKey Name) const                                      { return m_sNVS.Has(Name);           }

   ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const
   {
      eNameTypes Type;
      GetNameType(index,&Type);
      if(Type != btStringKey_t)
         return ENamedValuesBadType;

      //Adjust the index for non-string keys
      btUnsignedInt iNum;
      m_iNVS.GetNumNames(&iNum);
      index -= iNum;
      ENamedValues ret = m_sNVS.GetName(index, pName);
      return ret;
   }

#endif   // #ifndef _NO_STRING_KEYS_

   //--------------------------------------------------------------------------
   // Regular functions, not distinguished by key type
   //--------------------------------------------------------------------------

   ENamedValues Empty()          // Force the NVS to delete all its members
   {
      ENamedValues result;
      result = m_iNVS.Empty();
      if( result  != ENamedValuesOK)
         return result;

#ifndef _NO_STRING_KEYS_
      result = m_sNVS.Empty();
#endif
      return result;
   }

   btBool subset( const INamedValueSet& rOther) const { return Subset(rOther);}

   btBool Subset( const INamedValueSet& rOther) const {
      const CNamedValueSet& rCNVS = dynamic_cast<const CNamedValueSet&>(rOther);
      //return (m_iNVS.Subset(rCNVS.m_iNVS) && m_sNVS.Subset(rCNVS.m_sNVS));
      btBool iRet = m_iNVS.Subset(rCNVS.m_iNVS);
      btBool sRet = m_sNVS.Subset(rCNVS.m_sNVS);
      return (iRet && sRet);
   }

   btBool operator==( const INamedValueSet& rOther) const   {
      const CNamedValueSet& rCNVS = dynamic_cast<const CNamedValueSet&>(rOther);
      //return (m_iNVS==rCNVS.m_iNVS) && (m_sNVS==rCNVS.m_sNVS);
      btBool iRet = (m_iNVS==rCNVS.m_iNVS);
      btBool sRet = (m_sNVS==rCNVS.m_sNVS);
      return (iRet && sRet);
   }

   ENamedValues GetNumNames(btUnsignedInt *pNum)const
   {
      btUnsignedInt piNum,psNum;

      ENamedValues result = m_iNVS.GetNumNames(&piNum);
      if( result  != ENamedValuesOK)
         return result;

      result = m_sNVS.GetNumNames(&psNum);
      if( result  != ENamedValuesOK)
         return result;

      //Return the sum of both NVS
      *pNum= piNum + psNum;
      return result;
   }

   ENamedValues GetNameType( btUnsignedInt index, eNameTypes *pType)const
   {
      btUnsignedInt piNum;
      btUnsignedInt NumNames;

      //Get the total number of names
      GetNumNames(&NumNames);
      if(index >NumNames)
         return ENamedValuesIndexOutOfRange;

      //Get the total names that are indexed by integers
      ENamedValues result=m_iNVS.GetNumNames(&piNum);

      //Integers keys start at index zero
      index >= piNum ? *pType = btStringKey_t : *pType = btNumberKey_t;
      return result;
   }

}; // End of class CNamedValueSet

END_NAMESPACE(AAL)

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
   #pragma warning( pop)
#endif


// //=============================================================================
// // Name: operator << on NamedValueSet
// // Description: serializes a NamedValueSet to the file
// // Interface: public
// // Inputs: stream, and NamedValueSet
// // Outputs: serialized NamedValueSet
// // Comments: works for writing to any of cout, ostream, ofstream,
// //           ostringstream, fstream, stringstream
// //=============================================================================
// ostream& operator << (ostream& s, const NamedValueSet& nvs)
// {
//    NVSWriteNVS( s, nvs, 0);
//    return s;
// }
//
// //=============================================================================
// // Name: operator >> on NamedValueSet
// // Description: reads a serialized NamedValueSet from a file
// // Interface: public
// // Inputs: stream, and NamedValueSet
// // Outputs: none
// // Comments: works for reading from of cin, istream, ifstream,
// //           iostringstream, fstream, stringstream
// // Comments: The passed in NamedValueSet is not emptied first, so if it contains
// //           information already, that will simply be added to.
// //=============================================================================
// istream& operator >> (istream& s, NamedValueSet& rnvs)
// {
//    ENamedValues eRet = NVSReadNVS( s, static_cast<NamedValueSet*>(&rnvs));
//    if (ENamedValuesOK != eRet) {
//       s.setstate(ios_base::failbit);
//    }
//    return s;
// }


#endif // __AALSDK_AALCNAMEDVALUESET_H__

