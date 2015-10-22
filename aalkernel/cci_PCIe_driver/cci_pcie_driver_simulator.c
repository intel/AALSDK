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
//  Copyright(c) 2011-2015, Intel Corporation.
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
//        FILE: cci_pcie_driver_simulator.c
//     CREATED: Oct 14, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the implementation of the simulated CCI
//            device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus.h"

#include "cci_pcie_driver_internal.h"
#include "cci_pcie_driver_simulator.h"

#include "cci_pcie_driver_PIPsession.h"

#include "aalsdk/kernel/fappip.h"
#include "aalsdk/kernel/aalui-events.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD

int cci_create_sim_afu(btVirtAddr,uint ,struct aal_device_id*,struct list_head *);

#define CCIV4_MMIO_UMSG_TEST 0

#if CCIV4_MMIO_UMSG_TEST
// Turn on in AFUdev.cpp, as well
static char mmioafustring[]    = "CCIv4 MMIO test  \n";
static char umesgafustring[]   = "CCIv4 UMSG test  \n";
#endif

static int
CommandHandler(struct aaldev_ownerSession *,
               struct aal_pipmessage,
               void *);
int
cci_sim_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific);

//=============================================================================
// CCIV4_SIMAFUpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip cci_simAFUpip = {
   .m_messageHandler = {
      .sendMessage   = CommandHandler,       // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
     .mmap = cci_sim_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};

//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_Host,
   .m_busnum    = 1,       //
   .m_devicenum = 1,       //
   .m_subdevnum = 0        // AFU
};


//=============================================================================
// Name: cci_sim_alloc_next_afu_addr
// Description: Allocate the next AFU address
// Interface: public
// Returns 0 - success
// Inputs: none
// Outputs: none.
// Comments: Allocates sequential addresses using only the 16 LSBs of the
//           device number and address.  Eventually the address structure may
//           change to encode the device and subdevice into a single 32 bit int.
//=============================================================================
struct aal_device_addr
cci_sim_alloc_next_afu_addr(void)
{
   ++(nextAFU_addr.m_subdevnum);
   if( 0 == (nextAFU_addr.m_subdevnum &= 0xffff) ) {
      ++(nextAFU_addr.m_devicenum);
      nextAFU_addr.m_devicenum &= 0xffff;
   }
   return nextAFU_addr;
} // cci_alloc_next_afu_addr


//=============================================================================
// Name: cci_sim_discover_devices
// Description: Performs the functionality of the PCIe OS enumeration functions
//              for real hardware.
// Inputs: numdevices - Number of AFUs to "discover".
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cci_sim_discover_devices(ulong numdevices,
                             struct list_head *g_device_list)
{
   int                  ret               = 0;
   struct aal_device_id aalid;
   btVirtAddr           pAperture         = NULL;

   PVERBOSE("Creating %ld simulated CCI devices", numdevices);

   // Loop through and probe each simulated device
   while(numdevices--){

      // Create Channel AFU for this device
      aaldevid_addr(aalid) = cci_sim_alloc_next_afu_addr();

      // Allocate the BAR for the simulated device
      pAperture = kosal_kzmalloc( CCI_SIM_APERTURE_SIZE );
      if(NULL == pAperture){
         PERR("Unable to allocate system memory for simulated BAR\n");
         return -EINVAL;
      }

      // Create the AFU
      ret = cci_create_sim_afu(pAperture, CCI_SIM_APERTURE_SIZE, &aalid, g_device_list);
      ASSERT(0 == ret);
      if(0>ret){
         kfree(pAperture);
         PERR("Unable to create simulated device\n");
      }
   }// end while
   return 0;
}


//=============================================================================
// Name: cci_create_sim_afu
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: The steps are:
//               Create the CCI device object.
//               Set the CCI attributes (e.g., MMIO regions)
//               Attach an aal_device to the CCI device (used by framework)
//               Publish the new device with the framework
//               Add the device to the driver's global list
//=============================================================================
int cci_create_sim_afu( btVirtAddr virtAddr,
                        uint size,
                        struct aal_device_id *paalid,
                        struct list_head *pdevice_list)
{
   struct cci_device *pCCIdev    = NULL;
   btVirtAddr         ptemp      = NULL;
   int                ret        = 0;

   //=============================================================
   // Create the CCI device structure. The CCI device is the class
   // used by the Low Level Communications (PIP). It holds the
   // hardware specific attributes.

   // Construct the cci_device object
   pCCIdev = cci_create_device();

   //====================================================
   // Set up the cci_device attributes used by the driver
   //

   // Set up Simulated Config Space
   cci_dev_len_config(pCCIdev)  = size;
   cci_dev_kvp_config(pCCIdev)  = virtAddr;
   cci_dev_phys_config(pCCIdev) = virt_to_phys(cci_dev_kvp_config(pCCIdev));

   // Set CCI CSR Attributes
   cci_dev_phys_cci_csr(pCCIdev) = cci_dev_phys_config(pCCIdev);
   cci_dev_kvp_cci_csr(pCCIdev)  = cci_dev_kvp_config(pCCIdev);
   cci_dev_len_cci_csr(pCCIdev)  = CCI_SIM_APERTURE_SIZE;

   // Allocate uMSG space
   ptemp = kosal_kzmalloc(CCI_UMSG_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for uMsg area\n");
      cci_destroy_device(pCCIdev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,umesgafustring,strlen(umesgafustring));
#endif

   cci_dev_len_afu_umsg(pCCIdev) = size;
   cci_dev_kvp_afu_umsg(pCCIdev) = ptemp;
   cci_dev_phys_afu_umsg(pCCIdev) = virt_to_phys(ptemp);

   // Allocate MMIO space
   ptemp = kosal_kzmalloc(CCI_MMIO_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for MMIO area\n");
      kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev), cci_dev_len_afu_umsg(pCCIdev));
      cci_destroy_device(pCCIdev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,mmioafustring,strlen(mmioafustring));
#endif

   cci_dev_len_afu_mmio(pCCIdev) = size;
   cci_dev_kvp_afu_mmio(pCCIdev) = ptemp;
   cci_dev_phys_afu_mmio(pCCIdev) = virt_to_phys(cci_dev_kvp_afu_mmio(pCCIdev));

   // Direct user mode CSR interface not supported
   cci_dev_clr_allow_map_csr_write_space(pCCIdev);
   cci_dev_clr_allow_map_csr_read_space(pCCIdev);

   // Enable MMIO-R and uMSG space
   cci_dev_set_allow_map_mmior_space(pCCIdev);
   cci_dev_set_allow_map_umsg_space(pCCIdev);

   // Mark as simulated
   cci_set_simulated(pCCIdev);

   //===========================================================
   // Create the AAL device structure. The AAL device is the
   // base class for all devices in AAL. This object is used by
   // the AAL kernel framework. It also provides the pointer to
   // the low level physical interface protocol module (PIP).

   // Create the ID of the AAL device. Used for Resource Management.
   aaldevid_afuguidl(*paalid) = CCI_SIM_AFUIDL;
   aaldevid_afuguidh(*paalid) = CCI_SIM_AFUIDH;
   aaldevid_devtype(*paalid)  = aal_devtypeAFU;
   aaldevid_pipguid(*paalid)  = CCI_SIMAFUPIP_IID;
   aaldevid_vendorid(*paalid) = AAL_vendINTC;
   aaldevid_ahmguid(*paalid)  = HOST_AHM_GUID;

   // Create the AAL device
   pCCIdev->m_aaldev =  aaldev_create( "CCISIMAFU",
                                        paalid,
                                        &cci_simAFUpip);

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   pCCIdev->m_aaldev->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_dev_to_aaldev(pCCIdev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;
   if( cci_dev_allow_map_csr_read_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cci_dev_allow_map_csr_write_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   if( cci_dev_allow_map_mmior_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

   if( cci_dev_allow_map_umsg_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_UMSG;
   }

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_dev_to_aaldev(pCCIdev)) = (void*)pCCIdev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_dev_to_aaldev(pCCIdev), cci_release_device);

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pCCIdev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 AFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                               aaldevid_devaddr_devnum(*paalid),
                                                                               aaldevid_devaddr_subdevnum(*paalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pCCIdev), cci_dev_len_afu_mmio(pCCIdev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev),cci_dev_len_afu_umsg(pCCIdev));
      cci_destroy_device(pCCIdev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cci_dev_list_head(pCCIdev), pdevice_list);

   return 0;
}


//=============================================================================
// Name: CommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
//         MessageContext - used by UIDRV (TODO deprecate)
// Outputs: none.
// Comments:
//=============================================================================
int
CommandHandler(struct aaldev_ownerSession *pownerSess,
               struct aal_pipmessage       Message,
               void                       *MessageContext)
{
#if (1 == ENABLE_DEBUG)
#define AFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define AFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG

   // Private session object set at session bind time (i.e., when allocated)
   struct cci_PIPsession *pSess = (struct cci_PIPsession *)aalsess_pipHandle(pownerSess);
   struct cci_device  *pdev  = NULL;

   // Generalized payload pointer. Points to locally allocated and copied
   //    version of pmsg->payload of length pmsg->size
   void *p_localpayload = NULL;

   // Overall return value for this function. Set before exiting if there is an error.
   //    retval = 0 means good return.
   int retval = 0;

   // UI Driver message
   struct aalui_AFUmessage *pmsg = (struct aalui_AFUmessage *) Message.m_message;

   // Used to point to the response event
   struct uidrv_event_afu_response_event  *pafuresponse_evt = NULL;
   struct uidrv_event_afu_workspace_event *pafuws_evt       = NULL;

   // Used by WS allocation
   struct aal_wsid                        *wsidp            = NULL;

   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error = uid_errnumInvalidRequest;

   PINFO("In CCIv4 Command handler, AFUCommand().\n");

   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cciv4 device
   pdev = cci_PIPsessionp_to_cciv4dev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }
   //============================

   // Get the request copied into local memory
   p_localpayload = kosal_kzmalloc(pmsg->size);
   if( !p_localpayload ) {
      PDEBUG("Error: kosal_kzmalloc failed with size=%" PRIu64 "\n", pmsg->size);
      retval = -ENOMEM;
      goto ERROR;
   }
   if ( copy_from_user( p_localpayload, (void *) pmsg->payload, pmsg->size) ) {
      PDEBUG("Error: copy_from_user(p_localpayload=%p, pmsg->payload=%p, pmsg->size=%" PRIu64 ") failed.\n", p_localpayload,
                                                                                                             pmsg->payload,
                                                                                                             pmsg->size);
      retval = -EFAULT;
      goto ERROR;
   }


   // Check for MAFU message first. Invalid for this device.
   // Flow will fall through the switch statement which follows.
   //  and the error event will be sent in the default clause.
   if ( aalui_mafucmd == pmsg->cmd ) {
      PDEBUG("Permission denied. Not Management AFU\n");
      request_error = uid_errnumPermission;
   }

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {
      // Returns a workspace ID for the Config Space
      AFU_COMMAND_CASE(fappip_getCSRmap) {
         struct cciv4req *preq = (struct cciv4req *)p_localpayload;

         if ( !cci_dev_allow_map_csr_space(pdev) ) {
            PERR("Failed getCSR map Permission\n");
            pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
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
            if ( ( WSID_CSRMAP_WRITEAREA != preq->ahmreq.u.wksp.m_wsid ) &&
                 ( WSID_CSRMAP_READAREA  != preq->ahmreq.u.wksp.m_wsid ) ) {
               PERR("Failed getCSR map Parameter\n");
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                0,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on fappip_getCSRmap\n");

               retval = -EINVAL;
            } else {

               wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate CSR workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_CSR;
               PDEBUG("Getting CSR %s Aperature WSID %p using id %llx .\n",
                         ((WSID_CSRMAP_WRITEAREA == preq->ahmreq.u.wksp.m_wsid) ? "Write" : "Read"),
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_cci_csr(pdev), (int)cci_dev_len_cci_csr(pdev));

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                 pownerSess->m_device,
                                 wsidobjp_to_wid(wsidp),
                                 cci_dev_phys_cci_csr(pdev),        // Return the requested aperture
                                 cci_dev_len_cci_csr(pdev),         // Return the requested aperture size
                                 4,                                 // Return the CSR size in octets
                                 4,                                 // Return the inter-CSR spacing octets
                                 Message.m_tranID,
                                 Message.m_context,
                                 uid_errnumOK);

               PVERBOSE("Sending uid_wseventCSRMap Event\n");

               retval = 0;
            }
         }

         pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;

      // Returns a workspace ID for the MMIO-R Space
      AFU_COMMAND_CASE(fappip_getMMIORmap) {
         struct cciv4req *preq = (struct cciv4req *)p_localpayload;

         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Failed getCSR map Permission\n");
            pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
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
               PERR("Failed getCSR map Parameter\n");
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                0,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on fappip_getMMIORmap\n");

               retval = -EINVAL;
            } else {

               wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate MMIOR workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_CSR;
               PDEBUG("Getting CSR Aperature WSID %p WSID_MAP_MMIOR using id %llx .\n",
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_afu_mmio(pdev), (int)cci_dev_len_afu_mmio(pdev));

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                 pownerSess->m_device,
                                 wsidobjp_to_wid(wsidp),
                                 cci_dev_phys_afu_mmio(pdev),        // Return the requested aperture
                                 cci_dev_len_afu_mmio(pdev),         // Return the requested aperture size
                                 4,                                  // Return the CSR size in octets
                                 4,                                  // Return the inter-CSR spacing octets
                                 Message.m_tranID,
                                 Message.m_context,
                                 uid_errnumOK);

               PVERBOSE("Sending fappip_getMMIORmap Event\n");

               retval = 0;
            }
         }

         pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;

      AFU_COMMAND_CASE(fappip_getuMSGmap) {
         struct cciv4req *preq = (struct cciv4req *)p_localpayload;

         if ( !cci_dev_allow_map_umsg_space(pdev) ) {
            PERR("Failed getCSR map Permission\n");
            pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
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
            if ( WSID_MAP_UMSG != preq->ahmreq.u.wksp.m_wsid ) {
               PERR("Failed getCSR map Parameter\n");
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                0,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on fappip_getuMSGmap\n");

               retval = -EINVAL;
            } else {

               wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate UMSG workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_CSR;
               PDEBUG("Getting CSR Aperature WSID %p WSID_MAP_UMSG using id %llx .\n",
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_afu_umsg(pdev), (int)cci_dev_len_afu_umsg(pdev));

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                 pownerSess->m_device,
                                 wsidobjp_to_wid(wsidp),
                                 cci_dev_phys_afu_umsg(pdev),        // Return the requested aperture
                                 cci_dev_len_afu_umsg(pdev),         // Return the requested aperture size
                                 4,                                  // Return the CSR size in octets
                                 4,                                  // Return the inter-CSR spacing octets
                                 Message.m_tranID,
                                 Message.m_context,
                                 uid_errnumOK);

               PVERBOSE("Sending fappip_getuMSGmap Event\n");

               retval = 0;
            }
         }

         pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;

#if 0
   //============================
   //  Get/Set CSR block function
   //============================
   case fappip_afucmdCSR_GETSET:{

      // Used by Get/Set CSR
      unsigned             u;
      unsigned             index;
      csr_read_write_blk  *pcsr_rwb;
      unsigned             num_to_set;
      csr_offset_value    *pcsr_array;

      DPRINTF(ENCODER_DBG_AFU, "Get/Set function\n");
      // Make sure we are the transaction owner
      if( !spl2_sessionp_is_tranowner(pSess){
         // Device is currently busy.
          pafuresponse_evt = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                             &Message.m_tranID,
                                                             Message.m_context,
                                                             NULL,
                                                             uid_errnumPermission);
          // Send the event
          pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                         pownerSess->m_device,
                                         AALQIP(pafuresponse_evt),
                                         MessageContext);
          goto failed;

      }
      index       = 0;
      pcsr_rwb    = (csr_read_write_blk*) p_localpayload;
      num_to_set  = pcsr_rwb->num_to_set;
      pcsr_array  = csr_rwb_setarray(pcsr_rwb);

      DPRINTF(ENCODER_DBG_AFU, "Num to set %d\n", num_to_set);

      // Execute the simulator
      for (u = 0; u < num_to_set; ++u) {

         // Invoke the PIP writecsr method. (See encoder write csr
         if( 0 != encoder_writecsr( pSess->pencoderafu, pcsr_array[u].csr_offset, pcsr_array[u].csr_value) ){
            pafuresponse_evt = uidrv_event_afu_afucsrgetset_create(  pownerSess->m_device,
                                                                     pcsr_rwb,
                                                                     u,
                                                                     &Message.m_tranID,
                                                                     Message.m_context,
                                                                     uid_errnumOK);

         }
      }

      if( NULL == pafuresponse_evt){
         // Create the response event
         pafuresponse_evt = uidrv_event_afu_afucsrgetset_create( pownerSess->m_device,
                                                                 pcsr_rwb,
                                                                 index,
                                                                 &Message.m_tranID,
                                                                 Message.m_context,
                                                                 uid_errnumOK);
      }

      if (unlikely(pafuresponse_evt == NULL)) {
         DPRINTF(ENCODER_DBG_AFU, "Exception creating event! No memory\n");
         retval = -ENOMEM;
         goto failed;
      }

      // Send the event
      pownerSess->m_uiapi->sendevent(pSess->m_pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuresponse_evt),
                                     Message.m_context);

      break;
   }
#endif

   //============================
   //  Allocate Workspace
   //============================
   AFU_COMMAND_CASE(fappip_afucmdWKSP_VALLOC) {
      struct ahm_req          req;
      btVirtAddr krnl_virt = NULL;

      req = *(struct ahm_req *)p_localpayload;

      // Normal flow -- create the needed workspace.
      krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(req.u.wksp.m_size);
      if (NULL == krnl_virt) {
         pafuws_evt = uidrv_event_afu_afuallocws_create(pownerSess->m_device,
                                                        (btWSID) 0,
                                                        NULL,
                                                        (btPhysAddr)NULL,
                                                        req.u.wksp.m_size,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumNoMem);

         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         goto ERROR;
      }

      //------------------------------------------------------------
      // Create the WSID object and add to the list for this session
      //------------------------------------------------------------
      wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, (btWSID)krnl_virt);
      if ( NULL == wsidp ) {
         PERR("Couldn't allocate task workspace\n");
         retval = -ENOMEM;
         /* send a failure event back to the caller? */
         goto ERROR;
      }

      wsidp->m_size = req.u.wksp.m_size;
      wsidp->m_type = WSM_TYPE_PHYSICAL;
      PDEBUG("Creating Physical WSID %p.\n", wsidp);

      // Add the new wsid onto the session
      aalsess_add_ws(pownerSess, wsidp->m_list);

      PINFO("CCI WS alloc wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR  " kvp=0x%" PRIx64 " size=%" PRIu64 " success!\n",
               req.u.wksp.m_wsid,
               kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
               wsidp->m_id,
               wsidp->m_size);

      // Create the event
      pafuws_evt = uidrv_event_afu_afuallocws_create(
                                            aalsess_aaldevicep(pownerSess),
                                            wsidobjp_to_wid(wsidp), // make the wsid appear page aligned for mmap
                                            NULL,
                                            kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                                            req.u.wksp.m_size,
                                            Message.m_tranID,
                                            Message.m_context,
                                            uid_errnumOK);

      PVERBOSE("Sending the WKSP Alloc event.\n");
      // Send the event
      pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                     aalsess_aaldevicep(pownerSess),
                                     AALQIP(pafuws_evt),
                                     Message.m_context);

   } break; // case fappip_afucmdWKSP_VALLOC


   //============================
   //  Free Workspace
   //============================
   AFU_COMMAND_CASE(fappip_afucmdWKSP_VFREE) {
      struct ahm_req         req;
      btVirtAddr krnl_virt = NULL;

      req = *(struct ahm_req*)p_localpayload;

      ASSERT(0 != req.u.wksp.m_wsid);
      if ( 0 == req.u.wksp.m_wsid ) {
         PDEBUG("WKSP_IOC_FREE: WS id can't be 0.\n");
         // Create the exception event
         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);

         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);
         retval = -EFAULT;
         goto ERROR;
      }

      // Get the workspace ID object
      wsidp = wsid_to_wsidobjp(req.u.wksp.m_wsid);

      ASSERT(wsidp);
      if ( NULL == wsidp ) {
         // Create the exception event
         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);

         PDEBUG("Sending WKSP_FREE Exception\n");
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         retval = -EFAULT;
         goto ERROR;
      }

      // Free the buffer
      if(  WSM_TYPE_PHYSICAL != wsidp->m_type ) {
         PDEBUG( "Workspace free failed due to bad WS type. Should be %d but received %d\n",WSM_TYPE_PHYSICAL,
               wsidp->m_type);

         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
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
      pownerSess->m_uiapi->freewsid(wsidp);

      // Create the  event
      pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                     Message.m_tranID,
                                                     Message.m_context,
                                                     uid_errnumOK);

      PVERBOSE("Sending the WKSP Free event.\n");
      // Send the event
      pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuws_evt),
                                     Message.m_context);
   } break; // case fappip_afucmdWKSP_FREE

   default: {
      PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);

      pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                  &Message.m_tranID,
                                                                  Message.m_context,
                                                                  request_error);

      pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuresponse_evt),
                                     Message.m_context);

      retval = -EINVAL;
   } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

ERROR:
   if ( NULL != p_localpayload ) {
      kfree(p_localpayload);
   }

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
// Name: cci_sim_mmap
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
cci_sim_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific)
{

   struct vm_area_struct     *pvma = (struct vm_area_struct *) os_specific;

   struct cci_PIPsession   *pSess = NULL;
   struct cci_device       *pdev = NULL;
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

   pdev = cci_PIPsessionp_to_cciv4dev(pSess);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PDEBUG("CCIV4 Simulator mmap: no device");
      goto ERROR;
   }

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   pvma->vm_ops = NULL;

   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if ( WSM_TYPE_CSR == wsidp->m_type ) {
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
