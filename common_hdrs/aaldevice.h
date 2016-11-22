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
//        FILE: aaldevice.h
//     CREATED: 09/18/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains public definitions for AAL device objects
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09/18/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 11/13/2008     HM       Added AAL_GUID_t, standard GUID
// 01/04/2009     HM       Updated Copyright
// 02/26/2009     JG       Began dynamic config implementation
// 10/09/2009     JG       Added support for Host Based AFUs
// 10/22/2009     JG       Added aaldev_AddOwner_e in support of modified
//                         device methods
// 04/28/2010     HM       Added return value checks to kosal_sem_get_krnl_alertable()
// 03/06/2011     HM       Added comment showing expansion of aal_device_id
// 06/25/2013     JG       Added device address macros
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALDEVICE_H__
#define __AALSDK_KERNEL_AALDEVICE_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/iaaldevice.h>
#include <aalsdk/kernel/aalbus-device.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                               AAL Device
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define  AAL_DEVICE_VERSION   0x0001000000000200

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

#if 0
//=============================================================================
// Name: aaldevCallbacks
// Description: Interface for driver notifications
//=============================================================================
struct aaldevCallbacks
{
   void (*devReleased)(struct device*);                  // Called when device released
   btInt(*devAdd)( struct aal_device *dev);              // Function called when device is created
};
#endif

#if 1
// KObject length restrictions disapppear in 2.6.31 and later
#if   defined( __AAL_LINUX__ )
# if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#    define BUS_ID_SIZE 20
# endif
#elif defined( __AAL_WINDOWS__ )
# define BUS_ID_SIZE 20
#endif // OS


//=============================================================================
// Name: aal_device
// Description: AAL specific device object definition
//=============================================================================
struct aal_device {
   btUnsigned64bitInt m_version;       // Interface version
#define AAL_DEVICE_FLAG_IS_REGISTERED 0x00000001
   btUnsigned16bitInt m_flags;

   // Device interface used by AAL Bus
   AAL_DEVICE_INTERFACE          i;

   // Device specific information
   struct aal_device_id m_devid;       // Device ID

   //-----------------------
   // Interface plug-ins
   //-----------------------
#if DEPRECATED //NOT USED
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
   int (*m_uevent)(struct aal_device *dev, struct kobj_uevent_env *env);
#else
   int (*m_uevent)(struct aal_device *dev,
                   char             **envp,
                   int                num_envp,
                   char              *buffer,
                   int                buffer_size);
#endif
#endif // DEPRECATED

   // PIP Interface bindings
   struct aal_interface *m_ipip;           // PIP service interface container
   struct aal_ipip      *m_pip;            // PIP interface
   btObjectType          m_pipContext;     // PIP specific context


   btInt                 m_devstate;       // Device state 1 - if activated 0 - if quiescent
   kosal_list_head       m_ownerlist;      // List of owning sessions
   kosal_mutex           m_listsem;        // Lock used for protecting list manipulations
   btUnsigned32bitInt    m_numowners;      // Number of owners
   btUnsigned32bitInt    m_maxowners;      // MAximum number of owners
   btUnsigned16bitInt    m_mappableAPI;    // Supports direct access to PIP

   kosal_mutex           m_sem;            // Private mutex

   struct aal_bus       *m_bus;            // AAL Bus interface

   kosal_os_dev          m_dev;            // Device base class
   char                  m_basename[BUS_ID_SIZE];
   btPhysAddr            m_validator;      // Used for validation
   /* allocation list.  head is in aal_bus_type.alloc_list_head */
   kosal_list_head       m_alloc_list;

   btAny                 m_context;        //

};

//-----------------
//   Casting Macros
//-----------------
#define basedev_to_aaldev(dev)           ( kosal_container_of(dev, struct aal_device, m_dev) )
#define aaldev_to_basedev(dev)           ((dev)->m_dev)

#define aaldev_to_any(s,pdev)            ((s)(pdev->m_context))
#define aaldev_context(pdev)             (pdev->m_context)

#define aaldev_is_registered(p)          flag_is_set((p)->m_flags, AAL_DEVICE_FLAG_IS_REGISTERED)
#define aaldev_set_registered(p)         flag_setf((p)->m_flags,   AAL_DEVICE_FLAG_IS_REGISTERED)
#define aaldev_clr_registered(p)         flag_clrf((p)->m_flags,   AAL_DEVICE_FLAG_IS_REGISTERED)

//-----------------------
// aal_device Accesssors
//-----------------------
#define aaldev_pipid(d)                  ((d)->m_devid.m_pipGUID)                // PIP IID
#define aaldev_pipp(d)                   ((d)->m_pip)                            // PIP pointer
#define aaldev_pipmsgHandlerp(d)         ( &(aaldev_pipp(d))->m_messageHandler ) // Message handler
#define aaldev_interface(d)              ((d)->i)                                // Device interface
#define aaldev_pip_interface(d)          ((d)->m_ipip)                           // PIP interface container object
#define aaldev_pip_context(d)            ((d)->m_pipContext)                     // Context
#define aaldev_pip_context_to_obj(t,d)   ( (t) (d->m_pipContext) )               // Context cast to type

#if   defined( __AAL_LINUX__ )
#  if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
#     define aaldev_devname(dev)              ( dev_name(&aaldev_to_basedev(dev)) )
#  else
#     define aaldev_devname(dev)              ((dev)->m_dev.bus_id)
#  endif
#elif defined( __AAL_WINDOWS__ )
#     define aaldev_devname(dev)              ((dev)->m_dev.m_basename)
#endif // OS


#define aaldev_bus(dev)                  (*(dev)->m_bus)
#define aaldev_busp(dev)                 ((dev)->m_bus)
#define aaldev_basedriverp(d)            (aaldev_to_basedev(d).driver)

#define aaldev_basename(dev)             ((dev)->m_basename)

#define aaldev_devid(dev)                ((dev)->m_devid)
#define aaldev_devid_devtype(dev)        (((dev)->m_devid).m_devicetype)
#define aaldev_devid_ahmguid(dev)        (((dev)->m_devid).m_ahmGUID)
#define aaldev_devid_pipguid(dev)        (((dev)->m_devid).m_pipGUID)
#define aaldev_devid_afuguid(dev)        (((dev)->m_devid).m_afuGUID)
#define aaldev_devid_afuguidl(dev)       (((dev)->m_devid).m_afuGUIDl)
#define aaldev_devid_afuguidh(dev)       (((dev)->m_devid).m_afuGUIDh)
#define aaldev_devid_vendorid(dev)       (((dev)->m_devid).m_vendor)

#define aaldev_devaddr(dev)              ((dev)->m_devid.m_devaddr)
#define aaldev_devaddr_bustype(dev)      (aaldev_devaddr(dev).m_bustype)
#define aaldev_devaddr_busnum(dev)       (aaldev_devaddr(dev).m_busnum)
#define aaldev_devaddr_devnum(dev)       (aaldev_devaddr(dev).m_devicenum)
#define aaldev_devaddr_fcnnum(dev)       (aaldev_devaddr(dev).m_functnum)
#define aaldev_devaddr_subdevnum(dev)    (aaldev_devaddr(dev).m_subdevnum)
#define aaldev_devaddr_instanceNum(dev)  (aaldev_devaddr(dev).m_instanceNum)
#define aaldev_devaddr_socketnum(dev)    (aaldev_devaddr(dev).m_socketnum)

// Utility macros
#define aaldev_haspip(devp)              (NULL != (devp)->m_pip)
#define aaldev_mappableAPI(devp)         ((devp)->m_mappableAPI)
#define aaldev_allowsDirectAPI(devp)     (AAL_DEV_APIMAP_NONE != (devp)->m_mappableAPI)
#define aaldev_allowsAPIMode(devp, m)    ( ((devp)->m_mappableAPI & (m)) == (m) )

//-----------------
// Utility
//-----------------
#define link_aalchild_to_parent(c,p)     ((c)->m_dev.parent =  &((p)->m_dev))
#define is_driver_device_owner(drvp,dev) ( aaldrv_to_basep(drvp) == aaldev_basedriverp(dev) )
#endif


//=============================================================================
// Name: aaldev_create
// Description: Creates the device structure.
// Interface: public
// Returns aal_device * - success; NULL = failure
// Inputs: devIDp - pointer to aal_device ID to create
//         ipipp - pointer to PIP for this device.
// Outputs: none.
// Comments:
//=============================================================================
static inline struct aal_device *
         aaldev_create( char *szDevName,
                        struct aal_device_id *devID,
                        struct aal_ipip      *ipipp)
{
#define AAL_AFU_DEVICE_BASENAME     "AALAFU"

   struct aal_device    *paaldevice = NULL;

   ASSERT(devID);

   // Allocate the new aal_device
   paaldevice = (struct aal_device    *)kosal_kmalloc(sizeof(struct aal_device));
   if ( NULL == paaldevice ) {
      PERR( ": Error allocating device memory for bus type %d busID[%d:%d:%x:%d]\n",
                                 (unsigned) devID->m_devaddr.m_bustype,
                                 devID->m_devaddr.m_busnum,
                                 devID->m_devaddr.m_devicenum,
                                 devID->m_devaddr.m_subdevnum,
								 devID->m_devaddr.m_instanceNum);
      return NULL;
   }

   // DPRINTF do not resolve properly in a header file because there is no well-defined
   //   external linkage for the "debug" variable. This should be PINFO with the correct
   //   module linkage defined in the calling .c file set up prior to including this file.
   // See RedMine 566
   //   DPRINTF(AALBUS_DBG_MOD," Preparing AAL device %s\n", szDevName);

    // Prepare the new device
   memset(paaldevice, 0, sizeof(struct aal_device));

   paaldevice->m_devid        = *devID;

   // Store the base name
   if ( 0 == strlen(szDevName) ) {
      // Default
      strncpy(aaldev_basename(paaldevice), AAL_AFU_DEVICE_BASENAME, BUS_ID_SIZE-1);
   } else {
      strncpy(aaldev_basename(paaldevice), szDevName,      BUS_ID_SIZE-1);
   }
#if   defined( __AAL_LINUX__ )
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
   dev_set_name(&aaldev_to_basedev(paaldevice),
                "%s[%d:%d:%d:%x:%d:%d]",
                (char*)aaldev_basename(paaldevice),
                aaldev_devaddr_busnum(paaldevice),
                aaldev_devaddr_devnum(paaldevice),
                aaldev_devaddr_fcnnum(paaldevice),
                aaldev_devaddr_subdevnum(paaldevice),
                aaldev_devaddr_socketnum(paaldevice),
                aaldev_devaddr_instanceNum(paaldevice));
#else
   // Construct the device name from the base name and the device ID
   snprintf((char*)aaldev_devname(paaldevice), BUS_ID_SIZE,
            "%s[%d:%d:%d:%x:%d:%d]",
            (char*)aaldev_basename(paaldevice),
            aaldev_devaddr_busnum(paaldevice),
            aaldev_devaddr_devnum(paaldevice),
            aaldev_devaddr_fcnnum(paaldevice),
            aaldev_devaddr_subdevnum(paaldevice),
            aaldev_devaddr_socketnum(paaldevice),
            aaldev_devaddr_instanceNum(paaldevice));
#endif
#endif

   kosal_list_init(&paaldevice->m_ownerlist);
   kosal_list_init(&paaldevice->m_alloc_list);
   kosal_mutex_init(&paaldevice->m_sem);
   kosal_mutex_init(&paaldevice->m_listsem);

   // Version of the structure
   paaldevice->m_version      = AAL_DEVICE_VERSION;

   // Used as part of the handle validation
   paaldevice->m_validator = kosal_virt_to_phys( paaldevice );

   // DPRINTF do not resolve properly in a header file because there is no well-defined
   //   external linkage for the "debug" variable. This should be PINFO with the correct
   //   module linkage defined in the calling .c file set up prior to including this file.
   // See RedMine 566
   // Initialize the device ID info
   // DPRINTF(AALBUS_DBG_MOD," Initializing AAL device %p\n", paaldevice);

   // The Context is a PIP defined data value
   //   The standard AAL PIP pointer is stored as well
   paaldevice->m_pipContext   = NULL;
   aaldev_pipp(paaldevice)    = ipipp;

   return paaldevice;
}
END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALDEVICE_H__

