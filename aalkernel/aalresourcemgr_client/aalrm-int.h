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
//        FILE: aalrm-int.h
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file containe internal definitions for the
//          AAL Resource Manager Kernel Module.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/20/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
//****************************************************************************
#ifndef __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_INT_H__
#define __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_INT_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalrm_client.h"


#ifndef DRV_VERSION
# define DRV_VERSION      "EXPERIMENTAL VERSION"
#endif

#define DRV_DESCRIPTION   "Resource Manager Client Service Kernel Module"
#define DRV_AUTHOR        "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE       "GPL"
#define DRV_COPYRIGHT     "Copyright (c) 2008-2014 Intel Corporation"

extern btUnsignedInt debug;

#define AALRMC_DBG_ALL        AALRMC_DBG_MOD | AALRMC_DBG_FILE | AALRMC_DBG_MMAP | AALRMC_DBG_IOCTL
#define AALRMC_DBG_INVLID    ~(AALRMC_DBG_ALL)
#define AALRMC_DBG_DEFAULT    AALRMC_DBG_ALL


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      RESOURCE MANAGER CLIENT SERVICE     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
struct aalresmgr;


//=============================================================================
// Name: aalresmgr_session
// Description: Session structure holds state and other context for a user
//              session with the resource manager.
//=============================================================================
struct aalresmgr_session{
   // The resource manager client service
   struct aalresmgr     *m_resmgr;

   // Process ID of the process owning this session
   btPID                 m_tgpid;

   // List of devices owned by this session
   kosal_list_head       m_devlist;

   // Queue of sessions this session is on (RMC)
   kosal_list_head       m_sessq;

   // Queue of events to be sent to user space
   aal_queue_t           m_eventq;

   // Wait queue used for poll
   kosal_poll_object     m_waitq;

   // Private semaphore
   kosal_semaphore	    m_sem;

   // Counts transactions pending
   btUnsignedInt         m_transcnt;

};

//=============================================================================
// Name: aalresmgr
// Description: Resource manager class. This is the class definition for the
//              Resource Manager kernel service.
//              Holds resource manager client specific information.
//=============================================================================
struct aalresmgr{

   // Public Methods
   void (*register_sess)(kosal_list_head *psession);
   void (*unregister_sess)(kosal_list_head *psession);

   // Resource manager driver
   struct aal_driver       *m_driver;

   // Resource manager class device
   struct aal_classdevice  *m_class;

   // List of current sessions
   kosal_list_head         m_sessq;

    // RMS service interface
   struct aal_interface    *m_rmssrvs;

   // Private semaphore
   kosal_semaphore          m_sem;

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                  MACROS                  ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//RMC Driver object to AALBus
#define rmcdrv_to_aalbus(drv)       (*(drv)->m_bus)

//RMC Session to RMC
#define rmssess_to_rmcsrv(sess)     (*(sess)->m_resmgr)

//RMC Session to RMS
#define rmssess_to_rmssrv(sess)     ((sess)->m_resmgr->m_rmssrvs)


//--------
// Externs
//--------

// Singleton resource manager
extern struct aalresmgr resmgr;


extern btInt aalrm_open(struct inode *, struct file *);
extern btInt aalrm_close(struct inode *, struct file *);
#if HAVE_UNLOCKED_IOCTL
extern long aalrm_ioctl(struct file * , unsigned int , unsigned long );
#else
extern int aalrm_ioctl(struct inode * , struct file * ,
                       unsigned int , unsigned long );
#endif

extern btUnsignedInt aalrm_poll(struct file *, poll_table *);
extern btInt aalrm_flush_eventqueue(struct aalresmgr_session *);


#endif // __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_INT_H__

