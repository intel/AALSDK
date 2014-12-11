// Copyright (c) 2007-2014, Intel Corporation
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
/// @file InstanceRecord.h
/// @brief Define AALRegistrar-specific interface to a Database object.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 02/16/2009     HM       Initial version started
/// 02/17/2009     HM       Put in proper accessors/mutators and changes to
///                            member function signatures as functionality
///                            evolves
/// 03/09/2011     HM       Modifications to allocation counts in InstRec to
///                            accomodate "unlimited" allocations of an AFU@endverbatim
//****************************************************************************
#ifndef __AALSDK_RM_INSTANCERECORD_H__
#define __AALSDK_RM_INSTANCERECORD_H__
#include <aalsdk/kernel/KernelStructs.h>  // various operator<< and AALLogger
#include <aalsdk/utils/ResMgrUtilities.h> // string, name, and GUID inter-conversion operators
                                          //    also pulls in <aas/kernel/aaldevice.h>
#include <aalsdk/kernel/aalmafu.h>        // for MAFU_CONFIGURE_UNLIMTEDSHARES


/// @todo Document InstRec, InstRecMap, and related.

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)


class InstRec
{
private:
   aalrms_configUpDateEvent m_ConfigUpdate;
   btNumberKey              m_InstanceIndex; // Bus type, bus number, device number, sub-device number
   unsigned int             m_NumAllocations;
   unsigned int             m_MaxAllocations;
   btBool                   m_fUnlimitedAllocations;   // True if the number of allocations is unlimited
public:
   InstRec(aalrms_configUpDateEvent* pConfigUpdate);
   // Default copy constructor and assignment are okay
   virtual ~InstRec();
   // Worker routines
   btBool                            IsAvailable() const;
   btBool                   IncrementAllocations();
   btBool                   DecrementAllocations();
   // Accessors
   const aalrms_configUpDateEvent & ConfigStruct() const { return m_ConfigUpdate;          }
   btNumberKey                     InstanceIndex() const { return m_InstanceIndex;         }
   unsigned int                   NumAllocations() const { return m_NumAllocations;        }
   unsigned int                   MaxAllocations() const { return m_MaxAllocations;        }
   btBool                  fUnlimitedAllocations() const { return m_fUnlimitedAllocations; }
   // Mutators
   btBool                          ReplaceStruct(const aalrms_configUpDateEvent *pConfigUpdate);
   btNumberKey                     InstanceIndex(btNumberKey i) { m_InstanceIndex  = i; return i; }
   unsigned int                   MaxAllocations(int i)         { m_MaxAllocations = i; return i; }
private:
   InstRec(); // Disallow default constructor
}; // InstRec
 
std::ostream & operator << (std::ostream &s, const InstRec &instRec);

typedef InstRec                       *pInstRec_t;
typedef InstRec const                 *pcInstRec_t;
typedef InstRec                       &rInstRec_t;

typedef std::map<btNumberKey, InstRec> InstRecMap_t;
typedef InstRecMap_t::iterator         InstRecMap_itr_t;
typedef InstRecMap_t::const_iterator   InstRecMap_citr_t;

class InstRecMap
{
public:
   InstRecMap_t m_Map;
public:
         InstRecMap();
virtual ~InstRecMap();
   btBool  Add(const InstRec &instRec);
   btBool  Has(btNumberKey );
   btBool  Get(btNumberKey key, pInstRec_t *ppcInstRec);
   void Delete(btNumberKey );
};


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_RM_INSTANCERECORD_H__


