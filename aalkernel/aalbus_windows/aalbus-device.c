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
//        FILE: aalbus-device.c
//     CREATED: 09/15/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the implementation for AAL device generic
//           behavior for Microsoft Windows.
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           Accelerator Hardware Module bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09/15/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 12/16/2008     JG       Began support for abort and shutdown
//                            Added Support for WSID object
//                            Major interface changes.
// 12/18/2008     JG       Added use of owner Session initializer
//                            and copier to ensure pointers are initialized
// 12/23/2008     JG       Modified UpdateOWner to support passing
//                            pack same owner session as already set
// 01/04/2009     HM       Updated Copyright
// 10/01/2009     JG       Added remove method to the device
// 10/22/2009     JG       Added aaldev_AddOwner_e in support of modified
//                         device methods
// 04/28/2010     HM       Added return value checks to down_interruptible()
// 06/02/2014     JG       Created Windows version
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALBUS_DBG_MOD

//#include "aalbus-int.h"
//#include "aalsdk/kernel/aalinterface.h"
#include "aalbus_internal.h"

#include "aalsdk/kernel/aaldevice.h"
#include "aalsdk/kernel/aalbus_iupdate_config.h"
#include "aalsdk/kernel/aalbus_iconfigmonitor.h"
#include "aalsdk/kernel/aalbus-device.h"

#include "aalbus_events.h"


// TODO - NEEDS PROPER IMPLEMENTATION
inline static int
aaldev_valid(struct aal_device *pdev)
{
   UNREFERENCED_PARAMETER(pdev);
   return 1;
}

// TODO THESE FUNCIONS CAN BE MADE PORTABLE BY ABSTRACTING THE DEVICE aal_device in Linux and WDFDEVICE in WINDOWS

//=============================================================================
// Name:  aalbus_getDevAttributes
// Description: Returns a pointer to the device attributes 
// Interface: public
// Inputs: dev - Device to obtain interface from
// Outputs: NULL if no attributes exist
// Comments:
//=============================================================================
struct device_attributes     *aalbus_getDevAttributes( WDFDEVICE dev )
{
   struct device_attributes     *pdevAttributes = NULL;

   PTRACEIN;

   pdevAttributes = aalbus_config_dev_pattributes( dev );

   if( NULL == pdevAttributes ) {
      PERR( "Device has no attributes!\n" );
      return NULL;
   }

   return pdevAttributes;
}

//=============================================================================
// Name:  aalbus_getExtendedAttributes
// Description: Returns a pointer to the device extended attributes 
// Interface: public
// Inputs: dev - Device to obtain interface from
// Outputs: NULL if no attributes exist
// Comments:
//=============================================================================
btAny aalbus_getExtendedAttributes( WDFDEVICE dev )
{
   PAAL_PDO_DEVICE_CONTEXT context;
   PTRACEIN;

   context = aalbus_config_dev_pextattributes( btAny, WdfObjectGetAALContext( dev )->m_pdevAttributes );
   
   if( NULL == context ) {
      PERR("Device has no context!\n");
      return NULL;
   } else if( NULL == context->m_pdevAttributes) {
      PERR( "Device has no attributes!\n" );
      return NULL;
   }

   // aalbus_config_dev_pextattributes() returns NULL if no attributes
   return aalbus_config_dev_pextattributes( btAny, context->m_pdevAttributes );
}

//=============================================================================
// Name:  aalbus_getPublicAttributes
// Description: Returns a pointer to the device public attributes 
// Interface: public
// Inputs: dev - Device to obtain interface from
// Outputs: NULL if no attributes exist
// Comments:
//=============================================================================
btAny aalbus_getPublicAttributes( WDFDEVICE dev )
{
   PAAL_PDO_DEVICE_CONTEXT context;
   PTRACEIN;

   context = aalbus_config_dev_pextattributes( btAny, WdfObjectGetAALContext( dev )->m_pdevAttributes );

   if( NULL == context ) {
      PERR( "Device has no context!\n" );
      return NULL;
   } else if( NULL == context->m_pdevAttributes ) {
      PERR( "Device has no attributes!\n" );
      return NULL;
   }

   // aalbus_config_dev_ppubattributes() returns NULL if no attributes
   return aalbus_config_dev_ppubattributes( btAny, context->m_pdevAttributes );
}


//=============================================================================
// Name:  aalbus_sendDeviceUpdate
// Description: Sets the public visibility of the device/Resource by causing
//                a conf monitor event.
// Interface: public
// Inputs: dev - Device 
// Returns: TRUE - Success.
// Comments:
//=============================================================================
btBool aalbus_sendDeviceUpdate( struct device_attributes* pdevattr, 
                                krms_cfgUpDate_e updateType )
{
   struct aalbus_config_update_event * pEvt = NULL;
   btInt ret;

   // Create the event
   pEvt = aalbus_config_update_event_create(pdevattr, updateType);

   // Send it
   ret = Bus_IResMon_SendEvent( pEvt );

   return true;
}


//=============================================================================
// Name:  aalbus_destroy_device
// Description: Destroys an AAL device type
// Interface: public
// Inputs: devp - Device to destroy.
// Outputs: 1 - success.
// Comments:
//=============================================================================
int
aaldev_destroy_device(struct aal_device *devp)
{

   kosal_list_del(&devp->m_alloc_list);
   kosal_kfree(devp, sizeof(struct aal_device));
   DPRINTF(AALBUS_DBG_MOD, "device %p destroyed\n", devp);
   return 1;
}
#if 0 
//=============================================================================
// Name: aaldev_release_device
// Description: Called by Linux as a result of the unregister call. This
//              function removes the device from memory by calling the  AAL Bus
//              destroy()
// Interface: private
// Inputs: pdev - generic device structure
// Outputs: none
// Returns:void
// Comments:
//=============================================================================
void
aaldev_release_device(struct device *pdev)
{
   struct aal_bus    *AALbusp = aalbus_get_bus();
   struct aal_device *paaldev = basedev_to_aaldev(pdev);

   // If a user release method is installed call it.
   if ( dev_hasRelease(paaldev) ) {
      dev_release(paaldev, pdev);
   }

   // Destroy the device
   kosal_printk_level(KERN_INFO, "Destroying AFU Device.\n");
   aaldev_factp(AALbusp).destroy(paaldev);
}


//=============================================================================
// Name: aaldev_create_device
// Description: Creates the device and registers with AAL Bus.
// Interface: public
// Returns 0 - success
// Inputs: pManifest - pointer to manifest containing device attributes
// Outputs: none.
// Comments: Once this device has been created this DKSM must be ready to
//           accept requests as the MAFU will be exposed to the AAL user
//           subsystem.
//=============================================================================
struct aal_device *
   aaldev_create_device(struct device_attributes *pAttributes)
{
   struct aal_device_id *devID = &pAttributes->device_id;
   struct aal_device    *paaldevice = NULL;

   ASSERT(devID);

   paaldevice = (struct aal_device    *)kosal_kmalloc(sizeof(struct aal_device));
   if ( NULL == paaldevice ) {
      DPRINTF(AALBUS_DBG_MOD, ": Error allocating device memory for bus type %d busID[%d:%d]\n",
                                 (unsigned) devID->m_devaddr.m_bustype,
                                 devID->m_devaddr.m_devicenum,
                                 devID->m_devaddr.m_subdevnum);
      return NULL;
   }

   DPRINTF(AALBUS_DBG_MOD," Preping AAL device %p\n", paaldevice);

   // Prepare the new device
   memset(paaldevice, 0, sizeof(struct aal_device));

   kosal_list_init(&paaldevice->m_ownerlist);
   kosal_list_init(&paaldevice->m_alloc_list);
   kosal_mutex_init(&paaldevice->m_sem);
   kosal_mutex_init(&paaldevice->m_listsem);

   // Version of the structure
   paaldevice->m_version     = AAL_DEVICE_VERSION;

   // Initialize the device ID info
   DPRINTF(AALBUS_DBG_MOD," Initializing AAL device %p\n", paaldevice);
   paaldevice->m_devid = *devID;

   // The Context is a PIP defined data value
   //   The standard AAL PIP pointer is stored as well
   paaldevice->m_pipContext = NULL;

   // Set the maximum number of shares
   paaldevice->m_maxowners = pAttributes->maxOwners;

   // Is direct access to PIP enabled for this device?
   paaldevice->m_mappableAPI = pAttributes->enableDirectAPI;

   return paaldevice;
}
#endif

//=============================================================================
// Name: aaldev_find_pid_owner
// Description: Finds the device owner that has this pid
// Interface: private
// Inputs: pid - Process ID to search for
//         pdev - pointer to device
// Returns: aal_devowner * of owner or else NULL
// Comments: This routine is not protected but uses the safe version of the
//           list_for_each making it safe for removals.
//=============================================================================
static inline
struct aaldev_owner *
aaldev_find_pid_owner(struct aal_device *pdev,
                      btPID              pid)
{
   kosal_list_head     *pitr;
   kosal_list_head     *temp;
   struct aaldev_owner *result = NULL;

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return 0;
   }

   // Loop through the list looking for a match
   kosal_list_for_each_safe(pitr, temp, &pdev->m_ownerlist) {

      // If the pid matches
      result = kosal_container_of(pitr, struct aaldev_owner, m_ownerlist);
      DPRINTF (AALBUS_DBG_MOD, ": Owner pid %d  Requesting pid %d\n", result->m_pid, pid);

      if ( result->m_pid == pid ) {
         return result;
      }

   }

   return NULL;
}

//=============================================================================
// Name: aaldev_addOwner
// Description: Add a new owner to the ownership list
// Interface: public
// Inputs: pdev - pointer to the device
//         pid - ID of owning process
//         manifest - opaque block associated with owner
//         psessionlist - pointer to the new owning sessions owner list
// Returns: aaldev_addowner_OK - success
// Comments: Owner must be unique.  The psessionlist is used to enable a
//          session object to maintain a list of devices it owns
//=============================================================================
aaldev_AddOwner_e
aaldev_addOwner(struct aal_device *pdev,
                btPID              pid,
                void              *manifest,
                btAny              pContext,
                kosal_list_head   *psessionlist)
{
   aaldev_AddOwner_e    ret  = aaldev_addowner_OK;
   struct aaldev_owner *pown = NULL;

   DPRINTF (AALBUS_DBG_MOD, ": Adding Owner\n");

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return aaldev_addowner_NotOwner;
   }

   // Lock the list from any updates
   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return aaldev_addowner_Interrupted;
   }

   // Make sure the device owner count has not been exceeded
   // TODO support multiple owners (sharing)
   if( ( MAFU_CONFIGURE_UNLIMTEDSHARES != pdev->m_numowners ) &&
       ( pdev->m_numowners >= pdev->m_maxowners ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Failed - Maximum Owners assigned %d exceeds %d\n",
                                                   pdev->m_numowners,
                                                   pdev->m_maxowners);
      ret = aaldev_addowner_MaxOwners;
      goto out;
   }

   // Make sure this pid is unique
   if ( aaldev_find_pid_owner(pdev,pid) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Owner already assigned!%d\n",pid);
      ret = aaldev_addowner_DupOwner;
      goto out;
   }

   // Allocate the new owner struct
   pown = (struct aaldev_owner *)kosal_kmalloc(sizeof(struct aaldev_owner));
   if ( unlikely( NULL == pown ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": failed to allocate owner structure\n");
      ret = aaldev_addowner_SysErr;
      goto out;
   }

   // Create the owner
   aaldev_owner_init(pown, pid, NULL, manifest);

   // Record the device
   pown->m_device = pdev;

   // Add the owner to the device's owner list.
   // This is the list of owners for this device
   kosal_list_add(&pown->m_ownerlist, &pdev->m_ownerlist);

   // Add this owner object to the session's list
   // This is a list of devices owned by this session
   kosal_list_add(&pown->m_devicelist, psessionlist);

   // Increment number of owners
   if ( MAFU_CONFIGURE_UNLIMTEDSHARES != pdev->m_numowners ) {
      pdev->m_numowners++;
   }
#if 0
   // Send the configuration update event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerAdded,
                                              pid);
#endif
out:
   kosal_sem_put(&pdev->m_sem);
   return ret;
}


//=============================================================================
// Name: aaldev_isOwner
// Description: Returns whether the PID is an owner of the device
// Interface: public
// Inputs: pdev - pointer to the device
//         pid - ID of owning process
// Returns: aaldev_addowner_OK - An Owner
// Comments:
//=============================================================================
aaldev_AddOwner_e
aaldev_isOwner(struct aal_device *pdev,
               btPID              pid)
{
   aaldev_AddOwner_e ret = aaldev_addowner_OK;

   if( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return aaldev_addowner_InvalidDevice;
   }

   // Lock the list from any updates
   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return aaldev_addowner_Interrupted;
   }

   // Use find owner
   ret = (NULL == aaldev_find_pid_owner(pdev, pid) ? aaldev_addowner_NotOwner : aaldev_addowner_OK);

   kosal_sem_put(&pdev->m_sem);

   return ret;
}

//=============================================================================
// Name: aaldev_removeOwner
// Description: Remove an owner from the ownership list
// Interface: public
// Inputs: pdev - pointer to the device. NULL indicates remove all.
//         pid - ID of owner to remove from the list
// Returns: aaldev_addowner_OK - success
// Comments:
//=============================================================================
int
aaldev_removeOwner(struct aal_device *pdev, btPID pid)
{
   int                  ret  = 1;
   struct aaldev_owner *pown = NULL;

   DPRINTF (AALBUS_DBG_MOD, ": Removing Owner\n");

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return  aaldev_addowner_InvalidDevice;
   }

   // Lock the list from any updates
   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return aaldev_addowner_Interrupted;
   }

   pown = aaldev_find_pid_owner(pdev, pid);

   // Make sure this pid is unique
   if ( unlikely(NULL == pown) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Not Owner\n");
      ret = aaldev_addowner_NotOwner;
      goto out;
   }

   kosal_list_del_init(&pown->m_devicelist);
   kosal_list_del_init(&pown->m_ownerlist);

   // done list update
   kosal_kfree(pown, sizeof(struct aaldev_owner));

   // Decrement number of owners
   pdev->m_numowners--;
#if 0
   // Send the configuration update event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerRemoved,
                                              pid);
#endif
out:
   kosal_sem_put(&pdev->m_sem);
   return ret;
}

//=============================================================================
// Name: aaldev_updateOwner
// Description: Update an existing owner record
// Interface: public
// Inputs: pdev - pointer to the device
//         pid - ID of owner to find
//         ownerSessionp  - pointer to owner session
//         psessionlist - session list to replace with
// Returns: aaldev_addowner_OK - success
// Comments:
//=============================================================================
aaldev_AddOwner_e
aaldev_updateOwner(struct aal_device          *pdev,
                   btPID                       pid,
                   struct aaldev_ownerSession *ownerSessionp,
                   kosal_list_head            *psessionlist)
{
   aaldev_AddOwner_e    ret  = aaldev_addowner_OK;
   struct aaldev_owner *pown = NULL;

   DPRINTF (AALBUS_DBG_MOD, ": Updating Owner for device %p on list %p\n", pdev, psessionlist);

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return aaldev_addowner_InvalidDevice;
   }

   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return aaldev_addowner_Interrupted;
   }

   pown = aaldev_find_pid_owner(pdev, pid);
   if ( unlikely(NULL == pown) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Not Owner\n");
      ret = aaldev_addowner_NotOwner;
      goto out;
   }

   // Remove this entry from the previous session list
   kosal_list_del(&pown->m_devicelist);

   // If the session info is being updated
   if( ( NULL != ownerSessionp ) &&
       ( ownerSessionp != &pown->m_sess) ) {
      ownerSess_Copy(&pown->m_sess, ownerSessionp);
   }

   // Add it to the new session owners list
   kosal_list_add(&pown->m_devicelist, psessionlist);

   DPRINTF (AALBUS_DBG_MOD, ": Done Updating Owner\n");
#if 0
   // Send the configuration update event
   // TODO RIGHT NOW AFUs DON'T HAVE NECESSARILY HAVE A BUS SET.
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerUpdated,
                                              pid);
#endif
out:
   kosal_sem_put(&pdev->m_sem);
   return ret;
}

//=============================================================================
// Name: aaldev_getOwnerSession
// Description: Return the owner session
// Interface: public
// Inputs: pdev - pointer to the device
//         pid - owner ID
// Returns: pointer to owner session or NULL is not owner
// Comments:
//=============================================================================
struct aaldev_ownerSession *
aaldev_getOwnerSession(struct aal_device *pdev,
                       btPID              pid)
{
   struct aaldev_ownerSession *ret  = NULL;
   struct aaldev_owner        *pown = NULL;

   DPRINTF (AALBUS_DBG_MOD, ": Getting Owner of device %p\n", pdev);

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": Invalid device\n");
      return NULL;
   }

   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return NULL;
   }

   pown = aaldev_find_pid_owner(pdev, pid);
   if ( likely( NULL != pown ) ) {
      ret = &pown->m_sess;
      DPRINTF (AALBUS_DBG_MOD, ": Returning session %p\n", ret);
   }

   kosal_sem_put(&pdev->m_sem);
   return ret;
}


//=============================================================================
// Name: aaldev_doForeachOwner
// Description: Calls a function on each owner on the list until complete or
//              until the callback return non-zero.
// Interface: public
// Inputs: pdev - pointer to the device
// Returns: pointer to owner stopped on or NULL if list completed
// Comments:
//=============================================================================
struct aaldev_owner *
aaldev_doForeachOwner(struct aal_device      *pdev,
                      aaldev_OwnerProcessor_t fcn)
{
   kosal_list_head *pitr;
   kosal_list_head *temp;

   // Loop through the list looking for a match
   kosal_list_for_each_safe(pitr, temp, &pdev->m_ownerlist) {

      // If the function returns non-zero return current iterator
      if ( 0 != fcn(pdev, kosal_container_of(pitr, struct aaldev_owner, m_ownerlist)) ) {
         return kosal_container_of(pitr, struct aaldev_owner, m_ownerlist);
      }

   }

   return NULL;
}

//=============================================================================
// Name: aaldev_quiesce
// Description: Update an existing owner record
// Interface: public
// Inputs: pdev - pointer to the device
// Returns: 0 - success
// Comments:
//=============================================================================
int
aaldev_quiesce(struct aal_device *pdev, struct aaldev_owner *pown)
{
   UNREFERENCED_PARAMETER(pdev);
   UNREFERENCED_PARAMETER(pown);
   return 0;
}

//=============================================================================
// Name: aaldev_remove
// Description: Remove the device
// Interface: public
// Inputs: pdev - pointer to the device
// Returns: 1 - success
// Comments:
//=============================================================================
int
aaldev_remove(struct aal_device *pdev)
{
   int ret = 1;

   kosal_printk_level(KERN_INFO, "Removing device %p\n", pdev);

   if ( unlikely( !aaldev_valid(pdev) ) ) {
      kosal_printk_level(KERN_ERR, "Invalid device %p\n", pdev);
      return 0;
   }

   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
      kosal_printk_level(KERN_ERR, "kosal_sem_get_user_alertable interrupted\n");
      return 0;
   }

   // For now disallow destruction with non-zero owners.
   //  Eventually this should be changed to notify owners
   //  via a status update event to remove
   if ( unlikely( 0 != pdev->m_numowners ) ) {
      kosal_printk_level(KERN_ERR, "Non-zero owner count : %u\n", pdev->m_numowners);
      ret = 0;
      goto out;
   }
#if 0
   // Send an update configuration event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevRemoved,
                                              0);
#endif
   kosal_printk_level(KERN_INFO, "Done removing %p\n", pdev);

out:
   kosal_sem_put(&pdev->m_sem);
   return ret;
}

//=============================================================================
// Name: aaldev_init
// Description: Initialized the device structure
// Interface: public
// Inputs: pdev - pointer to the device
// Returns: 1 - success
// Comments:
//=============================================================================
int
aaldev_init(struct aal_device *pdev)
{
   kosal_printk_level(KERN_INFO, "Initializing 0x%p\n", pdev);

   if( unlikely( !aaldev_valid(pdev) ) ) {
      kosal_printk_level(KERN_ERR, "aaldev_initFailed\n");
      return 0;
   }
#if 0
   //  Initialize interface
   pdev->i.getDevAttributes      = aalbus_getDevAttributes;
   pdev->i.getExtendedAttributes = aalbus_getExtendedAttributes;
   pdev->i.getPublicAttributes   = aalbus_getPublicAttributes;
   pdev->i.sendDeviceUpdate      = aalbus_sendDeviceUpdate;
#endif
   pdev->i.addOwner              = aaldev_addOwner;
   pdev->i.isOwner               = aaldev_isOwner;
   pdev->i.removeOwner           = aaldev_removeOwner;
   pdev->i.updateOwner           = aaldev_updateOwner;
   pdev->i.getOwnerSession       = aaldev_getOwnerSession;
   pdev->i.doForeachOwner        = aaldev_doForeachOwner;
   pdev->i.quiesce               = aaldev_quiesce;
   pdev->i.remove                = aaldev_remove;

   kosal_list_init(&pdev->m_ownerlist);
   kosal_mutex_init(&pdev->m_sem);
   kosal_mutex_init(&pdev->m_listsem);

   return 1;
}

