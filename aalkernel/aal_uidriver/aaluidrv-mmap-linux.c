//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2015, Intel Corporation.
//
//  This program  is  free software;  you  can redistribute it  and/or  modify
//  it  under  the  terms of  version 2 of  the GNU General Public License  as
//  published by the Free Software Foundation.
//
//  This  program  is distributed  in the  hope that it  will  be useful,  but
//  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
//  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
//  General Public License for more details.
//
//  The  full  GNU  General Public License is  included in  this  distribution
//  in the file called README.GPLV2-LICENSE.TXT.
//
//  Contact Information:
//  Henry Mitchel, henry.mitchel at intel.com
//  77 Reed Rd., Hudson, MA  01749
//
//                                BSD LICENSE
//
//  Copyright(c) 2008-2015, Intel Corporation.
//
//  Redistribution and  use  in source  and  binary  forms,  with  or  without
//  modification,  are   permitted  provided  that  the  following  conditions
//  are met:
//
//    * Redistributions  of  source  code  must  retain  the  above  copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in  binary form  must  reproduce  the  above copyright
//      notice,  this  list of  conditions  and  the  following disclaimer  in
//      the   documentation   and/or   other   materials   provided  with  the
//      distribution.
//    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
//      contributors  may  be  used  to  endorse  or promote  products derived
//      from this software without specific prior written permission.
//
//  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
//  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
//  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
//  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
//  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
//  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
//  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
//  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
//  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
//******************************************************************************
//****************************************************************************
//        FILE: aaluidrv-mmap-linux.c
//      AUTHOR: Joseph  Grecco  <joe.grecco@intel.com>
//              Alvin Chen <alvin.chen@intel.com>
//
// PURPOSE: memory mapping functions
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 12/16/2008     JG       Added Support for WSID object
//                            Major interface changes.
// 01/04/2009     HM       Updated Copyright
// 01/14/2009     JG       Cleanup and refactoring
//                         Fixed a bug where device was not validated
// 08/06/2009     AC       Handle if the wsid is '0'
// 12/09/2009     JG       Pulled out AFU PIP commands aalui_afucmd_e
//                            and moved it to fappip.h and defined them as FAP
//                            pip specific.
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
// 05/05/2011     HM       Removed now extraneous capability checking and filed
//                            bug report explaining long term fix
// 09/03/2013     JG       Added kOSAL support. Changed name from aaluidrv-mmap
//                            to aaluidrv-mmap-linux to reflect OS specific
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS UIDRV_DBG_MMAP

#include "aaluidrv-int.h"

/** @brief search for a given wsid in the provided uidrv_session
 * @param[in] uidrv_sess_p pointer to uidrv session to dig through
 * @param[in] wsid_p pointer to workspace ID to check
 * @return NULL if pointer is not found, wsid_p if it is
 *
 * both input pointers are assumed already to be non-NULL.
 *
 * should this functionality be part of the workspace manager?  why are wsids
 * even leaked out of the workspace manager?  shouldn't everything out here be
 * manipulated through completely opaque workspace IDs (long long int)?
 */
static struct aal_wsid *find_wsid(const struct uidrv_session *uidrv_sess_p,
   struct aal_wsid *check_wsid_p)
{
   const struct aaldev_owner *owner_p;
   const struct aaldev_ownerSession *ownersess_p;
   const struct aal_wsid *cur_wsid_p = NULL;


   /* start by checking if the passed wsid is even valid */
   if (0 != uidrv_sess_p->m_msgHandler.valwsid(check_wsid_p)) {
      return NULL;
   }

   /* if this session is not associated with a device, don't bother checking
    * ownership of the wsid, since there may not be any.  */
   if (kosal_list_is_empty(&uidrv_sess_p->m_devicelist)) {
      return check_wsid_p;
   }

   /* if this session is associated with a device, (IE m_devicelist is not
    * empty,) then any wsid we handle needs to be on one of our device's
    * ownership lists, otherwise we shouldn't be touching it.
    *
    * struct uidrv_session contains list head of
    *   struct aaldev_owner which contains
    *     struct aaldev_ownerSession which contains the list head of
    *       struct aal_wsid */
   DPRINTF( UIDRV_DBG_MMAP, "looking at list head %p for wsid %p\n",
      &(uidrv_sess_p->m_devicelist), check_wsid_p );
   kosal_list_for_each_entry(owner_p, &(uidrv_sess_p->m_devicelist), m_devicelist, struct aaldev_owner) {
      DPRINTF( UIDRV_DBG_MMAP, "examining owner_p %p\n", owner_p);
      ownersess_p = &(owner_p->m_sess);

      kosal_list_for_each_entry(cur_wsid_p, &(ownersess_p->m_wshead), m_list, struct aal_wsid) {
         if (cur_wsid_p == check_wsid_p) {
            DPRINTF( UIDRV_DBG_MMAP, "  wsid ID %lld at %p found\n",
               cur_wsid_p->m_id, check_wsid_p);
            return check_wsid_p;
         }
      }
   }

   DPRINTF( UIDRV_DBG_MMAP, "wsid %p NOT found on any owner lists\n",
      check_wsid_p);
#if 0
   return NULL;
#else
   return check_wsid_p;
#endif
}

//=============================================================================
// Name: aalui_mmap
// Description: mmap system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: The mmap system call parameter "offset" (aka vm_pgoff) has been
//           overloaded to mean Workspace ID (wsid).  Because the mmap() call
//           expects a page aligned offset AND the kernel page aligns the
//           vm_pgoff value, the wsid (an unsigned long long) is encoded into
//           a page aligned value.
//=============================================================================
int
uidrv_mmap  (struct file *file, struct vm_area_struct *vma)
{
   struct aaldev_ownerSession    *ownerSessp = NULL;
   struct aal_wsid *wsidp = NULL;
   struct uidrv_session * psess = NULL;
   struct aal_device *pdev = NULL;

   //////////////////////////////////////////////////////////////////////////////////
   if(vma->vm_pgoff == 0 ) {
      DPRINTF( UIDRV_DBG_MMAP, "Invalid WSID\n");
      goto failed;
   }

   /* session information is squirreled away in our private data */
   psess = (struct uidrv_session *) file->private_data;
   if (NULL == psess) {
      DPRINTF( UIDRV_DBG_MMAP, "Invalid session\n");
      goto failed;
   }

   /* Get the WSID Object from the offset */
   wsidp = pgoff_to_wsidobj(vma->vm_pgoff);

   /* check wsidp vs known list of wsids */
   wsidp = find_wsid(psess, wsidp);
   if (NULL == wsidp) {
      DPRINTF( UIDRV_DBG_MMAP, "WSID not found on list owned WSIDs\n");
      goto failed;
   }

   // pull the aal_device out of the workspace
   pdev = wsidp->m_device;
   if(unlikely(!aaldev_valid(pdev))){
      DPRINTF( UIDRV_DBG_MMAP, "Invalid WSID\n");
      goto failed;
   }

   // Get the device session
   ownerSessp = dev_OwnerSession(pdev,psess->m_pid);
   if(unlikely(ownerSessp == NULL)){
      DPRINTF( UIDRV_DBG_MMAP, "Not device owner\n");
      goto failed;
   }

   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS pgoff = %lx \n", vma->vm_pgoff);
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS device = %p tid = %d\n", pdev,psess->m_pid );
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS %p id = 0x%llx.\n", wsidp,wsidp->m_id);

   //==================================================
   // Check for permission and correctness of interface
   //==================================================
#if 0
   // Move this code to pip-specific mmap functions AND
   // add intelligence about type of memory being mapped
   // e.g. && workspaceid.type == CSR // then fail
   // that is, CSR mapping is disabled but allow buffer mapping
   // or -- if CSR mapping is allowed but in multi-process mode then they would
   // not be allowed.
   if(!aaldev_allowsDirectAPI(pdev)){
      DPRINTF( UIDRV_DBG_MMAP, "Direct PIP access not allowed on this device.\n");
      goto failed;
   }
#endif

   if(!aaldev_haspip(pdev)) {
      DPRINTF( UIDRV_DBG_MMAP, "Device has no PIP.\n");
      goto failed;

   }

   if(!aalpip_hasmmap( aaldev_pipp(pdev) ) ){
      DPRINTF( UIDRV_DBG_MMAP, "Device PIP does not support mmap.\n");
      goto failed;
   }

   //------------------------------------
   // Call the device's PIP::fop:mmap
   //------------------------------------

   // Call through the device PIP to the mmap() method
   if( aalpip_mmap( aaldev_pipp(pdev) )( ownerSessp,
                                         wsidp,
                                         vma ) < 0) {
       DPRINTF( UIDRV_DBG_MMAP, "Mmap WS 0x%llx Failed.\n", wsidp->m_id);
       goto failed;
   }
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS Success.\n");
   return 0;

failed:
   return -EINVAL;

}

