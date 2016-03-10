// Copyright(c) 2008-2016, Intel Corporation
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
/// @file AALWorkSpaceUtilities.h
/// @brief Workspace Utilities
/// @ingroup AASUtils
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/12/2008     HM       Initial version started
/// 12/14/2008     HM       Allocate and Free done
/// 12/15/2008     HM       Added WorkSpaceMapper functionality for deciding
///                            if a buffer is in the map, used for SubmitDesc
/// 12/29/2008     HM       Add members to struct WkSp to track physical ptr
///                            and task mode
/// 01/04/2009     HM       Updated Copyright
/// 08/16/2009     HM       Added support for Zero-Length buffers by providing
///                            a Requested_Length as well as an actual length
///                         For Requested_Length==0, actual length is set to 1.
/// 02/05/20010    JG       Added Workspace enumeration Variant of GetWksp
/// 10/21/2011     JG       Added code for Windows compatibility@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_AALWORKSPACEUTILITIES_H__
#define __AALSDK_UTILS_AALWORKSPACEUTILITIES_H__
#include <aalsdk/AALTypes.h>            // btUnsigned, etc.
#include <aalsdk/kernel/AALWorkspace.h> // TTASK_MODE


/// @todo Document WorkSpaceMapper


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
    *       block of memory.
    *    In addition, any given pointer+length is either completely contained within
    *       one block, or it is not.
    */
   class AASLIB_API WorkSpaceMapper
   {
   public:
      // The thing that is mapped, default dtor, copy ctor, assignment should be okay
      struct WkSp {
         btVirtAddr m_ptr;           // User virtual pointer
         btWSSize   m_len;           // Length of buffer
         btWSSize   m_Requested_len; // Length of buffer originally requested. Usually == len, unless Requested_Len=0, then len==1
         btWSID     m_wsid;          // WorkSpace ID containing the buffer
         TTASK_MODE m_task_mode;     // Task type to use the buffer
         btPhysAddr m_phys_ptr;      // Physical pointer

         WkSp(btVirtAddr ptr,
              btWSSize   len,
              btWSID     wsid,
              TTASK_MODE task_mode,
              btPhysAddr phys_ptr=0) :
            m_ptr(ptr),
            m_len(len),
            m_Requested_len(len),         // If not specified, then assume Requested_Len is same as real len, this is the typical case
            m_wsid(wsid),
            m_task_mode(task_mode),
            m_phys_ptr(phys_ptr) {}

         WkSp(btVirtAddr ptr,
              btWSSize   len,
              btWSSize   Requested_len,
              btWSID     wsid,
              TTASK_MODE task_mode,
              btPhysAddr phys_ptr=0) :
            m_ptr(ptr),
            m_len(len),
            m_Requested_len(Requested_len),
            m_wsid(wsid),
            m_task_mode(task_mode),
            m_phys_ptr(phys_ptr) {}
      };
      typedef WkSp       *pWkSp_t;              // make one of these for return from GetWkSp()
      typedef WkSp const *pcWkSp_t;
      typedef WkSp       &rWkSp_t;

      // Return value, NOT_FOUND explicitly set to 0 to allow easy FOUND/NOT_FOUND checking
      enum eWSM_Ret {
         NOT_FOUND=0,         // The pointer is not in any mapped workspace (returned for any length passed in)
         FOUND_EXACT,         // The pointer matches the beginning of a workspace (length passed in is 0)
         FOUND_INCLUDED,      // The pointer is somewhere in a workspace (length passed in is 0)
         FOUND                // The pointer AND length passed in describe a buffer wholly within a workspace
                              //    FOUND is only returned if the length passed in is non-zero
         };

      typedef std::map<btObjectType, WkSp> wkspMap_t;
      typedef wkspMap_t::iterator          wkspMap_itr_t;
      typedef wkspMap_t::const_iterator    wkspMap_citr_t;

      WorkSpaceMapper();

      ~WorkSpaceMapper();

      btBool AddToMap(btVirtAddr ptr,
                      btWSSize   len,
                      btWSID     wsid,
                      TTASK_MODE task_mode,
                      btPhysAddr phys_ptr);

      btBool AddToMap(btVirtAddr ptr,
                      btWSSize   len,
                      btWSSize   Requested_len,
                      btWSID     wsid,
                      TTASK_MODE task_mode,
                      btPhysAddr phys_ptr);

      void RemoveFromMap(btVirtAddr ptr);

      /*
       * GetWkSP finds a workspace among the maps.
       * If Length is passed in as 0, (or defaults to 0), then only the Pointer is
       *    searched for.
       *    In this case, any of NOT_FOUND, FOUND_EXACT, or FOUND_INCLUDED can
       *       be returned.
       * If Length is non-zero, then the check is whether or not the buffer represented
       *    by the Pointer+Length is completely contained within a workspace.
       *    In this case, only either FOUND or NOT_FOUND are returned.
       */

      // Call it like this ...
      // WorkSpaceMapper::pcWkSp_t pWkSp;
      // WorkSpaceMapper::eWSM_Ret eRet = WSM.GetWkSp( Address, &pWkSp);
      eWSM_Ret GetWkSp(btVirtAddr ptr, pcWkSp_t *ppwksp, btWSSize len=0);

      // This iteration variant of the method gets the next workspace based on an internal iterator
      //   The iterator is reset to the beginning when the object is created, a WS is added or removed
      //   from the map or the restart value is set to true.
      eWSM_Ret GetWkSp(pcWkSp_t *ppwksp, btBool restart=false);

      void DumpWkSpMap(std::ostream &s) const;

   private:
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      wkspMap_t      m_map;
      wkspMap_citr_t m_wkspitr;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   }; // end of WorkSpaceMapper

   AASLIB_API std::ostream & operator << (std::ostream &s, const WorkSpaceMapper &WSM);
   AASLIB_API std::ostream & operator << (std::ostream &s, const WorkSpaceMapper::WkSp &wksp);
   AASLIB_API std::ostream & operator << (std::ostream &s, const WorkSpaceMapper::eWSM_Ret &e);

END_NAMESPACE(AAL)

#endif // __AALSDK_UTILS_AALWORKSPACEUTILITIES_H__

