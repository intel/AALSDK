// Copyright (c) 2008-2014, Intel Corporation
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
/// @file CAALWorkSpaceUtilities.cpp
/// @brief Workspace Utilities, including implementation if short enough.
/// @ingroup AASUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Alvin Chen, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/12/2008     HM       Initial version started
/// 12/14/2008     HM       Allocate and Free done
/// 12/15/2008     HM       Added WorkSpaceMapper functionality for deciding
///                            if a buffer is in the map, used for SubmitDesc
/// 01/04/2009     HM       Updated Copyright
/// 07/01/2009     AC       Added NULL pointer to the map
/// 08/16/2009     HM       Added support for Zero-Length buffers by providing
///                            a Requested_Length as well as an actual length
///                         For Requested_Length==0, actual length is set to 1.
///                         Specifically, modified WorkspaceMapper.
/// 02/05/20010    JG       Added Workspace enumeration Variant of GetWksp@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"
#include "aalsdk/utils/AALWorkSpaceUtilities.h" // This class' definition
#include "aalsdk/kernel/KernelStructs.h"        // ostream<< of various kernel structures


BEGIN_NAMESPACE(AAL)


/* WorkSpaceMapper - restricted to mapping the virtual addresses of a single address
 *    space.
 * Due to that restriction, we expect (and the code depends on) no overlapping mappings.
 * Specifically, memory is allocated as address/length blocks that may (or may not)
 *    end up being contiguous, but they should never overlap.
 * This has many implications, so if the assumption is invalid then the code will be.
 * Specifically:
 *    A Map[] is sufficient to hold the pointers. There should never be two valid
 *       concurrent allocations that map to the same virtual address.
 *    Any given pointer can only be a member of exactly zero or one allocated
 *       block of memory
 */
WorkSpaceMapper::WorkSpaceMapper() :
   m_map()
{ 
	AddToMap(0, 0, 0, SLAVE_MODE, 0);
	m_wkspitr=m_map.begin();
}

WorkSpaceMapper::~WorkSpaceMapper() { m_map.clear(); }

btBool WorkSpaceMapper::AddToMap(btVirtAddr ptr,
                                 btWSSize   len,
                                 btWSID     wsid,
                                 TTASK_MODE task_mode,
                                 btPhysAddr phys_ptr)
{
//   std::pair<wkspMap_itr_t, btBool> ret =
//      m_map.insert(std::pair<btObjectType, WkSp>( ptr, WkSp( ptr, len, wsid, task_mode, phys_ptr )));
//   return ret.second;
   return AddToMap(ptr, len, len, wsid, task_mode, phys_ptr);
}

btBool WorkSpaceMapper::AddToMap(btVirtAddr ptr,
                                 btWSSize   len,
                                 btWSSize   Requested_len,
                                 btWSID     wsid,
                                 TTASK_MODE task_mode,
                                 btPhysAddr phys_ptr)
{
   std::pair<wkspMap_itr_t, btBool> ret =
      m_map.insert( std::pair<btVirtAddr, WkSp>(ptr, WkSp(ptr, len, Requested_len, wsid, task_mode, phys_ptr)) );
      m_wkspitr = m_map.begin();
   return ret.second;
}

void WorkSpaceMapper::RemoveFromMap(btVirtAddr ptr)
{
   if ( NULL != ptr ) {
      wkspMap_itr_t itr = m_map.find(ptr);
      if ( itr != m_map.end() ) {
         m_map.erase(itr);
         m_wkspitr = m_map.begin();
      }
   }
}

// This iteration variant of the method gets the next workspace based on an internal iterator
//   The iterator is reset to the beginning when the object is created, a WS is added or removed
//   from the map or the restart value is set to true.
WorkSpaceMapper::eWSM_Ret WorkSpaceMapper::GetWkSp(pcWkSp_t *ppwksp, btBool restart)
{
   eWSM_Ret eRet(NOT_FOUND);        // Assume not found
   *ppwksp = NULL;

   // Reset the iterator?
   if ( restart ) {
      m_wkspitr = m_map.begin();
   }

   if ( !m_map.empty() && (m_wkspitr != m_map.end()) ) {
      *ppwksp = &(*m_wkspitr).second;
      ++m_wkspitr;
      eRet = FOUND;
   }

   return eRet;
}


// This variant gets a workspace based on a virtual pointer and length
WorkSpaceMapper::eWSM_Ret WorkSpaceMapper::GetWkSp(btVirtAddr ptr,
                                                   pcWkSp_t  *ppwksp,
                                                   btWSSize   len)
{
   eWSM_Ret eRet(NOT_FOUND);        // Assume not found
   *ppwksp = NULL;

   // First find if the pointer is within any block at all.

   wkspMap_citr_t itr=m_map.find(ptr);
   if (itr != m_map.end()) {        // found it directly
      *ppwksp = &((*itr).second);
      eRet = FOUND_EXACT;
   } else {                         // try inclusion, find first value > ptr
      itr=m_map.upper_bound(ptr);
      if (itr != m_map.begin()) {   // must not be at the beginning of the map
         --itr;                     // back up one
         if ( ptr < ( (*itr).second.m_ptr + (*itr).second.m_len ) ) { // is it in this block?
            *ppwksp = &((*itr).second);   // already know it is > ptr
            eRet = FOUND_INCLUDED;
         } else {                   // not in this block
         }
      } else {                      // at the beginning, so this ptr is less than any
      }
   }

   // if the pointer is in a block (i.e., if the above query succeeded),
   //    and there is a buffer query (len!=0), then check
   //    to see if the passed in buffer is within the mapped buffer

   if (len && (eRet != NOT_FOUND)) {   //
      // Know that ptr >= (*ppwksp)->m_ptr because of above algorithm
      btWSSize OffsetIntoKnownBuffer = ptr - (*ppwksp)->m_ptr;

      // (OffsetIntoKnownBuffer + len) is the offset of the end of the requested buffer
      if ((OffsetIntoKnownBuffer + len) <= (*ppwksp)->m_len) {
         eRet = FOUND;
      }
      else {
         // start pointer is inside a block, but the length trundles off the end.  treat this as not finding the
         // block at all.
         *ppwksp = NULL;       // ppwksp was set in first algorithm, but this failed, so reset it
         eRet = NOT_FOUND;
      }
   }

   return eRet;
}  // end of GetWkSp

void WorkSpaceMapper::DumpWkSpMap(std::ostream &s) const
{
   s << "Workspace Database @ " << reinterpret_cast<const void*>(this);
   if ( m_map.empty() ) {
      s << " is Empty\n";
   } else {
      s << "\n";
      for (wkspMap_citr_t itr = m_map.begin(); itr != m_map.end(); ++itr) { s<<(*itr).second;}
   }
}

std::ostream & operator << (std::ostream &s, const WorkSpaceMapper &WSM)
{
   WSM.DumpWkSpMap(s);
   return s;
}

std::ostream & operator << (std::ostream &s, const WorkSpaceMapper::WkSp &wksp)
{
   s << "WorkSpace @ " << reinterpret_cast<const void*>(&wksp) << std::showbase <<
        "\n\tptr:            " << std::hex << (void *)wksp.m_ptr <<
        "\n\tphysptr:        " << std::hex << wksp.m_phys_ptr <<
        "\n\tlen:            " << std::hex << wksp.m_len << "\t" << std::dec << wksp.m_len <<
        "\n\tRequested_len:  " << std::hex << wksp.m_Requested_len << "\t" << std::dec << wksp.m_Requested_len <<
        "\n\twsid:           " << std::hex << wksp.m_wsid <<
        "\n\ttask_mode:      " << wksp.m_task_mode <<
        std::endl;

   return s;
}  // end of operator << for WorkSpaceMapper::WkSp

std::ostream & operator << (std::ostream &s, const WorkSpaceMapper::eWSM_Ret &e)
{
   switch (e) {
      case WorkSpaceMapper::NOT_FOUND:
         s << " NOT_FOUND ";
         break;
      case WorkSpaceMapper::FOUND_EXACT:
         s << " FOUND_EXACT ";
         break;
      case WorkSpaceMapper::FOUND_INCLUDED:
         s << " FOUND_INCLUDED ";
         break;
      case WorkSpaceMapper::FOUND:
         s << " FOUND ";
         break;
   }
   return s;
}  // end of operator << for WorkSpaceMapper::WkSp


END_NAMESPACE(AAL)


