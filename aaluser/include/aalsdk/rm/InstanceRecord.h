// Copyright(c) 2007-2016, Intel Corporation
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
/// Accelerator Abstraction Layer
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


class InstRec
{
private:
   aalrms_configUpDateEvent m_ConfigUpdate;
   btNumberKey              m_InstanceIndex; // Bus type, bus number, device number, sub-device number
   unsigned int             m_NumAllocations;
   unsigned int             m_MaxAllocations;
   btBool                   m_fUnlimitedAllocations;   // True if the number of allocations is unlimited
public:
   /// @brief Constructor using event passed from RMS.
   /// @param[in] pConfigUpdate A pointer to an event detailing the
   ///                          characteristics of a device that has been added
   ///                          to the RMS.
   /// @return void
   InstRec(aalrms_configUpDateEvent* pConfigUpdate);
   // Default copy constructor and assignment are okay
   virtual ~InstRec();
   // Worker routines
   /// @brief Check if an Instance of an object is available - the number of
   ///        allocations is fewer than the maximum or the maximum is
   ///        unlimited.
   /// @retval True if there is room for another reference on the instance.
   /// @retval False if there is NOT room for another reference on the instance.
   btBool                            IsAvailable() const;
   /// @brief Increment the allocation count if possible.
   /// @retval True if the allocation count was incremented.
   /// @retval False if the allocation count could not be incremented.
   btBool                   IncrementAllocations();
   /// @brief Decrement the allocation count if possible.
   /// @retval True if the allocation count was decremented.
   /// @retval False if the allocation count could not be decremented.
   btBool                   DecrementAllocations();
   // Accessors
   /// @brief Access the ConfigUpdate event in the Instance Record
   /// @return A reference to the event.
   const aalrms_configUpDateEvent & ConfigStruct() const { return m_ConfigUpdate;          }
   /// @brief Access the InstanceIndex in the Instance Record
   /// @return The Instance Index - bus type, bus number, device number, and sub-device number.
   btNumberKey                     InstanceIndex() const { return m_InstanceIndex;         }
   /// @brief Access the number of allocations from the Instance Record
   /// @return The number of times this instance record has been allocated.
   unsigned int                   NumAllocations() const { return m_NumAllocations;        }
   /// @brief Access the maxium number of allocations from the Instance Record
   /// @return The maximum number of times this instance record can been allocated.
   unsigned int                   MaxAllocations() const { return m_MaxAllocations;        }
   /// @brief Access the unlimited allocations flag from the Instance Record
   /// @retval True if unlimited allocations are allowed.
   /// @retval False if unlimited allocations are NOT allowed.
   btBool                  fUnlimitedAllocations() const { return m_fUnlimitedAllocations; }
   // Mutators
   /// @brief Replace the ConfigUpdate event in the Instance Record
   /// @retval True if ConfigUpdate was replaced.
   /// @retval False if ConfigUpdate was NOT replaced.
   btBool                          ReplaceStruct(const aalrms_configUpDateEvent *pConfigUpdate);
   /// @brief Set the Instance Index (the device identifier) in the Instance Record
   /// @return The new Instance Index.
   btNumberKey                     InstanceIndex(btNumberKey i) { m_InstanceIndex  = i; return i; }
   /// @brief Set the maximum number of allocations in the Instance Record
   /// @return The new maximum number of allocations.
   unsigned int                   MaxAllocations(int i)         { m_MaxAllocations = i; return i; }
private:
   InstRec(); // Disallow default constructor
}; // InstRec

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
   /// @brief Add an Instance Record to the Map.
   /// @retval True if the record was added.
   /// @param[in] instRec A pointer to the Instance Record to add to the Map.
   /// @retval False if the record was NOT added.
   btBool  Add(const InstRec &instRec);
   /// @brief Check if a particular key is in the Map.
   /// @retval True if the key is in the Map.
   /// @retval False if the key is NOT in the Map.
   btBool  Has(btNumberKey );
   /// @brief Get an Instance Record from the Map, by key.
   /// @param[in] key The IndexInstance to search for.
   /// @param[in, out] ppcInstRec A pointer to a pointer that will receive the
   ///                            pointer to the Instance Record.
   /// @retval True if the Instance Record is found.
   /// @retval False if the Instance Record is NOT found.
   btBool  Get(btNumberKey key, pInstRec_t *ppcInstRec);
   /// @brief Delete an Instance Record from the Map, by key.
   /// <B>Parameters:</B> [in]  The IndexInstance of the Instance Record to delete.
   /// @return void
   void Delete(btNumberKey );
};

/// @brief InstanceRecord serializer.
/// <B>Parameters:</B> [in]  A reference to the stream.
/// @param[in] InstRec  A reference to the InstanceRecord to stream.
/// @return void
std::ostream & operator << (std::ostream & , const InstRec & );

END_NAMESPACE(AAL)

/// @brief InstanceRecord serializer.
/// <B>Parameters:</B> [in]  A reference to the stream.
/// @param[in] InstRec  A reference to the InstanceRecord to stream.
/// @return void
std::ostream & operator << (std::ostream & , const AAL::InstRec & );

#endif // __AALSDK_RM_INSTANCERECORD_H__


