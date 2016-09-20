//******************************************************************************
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
#include "aalsdk/kernel/ccip_defs.h"

#include "aalsdk/kernel/ccipdriver.h"
#include "ccipdrv-events.h"

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
                           struct aal_pipmessage*);
extern int cci_mmap(struct aaldev_ownerSession *pownerSess,
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
            cci_create_AAL_UAFU_Device( struct port_device        *pportdev,
                                        btPhysAddr                 phys_mmio,
                                        struct CCIP_AFU_Header    *pafu_hdr,
                                        struct aal_device_id      *paalid)
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
   if(NULL == pcci_aaldev ){
      return NULL;
   }

   // Make it a User AFU
   cci_aaldev_type(pcci_aaldev)     = cci_dev_UAFU;

   // Record parentage
   cci_aaldev_pport(pcci_aaldev)    = pportdev;       // Save its port
   cci_aaldev_pfme(pcci_aaldev)     = ccip_port_dev_fme(pportdev);
   cci_aaldev_pci_dev(pcci_aaldev) = ccip_dev_to_pci_dev( ccip_port_to_ccidev(pportdev) );

   // Device Address is the same as the Port. Set the AFU ID information
   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(*paalid)            = pafu_hdr->ccip_afu_id_l.afu_id_l;
   aaldevid_afuguidh(*paalid)            = pafu_hdr->ccip_afu_id_h.afu_id_h;
   aaldevid_pipguid(*paalid)             = CCIP_AFU_PIPIID;

   // Set the interface permissions
   // Enable MMIO-R
   cci_aaldev_set_allow_map_mmior_space(pcci_aaldev);

   // Setup the MMIO region parameters
   cci_aaldev_kvp_afu_mmio(pcci_aaldev)   = (btVirtAddr)pafu_hdr;
   cci_aaldev_len_afu_mmio(pcci_aaldev)   = CCI_MMIO_SIZE;
   cci_aaldev_phys_afu_mmio(pcci_aaldev)  = phys_mmio;

   // Create the AAL device and attach it to the CCI device object
   cci_aaldev_to_aaldev(pcci_aaldev)  =  aaldev_create( "CCIPAFU",          // AAL device base name
                                                        &*paalid,             // AAL ID
                                                        &cci_AFUpip);

   //CCI device object create fails, delete AFU AAL device
   if(NULL == cci_aaldev_to_aaldev(pcci_aaldev) ){
      cci_destroy_aal_device(pcci_aaldev);
      return NULL;
   }

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   cci_aaldev_to_aaldev(pcci_aaldev)->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;
   if( cci_aaldev_allow_map_csr_read_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cci_aaldev_allow_map_csr_write_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   if( cci_aaldev_allow_map_mmior_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

   if( cci_aaldev_allow_map_umsg_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_UMSG;
   }

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_aaldev_to_aaldev(pcci_aaldev)) = (void*)pcci_aaldev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_aaldev_to_aaldev(pcci_aaldev), cci_release_device);

   // Record the uAFU in the port
   ccip_port_uafu_dev(pportdev) = pcci_aaldev;

   // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pcci_aaldev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                  aaldevid_devaddr_devnum(*paalid),
                                                                  aaldevid_devaddr_fcnnum(*paalid),
                                                                  aaldevid_devaddr_subdevnum(*paalid));
      cci_destroy_aal_device(pcci_aaldev);
      ccip_port_uafu_dev(pportdev) = NULL;
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
               struct aal_pipmessage      *Message)
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
   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error       = uid_errnumInvalidRequest;

   // UI Driver message
   struct aalui_CCIdrvMessage *pmsg = (struct aalui_CCIdrvMessage *) Message->m_message;

   // Save original response buffer size in case we return something
   btWSSize         respBufSize     = Message->m_respbufSize;

   // Assume returning nothing. By setting the response buffer size to 0
   //   we tell the upstream side that there is no payload to copy back.
   //   Setting it here means we don't have to set it (or forget to) in each
   //   command.  We've recorded the payload buffer size above if we do need
   //   intend to send a payload.
   Message->m_respbufSize          = 0;

   PTRACEIN;

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

      AFU_COMMAND_CASE(ccipdrv_afucmdPort_afuQuiesceAndHalt) {
         if(0 != port_afu_quiesce_and_halt( cci_aaldev_pport(pdev) )){
            // Failure event
            PERR("TIMEOUT\n");
            Message->m_errcode = uid_errnumTimeout;
            break;
         }

         // Success Event
         Message->m_errcode = uid_errnumOK;

      } break;

      AFU_COMMAND_CASE(ccipdrv_afucmdPort_afuEnable) {
         if(0 != port_afu_Enable( cci_aaldev_pport(pdev) )){
            PERR("TIMEOUT\n");
            // Failure event
            Message->m_errcode = uid_errnumTimeout;
            break;
         }

         // Success Event
         Message->m_errcode = uid_errnumOK;

      } break;


      // Returns a workspace ID for the Config Space
      AFU_COMMAND_CASE(ccipdrv_getMMIORmap) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
         struct aalui_WSMEvent WSID;
         struct aal_wsid   *wsidp            = NULL;

         if ( !cci_aaldev_allow_map_mmior_space(pdev) ) {
            PERR("Failed ccipdrv_getMMIOR map Permission\n");
            PERR("Direct API access not permitted on this device\n");
            Message->m_errcode = uid_errnumPermission;
            break;
         }

         //------------------------------------------------------------
         // Create the WSID object and add to the list for this session
         //------------------------------------------------------------
         if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
            PERR("Failed ccipdrv_getMMIOR map Parameter\n");

            PERR("Bad WSID on ccipdrv_getMMIORmap\n");
            Message->m_errcode = uid_errnumBadParameter;
            break;
         }

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

         PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_aaldev_phys_afu_mmio(pdev), (int)cci_aaldev_len_afu_mmio(pdev));

         // Set up the return payload
         WSID.evtID           = uid_wseventMMIOMap;
         WSID.wsParms.wsid    = pwsid_to_wsidHandle(wsidp);
         WSID.wsParms.physptr = cci_aaldev_phys_afu_mmio(pdev);
         WSID.wsParms.size    = cci_aaldev_len_afu_mmio(pdev);

         // Make this atomic. Check the original response buffer size for room
         if(respBufSize >= sizeof(struct aalui_WSMEvent)){
            *((struct aalui_WSMEvent*)Message->m_response) = WSID;
            Message->m_respbufSize = sizeof(struct aalui_WSMEvent);
         }else{
            PERR("No room to return WSID. Required sized %ld but size provided %ld\n", sizeof(struct aalui_WSMEvent), (long int)respBufSize);
            Message->m_errcode = uid_errnumNoMem;
            ccidrv_freewsid(wsidp);
            break;
         }
         PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
         Message->m_errcode = uid_errnumOK;

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

      } break;

      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_ALLOC)
      {
         struct ccidrvreq    *preq        = (struct ccidrvreq *)pmsg->payload;
         btVirtAddr           krnl_virt   = NULL;
         struct aal_wsid     *wsidp       = NULL;
         struct aalui_WSMEvent WSID;
         btHANDLE             iova        = NULL;;

         PDEBUG( "Allocating %lu bytes \n", (unsigned long)preq->ahmreq.u.wksp.m_size);
         if( NULL== cci_aaldev_pci_dev(pdev) ) {
            // Normal flow -- create the needed workspace.
            krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(preq->ahmreq.u.wksp.m_size);
            if (NULL == krnl_virt) {
               Message->m_errcode = uid_errnumNoMem;
               break;
            }
         }else{
            krnl_virt = kosal_alloc_dma_coherent( ccip_dev_pci_dev(pdev), preq->ahmreq.u.wksp.m_size, &iova);
            if (NULL == krnl_virt) {
               Message->m_errcode = uid_errnumNoMem;
               break;
            }
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

         // Set up the return payload
         WSID.evtID           = uid_wseventAllocate;
         WSID.wsParms.wsid    = pwsid_to_wsidHandle(wsidp);
         if( NULL== cci_aaldev_pci_dev(pdev) ) {
            WSID.wsParms.physptr = (btWSID)kosal_virt_to_phys(krnl_virt);
            wsidp->m_dmahandle = (btHANDLE)kosal_virt_to_phys(krnl_virt);
         }else{
            WSID.wsParms.physptr = (btWSID)iova;
            wsidp->m_dmahandle = iova;
         }
         WSID.wsParms.size    = preq->ahmreq.u.wksp.m_size;

         // Make this atomic. Check the original response buffer size for room
         if(respBufSize >= sizeof(struct aalui_WSMEvent)){
            *((struct aalui_WSMEvent*)Message->m_response) = WSID;
            Message->m_respbufSize = sizeof(struct aalui_WSMEvent);
         }
         PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
         Message->m_errcode = uid_errnumOK;

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
            Message->m_errcode = uid_errnumBadParameter;
            break;
         }

         // Get the workspace ID object
         wsidp = ccidrv_valwsid(preq->ahmreq.u.wksp.m_wsid);

         ASSERT(wsidp);
         if ( NULL == wsidp ) {
            Message->m_errcode = uid_errnumBadParameter;
            break;
         }

         // Free the buffer
         if(  WSM_TYPE_VIRTUAL != wsidp->m_type ) {
            PDEBUG( "Workspace free failed due to bad WS type. Should be %d but received %d\n",WSM_TYPE_VIRTUAL,
                  wsidp->m_type);

            Message->m_errcode = uid_errnumBadParameter;
            break;
         }

         krnl_virt = (btVirtAddr)wsidp->m_id;
         if( NULL== cci_aaldev_pci_dev(pdev) ) {
            kosal_free_contiguous_mem(krnl_virt, wsidp->m_size);
         }else{
            kosal_free_dma_coherent( ccip_dev_pci_dev(pdev), krnl_virt, wsidp->m_size, wsidp->m_dmahandle);
         }

         // remove the wsid from the device and destroy
         kosal_list_del_init(&wsidp->m_list);
         ccidrv_freewsid(wsidp);

         PVERBOSE("Sending the WKSP Free event.\n");
         Message->m_errcode = uid_errnumOK;

      } break; // case fappip_afucmdWKSP_FREE

      //============================
      //  Get number of uMSGs
      //============================
      AFU_COMMAND_CASE(ccipdrv_afucmdGetNumUmsgs) {
         struct ccidrvreq    *presp                         = (struct ccidrvreq *)Message->m_response;
         struct CCIP_PORT_DFL_UMSG            *puMsgvirt    = NULL;

         if(false == get_port_feature( cci_aaldev_pport(pdev),
                                       CCIP_PORT_DFLID_USMG,
                                       NULL,
                                       (btVirtAddr*)&puMsgvirt)){
            Message->m_errcode = uid_errnumNoMap;
            break;
         }
         PVERBOSE("uMSG feature Header %p\n", puMsgvirt);

         PVERBOSE("Returning number of uMSGs = %d.\n",puMsgvirt->ccip_umsg_capability.no_umsg_alloc_port);
         presp->ahmreq.u.mem_uv2id.mem_id = puMsgvirt->ccip_umsg_capability.no_umsg_alloc_port;
         Message->m_respbufSize = respBufSize;
         Message->m_errcode = uid_errnumOK;

      } break; // case ccipdrv_afucmdGetNumUmsgs

      //============================
      //  Set uMSG Mode
      //============================
      AFU_COMMAND_CASE(ccipdrv_afucmdSetUmsgMode) {
         struct ccidrvreq    *preq                          = (struct ccidrvreq *)pmsg->payload;
         struct CCIP_PORT_DFL_UMSG            *puMsgvirt    = NULL;

         if(false == get_port_feature( cci_aaldev_pport(pdev),
                                       CCIP_PORT_DFLID_USMG,
                                       NULL,
                                       (btVirtAddr*)&puMsgvirt)){
            Message->m_errcode = uid_errnumNoMap;
            break;
         }
         PVERBOSE("uMSG feature Header %llx\n", (0xFFFFFFFF & preq->ahmreq.u.wksp.m_wsid));

         puMsgvirt->ccip_umsg_mode.umsg_hit = (0xFFFFFFFF & preq->ahmreq.u.wksp.m_wsid);
         Message->m_errcode = uid_errnumOK;

      } break; // case ccipdrv_afucmdSetUmsgMode

      AFU_COMMAND_CASE(ccipdrv_afucmdGet_UmsgBase)
      {
         struct ccidrvreq              *preq          = (struct ccidrvreq *)pmsg->payload;
         struct CCIP_PORT_DFL_UMSG     *puMsgvirt     = NULL;
         btVirtAddr                     krnl_virt     = NULL;
         struct aal_wsid               *wsidp         = NULL;
         struct aalui_WSMEvent          WSID;
         btWSSize                       size          = 0;
         btHANDLE                       iova          = 0;


         if(false == get_port_feature( cci_aaldev_pport(pdev),
                                       CCIP_PORT_DFLID_USMG,
                                       NULL,
                                       (btVirtAddr*)&puMsgvirt)){
            Message->m_errcode = uid_errnumNoMap;
            break;
         }

         if( 0 != puMsgvirt->ccip_umsg_base_address.umsg_base_address){
            Message->m_errcode = uid_errnumInvalidRequest;
            PERR("uMSG base already set\n");
            break;
         }
         size = puMsgvirt->ccip_umsg_capability.no_umsg_alloc_port * CCIP_UMSG_SIZE;
         PDEBUG( "Allocating %lu bytes for uMSG\n", (unsigned long)size);

         // Normal flow -- create the needed workspace.
         if( NULL== cci_aaldev_pci_dev(pdev) ) {
            // Normal flow -- create the needed workspace.
            krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(size);
            if (NULL == krnl_virt) {
               Message->m_errcode = uid_errnumNoMem;
               break;
            }
            puMsgvirt->ccip_umsg_base_address.umsg_base_address = kosal_virt_to_phys(krnl_virt);
         }else{
            krnl_virt = kosal_alloc_dma_coherent( ccip_dev_pci_dev(pdev),size, &iova);
            if (NULL == krnl_virt) {
               Message->m_errcode = uid_errnumNoMem;
               break;
            }
            puMsgvirt->ccip_umsg_base_address.umsg_base_address = (btPhysAddr)iova;
         }


         // Set the uMSG area
         PDEBUG("uMSG @ %p [%" PRIxPHYS_ADDR "]\n", krnl_virt, (btPhysAddr)puMsgvirt->ccip_umsg_base_address.umsg_base_address);

         // Enable uMsg operation
         // FIXME: Disable UMSgs until BBS bug with Readline_S is resolved
         puMsgvirt->ccip_umsg_capability.status_umsg_engine = 0;
         {
            // Wait for the uMSG engine to start
            btTime delay = 10;
            btTime totaldelay = 0;

            while(0 == puMsgvirt->ccip_umsg_capability.umsg_init_status)
            {
               // Sleep
               kosal_udelay(delay);

               // total delay
               totaldelay = totaldelay + delay;
               if (totaldelay > 1000)   {
                  PDEBUG("Timed out waiting for uMSG engine to start\n");
                  Message->m_errcode = uid_errnumTimeout;
                  puMsgvirt->ccip_umsg_base_address.umsg_base_address = 0;

                  if( NULL== cci_aaldev_pci_dev(pdev) ) {
                     kosal_free_contiguous_mem(krnl_virt, size);
                  }else{
                     kosal_free_dma_coherent( ccip_dev_pci_dev(pdev), krnl_virt, size, iova);
                  }

                  return 0;
               }
            }
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

         wsidp->m_size = size;
         wsidp->m_type = WSM_TYPE_VIRTUAL;
         PDEBUG("Creating uMSG WSID %p.\n", wsidp);

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

         PINFO("CCI uMSG wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR  " kvp=0x%" PRIx64 " size=%" PRIu64 " success!\n",
                  preq->ahmreq.u.wksp.m_wsid,
                  kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                  wsidp->m_id,
                  wsidp->m_size);

         // Set up the return payload
         WSID.evtID           = uid_wseventAllocate;
         WSID.wsParms.wsid    = pwsid_to_wsidHandle(wsidp);
         WSID.wsParms.physptr = (btWSID) puMsgvirt->ccip_umsg_base_address.umsg_base_address;
         WSID.wsParms.size    = size;

         // Make this atomic. Check the original response buffer size for room
         if(respBufSize >= sizeof(struct aalui_WSMEvent)){
            *((struct aalui_WSMEvent*)Message->m_response) = WSID;
            Message->m_respbufSize = sizeof(struct aalui_WSMEvent);
            PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
            Message->m_errcode = uid_errnumOK;
         }else{
            PERR("No room to return WSID\n");
            Message->m_errcode = uid_errnumNoMem;
         }


      } break; // case ccipdrv_afucmdGet_UmsgBase


      default: {

         PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);
         Message->m_errcode = request_error;
         retval = -EINVAL;
      } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

ERROR:
   return retval;
}
