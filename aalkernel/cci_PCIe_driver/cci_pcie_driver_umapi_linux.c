//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
//        FILE: cci_pcie_driver_umapi_linux.c
//     CREATED: 10/20/2015
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the main startup and shutdown code for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           User Mode Interface for the AAL CCI device driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/20/2015     JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS UIDRV_DBG_MOD

#include "cci_pcie_driver_umapi_linux.h"
#include "cci_pcie_driver_internal.h"
//#include "cciui-events.h"
//#include "aalsdk/kernel/aalui-events.h"


//////////////////////////////////////////////////////////////////////////////////////

// Major device number to use for the device nodes
btInt majornum = 0;

//////////////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Driver Parameters
//=============================================================================

//
//  Declarations for module parameters - Enables parameter value passing from
//                                       insmod and permissions as seen from /sys
//
MODULE_PARM_DESC(majornum, "major device number");
module_param    (majornum, int, 0444);

// Prototypes
btInt ccidrv_open(struct inode *, struct file *);
btInt ccidrv_close(struct inode *, struct file *);

btInt ccidrv_mmap(struct file *, struct vm_area_struct *);
btUnsignedInt ccidrv_poll(struct file *, poll_table *);

#if HAVE_UNLOCKED_IOCTL
long ccidrv_ioctl(struct file *file,
                 unsigned int cmd,
                 unsigned long arg);
#else
int ccidrv_ioctl(struct inode *inode,
                struct file *file,
                unsigned int cmd,
                unsigned long arg);
#endif

extern btInt ccidrv_messageHandler( struct ccidrv_session   *psess,
                                    btUnsigned32bitInt       cmd,
                                    struct ccipui_ioctlreq   *preq,
                                    btWSSize                 InbufSize,
                                    struct ccipui_ioctlreq   *presp,
                                    btWSSize                *pOutbufSize);



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
// Name: ui_driver
// Description: This is the UI Driver Object singleton. This is
//              object that gets registered with AALBus.  It is a device driver
//              in name only. It does not actually control any device HW.
//              As a device driver module it is allowed to expose a user mode
//              interface.
//=============================================================================
struct um_APIdriver thisDriver = {

      .m_fops = {
         .owner          = THIS_MODULE,
         .poll           = ccidrv_poll,
#if HAVE_UNLOCKED_IOCTL
         .unlocked_ioctl = ccidrv_ioctl,
#else
         .ioctl          = ccidrv_ioctl,  // Deprecated in 2.6.36
#endif
         .mmap           = ccidrv_mmap,
         .open           = ccidrv_open,
         .release        = ccidrv_close,
      },
};

struct um_driver *_um_driver = &thisDriver.m_common;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              UMAPI METHODS               ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: ccidrv_initUMAPI
// Description: Initialization routine for the module. Registers with the bus
//              driver
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
int
ccidrv_initUMAPI(void)
{
   int res                 = 0;

   char * devname          = "uidrv";

   PVERBOSE("Installing CCI Driver API\n");
   //---------------------------
   // Initialize data structures
   //---------------------------
   kosal_mutex_init(&thisDriver.m_common.m_qsem);
   kosal_list_init(&thisDriver.m_common.m_sessq);
   kosal_mutex_init(&thisDriver.m_common.m_sem);

   kosal_mutex_init(&thisDriver.m_common.wsid_list_sem);
   kosal_list_init(&thisDriver.m_common.wsid_list_head);

   PDEBUG("Allocating major number for \"%s\"\n",devname);

   res = alloc_chrdev_region(&thisDriver.m_devtype, 0, 1, devname);

   if ( res < 0 ) {
      PERR("Failed to allocate major device number for \"%s\"\n", devname);
      return res;
   }

   PDEBUG("Using major number %d for \"%s\"\n", MAJOR(thisDriver.m_devtype), devname);

   cdev_init(&thisDriver.m_cdev, &thisDriver.m_fops);
   thisDriver.m_cdev.ops   = &thisDriver.m_fops;
   thisDriver.m_cdev.owner = THIS_MODULE;

  res = cdev_add(&thisDriver.m_cdev, thisDriver.m_devtype, 1);
   if ( res ) {
     PERR("Failed to register character device : ret = %d\n", res);
     goto ERROR;
   }

   thisDriver.m_class = class_create(THIS_MODULE, devname);
   if(NULL == thisDriver.m_class){
      PERR("Could Not create class device\n");
            goto ERROR;
   }

   thisDriver.m_device = device_create(thisDriver.m_class, NULL, thisDriver.m_devtype, "%s", devname);

   return res;

   ERROR:
      device_destroy(thisDriver.m_class, thisDriver.m_devtype);
      cdev_del(&thisDriver.m_cdev);
      class_destroy(thisDriver.m_class);
      unregister_chrdev_region(thisDriver.m_devtype, 1);
      return res;

}


//=============================================================================
// Name: ccidrv_exitUMAPI
// Description: Removes device from filesystem and registration
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void ccidrv_exitUMAPI(void)
{

   PVERBOSE(" Exiting\n");

   // TODO FLUSH ALL Messages

   device_destroy(thisDriver.m_class, thisDriver.m_devtype);
   cdev_del(&thisDriver.m_cdev);
   class_destroy(thisDriver.m_class);
   unregister_chrdev_region(thisDriver.m_devtype, 1);

}

//=============================================================================
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          CCI SYSTEM CALL INTERFACE       ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalccidrv_ioctl
// Description: Implements the ioctl system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: Entry point for all requests from user space
//=============================================================================
#if HAVE_UNLOCKED_IOCTL
long ccidrv_ioctl(struct file *file,
                 unsigned int cmd,
                 unsigned long arg)
#else
int ccidrv_ioctl(struct inode *inode,
                struct file *file,
                unsigned int cmd,
                unsigned long arg)
#endif

{
   struct ccidrv_session *psess = (struct ccidrv_session *) file->private_data;

   // Generic variables
   int                     ret=0;
   struct ccipui_ioctlreq  req;                       // User IOCTL header

   struct ccipui_ioctlreq *pfullrequest      = NULL;  // Full message with var data
   btWSSize                FullRequestSize   = 0;     // Size of the user buffer (header + payload)
   btWSSize                Outbufsize        = 0;     // Size of usable return payload buffer
   struct ccipui_ioctlreq *pfullresponse     = NULL;  // Buffer to put return response if any.

   ASSERT(NULL != psess );
   if ( NULL == psess ) {
      PERR("No session for message\n");
      return -EINVAL;
   }

   //---------------------
   // Get the user request
   //---------------------
   // Read header
   if ( copy_from_user(&req, (void *)arg, sizeof(req)) ) {
      return -EFAULT;
   }

   // Total user buffer size is the size of the header structure ccipui_ioctlreq + payload size
   FullRequestSize = (sizeof(struct ccipui_ioctlreq)) + aalui_ioctlPayloadSize(&req);

   // If there is a payload then allocate a bige enough buffer and copy it in.
   if ( FullRequestSize > sizeof(struct ccipui_ioctlreq) ) {

      PINFO("UIDRV is reading message with payload of size %" PRIu64 "\n", aalui_ioctlPayloadSize(&req));
      pfullrequest = (struct ccipui_ioctlreq *) kosal_kzmalloc(FullRequestSize);

      // Read whole message
      if ( copy_from_user(pfullrequest, (void *)arg, FullRequestSize) ) {
         kosal_kfree(pfullrequest, FullRequestSize);
         kosal_kfree(pfullresponse, FullRequestSize);
         return -EFAULT;
      }
   } else {
      //Header is all there is. No need to read it again. Just point to req earlier.
      pfullrequest = &req;
   }

   // Allocate a temporary buffer for the response. Note that for simplicity the payload
   //   of the original request will be used for the response as well. I.e., The response buffer
   //   is the same size as the request buffer.  The response will be copied over the original
   //   user mode request.
   //-------------------------------------------------------------------------------------------
   pfullresponse = (struct ccipui_ioctlreq *) kosal_kzmalloc(FullRequestSize);

   // Limit on response payload.  This will be changed by the request processor to the actual return size
   //  or zero if no response data.
   Outbufsize = aalui_ioctlPayloadSize(&req);

   *pfullresponse = req;   // Copy the original header for the response and change as needed
   // Pass the message to OS independent processing. Note that some functions that don't return a
   // payload use only the request header to return their data. So the whole response buffer must be passed.
   ret = ccidrv_messageHandler(psess,
                               cmd,
                               pfullrequest,
                               FullRequestSize,
                               pfullresponse,                       // Pointer to output response
                               &Outbufsize);                        // Outbuf buffer size

   if(0 == ret){

      btWSSize FullResponseSize = sizeof(struct ccipui_ioctlreq) + Outbufsize;

      // Copy the Response back.
      PINFO("UIDRV is writing %" PRIu64 "-byte response message with payload of size %" PRIu64 " bytes with %llx\n", FullResponseSize, pfullresponse->size, (btWSID)(*pfullresponse->payload));
      ret = copy_to_user((void*)arg, pfullresponse, FullResponseSize);

   }else{
      PDEBUG("ccidrv_messageHandler failed\n");
      ret = -EINVAL;
   }

   // Free response buffer
   if( NULL != pfullresponse){
         kosal_kfree( pfullresponse, FullRequestSize);
   }

   // Free message copy if it had a payload
   if( &req != pfullrequest ) {
      kosal_kfree(pfullrequest, FullRequestSize);
   }

   return ret;
}


//=============================================================================
// Name: ccidrv_open
// Description: Implements the open system call
// Interface: public
// Inputs: inode - pointer to inode for device node
//         file - pointer to file instance for this open
// Outputs: none.
// Comments: Creates a per process session instance with the device control
//           subsystem.The session holds the state and context between an
//           application process, the device and associated servciecs (e.g,
//           workspace manager).
//           The UDDI maintains a list of all open sessions.
//=============================================================================
int ccidrv_open  (struct inode *inode, struct file *file)
{
   struct ccidrv_session *psess = NULL;
   int ret = 0;

   DPRINTF (UIDRV_DBG_FILE, ":Opened by pid = %d tgid = %d\n",current->pid, current->tgid );

   // Create a session
   psess = ccidrv_session_create( current->tgid );
   if( unlikely( psess == NULL ) ) {
      DPRINTF (UIDRV_DBG_FILE, "Create session failed.\n" );
      return -ENOMEM;
   }

   file->private_data = psess;

   // Add it to the list of sessions
   if (kosal_sem_get_krnl_alertable( &thisDriver.m_common.m_qsem)) { /* FIXME */ }
   list_add_tail( &psess->m_sessions, &thisDriver.m_common.m_sessq);
   up( &thisDriver.m_common.m_qsem);

   DPRINTF (UIDRV_DBG_FILE, "Application Session Created sess=%p\n", psess);
   return ret;
}

//=============================================================================
// Name: ccidrv_close
// Description: Implements the close system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: Close pulls the plug on any outstanding transactions. This implies
//           that notifications for completion may not be sent to the
//           application. Ideally the app is in a quiescent state before
//           calling.
//=============================================================================
int ccidrv_close (struct inode *inode, struct file *file)
{
   struct ccidrv_session *psess = file->private_data;
   file->private_data = NULL;
   DPRINTF (UIDRV_DBG_FILE, ": Closing session %p\n",psess);

   return ccidrv_session_destroy(psess);
}

//=============================================================================
// Name: ccidrv_poll
// Description: Called from select, poll or epoll call.
// Interface: public
// Inputs: .
// Outputs: none.
// Comments:
//=============================================================================
unsigned int ccidrv_poll ( struct file *file, poll_table *wait )
{
   struct ccidrv_session *psess = ( struct ccidrv_session * ) file->private_data;
   unsigned int mask = 0;

   // Put session's waitq in the poll table
   poll_wait ( file, &psess->m_waitq, wait );

   // If there is a request on the queue wakeup sleeper
   if (kosal_sem_get_krnl_alertable( &psess->m_sem )) { /* FIXME */ }
   if( !_aal_q_empty(&psess->m_eventq) ){
      DPRINTF( UIDRV_DBG_FILE, ": Message available. Waking sleepers\n" );
      mask |= POLLPRI;  // Device request completion
   }
   up ( &psess->m_sem );

   return mask;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////               MMAP METHOD                 ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================


//=============================================================================
// Name: ccidrv_mmap
// Description: mmap system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: The mmap system call parameter "offset" (aka vm_pgoff) has been
//           overloaded to mean Workspace ID (wsid).  Because the mmap() call
//           expects a page aligned offset AND the kernel page aligns the
//           vm_pgoff value, the wsid (an unsigned long long) is encoded into
//           a page aligned value.
//=============================================================================
int
ccidrv_mmap  (struct file *file, struct vm_area_struct *vma)
{
   struct aaldev_ownerSession    *ownerSessp = NULL;
   struct aal_wsid *wsidp = NULL;
   struct ccidrv_session * psess = NULL;
   struct aal_device *pdev = NULL;

   PTRACEIN;
   PVERBOSE("In UIDRV  MMAP\n");

   //////////////////////////////////////////////////////////////////////////////////
   if(vma->vm_pgoff == 0 ) {
      DPRINTF( UIDRV_DBG_MMAP, "Invalid WSID\n");
      goto failed;
   }

   /* session information is squirreled away in our private data */
   psess = (struct ccidrv_session *) file->private_data;
   if (NULL == psess) {
      DPRINTF( UIDRV_DBG_MMAP, "Invalid session\n");
      goto failed;
   }
   PDEBUG("WSID offset %lu  handle is %llx\n",vma->vm_pgoff, pgoff_to_wsidHandle(vma->vm_pgoff));

   /* check wsidp vs known list of wsids */
   wsidp = find_wsid(psess, pgoff_to_wsidHandle(vma->vm_pgoff));
   if (NULL == wsidp) {
      DPRINTF( UIDRV_DBG_MMAP, "WSID not found on list owned WSIDs\n");
      goto failed;
   }

   // pull the aal_device out of the workspace
   pdev = wsidp->m_device;
   if(unlikely(!aaldev_valid(pdev))){
      DPRINTF( UIDRV_DBG_MMAP, "Invalid WSID\n");
      goto failed;
   }

   // Get the device session
   ownerSessp = dev_OwnerSession(pdev,psess->m_pid);
   if(unlikely(ownerSessp == NULL)){
      DPRINTF( UIDRV_DBG_MMAP, "Not device owner\n");
      goto failed;
   }

   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS pgoff = %lx \n", vma->vm_pgoff);
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS device = %p tid = %d\n", pdev,psess->m_pid );
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS %p id = 0x%llx.\n", wsidp,wsidp->m_id);

   //==================================================
   // Check for permission and correctness of interface
   //==================================================
#if 0
   // Move this code to pip-specific mmap functions AND
   // add intelligence about type of memory being mapped
   // e.g. && workspaceid.type == CSR // then fail
   // that is, CSR mapping is disabled but allow buffer mapping
   // or -- if CSR mapping is allowed but in multi-process mode then they would
   // not be allowed.
   if(!aaldev_allowsDirectAPI(pdev)){
      DPRINTF( UIDRV_DBG_MMAP, "Direct PIP access not allowed on this device.\n");
      goto failed;
   }
#endif

   if(!aaldev_haspip(pdev)) {
      DPRINTF( UIDRV_DBG_MMAP, "Device has no PIP.\n");
      goto failed;

   }

   if(!aalpip_hasmmap( aaldev_pipp(pdev) ) ){
      DPRINTF( UIDRV_DBG_MMAP, "Device PIP does not support mmap.\n");
      goto failed;
   }

   //------------------------------------
   // Call the device's PIP::fop:mmap
   //------------------------------------

   // Call through the device PIP to the mmap() method
   if( aalpip_mmap( aaldev_pipp(pdev) )( ownerSessp,
                                         wsidp,
                                         vma ) < 0) {
       DPRINTF( UIDRV_DBG_MMAP, "Mmap WS 0x%llx Failed.\n", wsidp->m_id);
       goto failed;
   }
   DPRINTF( UIDRV_DBG_MMAP, "Mmap WS Success.\n");
   return 0;

failed:
   return -EINVAL;

}

