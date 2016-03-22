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
//        FILE: aalrm_server-main.c
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file implements the initialization and cleanup code for the
//          AAL Resouce Manager Server Kernel Module.
// HISTORY:
// COMMENTS: This module implemeents RMS. The Resource
//           manager server is an service interface device used by the resource
//           management user space service daemon responsible for all of the
//           policy decisions of the resource manager.
// WHEN:          WHO:     WHAT:
// 02/20/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/21/2009     JG       Initial code cleanup
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 02/26/2009     JG       Nulled unused match()
// 04/13/2009     JG       Added support for version kernel 2.6.27
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMS_DBG_MOD

#include "aalrm_server-int.h"
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
btUnsignedInt debug = 0; //AALRMS_DBG_DEFAULT;

// Major device number to use for the device nodes
btInt majornum = 0;
#if defined( __AAL_LINUX__ )

//
//  Declarations for module parameters - Enables parameter value passing from
//                                       insmod and permissions as seen from /sys
//
MODULE_PARM_DESC(debug, "debug level");
module_param    (debug, int, 0644);

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

   DPRINTF(AALRMS_DBG_MOD, ": Attribute change - debug = %d\n", temp);
   return size;
}

// Attribute accessors for debug
DRIVER_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);



#endif // __AAL_LINUX__

// Declare standard entry points
static int
aalrm_server_init(void);
static void
aalrm_server_exit(void);

module_init(aalrm_server_init);
module_exit(aalrm_server_exit);


// Prototypes
void
aalrm_server_register_sess(kosal_list_head *psession);
void
aalrm_server_unregister_sess(kosal_list_head *psession);

/*
int
aalrm_server_initialize_driver(const char             *name,
                               int                     majornum,
                               struct aal_driver      *pdriver,
                               struct aal_classdevice *pclassdev);
void
aalrm_server_removedriver(struct aal_driver      *pdriver,
                          struct aal_classdevice *pclassdev);
*/

int
aalrm_server_drv_match(struct aal_driver *drv, struct aal_device *dev);

struct aal_interface *
aalrm_server_get_interface(struct aal_driver *drv, btID iid);
int
aalrm_server_has_interface(struct aal_driver *drv, btID iid);
int
aalrm_server_supports_interface(struct aal_driver *drv, btID iid);
int
rm_server_init(void);
int
rm_server_destroy(void);

//=============================================================================
// Name: devIID_tbl
// Description: Lists the IDs of the interfaces this service exports to the
//              AALBus interface broker
//=============================================================================
static btID devIID_tbl[] = {
   AAL_RMSAPI_IID_01,
   0
};

//=============================================================================
// Name: id_table
// Description: This driver supports the following devices
//=============================================================================
static struct aal_device_id id_table[] = {
   aal_device_id_terminator // none
};
MODULE_DEVICE_TABLE(aal, id_table);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      RESOURCE MANAGER SERVER OBJECTS     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================


//=============================================================================
// Name: rm_server_service
// Description: RMS service interface declaration. This interface exposes
//              the services provided to the Resource Manager Client Service.
//              This interface is registered with the AALBus Service Interface
//              broker.
//=============================================================================
static struct aalrm_server_service rm_server_service = {
   .request_device      = aalrms_request_device,
   .registrar_request   = aalrms_registrar_request,
   .cancel_all_requests = aalrms_cancel_all_requests
};

// AAL interface used when registering service interfaces with AALBus
static struct aal_interface rm_server_intfc;

//=============================================================================
// Name: aalrm_server_class
// Description Class device. Wrapper object that contains both the Linux
//             DD Model class information but also AAL specific class
//             information. The modules "class" defines its unique interface
//             attributes.
//=============================================================================
static struct aal_classdevice rms_class = {
   .m_classid = {
      .m_majorversion   = AALRMS_API_MAJVERSION,
      .m_minorversion   = AALRMS_API_MINVERSION,
      .m_releaseversion = AALRMS_API_RELEASE,
      .m_classGUID      = AALRMS_API_INTC,
   },
   .m_devIIDlist = devIID_tbl,        // List of supported device module APIs
};


//=============================================================================
// Name: rms_driver
// Description: This is the Resource Manager Driver Object singleton. This is
//              object that gets registered with AALBus.  It is a device driver
//              in name only. It does not actually control any device HW.
//              As a device driver module it is allowed to expose a user mode
//              interface.
//=============================================================================
static struct aal_driver rms_driver = {

   // File operations (User Space control interface)
   // Exposes methods invoked by the user space device driver interface
   .m_fops = {
      .owner           = THIS_MODULE,
      .open            = aalrm_server_open,
      .release         = aalrm_server_close,
#if HAVE_UNLOCKED_IOCTL
      .unlocked_ioctl  = aalrm_server_ioctl,
#else
      .ioctl           = aalrm_server_ioctl,  // Deprecated in 2.6.36
#endif
      .poll            = aalrm_server_poll,
   },

   // Return an interface pointer based on the iid. Used to dynamically
   // bind to a custom interface the driver may export. Typically used to
   // export the internal control interface used by a user space interface
   // driver.
   .get_interface      = aalrm_server_get_interface,

   // Return 1 if interface zupported
   .has_interface      = aalrm_server_has_interface,

   // Returns 1 if the interface specified in iid can be used. Typically
   // used to find out if a user space interface driver can "speak" particular
   // interface.
   .supports_interface = aalrm_server_supports_interface,

   // List of implemented dynamic interfaces
   .m_iids             = 0,

   // ID list of supported devices if this is a device driver
   .m_idtable          = id_table,

   // Called to determine if this driver supports a device
   //.m_match                = aalrm_server_drv_match,
   .m_match            = NULL,

   // Called when a device matches
   .m_probe            = NULL,

   // Base structure
   .m_driver =  {
      .owner = THIS_MODULE,
      .name  = RMS_DEV_NAME,
   },
};

//=============================================================================
// Name: rmserver
// Description: RMS object instance.
//=============================================================================
struct aalrm_server rmserver = {
   // Public Methods
   .register_sess   = aalrm_server_register_sess,
   .unregister_sess = aalrm_server_unregister_sess,

   // Driver and class
   .m_driver = &rms_driver,
   .m_class  = &rms_class,
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
// Name: aalrm_server_register_sess
// Description: Registers a session with the RMS
// Interface: public
// Inputs: psession - session pointer
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_server_register_sess(kosal_list_head *psession)
{
   if (kosal_sem_get_krnl_alertable(&rmserver.m_sem)) { /* FIXME */ }
   list_add( psession, &rmserver.m_sessq);
   kosal_sem_put(&rmserver.m_sem);
}

//=============================================================================
// Name: aalrm_server_unregister_sess
// Description: Unregisters a session with the resource manager
// Interface: public
// Inputs: psession - session pointer
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_server_unregister_sess(kosal_list_head *psession)
{
   if (kosal_sem_get_krnl_alertable(&rmserver.m_sem)) { /* FIXME */ }
   list_del_init(psession);
   kosal_sem_put(&rmserver.m_sem);
}

//=============================================================================
// Name: aalrm_server_init
// Description: Initialization routine for the module. Registers with the bus
//              driver
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static int
aalrm_server_init(void)
{
   int ret = 0;

   //-------------------
   // Display the signon
   //-------------------
   kosal_printk_level(KERN_INFO, "Accelerator Abstraction Layer\n");
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_DESCRIPTION);
   kosal_printk_level(KERN_INFO, "-> Version %s\n",DRV_VERSION);
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_COPYRIGHT);

   //---------------------------------------
   // Initialize the Resource Manager Server
   //---------------------------------------
   ret = rm_server_init();
   return ret;
}

//=============================================================================
// Name: rm_server_init
// Description: Initialization the resource manager server object
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int rm_server_init(void)
{
   int ret = 0;

   //---------------------------------------
   // Initialize the device driver subsystem
   //  registers the driver with AALBus and
   //  registers class device, creating user
   //  mode device node.
   //---------------------------------------

   ret = aalbus_get_bus()->init_driver((kosal_ownermodule *)THIS_MODULE,
                                      &rms_driver,
                                      &rms_class,
                                      RMS_DEV_NAME,
                                      majornum);

/*
   ret = aalrm_server_initialize_driver(RMS_DEV_NAME,
                                        majornum,
                                       &rms_driver,
                                       &rms_class);
*/

   if ( ret < 0 ) {
      DPRINTF(AALRMS_DBG_MOD, ": failed to register driver %s\n", RMS_DEV_NAME);
      return ret;
   }


   if(driver_create_file(&rms_driver.m_driver,&driver_attr_debug)){
       DPRINTF (AALRMS_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
       // Unregister the driver with the bus
       aalbus_get_bus()->unregister_driver( &rms_driver );
       return -EIO;
   }



   // Initialize structures
   kosal_list_init(&rmserver.m_sessq);          // Session queue
   aal_queue_init(&rmserver.m_reqq);           // Request queue m_pendq
   aal_queue_init(&rmserver.m_pendq);          // Pending queue
   kosal_mutex_init(&rmserver.m_sem);                // Private semaphore
   kosal_init_waitqueue_head(&rmserver.m_reqq_wq);   // Wait queue for requests

   //------------------------------------------
   // Create and register the service interface
   //  with AALBus Service Interface Broker.
   //  Allows this service to be discovered and
   //  linked to at runtime.
   //------------------------------------------

   // Initialize the aal_interface
   aal_interface_init(rm_server_intfc,       // aal_interface wrapper
                      &rm_server_service,    // service interface
                      AAL_RMSAPI_IID_01);    // service interface ID

   // Register with the service interface with the AAL Bus Service INterface Broker
   DPRINTF(AALRMS_DBG_MOD, "Registering service interface 0x%Lx\n", (long long)AAL_RMSAPI_IID_01);
   ret = aalbus_get_bus()->register_service_interface(&rm_server_intfc);
   if(ret < 0){
	   DPRINTF(AALRMS_DBG_MOD, "Failed registeer service interface\n");
      //aalrm_server_removedriver(&rms_driver, &rms_class);
      aalbus_get_bus()->release_driver(&rms_driver, &rms_class);
      return ret;
   }
   return 0;
}

//=============================================================================
// Name: aalrm_server_exit
// Description: Removes device from filesystem and deregisters
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_server_exit(void)
{
   //TODO FLUSH ALL Messages on Resource manager queues

   // Unregister service interface with AALBus Interface Broker
   // TODO - This needs to be more sophisticated.  Must notify current interface
   //        holders that it is going.  Holders must release and then the interface
   //        can be revoked.  Perhaps the interface should be disabled so that
   //        using it always fails. The interface must not be allocatable while
   //        it is being revoked.
   //aalbus_get_bus()->unregister_service_interface(&rm_server_intfc);

   // Unregister drivers
   rm_server_destroy();

  DPRINTF(AALRMS_DBG_MOD,  "<- %s removed\n", DRV_DESCRIPTION);
   return;
}

//=============================================================================
// Name: rm_server_destroy
// Description: Destroys the RMS object
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int rm_server_destroy()
{
   //TODO HIGH PRIORITY FLUSH ALL Messages on queues

   // Unregister service interface
   aalbus_get_bus()->unregister_service_interface(&rm_server_intfc);

   aalbus_get_bus()->release_driver(&rms_driver, &rms_class);
   //aalrm_server_removedriver(&rms_driver, &rms_class);
   return 0;
}


#if 0
//=============================================================================
// Name: aalrm_server_initialize_driver
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
int
aalrm_server_initialize_driver(const char             *name,
                               int                     majornum,
                               struct aal_driver      *pdriver,
                               struct aal_classdevice *pclassdev)
{
   int ret = 0;
   dev_t dev;

   // Get a major device number
   if ( 0 != majornum ) {
      // major number provided in command line
      DPRINTF(AALRMS_DBG_MOD, ": Registering static major number %d\n", majornum);
      dev = MKDEV(majornum, 0);

      ret = register_chrdev_region(dev, 1, name);

      if ( ret < 0 ) {
         DPRINTF(AALRMS_DBG_MOD, ": Failed to register major device number %d for %s\n",
                    majornum, name);
         return ret;
      }
   } else {
      // Dynamically allocate the major number
      ret = alloc_chrdev_region(&dev, 0, 1, name);
      pdriver->m_major = MAJOR(dev);
      DPRINTF(AALRMS_DBG_MOD, ": Allocating dynamic major number %d for %s\n", pdriver->m_major, name);
   }


   if(ret < 0) {
      DPRINTF(AALRMS_DBG_MOD, ": Failed to assign major device number %d for %s\n", pdriver->m_major, name);
      return ret;
   }

   // Register the driver with the bus
   ret = aalbus_get_bus()->register_driver( pdriver );
   if( ret < 0 ){
      DPRINTF(AALRMS_DBG_MOD, ": Failed to register ret = %d\n", ret);
      unregister_chrdev_region(dev, 1);
      return ret;
   }

   // Register the UI character device with the kernel. Uses the cdev structure
   // embedded in the aal_driver.
   // This simply hooks our entry points. The device node is created
   // with the class device below
   cdev_init(aaldrv_cdevp(pdriver), &pdriver->m_fops);
   aaldrv_cdevp(pdriver)->owner = THIS_MODULE;
   aaldrv_cdevp(pdriver)->ops   = &pdriver->m_fops;

   ret = cdev_add(aaldrv_cdevp(pdriver), dev, 1);
   if ( ret ) {
      DPRINTF(AALRMS_DBG_MOD, ": Failed to register charcter device - ret = %d\n", ret);
      rms_drv_to_ibus(pdriver).unregister_driver( pdriver );
      unregister_chrdev_region(dev, 1);
      return ret;
   }

   // Initialize and create the class device.
   // This is where the actual device node will be
   // created in user land
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
   dev_set_name(&pclassdev->m_classdev,name);
#else
   strncpy(AALCLASSNAME(pclassdev),name, BUS_ID_SIZE);
#endif

   aal_classdev_devtype(pclassdev) = dev;

   DPRINTF(AALRMS_DBG_MOD, ": Registering class %s\n", aal_classdev_get_name(pclassdev));
   ret = aaldrv_aalbus(pdriver).register_class_device(pclassdev);
   if(ret < 0) {
      DPRINTF(AALRMS_DBG_MOD, ": Failed to register class\n");
      rms_drv_to_ibus(pdriver).unregister_driver(pdriver);
      unregister_chrdev_region(dev, 1);
      cdev_del(aaldrv_cdevp(pdriver));
      return ret;
   }

   return ret;
}
#endif

#if 0
//=============================================================================
// Name: aalrm_server_removedriver
// Description: Removes device from filesystem and registration
// Interface: public
// Inputs: pdriver - driver to remove,
//         pclassdev - class device
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_server_removedriver(struct aal_driver* pdriver,
                        struct aal_classdevice *pclassdev)
{
   dev_t dev;
   int majornum = pdriver->m_major;

   DPRINTF(AALRMS_DBG_MOD, ": Removing driver\n");

   // Remove the class device
   rms_drv_to_ibus(pdriver).unregister_class_device(pclassdev);

   // Unregister the character driver
   cdev_del(aaldrv_cdevp(pdriver));

   // Unregister the driver
   rms_drv_to_ibus(pdriver).unregister_driver( pdriver );

   dev = MKDEV(majornum, 0);
   unregister_chrdev_region(dev, 1);
   return;
}
#endif

//=============================================================================
// Name:  aalrm_server_drv_match
// Description: Driver specific function for matching a suported device
// Interface: public
// Inputs: drv - this driver
//         dev - device to check
// Outputs: none.
// Comments:
//=============================================================================
int aalrm_server_drv_match(struct aal_driver* drv, struct aal_device* dev)
{
   DPRINTF(AALRMS_DBG_MOD, ": Device not supported by this driver.\n");
   return 0;
}

//=============================================================================
// Name:  aalrm_server_get_interface
// Description: Returns the requested interface if supported
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: interface pointer.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
struct aal_interface  *aalrm_server_get_interface(struct aal_driver* drv,
                                                  btID iid)
{
   return NULL;
}


//=============================================================================
// Name:  aalrm_server_has_interface
// Description: Reports if this driver implements a specifc interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if interface implemented.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
int aalrm_server_has_interface(struct aal_driver* drv, btID iid)
{
   return 0;
}

//=============================================================================
// Name:  aalrm_server_supports_interface
// Description: Report whether this driver can use a specific interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if supported.
// Comments:
//=============================================================================
int aalrm_server_supports_interface(struct aal_driver* drv, btID iid)
{
   if(iid != AAL_DDAPI_IID_07){  //TODO this needs to be properly defined
      return 0;
   }
   return 1;
}
