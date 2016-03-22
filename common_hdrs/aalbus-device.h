//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2016, Intel Corporation.
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
//  Copyright(c) 2008-2016, Intel Corporation.
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
//        FILE: aalbus-device.h
//     CREATED: 09/15/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the definitions for AAL device generic behavior.
//           Accelerator Abstraction Layer (AAL)
//           Accelerator Hardware Module bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09/15/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 11/25/2008     HM       Large merge
// 12/10/2008     JG       Added support for bound wsmgr
// 12/16/2008     JG       Began support for abort and shutdown
//                            Added Support for WSID object
//                            Major interface changes.
// 12/18/2008     JG       Added owner Session initializer and
//                            copier.
// 01/04/2009     HM       Updated Copyright
// 02/26/2009     JG       Began dynamic config implementation
// 03/17/2009     JG       Added macro for pipContext
// 03/23/2009     JG       Removed bridge device
// 04/13/2009     JG       Added support for version kernel 2.6.27
// 05/11/2009     JG/HM    Added <linux/device.h> and other headers
// 05/20/2009     JG       Changed aaldev_pipp to use real pip interface
//                         instead of aal_interface
// 10/22/2009     JG       Added aaldev_AddOwner_e in support of modified
//                         device methods
// 02/11/2010     JG       Support for kernel 2.6.31
// 02/14/2012     JG       Removed some unused and deprecated macros.
// 12/04/2012     JG       Moved PIP definitions 
// 02/26/2013     AG       Add wsid tracking and validation routines
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALBUS_DEVICE_H__
#define __AALSDK_KERNEL_AALBUS_DEVICE_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/aalqueue.h>
#include <aalsdk/kernel/aalmafu.h>
#include <aalsdk/kernel/AALTransactionID_s.h>
#include <aalsdk/kernel/aalbus-ipip.h>


BEGIN_NAMESPACE(AAL)




//=============================================================================
// Name: aal_device_factory
// Description: AAL device factory interface. This interface is imeplemented
//              by objects that create and destroy aal_device objects
//=============================================================================
struct aal_device_factory {
#if 0
   struct aal_device * (*create) (struct aal_device_id *devID,
                                  btObjectType          manifest);
#endif
   btInt               (*destroy)(struct aal_device *dev);

   struct aal_device * (*create) (struct mafu_CreateAFU *pRequest,
                                  void (*relfcnp)(pkosal_os_dev ),
                                  struct aal_ipip       *ipipp);

};

#define _DECLARE_DEVFACTORY_TYPE struct aal_device_factory m_devfact
#define INIT_FACT_CREATE_DEVICE  m_devfact.create
#define INIT_FACT_DESTROY_DEVICE m_devfact.destroy

#define aaldev_factp(c) c->m_devfact
#define aaldev_fact(c)  c.m_devfact


//-----------------------
//  Device creation flags
//-----------------------
#define AAL_DEV_CREATE_UNREGISTERED (0x01)


// Default methods
static inline
struct aal_device *
aaldev_def_create_device(struct mafu_CreateAFU   *pRequest,
                         void (*relfcnp)(pkosal_os_dev ),
                         struct aal_ipip         *ipipp)
{
   UNREFERENCED_PARAMETER(pRequest);
   UNREFERENCED_PARAMETER(relfcnp);
   UNREFERENCED_PARAMETER(ipipp);
   return NULL;
}

static inline
btInt
aaldev_def_destroy_device(struct aal_device *devp)
{
   UNREFERENCED_PARAMETER(devp);
   return 0;
}

// Prototypes for AAL Bus interface
struct aal_device *
aaldev_create_device(struct mafu_CreateAFU *,
                     void (*relfcnp)(pkosal_os_dev ),
                     struct aal_ipip *);
btInt
aaldev_destroy_device(struct aal_device * );

// Used by the matching routines to mask individual fields of the ID
#define AAL_DEV_ID_MASK_VENDOR      (0x1 <<  1)
#define AAL_DEV_ID_MASK_AHMGUID     (0x1 <<  2)
#define AAL_DEV_ID_MASK_AFUGUID     (0x1 <<  3)
#define AAL_DEV_ID_MASK_PIPGUID     (0x1 <<  4)
#define AAL_DEV_ID_MASK_DEVID       (0x1 <<  5)
#define AAL_DEV_ID_MASK_SUBDEVNUM   (0x1 <<  6)
#define AAL_DEV_ID_MASK_BUSTYPE     (0x1 <<  7)
#define AAL_DEV_ID_MASK_DEVTYPE     (0x1 <<  8)
#define AAL_DEV_ID_MASK_ADDR        (0x1 <<  9)
#define AAL_DEV_ID_MASK_DEVNUM      (0x1 << 10)
#define AAL_DEV_ID_MASK_BUSNUM      (0x1 << 11)

// Common mask for resource manager
#define AAL_DEV_ID_MASK_EXCEPT_ADDR  AAL_DEV_ID_MASK_VENDOR+AAL_DEV_ID_MASK_AHMGUID+AAL_DEV_ID_MASK_AFUGUID+AAL_DEV_ID_MASK_PIPGUID+AAL_DEV_ID_MASK_BUSTYPE+AAL_DEV_ID_MASK_DEVTYPE

#define AAL_DEV_ID_MASK_EXACT       0

struct aaldev_ownerSession; //forward reference
struct aalui_wsid;

//=============================================================================
// Name: aal_uiapi
// Description: UI interface - Interface adaptor between and the
//              PIP and the Universal Interface Driver.
//=============================================================================
struct aal_uiapi {
   // Send a PIP event back to the Application
   btInt (*sendevent)(btObjectType ,                 // UI Session handle
                      struct aal_device *,           // Device
                      struct aal_q_item *,           // Event
                      btObjectType );                // Message context

   // Get a wsid for the workspace manager
   struct aal_wsid * (*getwsid)(struct aal_device *, // Device
                                btWSID );            // Internal wsid

   btInt (*freewsid)(struct aal_wsid *);              // Internal wsid

   /* validate a wsid against known list of allocated wsids */
   int (*valwsid)(struct aal_wsid *);
};



//=============================================================================
// Name: ownerSess_Init
// Description: Initialize a Owner Session
//=============================================================================
static inline
void
ownerSess_Init(struct aaldev_ownerSession *pSess, btAny ownerContext)
{
   memset(pSess, 0, sizeof(struct aaldev_ownerSession));
   pSess->m_ownerContext = ownerContext;  // Typically used by AIA to carry teh owner interface
   kosal_list_init(&pSess->m_wshead);
}


//=============================================================================
// Name: ownerSess_Copy
// Description: Copies an ownerSess object
//=============================================================================
static inline
void
ownerSess_Copy(struct aaldev_ownerSession *destSess,
               struct aaldev_ownerSession *srcSess)
{
   *destSess = *srcSess;
   kosal_list_replace_init(&srcSess->m_wshead, &destSess->m_wshead);
}

//=============================================================================
// Name: aaldev_owner
// Description: Represents a device owner. This object sits on 2 lists. The
//              devicelist is headed by the owning session (e.g., will have a
//              common pid). It represents the list of devices owned by the
//              session.
//              The ownerlist is headed by the device and
//              maintains a list owners of the device.
//=============================================================================
struct aaldev_owner {
   btPID                      m_pid;         // Pid of the owning process
   struct aal_device         *m_device;      // Device being owned
   btObjectType               m_manifest;    // Opaque manifest
   struct aaldev_ownerSession m_sess;        // Owner session
   kosal_list_head            m_devicelist;  // Session owner's device list
   kosal_list_head            m_ownerlist;   // Device owner list it is on
   kosal_semaphore            m_sem;         // Private semaphore
};


//=============================================================================
// Name: aaldev_owner_init
// Description: Initialize the Device Owner structure
// Interface: public
// Inputs:  pdevown - pointer to the device owner
//          pid - Process ID of owner
//          manifest - opaque pointer to a owner manifest
// Comments:
//=============================================================================
static inline
void
aaldev_owner_init(struct aaldev_owner *pdevown,
                  btPID                pid,
                  btAny                ownerContext,
                  btObjectType         manifest)
{
   kosal_list_init(&pdevown->m_devicelist);
   kosal_list_init(&pdevown->m_ownerlist);
   kosal_mutex_init(&pdevown->m_sem);

   pdevown->m_pid      = pid;
   pdevown->m_device   = NULL;
   pdevown->m_manifest = manifest;
   ownerSess_Init(&pdevown->m_sess, ownerContext);
}

//=============================================================================
// Name: AAL_PDO_DEVICE_CONTEXT
// Description: The context of the AAL Device.  
// Interface: public
// Comments:
//=============================================================================
typedef struct _AAL_PDO_DEVICE_CONTEXT
{
   // Common AAL Device Structure
   struct aal_device            *m_aaldevice;
   struct device_attributes     *m_pdevAttributes;
   // UNUSED
   //   AAL_BUS_WMI_STD_DATA   StdAALdeviceData;
   //   WDFDEVICE              pMontorConfigAPIControlDevice;


} AAL_PDO_DEVICE_CONTEXT, *PAAL_PDO_DEVICE_CONTEXT;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            AAL Class
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if   defined( __AAL_LINUX__ )
//=============================================================================
// Name: aal_class_id
// Description: Class ID identifies the interface
//=============================================================================
struct aal_class_id {
   btUnsigned32bitInt m_majorversion;
   btUnsigned32bitInt m_minorversion;
   btUnsigned64bitInt m_releaseversion;
   btUnsigned64bitInt m_classGUID;
};


//=============================================================================
// Name: aal_classdevice
// Description: AAL specific class device
//=============================================================================
struct aal_classdevice {
#define AAL_CLASSDEV_IS_REGISTERED 0x00000001
   btUnsigned32bitInt   m_flags;

   struct aal_class_id  m_classid;        // ID for  the public interface
   btIID               *m_devIIDlist;     // List of supported device module APIs

   struct aal_bus      *m_bus;            // AAL Bus interface

   // Base class
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
    struct device       m_classdev;       // Class device
#else
    struct class_device m_classdev;
#endif
};
#define aal_classdev_is_registered(p)  flag_is_set((p)->m_flags, AAL_CLASSDEV_IS_REGISTERED)
#define aal_classdev_set_registered(p) flag_setf((p)->m_flags,   AAL_CLASSDEV_IS_REGISTERED)
#define aal_classdev_clr_registered(p) flag_clrf((p)->m_flags,   AAL_CLASSDEV_IS_REGISTERED)

#define aal_classdevp_to_devp(p)       (&(p)->m_classdev)
#define aal_classdevp_to_dev(p)        ((p)->m_classdev)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
#define aal_classdev_set_name(p, cname) (dev_set_name(&(p)->m_classdev, cname))
#define aal_classdev_get_name(p)        (dev_name(&(p)->m_classdev))
#else
#define aal_classdev_set_name(p, cname) (strncpy((char *)(p)->m_classdev.class_id, cname, BUS_ID_SIZE))
#define aal_classdev_get_name(p)        ((const char *)( (p)->m_classdev.class_id ))
#endif

#define aal_classdev_devtype(p)        ((p)->m_classdev.devt)

#endif // OS

#if 0
//=============================================================================
// Name: aaldev_prep
// Description: Prepare the device structure for initialization
// Interface: public
// Inputs: pdev - pointer to the device
// Comments:
//=============================================================================
static inline void aaldev_prep(struct aal_device *pdev)
{
   // Initialize the structure
   memset(pdev, 0, sizeof(struct aal_device));

   dev_markUnRegistered(pdev);

   //Version of the structure
   pdev->m_version  = AAL_DEVICE_VERSION;

   // Used as part of the handle validation
   pdev->m_validator = (btUnsigned64bitInt)virt_to_phys(pdev);

   kosal_list_init(&pdev->m_ownerlist);
   kosal_mutex_init(&pdev->m_sem);
   kosal_mutex_init(&pdev->m_listsem);
}
#endif


//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------
extern
btInt
aaldev_init(struct aal_device * );

extern
aaldev_AddOwner_e
aaldev_addOwner(struct aal_device * ,
                btPID ,
                btObjectType ,
                btAny,
                pkosal_list_head );

extern
btInt
aaldev_removeOwner(struct aal_device * , btPID );

extern
aaldev_AddOwner_e
aaldev_udateOwner(struct aal_device * ,
                  btPID ,
                  struct aaldev_ownerSession * ,
                  pkosal_list_head );


END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALBUS_DEVICE_H__

