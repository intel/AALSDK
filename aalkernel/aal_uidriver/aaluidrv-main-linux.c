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
//        FILE: aaluidrv-main-linux.c
//     CREATED: 08/26/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the main startup and shutdown code for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           Universal Interface Driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 08/26/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/14/2009     JG       Cleanup and refactoring
// 02/24/2009     JG       Removed unused match()
// 02/11/2010     JG       Support for kernel 2.6.31
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
// 02/26/2013     AG       Add wsid tracking and validation routines
// 09/03/2013     JG       Renamed to aaluidrv-main-linux from aaluidrv-main
//                           to reflect that this file will hold OS specific
//                           code.
//****************************************************************************
#define MODULE_FLAGS UIDRV_DBG_MOD

#include "aalsdk/kernel/kosal.h"
#include "aaluidrv-int.h"

//////////////////////////////////////////////////////////////////////////////////////

// Major device number to use for the device nodes
btInt majornum = 0;

//////////////////////////////////////////////////////////////////////////////////////
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
  | UIDRV_DBG_MOD
  | UIDRV_DBG_FILE
  | UIDRV_DBG_MMAP
  | UIDRV_DBG_IOCTL
#endif
;

//
//  Declarations for module parameters - Enables parameter value passing from
//                                       insmod and permissions as seen from /sys
//
MODULE_PARM_DESC(debug, "debug level");
module_param    (debug, int, 0770);

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
DRIVER_ATTR(debug,S_IRUGO|S_IWUGO, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);


//
// Module ops
//
static int  uidrv_init(void);
static void uidrv_exit(void);

module_init(uidrv_init);
module_exit(uidrv_exit);

// Prototypes
//static int uidrv_match(struct aal_driver* drv, struct aal_device* dev);
static
struct aal_interface *
uidrv_get_interface(struct aal_driver *drv, btID iid);

static
int
uidrv_has_interface(struct aal_driver *drv, btID iid);

static
int
uidrv_supports_interface(struct aal_driver *drv, btID iid);

#if HAVE_UNLOCKED_IOCTL
long uidrv_ioctl(struct file *file,
                 unsigned int cmd,
                 unsigned long arg);
#else
int uidrv_ioctl(struct inode *inode,
                struct file *file,
                unsigned int cmd,
                unsigned long arg);
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     UNIVERSAL DEVICE DRIVER INTERFACE     ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================



//=============================================================================
// Name: id_table
// Description: This driver supports the following devices
//              The UDDI is a device driver by name only. It is actually the
//              front end to the Physical Interface Protocol modules which
//              are responsible for the actual device control.  The UDDI
//              provides the user mode interface.
//=============================================================================
static struct aal_device_id id_table[] = {
   aal_device_id_terminator // None.
};

// Used for hotplug system
MODULE_DEVICE_TABLE( aal, id_table );

//=============================================================================
// Name: devIID_tbl
// Description: Lists the IDs of the interfaces this service exports to the
//              AALBus interface broker.
//=============================================================================
static btID devIID_tbl[] = {
   0  // None
};

//=============================================================================
// Name: uidrv_class
// Description: Class device. Wrapper object that contains both the Linux
//              DD Model class information but also AAL specific class
//              information. The modules "class" defines its unique interface
//              attributes.
//=============================================================================
static struct aal_classdevice ui_class = {
   .m_classid = {
      .m_majorversion   = AALUI_DRV_MAJVERSION,
      .m_minorversion   = AALUI_DRV_MINVERSION,
      .m_releaseversion = AALUI_DRV_RELEASE,
      .m_classGUID      = AALUI_DRV_INTC,
   },
   .m_devIIDlist = devIID_tbl,        // List of supported device module APIs
};

//=============================================================================
// Name: ui_driver
// Description: This is the UI Driver Object singleton. This is
//              object that gets registered with AALBus.  It is a device driver
//              in name only. It does not actually control any device HW.
//              As a device driver module it is allowed to expose a user mode
//              interface.
//=============================================================================
struct ui_driver thisDriver = {
   .m_aaldriver = {
      // Return an interface pointer based on the iid. Used to dynamically
      // bind to a custom interface the driver may export. Typically used to
      // export the internal control interface used by a user space interface
      // driver.
      .get_interface      = uidrv_get_interface,

      // Return 1 if interface supported
      .has_interface      = uidrv_has_interface,

      // Returns 1 if the interface specified in iid can be used. Typically
      // used to find out if a user space interface driver can "speak" particular
      // interface.
      .supports_interface = uidrv_supports_interface,

      // File operations (User Space control interface)
      // Exposes methods invoked by the user space device driver interface
      .m_fops = {
         .owner          = THIS_MODULE,
         .poll           = uidrv_poll,
#if HAVE_UNLOCKED_IOCTL
         .unlocked_ioctl = uidrv_ioctl,
#else
         .ioctl          = uidrv_ioctl,  // Deprecated in 2.6.36
#endif
         .mmap           = uidrv_mmap,
         .open           = uidrv_open,
         .release        = uidrv_close,
      },

      // List of implemented dynamic interfaces
      .m_iids    = 0,

      // ID list of supported devices if this is a device driver
      .m_idtable = id_table,

      // Base structure
      .m_driver =  {
         .owner = THIS_MODULE,
         .name  = DEV_NAME,
      },
   },
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////               UDDI METHODS               ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: uidrv_init
// Description: Initialization routine for the module. Registers with the bus
//              driver
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static int
uidrv_init(void)
{
   int res = 0;

   // Display the signon
   kosal_printk_level(KERN_INFO, "Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_DESCRIPTION);
   kosal_printk_level(KERN_INFO, "-> Version %s\n",DRV_VERSION);
   kosal_printk_level(KERN_INFO, "-> %s\n", DRV_COPYRIGHT);

   //---------------------------
   // Initialize data structures
   //---------------------------
   kosal_mutex_init(&thisDriver.m_qsem);
   kosal_list_init(&thisDriver.m_sessq);
   kosal_mutex_init(&thisDriver.m_sem);

   kosal_mutex_init(&thisDriver.wsid_list_sem);
   kosal_list_init(&thisDriver.wsid_list_head);

   res = aalbus_get_bus()->init_driver((kosal_ownermodule *)THIS_MODULE,
                                      &thisDriver.m_aaldriver,
                                      &ui_class,
                                       DEV_NAME,
                                       majornum);
   ASSERT(res >= 0);

   if(driver_create_file(&thisDriver.m_aaldriver.m_driver,&driver_attr_debug)){
       DPRINTF (AHMPIP_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
       // Unregister the driver with the bus
       aalbus_get_bus()->unregister_driver( &thisDriver.m_aaldriver );
       return -EIO;
   }

   return res;
}


//=============================================================================
// Name: uidrv_exit
// Description: Removes device from filesystem and registration
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static
void
uidrv_exit(void)
{
#if (1 == ENABLE_ASSERT)
   int res;
#endif // ENABLE_ASSERT

   kosal_printk_level(KERN_INFO, "<- Exiting\n");

   // TODO FLUSH ALL Messages

#if (1 == ENABLE_ASSERT)
   res =
#endif // ENABLE_ASSERT
   aalbus_get_bus()->release_driver(&thisDriver.m_aaldriver, &ui_class);

   ASSERT(res >= 0);

   kosal_printk_level(KERN_INFO, "<- %s removed\n", DRV_DESCRIPTION);
}

//=============================================================================
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////         USER MODE MESSAGE INTERFACE      ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aaluidrv_ioctl
// Description: Implements the ioctl system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: Entry point for all requests from user space
//=============================================================================
#if HAVE_UNLOCKED_IOCTL
long uidrv_ioctl(struct file *file,
                 unsigned int cmd,
                 unsigned long arg)
#else
int uidrv_ioctl(struct inode *inode,
                struct file *file,
                unsigned int cmd,
                unsigned long arg)
#endif

{
   struct uidrv_session *psess = (struct uidrv_session *) file->private_data;

   // Generic variables
   int                    ret=0;
   struct aalui_ioctlreq  req;                 // User IOCTL header

   struct aalui_ioctlreq *pfullrequest = NULL; // Full message with var data
   btWSSize               FullRequestSize;
   btWSSize               Outbufsize;

   DPRINTF(UIDRV_DBG_MOD,"check point arg=0x%p\n", (void *)arg);
   if ( NULL == psess ) {
      PERR("No session for message\n");
      return -EINVAL;
   }
   //---------------------
   // Get the user request - TODO This implementation is inefficient as it requires
   // 2 reads to get the entire contents of message. Should be redesigned  for more
   // efficient transfer using read/write, mmap() or similar (JG)
   //---------------------

   // Read header
   if ( copy_from_user(&req, (void *)arg, sizeof(req)) ) {
      return -EFAULT;
   }

   // Total user buffer size is the size of structure aalui_ioctlreq + payload size
   Outbufsize = FullRequestSize = sizeof(struct aalui_ioctlreq) + aalui_ioctlPayloadSize(&req);

   // Check to see if there is a payload
   if ( FullRequestSize > sizeof(struct aalui_ioctlreq) ) {

      PINFO("UIDRV is reading message with payload of size %" PRIu64 "\n", aalui_ioctlPayloadSize(&req));
      pfullrequest = (struct aalui_ioctlreq *) kosal_kmalloc(FullRequestSize);

      // Read whole message
      if ( copy_from_user(pfullrequest, (void *)arg, FullRequestSize) ) {
         kosal_kfree(pfullrequest, FullRequestSize);
         return -EFAULT;
      }
   } else {
      //Header is all there is
      pfullrequest = &req;
   }

   // Pass the message to OS independent processing
   ret = uidrv_messageHandler(psess,
                              cmd,
                              pfullrequest,
                              FullRequestSize,
                              pfullrequest,   // Pointer to output buffer
                              &Outbufsize);   // Outbuf buffer size

   if ( 0 != ret ) {
      PDEBUG("uidrv_messageHandler failed\n");
   } else {
      // Copy back response if any.
      if ( 0 != Outbufsize ) {
         PINFO("UIDRV is writing %" PRIu64 "-byte response message with payload of size %" PRIu64 " bytes\n", Outbufsize, pfullrequest->size);
         ret = copy_to_user((void*)arg, pfullrequest, Outbufsize);
      }
   }

   // Free message copy if it had a payload
   if ( &req != pfullrequest ) {
      kosal_kfree(pfullrequest, FullRequestSize);
   }

   return ret;
}



//=============================================================================
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              DEPRICATED                  ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name:  uidrv_get_interface
// Description: Returns the requested interface if supported
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: interface pointer.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
struct aal_interface *
uidrv_get_interface(struct aal_driver *drv, btID iid)
{
   return NULL;
}


//=============================================================================
// Name:  uidrv_has_interface
// Description: Reports if this driver implements a specifc interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if interface implemented.
// Comments: This driver does not publish any dynamic interfaces
//=============================================================================
int
uidrv_has_interface(struct aal_driver *drv, btID iid)
{
   return 0;
}


//=============================================================================
// Name:  uidrv_supports_interface
// Description: Report whether this driver can use a specific interface
// Interface: public
// Inputs:  drv - base pointer to this driver
//.         iid - interface ID
// Outputs: 1 - if supported.
// Comments:
//=============================================================================
int
uidrv_supports_interface(struct aal_driver *drv, btID iid)
{
   return AAL_DDAPI_IID_07 == iid;
}
