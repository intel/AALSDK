//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
//        FILE: cci_pcie_driver_umapi_linux.h
//     CREATED: 10/20/2015
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains private definitions for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           User Mode Interface for the AAL CCI device driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/20/2015     JG       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_AAL_PCIE_DRIVER_UMAPI_LINUX_H__
#define __AALKERNEL_AAL_PCIE_DRIVER_UMAPI_LINUX_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/ccipdriver.h"

#define DEV_NAME          "aalui"

#ifndef DRV_VERSION
# define DRV_VERSION      "EXPERIMENTAL VERSION"
#endif

#define AALUI_DRV_MAJVERSION     (0x00000001)
#define AALUI_DRV_MINVERSION     (0x00000000)
#define AALUI_DRV_RELEASE        (0x00000000)

// TODO THESE NEED PROPER DEFINITION IN IDS
#define  AALUI_DRV_INTC          (0x0000000000002000)

//=============================================================================
// Name: um_APIdriver
// Description: CCI User Mode API Class
//=============================================================================
struct um_APIdriver {
   dev_t           m_devtype;

#if   defined( __AAL_LINUX__ )
   struct file_operations      m_fops;  // Interface
   struct cdev                 m_cdev;  // character device
   btInt                       m_major; // major number of device node
#elif defined( __AAL_WINDOWS__ )
   btInt (*ioctl)(btAny ,
                  btUnsigned32bitInt ,
                  btAny ,
                  btWSSize ,
                  btAny ,
                  btWSSize * );
   btUnsigned64bitInt
         (*poll)(kosal_poll_object,
                 btAny );
#endif

   struct device              *m_device;
   struct class               *m_class;

   // List of current sessions
   // this is the head, and is linked with ccidrv_session->m_sessions
   kosal_semaphore          m_qsem;
   kosal_list_head          m_sessq;

   // Private semaphore
   kosal_semaphore          m_sem;

   /* list of allocated wsids */
   kosal_semaphore          wsid_list_sem;
   kosal_list_head          wsid_list_head;
};

//=============================================================================
// Name: ccidrv_session
// Description: Session structure holds state and other context for a user
//              session with the device subsystem.
//=============================================================================
struct ccidrv_session {
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


#endif // __AALKERNEL_AAL_PCIE_DRIVER_UMAPI_LINUX_H__

