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
///          Tim Whisonant, Intel Corporation
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
///                            specific typedefs: btStringKey and btStringKey
/// 11/16/2008     HM       New utilities, for converting between strings and
///                            and NamedValueSets, and merging NVS's
/// 11/20/2008     HM       FormattedNVS output provides tabbing and hook point
///                            for human readable AAL_ID->string conversion
/// 01/04/2009     HM       Updated Copyright
/// 09/21/2011     JG       Added constructor from const std:string@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALNAMEDVALUESET_H__
#define __AALSDK_AALNAMEDVALUESET_H__
#include <aalsdk/INamedValueSet.h>

BEGIN_NAMESPACE(AAL)

#ifdef __cplusplus

/// @addtogroup BasicTypes
/// @{

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
AASLIB_API void BufFromString(void *pBuf, std::string const &s);

/// Wrapper class for NamedValueSet implementation.
class AASLIB_API NamedValueSet : public INamedValueSet
{
public:
   /// NamedValueSet Default Constructor.
   NamedValueSet() :
      m_namedvalues(AAL::NewNVS())
   {}
   /// NamedValueSet Destructor.
   virtual ~NamedValueSet()
   {
      AAL::DeleteNVS(m_namedvalues);
   }

   /// NamedValueSet Assignment.
   NamedValueSet & operator = (const NamedValueSet &rOther)
   {
      if ( &rOther != this ) { // Don't dup yourself
         //Copy the NamedValueSet
         Copy(*rOther.m_namedvalues);
      }
      return *this;
   }

   /// NamedValueSet Copy Constructor.
   NamedValueSet(const NamedValueSet &rOther) :
      m_namedvalues(rOther.Clone())
   {}

   /// NamedValueSet Construct from const INamedValueSet &.
   NamedValueSet(const INamedValueSet &rOther) :
      m_namedvalues(rOther.Clone())
   {}

   /// NamedValueSet Construct from std::string.
   NamedValueSet(const std::string &rstr) :
      m_namedvalues(AAL::NewNVS())
   {
      FromStr(rstr);
   }

   /// NamedValueSet Construct from btcString, btWSSize
   NamedValueSet(void *p, btWSSize sz) :
      m_namedvalues(AAL::NewNVS())
   {
      FromStr(p, sz);
   }

   /// NamedValueSet type conversion to std::string
   operator std::string () const { return ToStr(); }

   /// NamedValueSet equality.
   btBool      operator == (const INamedValueSet &rOther)              const { return *m_namedvalues == rOther;      }
   btBool            Subset(const INamedValueSet &rOther)              const { return m_namedvalues->Subset(rOther); }

   ENamedValues GetNameType(btUnsignedInt index, eNameTypes *pType)    const { return m_namedvalues->GetNameType(index, pType); }

   ENamedValues Add(btNumberKey Name, btBool                Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btByte                Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, bt32bitInt            Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btUnsigned32bitInt    Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, bt64bitInt            Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btUnsigned64bitInt    Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btFloat               Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btcString             Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, btObjectType          Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btNumberKey Name, const INamedValueSet *Value)           { return m_namedvalues->Add(Name, Value); }

   ENamedValues Add(btNumberKey             Name,
                    btByteArray             Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    bt32bitIntArray         Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned32bitIntArray Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    bt64bitIntArray         Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btUnsigned64bitIntArray Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btFloatArray            Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btStringArray           Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btNumberKey             Name,
                    btObjectArray           Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }

   ENamedValues Get(btNumberKey Name, btBool                  *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByte                  *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitInt              *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitInt      *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitInt              *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitInt      *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloat                 *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btcString               *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectType            *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, INamedValueSet const   **pValue) const { return m_namedvalues->Get(Name, pValue); }

   ENamedValues Get(btNumberKey Name, btByteArray             *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitIntArray         *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitIntArray         *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloatArray            *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btStringArray           *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectArray           *pValue) const { return m_namedvalues->Get(Name, pValue); }

   btBool               Has(btNumberKey   Name)                        const { return m_namedvalues->Has(Name);             }
   ENamedValues      Delete(btNumberKey   Name)                              { return m_namedvalues->Delete(Name);          }
   ENamedValues     GetSize(btNumberKey   Name,  btWSSize    *pSize)   const { return m_namedvalues->GetSize(Name, pSize);  }
   ENamedValues        Type(btNumberKey   Name,  eBasicTypes *pType)   const { return m_namedvalues->Type(Name, pType);     }
   ENamedValues     GetName(btUnsignedInt Index, btNumberKey *pName)   const { return m_namedvalues->GetName(Index, pName); }

   ENamedValues GetNumNames(btUnsignedInt *pNum)                       const { return m_namedvalues->GetNumNames(pNum);     }
   ENamedValues       Empty()                                                { return m_namedvalues->Empty();               }

   //String Key methods
   ENamedValues Add(btStringKey Name, btBool                Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btByte                Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, bt32bitInt            Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btUnsigned32bitInt    Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, bt64bitInt            Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btUnsigned64bitInt    Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btFloat               Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btcString             Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, btObjectType          Value)           { return m_namedvalues->Add(Name, Value); }
   ENamedValues Add(btStringKey Name, const INamedValueSet *Value)           { return m_namedvalues->Add(Name, Value); }

   ENamedValues Add(btStringKey             Name,
                    btByteArray             Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    bt32bitIntArray         Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned32bitIntArray Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    bt64bitIntArray         Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btUnsigned64bitIntArray Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btFloatArray            Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btStringArray           Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }
   ENamedValues Add(btStringKey             Name,
                    btObjectArray           Value,
                    btUnsigned32bitInt      NumElements)                     { return m_namedvalues->Add(Name, Value, NumElements); }

   ENamedValues Get(btStringKey Name, btBool                  *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByte                  *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitInt              *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitInt      *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitInt              *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitInt      *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloat                 *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btcString               *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectType            *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, INamedValueSet const   **pValue) const { return m_namedvalues->Get(Name, pValue); }

   ENamedValues Get(btStringKey Name, btByteArray             *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitIntArray         *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitIntArray         *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloatArray            *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btStringArray           *pValue) const { return m_namedvalues->Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectArray           *pValue) const { return m_namedvalues->Get(Name, pValue); }

   btBool               Has(btStringKey   Name)                        const { return m_namedvalues->Has(Name);             }
   ENamedValues      Delete(btStringKey   Name)                              { return m_namedvalues->Delete(Name);          }
   ENamedValues     GetSize(btStringKey   Name,  btWSSize    *pSize)   const { return m_namedvalues->GetSize(Name, pSize);  }
   ENamedValues        Type(btStringKey   Name,  eBasicTypes *pType)   const { return m_namedvalues->Type(Name, pType);     }
   ENamedValues     GetName(btUnsignedInt Index, btStringKey *pName)   const { return m_namedvalues->GetName(Index, pName); }

   virtual ENamedValues     Read(std::istream &is)                           { return m_namedvalues->Read(is);            }
   virtual ENamedValues    Write(std::ostream &os)                     const { return m_namedvalues->Write(os);           }
   virtual ENamedValues    Write(std::ostream &os, unsigned level)     const { return m_namedvalues->Write(os, level);    }
   virtual ENamedValues WriteOne(std::ostream &os, unsigned level)     const { return m_namedvalues->WriteOne(os, level); }

#ifdef NVSFileIO

   virtual ENamedValues     Read(FILE *f)                                    { return m_namedvalues->Read(f);             }
   virtual ENamedValues    Write(FILE *f)                              const { return m_namedvalues->Write(f);            }
   virtual ENamedValues    Write(FILE *f, unsigned level)              const { return m_namedvalues->Write(f, level);     }
   virtual ENamedValues WriteOne(FILE *f, unsigned level)              const { return m_namedvalues->WriteOne(f, level);  }

#endif // NVSFileIO

   virtual ENamedValues   Merge(const INamedValueSet &nvs)                   { return m_namedvalues->Merge(nvs);         }

   virtual ENamedValues FromStr(const std::string &s)                        { return m_namedvalues->FromStr(s);         }
   virtual ENamedValues FromStr(void *p, btWSSize sz)                        { return m_namedvalues->FromStr(p, sz);     }
   virtual std::string    ToStr() const                                      { return m_namedvalues->ToStr();            }

protected:
   virtual ENamedValues               Copy(const INamedValueSet &Other)      { return m_namedvalues->Copy(Other);        }
   virtual INamedValueSet *          Clone()                           const { return m_namedvalues->Clone();            }
   virtual INamedValueSet const * Concrete()                           const { return m_namedvalues->Concrete();         }

   INamedValueSet *m_namedvalues;
};


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
   const NamedValueSet *m_nvs;
   const btInt          m_tab;
   const btBool         m_fDebug;
   FormattedNVS(const NamedValueSet *nvs, btInt tab=0, btBool fDebug=false) :
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
//              FormattedNVS is defined in AALNamedValueSet.h
//=============================================================================
inline std::ostream & operator << (std::ostream &s, const FormattedNVS &fnvs)
{
   fnvs.m_nvs->Write(s, fnvs.m_tab);
   // TODO: add debug (which is really just a human-readable form of the numeric keys) and both
   //       hex and dec output of numerics. Debug format is NOT intended for being read back in!
   return s;
}

#endif //__cplusplus

/// @}

END_NAMESPACE(AAL)

#endif //__AALSDK_AALNAMEDVALUESET_H__

