//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2014, Intel Corporation.
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
//  Copyright(c) 2008-2014, Intel Corporation.
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
//        FILE: aaluidrv-int.h
//     CREATED: 08/26/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains private definitions for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           Universal Interface Driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 08/26/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 12/10/2008     JG       Added support for Close and cleanup
// 12/16/2008     JG       Began support for abort and shutdown
//                         Added Support for WSID object
//                         Major interface changes.
// 01/04/2009     HM       Updated Copyright
// 01/14/2009     JG       Cleanup and refactoring
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
// 02/26/2013     AG       Add wsid tracking and validation routines
//****************************************************************************
#ifndef __AALKERNEL_AAL_UIDRIVER_AALUIDRV_INT_H__
#define __AALKERNEL_AAL_UIDRIVER_AALUIDRV_INT_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalui.h"

#define DEV_NAME          "aalui"

#ifndef DRV_VERSION
# define DRV_VERSION      "EXPERIMENTAL VERSION"
#endif

#define AALUI_DRV_MAJVERSION     (0x00000001)
#define AALUI_DRV_MINVERSION     (0x00000000)
#define AALUI_DRV_RELEASE        (0x00000000)

// TODO THESE NEED PROPER DEFINITION IN IDS
#define  AALUI_DRV_INTC          (0x0000000000002000)

#define DRV_DESCRIPTION   "AAL Universal Device Driver Interface Module"
#define DRV_AUTHOR        "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE       "GPL"
#define DRV_COPYRIGHT     "Copyright (c) 2008-2014 Intel Corporation"


//=============================================================================
// Name: ui_driver
// Description: Universal Device Driver Interface Class
//=============================================================================
struct ui_driver {
   // AAL base class
   struct aal_driver        m_aaldriver;

   // UI driver class device
   struct aal_classdevice  *m_class;

   // List of current sessions
   // this is the head, and is linked with uidrv_session->m_sessions
   kosal_semaphore          m_qsem;
   kosal_list_head          m_sessq;

   // Private semaphore
   kosal_semaphore          m_sem;

   /* list of allocated wsids */
   kosal_semaphore          wsid_list_sem;
   kosal_list_head          wsid_list_head;
};

//=============================================================================
// Name: uidrv_session
// Description: Session structure holds state and other context for a user
//              session with the device subsystem.
//=============================================================================
struct uidrv_session {
   // PIP to UI driver interface
   struct aal_uiapi           m_msgHandler;

   // Owning driver module (UDDI in this case)
   struct ui_driver          *m_aaldriver;

   // Head of list of devices (struct aaldev_owner->m_devicelist) owned by this session
   kosal_list_head            m_devicelist;

   // Wait queue used for poll
   kosal_poll_object          m_waitq;

   // Private semaphore
   kosal_semaphore            m_sem;

   // Link to global UDDI session list.  head is ui_driver->m_sessq.
   kosal_list_head            m_sessions;

   // Event queue
   aal_queue_t                m_eventq;

   // Pid of process associated with this session
   btPID                      m_pid;

};


//----------
// Externals
//----------
extern btInt uidrv_session_destroy(struct uidrv_session * );
extern btInt uidrv_fasync (btInt fd, struct file *file, btInt on);
extern btInt uidrv_flush_eventqueue(  struct uidrv_session *psess);

extern btInt uidrv_open(struct inode *, struct file *);
extern btInt uidrv_close(struct inode *, struct file *);

extern btInt
uidrv_messageHandler(struct uidrv_session  *,
                     btUnsigned32bitInt     ,
                     struct aalui_ioctlreq *,
                     btWSSize               ,
                     struct aalui_ioctlreq *,
                     btWSSize              *);

extern btInt uidrv_mmap(struct file *, struct vm_area_struct *);
extern btUnsignedInt uidrv_poll(struct file *, poll_table *);
extern btInt uidrv_sendevent( void *,
                              struct aal_device *,
                              struct aal_q_item *,
                              void *);
extern  struct aal_wsid* uidrv_getwsid(struct aal_device *,
                                btUnsigned64bitInt );
extern btInt uidrv_freewsid(struct aal_wsid *);
extern btInt uidrv_valwsid(struct aal_wsid *);

// UDDI singleton driver module
extern struct ui_driver thisDriver;

#endif // __AALKERNEL_AAL_UIDRIVER_AALUIDRV_INT_H__

