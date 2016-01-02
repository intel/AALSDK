//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
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
//        FILE: aalbus-device.c
//     CREATED: 09/15/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the implementation for AAL device generic
//           behavior.
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
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALBUS_DBG_MOD

#include "aalbus-int.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/aaldevice.h"


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
   int status;
   int retval = -EINVAL;
   struct aal_bus *aal_bus_p;
   struct aal_bus_type *aal_bus_type_p;

   /* get a reference to the public aal_bus as well as the "private"
    * aal_bus_type.  aal_bus_type_p will always point to something if
    * aal_bus_p is non-null, so there's no reason to check it for validity.
    * (although if we're way off into the weeds, it may point further off into
    * the weeds...) */
   aal_bus_p = aalbus_get_bus();
   if (NULL == aal_bus_p) {
      DPRINTF(AALBUS_DBG_MOD, " aalbus_get_bus failed\n");
      return -1;
   }
   aal_bus_type_p = kosal_container_of(aal_bus_p, struct aal_bus_type, m_bus);

   /* check device validity before proceeding.  if devp isn't valid, it isn't
    * a device that is known to us, and is likely a bug. */
   status = aal_bus_p->dev_is_valid(devp);
   if (0 != status) {
      status = kosal_sem_get_user_alertable(&aal_bus_type_p->alloc_list_sem);
      if (0 != status) {
         DPRINTF(AALBUS_DBG_MOD, " interrupted while acquiring lock for allocation list\n");
         return status;
      }
      list_del(&devp->m_alloc_list);
      kosal_sem_put(&aal_bus_type_p->alloc_list_sem);
      kfree(devp);
      retval = 1;
      DPRINTF(AALBUS_DBG_MOD, "device %p destroyed\n", devp);
   } else {
      DPRINTF(AALBUS_DBG_MOD, "device %p not a valid aal_device!\n", devp);
      /* not a valid device, so we don't touch it.  error will be propagated
       * back to the caller. */
   }

   return retval;
}

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
   DPRINTF(AALBUS_DBG_MOD, "Destroying AFU Device.\n");
   aaldev_factp(AALbusp).destroy(paaldev);
}

//=============================================================================
// Name: aaldev_create_device
// Description: Creates the device and registers with AAL Bus.
// Interface: public
// Returns 0 - success
// Inputs: devIDp - pointer to aal_device ID to create
//         basename - basename to use. /0 - uses default
//         relfcnp - pointer to driver release function. NULL - uses default
//         ipipp - pointer to PIP for this device.
// Outputs: none.
// Comments: Once this device has been created this DKSM must be ready to
//           accept requests as the MAFU will be exposed to the AAL user
//           subsystem.
// TODO - Add manifest and uevent
//=============================================================================
struct aal_device *
aaldev_create_device(struct mafu_CreateAFU *pRequest,
                     void (*relfcnp)(struct device*),
                     struct aal_ipip       *ipipp)
{
   struct aal_device_id *devID      = &pRequest->device_id;
   struct aal_device    *paaldevice = NULL;
   struct aal_bus 		*aal_bus_p;
   struct aal_bus_type 	*aal_bus_type_p;
   int status;


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

   //aaldev_prep(paaldevice);
   {
      // Prepare the new device
      memset(paaldevice, 0, sizeof(struct aal_device));

      kosal_list_init(&paaldevice->m_ownerlist);
      kosal_list_init(&paaldevice->m_alloc_list);
      kosal_mutex_init(&paaldevice->m_sem);
      kosal_mutex_init(&paaldevice->m_listsem);

      // Version of the structure
      paaldevice->m_version     = AAL_DEVICE_VERSION;

      // Used as part of the handle validation
      paaldevice->m_validator   = (__u64)virt_to_phys(paaldevice);

      paaldevice->m_dev.release = aaldev_release_device; // System method

   }

   /*
    * add to list of allocated devices
    */

   /* get handle to aalbus and outer aalBus */
   aal_bus_p = aalbus_get_bus();
   if (NULL == aal_bus_p) {
      DPRINTF(AALBUS_DBG_MOD, " aalbus_get_bus failed\n");
      kfree(paaldevice);
      return NULL;
   }
   aal_bus_type_p = kosal_container_of(aal_bus_p, struct aal_bus_type, m_bus);
   DPRINTF(AALBUS_DBG_MOD, " aal_bus at %p has bus_type at %p, with flags "
      "%x\n", aal_bus_p, aal_bus_type_p, aal_bus_type_p->m_flags);

   status = kosal_sem_get_user_alertable(&aal_bus_type_p->alloc_list_sem);
   if (0 != status) {
      DPRINTF(AALBUS_DBG_MOD, " Failed to acquire allocate list semaphore\n");
      kfree(paaldevice);
      return NULL;
   } else {
      list_add(&paaldevice->m_alloc_list, &aal_bus_type_p->alloc_list_head);
      kosal_sem_put(&aal_bus_type_p->alloc_list_sem);
   }

   // Initialize the device ID info
   DPRINTF(AALBUS_DBG_MOD," Initializing AAL device %p\n", paaldevice);
   paaldevice->m_devid = *devID;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(paaldevice, relfcnp);                  // User method


   // The Context is a PIP defined data value
   //   The standard AAL PIP pointer is stored as well
   paaldevice->m_pipContext = NULL;
   aaldev_pipp(paaldevice)  = ipipp;

   // Set the maximum number of shares
   paaldevice->m_maxowners = pRequest->maxshares;

   // Is direct access to PIP enabled for this device?
   paaldevice->m_mappableAPI = pRequest->enableDirectAPI;

   // Store the base name
   if ( 0 == strlen(pRequest->basename) ) {
      // Default
      strncpy(aaldev_basename(paaldevice), AAL_AFU_DEVICE_BASENAME, BUS_ID_SIZE-1);
   } else {
      strncpy(aaldev_basename(paaldevice), pRequest->basename,      BUS_ID_SIZE-1);
   }
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
   dev_set_name(&aaldev_to_basedev(paaldevice),
                "%s[%d:%d:%d]",
                (char*)aaldev_basename(paaldevice),
                aaldev_devaddr_busnum(paaldevice),
                aaldev_devaddr_devnum(paaldevice),
                aaldev_devaddr_subdevnum(paaldevice));
#else
   // Construct the device name from the base name and the device ID
   snprintf((char*)aaldev_devname(paaldevice), BUS_ID_SIZE,
            "%s[%d:%d:%d]",
            (char*)aaldev_basename(paaldevice),
            aaldev_devaddr_busnum(paaldevice),
            aaldev_devaddr_devnum(paaldevice),
            aaldev_devaddr_subdevnum(paaldevice));
#endif

   //---------------------------------------------------------
   // Register the device
   //  This will cause the bus's match function to be called.
   //  That call will be forwarded to this driver as it is
   //  registered with the aalBus.  The rest of initialization
   //  occurs there.
   //---------------------------------------------------------
   if ( pRequest->actionflags & AAL_DEV_CREATE_UNREGISTERED ) {

      // Stop here
      DPRINTF(AALBUS_DBG_MOD, ": AFU device created but not registered.  Action flags = %x\n",pRequest->actionflags);
      return paaldevice;

   } else if( 0 != aal_bus_p->register_device(paaldevice) ) {
      DPRINTF(AALBUS_DBG_MOD, ": AFU device failed to register!.\n");
      status = kosal_sem_get_user_alertable(&aal_bus_type_p->alloc_list_sem);
      if (0 != status) {
         DPRINTF(AALBUS_DBG_MOD, " interrupted while acquiring lock for"
            " allocation list, leaking memory as a result\n");
      } else {
         list_del(&paaldevice->m_alloc_list);
         kosal_sem_put(&aal_bus_type_p->alloc_list_sem);
         kfree(paaldevice);
      }
      return NULL;
   }

   return paaldevice;
}


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
   aaldev_owner_init(pown, pid, manifest);

   // Record the device
   pown->m_device = pdev;

   // Add the owner to the device's owner list.
   // This is the list of owners for this device
   list_add(&pown->m_ownerlist, &pdev->m_ownerlist);

   // Add this owner object to the session's list
   // This is a list of devices owned by this session
   list_add(&pown->m_devicelist, psessionlist);

   // Increment number of owners
   if ( MAFU_CONFIGURE_UNLIMTEDSHARES != pdev->m_numowners ) {
      pdev->m_numowners++;
   }

   // Send the configuration update event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerAdded,
                                              pid);

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

   list_del_init(&pown->m_devicelist);
   list_del_init(&pown->m_ownerlist);

   // done list update
   kfree(pown);

   // Decrement number of owners
   pdev->m_numowners--;

   // Send the configuration update event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerRemoved,
                                              pid);
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
   list_del(&pown->m_devicelist);

   // If the session info is being updated
   if( ( NULL != ownerSessionp ) &&
       ( ownerSessionp != &pown->m_sess) ) {
      ownerSess_Copy(&pown->m_sess, ownerSessionp);
   }

   // Add it to the new session owners list
   list_add(&pown->m_devicelist, psessionlist);

   DPRINTF (AALBUS_DBG_MOD, ": Done Updating Owner\n");

   // Send the configuration update event
   // TODO RIGHT NOW AFUs DON'T HAVE NECESSARILY HAVE A BUS SET.
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevOwnerUpdated,
                                              pid);

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

   DPRINTF(AALBUS_DBG_MOD, "Removing device %p\n", pdev);

   if ( unlikely( !aaldev_valid(pdev) ) ) {
	   DPRINTF(AALBUS_DBG_MOD, "Invalid device %p\n", pdev);
      return 0;
   }

   if ( unlikely( kosal_sem_get_user_alertable(&pdev->m_sem) ) ) {
	   DPRINTF(AALBUS_DBG_MOD, "kosal_sem_get_user_alertable interrupted\n");
      return 0;
   }

   // For now disallow destruction with non-zero owners.
   //  Eventually this should be changed to notify owners
   //  via a status update event to remove
   if ( unlikely( 0 != pdev->m_numowners ) ) {
	   DPRINTF(AALBUS_DBG_MOD, "Non-zero owner count : %u\n", pdev->m_numowners);
      ret = 0;
      goto out;
   }

   // Send an update configuration event
   aalbus_get_bus()->send_config_update_event(pdev,
                                              krms_ccfgUpdate_DevRemoved,
                                              0);

   DPRINTF(AALBUS_DBG_MOD, "Done removing %p\n", pdev);

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

#if 0
   if( unlikely( !aaldev_valid(pdev) ) ) {
      kosal_printk_level(KERN_ERR, "aaldev_initFailed\n");
      return 0;
   }
#endif
   //  Initialize interface
   pdev->i.addOwner        = aaldev_addOwner;
   pdev->i.isOwner         = aaldev_isOwner;
   pdev->i.removeOwner     = aaldev_removeOwner;
   pdev->i.updateOwner     = aaldev_updateOwner;
   pdev->i.getOwnerSession = aaldev_getOwnerSession;
   pdev->i.doForeachOwner  = aaldev_doForeachOwner;
   pdev->i.quiesce         = aaldev_quiesce;
   pdev->i.remove          = aaldev_remove;

   kosal_list_init(&pdev->m_ownerlist);
   kosal_mutex_init(&pdev->m_sem);
   kosal_mutex_init(&pdev->m_listsem);

   return 1;
}

