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
//        FILE: aalbus-ipip.h
//     CREATED: 12/5/2012
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the definitions for AAL Physical Interface 
//           Protocol (PIP).
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           Accelerator Hardware Module bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 12/04/2012     JG       Seperated from aalbus-device.h for Windows 
//                         portability.
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALBUS_IPIP_H__
#define __AALSDK_KERNEL_AALBUS_IPIP_H__
#include <aalsdk/kernel/aalwsservice.h>
#include <aalsdk/kernel/AALTransactionID_s.h>

BEGIN_NAMESPACE(AAL)

struct aal_device;
struct aaldev_ownerSession;
//=============================================================================
// Name: aal_fops
// Description: AAL hooks for file operations used optionally by PIPS
//=============================================================================
struct aal_fops
{
   // mmap
   btInt (*mmap)(struct aaldev_ownerSession *, // Owner sesssion
                 struct aal_wsid *,            // WSID
                 btAny );                      // OS specific structure
};

//=============================================================================
// Name: aal_pipapi
// Description: PIP interface handler - Interface adaptor between Universal
//              Interface Driver and the PIP.
//=============================================================================

// Message/Transaction wrapper
struct aal_pipmessage
{
   btVirtAddr        m_message;        // Message body
   btVirtAddr        m_response;       // Response body
   btWSSize         *m_prespbufSize;   // Response buffer size
   stTransactionID_t m_tranID;         // Transaction ID to identify result
   btObjectType      m_context;        // Optional token
};

// Message handler interface definition
struct aal_pipmsghandler
{
   // Binds the UI driver session interface with PIP
   btInt (*bindSession)(struct aaldev_ownerSession *);

   // UnBinds the UI driver session
   btInt (*unBindSession)(struct aaldev_ownerSession *);

   // Send a UI message to the PIP
   btInt (*sendMessage)(struct aaldev_ownerSession *,   // Owner Session
                        struct aal_pipmessage);         // Message

};


//=============================================================================
// Name: aal_ipip
// Description: Physical Interface Protocol Interface
//=============================================================================
struct aal_ipip
{
   struct aal_pipmsghandler   m_messageHandler; // PIP's message handler

   // Methods for binding and unbinding PIP to generic aal_device
   //  these may be used to allow PIPs to perform device initialization
   //  if the PIP is implemented independent of the driver Object's probe
   //  method.
   btInt (*binddevice)(struct aal_ipip *, struct aal_device *);   // Binds the PIP to the device
   btInt (*unbinddevice)(struct aal_device *);                    // Unbinds the PIP

   // Used for driver interface forwarding
   struct aal_fops            m_fops;

   // Workspace manager
   struct aal_interface      *m_iwsmgr;  // Workspace manager interface container
   struct aal_wsservice      *m_wsmgr;   // Workspace manager interface

   // Channel activate and deactivate commands
   btInt (*activate_channel)(struct aal_device * );
   btInt (*deactivate_channel)(struct aal_device * );
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
};

// Convenience macros
#define aalpip_hasBind(p)     (NULL != (p)->binddevice)
#define aalpip_hasUnbind(p)   (NULL != (p)->unbinddevice)
#define aalpip_bindevice(p,d) ((p)->binddevice(p,d))
#define aalpip_unbindevice(p) ((p)->unbinddevice)

// fop macros
#define aalpip_hasmmap(p)     (NULL != (p)->m_fops.mmap)
#define aalpip_mmap(p)        ((p)->m_fops.mmap)

#define aalpip_iwsmgr(d)      ((d)->m_iwsmgr)
#define aalpip_wsmgrp(d)      ((d)->m_wsmgr)


//=============================================================================
// Name: aalpip_init
// Description:Initialize the PIP structure
//=============================================================================
static inline 
void
aalpip_init(struct aal_ipip *ppip)
{
   memset(ppip, 0, sizeof(struct aal_ipip));
}

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALBUS_IPIP_H__

