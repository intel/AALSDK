// Copyright(c) 2011-2016, Intel Corporation
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
/// @file AALNVSMarshaller.h
/// @brief This file contains the implementation of the NamedValueSet of the
///        AAL Marshaller/Unmarshallers used for RPC
/// @ingroup BasicTypes
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/07/2011      JG      Removed from AALService.h@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALNVSMARSHALLER_H__
#define __AALSDK_AALNVSMARSHALLER_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/AALLoggerExtern.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: NVSMarshaller
// Description: Marshaller/unmarshaller
//=============================================================================
class NVSMarshaller : public IAALMarshaller
{
public:
   NVSMarshaller() :
      m_NamedValueSet(),
      m_tempbuf(NULL)
   {}

   ENamedValues   Empty() { return m_NamedValueSet.Empty(); }
   btBool           Has(btcString Name)                              const { return m_NamedValueSet.Has(Name);             }
   ENamedValues  Delete(btStringKey Name)                                  { return m_NamedValueSet.Delete(Name);          }
   ENamedValues GetSize(btStringKey Name, btWSSize *pSize)           const { return m_NamedValueSet.GetSize(Name, pSize);  }
   ENamedValues    Type(btStringKey Name, eBasicTypes *pType)        const { return m_NamedValueSet.Type(Name, pType);     }
   ENamedValues GetName(btUnsignedInt index, btStringKey *pName)     const { return m_NamedValueSet.GetName(index, pName); }

   ENamedValues Add(btNumberKey Name, btBool value)                { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btByte value)                { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, bt32bitInt value)            { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btUnsigned32bitInt value)    { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, bt64bitInt value)            { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btUnsigned64bitInt value)    { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btFloat value)               { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, btcString value)             { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btNumberKey Name, const INamedValueSet *value) { return m_NamedValueSet.Add(Name, value); }

   ENamedValues Add(btNumberKey Name, btByteArray value,             btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, bt32bitIntArray value,         btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btUnsigned32bitIntArray value, btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, bt64bitIntArray value,         btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btUnsigned64bitIntArray value, btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btObjectType value)                                            { return m_NamedValueSet.Add(Name, value);              }
   ENamedValues Add(btNumberKey Name, btFloatArray value,            btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btStringArray value,           btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btNumberKey Name, btObjectArray value,           btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }

   ENamedValues Add(btStringKey Name, btBool value)                { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btByte value)                { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, bt32bitInt value)            { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btUnsigned32bitInt value)    { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, bt64bitInt value)            { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btUnsigned64bitInt value)    { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btFloat value)               { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, btcString value)             { return m_NamedValueSet.Add(Name, value); }
   ENamedValues Add(btStringKey Name, const INamedValueSet *value) { return m_NamedValueSet.Add(Name, value); }

   ENamedValues Add(btStringKey Name, btByteArray value,             btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, bt32bitIntArray value,         btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btUnsigned32bitIntArray value, btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, bt64bitIntArray value,         btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btUnsigned64bitIntArray value, btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btObjectType value)                                            { return m_NamedValueSet.Add(Name, value);              }
   ENamedValues Add(btStringKey Name, btFloatArray value,            btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btStringArray value,           btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }
   ENamedValues Add(btStringKey Name, btObjectArray value,           btUnsigned32bitInt NumElements) { return m_NamedValueSet.Add(Name, value, NumElements); }

    // Extract a byte stream from marshaller
   btcString pmsgp(btWSSize *len)
   {
//      std::string s(StdStringFromNamedValueSet(m_NamedValueSet));

      std::ostringstream oss;
      oss << m_NamedValueSet << '\0';  // add a final, ensuring, terminating null
      std::string s = oss.str();
      if ( NULL != m_tempbuf ) {
         delete[] m_tempbuf;
      }
      *len = (btWSSize)s.length();
      m_tempbuf = new char[(size_t)*len];
      BufFromString(m_tempbuf, s);
      return m_tempbuf;
   }

protected:
   NamedValueSet m_NamedValueSet;
   btString      m_tempbuf;
};


//=============================================================================
// Name: NVSUnMarshaller
// Description: Marshaller/unmarshaller
//=============================================================================
class NVSUnMarshaller : public IAALUnMarshaller
{
public:
   NVSUnMarshaller() :
      m_NamedValueSet()
   {}

   ENamedValues   Empty()                                                 { return m_NamedValueSet.Empty();               }
   btBool           Has(btcString Name)                             const { return m_NamedValueSet.Has( Name);            }
   ENamedValues  Delete(btStringKey Name)                                 { return m_NamedValueSet.Delete(Name);          }
   ENamedValues GetSize(btStringKey Name, btWSSize *pSize)          const { return m_NamedValueSet.GetSize(Name,pSize);   }
   ENamedValues    Type(btStringKey Name,eBasicTypes *pType)        const { return m_NamedValueSet.Type( Name,pType);     }
   ENamedValues GetName(btUnsignedInt index, btStringKey *pName)    const { return m_NamedValueSet.GetName(index, pName); }

   ENamedValues Get(btNumberKey Name, btBool *pValue)                  const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByte *pValue)                  const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitInt *pValue)              const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitInt *pValue)      const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitInt *pValue)              const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitInt *pValue)      const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloat *pValue)                 const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btcString *pValue)               const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, INamedValueSet const**pValue)    const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btByteArray *pValue)             const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt32bitIntArray *pValue)         const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray *pValue) const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, bt64bitIntArray *pValue)         const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray *pValue) const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectType *pValue)            const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btFloatArray *pValue)            const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btStringArray *pValue)           const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btNumberKey Name, btObjectArray *pValue)           const { return m_NamedValueSet.Get(Name, pValue); }

   ENamedValues Get(btStringKey Name, btBool *pValue)                  const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByte *pValue)                  const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitInt *pValue)              const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitInt *pValue)      const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitInt *pValue)              const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitInt *pValue)      const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btFloat *pValue)                 const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btcString *pValue)               const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, INamedValueSet const**pValue)    const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btByteArray *pValue)             const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt32bitIntArray *pValue)         const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray *pValue) const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, bt64bitIntArray *pValue)         const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray *pValue) const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectType *pValue)            const { return m_NamedValueSet.Get(Name, pValue); }

   ENamedValues Get(btStringKey Name, btFloatArray *pValue)            const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btStringArray *pValue)           const { return m_NamedValueSet.Get(Name, pValue); }
   ENamedValues Get(btStringKey Name, btObjectArray *pValue)           const { return m_NamedValueSet.Get(Name, pValue); }

   // Import a byte stream to the marshaller
   void importmsg(char const * pmsg, btWSSize len)
   {
      m_NamedValueSet.Empty();
      m_NamedValueSet.FromStr(const_cast<char *>(pmsg), len);
   }

protected:
   NamedValueSet m_NamedValueSet;
};


END_NAMESPACE(AAL)

#endif // __AALSDK_AALNVSMARSHALLER_H__


