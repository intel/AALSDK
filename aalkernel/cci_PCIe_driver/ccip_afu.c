//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015, Intel Corporation.
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
//  Copyright(c) 2015, Intel Corporation.
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
/// @file ccip_afu.c
/// @brief  Definitions for ccip User AFU.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_port_mmio.c
//     CREATED: Nov 9, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the implementation of the CCIP AFU
//             low-level function (i.e., Physical Interface Protocol driver).
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "aalsdk/kernel/AALTransactionID_s.h"
#include "aalsdk/kernel/aalbus-ipip.h"

#include "aalsdk/kernel/ccipdriver.h"
#include "ccipdrv-events.h"

#include "ccip_defs.h"
#include "ccip_port.h"
#include "cci_pcie_driver_PIPsession.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////           AAL SUPPORT FUNCTIONS          ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int CommandHandler( struct aaldev_ownerSession *,
                           struct aal_pipmessage);
static int cci_mmap(struct aaldev_ownerSession *pownerSess,
                           struct aal_wsid *wsidp,
                           btAny os_specific);


//=============================================================================
// cci_FMEpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip cci_AFUpip = {
   .m_messageHandler = {
      .sendMessage   = CommandHandler,       // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
     .mmap = cci_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};



///============================================================================
/// Name: cci_create_AAL_UAFU_Device
/// @brief Creates and registers User AFU object (resource) we want to
///        expose through AAL.
///
/// @param[in] pportdev - Port device
/// @param[in] paalid - Base AAL ID for this device.
/// @return    AAL Device pointer
///============================================================================
struct cci_aal_device   *
            cci_create_AAL_UAFU_Device( struct port_device  * pportdev,
                                        struct CCIP_AFU_Header *pafu_hdr,
                                        struct aal_device_id *paalid)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   int ret;

   PTRACEIN;

   PVERBOSE("Instantiating User AFU\n");
   PVERBOSE("User AFU\n");
   PVERBOSE( "> Feature_ID = %x \n",pafu_hdr->ccip_dfh.Feature_ID);
   PVERBOSE("> Feature_rev = %x \n",pafu_hdr->ccip_dfh.Feature_rev);
   PVERBOSE( "> Type = %x \n",pafu_hdr->ccip_dfh.Type);
   PVERBOSE( "> afu_id_l.afu_id_l= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_l.afu_id_l);
   PVERBOSE( "> afu_id_h.afu_id_h= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_h.afu_id_h);
   PVERBOSE( "> next_DFH_offset = %x \n",pafu_hdr->ccip_dfh.next_DFH_offset);
   PVERBOSE( "> next_afu.afu_id_offset= %x \n",pafu_hdr->ccip_next_afu.afu_id_offset);

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   ASSERT(NULL != pcci_aaldev);

   // Make it a User AFU
   cci_dev_type(pcci_aaldev) = cci_dev_AFU;

   // Bind the AAL device to the Port's AFU object. This enables the driver
   //   to get to the Port AFU object from the AAL device.
   set_cci_dev_subclass(pcci_aaldev, &ccip_port_afu_dev(pportdev));

   // Device Address is the same as the Port. Set the AFU ID information
   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(*paalid)            = pafu_hdr->ccip_afu_id_l.afu_id_l;
   aaldevid_afuguidh(*paalid)            = pafu_hdr->ccip_afu_id_h.afu_id_h;
   aaldevid_pipguid(*paalid)             = CCIP_AFU_PIPIID;

   // Set the interface permissions
   // Enable MMIO-R
   cci_dev_set_allow_map_mmior_space(pcci_aaldev);

   // Setup the MMIO region parameters
   cci_dev_kvp_afu_mmio(pcci_aaldev)   = (btVirtAddr)pafu_hdr;
   cci_dev_len_afu_mmio(pcci_aaldev)   = CCI_MMIO_SIZE;
   cci_dev_phys_afu_mmio(pcci_aaldev)  = kosal_virt_to_phys(pafu_hdr);

   // Create the AAL device and attach it to the CCI device object
   pcci_aaldev->m_aaldev =  aaldev_create( "CCIPAFU",          // AAL device base name
                                           &*paalid,             // AAL ID
                                           &cci_AFUpip);

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   pcci_aaldev->m_aaldev->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;
   if( cci_dev_allow_map_csr_read_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cci_dev_allow_map_csr_write_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   if( cci_dev_allow_map_mmior_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

   if( cci_dev_allow_map_umsg_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_UMSG;
   }

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_aaldev_to_aaldev(pcci_aaldev)) = (void*)pcci_aaldev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_aaldev_to_aaldev(pcci_aaldev), cci_release_device);

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pcci_aaldev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                  aaldevid_devaddr_devnum(*paalid),
                                                                  aaldevid_devaddr_fcnnum(*paalid),
                                                                  aaldevid_devaddr_subdevnum(*paalid));
      cci_destroy_aal_device(pcci_aaldev);
      return NULL;
   }

   PTRACEOUT;
   return pcci_aaldev;
}



//=============================================================================
// Name: CommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
// Outputs: none.
// Comments:
//=============================================================================
int
CommandHandler(struct aaldev_ownerSession *pownerSess,
               struct aal_pipmessage       Message)
{
#if (1 == ENABLE_DEBUG)
#define AFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define AFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG

   // Private session object set at session bind time (i.e., when object allocated)
   struct cci_PIPsession *pSess = (struct cci_PIPsession *)aalsess_pipHandle(pownerSess);
   struct cci_aal_device  *pdev  = NULL;

   // Overall return value for this function. Set before exiting if there is an error.
   //    retval = 0 means good return.
   int retval = 0;

   // UI Driver message
   struct aalui_CCIdrvMessage *pmsg = (struct aalui_CCIdrvMessage *) Message.m_message;


   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error = uid_errnumInvalidRequest;

   PINFO("In CCI Command handler, AFUCommand().\n");

   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cci device
   pdev = cci_PIPsessionp_to_ccidev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {
      struct ccipdrv_event_afu_response_event *pafuws_evt       = NULL;
      // Returns a workspace ID for the Config Space
      AFU_COMMAND_CASE(ccipdrv_getMMIORmap) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;

         // Used to hold the workspace ID
         struct aal_wsid   *wsidp            = NULL;

         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Failed ccipdrv_getMMIOR map Permission\n");
            pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create(pownerSess->m_device,
                                                             0,
                                                             (btPhysAddr)NULL,
                                                             0,
                                                             0,
                                                             0,
                                                             Message.m_tranID,
                                                             Message.m_context,
                                                             uid_errnumPermission);
            PERR("Direct API access not permitted on this device\n");

            retval = -EPERM;
         } else {

            //------------------------------------------------------------
            // Create the WSID object and add to the list for this session
            //------------------------------------------------------------
            if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
               PERR("Failed ccipdrv_getMMIOR map Parameter\n");
               pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                0,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on ccipdrv_getMMIORmap\n");

               retval = -EINVAL;
            } else {

               wsidp = ccidrv_getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_MMIO;
               PDEBUG("Getting CSR %s Aperature WSID %p using id %llx .\n",
                         ((WSID_CSRMAP_WRITEAREA == preq->ahmreq.u.wksp.m_wsid) ? "Write" : "Read"),
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_cci_csr(pdev), (int)cci_dev_len_cci_csr(pdev));

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create( pownerSess->m_device,
                                                                   wsidobjp_to_wid(wsidp),
                                                                   cci_dev_phys_afu_mmio(pdev),       // Return the requested aperture
                                                                   cci_dev_len_afu_mmio(pdev),        // Return the requested aperture size
                                                                   4,                                 // Return the CSR size in octets
                                                                   4,                                 // Return the inter-CSR spacing octets
                                                                   Message.m_tranID,
                                                                   Message.m_context,
                                                                   uid_errnumOK);

               PVERBOSE("Sending ccipdrv_getMMIORmap Event Event ID = %d\n",((struct aalui_WSMEvent*)(pafuws_evt->m_payload))->evtID );

               retval = 0;
            }
         }

         ccidrv_sendevent( aalsess_uiHandle(pownerSess),
                           aalsess_aaldevicep(pownerSess),
                           AALQIP(pafuws_evt),
                           Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;

      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_ALLOC)
      {
         struct ccidrvreq    *preq        = (struct ccidrvreq *)pmsg->payload;
         btVirtAddr           krnl_virt   = NULL;
         struct aal_wsid     *wsidp       = NULL;

         // Normal flow -- create the needed workspace.
         krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(preq->ahmreq.u.wksp.m_size);
         if (NULL == krnl_virt) {
            pafuws_evt = ccipdrv_event_afu_afuallocws_create(pownerSess->m_device,
                                                           (btWSID) 0,
                                                           NULL,
                                                           (btPhysAddr)NULL,
                                                           preq->ahmreq.u.wksp.m_size,
                                                           Message.m_tranID,
                                                           Message.m_context,
                                                           uid_errnumNoMem);

            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message.m_context);

            goto ERROR;
         }

         //------------------------------------------------------------
         // Create the WSID object and add to the list for this session
         //------------------------------------------------------------

         wsidp = ccidrv_getwsid(pownerSess->m_device, (btWSID)krnl_virt);
         if ( NULL == wsidp ) {
            PERR("Couldn't allocate task workspace\n");
            retval = -ENOMEM;
            /* send a failure event back to the caller? */
            goto ERROR;
         }

         wsidp->m_size = preq->ahmreq.u.wksp.m_size;
         wsidp->m_type = WSM_TYPE_VIRTUAL;
         PDEBUG("Creating Physical WSID %p.\n", wsidp);

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

         PINFO("CCI WS alloc wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR  " kvp=0x%" PRIx64 " size=%" PRIu64 " success!\n",
                  preq->ahmreq.u.wksp.m_wsid,
                  kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                  wsidp->m_id,
                  wsidp->m_size);

         // Create the event
         pafuws_evt = ccipdrv_event_afu_afuallocws_create(
                                               aalsess_aaldevicep(pownerSess),
                                               wsidobjp_to_wid(wsidp), // make the wsid appear page aligned for mmap
                                               NULL,
                                               kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                                               preq->ahmreq.u.wksp.m_size,
                                               Message.m_tranID,
                                               Message.m_context,
                                               uid_errnumOK);

         PVERBOSE("Sending the WKSP Alloc event.\n");
         // Send the event
         ccidrv_sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

      } break; // case fappip_afucmdWKSP_VALLOC


      //============================
      //  Free Workspace
      //============================
      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_FREE) {
         struct ccidrvreq    *preq        = (struct ccidrvreq *)pmsg->payload;
         btVirtAddr           krnl_virt   = NULL;
         struct aal_wsid     *wsidp       = NULL;

         ASSERT(0 != preq->ahmreq.u.wksp.m_wsid);
         if ( 0 == preq->ahmreq.u.wksp.m_wsid ) {
            PDEBUG("WKSP_IOC_FREE: WS id can't be 0.\n");
            // Create the exception event
            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message.m_tranID,
                                                           Message.m_context,
                                                           uid_errnumBadParameter);

            // Send the event
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message.m_context);
            retval = -EFAULT;
            goto ERROR;
         }

         // Get the workspace ID object
         wsidp = wsid_to_wsidobjp(preq->ahmreq.u.wksp.m_wsid);

         ASSERT(wsidp);
         if ( NULL == wsidp ) {
            // Create the exception event
            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message.m_tranID,
                                                           Message.m_context,
                                                           uid_errnumBadParameter);

            PDEBUG("Sending WKSP_FREE Exception\n");
            // Send the event
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message.m_context);

            retval = -EFAULT;
            goto ERROR;
         }

         // Free the buffer
         if(  WSM_TYPE_VIRTUAL != wsidp->m_type ) {
            PDEBUG( "Workspace free failed due to bad WS type. Should be %d but received %d\n",WSM_TYPE_VIRTUAL,
                  wsidp->m_type);

            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message.m_tranID,
                                                           Message.m_context,
                                                           uid_errnumBadParameter);
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message.m_context);

            retval = -EFAULT;
            goto ERROR;
         }

         krnl_virt = (btVirtAddr)wsidp->m_id;

         kosal_free_contiguous_mem(krnl_virt, wsidp->m_size);

         // remove the wsid from the device and destroy
         list_del_init(&wsidp->m_list);
         ccidrv_freewsid(wsidp);

         // Create the  event
         pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumOK);

         PVERBOSE("Sending the WKSP Free event.\n");
         // Send the event
         ccidrv_sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);
      } break; // case fappip_afucmdWKSP_FREE

      default: {
         struct ccipdrv_event_afu_response_event *pafuresponse_evt = NULL;

         PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);

         pafuresponse_evt = ccipdrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                     &Message.m_tranID,
                                                                     Message.m_context,
                                                                     request_error);

        ccidrv_sendevent( pownerSess->m_UIHandle,
                          pownerSess->m_device,
                          AALQIP(pafuresponse_evt),
                          Message.m_context);

         retval = -EINVAL;
      } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

ERROR:
   return retval;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             CCI SIM PIP MMAP             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: csr_vmaopen
// Description: Called when the vma is mapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaopen(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA OPEN.\n" );
}
#endif


//=============================================================================
// Name: wksp_vmaclose
// Description: called when vma is unmapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaclose(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA CLOSE.\n" );
}
#endif

#ifdef NOT_USED
static struct vm_operations_struct csr_vma_ops =
{
   .open    = csr_vmaopen,
   .close   = csr_vmaclose,
};
#endif


//=============================================================================
// Name: cci_mmap
// Description: Method used for mapping kernel memory to user space. Called by
//              uidrv.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: This method front ends all operations that require mapping shared
//           memory. It examines the wsid to determine the appropriate service
//           to perform the map operation.
//=============================================================================
int
cci_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific)
{

   struct vm_area_struct     *pvma = (struct vm_area_struct *) os_specific;

   struct cci_PIPsession   *pSess = NULL;
   struct cci_aal_device       *pdev = NULL;
   unsigned long              max_length = 0; // mmap length requested by user
   int                        res = -EINVAL;

   ASSERT(pownerSess);
   ASSERT(wsidp);

   // Get the spl2 aal_device and the memory manager session
   pSess = (struct cci_PIPsession *) aalsess_pipHandle(pownerSess);
   ASSERT(pSess);
   if ( NULL == pSess ) {
      PDEBUG("CCIV4 Simulator mmap: no Session");
      goto ERROR;
   }

   pdev = cci_PIPsessionp_to_ccidev(pSess);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PDEBUG("CCIV4 Simulator mmap: no device");
      goto ERROR;
   }

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   pvma->vm_ops = NULL;

   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if ( WSM_TYPE_MMIO == wsidp->m_type ) {
      void *ptr;
      size_t size;
      switch ( wsidp->m_id )
      {
            case WSID_CSRMAP_WRITEAREA:
            case WSID_CSRMAP_READAREA:
            case WSID_MAP_MMIOR:
            case WSID_MAP_UMSG:
            break;
         default:
            PERR("Attempt to map invalid WSID type %d\n", (int) wsidp->m_id);
            goto ERROR;
      }

      // Verify that we can fulfill the request - we set flags at create time.
      if ( WSID_CSRMAP_WRITEAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_write_space(pdev));

         if ( !cci_dev_allow_map_csr_write_space(pdev) ) {
            PERR("Denying request to map CSR Write space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_CSRMAP_READAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_read_space(pdev));

         if ( !cci_dev_allow_map_csr_read_space(pdev) ) {
            PERR("Denying request to map CSR Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_MAP_MMIOR == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_mmior_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_mmio(pdev);
         size = cci_dev_len_afu_mmio(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,               // Virtual Memory Area
            pvma->vm_start,                        // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT,   // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped MMR region.
         return 0;
      }

      if ( WSID_MAP_UMSG == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_umsg_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_umsg_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_umsg(pdev);
         size = cci_dev_len_afu_umsg(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,                             // Virtual Memory Area
            pvma->vm_start,                   // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped UMSG region.
         return 0;
      }

      // TO REST OF CHECKS

      // Map the PCIe BAR as the CSR region.
      ptr = (void *) cci_dev_phys_cci_csr(pdev);
      size = cci_dev_len_cci_csr(pdev);

      PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
         ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
         ptr,
         size,
         (void *)pvma->vm_start);

      // Map the region to user VM
      res = remap_pfn_range(pvma,                             // Virtual Memory Area
         pvma->vm_start,                   // Start address of virtual mapping
         ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
         size,
         pvma->vm_page_prot);

      if ( unlikely(0 != res) ) {
         PERR("remap_pfn_range error at CSR mmap %d\n", res);
         goto ERROR;
      }

      // Successfully mapped CSR region.
      return 0;
   }

   //------------------------
   // Map normal workspace
   //------------------------

   max_length = min(wsidp->m_size, (btWSSize)(pvma->vm_end - pvma->vm_start));

   PVERBOSE( "MMAP: start 0x%lx, end 0x%lx, KVP 0x%p, size=%" PRIu64 " 0x%" PRIx64 " max_length=%ld flags=0x%lx\n",
      pvma->vm_start, pvma->vm_end, (btVirtAddr)wsidp->m_id, wsidp->m_size, wsidp->m_size, max_length, pvma->vm_flags);

   res = remap_pfn_range(pvma,                              // Virtual Memory Area
      pvma->vm_start,                    // Start address of virtual mapping, from OS
      (kosal_virt_to_phys((btVirtAddr) wsidp->m_id) >> PAGE_SHIFT),   // physical memory backing store in pfn
      max_length,                        // size in bytes
      pvma->vm_page_prot);               // provided by OS
   if ( unlikely(0 != res) ) {
      PERR("remap_pfn_range error at workspace mmap %d\n", res);
      goto ERROR;
   }

   ERROR:
   return res;
}

