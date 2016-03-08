// Copyright(c) 2006-2016, Intel Corporation
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
/// @file InstanceRecord.cpp
/// @brief Encapsulate Instance record processing for the ResourceManager.
/// @ingroup ResMgr
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 02/16/2009     HM       Initial version
/// 03/08/2009     HM       Added m_numAllocations initialization to constructor
/// 03/09/2011     HM       Modifications to allocation counts in InstRec to
///                            accomodate "unlimited" allocations of an AFU@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/rm/InstanceRecord.h"

BEGIN_NAMESPACE(AAL)


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////////                  ////////////////////////////////
/////////////////         I N S T R E C        //////////////////////////
///////////////////////                  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:        InstRec::InstRec( aalrms_configUpDateEvent)
// Description: Constructor with passed in ConfigUpdate
//=============================================================================
InstRec::InstRec(aalrms_configUpDateEvent *pConfigUpdate) :
   m_ConfigUpdate(*pConfigUpdate),                   // make a copy
   m_InstanceIndex(IntNameFromDeviceAddress( &pConfigUpdate->devattrs.devid.m_devaddr)),
   m_NumAllocations(pConfigUpdate->devattrs.numOwners),
   m_MaxAllocations(pConfigUpdate->devattrs.maxOwners),
   m_fUnlimitedAllocations(MAFU_CONFIGURE_UNLIMTEDSHARES == pConfigUpdate->devattrs.maxOwners)
{}

InstRec::InstRec() {/*empty*/}

//=============================================================================
// Name:        InstRec::~InstRec
// Description: Destructor, implementation
//=============================================================================
InstRec::~InstRec() {}

//=============================================================================
// Name:        InstRec::IsAvailable
// Description: Check whether this instance can have any more allocations
// Comment:     Implemented in header, good to be inline
//=============================================================================
btBool InstRec::IsAvailable() const
{
   return m_fUnlimitedAllocations || (m_NumAllocations < m_MaxAllocations);
}

//=============================================================================
// Name:        InstRec::IncrementAllocations
// Description: Called before sending the Instance away in a goal record
//=============================================================================
btBool InstRec::IncrementAllocations()
{
   if ( IsAvailable() ) {
      ++m_NumAllocations;
      return true;
   }

   AAL_ERR(LM_ResMgr, "InstRec::IncrementAllocations would have resulted in over allocation. Instance record:\n" << this);
   return false;
}  // InstRec::IncrementAllocations()

//=============================================================================
// Name:        InstRec::DecrementAllocations
// Description: Called in conjunction with releasing a resource
//=============================================================================
btBool InstRec::DecrementAllocations()
{
   if ( m_NumAllocations > 0 ) {
      --m_NumAllocations;
      return true;
   }

   AAL_ERR(LM_ResMgr, "InstRec::DecrementAllocations would have resulted in negative allocation. Instance record:\n" << this);
   return false;
}  // InstRec::DecrementAllocations()

//=============================================================================
// Name:        InstRec::ReplaceStruct
// Description: Replace the struct with a new one. If the device names do not
//                match then the operation fails. It is intended ONLY for
//                replacement by a structure with the same device name
// NOTE:        The number of allocations of a device can temporarily differ
//                between the kernel and the Resource Manager. For example,
//                the kernel does not think of a device as allocated until
//                ownership has changed from the RM, but the RM thinks that
//                the device is allocated as soon as the device request is
//                satisfied and shipped. If later on the device request is
//                aborted, the RM decrements its allocation whereas the kernel
//                never updated its allocation in the first place. However,
//                this is required functionality because if the RM does NOT
//                mark allocation as soon as device request is satisfied, then
//                it may allocate the same device twice due to a race condition.
//=============================================================================
btBool InstRec::ReplaceStruct(const aalrms_configUpDateEvent *pConfigUpdate)
{
   if ( IntNameFromDeviceAddress(&pConfigUpdate->devattrs.devid.m_devaddr) == m_InstanceIndex ) {
      m_ConfigUpdate = *pConfigUpdate;
      return true;
   }

   AAL_DEBUG(LM_ResMgr, "InstRec::ReplaceStruct called with InstanceIndex not identical.\n");
   return false;
}  // InstRec::ReplaceStruct()


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////                      //////////////////////////////
/////////////////     I n s t R e c M a p      //////////////////////////
/////////////////////                      //////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:        InstRecMap::InstRecMap
// Description: Default constructor
//=============================================================================
InstRecMap::InstRecMap() :
   m_Map()
{}

//=============================================================================
// Name:        InstRecMap::~InstRecMap
// Description: Default destructor
//=============================================================================
InstRecMap::~InstRecMap()
{
   m_Map.clear();
}

//=============================================================================
// Name:        InstRecMap::Add
// Description: Add an Instance Record to the map
//=============================================================================
btBool InstRecMap::Add(const InstRec &instRec)
{
   std::pair<InstRecMap_itr_t, btBool> ret =
      m_Map.insert( std::pair<btNumberKey, InstRec>(instRec.InstanceIndex(), instRec) );
   return ret.second;
}  // InstRecMap::Add

//=============================================================================
// Name:        InstRecMap::Has
// Description: Check if a particular key is in the Map
//=============================================================================
btBool InstRecMap::Has(btNumberKey key)
{
   InstRecMap_itr_t itr = m_Map.find(key);
   return itr != m_Map.end();
}  // InstRecMap::Has

//=============================================================================
// Name:        InstRecMap::Get
// Description: Check if a particular key is in the Map
//=============================================================================
btBool InstRecMap::Get(btNumberKey key, pInstRec_t *ppcInstRec)
{
   InstRecMap_itr_t itr = m_Map.find(key);

   if ( itr != m_Map.end() ) {
      *ppcInstRec = &((*itr).second);
      return true;
   }

   *ppcInstRec = NULL;
   return false;
}  // InstRecMap::Get

//=============================================================================
// Name:        InstRecMap::Delete
// Description: Delete the element in the map based on the key
//=============================================================================
void InstRecMap::Delete(btNumberKey key)
{
   InstRecMap_itr_t itr=m_Map.find(key);

   if ( itr != m_Map.end() ) {
      m_Map.erase(key);
   }
}  // InstRecMap::Delete

//=============================================================================
// Name:        std::ostream& operator << of instRec
// Description: writes a description of the object to the ostream
//=============================================================================
std::ostream & operator << (std::ostream &s, const InstRec &instRec)
{
   s << std::hex << std::uppercase << std::showbase <<
      "InstRec  at    "   << reinterpret_cast<const void *>(&instRec) <<
      "\nInstanceIndex: " << instRec.InstanceIndex() <<
      "\nNumAllocated:  " << instRec.NumAllocations() <<
      "\nMaxAllowed:    " << instRec.MaxAllocations() <<
      "\nUnlimited:     " << instRec.fUnlimitedAllocations() <<
      "\n"                << instRec.ConfigStruct();
   return s;
}

END_NAMESPACE(AAL)

std::ostream & operator << (std::ostream &s, const AAL::InstRec &x) { return AAL::operator <<(s, x); }

