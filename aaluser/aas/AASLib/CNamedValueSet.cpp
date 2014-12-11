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
/// @file CNamedValueSet.cpp
/// @brief Concrete implementations of the Named Value Set classes.
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
#include "aalsdk/AALCNamedValueSet.h"

USING_NAMESPACE(std)

#define MAX_VALID_NVS_ARRAY_ENTRIES (1024 * 1024)



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

INamedValueSet::~INamedValueSet() {}

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
// Name: TNamedValuesSet
// Description: Constructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
template<class Kt>
TNamedValueSet<Kt>::TNamedValueSet(void) {}

//=============================================================================
// Name: TNamedValues
// Description: Copy Constructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
template<class Kt>
TNamedValueSet<Kt>::
TNamedValueSet(const TNamedValueSet<Kt> &rOther) {
   *this=(TNamedValueSet<Kt> &)rOther;
}

//=============================================================================
// Name: TNamedValues
// Description: Assignment
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
template<class Kt>
TNamedValueSet<Kt> &TNamedValueSet<Kt>::
operator =(const TNamedValueSet<Kt> &rOther) {
   AutoLock(this);
   btUnsignedInt NumNames;
   Kt CurrName;
   eBasicTypes Type;

   //Ignore assigning self to self
   if(this == &rOther) {
      return( *this);
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
            NamedValueSet const *pval;
            rOther.Get(CurrName,&pval);
            Add(CurrName,*pval);
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
template<class Kt>
btBool TNamedValueSet<Kt>::Subset(const TNamedValueSet<Kt> &rOther,
                                  btBool fEqual) const
{
   AutoLock(this);
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
            const NamedValueSet *valThis, *valOther;
            Get(CurrName,&valThis);
            rOther.Get(CurrName,&valOther);
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
// Name: TNamedValues
// Description: operator ==
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Algorithm is to check if the number of elements is equal, and if
//           so, then do a subset. If the subset is true and the number of
//           elements are equal, the NVS's must be equal.
//=============================================================================
template<class Kt>
btBool TNamedValueSet<Kt>::operator==(const TNamedValueSet<Kt> &rOther) const {
   AutoLock(this);
   btUnsignedInt NumNamesThis, NumNamesOther;

   GetNumNames(&NumNamesThis);
   rOther.GetNumNames(&NumNamesOther);

   if (NumNamesThis == NumNamesOther)
      return Subset(rOther,true);        // Is this a subset of rOther?, and
   //   are embedded NVS's also ==
   else
      return false;
}  // end of operator ==

//=============================================================================
// Name: Empty
// Description: Empties the NVS
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Iterate through list and free all named value sets.
//=============================================================================
template<class Kt>
ENamedValues TNamedValueSet<Kt>::Empty(void) {
   AutoLock(this);

   btUnsignedInt NumNames;
   Kt CurrName;

   GetNumNames(&NumNames);

   while(NumNames--) {
      GetName(NumNames,&CurrName);
      //Remove it
      ENamedValues result;
      result = Delete(CurrName);
      if(result != ENamedValuesOK) {
         return result;
      }
   }
   return ENamedValuesOK;
}

//=============================================================================
// Name: ~TNamedValues
// Description: Destructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Iterate through list and free all named value sets.
//=============================================================================
template<class Kt>
TNamedValueSet<Kt>::~TNamedValueSet(void) {
   AutoLock(this);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btBool value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btByte value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();
   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         bt32bitInt value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btUnsigned32bitInt  value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);

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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         bt64bitInt value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btUnsigned64bitInt value) {
   AutoLock(this);
   CValue tempVal;
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btFloat value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btcString value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
   return ENamedValuesOK;
}

//=============================================================================
// Name: Add
// Description: Add a NamedValueSet value
// Interface: public
// Inputs: Name - Parameter name.
//         Value - Value
// Outputs: none.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         NamedValueSet const &value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         bt32bitIntArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   CValue tempVal;
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btByteArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   CValue tempVal;
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btUnsigned32bitIntArray  value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         bt64bitIntArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);

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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btUnsigned64bitIntArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btObjectType value) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btFloatArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();


   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btStringArray value,
                         btUnsigned32bitInt NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();


   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Add( Kt Name,
                         btObjectArray value,
                         btUnsigned32bitInt  NumElements) {
   AutoLock(this);
   typename std::map<Kt,CValue>::iterator itr=m_NVSet.end();

   //Check for exclusivity
   if(m_NVSet.find(Name) != itr) {
      return ENamedValuesDuplicateName;
   }

   //Store the value
   m_NVSet[Name].Put(value,NumElements);
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btBool *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a btByte value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btByte *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}



//=============================================================================
// Name: Get
// Description: Get a 32bitInt value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         bt32bitInt *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a Unsigned32bitInt value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btUnsigned32bitInt *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a 64bitInt value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         bt64bitInt *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a Unsigned64bitInt value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btUnsigned64bitInt *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a float value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btFloat *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a string value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btcString *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a NamedValueSet value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         NamedValueSet const **pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a ByteArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btByteArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a 32bitIntArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         bt32bitIntArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a Unsigned32bitIntArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btUnsigned32bitIntArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a 64bitIntArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         bt64bitIntArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a Unsigned64bitIntArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,
                         btUnsigned64bitIntArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a ObjectType value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,btObjectType *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a floatArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,btFloatArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);

}

//=============================================================================
// Name: Get
// Description: Get a stringArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,btStringArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Get
// Description: Get a ObjectArray value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: pvalue - Place to return value.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Get( Kt Name,btObjectArray *pvalue)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }
   return (*itr).second.Get(pvalue);
}

//=============================================================================
// Name: Delete
// Description: Delete a named value
// Interface: public
// Inputs: Name - Parameter name.
// Outputs: none.
// Comments:
//=============================================================================
template<class Kt>
ENamedValues  TNamedValueSet<Kt>::Delete( Kt Name) {
   AutoLock(this);

   //Find the named value pair
   if(m_NVSet.find(Name) == m_NVSet.end()) {
      return ENamedValuesNameNotFound;
   }

   //Remove the entry from the set
   m_NVSet.erase(Name);

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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::GetSize(Kt        Name,
                            btWSSize *pSize) const {
   AutoLock(this);

   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::Type( Kt Name,eBasicTypes *pType)const {
   AutoLock(this);
   CValue tempVal;
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if((itr=m_NVSet.find(Name)) == m_NVSet.end()) {
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::GetNumNames(btUnsignedInt *pNum)const {
   AutoLock(this);
   *pNum=static_cast<btUnsignedInt>(m_NVSet.size());  // size_t truncation to int possible
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
template<class Kt>
ENamedValues
TNamedValueSet<Kt>::GetName( btUnsignedInt index,
                             Kt *pName)const {
   AutoLock(this);
   typename std::map<Kt,CValue>::const_iterator itr;

   //Find the named value pair
   if(m_NVSet.size() < (unsigned) index ) {
      return ENamedValuesNameNotFound;
   }

   //Get the right entry
   for(itr=m_NVSet.begin(); index!=0; index--,itr++) {/*empty*/}

   //Return the name
   *pName = (Kt)(*itr).first;

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
template<class Kt>
btBool  TNamedValueSet<Kt>::Has( Kt Name)const {
   AutoLock(this);
   CValue tempVal;
   typename std::map<Kt,CValue>::const_iterator itr=m_NVSet.end();

   //Find the named value pair
   if(m_NVSet.find(Name) == itr) {
      return false;
   }
   return true;
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@                    G E N E R I C S                         @@@@@@@@*/
/*@@@@@@@                                                            @@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/// NamedValueSet factory.
/// @ingroup BasicTypes
///
/// @return Pointer to newly-allocated INamedValuesSet interface.
INamedValueSet * _NewNamedValueSet() {
   return new CNamedValueSet;
}

/// Deletes an INamedValueSet object.
/// @ingroup BasicTypes
void _DeleteNamedValueSet(INamedValueSet *pNVS) {
   if ( NULL != pNVS ) {
      delete pNVS;
   }
}

/// Copies a Named Value Set.
/// @ingroup BasicTypes
///
/// @param[out]  pTarget  Pointer to target INamedValueSet interface.
/// @param[in]   pOther   Named Value Set to copy.
void _DupNamedValueSet(INamedValueSet *pTarget,
                       INamedValueSet *pOther)
{
   *(dynamic_cast<CNamedValueSet*>(pTarget)) = *(dynamic_cast<CNamedValueSet*>(pOther));
}

//=============================================================================
// Name:        operator << on NamedValueSet
// Description: serializes a NamedValueSet to a stream
// Interface:   public
// Inputs:      stream, and NamedValueSet
// Outputs:     serialized NamedValueSet
// Comments:    works for writing to any of cout, ostream, ofstream,
//                 ostringstream, fstream, stringstream
//=============================================================================
std::ostream & operator << (std::ostream &s, const NamedValueSet &nvs)
{
   NVSWriteNVS(s, nvs, 0);
   return s;
}

//=============================================================================
// Name:        operator << on FormattedNVS
// Description: serializes a NamedValueSet to a stream in a formatted manner
// Interface:   public
// Inputs:      stream, FormattedNVS which was constructed from a
//                 NamedValueSet, a tab value, and a debug value
// Outputs:     serialized formatted NamedValueSet
// Comments:    works for writing to any of cout, ostream, ofstream,
//                 ostringstream, fstream, stringstream
//              FormattedNVS is defined in AALNamedValueSet.h
//=============================================================================
std::ostream & operator << (std::ostream &s, const FormattedNVS &fnvs)
{
   NVSWriteNVS(s, fnvs.m_nvs, fnvs.m_tab);
   // TODO: add debug (which is really just a human-readable form of the numeric keys) and both
   //       hex and dec output of numerics. Debug format is NOT intended for being read back in!
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
std::istream & operator >> (std::istream &s, NamedValueSet &rnvs)
{
   ENamedValues eRet = NVSReadNVS(s, static_cast<NamedValueSet *>(&rnvs));
   if (ENamedValuesOK != eRet) {
      s.setstate(std::ios_base::failbit);
   }
   return s;
}

//=============================================================================
// Name:        NamedValueSetFromStdString
// Description: Convert a std::string length into an NVS
// Interface:   public
// Inputs:      std::string containing the serialized representation of the
//              NamedValueSet.
// Outputs:     nvs is a non-const reference to the returned NamedValueSet
//=============================================================================
void NamedValueSetFromStdString(const std::string &s, NamedValueSet &nvs)
{
   istringstream iss(s);      // put the string inside an istringstream
   iss >> nvs;                // use operator >> to get it out
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
void NamedValueSetFromCharString(void *pv, btWSSize len, NamedValueSet &nvs)
{
   std::string s(static_cast<char *>(pv), (size_t)len);   // initializing this way allows embedded nulls
   NamedValueSetFromStdString( s, nvs);
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
std::string StdStringFromNamedValueSet(const NamedValueSet &nvs)
{
   ostringstream oss;
   oss << nvs << '\0';  // add a final, ensuring, terminating null
   return oss.str();
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
void BufFromString(void *pBuf, std::string const &s)
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
void NVSWriteLevel(FILE *file, unsigned level) {
   while (level--)
      fputc ('\t', file);
}

//=============================================================================
// Name: NVSWriteUnsigned - write a single unsigned, at the level passed in, followed by a space
//=============================================================================
void NVSWriteUnsigned(FILE *file, unsigned u, unsigned level) {
   NVSWriteLevel(file, level);
   fprintf(file, "%u ", u);
}

//=============================================================================
// Name: NVSReadUnsigned - read a single unsigned integer, ignoring preceeding whitespace
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
#ifdef _MSC_VER
   #pragma warning( push)      // fscanf
   #pragma warning( disable : 4996)      // fscanf
#endif // _MSC_VER
int NVSReadUnsigned (FILE *file, btUnsignedInt *pu) {
   return fscanf( file, "%u", pu);   // eats preceeding whitespace
}

//=============================================================================
//=============================================================================
int NVSReadNumberKey(FILE *file, btNumberKey *pu) {
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
void NVSWriteString(FILE *file, btcString sz, unsigned level) {
   NVSWriteLevel( file, level);
   fprintf(file, "%u %s\n", (unsigned)strlen(sz), sz);
}

//=============================================================================
// Name: NVSReadString from an open file - must free the result if not NULL
//       Format is LEN STRING, e.g. 5 hello\n
//          optionally preceeded by whitespace
//       Input is open file
//       Returns pointer to malloc'd buffer if successful, NULL if not
//=============================================================================
btString NVSReadString(FILE *file) {
   btUnsignedInt u = 0;
   btWSSize      szlen;
   btWSSize      Total;
   btWSSize      ThisTime;

   if ( EOF == NVSReadUnsigned(file, &u) ) {
      return NULL;                  // Get length of string
   }

   if ( 0 == u ) {
      return NULL;
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
   btString psz = new btByte[szlen + 1];

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
void NVSWriteName(FILE *file, eNameTypes typeName, btStringKey pszName, unsigned level) {
   NVSWriteLevel(file, level);                      // tab in
   fprintf(file, "%u ", static_cast<unsigned>(typeName));
   NVSWriteString(file, pszName, 0);                // write the name
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void NVSWriteName(FILE *file, eNameTypes typeName, btNumberKey uName, unsigned level) {
   NVSWriteLevel(file, level);                       // tab in
   fprintf(file, "%u ", static_cast<unsigned>(typeName));
   fprintf(file, "%llu\n", uName);                   // Depends on underlying type of btNumberKey
//   fprintf (file, "%u\n", uName);                    // No way to get around it in here
}

#if DEPRECATED
//=============================================================================
// Name: NVSWriteName to an open file
//       name can only be integer or string, should be checked before calling
//=============================================================================
void NVSWriteName (FILE* file, eNameTypes typeName, void* pName, unsigned level) {
   NVSWriteLevel(file, level);
   fprintf (file, "%u ", static_cast<unsigned>(typeName));
   if (btStringKey_t == typeName)      // btString_t type name
      NVSWriteString (file, reinterpret_cast<btStringKey>(pName), 0);
   else                             // btUnsignedInt_t type name
      fprintf (file, "%u\n", *reinterpret_cast<unsigned*>(pName));
}
#endif

//=============================================================================
// Name: NVSWriteEndOfNVS
//       Write an EndOfNVS marker to an open file
//       Name is hardcoded and fake - will never actually be read or used, can
//          something already in the NVS without problem
//=============================================================================
void NVSWriteEndOfNVS(FILE *file, unsigned level) {
   NVSWriteName(file, btStringKey_t, "---- End of NVS ----", level);
   NVSWriteUnsigned(file, btEndOfNVS_t, level+1); // This is the real end marker
   fprintf(file, "\n");
}  // End of NVSWriteEndOfNVS


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
ENamedValues NVSWriteNVS(FILE                *file,
                         NamedValueSet const &nvsToWrite,
                         unsigned             level)
{
   btUnsignedInt uElements;                  // number of elements in the NVS
   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName;                      // to hold an integer name
   btStringKey   sName = NULL;               // to hold a string name
   eBasicTypes   typeData = btUnknownType_t; // type of data


   nvsToWrite.GetNumNames(&uElements);     // initialize the loop to the number of elements in the NVS

   //Write one at a time
   for ( irg = 0 ; irg < uElements ; irg++ ) {        // walk forward through NVS
      nvsToWrite.GetNameType(irg, &typeName);// Get name type
      switch ( typeName ) {
         case btStringKey_t:
            nvsToWrite.GetName(irg,&sName);     // Get the name
            NVSWriteName(file, typeName, sName, level); // Write the name
            nvsToWrite.Type(sName,&typeData);   // Get the data type
         break;
         case btNumberKey_t:                 // same here, for UINT names
            nvsToWrite.GetName(irg,&iName);
            NVSWriteName(file, typeName, iName, level);
            nvsToWrite.Type(iName,&typeData);
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
               nvsToWrite.Get(sName,&val);
            } else {
               nvsToWrite.Get(iName,&val);
            }
            NVSWriteUnsigned(file, typeData, level+1);
            fprintf(file, "%u\n", static_cast<unsigned>(val));
         } break;
         case btByte_t : {
            btByte val;
            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName,&val);
            } else {
               nvsToWrite.Get(iName,&val);
            }
            NVSWriteUnsigned(file, typeData, level+1);
//            fprintf( file, "%c\n", val);
            fwrite (&val, sizeof(btByte), 1, file);
            fprintf(file, "\n");
         } break;
         case bt32bitInt_t : {
            bt32bitInt val;
            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName,&val);
            } else {
               nvsToWrite.Get(iName,&val);
            }
            NVSWriteUnsigned(file, typeData, level+1);
            fprintf(file, "%d\n", val);
         } break;
         case btUnsigned32bitInt_t: {
            btUnsigned32bitInt val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            NVSWriteUnsigned( file, typeData, level+1);
            fprintf( file, "%u\n", val);
         }
         break;
         case bt64bitInt_t: {
            bt64bitInt val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            NVSWriteUnsigned( file, typeData, level+1);
            fprintf( file, "%lld\n", val);
         }
         break;
         case btUnsigned64bitInt_t: {
            btUnsigned64bitInt val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            NVSWriteUnsigned( file, typeData, level+1);
            fprintf( file, "%llu\n", val);
         }
         break;
         case btFloat_t: {
            btFloat val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            NVSWriteUnsigned( file, typeData, level+1);
            fprintf( file, "%g\n", val);
         }
         break;
         case btString_t: {
            btcString val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteString( file, val, 0);
         }
         break;
         case btNamedValueSet_t: {
            NamedValueSet const *pval;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&pval);
            else
               nvsToWrite.Get(iName,&pval);
            // NVS Name already written, write the NVS DataType
            NVSWriteUnsigned( file, typeData, level+1);
            fputc('\n',file);
            NVSWriteNVS(file, *pval, level+2);  // Write the NVS itself
            // Write the trailer - String Name, and DataType 'End of NVS'
            NVSWriteName(file, btStringKey_t, "---- End of embedded NVS ----", level);
            NVSWriteUnsigned( file, btEndOfNVS_t, level+1); // This is the real end marker
            fprintf( file, "\n");
         }
         break;
         case btByteArray_t: {
            btByte  *val;
            btWSSize Num = 0;
            btWSSize NumWritten = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while (Num) {
               NumWritten = fwrite( val, sizeof(btByte), Num, file);
               Num -= NumWritten;
            }
            fprintf( file, "\n");
         }
         break;
         case bt32bitIntArray_t: {
            bt32bitInt *val;
            btWSSize    Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               fprintf( file, "%d ", *val++);
            }
            fprintf( file, "\n");
         }
         break;
         case btUnsigned32bitIntArray_t: {
            btUnsigned32bitInt *val;
            btWSSize            Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               fprintf( file, "%u ", *val++);
            }
            fprintf( file, "\n");
         }
         break;
         case bt64bitIntArray_t: {
            bt64bitInt *val;
            btWSSize    Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               fprintf( file, "%lld ", *val++);
            }
            fprintf( file, "\n");
         }
         break;
         case btUnsigned64bitIntArray_t: {
            btUnsigned64bitInt *val;
            btWSSize            Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               fprintf( file, "%llu ", *val++);
            }
            fprintf( file, "\n");
         }
         break;
         case btObjectType_t:      // Pointer, could be 32 or 64 bit
         {
            btObjectType val;
            if (btStringKey_t == typeName)
               nvsToWrite.Get(sName,&val);
            else
               nvsToWrite.Get(iName,&val);
            btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(val);
            NVSWriteUnsigned( file, typeData, level+1);
            fprintf( file, "%llu\n", u64temp);
         }
         break;
         case btFloatArray_t: {
            btFloat *val;
            btWSSize Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               fprintf( file, "%g ", *val++);
            }
            fprintf( file, "\n");
         }
         break;
         case btStringArray_t: {
            btString *val;
            btWSSize  Num = 0;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            fputs("\n", file);
            while ( Num-- ) {
               NVSWriteString( file, *val++, level+2);
            }
         }
         break;
         case btObjectArray_t: {
            btObjectType *val;     // pointer to array of pointers
            btWSSize      Num = 0; // number of pointers
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName,&val);
               nvsToWrite.GetSize(sName,&Num);
            } else {
               nvsToWrite.Get(iName,&val);
               nvsToWrite.GetSize(iName,&Num);
            }
            NVSWriteUnsigned( file, typeData, level+1);
            NVSWriteUnsigned( file, Num, 0);
            while ( Num-- ) {
               btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(*val++);
               fprintf( file, "%llu ", u64temp);
            }
            fprintf( file, "\n");
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
ENamedValues NVSWriteOneNVSToFile(FILE                *file,
                                  NamedValueSet const &nvsToWrite,
                                  unsigned             level) {
   ENamedValues retval = NVSWriteNVS(file, nvsToWrite, level);
   NVSWriteEndOfNVS(file, level);
   return retval;
}  // End of NVSWriteOneNVSToFile

//=============================================================================
// Utility function of NVSReadNVS for handling error returns
//=============================================================================
ENamedValues NVSReadNVSErrorFile(btmStringKey sName, ENamedValues error) {
   if ( NULL != sName ) {
      delete[] sName;       // clean up before error return
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
//
//=============================================================================
#if defined( _MSC_VER )
# pragma warning( push)           // fscanf
# pragma warning( disable : 4996) // fscanf
#endif // _MSC_VER

ENamedValues NVSReadNVS(FILE *file, NamedValueSet *nvsToRead) {
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
      unsigned tempType;

      tempType = 0;
      if ( EOF == NVSReadUnsigned(file, &tempType) ) {
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

            sName = NVSReadString(file);
            if ( NULL == sName ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }

         } break;

         case btNumberKey_t : {         // integer name

            if ( EOF == NVSReadNumberKey(file, &iName) ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }

         } break;

         default : return ENamedValuesInternalError_InvalidNameFormat;
      }
      // At this point, know that that typeName is either btString_t or btUnsignedInt_t
      //    and corresponding name is in either sName or iName

      // Get type of the DATA - expect an unsigned here
      tempType = 0;
      if (EOF == NVSReadUnsigned(file, &tempType)) {
         return NVSReadNVSErrorFile( sName, ENamedValuesInternalError_UnexpectedEndOfFile);
      }
      if (tempType  > btUnknownType_t &&   // range check
          tempType != btEndOfNVS_t) {
         return NVSReadNVSErrorFile(sName, ENamedValuesBadType);
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
      nvsToRead->Add(sName, __x);     \
   } else {                           \
      nvsToRead->Add(iName, __x);     \
   }                                  \
}while(0)

#define NVSREADNVS_FSCANF(__type, __initializer, __fmt)                                      \
case __type##_t : {                                                                          \
   __type val = __initializer;                                                               \
   if ( EOF == fscanf(file, __fmt, &val) ) { /* what happens if number is too big to fit? */ \
      return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);      \
   }                                                                                         \
   NVS_ADD(val);                                                                             \
} break

#define NVS_ADD_ARRAY(__A, __len)        \
do                                       \
{                                        \
   if ( btStringKey_t == typeName ) {    \
      nvsToRead->Add(sName, __A, __len); \
   } else {                              \
      nvsToRead->Add(iName, __A, __len); \
   }                                     \
}while(0)

#define NVSREADNVS_FSCANF_ARRAY(__type, __fmt)                                             \
case __type##Array_t : {                                                                   \
   __type##Array      val, p;                                                              \
   btUnsigned32bitInt i,   Num = 0;                                                        \
   btUnsigned32bitInt ThisTime;                                                            \
   if ( EOF == NVSReadUnsigned(file, &Num) ) { /* read number of elements */               \
      return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);    \
   }                                                                                       \
   if ( ( 0 == Num ) || ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) ) {                          \
      break; /* end now on bad item count */                                               \
   }                                                                                       \
   if ( NULL == (val = new __type[Num]) ) {                                                \
      return NVSReadNVSErrorFile(sName, ENamedValuesOutOfMemory);                          \
   }                                                                                       \
   i = Num;                                                                                \
   p = val;                                                                                \
   while ( i-- ) {                             /* read the array */                        \
      ThisTime = fscanf(file, __fmt, p++);                                                 \
      if ( ( 1 != ThisTime ) || ferror(file) ) {                                           \
         delete[] val;                                                                     \
         return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile); \
      }                                                                                    \
   }                                                                                       \
   NVS_ADD_ARRAY(val, Num);                                                                \
   delete[] val;                                                                           \
} break

      switch ( typeData ) {
         case btBool_t : {                // Read Data
            btBool val   = false;
            int    uTemp = 0;

            if (EOF == fscanf(file, "%d", &uTemp)) {
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ((uTemp < 0) || (uTemp > 1)) {
               return NVSReadNVSErrorFile(sName, ENamedValuesBadType);
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
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 1 != fread(&val, sizeof(btByte), 1, file) ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         case bt32bitInt_t : {            // Read Data
            bt32bitInt val = 0;
            if (EOF == NVSReadUnsigned(file, reinterpret_cast<btUnsigned32bitInt*>(&val))) {
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         case btUnsigned32bitInt_t : {    // Read Data
            btUnsigned32bitInt val = 0;
            if ( EOF == NVSReadUnsigned(file, &val) ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
         } break;

         NVSREADNVS_FSCANF(bt64bitInt,         0,   "%lld");
         NVSREADNVS_FSCANF(btUnsigned64bitInt, 0,   "%llu");
         NVSREADNVS_FSCANF(btFloat,            0.0, "%g"  );

         case btString_t : {              // Read Data
            btString val = NVSReadString(file);
            if (NULL == val) {
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
            delete[] val;
         } break;

         case btNamedValueSet_t : {         // Read Data
            NamedValueSet val;
            ENamedValues  ret;

            ret = NVSReadNVS(file, &val);
            if ( ENamedValuesOK == ret ) {
               NVS_ADD(val);
            } else {
               return NVSReadNVSErrorFile(sName, ret);
            }
         } break;

         // Normal end of embedded NVS, not really an error, but need to free sName
         case btEndOfNVS_t  : return NVSReadNVSErrorFile(sName, ENamedValuesOK);

         case btByteArray_t : {       // Read Data
            btByteArray        val;
            btUnsigned32bitInt Num = 0;
            btUnsigned32bitInt Total;
            btUnsigned32bitInt ThisTime;

            if ( EOF == NVSReadUnsigned(file, &Num) ) {  // read number of elements
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                            // crazy value, but possible, end now
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesIndexOutOfRange);
            }

            if ( NULL == (val = new btByte[Num + 1]) ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesOutOfMemory);
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
                  return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
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
         NVSREADNVS_FSCANF_ARRAY(btFloat,            "%g"  );

         // Read Data
         case btObjectType_t : {             // Pointer, could be 32 or 64 bit
            btObjectType       val;
            btUnsigned64bitInt u64 = 0;

            if ( EOF == fscanf(file, "%llu", &u64) ) {   // what happens if number is too big to fit?
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            val = reinterpret_cast<btObjectType>(u64);

            NVS_ADD(val);
         } break;

         case btStringArray_t : {            // Read Data
            btStringArray      val;
            btUnsigned32bitInt i, Num = 0;

            if ( EOF == NVSReadUnsigned(file, &Num) ) {  // read number of elements
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesIndexOutOfRange);
            }
            if ( NULL == (val = new btString[Num]) ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesOutOfMemory);
            }

            for ( i = 0 ; i < Num ; ++i ) {                     // initialize, helps with final cleanup
               val[i] = NULL;
            }

            for ( i = 0 ; i < Num ; ++i ) {                     // read strings, break if run out of memory
               if ( NULL == (val[i] = NVSReadString(file)) ) {
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

            if ( EOF == NVSReadUnsigned(file, &Num) ) {  // read number of elements
               return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                            // crazy value, but possible, end now
               break;
            } else if ( Num > MAX_VALID_NVS_ARRAY_ENTRIES ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesIndexOutOfRange);
            }

            if ( NULL == (val = new btObjectType[Num]) ) {
               return NVSReadNVSErrorFile(sName, ENamedValuesOutOfMemory);
            }

            btUnsigned64bitInt u64;
            for ( i = 0 ; i < Num ; ++i ) {                     // read the array
               u64 = 0;
               ThisTime = fscanf(file, "%llu", &u64);
               if ( ( 1 != ThisTime ) || ferror(file) ) {
                  delete[] val;
                  return NVSReadNVSErrorFile(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
               }
               val[i] = reinterpret_cast<btObjectType>(u64);
            }

            NVS_ADD_ARRAY(val, Num);
            delete[] val;
         } break;

         case btUnknownType_t : return NVSReadNVSErrorFile(sName, ENamedValuesBadType);
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
void NVSWriteLevel (ostream& os, unsigned level) {
   while (level--) {
      os << "\t";
   }
}

//=============================================================================
// Name: NVSWriteUnsigned - write a single unsigned, at the level passed in, followed by a space
//=============================================================================
void NVSWriteUnsigned (ostream& os, unsigned u, unsigned level) {
   NVSWriteLevel(os, level);
   os << u << " ";
}

//=============================================================================
// Name: NVSReadUnsigned - read a single unsigned integer, ignoring preceeding whitespace
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
btBool NVSReadUnsigned (istream& is, btUnsignedInt* pu) {
   is >> *pu;
   return is.good();    // true implies good health on the stream
}

//=============================================================================
// Name: NVSReadNumberKey - read a number that is supposed to represent a number key
//       Returns EOF on error, or other values - just check for EOF
//=============================================================================
btBool NVSReadNumberKey (istream& is, btNumberKey* pu) {
   is >> *pu;
   return is.good();    // true implies good health on the stream
}

//=============================================================================
// Name: WriteString to an open file
//       Format is LEN STR\n, e.g. 10 HelloWorld\n
//       not expecting errors here
//       level preceeds string with an appropriate number of tabs
//=============================================================================
void NVSWriteString (ostream& os, btcString sz, unsigned level) {
   NVSWriteLevel( os, level);
   os << (unsigned)strlen(sz) << " " << sz << "\n";
}

//=============================================================================
// Name: NVSReadString from an open file - must free the result if not NULL
//       Format is LEN STRING, e.g. 5 hello\n
//          optionally preceeded by whitespace
//       Input is open file
//       Returns pointer to malloc'd buffer if successful, NULL if not
//=============================================================================
char* NVSReadString (istream& is) {
   unsigned szlen = 0;

   if ( !NVSReadUnsigned(is, &szlen) ) {
      return NULL;                  // Get length of string
   }
   if ( 0 == szlen ) {
      return NULL;
   }

   if ( szlen > 256 ) {
      clog << "WARNING: NVSReadString: input length " << szlen << "greater than 256" << endl;
   }

   btString psz = new btByte[szlen + 1]; // get an input buffer, exception if NULL
                                         // valid index is [0] to [szlen]
   if ( NULL != psz ) {                  // new worked, read the string
      is.ignore(1);                      // eat the intervening space
      // incorporate terminating \n
      is.read(psz, szlen+1);
      psz[szlen] = 0;                    // overwrite terminating \n
   } else {                              // should have thrown an exception
      cerr << "ERROR: NVSReadString: out of memory\n" << endl;
      // leave null psz for return
   }

   return psz;
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void NVSWriteName (ostream& os, eNameTypes typeName, btStringKey pszName, unsigned level) {
   NVSWriteLevel(os, level);                       // tab in
   os << static_cast<unsigned>(typeName) << " ";   // type of name (string)
   NVSWriteString(os, pszName, 0);                 // write the name
}

//=============================================================================
// Name: NVSWriteName to an open file
//=============================================================================
void NVSWriteName (ostream& os, eNameTypes typeName, btNumberKey uName, unsigned level) {
   NVSWriteLevel(os, level);                       // tab in
   os << static_cast<unsigned>(typeName) << " ";   // type of name (int)
   os << uName << "\n";                            // write the name
}

//=============================================================================
// Name: NVSWriteEndOfNVS
//       Write an EndOfNVS marker to an open file
//       Name is hardcoded and fake - will never actually be read or used, can
//          something already in the NVS without problem
//=============================================================================
void NVSWriteEndOfNVS (ostream& os, unsigned level) {
   NVSWriteName(os, btStringKey_t, "---- End of NVS ----", level);
   NVSWriteUnsigned( os, btEndOfNVS_t, level+1); // This is the real end marker
   os << "\n";
}  // End of NVSWriteEndOfNVS

//=============================================================================
// Name: NVSWriteSingleValue
//       Templatized code for writing a data type that has only one value
//=============================================================================
template <typename dataT>
void NVSWriteSingleValue (ostream&       os,
                          NamedValueSet  const& nvsToWrite,
                          unsigned       level,
                          eNameTypes     typeName,
                          eBasicTypes    typeData,
                          btNumberKey    iName,
                          btStringKey    sName)
{
   dataT val;
   if ( btStringKey_t == typeName ) {
      nvsToWrite.Get(sName, &val);
   } else {
      nvsToWrite.Get(iName, &val);
   }
   NVSWriteUnsigned(os, typeData, level+1);
   os << val << "\n";
}  // End of NVSWriteSingleValue

//=============================================================================
// Name: NVSWriteArrayValue
//       Templatized code for writing an array data type to a stream
//=============================================================================
template <typename dataT>
void NVSWriteArrayValue (ostream&       os,
                         NamedValueSet  const& nvsToWrite,
                         unsigned       level,
                         eNameTypes     typeName,
                         eBasicTypes    typeData,
                         btNumberKey    iName,
                         btStringKey    sName)
{
   dataT   *val;
   btWSSize Num = 0;

   if ( btStringKey_t == typeName ) {
      nvsToWrite.Get(sName, &val);
      nvsToWrite.GetSize(sName, &Num);
   } else {
      nvsToWrite.Get(iName, &val);
      nvsToWrite.GetSize(iName, &Num);
   }
   NVSWriteUnsigned(os, typeData, level+1);
   NVSWriteUnsigned(os, Num, 0);
   while ( Num-- ) {
      os << *val++ << " ";
   }
   os << "\n";
}  // End of NVSWriteArrayValue

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
ENamedValues NVSWriteNVS (ostream& os,
                          NamedValueSet const& nvsToWrite,
                          unsigned level)
{
   btUnsignedInt uElements = 0;              // number of elements in the NVS
   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName(0);                   // to hold an integer name
   btStringKey   sName(NULL);                // to hold a string name
   eBasicTypes   typeData = btUnknownType_t; // type of data

   nvsToWrite.GetNumNames(&uElements);          // initialize the loop to the number of elements in the NVS

   //Write one at a time
   for ( irg = 0 ; irg < uElements ; ++irg ) {        // walk forward through NVS
      nvsToWrite.GetNameType(irg, &typeName);         // Get name type
      switch ( typeName ) {
         case btStringKey_t : {
            nvsToWrite.GetName(irg, &sName);          // Get the name
            NVSWriteName(os, typeName, sName, level); // Write the name
            nvsToWrite.Type(sName, &typeData);        // Get the data type
         } break;

         case btNumberKey_t : {                       // same here, for UINT names
            nvsToWrite.GetName(irg, &iName);
            NVSWriteName(os, typeName, iName, level);
            nvsToWrite.Type(iName, &typeData);
         } break;

         default : {
            cerr << "ERROR: NVSWriteNVS: name type " << static_cast<unsigned>(typeName) << "unsupported. Aborting." << endl;
            return ENamedValuesInternalError_InvalidNameFormat;
         }
      }

      // write data
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // In this section, there is a matrix of typeName vs. typeData, with a different function
      //    having to be called for each. Clearly this should be templatized, or something, someday.

#define NVSWRITENVS_CASE(__t)       case __t##_t      : NVSWriteSingleValue< __t >(os, nvsToWrite, level, typeName, typeData, iName, sName); break
#define NVSWRITENVS_ARRAY_CASE(__t) case __t##Array_t : NVSWriteArrayValue<  __t >(os, nvsToWrite, level, typeName, typeData, iName, sName); break

      switch( typeData ) {

         NVSWRITENVS_CASE(btBool);
         NVSWRITENVS_CASE(btByte);
         NVSWRITENVS_CASE(bt32bitInt);
         NVSWRITENVS_CASE(btUnsigned32bitInt);
         NVSWRITENVS_CASE(bt64bitInt);
         NVSWRITENVS_CASE(btUnsigned64bitInt);
         NVSWRITENVS_CASE(btFloat);

         case btString_t : {
            btcString val = NULL;
            if (btStringKey_t == typeName) {
               nvsToWrite.Get(sName, &val);
            } else {
               nvsToWrite.Get(iName, &val);
            }
            NVSWriteUnsigned(os, typeData, level+1);
            NVSWriteString(os, val, 0);
         } break;

         case btNamedValueSet_t : {
            NamedValueSet const *pval;
            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName, &pval);
            } else {
               nvsToWrite.Get(iName, &pval);
            }
            // NVS Name already written, write the NVS DataType
            NVSWriteUnsigned(os, typeData, level+1);
            os << "\n";
            NVSWriteNVS(os, *pval, level+2);    // Write the NVS itself
            // Write the trailer - String Name, and DataType 'End of NVS'
            //NVSWriteEndOfNVS( os, level);     // Could use this instead, but the string is
                                                // slightly different, and the archive test files
                                                // would have to change. TODO: go ahead and do this.
            NVSWriteName(os, btStringKey_t, "---- End of embedded NVS ----", level);
            NVSWriteUnsigned(os, btEndOfNVS_t, level+1); // This is the real end marker
            os << "\n";
         } break;

         case btByteArray_t : {
//            NVSWriteArrayValue<btByte> (os, nvsToWrite, level, typeName, typeData, iName, sName);
            btByteArray val;
            btWSSize    Num = 0;
            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName, &val);
               nvsToWrite.GetSize(sName, &Num);
            } else {
               nvsToWrite.Get(iName, &val);
               nvsToWrite.GetSize(iName, &Num);
            }
            NVSWriteUnsigned(os, typeData, level+1);
            NVSWriteUnsigned(os, Num, 0);
            os.write(val, Num);
            os << "\n";
         } break;

         NVSWRITENVS_ARRAY_CASE(bt32bitInt);
         NVSWRITENVS_ARRAY_CASE(btUnsigned32bitInt);
         NVSWRITENVS_ARRAY_CASE(bt64bitInt);
         NVSWRITENVS_ARRAY_CASE(btUnsigned64bitInt);
         NVSWRITENVS_ARRAY_CASE(btFloat);

         case btObjectType_t : {      // Pointer, could be 32 or 64 bit
            btObjectType val = NULL;

            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName, &val);
            } else {
               nvsToWrite.Get(iName, &val);
            }

            btUnsigned64bitInt u64temp = reinterpret_cast<btUnsigned64bitInt>(val);
            NVSWriteUnsigned(os, typeData, level+1);
            os << u64temp << "\n";
         } break;

         case btStringArray_t : {
            btString *val = NULL;
            btWSSize  Num = 0;

            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName, &val);
               nvsToWrite.GetSize(sName, &Num);
            } else {
               nvsToWrite.Get(iName, &val);
               nvsToWrite.GetSize(iName, &Num);
            }
            NVSWriteUnsigned(os, typeData, level+1);
            NVSWriteUnsigned(os, Num, 0);
            os << "\n";
            while ( Num-- ) {
               NVSWriteString(os, *val++, level+2);
            }
         } break;

         case btObjectArray_t : {
            btObjectType *val = NULL; // pointer to array of pointers
            btWSSize      Num = 0;    // number of pointers

            if ( btStringKey_t == typeName ) {
               nvsToWrite.Get(sName, &val);
               nvsToWrite.GetSize(sName, &Num);
            } else {
               nvsToWrite.Get(iName, &val);
               nvsToWrite.GetSize(iName, &Num);
            }
            NVSWriteUnsigned(os, typeData, level+1);
            NVSWriteUnsigned(os, Num, 0);

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
ENamedValues NVSWriteOneNVSToFile(ostream             &os,
                                  NamedValueSet const &nvsToWrite,
                                  unsigned             level)
{
   ENamedValues retval = NVSWriteNVS(os, nvsToWrite, level);
   NVSWriteEndOfNVS(os, level);
   return retval;
}  // End of NVSWriteOneNVSToFile

//=============================================================================
// Utility function of NVSReadNVS for handling error returns
//=============================================================================
ENamedValues NVSReadNVSError(btmStringKey sName, ENamedValues error) {
   if ( sName ) {
      delete[] sName;     // clean up before error return
   }
   return error;
}

//=============================================================================
// Name: NVSReadSingleValue
//       Templatized code for writing a data type that has only one value
//=============================================================================
template <typename dataT>
ENamedValues NVSReadSingleValue (istream       &is,
                                 NamedValueSet *nvsToRead,
                                 eNameTypes     typeName,
                                 btNumberKey    iName,
                                 btmStringKey   sName)
{
   dataT val;
   is >> val;
   if ( !is.good() ) {
      return NVSReadNVSError(sName, ENamedValuesBadType);
   }
   if (btStringKey_t == typeName) {
      nvsToRead->Add(sName,val);
   } else {
      nvsToRead->Add(iName,val);
   }
   return ENamedValuesOK;
}  // End of NVSReadSingleValue

#if DEPRECATED
//=============================================================================
// Name: NVSReadArrayValue
//       Templatized code for writing an array data type to a stream
// PROBLEM: nvsToRead->Add() must vector to the correct type, but
//    generally vectors to void* (that is, btObjectType). That doesn't
//    work, obviously. The "static_cast<arrayT>(val))" is an unsuccessful
//    attempt to vector it correctly.
// TODO: templatize if possible
//=============================================================================
template <typename singleT, typename arrayT>
ENamedValues NVSReadArrayValue (istream&       is,
                                NamedValueSet* nvsToRead,
                                eNameTypes     typeName,
                                btNumberKey    iName,
                                btStringKey    sName)
{
   arrayT             val, p;
   btUnsigned32bitInt i, Num = 0;

   if ( !NVSReadUnsigned(is, &Num) ) {         // read number of elements
      return NVSReadNVSError((char *)sName, ENamedValuesInternalError_UnexpectedEndOfFile);
   }
   if ( 0 == Num ) {                           // crazy value, but possible, end now
      return ENamedValuesOK;
   }
   val = new singleT[Num];
   i = Num;
   p = val;
   while ( is && i-- ) {                       // read the array
      is >> *p++;
   }
   if (btStringKey_t == typeName) {
      nvsToRead->Add(sName, val);
   } else {
      nvsToRead->Add(iName, (val));
      //nvsToRead->Add(iName,static_cast<arrayT>(val));
   }
   delete[] val;
   return ENamedValuesOK;
}  // End of NVSReadArrayValue
#endif // DEPRECATED

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
ENamedValues NVSReadNVS(istream &is, NamedValueSet *nvsToRead)
{
   btUnsignedInt irg;                        // index for each element in the NVS
   eNameTypes    typeName;                   // to hold the type of the name
   btNumberKey   iName    = 0;               // to hold an integer name
   btmStringKey  sName    = NULL;            // to hold a string name, returned from NVSReadString
   eBasicTypes   typeData = btUnknownType_t; // type of data
   ENamedValues  eRetVal  = ENamedValuesOK;

   if ( NULL == nvsToRead ) {                // if return parameter NULL, abort
      return ENamedValuesInvalidReadToNull;
   }

   //Read one at a time, counting as we go
   for ( irg = 0 ; ; ++irg ) {         // walk forward through NVS
      // Read NVS Name
      //       Format is TYPE NAME
      //       Where TYPE must be either btString_t or btUnsignedInt_t
      //       Where NAME is either an INTEGER or a LEN STRING, depending upon the TYPE

      // Get type of the NAME
      unsigned tempType = 0;
      if ( !NVSReadUnsigned(is, &tempType) ) {
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
            sName = NVSReadString(is);
            if ( NULL == sName ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }
         } break;

         case btNumberKey_t : {            // integer name
            if ( !NVSReadNumberKey(is, &iName) ) {
               return ENamedValuesInternalError_InvalidNameFormat;
            }
         } break;

         default : return ENamedValuesInternalError_InvalidNameFormat;
      }
      // At this point, know that that typeName is either btString_t or btUnsignedInt_t
      //    and corresponding name is in either sName or iName

      // Get type of the DATA - expect an unsigned here
      if ( !NVSReadUnsigned(is, &tempType) ) {
         return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
      }
      if ( tempType >  btUnknownType_t &&   // range check
           tempType != btEndOfNVS_t ) {
         return NVSReadNVSError(sName, ENamedValuesBadType);
      }
      typeData = static_cast<eBasicTypes>(tempType); // safe, because of range check

      // READ DATA
      // Note: it is known at this point that typeName is either btString_t or btUnsignedInt_t
      //    so the following code makes use of that by only checking the one value.

      // If typeName is btString_t, sName must be freed. This is done at the end of the switch.
      //    Error returns or other aborts from the switch need to free sName.


#define NVSREADNVS_SINGLE(__t)                                                 \
case __t##_t : {                                                               \
   eRetVal = NVSReadSingleValue< __t >(is, nvsToRead, typeName, iName, sName); \
   if ( eRetVal != ENamedValuesOK ) {                                          \
      return eRetVal;                                                          \
   }                                                                           \
} break

#define NVSREADNVS_STREAM_ARRAY(__type)                                             \
case __type##Array_t : {                                                            \
   __type##Array      val, p;                                                       \
   btUnsigned32bitInt i,   Num = 0;                                                 \
   if ( !NVSReadUnsigned(is, &Num) ) { /* read number of elements */                \
      return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile); \
   }                                                                                \
   if ( 0 == Num ) {                   /* crazy value, but possible, end now */     \
      break;                                                                        \
   }                                                                                \
   val = new __type[Num];                                                           \
   i = Num;                                                                         \
   p = val;                                                                         \
   while ( is && i-- ) {               /* read the array */                         \
      is >> *p++;                                                                   \
   }                                                                                \
   NVS_ADD_ARRAY(val, Num);                                                         \
   delete[] val;                                                                    \
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
         NVSREADNVS_SINGLE(btFloat);

         case btString_t : {              // Read Data
            btString val = NVSReadString(is);
            if ( NULL == val ) {
               return NVSReadNVSError( sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            NVS_ADD(val);
            delete[] val;
         } break;

         case btNamedValueSet_t : {         // Read Data
            NamedValueSet  val;
            ENamedValues   ret;
            ret = NVSReadNVS(is, &val);
            if ( ENamedValuesOK == ret ) {
               NVS_ADD(val);
            } else {
               return NVSReadNVSError(sName, ret);
            }
         } break;

         // Normal end of embedded NVS, not really an error, but need to free sName
         case btEndOfNVS_t  : return NVSReadNVSError(sName, ENamedValuesOK);

         case btByteArray_t : {      // Read Data
            btByteArray        val;
            btUnsigned32bitInt Num = 0;

            if ( !NVSReadUnsigned(is, &Num) ) {          // read number of elements
               return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
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
         NVSREADNVS_STREAM_ARRAY(btFloat);

#if DEPRECATED
         case bt32bitIntArray_t:       // Read Data
         /* TODO: templatize if possible. See header of NVSReadArrayValue for issues */
            eRetVal = NVSReadArrayValue <bt32bitInt, bt32bitIntArray> (is, nvsToRead, typeName, iName, sName);
            if (eRetVal != ENamedValuesOK) {
               return eRetVal;
            }
#endif // DEPRECATED

         case btObjectType_t : {             // Pointer, could be 32 or 64 bit
            btObjectType       val = NULL;
            btUnsigned64bitInt u64 = 0;
            is >> u64;
            if ( !is.good() ) {
               return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            val = reinterpret_cast<btObjectType>(u64);
            NVS_ADD(val);
         } break;

         case btStringArray_t : {                       // Read Data
            btStringArray      val;
            btUnsigned32bitInt i, Num = 0;

            if ( !NVSReadUnsigned(is, &Num) ) {         // read number of elements
               return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
            }
            if ( 0 == Num ) {                           // crazy value, but legal?, end now
               break;
            }
            val = new btString[Num];

            for ( i = 0 ; i < Num ; ++i ) {             // initialize, helps with final cleanup
               val[i] = NULL;
            }
            for ( i = 0 ; i < Num ; ++i ) {             // read strings, break if run out of memory
               if ( NULL == (val[i] = NVSReadString(is)) ) {
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
            btUnsigned32bitInt i, Num = 0;

            if ( !NVSReadUnsigned(is, &Num) ) {   // read number of elements
               return NVSReadNVSError(sName, ENamedValuesInternalError_UnexpectedEndOfFile);
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

         case btUnknownType_t : return NVSReadNVSError(sName, ENamedValuesBadType);
         default              : break;
      } // switch typeData

      if ( NULL != sName ) {                       // Free read in name
         delete[] sName;
         sName = NULL;
      }

   }  // for loop

   return eRetVal;
}  // NVSReadNVS


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
ENamedValues NVSMerge(NamedValueSet &nvsOutput, const NamedValueSet &nvsInput)
{
   // Write nvsInput to a stringstream
   std::stringstream ss;
   ss << nvsInput;
   ss >> nvsOutput;
   return ENamedValuesOK;
}  // NVSMerge


END_NAMESPACE(AAL)


