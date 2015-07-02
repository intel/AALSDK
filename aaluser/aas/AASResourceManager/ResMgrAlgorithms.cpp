// Copyright (c) 2006-2015, Intel Corporation
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
/// @file ResMgrAlgorithms.cpp
/// @brief Implement the more complex algorithms of the ResourceManager class.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/15/2008     HM       Initial version<0>
/// 11/16/2008     HM       Initial Backdoor versions
/// 11/18/2008     HM       Backdoor working now
/// 11/28/2008     HM       Fixed legal header
/// 12/04/2008     HM       Goal Record creation is now FIFO instead of LIFO
/// 12/17/2008     HM       Add Channel Number to BackDoor record
/// 01/04/2009     HM       Updated Copyright
/// 02/04/2009     HM       Updated backdoor<1> to include selection of
///                            channel number
/// 02/15/2009     HM       Added IncrementNumAllocated()
/// 02/20/2009     HM       Changed ComputeBackdoorGoalRecords to use Instance
///                            Records, and implemented Backdoor Algorithm 2
/// 02/23/2009     HM       Fixed Backdoor Algorithm 2 to not scan Instance
///                            Records if a SW AFU or Service is requested
/// 03/04/2009     HM       Fixed AAL_VERBOSE output bug that mixed ChannelNumber
///                            and DeviceNumber
/// 03/27/2009     JG       Added support for MGMT AFU allocation
/// 03/09/2011     HM       Modifications to allocation counts in InstRec to
///                            accomodate "unlimited" allocations of an AFU@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"               // btUnsigned32bitInt, etc., plus AALIDDefs.h
#include "aalsdk/AALLoggerExtern.h"        // theLogger, a reference to a singleton ILogger interface
#include "aalsdk/rm/CAASResourceManager.h" // Also brings in the skeleton, which brings in the database, the proxy, and <string>
                                           // and <aas/kernel/aalrm_server.h>, the definition of aalrm_ioctlreq
                                           // Also defines debug LogMask_t bitmasks for detailed debugging of Resource Manager
#include "aalsdk/kernel/KernelStructs.h"   // various operator<<
#include "aalsdk/utils/ResMgrUtilities.h"  // string, name, and GUID inter-conversion operators

#include "aalsdk/INTCDefs.h"               // for AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED


BEGIN_NAMESPACE(AAL)


/*
 * Backdoor 0:
 * Initial implementation of RequestDevice is to just return the most recent handle
 * seen in a configuration update
 */

/*
 * Backdoor 1:
 * A backdoor is being used if there is in the manifest passed to RequestDevice
 *    a field named AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED (from CAALObjectFactory.h)
 *    which will contain an NVS that is to be passed back to the caller.
 * The algorithm for the backdoor depends on whether or not there is an keyRegAFU_ID
 *    NVP inside the backdoor NVS.
 * If there is NOT a keyRegAFU_ID NVP then just return with keyRegHandle set to 0 and
 *    the handle in the response also set to 0.
 * If there IS a keyRegAFU_ID NVP, then do a minimal search to find a real AFU that
 *    matches it.
 *    Find all the Instance Records that contains that AFU_ID, and has a non-zero
 *       handle, and has NumOwners=0. (will get more sophisticated later)
 *    Additionally select on keyRegChannelNumber if it is included in the Goal
 *       record.
 *    This basic search will allow allocation of multiple AFUs of the same or
 *       different types, right away.
 *    Merge the keyRegHandle and keyRegChannelNumber (for Activate) into the
 *       manifest at the top level, and return them.
 *    Whatever value is in keyRegHandle is also returned in the response
 * Regardless of the mechanism, if nothing is found, there must be passed back
 *    at least the input manifest with a Handle of 0. This is currently required
 *    for HOST-ONLY AFU's to work properly.
 */

/*
 * Backdoor 2: THIS IS THE CURRENT IMPLEMENTATION
 *
 * SAME AS BACKDOOR 1, EXCEPT that if any of the following fields are present
 *    it is considered to be looking for a real device. This corresponds to
 *    Backdoor 1 with an AFU_ID specified.
 * The list of fields that are searched for are:
 *    AFU_ID
 *    AHM_ID
 *    BusType
 *    BusNumber
 *    DeviceNumber
 *    Channel Number
 * Also, Instead of checking for NumOwners=0, check that NumAllocations is
 *    less than MaxAllocations
 *
 * If none of these exist in the backdoor record, then the record is considered
 *    for a HOST-ONLY AFU (or a service)
 */



//=============================================================================
// Name:          CResMgr::IsBackdoorRecordGood
// Description:   Utility function of CResMgr::ComputeBackdoorGoalRecords
// Interface:     private
// Inputs:        Pointer to NamedValueSet found in database that matches
//                   search criteria. This is expected to be an instance record.
// Outputs:       none
// Returns:       true if the NVS passes certain further criteria that cannot
//                   be tested for directly in the search.
// Comments:      logic is specific to the backdoor algorithm
//=============================================================================
//btBool CResMgr::IsBackdoorRecordGood (const NamedValueSet* pInstRec)
//{
//   if (NULL == pInstRec) {
//      AAL_WARNING(LM_ResMgr, "CResMgr::IsBackdoorRecordGood: Database Logic Error, search succeeded but no NVS returned.\n");
//      return false;
//   }
//
//   // Check value of Handle
//   btObjectType Handle = NULL;
//   pInstRec->Get( keyRegHandle, &Handle);    // If Get fails, Handle will still be NULL, causing exit false
//   if (NULL == Handle) {
//      return false;                          // Don't want a null handle, else keep processing
//   }
//
//   // Check value of NumOwners
//   btUnsignedInt NumOwners = 1;              // If Get fails, NumOwners will still be 1, causing exit false
//   pInstRec->Get( keyRegNumOwners, &NumOwners);
//   if (0 != NumOwners) {
//      return false;                          // If already owned, don't want it, else keep processing
//   }
//
//   return true;
//}  // end of CResMgr::IsBackdoorRecordGood

//=============================================================================
// Name:          CResMgr::ComputeBackdoorGoalRecords
// Description:   Utility function for ComputeGoalRecords. Definitely known
//                   there is a backdoor record, that is, one named
//                   AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED
// Interface:     private
// Inputs:        Same as ComputeGoalRecords
// Outputs:       Same as ComputeGoalRecords
// Returns:       Same as ComputeGoalRecords
// NOTE:          Currently implementing Backdoor algorithm 2
//=============================================================================
btBool CResMgr::ComputeBackdoorGoalRecords (const NamedValueSet& nvsManifest, nvsList& listGoal)
{
   // Yes, there is a backdoor
   INamedValueSet const *pTemp = NULL;
   nvsManifest.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &pTemp);

   NamedValueSet nvsBackDoorRecord(*pTemp);              // Get a copy of it

   // Search instance record list for:
   //    If the instance is available
   //    Each field in the pattern that is known
   // This is not run-time expandable like a NVS pattern search would be, but
   //    instance records have well-known fields and so run-time expandability
   //    should not be required.
   // Determine all relevant fields up front. Could be in the loop but this
   //    should be a bit faster.
   // Variables are hard-coded because they are different data types, don't see
   //    a clean way of converting this to a data-driven (e.g. table-driven)
   //    design

   ////////////////////////////////////////////////////////////////////////////
   // Determine the relevant fields
   ////////////////////////////////////////////////////////////////////////////

   /*
    * Get AFU_ID
    */
   btcString          sAFU_ID;
   btBool             testAFU_ID(false);

   if (nvsBackDoorRecord.Has(keyRegAFU_ID)) {
      // Yes, BackDoor with AFU_ID, find all Instance records with that AFU_ID
//      AAL_INFO(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Using backdoor config record WITH AFU_ID\n");
      nvsBackDoorRecord.Get( keyRegAFU_ID, &sAFU_ID);
      testAFU_ID = true;
      AAL_DEBUG(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Desired AFU_ID is: " << sAFU_ID << std::endl);
   }

   /*
    * Get AHM_ID
    */
   btUnsigned64bitInt AHM_ID;
   btBool             testAHM_ID(false);

   if (nvsBackDoorRecord.Has( keyRegAHM_ID)) {
      nvsBackDoorRecord.Get( keyRegAHM_ID, &AHM_ID);
      testAHM_ID = true;
      AAL_DEBUG( LM_ResMgr,  "CResMgr::ComputeBackdoorGoalRecords: Desired AHM_ID is: " << AHM_ID << std::endl);
   }

   /*
    * Get BusType
    */
   bt32bitInt         BusType;
   btBool             testBusType(false);

   if (nvsBackDoorRecord.Has( keyRegBusType)) {
      nvsBackDoorRecord.Get( keyRegBusType, &BusType);
      testBusType = true;
      AAL_DEBUG( LM_ResMgr,  "CResMgr::ComputeBackdoorGoalRecords: Desired BusType is: " << BusType << std::endl);
   }

   /*
    * Get BusNumber
    */
   btUnsigned32bitInt BusNumber;
   btBool             testBusNumber(false);

   if (nvsBackDoorRecord.Has( keyRegBusNumber)) {
      nvsBackDoorRecord.Get( keyRegBusNumber, &BusNumber);
      testBusNumber = true;
      AAL_DEBUG( LM_ResMgr,  "CResMgr::ComputeBackdoorGoalRecords: Desired BusNumber is: " << BusNumber << std::endl);
   }

   /*
    * Get DeviceNumber
    */
   btUnsigned32bitInt DeviceNumber;
   btBool             testDeviceNumber(false);

   if (nvsBackDoorRecord.Has( keyRegDeviceNumber)) {
      nvsBackDoorRecord.Get( keyRegDeviceNumber, &DeviceNumber);
      testDeviceNumber = true;
      AAL_DEBUG( LM_ResMgr,  "CResMgr::ComputeBackdoorGoalRecords: Desired DeviceNumber is: " << DeviceNumber << std::endl);
   }

   /*
    * Get Channel Number
    */
   bt32bitInt         ChannelNumber;
   btBool             testChannelNumber(false);

   if (nvsBackDoorRecord.Has( keyRegChannelNumber)) {
      nvsBackDoorRecord.Get( keyRegChannelNumber, &ChannelNumber);
      testChannelNumber = true;
      AAL_DEBUG( LM_ResMgr,  "CResMgr::ComputeBackdoorGoalRecords: Desired Channel Number is: " << ChannelNumber << std::endl);
   }

   /*
    * Summary: All fields current selected for by client are:
    *    AFU_ID
    *    AHM_ID
    *    BusType
    *    BusNumber
    *    DeviceNumber
    *    Channel Number
    */

   /*
    * Only search if there are fields to search
    */
   if ( testAFU_ID || testAHM_ID || testBusType || testBusNumber || testDeviceNumber || testChannelNumber ) {
      ////////////////////////////////////////////////////////////////////////////
      // Loop over the instance records
      ////////////////////////////////////////////////////////////////////////////

      for (InstRecMap_citr_t itr = m_InstRecMap.m_Map.begin(); itr != m_InstRecMap.m_Map.end(); ++itr)
      {
         // Get the Instance Record in the map
         const InstRec &rInstRec = (*itr).second;
         AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered is:\n" << rInstRec);

         /////////////////////////////////////////////////////////////////////////
         // Select for baseline parameters that must be true
         /////////////////////////////////////////////////////////////////////////

         // Select based on type: must be AFU
         if (( aal_devtypeAFU != rInstRec.ConfigStruct().devattrs.devid.m_devicetype )
            && ( aal_devtypeMgmtAFU != rInstRec.ConfigStruct().devattrs.devid.m_devicetype )){  // TODO JG HACK to allow MAFU selection
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered is not an AFU or Management AFU.\n");
            continue;
         }

         if ( aal_devtypeMgmtAFU == rInstRec.ConfigStruct().devattrs.devid.m_devicetype ){  // TODO JG HACK to allow MAFU selection
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered is a Management AFU.\n");
         }

         // Select based on availability, number of Allocations must be less than the Max
//         if ( rInstRec.NumAllocations() >= rInstRec.MaxAllocations()) {
         if ( ! rInstRec.IsAvailable() ) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered NumAlloc " << rInstRec.NumAllocations() <<
                  " >= MaxAlloc " << rInstRec.MaxAllocations() << std::endl);
            continue;
         }

         /////////////////////////////////////////////////////////////////////////
         // Select based on fields passed in
         /////////////////////////////////////////////////////////////////////////

         // AFU_ID
         if ( ( testAFU_ID ) &&
              ( sAFU_ID != AFU_IDNameFromConfigStruct( rInstRec.ConfigStruct()))) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered AFU_ID is " <<
                  AFU_IDNameFromConfigStruct( rInstRec.ConfigStruct()) << " but desired AFU_ID is " << sAFU_ID << std::endl);
            continue;
         }

         // AHM_ID
         if ( ( testAHM_ID ) &&
              ( AHM_ID != rInstRec.ConfigStruct().devattrs.devid.m_ahmGUID)) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered AHM_ID is " <<
                  rInstRec.ConfigStruct().devattrs.devid.m_ahmGUID << " but desired AHM_ID is " << AHM_ID << std::endl);
            continue;
         }

         // BusType
         if ( ( testBusType ) &&
              ( BusType != rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_bustype)) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered BusType is " <<
                  rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_bustype << " but desired BusType is " << BusType << std::endl);
            continue;
         }

         // BusNumber
         if ( ( testBusNumber ) &&
              ( BusNumber != rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_busnum)) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered BusNumber is " <<
                  rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_busnum << " but desired BusNumber is " << BusNumber << std::endl);
            continue;
         }

         // DeviceNumber
         if ( ( testDeviceNumber ) &&
              ( DeviceNumber != rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_devicenum)) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered DeviceNumber is " <<
                  rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_devicenum << " but desired DeviceNumber is " << DeviceNumber << std::endl);
            continue;
         }

         // Channel Number
         if ( ( testChannelNumber ) &&
              ( ChannelNumber != rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_subdevnum)) {
            AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Instance Record being considered ChannelNumber is " <<
                  rInstRec.ConfigStruct().devattrs.devid.m_devaddr.m_subdevnum << " but desired ChannelNumber is " << ChannelNumber << std::endl);
            continue;
         }

         /////////////////////////////////////////////////////////////////////////
         // If here, then this is a valid record. Put it on the goal list.
         /////////////////////////////////////////////////////////////////////////

         // Create the Instance Record as an NVS
         NamedValueSet nvsInstRec;
         NVSFromConfigUpdate( rInstRec.ConfigStruct(), nvsInstRec);

         // Add it to the goal list
         AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: Found Instance Record:\n" << rInstRec.ConfigStruct());

         // Create a Goal Record in the return list
         nvsContainer ncNil;                                         // A default container to add to the list
         listGoal.m_nvsList.push_back(ncNil);                        // Create list item
         nvsList_itr_t nvsitr = listGoal.m_nvsList.end();            // Retrieve it in place, (*itr) is a nvsContainer
         --nvsitr;                                                   // back up to get it

         // Pre-load the goal record with the Instance Record
         (*nvsitr).m_nvs = nvsInstRec;

         // Add the Manifest in as well
         (*nvsitr).m_nvs.Merge(nvsManifest);

      }  // Instance Record Loop

   }  // if ( testAFU_ID || testAHM_ID || testBusType || testBusNumber || testDeviceNumber || testChannelNumber )

   // If at end no records were found, return the manifest with a null handle
   if (listGoal.m_nvsList.empty()) {
      AAL_INFO(LM_ResMgr, "CResMgr::ComputeBackdoorGoalRecords: no match found.\n");
//      AddNullHandleRecToList( nvsManifest, listGoal );
      return false;
   }

   return true;

}  // end of CResMgr::ComputeBackdoorGoalRecords


//=============================================================================
// Name:          CResMgr::ComputeGoalRecords
// Description:   Top level of fan out to computing Goal Records, the key
//                   resource manager functionality
// Interface:     private
// Inputs:        NamedValueSet nvsManifest of desired goals
// Outputs:       nvsList listGoal containing all of the discovered Goal Records
// Returns:       true for Success, false for failure, with debugging output
//                   having already been produced.
//=============================================================================
btBool CResMgr::ComputeGoalRecords(const NamedValueSet& nvsManifest, nvsList& listGoal)
{
   // decide if there is a backdoor
   if (nvsManifest.Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)) {
      return ComputeBackdoorGoalRecords( nvsManifest, listGoal);
   }
   else {
      // No backdoor, do regular algorithm
      AAL_ERR(LM_ResMgr, "CResMgr::ComputeGoalRecords: No backdoor found, regular algorithm not yet implemented, failed\n");
      return false;
   }
}  // CResMgr::ComputeGoalRecords

//=============================================================================
// Name:          CResMgr::AddNullHandleRecToList
// Description:   Utility function for ComputeGoalRecords
// Interface:     private
// Inputs:        nvsList listGoal containing all of the discovered Goal Records
// Outputs:       NamedValueSet nvsGoal containg the single Goal Record desired
// Returns:       true for Success, false for failure, with debugging output
//                   having already been produced.
// Comments:      Needs to turn into a service call or hook for an external
//                   Policy Manager
//=============================================================================
void CResMgr::AddNullHandleRecToList( const NamedValueSet& nvsManifest, nvsList& listGoal )
{
   nvsContainer ncNil;                                         // A default container to add to the list
   listGoal.m_nvsList.push_front(ncNil);                       // Create list item
   nvsList_itr_t itr = listGoal.m_nvsList.begin();             // Retrieve it in place, (*itr) is a nvsContainer
   (*itr).m_nvs = nvsManifest;                                 // Copy the Manifest into list at itr
   (*itr).m_nvs.Add( keyRegHandle, static_cast<btObjectType>(0));     // Add the handle, type must match btObjectType
}


//=============================================================================
// Name:          CResMgr::ComputeBackdoorPolicy
// Description:   Utility function for GetPolicyResults. Definitely known
//                   there is a backdoor record, that is, one named
//                   AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, and the list
//                   is not empty
// Interface:     private
// Inputs:        Same as GetPolicyResults
// Outputs:       Same as GetPolicyResults
// Returns:       Same as GetPolicyResults
//=============================================================================
btBool CResMgr::ComputeBackdoorPolicy(const nvsList& listGoal, NamedValueSet& nvsGoal)
{
#if 0
   /*
    * This test has been subsumed into ComputeBackdoorGoalRecords, no need for it now
    */
   nvsList_citr_t citr;                   // iterator for listGoal
   for( citr=listGoal.m_nvsList.begin(); citr!=listGoal.m_nvsList.end(); ++citr) {

      // Get the BackDoor record
      const NamedValueSet* pbd;
      (*citr).m_nvs.Get( AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &pbd);

      // Get the requested Channel Number, if there is one
      bt32bitInt DesiredChannelNum;
      if (ENamedValuesOK != pbd->Get( keyRegChannelNumber, &DesiredChannelNum)) continue;

      // Get the available Channel Number, if there is one
      bt32bitInt ChannelNum;
      if (ENamedValuesOK != (*citr).m_nvs.Get( keyRegChannelNumber, &ChannelNum)) continue;

      // Does the available channel number match the desired channel number?
      if (DesiredChannelNum == ChannelNum) { // found it!
         nvsGoal = (*citr).m_nvs;
         return true;
      }
   }

   AAL_VERBOSE(LM_ResMgr, "CResMgr::ComputeBackdoorPolicy: No BackDoor Records Matched, reverting to standard Policy.\n");
#endif

   return false;                          // something weird happened, no list
}  // end of CResMgr::ComputeBackdoorPolicy

//=============================================================================
// Name:          CResMgr::GetPolicyResults
// Description:   Test routine for Policy Manager
// Interface:     private
// Inputs:        nvsList listGoal containing all of the discovered Goal Records
// Outputs:       NamedValueSet nvsGoal containg the single Goal Record desired
// Returns:       true for Success, false for failure, with debugging output
//                   having already been produced.
// Comments:      Needs to turn into a service call or hook for an external
//                   Policy Manager
//=============================================================================
btBool CResMgr::GetPolicyResults(const nvsList& listGoal, NamedValueSet& nvsGoal)
{
   // First hack is just take the first one
   if (!listGoal.m_nvsList.empty()) {
      nvsList_citr_t citr = listGoal.m_nvsList.begin();       // Retrieve it in place, (*itr) is a nvsContainer

      // Is there a backdoor record?
      if ((*citr).m_nvs.Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)) {
         if (ComputeBackdoorPolicy( listGoal, nvsGoal)) return true;
      }

      // If not, or if backdoor policy failed, then current policy is to return the first one
      // Should not have to reset the itr, as it is const and the embedded routines are also const
      nvsGoal = (*citr).m_nvs;
      return true;
   }
   else {
      // Nothing there, nothing to do
      AAL_ERR(LM_ResMgr, "CResMgr::GetPolicyResults: no Goal Records passed in, failure.\n");
      return false;
   }
}  // CResMgr::GetPolicyResults

//=============================================================================
// Name:          CResMgr::IncrementNumAllocated
// Description:   When a Goal Record has been selected, the "keyRegNumAllocated"
//                   field of its associated Instance Record must be incremented
// Interface:     Public
// Inputs:        nvsGoalRecord that is about to be returned
// Outputs:       nvsGoalRecord will have its keyRegNumAllocated incremented,
//                   and it must then be updated in the database
// Returns:       false if failure for any reason
// Comments:
//=============================================================================
//#ifndef NEW_INST_RECS
//btBool CResMgr::IncrementNumAllocated ( NamedValueSet& nvsGoal)
//{
//   // Get the key from the record
//   CRegDB RegDB;
//   ENamedValues eNVRet;
//   eNVRet = nvsGoal.Get( keyRegRecordNum, &RegDB.m_PrimaryKey);
//
//   if (ENamedValuesOK != eNVRet) {
//      AAL_ERR(LM_ResMgr, "CResMgr::IncrementNumAllocated Get keyRegRecordNum failure code: " << eNVRet << std::endl);
//      return false;
//   }
//
//   // Get NumAllocated and increment it
//   int NumAllocated;
//   eNVRet = nvsGoal.Get( keyRegNumAllocated, &NumAllocated);
//   ++NumAllocated;
//
//   if (ENamedValuesOK != eNVRet) {
//      AAL_ERR(LM_ResMgr, "CResMgr::IncrementNumAllocated Get keyRegNumAllocated failure code: " << eNVRet << std::endl);
//      return false;
//   }
//
//   // Lock the record in the database
//   eReg RegRet;
//   RegRet = m_pRegDBSkeleton->Database()->GetByKey( RegDB);
//
//   if (eRegOK != RegRet) {       // Got it for modification
//      AAL_ERR(LM_ResMgr, "CResMgr::IncrementNumAllocated GetByKey() failure code: " << RegRet << std::endl);
//      return false;
//   }
//
//   // Store it in the Goal Record
//   nvsGoal.Delete( keyRegNumAllocated);
//   nvsGoal.Add( keyRegNumAllocated, NumAllocated);
//
//   // Store it in the Instance Record in the database, update, commit
//   NamedValueSet nvsInstanceRecord;
//   nvsInstanceRecord = *RegDB.NVS();   // make a copy
//   nvsInstanceRecord.Delete( keyRegNumAllocated);
//   nvsInstanceRecord.Add( keyRegNumAllocated, NumAllocated);
//
//   // Commit the new record to the database
//   RegRet = m_pRegDBSkeleton->Database()->Commit( nvsInstanceRecord, RegDB);
//
//   if (eRegOK != RegRet) {
//      AAL_ERR(LM_ResMgr, "CResMgr::IncrementNumAllocated Commit() failure code: " << RegRet <<
//            " NVS:\n" << nvsInstanceRecord);
//      return false;
//   }
//
//   return true;
//
//}  // CResMgr::IncrementNumAllocated
//#endif


END_NAMESPACE(AAL)


