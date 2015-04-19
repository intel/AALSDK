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
//        FILE: aalrm-main.c
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file implements the initialization and cleanup code for the
//          AAL Resource Manager Client Service Module.
// HISTORY:
// COMMENTS: This module implements the resource manager client.
// WHEN:          WHO:     WHAT:
// 02/20/08       JG       Initial version created
// 06/17/08       JG       Separated Policy manager (a.k.a. RMS)
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
// 02/26/2009     JG       Nulled unused match()
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMC_DBG_MOD

#include "aalrm-int.h"
#include "aalsdk/kernel/aalrm-services.h"
#include "aalsdk/kernel/aalrm_server-services.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"


MODULE_VERSION    (DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR     (DRV_AUTHOR);
MODULE_LICENSE    (DRV_LICENSE);

//=============================================================================
// Driver Parameters
//=============================================================================

// debug flags with default values
btUnsignedInt debug = 0
#if 1
   | AALRMC_DBG_DEFAULT
#endif
;

// Major device number to use for the device nodes
btInt majornum = 0;


//
//  Declarations for module parameters - Enables parameter value passing from
//                                       insmod and permissions as seen from /sys
//
MODULE_PARM_DESC(debug, "debug level");
module_param    (debug, int, 0444);

MODULE_PARM_DESC(majornum, "major device number");
module_param    (majornum, int, 0444);


//=============================================================================
// Name: aalbus_attrib_show_debug
//=============================================================================
static ssize_t ahmpip_attrib_show_debug(struct device_driver *drv, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%d\n",debug));
}

//=============================================================================
// Name: aalbus_attrib_store_debug
//=============================================================================
static ssize_t ahmpip_attrib_store_debug(struct device_driver *drv,
                                         const char *buf,
                                         size_t size)
{
   int temp = 0;
   sscanf(buf,"%d", &temp);

   debug = temp;

   printk(KERN_INFO DRV_NAME ": Attribute change - debug = %d\n", temp);
   return size;
}

// Attribute accessors for debug
DRIVER_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);


// Declare standard entry points
static btInt  aalrm_init(void);
static void aalrm_exit(void);

module_init(aalrm_init);
module_exit(aalrm_exit);

static btInt resourcemgr_init(void);
static void aalrm_register_sess(kosal_list_head *);
static void aalrm_unregister_sess(kosal_list_head *);

#if 0
static btInt aalrm_initialize_driver(const char *,
                                     int,
                                     struct aal_driver *,
                                     struct aal_classdevice *);
static void aalrm_removedriver(struct aal_driver*,
                               struct aal_classdevice *);
#endif

static struct aal_interface *aalrm_get_interface(struct aal_driver*,
                                                 btID );
static btInt aalrm_has_interface(struct aal_driver*,
                                 btID);
static btInt aalrm_supports_interface(struct aal_driver*,
                                      btID);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      RESOURCE MANAGER CLIENT OBJECTS     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: devIID_tbl
// Description: Lists the IDs of the interfaces this service exports to the
//              AALBus interface broker
//=============================================================================
static btID  devIID_tbl[] = {
   AAL_RMAPI_IID_01,
   0
};

//=============================================================================
// Name: id_table
// Description: This driver supports the following devices
//=============================================================================
static struct aal_device_id id_table[] = {
   aal_device_id_terminator // None
};

// Used for hotplug system
MODULE_DEVICE_TABLE( aal, id_table );

//=============================================================================
// Name: aalrm_class
// Description: Class device. Wrapper object that contains both the Linux
//              DD Model class information but also AAL specific class
//              information. The modules "class" defines its unique interface
//              attributes.
//=============================================================================
struct aal_classdevice  rm_class = {
   .m_classid = {
      .m_majorversion   = AALRM_API_MAJVERSION,
      .m_minorversion   = AALRM_API_MINVERSION,
      .m_releaseversion = AALRM_API_RELEASE,
      .m_classGUID      = AALRM_API_INTC,
   },
   .m_devIIDlist = devIID_tbl,        // List of supported APIs
};


//=============================================================================
//=============================================================================
// Name: rm_driver
// Description: This is the Resource Manager Driver Object singleton. This is
//              object that gets registered with AALBus.  It is a device driver
//              in name only. It does not actually control any device HW.
//              As a device driver module it is allowed to expose a user mode
//              interface.
//=============================================================================
//=============================================================================
static struct aal_driver rm_driver = {

   // File operations (User Space control interface)
   .m_fops = {
      .owner    = THIS_MODULE,
      .open     = aalrm_open,
      .release  = aalrm_close,

#if HAVE_UNLOCKED_IOCTL
      .unlocked_ioctl = aalrm_ioctl,
#else
      .ioctl          = aalrm_ioctl,  // Deprecated in 2.6.36
#endif

      .poll     = aalrm_poll,
   },

   // Return an interface pointer based on the iid. Used to dynamically
   // bind to a custom interface the driver may export. Typically used to
   // export the internal control interface used by a user space interface
   // driver.
   .get_interface          = aalrm_get_interface,

   // Return 1 if interface supported
   .has_interface          = aalrm_has_interface,

   // Returns 1 if the interface specified in iid can be used. Typically
   // used to find out if a user space interface driver can "speak" particular
   // interface.
   .supports_interface     = aalrm_supports_interface,

   // List of implemented dynamic interfaces
   .m_iids                 = 0,

   // ID list of supported devices if this is a device driver
   .m_idtable              = id_table,

   // Called to determine if this driver supports a device
   //.m_match                = aalrm_drv_match,
   .m_match                = NULL,

   // Called when a device matches
   .m_probe                = NULL,

   // Base structure
   .m_driver = {
      .owner   = THIS_MODULE,
      .name    = RESMGR_DEV_NAME,
   },
};


//=============================================================================
// Name: resmgr
// Description: Resource manager service object instance.
//=============================================================================
struct aalresmgr resmgr = {
   // Public Methods
   .register_sess    = aalrm_register_sess,
   .unregister_sess  = aalrm_unregister_sess,

   // Driver and class
   .m_driver   =  &rm_driver,
   .m_class    =  &rm_class,
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////         RESOURCE MANAGER METHODS         ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrm_register_sess
// Description: Registers a session with the resource manager
// Interface: public
// Inputs: psession - session pointer
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_register_sess(kosal_list_head *psession)
{
   if (kosal_sem_get_krnl_alertable(&resmgr.m_sem)) { /* FIXME */ }
   kosal_list_add_head( psession, &resmgr.m_sessq);
   kosal_sem_put(&resmgr.m_sem);
}

//=============================================================================
// Name: aalrm_unregister_sess
// Description: Unregisters a session with the resource manager
// Interface: public
// Inputs: psession - session pointer
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_unregister_sess(kosal_list_head *psession)
{
   if (kosal_sem_get_krnl_alertable(&resmgr.m_sem)) { /* FIXME */ }
   kosal_list_del_init(psession);
   kosal_sem_put(&resmgr.m_sem);
}

//=============================================================================
// Name: aalrm_init
// Description: Initialization routine for the module.  This is the module
//              entry point.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
btInt aalrm_init(void)
{
   btInt ret = 0;

   //-------------------
   // Display the signon
   //-------------------
   kosal_printk_level(KERN_INFO, "Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_DESCRIPTION);
   kosal_printk_level(KERN_INFO, "-> Version %s\n",DRV_VERSION);
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_COPYRIGHT);

   //--------------------------------
   // Initialize the Resource Manager
   //--------------------------------
   ret = resourcemgr_init();

   return ret;
}


//=============================================================================
// Name: resourcemgr_init
// Description: Initialization the resource manager object
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
btInt resourcemgr_init()
{
   btInt ret = 0;

   // Initialize user space interface driver
   //    Causes the module to be registered with AALBus
   //    and instantiates the user mode device interface

   aalbus_get_bus()->init_driver((kosal_ownermodule *)THIS_MODULE,
                                &rm_driver,
                                &rm_class,
                                RESMGR_DEV_NAME,
                                majornum);

   if(ret < 0) {
      DPRINTF(AALRMC_DBG_MOD, ": failed to register driver %s\n", RESMGR_DEV_NAME);
      return ret;
   }

   if(driver_create_file(&rm_driver.m_driver,&driver_attr_debug)){
       DPRINTF (AHMPIP_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
       // Unregister the driver with the bus
       aalbus_get_bus()->unregister_driver( &rm_driver );
       return -EIO;
   }


   // Initialize structures
   kosal_list_init(&resmgr.m_sessq);    // Queue of sessions
   kosal_mutex_init(&resmgr.m_sem);          // Private semaphore

   // Get the Resource Manager Server Service Interface from AALBus interface broker
   resmgr.m_rmssrvs = aalbus_get_bus()->get_service_interface(AAL_RMSAPI_IID_01);

   //TODO - MAKE THIS SHOULD/COULD USE NOTIFY interface service once it is implemented
   if(resmgr.m_rmssrvs == NULL){
      DPRINTF(AALRMC_DBG_MOD, ": failed to get RMS service interface [%x]\n", AAL_RMSAPI_IID_01);
      //aalrm_removedriver(&rm_driver, &rm_class);
      aalbus_get_bus()->release_driver(&rm_driver, &rm_class);
      return -ENODEV;
   }
   return 0;
}

#if 0
//=============================================================================
// Name: aalrm_initialize_driver
// Description: Initialization routine for the  driver. Registers with the bus
//              driver
// Interface: private
// Inputs: name - driver/class name
//         majornum - major device node number to assign
//         pdriver - driver to register
//         pclassdev - class device to register
// Outputs: none.
// Comments:
//=============================================================================
btInt aalrm_initialize_driver( const char * name,
                             btInt majornum,
                             struct aal_driver *pdriver,
                             struct aal_classdevice *pclassdev)
{
   btInt ret = 0;
   dev_t dev;

   // Get a major device number
   if (majornum != 0) {
      // major number provided in command line
      DPRINTF(AALRMC_DBG_MOD, ": Registering static major number %d\n", majornum);
      dev = MKDEV(majornum, 0);
      ret = register_chrdev_region(dev, 1, name);
   }
   else {
      // Dynamically allocate the major number
      ret = alloc_chrdev_region(&dev, 0, 1, name);
      pdriver->m_major = MAJOR(dev);
      DPRINTF(AALRMC_DBG_MOD, ": Allocating dynamic major number %d for %s\n", pdriver->m_major, name);
   }
   if(ret < 0) {
      DPRINTF(AALRMC_DBG_MOD, ": Failed to assign major device number %d for %s\n", pdriver->m_major, name);
      return ret;
   }

   // Register the driver with the bus
   ret = aalbus_get_bus()->register_driver( pdriver );
   if( ret < 0 ) {
      DPRINTF(AALRMC_DBG_MOD, ": Failed to register ret = %d\n", ret);
      unregister_chrdev_region(dev, 1);
      return ret;
   }

   //---------------------------------------------------------------------------
   // Register the UI character device with the kernel.
   // Uses the cdev structure embedded in the aal_driver.
   // This simply hooks our entry points. The device node is created
   // with the class device below.
   //---------------------------------------------------------------------------
   cdev_init(aaldrv_cdevp(pdriver), &pdriver->m_fops);
   aaldrv_cdevp(pdriver)->owner = THIS_MODULE;
   aaldrv_cdevp(pdriver)->ops   = &pdriver->m_fops;

   ret = cdev_add(aaldrv_cdevp(pdriver), dev, 1);
   if ( ret ) {
      DPRINTF(AALRMC_DBG_MOD, ": Failed to register charcter device - ret = %d\n", ret);
      rmcdrv_to_aalbus(pdriver).unregister_driver( pdriver );
      unregister_chrdev_region(dev, 1);
      return ret;
   }

   //-----------------------------------------------
   // Initialize and create the class device.
   // This is where the actual device node will be
   // created in user space
   //-----------------------------------------------
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
   dev_set_name(&pclassdev->m_classdev,name);
#else
   strncpy(AALCLASSNAME(pclassdev),name, BUS_ID_SIZE);
#endif
   aal_classdev_devtype(pclassdev) = dev;

   DPRINTF(AALRMC_DBG_MOD, ": Registering class \"%s\"\n", aal_classdev_get_name(pclassdev));
   ret = aaldrv_aalbus(pdriver).register_class_device(pclassdev);
   if(ret < 0) {
      DPRINTF(AALRMC_DBG_MOD, ": Failed to register class\n");
      rmcdrv_to_aalbus(pdriver).unregister_driver(pdriver);
      unregister_chrdev_region(dev, 1);
      cdev_del(aaldrv_cdevp(pdriver));
      return ret;
   }

   return ret;
}
#endif


//=============================================================================
// Name: aalrm_exit
// Description: Removes device from filesystem and unregisters
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_exit(void)
{
   // Get the RMS interface
   aalbus_get_bus()->release_service_interface(resmgr.m_rmssrvs);

   // Unregister drivers
   aalbus_get_bus()->release_driver(&rm_driver, &rm_class);
   //aalrm_removedriver(&rm_driver, &rm_class);

   kosal_printk_level(KERN_INFO, "<- %s removed\n", DRV_DESCRIPTION);
   return;
}

#if 0
//=============================================================================
// Name: aalrm_removedriver
// Description: Removes device from filesystem and registration
// Interface: public
// Inputs: pdriver - driver to remove,
//         pclassdev - class device
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_removedriver(struct aal_driver* pdriver,
                        struct aal_classdevice *pclassdev)
{
   dev_t dev;
   btInt majornum = pdriver->m_major;

   DPRINTF(AALRMC_DBG_MOD, ": Removing driver\n");

   // Remove the class device
   rmcdrv_to_aalbus(pdriver).unregister_class_device(pclassdev);

   // Unregister the character driver
   cdev_del(aaldrv_cdevp(pdriver));

   // Unregister the driver
   rmcdrv_to_aalbus(pdriver).unregister_driver( pdriver );

   dev = MKDEV(majornum, 0);
   unregister_chrdev_region(dev, 1);
   return;
}
#endif

//=============================================================================
// Name:  aalrm_get_interface
// Description: Returns the requested interface if supported
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: interface pointer.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
struct aal_interface  *aalrm_get_interface(struct aal_driver* drv,
                                           btID iid)
{
   return NULL;
}


//=============================================================================
// Name:  aalrm_has_interface
// Description: Reports if this driver implements a specifc interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if interface implemented.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
btInt aalrm_has_interface(struct aal_driver* drv, btID iid)
{
   return 0;
}

//=============================================================================
// Name:  aalrm_supports_interface
// Description: Report whether this driver can use a specific interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if supported.
// Comments:
//=============================================================================
btInt aalrm_supports_interface(struct aal_driver* drv, btID iid)
{
   if(iid != AAL_DDAPI_IID_07){
      return 0;
   }
   return 1;
}
