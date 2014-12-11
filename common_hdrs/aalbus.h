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
//        FILE: aalbus.h
//     CREATED: 02/14/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  External definitions for the AAL Logical Bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-14-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 11/25/2008     HM       Large merge
// 01/04/2009     HM       Updated Copyright
// 02/26/2009     JG       Began dynamic config implementation
// 02/11/2010     JG       Support for kernel 2.6.31
// 07/15/2012     HM       Support for kernel 2.6.18
// 05/15/2014     JG       Added Windows support.  AAL 4.x refactoring.
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALBUS_H__
#define __AALSDK_KERNEL_AALBUS_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/aalbus-device.h>
#include <aalsdk/kernel/aaldevice.h>

#if defined( __AAL_LINUX__ )
struct inode;               // forward reference for 2.6.18 kernel for #include cdev.h

#include <linux/device.h>
#include <linux/cdev.h>     // struct cdev
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/kobject.h>
#elif defined( __AAL_WINDOWS__ )

// Specifies the device class for objects on the bus
DEFINE_GUID(GUID_DEVCLASS_AAL_DEVICE,
   0x89cb01b0, 0x1c62, 0x4c24, 0x9f, 0x51, 0x4f, 0x27, 0xdb, 0x38, 0x4c, 0xc4);
// {89CB01B0-1C62-4C24-9F51-4F27DB384CC4}

#endif


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            Exports
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern
struct aal_bus *
aalbus_get_bus(void);

// TODO REMOVE
struct aal_device_id; // Forward reference
extern
struct aal_device *
aalbus_get_device(struct aal_device_id *devID,
                  btUnsigned32bitInt mask );

// Forward reference
struct aal_bus;

//-----------------------------------------------------------------------------
// Public Interface
//-----------------------------------------------------------------------------
#define  AAL_vendINTC         (0x8086LL)     //Should come out of INTCDEFS.h TODO

//TODO ALL GO AWAY
//=============================================================================
// IDs for device driver module APIs i.e., Interface between User API module
// and the low-level device driver module. This is the interface exposed
// by a device driver that is used by an UI driver.  Acquired via the
// get_interface() method
//=============================================================================
#define  AAL_DDAPI_IID_07      (0x00007000)   // Interface used in 0.70
#define  AAL_DDAPI_IID_100     (0x01000000)   // Interface used in 1.00

//=============================================================================
// PIP Version IDs - This represents the physical interface implemented by the
// device and device driver
//=============================================================================
#define AAL_FSB_PIPID_V10      (0x0000A000)   // Interface used in 1.0
#define AAL_FSB_PIPID_V07      (0x00007000)   // Interface used in 0.70
#define AAL_ASM_PIPID_V10      (0x1000A000)   // Interface used in 1.0
#define AAL_ASM_PIPID_V07      (0x10007000)   // Interface used in 0.70
// TODO HERE


// forward references
struct aal_driver;
struct aal_classdevice;
struct aal_interface;
struct aal_device_id;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            AAL Bus
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//=============================================================================
// Name: aalbus_event_config_update_t
// Description:SIgnature for config update event callback handler
// Inputs: devhndl - handle to the device
//         devid - device ID
//         context - context
//=============================================================================
typedef void (*aalbus_event_config_update_t)(struct aal_device *devhndl,
                                             krms_cfgUpDate_e   updateType,
                                             btPID              pid,
                                             btObjectType       context);


//=============================================================================
// Prototypes for default AALbus methods
//=============================================================================
static inline
btInt
aalbus_def_register_class_device(struct aal_classdevice *classdevice);
static inline
void
aalbus_def_unregister_class_device(struct aal_classdevice *classdevice);
static inline
btInt
aalbus_def_register_device(struct aal_device *dev,
                           pkosal_bus_type bustype);
static inline
void
aalbus_def_unregister_device(struct aal_device *dev);
static inline
btInt
aalbus_def_init_driver(kosal_ownermodule      *powner,
                       struct aal_driver      *driver,
                       struct aal_classdevice *pclassdev,
                       const char             *devname,
                       btInt                   devmajor);
static inline
btInt
aalbus_def_release_driver(struct aal_driver      *driver,
                          struct aal_classdevice *pclassdev);
static inline
btInt
aalbus_def_register_driver(struct aal_driver *driver);
static inline
void
aalbus_def_unregister_driver(struct aal_driver *driver);
static inline
btInt
aalbus_def_register_service_interface(struct aal_interface *pinterface);
static inline
btInt
aalbus_def_unregister_service_interface(struct aal_interface *pinterface);
static inline
struct aal_interface *
aalbus_def_get_service_interface(btIID iid);
static inline
void
aalbus_def_release_service_interface(struct aal_interface *pinterface);
static inline
btInt
aalbus_def_has_interface(btIID iid);
static inline
struct aal_device *
aalbus_def_get_device(struct aal_device_id *, btUnsigned32bitInt );
static inline
struct aal_device *
aalbus_def_handle_to_device(btObjectType handle);
static inline
btInt
aal_bus_def_register_config_update_handler(aalbus_event_config_update_t EventHandler,
                                           btObjectType                 context);
#if 0
//static inline btInt aalbus_walk_device_chain(void);
#endif
static inline
btInt
aal_bus_def_send_config_update_event(struct aal_device *pdev,
                                     krms_cfgUpDate_e   updateType,
                                     btPID              pid);
//static inline btInt aalbus_def_send_uevent(struct aal_device *dev, enum kobject_action act, char *env[] );



// Used in register_device to indicate register with the native bus
#define AAL_BUSTYPE_NATIVE    (kosal_bus_type )(-1)

//=============================================================================
// Name: aal_bus
// Description: AAL bus class - Public members
//=============================================================================
struct aal_bus {
#define AAL_BUS_VERSION 0x0001000000000001ULL
   btUnsigned64bitInt      m_version; // Interface version

   // Declare as a device factory
   _DECLARE_DEVFACTORY_TYPE;

   // Device bus methods
   btInt                   (*register_device)( struct aal_device *dev,
                                               pkosal_bus_type bustype );
   void                    (*unregister_device)( struct aal_device *dev );


   struct aal_device *     (*get_device)( struct aal_device_id *devID, btUnsigned32bitInt mask );
   struct aal_device *     (*handle_to_device)( void *handle );

   // Driver registration methods
   btInt                   (*init_driver)(kosal_ownermodule      *powner,
                                          struct aal_driver      *driver,
                                          struct aal_classdevice *pclassdev,
                                          const char             *devname,
                                          btInt                   devmajor);
   btInt                   (*release_driver)(struct aal_driver      *driver,
                                             struct aal_classdevice *pclassdev);
   btInt                   (*register_driver)(struct aal_driver *driver);
   void                    (*unregister_driver)(struct aal_driver *driver);

   // Class registration methods
   btInt                   (*register_class_device)( struct aal_classdevice *classdevice );
   void                    (*unregister_class_device)( struct aal_classdevice *classdevice );

   // Service interface methods  // TODO add event handler for deferred allocation as well as shutdown notifications
   btInt                   (*register_service_interface)( struct aal_interface *pinterface );
   btInt                   (*unregister_service_interface)( struct aal_interface *pinterface );
   btInt                   (*unregister_service_interface_id)(btIID iid);
   struct aal_interface *  (*get_service_interface)(btIID iid);
   void                    (*release_service_interface)( struct aal_interface *pinterface );
   btInt                   (*has_interface)(btIID iid);

   btInt                   (*register_config_update_handler)(aalbus_event_config_update_t EventHandler,
                                                             btObjectType context );
#if 0
   btInt                   (*walk_device_chain)(void);
#endif
   btInt                   (*send_config_update_event)( struct aal_device *pdev,
                                                        krms_cfgUpDate_e updateType,
                                                        btPID pid );
//   btInt                   (*send_uevent)(struct aal_device *dev, enum kobject_action act, char *env[] );

   btInt                   (*dev_is_valid)(struct aal_device *);

   struct aal_bus *        m_parent;
};



//=============================================================================
//               Default bus method implementations
// These methods are set by the aal_bus_init() function to insure that a valid
// implementation exists for all member pointers.
//
// These defaults are primarily intended for bus drivers that are derived from
// aal_bus.  NOTE that several of these defaults call the aal_bus base instance.
// The aal_bus_type singleton driver instance MUST implement these functions
// aal_bus cannot use the default as infinite recursion will occur!
//=============================================================================
static inline
btInt
aalbus_def_register_device(struct aal_device *dev,
                           pkosal_bus_type    bustype )
{
   return aalbus_get_bus()->register_device(dev, bustype);
}
static inline
void
aalbus_def_unregister_device(struct aal_device *dev)
{
   UNREFERENCED_PARAMETER(dev);
   return;
}
static inline
struct aal_device *
aalbus_def_get_device(struct aal_device_id *devID, btUnsigned32bitInt mask)
{
   UNREFERENCED_PARAMETER(devID);
   UNREFERENCED_PARAMETER(mask);
   return NULL;
}
static inline
struct aal_device *
aalbus_def_handle_to_device(btObjectType handle)
{
   UNREFERENCED_PARAMETER(handle);
   return NULL;
}
static inline
btInt
aalbus_def_init_driver(kosal_ownermodule      *powner,
                       struct aal_driver      *driver,
                       struct aal_classdevice *pclassdev,
                       const char             *devname,
                       btInt                   devmajor)
{
   UNREFERENCED_PARAMETER(powner);
   UNREFERENCED_PARAMETER(driver);
   UNREFERENCED_PARAMETER(pclassdev);
   UNREFERENCED_PARAMETER(devname);
   UNREFERENCED_PARAMETER(devmajor);
   return -EINVAL;
}
static inline
btInt
aalbus_def_release_driver(struct aal_driver      *driver,
                          struct aal_classdevice *pclassdev)
{
   UNREFERENCED_PARAMETER(driver);
   UNREFERENCED_PARAMETER(pclassdev);
   return -EINVAL;
}
static inline
btInt
aalbus_def_register_driver(struct aal_driver *driver)
{
   UNREFERENCED_PARAMETER(driver);
   return -EINVAL;
}
static inline
void
aalbus_def_unregister_driver(struct aal_driver *driver)
{
   UNREFERENCED_PARAMETER(driver);
   return;
}
static inline
btInt
aalbus_def_register_class_device(struct aal_classdevice *classdevice)
{
   return aalbus_get_bus()->register_class_device( classdevice );
}
static inline
void
aalbus_def_unregister_class_device( struct aal_classdevice *classdevice )
{
   UNREFERENCED_PARAMETER(classdevice);
   return;
}
static inline
btInt
aalbus_def_register_service_interface(struct aal_interface *pinterface)
{
   UNREFERENCED_PARAMETER(pinterface);
   return -EINVAL;
}
static inline
btInt
aalbus_def_unregister_service_interface(struct aal_interface *pinterface)
{
   UNREFERENCED_PARAMETER(pinterface);
   return -EINVAL;
}
static inline
struct aal_interface *
aalbus_def_get_service_interface(btIID iid)
{
   UNREFERENCED_PARAMETER(iid);
   return NULL;
}
static inline
void
aalbus_def_release_service_interface(struct aal_interface *pinterface)
{
   UNREFERENCED_PARAMETER(pinterface);
   return;
}
static inline
btInt
aalbus_def_has_interface(btIID iid)
{
   UNREFERENCED_PARAMETER(iid);
   return -EINVAL;
}
static inline
btInt
aal_bus_def_register_config_update_handler(aalbus_event_config_update_t EventHandler,
                                           btObjectType                 context)
{
   UNREFERENCED_PARAMETER(EventHandler);
   UNREFERENCED_PARAMETER(context);
   return -EINVAL;
}

#if 0
static inline
btInt
aal_bus_def_walk_device_chain(void)
{
   return -EINVAL;
}
#endif

static inline
btInt
aal_bus_def_send_config_update_event(struct aal_device *pdev,
                                     krms_cfgUpDate_e   updateType,
                                     btPID              pid)
{
   // Send to root AAL Bus
   return aalbus_get_bus()->send_config_update_event(pdev, updateType, pid );
}

#if 0
static inline
btInt
aalbus_def_send_uevent(struct aal_device *dev, enum kobject_action act, char *env[] )
{
   UNREFERENCED_PARAMETER(dev);
   UNREFERENCED_PARAMETER(dev);
   UNREFERENCED_PARAMETER(dev);
   return 0;
}
#endif

inline static
btInt
aalbus_def_validate_device(struct aal_device *ignored)
{
   UNREFERENCED_PARAMETER(ignored);
   return 0;
}
//=============================================================================
// Name: aal_bus_init
// Description: Insures aal_bus in an initialized state
// Interface: private
// Inputs: pbus - aal_bus structure.
// Outputs: none.
// Comments: NOTE the aal_bus driver singleton MUST implement some of these
//           methods (override).  Derived busses may choose not to implement
//           all methods
//=============================================================================
static inline
void
aal_bus_init(struct aal_bus *pbus)
{
   memset(pbus, 0, sizeof(struct aal_bus));
   pbus->register_device               = aalbus_def_register_device;
   pbus->unregister_device             = aalbus_def_unregister_device;
   // MEGAMERGE: create_device --> INIT_FACT_CREATE_DEVICE, and destroy_device --> INIT_FACT_DESTROY_DEVICE
   pbus->INIT_FACT_CREATE_DEVICE       = aaldev_def_create_device;
   pbus->INIT_FACT_DESTROY_DEVICE      = aaldev_def_destroy_device;
   pbus->get_device                    = aalbus_def_get_device;
   pbus->handle_to_device              = aalbus_def_handle_to_device;
   pbus->init_driver                   = aalbus_def_init_driver;
   pbus->release_driver                = aalbus_def_release_driver;
   pbus->register_driver               = aalbus_def_register_driver;
   pbus->unregister_driver             = aalbus_def_unregister_driver;
   pbus->register_class_device         = aalbus_def_register_class_device;
   pbus->unregister_class_device       = aalbus_def_unregister_class_device;
   pbus->register_service_interface    = aalbus_def_register_service_interface;
   pbus->unregister_service_interface  = aalbus_def_unregister_service_interface;
   pbus->get_service_interface         = aalbus_def_get_service_interface;
   pbus->release_service_interface     = aalbus_def_release_service_interface;
   pbus->has_interface                 = aalbus_def_has_interface;
   pbus->register_config_update_handler = aal_bus_def_register_config_update_handler;
//   pbus->walk_device_chain             = aalbus_walk_device_chain;
   pbus->release_service_interface     = aalbus_def_release_service_interface;
   pbus->send_config_update_event      = aal_bus_def_send_config_update_event;
//   pbus->send_uevent                   = aalbus_def_send_uevent;

   pbus->m_version                     = AAL_BUS_VERSION;
}


//=============================================================================
// Name: aal_bus_sane
// Description: Insures aal_bus is complete with no unset interface pointers
// Interface: private
// Inputs: pbus - aal_bus structure.
// Outputs: Sets uninitialized fields
// Comments:
//=============================================================================
//=============================================================================
static inline
void
aal_bus_sane(struct aal_bus *pbus)
{
   if(!pbus->register_device){
      pbus->register_device = aalbus_def_register_device;
   }

   if(!pbus->unregister_device){
      pbus->unregister_device = aalbus_def_unregister_device;
   }

   if(!pbus->get_device){
      pbus->get_device = aalbus_def_get_device;
   }

   if(!pbus->INIT_FACT_CREATE_DEVICE){
      pbus->INIT_FACT_CREATE_DEVICE = aaldev_def_create_device;
   }

   if(!pbus->INIT_FACT_DESTROY_DEVICE){
      pbus->INIT_FACT_DESTROY_DEVICE = aaldev_def_destroy_device;
   }

   if(!pbus->handle_to_device){
      pbus->handle_to_device = aalbus_def_handle_to_device;
   }

   if ( !pbus->init_driver ) {
      pbus->init_driver = aalbus_def_init_driver;
   }

   if ( !pbus->release_driver ) {
      pbus->release_driver = aalbus_def_release_driver;
   }

   if(!pbus->register_driver){
      pbus->register_driver = aalbus_def_register_driver;
   }

   if(!pbus->unregister_driver){
      pbus->unregister_driver = aalbus_def_unregister_driver;
   }

   if(!pbus->register_class_device){
      pbus->register_class_device = aalbus_def_register_class_device;
   }

   if(!pbus->unregister_class_device){
      pbus->unregister_class_device = aalbus_def_unregister_class_device;
   }

   if(!pbus->register_service_interface){
      pbus->register_service_interface = aalbus_def_register_service_interface;
   }

   if(!pbus->unregister_service_interface){
      pbus->unregister_service_interface = aalbus_def_unregister_service_interface;
   }

   if(!pbus->get_service_interface){
      pbus->get_service_interface = aalbus_def_get_service_interface;
   }

   if(!pbus->release_service_interface){
      pbus->release_service_interface  = aalbus_def_release_service_interface;
   }

   if(!pbus->has_interface){
      pbus->has_interface = aalbus_def_has_interface;
   }

   if(!pbus->register_config_update_handler){
      pbus->register_config_update_handler = aal_bus_def_register_config_update_handler;
   }

   if(!pbus->send_config_update_event){
      pbus->send_config_update_event = aal_bus_def_send_config_update_event;
   }
#if 0
   if( !pbus->send_uevent){
      pbus->send_uevent = aalbus_def_send_uevent;
   }
#endif
   if( !pbus->dev_is_valid){
      pbus->dev_is_valid = aalbus_def_validate_device;
   }

}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            AAL Driver
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//=============================================================================
// Name: aal_driver
// Description: AAL specific driver object definition
//=============================================================================
struct aal_driver {
#if (1 == ENABLE_CANARIES)
#define _struct_aal_driver_canary_size              20
#define _struct_aal_driver_start_canary_const       "s:struct aal_driver"
#define _struct_aal_driver_define_start_canary      char _struct_aal_driver_start_canary[_struct_aal_driver_canary_size]
#define _struct_aal_driver_end_canary_const         "e:struct aal_driver"
#define _struct_aal_driver_define_end_canary        char _struct_aal_driver_end_canary[_struct_aal_driver_canary_size]
#define _struct_aal_driver_start_canary_initializer { 's', ':', 's', 't', 'r', 'u', 'c', 't', ' ', 'a', 'a', 'l', '_', 'd', 'r', 'i', 'v', 'e', 'r', 0 },
#define _struct_aal_driver_end_canary_initializer   { 'e', ':', 's', 't', 'r', 'u', 'c', 't', ' ', 'a', 'a', 'l', '_', 'd', 'r', 'i', 'v', 'e', 'r', 0 }
   _struct_aal_driver_define_start_canary;
#else
#define _struct_aal_driver_start_canary_initializer
#define _struct_aal_driver_end_canary_initializer
#endif // ENABLE_CANARIES

#define AAL_DRIVER_FLAG_CHRDEV_REGION_OBTAINED 0x00000001
#define AAL_DRIVER_FLAG_IS_REGISTERED          0x00000002
#define AAL_DRIVER_FLAG_CHRDEV_ADDED           0x00000004
   btUnsigned16bitInt         m_flags;

   // List of sessions owned by this driver
   kosal_list_head            m_sesslist;

   // Return an interface pointer based on the iid. Used to dynamically
   // bind to a custom interface the driver may export. Typically used to
   // export the internal control interface used by a user space interface
   // driver.
   struct aal_interface *(*get_interface)(struct aal_driver* drv, btIID iid);

   // Return 1 if interface supported
   btInt (*has_interface)(struct aal_driver* drv, btIID iid);

   // Returns 1 if the interface specified in iid can be used. Typically
   // used to find out if a user space interface driver can "speak" a particular
   // interface.
   btInt (*supports_interface)(struct aal_driver* drv, btIID iid);

   // Char device file operations interface methods for user space interface
   // or may be used to hold the internal interface of a device driver
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


   // List of implemented dynamic interfaces
   btIID                      *m_iids;

   // ID list of supported devices if this is a device driver
   const struct aal_device_id *m_idtable;

   // Called to determine if this driver supports a device
   btInt (*m_match)(struct aal_driver* drv, struct aal_device* dev);

   // Called to initialize the hardware
   btInt (*m_probe)(struct aal_device* dev);

   // AAL Bus interface
   struct   aal_bus           *m_bus;
#if defined( __AAL_LINUX__ )
   //Generic device driver base
   struct device_driver        m_driver;
#endif // __AAL_LINUX__

#if (1 == ENABLE_CANARIES)
   _struct_aal_driver_define_end_canary;
#endif // ENABLE_CANARIES
};

#define aaldrv_is_chrdev_region_obtained(drv)  ((drv)->m_flags & AAL_DRIVER_FLAG_CHRDEV_REGION_OBTAINED)
#define aaldrv_set_chrdev_region_obtained(drv) ((drv)->m_flags |= AAL_DRIVER_FLAG_CHRDEV_REGION_OBTAINED)
#define aaldrv_clr_chrdev_region_obtained(drv) ((drv)->m_flags &= ~AAL_DRIVER_FLAG_CHRDEV_REGION_OBTAINED)

#define aaldrv_is_registered(drv)     ((drv)->m_flags & AAL_DRIVER_FLAG_IS_REGISTERED)
#define aaldrv_set_is_registered(drv) ((drv)->m_flags |= AAL_DRIVER_FLAG_IS_REGISTERED)
#define aaldrv_clr_is_registered(drv) ((drv)->m_flags &= ~AAL_DRIVER_FLAG_IS_REGISTERED)

#define aaldrv_is_chrdev_added(drv)   ((drv)->m_flags & AAL_DRIVER_FLAG_CHRDEV_ADDED)
#define aaldrv_set_chrdev_added(drv)  ((drv)->m_flags |= AAL_DRIVER_FLAG_CHRDEV_ADDED)
#define aaldrv_clr_chrdev_added(drv)  ((drv)->m_flags &= ~AAL_DRIVER_FLAG_CHRDEV_ADDED)

//-----------------------------
// aal_driver casting operators
//-----------------------------
#define base_to_aaldrv(drv)      (container_of( drv, struct aal_driver, m_driver ) )
#define aaldrv_to_basep(drv)     (&(drv)->m_driver)
#define aaldrv_to_base(drv)      ((drv)->m_driver)
#define aaldrv_to_base_busp(drv) (aaldrv_to_base(drv).bus)


//-----------------------
// aal_driver Accesssors
//-----------------------
#define aaldrv_sess_list(drv) ((drv)->m_sesslist)
#define aaldrv_fops(drv)      ((drv)->m_fops)
#define aaldrv_fopsp(drv)     (&(drv)->m_fops)
#define aaldrv_cdevp(drv)     (&(drv)->m_cdev)
#define aaldrv_dev_major(drv) ((drv)->m_major)
#define aaldrv_iids(drv)      ((drv)->m_iids)
#define aaldrv_aalbus(drv)    (*((drv)->m_bus))
#define aaldrv_aalbusp(drv)   ((drv)->m_bus)

//=============================================================================
// Name: aal_driver_init
// Description: Initialize the structure
//=============================================================================
#if 0
// Most drivers embed a subset. Careful about wiping it clean here.
#define aal_driver_init(p)                  \
do                                          \
{                                           \
   memset(p, 0, sizeof(struct aal_driver)); \
   kosal_list_init((&(p)->m_sesslist);      \
}while(0)
#endif


/** @brief wrapper for checking valid device structure
 * @param[in] pdev pointer to the device to check
 * @return non-zero if pdev appears to be a valid device
 *
 * Checking the validity of a device requires an aalbus, so can't be performed
 * inside the aalbus-device.h.  This inline exists for backwards-compatiblity.
 * If the caller already has a reference to aalbus, it can invoke the check
 * directly.  */
inline static int
aaldev_valid(struct aal_device *pdev) {
   struct aal_bus *aalbus_p;

   aalbus_p = aalbus_get_bus();

   ASSERT(NULL != aalbus_p);
   if ( NULL == aalbus_p ) {
      return 0;
   }

   ASSERT(NULL != aalbus_p->dev_is_valid);
   if ( NULL == aalbus_p->dev_is_valid ) {
      return 0;
   }

   return aalbus_p->dev_is_valid(pdev);
}


/** @brief casting from btObjectType to aal_device, with sanity checking
 * @param[in] handle_p pointer to handle to convert to aal_device
 * @return pointer to struct aal_device on success, NULL on failure */
static inline
struct aal_device *aaldev_handle_to_devp(btObjectType handle)
{
   /* this will break if btObjectType becomes something other than a (void *) */
   struct aal_device *dev_p = handle;

   if (0 == aaldev_valid(dev_p)) {
      return NULL;
   } else {
      return dev_p;
   }
}


/** @brief casting from aal_device to btObject, with sanity checking
 * @param[in] dev_p pointer to aal_device to convert to handle
 * @return pointer to btObjectType on success, NULL on failure */
static inline
btObjectType aaldevp_to_devhandle(struct aal_device *dev_p)
{
   if (0 == aaldev_valid(dev_p)) {
      return NULL;
   } else {
      /* this will break if btObjectType becomes something other than a
       * (void *) */
      return dev_p;
   }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            AAL Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// casting macros
#define base_to_aal_bus(bus) (container_of( bus, struct aal_bus_type, m_bustype ) )
#define ibus_to_aal_bus(bus) (container_of( bus, struct aal_bus_type, m_ibus ) )

//#define base_to_aal_dev(dev) (container_of( dev, struct aal_device, m_dev ) )

#endif // __AALSDK_KERNEL_AALBUS_H__

