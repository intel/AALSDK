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
//***************************************************************************
//        FILE: aalbus-main.c
//     CREATED: 02/15/2008
//      AUTHOR: Joseph Grecco
//              Alvin Chen
//
// PURPOSE:  This file contains the main startup and shutdown code for the
//           Accelerator Abstraction Layer (AAL)
//           Accelerator Hardware Module bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/15/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 11/25/2008     HM       Large merge
// 01/04/2009     HM       Updated Copyright
// 02/26/2009     JG       Support for NULL match() Implemented probe
//                JG       Began dynamic config implementation
//                         Eliminated Bridge device
//                         Changed how register device works
//                         Began supporting more get_device flags
// 04/13/2009     JG       Added support for version kernel 2.6.27
// 10/01/2009     HM       Fixed deactivate hang found by XDI and Joe
// 10/01/2009     JG       Fixed a bug in unregister_device that caused
//                         a segfault due to acessing the device after
//                         destruction
// 10/09/2009     AC       Added active uevent sending feature.
// 02/11/2010     JG       Support for kernel 2.6.31
// 04/28/2010     HM       Added return value checks to kosal_sem_get_krnl_alertable()
//****************************************************************************
#include "aalsdk/kernel/kosal.h"
#include "aalbus-int.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/aalbus-device.h"

#define MODULE_FLAGS AALBUS_DBG_MOD

MODULE_VERSION    (DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR     (DRV_AUTHOR);
MODULE_LICENSE    (DRV_LICENSE);

//=============================================================================
// Driver Parameters
//=============================================================================


//
//  Declarations for module parameters - Enables parameter value passing from
//                                       insmod and permissions as seen from /sys
//

// debug flag with default values

btUnsignedInt debug = 0
#if 0
  | AALBUS_DBG_MOD
  | AALBUS_DBG_FILE
  | AALBUS_DBG_MMAP
  | AALBUS_DBG_IOCTL
#endif
;

#if   defined( __AAL_LINUX__ )
MODULE_PARM_DESC(debug, "debug level");
module_param    (debug, int, 0644);
#endif // __AAL_LINUX__

extern void aaldev_release_device(struct device *pdev);

// Declare standard entry points
static int
aalbus_init(void);

static
void
aalbus_exit(void);

module_init(aalbus_init);
module_exit(aalbus_exit);

//=============================================================================
// Prototypes
//=============================================================================
int
aalbus_register_class_device(struct aal_classdevice *classdevice);
void
aalbus_unregister_class_device(struct aal_classdevice *classdevice);

int
aalbus_register_device(struct aal_device *dev);
void
aalbus_unregister_device(struct aal_device *dev);

#if 0
int
aalbus_destroy_device(struct aal_device *devp);
struct aal_device *
aalbus_create_device(struct aal_device_id *devID,
                     void * manifest);
#endif

btInt
aalbus_init_driver(kosal_ownermodule      * ,
                   struct aal_driver      * ,
                   struct aal_classdevice * ,
                   const char             * ,
                   btInt );
int
aalbus_release_driver(struct aal_driver      * ,
                      struct aal_classdevice * );
int
aalbus_register_driver(struct aal_driver *driver);
void
aalbus_unregister_driver(struct aal_driver *driver);

int
aalbus_register_service_interface(struct aal_interface *pinterface);
int
aalbus_unregister_service_interface(struct aal_interface *pinterface);

int
aalbus_send_uevent(struct aal_device *dev, enum kobject_action act, char *env[]);

int
aalbus_validate_device(struct aal_device *);

//////////////////////////////////////////////////////////////////////////////////////////////
struct aal_interface *
aalbus_get_service_interface(btIID iid);
void
aalbus_release_service_interface(struct aal_interface *pinterface);
int
aalbus_has_interface(btIID iid);

int
aalbus_match(struct device* dev, struct device_driver *drv);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
int
aalbus_uevent(struct device *dev, struct kobj_uevent_env *env);
#else
int
aalbus_uevent(struct device *dev,
              char **envp,
              int num_envp,
              char *buffer,
              int buffer_size);
#endif

int
aalbus_probe(struct device * dev);
int
aalbus_remove(struct device * dev);
void
aalbus_shutdown(struct device * dev);
int
aalbus_suspend(struct device * dev, pm_message_t state);
int
aalbus_resume(struct device * dev);

// Class methods
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
   #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
   static
   int
   aalbus_class_uevent(struct class_device *dev,
                       struct kobj_uevent_env *env);
   #else
   static
   int
   aalbus_class_uevent(struct class_device *dev,
                       char **env,
                       int num_envp,
                       char *buffer,
                       int buffer_size);
   #endif
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
void
aalbus_class_device_release(struct device *);
#else
void
aalbus_class_device_release(struct class_device *);
#endif


void
aalbus_class_release(struct class *class);
void
aalbus_class_devrelease(struct device *dev);

struct aal_device *
aalbus_get_device(struct aal_device_id *devID, __u32 mask);
struct aal_device *
aalbus_handle_to_device(void *handle);

int
aalbus_register_config_update_handler(aalbus_event_config_update_t EventHandler,
                                      void * context);
int
aalbus_config_update_event(struct device *pdev, void *data);
int
aalbus_send_config_update_event(struct aal_device *pdev,
                                krms_cfgUpDate_e updatetype,
                                btPID pid);

//=============================================================================
//=============================================================================

static
ssize_t
aalbus_attrib_show_version(struct bus_type *bus, char *buf);

static
ssize_t
aalbus_attrib_show_debug(struct bus_type *bus, char *buf);
static
ssize_t
aalbus_attrib_store_debug(struct bus_type *bus, const char *buf, size_t size);

//----------------------
// Static bus attributes
//----------------------
BUS_ATTR(version,S_IRUGO,aalbus_attrib_show_version,NULL);
BUS_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP,aalbus_attrib_show_debug,aalbus_attrib_store_debug);

//---------------
// Bus Attributes
//---------------

//=============================================================================
// Name: aalbus_attrib_show_version
//=============================================================================
static
ssize_t
aalbus_attrib_show_version(struct bus_type *bus, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%s\n",DRV_VERSION));
}

//=============================================================================
// Name: aalbus_attrib_show_debug
//=============================================================================
static
ssize_t
aalbus_attrib_show_debug(struct bus_type *bus, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%d\n",debug));
}

//=============================================================================
// Name: aalbus_attrib_store_debug
//=============================================================================
static
ssize_t
aalbus_attrib_store_debug(struct bus_type *bus,
                          const char *buf,
                          size_t size)
{
   int temp = 0;
   sscanf(buf,"%d", &temp);
   // Check for valid setting
   if(temp & AALBUS_DBG_INVLID) {
      // Invalid debug flag
      printk (KERN_INFO DRV_NAME ": Ignoring invalid value (%d) for attribute [debug] \n", temp);
      return -1;
   }
   debug = temp;
   printk (KERN_INFO DRV_NAME ": Attribute change - Debug = %d\n", temp);
   return size;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                        Bus Module Public Interface
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//=============================================================================
// Name: amhe_bus_type
// Description: Static object presenting the Bus driver
//=============================================================================
static struct aal_bus_type aalBus =
{
   // Bus public interface
   .m_bus =
   {
      .m_version                       = AAL_BUS_VERSION,
      .INIT_FACT_CREATE_DEVICE         = aaldev_create_device,
      .INIT_FACT_DESTROY_DEVICE        = aaldev_destroy_device,
      .register_device                 = aalbus_register_device,
      .unregister_device               = aalbus_unregister_device,
      .get_device                      = aalbus_get_device,
      .handle_to_device                = aalbus_handle_to_device,
      .init_driver                     = aalbus_init_driver,
      .release_driver                  = aalbus_release_driver,
      .register_driver                 = aalbus_register_driver,
      .unregister_driver               = aalbus_unregister_driver,
      .register_class_device           = aalbus_register_class_device,
      .unregister_class_device         = aalbus_unregister_class_device,
      .register_service_interface      = aalbus_register_service_interface,
      .unregister_service_interface    = aalbus_unregister_service_interface,
      .get_service_interface           = aalbus_get_service_interface,
      .release_service_interface       = aalbus_release_service_interface,
      .has_interface                   = aalbus_has_interface,
      .register_config_update_handler  = aalbus_register_config_update_handler,
#if 0
      .walk_device_chain               = aalbus_walk_device_chain,
#endif
      .send_config_update_event        = aalbus_send_config_update_event,
//      .send_uevent                     = aalbus_send_uevent,
      .dev_is_valid                    = aalbus_validate_device,
   },

   .config_update_event_handler = {
      .EventHandler = NULL,
      .context      = NULL,
   },

   .m_bustype = {
      .name     = "aal",
      .match    = aalbus_match,
      .uevent   = aalbus_uevent,
      .probe    = aalbus_probe,
      .remove   = aalbus_remove,
      .shutdown = aalbus_shutdown,
      .suspend  = aalbus_suspend,
      .resume   = aalbus_resume,
   },

   .m_flags = 0,
};


//=============================================================================
// Name: aal_class
// Description: Class object for all AAL real and  virtual devices
//=============================================================================
struct class_info
{
   struct class m_class;
   btBool       m_isregistered;
};
static struct class_info aal_class =
{
   .m_class = {
      .name             = "aal",
      .owner            = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
      .uevent           = aalbus_class_uevent,
      .release          = aalbus_class_device_release,
#endif
      .class_release    = aalbus_class_release,
   },
   .m_isregistered = false,
};

//=============================================================================
// Name: aalbus_init
// Description: Initialization routine for the module
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static
int 
aalbus_init(void)
{
   int ret;

   kosal_printk_level(KERN_INFO, "Accelerator Abstraction Layer\n");
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_DESCRIPTION);
   kosal_printk_level(KERN_INFO, "-> Version %s\n",DRV_VERSION);
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_COPYRIGHT);

   /* initialize list heads and associated semaphores */
   kosal_list_init(&aalBus.m_servicelist);
   kosal_mutex_init(&aalBus.m_sem);
   kosal_list_init(&aalBus.alloc_list_head);
   kosal_mutex_init(&aalBus.alloc_list_sem);

   //
   // Register our bus subsystem with the kernel
   //
   ret = bus_register( &aalBus.m_bustype );
   if ( ret < 0 ) {
      kosal_printk_level(KERN_ERR, "Failed to register ret = %d\n", ret);
      goto ERROR;
   }
   aal_bus_type_set_registered(&aalBus);

   //----------------------------------
   // Create the class container object
   //----------------------------------
   ret = class_register(&aal_class.m_class);
   if ( 0 != ret ) {
      kosal_printk_level(KERN_ERR, "Failed to register Class ret = %d\n", ret);
      goto ERROR;
   }
   aal_class.m_isregistered = true;

   //------------------
   // Create attributes
   //------------------
   if ( bus_create_file(&aalBus.m_bustype, &bus_attr_version) ) {
      kosal_printk_level(KERN_ERR, "Failed to create version attribute - Unloading module\n");
      ret = -ENOMEM;
      goto ERROR;
   }

   if ( bus_create_file(&aalBus.m_bustype, &bus_attr_debug) ) {
      kosal_printk_level(KERN_ERR, "Failed to create debug attribute - Unloading module\n");
      ret = -ENOMEM;
      goto ERROR;
   }

   return 0;

ERROR:

   if ( aal_class.m_isregistered ) {
      class_unregister(&aal_class.m_class);
      aal_class.m_isregistered = false;
   }

   if ( aal_bus_type_is_registered(&aalBus) ) {
      bus_unregister(&aalBus.m_bustype);
      aal_bus_type_clr_registered(&aalBus);
   }

   return ret;
}
//=============================================================================
// Name: aalbus_exit
// Description: Removes device from filesystem and acpm registration
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static
void
aalbus_exit(void)
{
   kosal_printk_level(KERN_INFO, "<- AAL Bus exiting\n");

   if ( aal_class.m_isregistered ) {
      class_unregister(&aal_class.m_class);
      aal_class.m_isregistered = false;
   }

   if ( aal_bus_type_is_registered(&aalBus) ) {
      bus_unregister(&aalBus.m_bustype);
      aal_bus_type_clr_registered(&aalBus);
   }

   kosal_printk_level(KERN_INFO, "<- %s removed.\n", DRV_DESCRIPTION);
}


//=============================================================================
// Name: aalbus_get_bus
// Description: Returns a pointer to the bus object
// Interface: public export
// Inputs: none.
// Returns: Pointer to aalbus object
// Comments: Used by clients of the bus to get access to the bus interface
//=============================================================================
struct aal_bus *
aalbus_get_bus(void)
{
   return aal_bus_type_aal_busp(&aalBus);
}
//=============================================================================
// Name:  aalbus_match
// Description: Queries a driver to determine if it supports a device
// Interface: public
// Inputs: dev - device to examine
//         drv - driver to query
// Outputs: 1 - match.
// Comments:
//=============================================================================
int
aalbus_match(struct device *dev, struct device_driver *drv)
{
#if (1 == ENABLE_CANARIES)
   btBool valid;
   struct aal_driver *paal_driver = base_to_aaldrv(drv);

   ASSERT(dev);
   ASSERT(drv);
   if ( (NULL == dev) || (NULL == drv) ) {
      return 0;
   }

   canaries_are_valid(struct_aal_driver, paal_driver, valid);
   ASSERT(valid);
   if ( !valid ) {
      return 0;
   }
#endif // ENABLE_CANARIES

   if ( NULL != base_to_aaldrv(drv)->m_match ) {
      return base_to_aaldrv(drv)->m_match(base_to_aaldrv(drv),
                                          basedev_to_aaldev(dev));
   }
   return 0;
}

//=============================================================================
// Name:  aalbus_uevent
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
int
aalbus_uevent(struct device          *dev,
              struct kobj_uevent_env *env)
{
   PTRACEIN;
#if 0
   if ( NULL != basedev_to_aaldev(dev)->m_uevent ) {
      return basedev_to_aaldev(dev)->m_uevent(basedev_to_aaldev(dev), env);
   }
#endif
   return 0;
}
#else
int
aalbus_uevent(struct device *dev,
              char         **envp,
              int            num_envp,
              char          *buffer,
              int            buffer_size)
{
   kosal_printk_level(KERN_INFO, "Entered.\n");
   if ( NULL != basedev_to_aaldev(dev)->m_uevent ) {
      return basedev_to_aaldev(dev)->m_uevent(basedev_to_aaldev(dev), envp, num_envp, buffer, buffer_size);
   }
   return 0;
}
#endif

//=============================================================================
// Name:  aalbus_probe
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int
aalbus_probe(struct device *dev)
{
#if (1 == ENABLE_CANARIES)
   btBool valid;
   struct aal_driver *paal_driver = base_to_aaldrv(dev->driver);

   ASSERT(dev);
   if ( NULL == dev ) {
      return 0;
   }

   canaries_are_valid(struct_aal_driver, paal_driver, valid);
   ASSERT(valid);
   if ( !valid ) {
      return 0;
   }
#endif // ENABLE_CANARIES

   kosal_printk_level(KERN_INFO, "Entered.\n");
   if ( NULL != base_to_aaldrv(dev->driver)->m_probe ) {
      return base_to_aaldrv(dev->driver)->m_probe(basedev_to_aaldev(dev));
   } else {
      kosal_printk_level(KERN_INFO, "No probe function for matched driver.\n");
   }
   return 0;
}


//=============================================================================
// Name:  aalbus_remove
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int
aalbus_remove(struct device *dev)
{
   kosal_printk_level(KERN_INFO, "Entered.\n");
   return 0;
}

//=============================================================================
// Name:  aalbus_shutdown
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_shutdown(struct device *dev)
{
   PTRACEIN;
   return;
}

//=============================================================================
// Name:  aalbus_suspend
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int
aalbus_suspend(struct device *dev, pm_message_t state)
{
   PTRACEIN;
   return 0;
}


//=============================================================================
// Name:  aalbus_resume
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int
aalbus_resume(struct device *dev)
{
   PTRACEIN;
   return 0;
}

//=============================================================================
// Name:  aalbus_register_device
// Description:  This function registers a device with the core. It is
//               called by the bus level driver whenever a new device is
//               detected. This includes AHMs and AFUs in the case of a
//               reconfigure request. If there are drivers installed they will
//               be probed to determine if any support this device.
// Interface:
// Inputs: paaldev - device to register
// Outputs: none.
// Comments:
//=============================================================================
int
aalbus_register_device(struct aal_device *paaldev)
{
   struct update_config_parms parms;

   DPRINTF(AALBUS_DBG_MOD, "Registering device %s\n", aaldev_devname(paaldev));

   if ( unlikely( !aaldev_init(paaldev) ) ) {
      kosal_printk_level(KERN_ERR, "Device registration failed\n");
      return -1;
   }

   // Do not permit multiple registrations
   ASSERT(!aaldev_is_registered(paaldev));
   if ( aaldev_is_registered(paaldev) ) {
	   DPRINTF(AALBUS_DBG_MOD, "Device already registered with AAL Bus\n");
      return -1;
   }

   // kosal_sem_get_user_alertable(&aalBus.m_sem);
   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      kosal_printk_level(KERN_ERR, "kosal_sem_get_user_alertable interrupted\n");
      return -1;
   }

   aaldev_to_basedev(paaldev).bus = &aalBus.m_bustype;

   // Add this device to the bus' list of devices
   if (0 !=  kosal_sem_get_user_alertable(&aalBus.alloc_list_sem)) {
      DPRINTF(AALBUS_DBG_MOD, " Failed to acquire allocate list semaphore\n");
      return -1;
   } else {
      list_add(&paaldev->m_alloc_list, &aalBus.alloc_list_head);
      kosal_sem_put(&aalBus.alloc_list_sem);
   }

   paaldev->m_dev.release = aaldev_release_device; // System method

   // device_register(&AALDEVBASE(dev));
   if ( device_register(&aaldev_to_basedev(paaldev)) ) {
      PERR("device_register() failed; could not register device %s\n", paaldev->m_basename);
      kosal_sem_put(&aalBus.m_sem);
      return -1;
   }


   // Generate a config update event TODO - This should move to Device
   if ( aalBus.config_update_event_handler.EventHandler ) {
      parms.Handler    = aalBus.config_update_event_handler.EventHandler;
      parms.updateType = krms_ccfgUpdate_DevAdded;
      parms.pid        = 0;
      aalbus_config_update_event(&aaldev_to_basedev(paaldev), &parms);
   }

   aaldev_set_registered(paaldev);
   kosal_sem_put(&aalBus.m_sem);

   DPRINTF(AALBUS_DBG_MOD,  "Done registering device %s\n",aaldev_devname(paaldev));

   return 0;
}

//=============================================================================
// Name:  aalbus_unregister_device
// Description: Unregisters a device from the core.  Issued by the bus driver
//              when an AHM or AFU are taken off line or removed.
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_unregister_device(struct aal_device *dev)
{
#if (1 == ENABLE_DEBUG)
   if( &aalBus.m_bustype == aaldev_to_basedev(dev).bus ){
      kosal_printk_level(KERN_INFO, "Unregistering AAL device \"%s\"\n", aaldev_devname(dev));
   }
#endif // ENABLE_DEBUG

   // Do not unregister devices not registered
   ASSERT(aaldev_is_registered(dev));
   if ( !aaldev_is_registered(dev) ) {
      kosal_printk_level(KERN_WARNING, "Device not registered with AAL Bus\n");
      return;
   }

   // Remove the device. This causes the device to signal its
   //  removal from the system
   dev_removedevice(dev);

   // Begin the process of unplugging the device from the framework
   //  the device_unregister() causes the Linux DD framework to generate
   //  the device::release() callback to be invoked. This is where the device
   //  specifc clean-up will occur.  Once the device_register() returns assume
   //  dev is deleted.
   device_unregister(&aaldev_to_basedev(dev));
   aaldev_clr_registered(dev);

   kosal_printk_level(KERN_INFO, "Done unregistering device\n");
   return;
}


#if (0 && (1 == ENABLE_DEBUG))

static
struct aal_interface *
debug_hook_get_interface(struct aal_driver *drv, btIID iid)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(drv=0x%p, iid=0x%llx)\n",
              __AAL_FUNC__, drv, iid);
   return NULL;
}
static
int
debug_hook_has_interface(struct aal_driver *drv, btIID iid)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(drv=0x%p, iid=0x%llx)\n",
              __AAL_FUNC__, drv, iid);
   return 0;
}
static
int
debug_hook_supports_interface(struct aal_driver *drv, btIID iid)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(drv=0x%p, iid=0x%llx)\n",
              __AAL_FUNC__, drv, iid);
   return 0;
}
static
int
debug_hook_match(struct aal_driver *drv, struct aal_device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(drv=0x%p, dev=0x%p)\n",
              __AAL_FUNC__, drv, dev);
   return 0;
}
static
int
debug_hook_probe(struct aal_device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(dev=0x%p)\n",
              __AAL_FUNC__, dev);
   return 0;
}
static
unsigned int
debug_hook_fops_poll(struct file *f, struct poll_table_struct *pt)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct file *=0x%p, struct poll_table_struct *=0x%p)\n",
              __AAL_FUNC__, f, pt);
   return 0;
}
static
long
debug_hook_fops_unlocked_ioctl(struct file *f, unsigned int a, unsigned long b)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct file *=0x%p, unsigned int=%u, unsigned long=%lu)\n",
              __AAL_FUNC__, f, a, b);
   return 0;
}
static
int
debug_hook_fops_mmap(struct file *f, struct vm_area_struct *vma)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct file *=0x%p, struct vm_area_struct *=0x%p)\n",
              __AAL_FUNC__, f, vma);
   return 0;
}
static
int
debug_hook_fops_open(struct inode *inod, struct file *f)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct inode *=0x%p, struct file *=0x%p)\n",
              __AAL_FUNC__, inod, f);
   return 0;
}
static
int
debug_hook_fops_release(struct inode *inod, struct file *f)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct inode *=0x%p, struct file *=0x%p)\n",
              __AAL_FUNC__, inod, f);
   return 0;
}

static
void
aalbus_driver_debug_hook_fops(struct file_operations *fops)
{
   if ( NULL == fops->poll ) {
      fops->poll = debug_hook_fops_poll;
      DPRINTF(AALBUS_DBG_MOD, " hooked fops->poll\n");
   }
   if ( NULL == fops->unlocked_ioctl ) {
      fops->unlocked_ioctl = debug_hook_fops_unlocked_ioctl;
      DPRINTF(AALBUS_DBG_MOD, " hooked fops->unlocked_ioctl\n");
   }
   if ( NULL == fops->mmap ) {
      fops->mmap = debug_hook_fops_mmap;
      DPRINTF(AALBUS_DBG_MOD, " hooked fops->mmap\n");
   }
   if ( NULL == fops->open ) {
      fops->open = debug_hook_fops_open;
      DPRINTF(AALBUS_DBG_MOD, " hooked fops->open\n");
   }
   if ( NULL == fops->release ) {
      fops->release = debug_hook_fops_release;
      DPRINTF(AALBUS_DBG_MOD, " hooked fops->release\n");
   }
}

static
int
debug_hook_dd_probe(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p)\n",
              __AAL_FUNC__, dev);
   return 0;
}
static
int
debug_hook_dd_remove(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p)\n",
              __AAL_FUNC__, dev);
   return 0;
}
static
void
debug_hook_dd_shutdown(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p)\n",
              __AAL_FUNC__, dev);
}
static
int
debug_hook_dd_suspend(struct device *dev, pm_message_t state)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p, pm_message_t)\n",
              __AAL_FUNC__, dev);
   return 0;
}
static
int
debug_hook_dd_resume(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p)\n",
              __AAL_FUNC__, dev);
   return 0;
}

static
void
aalbus_driver_debug_hook_device_driver(struct device_driver *dd)
{
   if ( NULL == dd->probe ) {
      dd->probe = debug_hook_dd_probe;
      DPRINTF(AALBUS_DBG_MOD, " hooked dd->probe\n");
   }
   if ( NULL == dd->remove ) {
      dd->remove = debug_hook_dd_remove;
      DPRINTF(AALBUS_DBG_MOD, " hooked dd->remove\n");
   }
   if ( NULL == dd->shutdown ) {
      dd->shutdown = debug_hook_dd_shutdown;
      DPRINTF(AALBUS_DBG_MOD, " hooked dd->shutdown\n");
   }
   if ( NULL == dd->suspend ) {
      dd->suspend = debug_hook_dd_suspend;
      DPRINTF(AALBUS_DBG_MOD, " hooked dd->suspend\n");
   }
   if ( NULL == dd->resume ) {
      dd->resume = debug_hook_dd_resume;
      DPRINTF(AALBUS_DBG_MOD, " hooked dd->resume\n");
   }
}

static
void
aalbus_driver_debug_hook_class_device_release(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD, "FAILED %s(struct device *=0x%p)\n",
              __AAL_FUNC__, dev);
}

static
void
aalbus_driver_debug_hook_class_device(struct aal_driver      *pdrv,
                                      struct aal_classdevice *pclassdev)
{
   int    i;
   btIID *devid;

   if ( NULL == pclassdev ) {
      return;
   }

   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_flags=0x%x\n", pclassdev->m_flags);
   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classid.m_majorversion=0x%x\n", pclassdev->m_classid.m_majorversion);
   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classid.m_minorversion=0x%x\n", pclassdev->m_classid.m_minorversion);
   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classid.m_releaseversion=0x%llx\n", pclassdev->m_classid.m_releaseversion);
   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classid.m_classGUID=0x%llx\n", pclassdev->m_classid.m_classGUID);

   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_devIIDlist=0x%p\n", pclassdev->m_devIIDlist);
   if ( NULL != pclassdev->m_devIIDlist ) {
      i = 0;
      devid = pclassdev->m_devIIDlist;
      while ( *devid ) {
         DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_devIIDlist[%d]=%llx\n", i, *devid);
         ++i;
         ++devid;
      }
   }


   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_bus=0x%p\n", pclassdev->m_bus);
/*
   if ( NULL == pclassdev->m_bus ) {
      pclassdev->m_bus = &aalBus.m_bus;
      //pclassdev->m_bus = aaldrv_aalbusp(pdrv);
      DPRINTF(AALBUS_DBG_MOD, " patched NULL pclassdev->m_bus (0x%p)\n", pclassdev->m_bus);
   }
*/

   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classdev.bus=0x%p\n", pclassdev->m_classdev.bus);
/*
   if ( NULL == pclassdev->m_classdev.bus ) {
      pclassdev->m_classdev.bus = &aalBus.m_bustype;
      //pclassdev->m_classdev.bus = aaldrv_to_base_busp(pdrv);
      DPRINTF(AALBUS_DBG_MOD, " patched NULL pclassdev->m_classdev.bus (0x%p)\n", pclassdev->m_classdev.bus);
   }
*/

   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classdev.driver=0x%p\n", pclassdev->m_classdev.driver);
/*
   if ( NULL == pclassdev->m_classdev.driver ) {
      pclassdev->m_classdev.driver = aaldrv_to_basep(pdrv);
      DPRINTF(AALBUS_DBG_MOD, " patched NULL pclassdev->m_classdev.driver (0x%p)\n", pclassdev->m_classdev.driver);
   }
*/

   DPRINTF(AALBUS_DBG_MOD, " pclassdev->m_classdev.driver=0x%p\n", pclassdev->m_classdev.driver);
/*
   if ( NULL == pclassdev->m_classdev.init_name ) {
      pclassdev->m_classdev.init_name = "patched";
      DPRINTF(AALBUS_DBG_MOD, " patched NULL pclassdev->m_classdev.init_name (\"%s\")\n", pclassdev->m_classdev.init_name);
   }
*/
//   pclassdev->m_classdev.release = aalbus_driver_debug_hook_class_device_release;
}

static
void
aalbus_driver_debug_hook(struct module          *powner,
                         struct aal_driver      *pdrv,
                         struct aal_classdevice *pclassdev,
                         const char             *devname,
                         int                     devmajor)
{
   const struct aal_device_id *devid;
   int                         i;

   DPRINTF(AALBUS_DBG_MOD, " Entering %s(powner=0x%p, pdrv=0x%p, pclassdev=0x%p, devname=\"%s\", devmajor=%d)\n",
              __AAL_FUNC__,
              powner,
              pdrv,
              pclassdev,
              (NULL == devname) ? "<null>" : devname,
              devmajor
              );

   ASSERT(powner);
   if ( NULL == powner ) {
      return;
   }

   ASSERT(pdrv);
   if ( NULL == pdrv ) {
      return;
   }

   DPRINTF(AALBUS_DBG_MOD, " pdrv->m_flags=0x%x\n", pdrv->m_flags);

   if ( NULL == pdrv->get_interface ) {
      pdrv->get_interface = debug_hook_get_interface;
      DPRINTF(AALBUS_DBG_MOD, " hooked get_interface\n");
   }
   if ( NULL == pdrv->has_interface ) {
      pdrv->has_interface = debug_hook_has_interface;
      DPRINTF(AALBUS_DBG_MOD, " hooked has_interface\n");
   }
   if ( NULL == pdrv->supports_interface ) {
      pdrv->supports_interface = debug_hook_supports_interface;
      DPRINTF(AALBUS_DBG_MOD, " hooked supports_interface\n");
   }

   aalbus_driver_debug_hook_fops(&pdrv->m_fops);

   //struct cdev                 m_cdev;  // character device

   DPRINTF(AALBUS_DBG_MOD, " pdrv->m_major=%d\n", pdrv->m_major);
   DPRINTF(AALBUS_DBG_MOD, " pdrv->m_iids=0x%p\n", pdrv->m_iids);

   DPRINTF(AALBUS_DBG_MOD, " pdrv->m_idtable=0x%p\n", pdrv->m_idtable);
   if ( NULL != pdrv->m_idtable ) {
      devid = pdrv->m_idtable;
      i = 0;
      while ( devid->m_devaddr.m_bustype ) {
         DPRINTF(AALBUS_DBG_MOD, " devid[%d].m_vendor=0x%x\n", i, devid->m_vendor);
         ++i;
         ++devid;
      }
   }

   if ( NULL == pdrv->m_match ) {
      pdrv->m_match = debug_hook_match;
      DPRINTF(AALBUS_DBG_MOD, " hooked m_match\n");
   }
   if ( NULL == pdrv->m_probe ) {
      pdrv->m_probe = debug_hook_probe;
      DPRINTF(AALBUS_DBG_MOD, " hooked m_probe\n");
   }

   DPRINTF(AALBUS_DBG_MOD, " pdrv->m_bus=0x%p\n", pdrv->m_bus);

   aalbus_driver_debug_hook_device_driver(&pdrv->m_driver);

   aalbus_driver_debug_hook_class_device(pdrv, pclassdev);

   DPRINTF(AALBUS_DBG_MOD, " Leaving %s()\n", __AAL_FUNC__);
}

#endif // ENABLE_DEBUG


//=============================================================================
// Name:  aalbus_init_driver
// Description:
//
//
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Initializes all of the driver structures
//=============================================================================
btInt
aalbus_init_driver(kosal_ownermodule      *powner,
                   struct aal_driver      *pdrv,
                   struct aal_classdevice *pclassdev,
                   const char             *devname,
                   btInt                   devmajor)
{
   int             res     = 0;
   dev_t           devtype = 0;
   struct aal_bus *pbus;

   ASSERT(powner);
   if ( NULL == powner ) {
      PERR("Got NULL struct module *\n");
      return -EINVAL;
   }

   ASSERT(pdrv);
   if ( NULL == pdrv ) {
      PERR("Got NULL struct aal_driver *\n");
      return -EINVAL;
   }

   if ( NULL == aaldrv_to_base(pdrv).name ) {
      ASSERT(devname);
      if ( NULL == devname ) {
         PERR("No name in struct device_driver and NULL devname.\n");
         return -EINVAL;
      }

      aaldrv_to_base(pdrv).name = devname;
   }

   if ( NULL != pclassdev ) {
      ASSERT(devname);
      if ( NULL == devname ) {
         PERR("Class device 0x%p requested, but devname is NULL.\n", pclassdev);
         return -EINVAL;
      }
   }

   pbus = aalbus_get_bus();
   ASSERT(pbus);
   if ( NULL == pbus ) {
      return -EFAULT;
   }

   kosal_list_init(&pdrv->m_sesslist);

   if ( NULL != devname ) {
      // char device node requested - get a major device number

      //  use the value passed at load time or allocate dynamically
      if ( 0 != devmajor ) {
         // major number requested at load time
         PDEBUG("Registering static major number %d for \"%s\"\n",
                   devmajor, devname);

         devtype = MKDEV(devmajor, 0);
         res = register_chrdev_region(devtype, 1, devname);

         if ( res < 0 ) {
            PERR("Failed to register major device number %d for \"%s\"\n",
                    devmajor, devname);
            return res;
         }
      } else {
         // Dynamically allocate the major number
         PDEBUG("Allocating major number for \"%s\"\n", devname);

         res = alloc_chrdev_region(&devtype, 0, 1, devname);

         if ( res < 0 ) {
            PERR("Failed to allocate major device number for \"%s\"\n", devname);
            return res;
         }

         aaldrv_dev_major(pdrv) = MAJOR(devtype);
         PDEBUG("Using major number %d for \"%s\"\n", aaldrv_dev_major(pdrv), devname);
      }

      aaldrv_set_chrdev_region_obtained(pdrv);
   }


   // Register the driver with the AALbus
   aaldrv_fops(pdrv).owner    = (struct module *)powner;
   aaldrv_to_base(pdrv).owner = (struct module *)powner;

   res = pbus->register_driver(pdrv);
   if ( res < 0 ) {
      goto ERROR;
   }

   if ( NULL != devname ) {
      // Add the char device

      cdev_init(aaldrv_cdevp(pdrv), aaldrv_fopsp(pdrv));
      aaldrv_cdevp(pdrv)->ops   = aaldrv_fopsp(pdrv);
      aaldrv_cdevp(pdrv)->owner = (struct module *)powner;

      res = cdev_add(aaldrv_cdevp(pdrv), devtype, 1);
      if ( res ) {
         PERR("Failed to register character device : ret = %d\n", res);
         goto ERROR;
      }

      aaldrv_set_chrdev_added(pdrv);
   }


   // Register the class device, if any.
   if ( NULL != pclassdev ) {

      aal_classdev_set_name(pclassdev, devname);
      aal_classdev_devtype(pclassdev) = devtype;

      PDEBUG("Registering class \"%s\"\n", aal_classdev_get_name(pclassdev));

      res = pbus->register_class_device(pclassdev);
      if ( res < 0 ) {
         goto ERROR;
      }
   }

#if (0 && (1 == ENABLE_DEBUG))
   aalbus_driver_debug_hook(powner, pdrv, pclassdev, devname, devmajor);
#endif

   return res;

ERROR:

   if ( NULL != pclassdev ) {
      if ( aal_classdev_is_registered(pclassdev) ) {
         pbus->unregister_class_device(pclassdev);
      }
   }

   if ( aaldrv_is_chrdev_added(pdrv) ) {
      cdev_del(aaldrv_cdevp(pdrv));
      aaldrv_clr_chrdev_added(pdrv);
   }

   if ( aaldrv_is_registered(pdrv) ) {
      pbus->unregister_driver(pdrv);
   }

   if ( aaldrv_is_chrdev_region_obtained(pdrv) ) {
      unregister_chrdev_region(devtype, 1);
      aaldrv_clr_chrdev_region_obtained(pdrv);
   }

   return res;
}

//=============================================================================
// Name:  aalbus_release_driver
// Description:
//
//
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Initializes all of the driver structures
//=============================================================================
int
aalbus_release_driver(struct aal_driver      *pdrv,
                      struct aal_classdevice *pclassdev)
{
   int             devmajor;
   dev_t           devtype;
   struct aal_bus *pbus;

   ASSERT(pdrv);
   if ( NULL == pdrv ) {
      PERR("Got NULL struct aal_driver *\n");
      return -EINVAL;
   }

   pbus = aalbus_get_bus();
   ASSERT(pbus);
   if ( NULL == pbus ) {
      return -EFAULT;
   }

   if ( NULL != pclassdev ) {
      if ( aal_classdev_is_registered(pclassdev) ) {
         pbus->unregister_class_device(pclassdev);
      }
   }

   if ( aaldrv_is_chrdev_added(pdrv) ) {
      cdev_del(aaldrv_cdevp(pdrv));
      aaldrv_clr_chrdev_added(pdrv);
   }

   if ( aaldrv_is_registered(pdrv) ) {
      pbus->unregister_driver(pdrv);
   }

   if ( aaldrv_is_chrdev_region_obtained(pdrv) ) {
      devmajor = aaldrv_dev_major(pdrv);
      devtype  = MKDEV(devmajor, 0);
      unregister_chrdev_region(devtype, 1);
      aaldrv_clr_chrdev_region_obtained(pdrv);
   }

   return 0;
}

//=============================================================================
// Name:  aalbus_register_driver
// Description:  Called by a device driver to register with the bus.  If there
//               are any devices installed they will be probed to determine if
//               the driver supports them.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Initializes all of the driver structures
//=============================================================================
int
aalbus_register_driver(struct aal_driver *driver)
{
   int ret = 0;
#if (1 == ENABLE_CANARIES)
   btBool valid;
   canaries_are_valid(struct_aal_driver, driver, valid);
   ASSERT(!valid);
   canaries_init(struct_aal_driver, driver);
#endif // ENABLE_CANARIES

   DPRINTF(AALBUS_DBG_MOD, ": Registering device driver\n");

   ASSERT(!aaldrv_is_registered(driver));
   if ( aaldrv_is_registered(driver) ) {
      return -EINVAL;
   }

   if ( NULL == driver->m_match ) {
	   DPRINTF(AALBUS_DBG_MOD, ": No match method\n");
   }
   // Point driver at bus. Bus performs canonical methods for driver
   aaldrv_to_base_busp(driver) = &aalBus.m_bustype;
   aaldrv_aalbusp(driver)      = &aalBus.m_bus;

   // Register driver with system.  Will cause devices to be probed
   ret = driver_register(aaldrv_to_basep(driver));
   if ( ret < 0 )   {
	   DPRINTF(AALBUS_DBG_MOD, ": Driver_registration failed!\n");
      return ret;
   }

   aaldrv_set_is_registered(driver);
   return 0;
}

//=============================================================================
// Name:  aalbus_unregister_driver
// Description: Called by a device driver to unregister
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_unregister_driver(struct aal_driver *driver)
{
#if (1 == ENABLE_CANARIES)
   btBool valid;
   canaries_are_valid(struct_aal_driver, driver, valid);
   ASSERT(valid);
#endif // ENABLE_CANARIES

   DPRINTF(AALBUS_DBG_MOD, ": Unregistering device driver\n");

   ASSERT(aaldrv_is_registered(driver));
   if ( aaldrv_is_registered(driver) ) {
      driver_unregister(aaldrv_to_basep(driver));
      aaldrv_clr_is_registered(driver);
   }

   DPRINTF(AALBUS_DBG_MOD,  "Done Unregistering device driver\n");

#if (1 == ENABLE_CANARIES)
   canaries_clear(struct_aal_driver, driver);
#endif
}

//=============================================================================
// Name:  aalbus_register_class_device
// Description: Called by drivers such as interface drivers to create class
//              devices
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Initializes all of the driver structures
//=============================================================================
int
aalbus_register_class_device(struct aal_classdevice *pclassdevice)
{
   int ret = 0;

   ASSERT(!aal_classdev_is_registered(pclassdevice));
   if ( aal_classdev_is_registered(pclassdevice) ) {
      return -EINVAL;
   }

   // Point class device at AAL class.
   pclassdevice->m_classdev.class = &aal_class.m_class;

   // Register driver with system.  Will cause devices to be probed
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)

   //Install the default release handler if one has not been specified
   if ( NULL == aal_classdevp_to_dev(pclassdevice).release ) {
      aal_classdevp_to_dev(pclassdevice).release = aalbus_class_device_release;
   }
   ret = device_register(&pclassdevice->m_classdev);
#else
   DPRINTF(AALBUS_DBG_MOD, ": Registering class device %s.\n",pclassdevice->m_classdev.class_id);
   ret = class_device_register(&pclassdevice->m_classdev);
#endif
   if( ret < 0 )   {
      DPRINTF(AALBUS_DBG_MOD, ": Class device registration failed!\n");
      return ret;
   }

   aal_classdev_set_registered(pclassdevice);
   return 0;
}

//=============================================================================
// Name:  aalbus_unregister_class_device
// Description: Called by a drivers to unregister class devices
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_unregister_class_device(struct aal_classdevice *pclassdevice)
{
   DPRINTF(AALBUS_DBG_MOD,  "Unregistering class device driver\n");

   ASSERT(aal_classdev_is_registered(pclassdevice));
   if ( aal_classdev_is_registered(pclassdevice) ) {

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
      device_unregister(&pclassdevice->m_classdev);
#else
      class_device_unregister(&pclassdevice->m_classdev);
#endif

      aal_classdev_clr_registered(pclassdevice);
   }
   return;
}

//=============================================================================
// Name:  aalbus_send_uevent
// Description: 
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
int
aalbus_send_uevent(struct aal_device *dev, enum kobject_action act, char *env[])
{
   DPRINTF(AALBUS_DBG_MOD, "in.\n");

   return kobject_uevent_env(&dev->m_dev.kobj, act, env);
}
#else
int
aalbus_send_uevent(struct aal_device *dev, enum kobject_action act, char *env[])
{
   DPRINTF(AALBUS_DBG_MOD, "Not implemented in.\n");

   return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
//=============================================================================
// Name:  aalbus_class_uevent
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
int aalbus_class_uevent(struct class_device    *dev,
                        struct kobj_uevent_env *env)
#else
int aalbus_class_uevent(struct class_device *dev,
                        char               **env,
                        int                  num_envp,
                        char                *buffer,
                        int                  buffer_size)

#endif

{
   DPRINTF(AALBUS_DBG_MOD, ": Entered\n");
   return 0;
}

#endif

//=============================================================================
// Name:  aalbus_class_device_release
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
void
aalbus_class_device_release(struct device *dev)
#else
void
aalbus_class_device_release(struct class_device *dev)
#endif
{
   DPRINTF(AALBUS_DBG_MOD,  "Default class device release handler\n");
   return;
}

//=============================================================================
// Name:  aalbus_class_release
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_class_release(struct class *class)
{
   DPRINTF(AALBUS_DBG_MOD,  "Entered\n");
   return;
}

//=============================================================================
// Name: aalbus_class_devrelease
// Description:
// Interface:
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_class_devrelease(struct device *dev)
{
   DPRINTF(AALBUS_DBG_MOD,  "Entered\n");
   return;
}

//=============================================================================
// Name: aalbus_register_service_interface
// Description: Registers a service interface with the bus
// Interface: public
// Inputs: pinterface - interface to register
//         iid - interface ID.
// Outputs: 0 - successful
// Comments: This function does not increment the ownind modufes refcnt. The
//           interface ID must be unique to the service list.
//=============================================================================
int
aalbus_register_service_interface(struct aal_interface *pinterface)
{
   int res;

   DPRINTF(AALBUS_DBG_MOD,  "Registering service interface 0x%" PRIx64 "\n", pinterface->m_iid);

   ASSERT(!aalinterface_is_registered(pinterface));

   // aal_interface_add() determines whether it has already been added.
   res = aal_interface_add(&aalBus.m_servicelist, &aalBus.m_sem, pinterface);
   if ( 0 == res ) {
      aalinterface_set_registered(pinterface);
   }
   return res;
}

//=============================================================================
// Name: aalbus_unregister_service_interface
// Description: Unregisters a service interface with the bus
// Interface: public
// Inputs: pinterface - interface to register
//         iid - interface ID.
// Outputs: 0 - successful
// Comments: This function does not increment the ownind modufes refcnt. The
//           interface ID must be unique to the service list.
//=============================================================================
int
aalbus_unregister_service_interface(struct aal_interface *pinterface)
{
   int res;

   DPRINTF(AALBUS_DBG_MOD,  "Unregistering service interface 0x%Lx\n", pinterface->m_iid);

   ASSERT(aalinterface_is_registered(pinterface));

   // aal_interface_del() determines whether it has already been added.
   res = aal_interface_del(&aalBus.m_servicelist, &aalBus.m_sem, pinterface);
   if ( 0 == res ) {
      aalinterface_clr_registered(pinterface);
   }
   return res;
}


//=============================================================================
// Name: aalbus_get_service_interface
// Description: Aquires a service interface and increments the modules refcnt
// Interface: public
// Inputs: iid - ID of the interface to get
// Outputs: NULL - if failure
// Comments: This function increments the refcn of the module that owns the
//           interface. This will prevent the module from being released while
//           another module may be using the interface. It is important that
//           the interface be released using aalbus_release_service_interface()
//           to allow the service module to eventually be unloaded.
//=============================================================================
struct aal_interface *
aalbus_get_service_interface(btIID iid)
{
   struct aal_interface *pinterface =
         aal_interface_find(&aalBus.m_servicelist, iid);

   if ( NULL == pinterface ) {
      return NULL; // Interface not found.
   }

   // Try the module to make sure it is not getting released
   if ( !try_module_get(pinterface->m_owner) ) {
      return NULL;
   }

   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      ASSERT(false);
      return pinterface;
   }
   aalinterface_get(pinterface);
   kosal_sem_put(&aalBus.m_sem);

   return pinterface;
}

//=============================================================================
// Name: aalbus_release_service_interface
// Description: Releases a service interface and decrements the owning module's
//              refcnt.
// Interface: public
// Inputs:  pinterface - interface to release.
// Outputs: none.
// Comments:
//=============================================================================
void
aalbus_release_service_interface(struct aal_interface *pinterface)
{
   DPRINTF(AALBUS_DBG_MOD,  "Releasing service interface 0x%Lx\n", pinterface->m_iid);

   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      ASSERT(false);
      module_put(pinterface->m_owner);
      return;
   }

   ASSERT(aalinterface_count(pinterface) > 0);
   if ( aalinterface_count(pinterface) > 0 ) {
      aalinterface_put(pinterface);
   }
   kosal_sem_put(&aalBus.m_sem);

   module_put(pinterface->m_owner);
}

//=============================================================================
// Name: aalbus_has_interface
// Description: Checks to see if an interface has been registered
// Interface: public
// Inputs: iid - ID of interface.
// Outputs: 1 if interface is available.
// Comments:
//=============================================================================
int
aalbus_has_interface(btIID iid)
{
   return (NULL != aal_interface_find(&aalBus.m_servicelist, iid));
}


//=============================================================================
// Name: device_find_param
// Description: Parameter used in get_device search functions
//=============================================================================
struct device_find_param {
   __u32                  m_mask;      // Input
   struct aal_device_id  *m_devID;     // Input
   struct device         *m_dev;       // Output
};

//=============================================================================
// Name: aalbus_itr_get_device
// Description: Returns next found device
// Interface: public
// Inputs: pdev       - generic device structure
//         data       - pointer to the device find param
//         mask       - fields in the id to examine
// Outputs: 1 - if present
// Comments:
//=============================================================================
static
int
aalbus_itr_get_device(struct device *pdev, void *data)
{
   struct device_find_param *param = (struct device_find_param *)data;
   struct aal_device        *paal  = basedev_to_aaldev(pdev);
   __u32 mask = param->m_mask;

   // Determine if this is the device being looked for
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
   DPRINTF(AALBUS_DBG_MOD, ": Checking device %s  mask %x paal %p\n", (char*)dev_name(pdev),param->m_mask, paal);
#else
   DPRINTF(AALBUS_DBG_MOD, ": Checking device %s  mask %x paal %p\n", pdev->bus_id,param->m_mask, paal);
#endif

   //---------------------------------------------------
   // Loop examine device and all of its children
   //  If a mask was set then see if the device matches
   //  all of the attributes specified. If any fail then
   //  no match. Null mask means exact match.
   //---------------------------------------------------
   if ( mask ) {
      // Optimization for common case
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_ADDR\n");
      if( mask & AAL_DEV_ID_MASK_ADDR){
         if( memcmp( &param->m_devID->m_devaddr, &aaldev_devaddr(paal), sizeof( struct aal_device_addr ) ) != 0 ){
            DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_ADDR\n");
            return 0;
         }
      } // if( mask & AAL_DEV_ID_MASK_ADDR)

      // Match AHM GUID
      DPRINTF(AALBUS_DBG_MOD, ": check AHMGUID\n");
      if( mask & AAL_DEV_ID_MASK_AHMGUID){
        if(param->m_devID->m_ahmGUID != aaldev_devid(paal).m_ahmGUID){
            DPRINTF(AALBUS_DBG_MOD, ": No match on AHMGUID\n");
            return 0;
         }
      }

      // Match the Board number
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_BUSNUM\n");
      if( (mask & AAL_DEV_ID_MASK_BUSNUM)){
          if(param->m_devID->m_devaddr.m_busnum != aaldev_devaddr_busnum(paal)){
             DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_BUSNUM\n");
             return 0;
           }
      }

      // Match the Board number
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_DEVNUM\n");
      if( (mask & AAL_DEV_ID_MASK_DEVNUM)){
          if(param->m_devID->m_devaddr.m_devicenum != aaldev_devaddr_devnum(paal)){
             DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_DEVNUM\n");
             return 0;
           }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AHMGUID\n");
      if( mask & AAL_DEV_ID_MASK_SUBDEVNUM){
            if(param->m_devID->m_devaddr.m_subdevnum != aaldev_devaddr_subdevnum(paal)){
               DPRINTF(AALBUS_DBG_MOD, ": No match on AHMGUID\n");
               return 0;
             }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_BUSTYPE\n");
      if( (mask & AAL_DEV_ID_MASK_BUSTYPE)){
          if(param->m_devID->m_devaddr.m_bustype != aaldev_devaddr_bustype(paal)){
             DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_BUSTYPE\n");
             return 0;
          }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AHMGUID\n");
      if( (mask & AAL_DEV_ID_MASK_DEVTYPE)){
          if(param->m_devID->m_devicetype != aaldev_devid_devtype(paal)){
             DPRINTF(AALBUS_DBG_MOD, ": No match on AHMGUID\n");
             return 0;
          }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_AHMGUID\n");
      if( mask & AAL_DEV_ID_MASK_AHMGUID){
        if(param->m_devID->m_ahmGUID != aaldev_devid_ahmguid(paal)){
           DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_AHMGUID\n");
           return 0;
         }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_AFUGUID\n");
      if( mask & AAL_DEV_ID_MASK_AFUGUID){
        if(param->m_devID->m_afuGUID != aaldev_devid_afuguid(paal)){
           DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_AFUGUID\n");
            return 0;
         }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_PIPGUID\n");
      if( mask & AAL_DEV_ID_MASK_PIPGUID){
        if(param->m_devID->m_pipGUID != aaldev_devid_pipguid(paal)){
            DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_PIPGUID\n");
            return 0;
         }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_DEVID\n");
      if( mask & AAL_DEV_ID_MASK_DEVID){
          if(memcmp(&param->m_devID, &aaldev_devid(paal), sizeof(struct aal_device_id) ) !=0){
            DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_DEVID\n");
            return 0;
          }
      }
      DPRINTF(AALBUS_DBG_MOD, ": check AAL_DEV_ID_MASK_VENDOR\n");
      // Check for match of unmasked fields
      if( mask & AAL_DEV_ID_MASK_VENDOR){
        if(param->m_devID->m_vendor != aaldev_devid_vendorid(paal)){
            DPRINTF(AALBUS_DBG_MOD, ": No match on AAL_DEV_ID_MASK_VENDOR\n");
            return 0;
        }
      }

      // Passed all of the criteria
      param->m_dev = pdev;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
      DPRINTF(AALBUS_DBG_MOD, ": Found device %s.  Passed all criteria\n", dev_name(pdev));
#else
      DPRINTF(AALBUS_DBG_MOD, ": Found device %s.  Passed all criteria\n",  pdev->bus_id);
#endif
      return 1;
   } // if(mask)
   else{
      // Check for exact match
      if( memcmp( param->m_devID, &paal->m_devid, sizeof( struct aal_device_id ) ) == 0 ){
         param->m_dev = pdev;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
         DPRINTF(AALBUS_DBG_MOD, ": Found device %s\n", dev_name(pdev));
#else
         DPRINTF(AALBUS_DBG_MOD, ": Found device %s\n", pdev->bus_id);
#endif
         return 1;
      }
   }

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
   DPRINTF(AALBUS_DBG_MOD, ": Checking children of %s\n", dev_name(pdev));
#else
   DPRINTF(AALBUS_DBG_MOD, ": Checking children of %s\n", pdev->bus_id);
#endif

   // If the above did not match check the child devices if any
   device_for_each_child(pdev, param, aalbus_itr_get_device);
   if ( NULL != param->m_dev ) {
      // Device found
      return 1;
   }
   return 0;
}


//=============================================================================
// Name: aalbus_get_device
// Description: Get device by device ID.
// Interface: public
// Inputs: devID - generic device structure
// Outputs: none
// Returns: the device object pointer - if successful otherwise NULL
// Comments:
//=============================================================================
struct aal_device *
aalbus_get_device(struct aal_device_id *devID, __u32 mask)
{
   struct device_find_param param =
   {
      .m_mask     = mask,
      .m_devID    = devID,
      .m_dev      = NULL,
   };
   DPRINTF( AALBUS_DBG_MOD, ": in\n");
   // kosal_sem_get_user_alertable(&aalBus.m_sem);
   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return NULL;
   }

   DPRINTF( AALBUS_DBG_MOD, ": searching device tree\n");
   bus_for_each_dev(&aalBus.m_bustype, NULL, &param, aalbus_itr_get_device);
   kosal_sem_put(&aalBus.m_sem);

   DPRINTF( AALBUS_DBG_MOD, ": detected %p\n",param.m_dev != NULL ? basedev_to_aaldev(param.m_dev) : NULL);
   return (NULL != param.m_dev) ? basedev_to_aaldev(param.m_dev) : NULL;
}

//=============================================================================
// Name: aalbus_handle_to_device
// Description: Get device by handle.
// Interface: public
// Inputs: handle - device handle
// Outputs: none
// Returns: the device object pointer - if successful
// Comments:
//=============================================================================
struct aal_device *
aalbus_handle_to_device(void *handle)
{
   // Handle is really the dev pointer
   struct aal_device *devp = (struct aal_device *)handle;

   // Sanity check the handle
   if ( likely( devp &&
                ( (struct aal_bus *)aaldev_busp(devp) == (struct aal_bus *)&aalBus) ) ) {
      return devp;
   }

   return NULL;
}


//=============================================================================
// Name: aalbus_send_config_update_event
// Description: send a device update event for a single device
// Interface: private
// Inputs:  dev - Device
//          EventHandler - Event Handler
//
// Outputs: none
// Returns: none
// Comments: TODO - Only supports one handler. May need to extend
//=============================================================================
int
aalbus_send_config_update_event(struct aal_device *pdev,
                                krms_cfgUpDate_e   updateType,
                                btPID              pid)
{
   struct update_config_parms parms;

   DPRINTF(AALBUS_DBG_MOD,  "Sending Update Event dev = %p\n", pdev);

   ////////////////////////////////////////////////////////////////////////
   if ( aalBus.config_update_event_handler.EventHandler ) {
      if ( !pdev ) {
         kosal_printk_level(KERN_WARNING, "NULL pdev\n");
         return 0;
      }

      parms.Handler    = aalBus.config_update_event_handler.EventHandler;
      parms.updateType = updateType;
      parms.pid        = pid;

      if ( NULL != parms.Handler ) {
         parms.Handler(pdev, parms.updateType, parms.pid,
                          aalBus.config_update_event_handler.context);
      }
   } // if (aalBus.config_update_event_handler.EventHandler)
   return 0;
}

//=============================================================================
// Name: aalbus_config_update_event
// Description: generate a device update event for all devices in chain
// Interface: private
// Inputs:  dev - Device
// Outputs: none
// Returns: none
// Comments: TODO - Only supports one handler. May need to extend
//=============================================================================
int
aalbus_config_update_event(struct device *pbasedev, void *data)
{
   struct aal_device          *pdev   = basedev_to_aaldev(pbasedev);
   struct update_config_parms *pparms = data;

   if ( !pdev ) {
	   DPRINTF( AALBUS_DBG_MOD, "NULL pdev\n");
      return 0;
   }

   DPRINTF( AALBUS_DBG_MOD, "\n===============================\n");

   DPRINTF(AALBUS_DBG_MOD,"Type: ");
   switch ( pparms->updateType ) {
      case krms_ccfgUpdate_DevAdded:
         DPRINTF( AALBUS_DBG_MOD," Device Added\n");
      break;
      case krms_ccfgUpdate_DevRemoved:
         DPRINTF( AALBUS_DBG_MOD," Device Removed\n");
      break;
      case krms_ccfgUpdate_DevOwnerAdded:
         DPRINTF( AALBUS_DBG_MOD," Owner Added\n");
      break;
      case krms_ccfgUpdate_DevOwnerUpdated:
         DPRINTF( AALBUS_DBG_MOD," Owner Updated\n");
      break;

      case krms_ccfgUpdate_DevOwnerRemoved:
         DPRINTF( AALBUS_DBG_MOD," Owner Removed\n");
      break;
      case krms_ccfgUpdate_DevActivated:
         DPRINTF( AALBUS_DBG_MOD," Device Activated\n");
      break;
      case krms_ccfgUpdate_DevQuiesced:
         DPRINTF( AALBUS_DBG_MOD," Device Quiecsed\n");
      break;

      default:
         DPRINTF( AALBUS_DBG_MOD," Unknown\n");
      break;
   }
   DPRINTF( AALBUS_DBG_MOD, "On Device \"%s\" channel %d.\n",
              aaldev_devname(pdev),
              aaldev_devaddr_subdevnum(pdev));

   //////////////////////////////////////////////////////////////////////////////////////////
   if ( NULL != pparms->Handler ) {
      DPRINTF( AALBUS_DBG_MOD, ": Sending Update Event\n");
      pparms->Handler(pdev,
                      pparms->updateType,
                      pparms->pid,
                      aalBus.config_update_event_handler.context);
   }
   DPRINTF( AALBUS_DBG_MOD, "\n===============================\n");

   // Walk the chain down through all children
   device_for_each_child(pbasedev, data, aalbus_config_update_event);
   return 0;
}


//=============================================================================
// Name: aalbus_register_config_update_handler
// Description: Registers an event handler to accept config update events
// Interface: public
// Inputs: EventHandler - Event Handler (NULL means unistall)
//         context -
// Outputs: none
// Returns: 0 - if successful
// Comments: TODO - Only supports one handler. May need to extend
//=============================================================================
int
aalbus_register_config_update_handler(aalbus_event_config_update_t EventHandler,
                                      void                        *context)

{
   // Create a device added event - Generated to send the initial update
   struct update_config_parms parms = {
      .updateType = krms_ccfgUpdate_DevAdded,
      .pid        = 0,
      .Handler    = EventHandler,
   };

   // Only support one handler now
   if( (NULL != EventHandler) &&
       (NULL != aalBus.config_update_event_handler.EventHandler) ) {
      return -EINVAL;
   }

   // kosal_sem_get_user_alertable(&aalBus.m_sem);
   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return -EINTR;
   }

   aalBus.config_update_event_handler.EventHandler = EventHandler;
   aalBus.config_update_event_handler.context      = context;

   kosal_sem_put(&aalBus.m_sem);

   // If the handler was disabled then done.
   if ( NULL == aalBus.config_update_event_handler.EventHandler ) {
      return 0;
   }

   //  If a new handler was enabled then generate the whole configuration state
   DPRINTF( AALBUS_DBG_MOD, ": Sending Config Updates\n");

   // Generate a config update for every device in the system
   // kosal_sem_get_user_alertable(&aalBus.m_sem);
   if ( unlikely( kosal_sem_get_user_alertable(&aalBus.m_sem) ) ) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return -EINTR;
   }
   DPRINTF( AALBUS_DBG_MOD," \n\n\n\n\n************************************************\n");
   bus_for_each_dev(&aalBus.m_bustype,
                    NULL,
                    &parms,
                    aalbus_config_update_event);
   DPRINTF( AALBUS_DBG_MOD,"************************************************\n\n\n");
   kosal_sem_put(&aalBus.m_sem);
   return 0;
}

#if 0
// #if 0 - aalbus_walk_device_chain() is unused.
//=============================================================================
// Name: aalbus_walk_device_chain
// Description: Walks the device chain fro debug purposes
// Interface: public
// Inputs:
//         context -
// Outputs: none
// Returns: 0 - if successful
// Comments: TODO - Only supports one handler. May need to extend
//=============================================================================
int aalbus_walk_device_chain(void)

{
   // Create a device added event - Generated to send the initial update
   struct update_config_parms  parms={
      .updateType = krms_ccfgUpdate_DevAdded,
      .pid        = 0,
      .Handler    = NULL,
   };

   DPRINTF( AALBUS_DBG_MOD, ": WALKING BUS DEVICE CHAIN\n");

   // Generate a config update for every device in the system
   // kosal_sem_get_user_alertable(&aalBus.m_sem);
   if (unlikely (kosal_sem_get_user_alertable(&aalBus.m_sem))) {
      DPRINTF (AALBUS_DBG_MOD, ": kosal_sem_get_user_alertable interrupted\n");
      return -EINTR;
   }
   DPRINTF( AALBUS_DBG_MOD," \n********** DEVICE CHAIN **************\n");
   bus_for_each_dev( &aalBus.m_bustype,
                     NULL,
                     &parms,
                     aalbus_config_update_event );
   DPRINTF( AALBUS_DBG_MOD,"************* END DEVICE CHAIN **********\n\n\n");
   kosal_sem_put(&aalBus.m_sem);
   return 0;
}
#endif // #if 0

/** @brief validate the passed device
 * @return non-zero if device is a valid AAL device */
int aalbus_validate_device(struct aal_device *pdev)
{
   struct aal_bus *aal_bus_p;
   struct aal_bus_type *aal_bus_type_p;
   int status;
   bool found = false;
   struct aal_device *curdev_p;


   /* simple check before proceeding */
   if ( NULL == pdev ) {
      DPRINTF(AALBUS_DBG_MOD, " Failed NULL pointer\n");
      return false;
   }

   /* get handle to aalbus and outer aalBus */
   aal_bus_p = aalbus_get_bus();
   if (NULL == aal_bus_p) {
      DPRINTF(AALBUS_DBG_MOD, " aalbus_get_bus failed\n");
      return false;
   }
   aal_bus_type_p = container_of(aal_bus_p, struct aal_bus_type, m_bus);

   /* grab the allocation list mutex and search the allocation list for
    * our device */
   status = kosal_sem_get_user_alertable(&aal_bus_type_p->alloc_list_sem);
   if (0 != status) {
      DPRINTF(AALBUS_DBG_MOD,
          " Failed to acquire allocate list semaphore\n");
      return false;
   } else {
      kosal_list_for_each_entry(curdev_p,
          &aal_bus_type_p->alloc_list_head,
          m_alloc_list, struct aal_device)
      {
         DPRINTF(AALBUS_DBG_MOD, "%p on list\n", curdev_p);
         if (curdev_p == pdev) {
            found = true;
            break;
         }
      }
      kosal_sem_put(&aal_bus_type_p->alloc_list_sem);
   }

   /* if we've verified pdev is a known aal_device, do some additional
    * sanity checks on its members */
   if (true == found) {
      status = ( ( (__u64)virt_to_phys(pdev) == pdev->m_validator )
          && ( AAL_DEVICE_VERSION == pdev->m_version   ) );
   } else {
      DPRINTF(AALBUS_DBG_MOD,
          " couldn't find device %p on allocated list\n",
          pdev);
      status = false;
   }

   DPRINTF(AALBUS_DBG_MOD, " device %p was %sfound\n", pdev,
       (status ? "" : "NOT "));

   return status;
}


//
// Export the public interfaces
//
EXPORT_SYMBOL( aalbus_get_bus );
EXPORT_SYMBOL( aalbus_get_device );  //TODO get rid

