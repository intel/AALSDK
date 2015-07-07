//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
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
//  Copyright(c) 2012-2015, Intel Corporation.
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
//        FILE: spl2_primitives.c
//     CREATED: 02/10/2012
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Henry Mitchel, Intel <henry.mitchel@intel.com>
//
// PURPOSE:  Miscellaneous primitive functions
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/10/2012     JG       Ported from user mode test program written by
//                           Henry Mitchel.
// 10/25/2012     TSW      Cleanup for faplib
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_CFG

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalids.h"              // for HOST_AHM_GUID
#include "aalsdk/kernel/spl2defs.h"
#include "spl2_pip_internal.h"
#include "spl2_pip_public.h"
#include "spl2_session.h"
#include "xsid.h"
#include "mem-sess.h"
#include "spl2mem.h"
#include "spl2mem-kern.h"

// Prototypes
static int
spl2_set_afu_dsm(struct spl2_device *pdev, btCSROffset csroffset);

#define SPL2PIP_USE_PCI_CONFIG 0

//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_QPI,
   .m_busnum    = 0, // Always zero
   .m_devicenum = 1, //
   .m_subdevnum = 0  // AFU
};


//=============================================================================
// Name: spl2_alloc_next_afu_addr
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
spl2_alloc_next_afu_addr(void)
{
   ++(nextAFU_addr.m_subdevnum);
   if( 0 == (nextAFU_addr.m_subdevnum &= 0xffff) ) {
      ++(nextAFU_addr.m_devicenum);
      nextAFU_addr.m_devicenum &= 0xffff;
   }
   return nextAFU_addr;
} // spl2_alloc_next_afu_addr

//=============================================================================
// Name: check_pip_type_id
// Description: determine the physical protocol
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
static inline
int
check_pip_type_id(struct spl2_device *pspl2dev)
{
   // Check configuration space ID.
   struct PCIE_FEATURE_HDR_F3 volatile feature_id;

   feature_id.csr = read_cci_csr(pspl2dev, byte_offset_PCIE_FEATURE_HDR_F3);

   return feature_id.protocol;
} // check_pip_type_id

//=============================================================================
// Name: check_qlp_feature_id
// Description: check the feature id
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
static inline
int
check_qlp_feature_id(struct spl2_device *pspl2dev)
{
   // Check configuration space ID.
   struct PCIE_FEATURE_HDR_F2 volatile feature_id;

   feature_id.csr = read_cci_csr(pspl2dev, byte_offset_PCIE_FEATURE_HDR_F2);

   ASSERT(FeatureID_CCI3 == feature_id.FeatureID);
   if( FeatureID_CCI3 == feature_id.FeatureID ) {
      return 0; // success
   }

   PERR("CCI FeatureID found is 0x%X, but desired value is 0x%X.\n",
           feature_id.FeatureID, FeatureID_CCI3);

   return -1;
} // check_qlp_feature_id

//=============================================================================
// Name: spl2_identify_device
// Description: Reset the SPL
// Input: pdev - device to reset
// Comment:
// Returns: none
// Comments:
//=============================================================================
int spl2_identify_device(struct spl2_device *pspl2dev)
{
   int res;

   // Set CCI CSR Attributes
   spl2_dev_phys_cci_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev);
   spl2_dev_kvp_cci_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev);
   spl2_dev_len_cci_csr(pspl2dev) = QPI_APERTURE_SIZE;


   PNOTICE("QPI device found at phys=0x%" PRIxPHYS_ADDR " kvp=0x%p len=%" PRIuSIZE_T "\n",
              spl2_dev_phys_config(pspl2dev),
              spl2_dev_kvp_config(pspl2dev),
              spl2_dev_len_config(pspl2dev));

   // Verify the QLP version - note this is specific to QPI.
   res = check_qlp_feature_id(pspl2dev);
   ASSERT(0 == res);
   if ( 0 != res ) {
      PERR("Failed to verify CCI 3 protocol.\n");
      return -ENXIO;
   }
   PVERBOSE("Verified that device is running CCI 3 protocol.\n");

   // Get the Interface Protocol. CCI or SPL.
   spl2_dev_protocol(pspl2dev) = check_pip_type_id(pspl2dev);
   return 0;
}

//=============================================================================
// Name: spl2_cci_reset
// Description: Reset the SPL
// Input: pdev - device to reset
// Comment:
// Returns: success = 0; fail = 1
// Comments:
//=============================================================================
static inline
int
spl2_cci_reset(struct spl2_device *pdev)
{
   bt32bitCSR csr;
   btInt IsCpiustat0, IsCpiustat1, done;
   btInt timeout = 20;


   //Assert reset
   csr = read_cci_csr(pdev, byte_offset_CCI_CH_CTRL );

   csr |= 0x01000000;
   write_cci_csr32(pdev, byte_offset_CCI_CH_CTRL, csr);

   // Check for QLP transactions flushed
   IsCpiustat0 = (read_cci_csr(pdev, byte_offset_CCI_CH_STAT0) & 0x80000000);
   IsCpiustat1 = (read_cci_csr(pdev, byte_offset_CCI_CH_STAT1) & 0x80000000);
   done = IsCpiustat0 && IsCpiustat1;

   // While the timeout hasn't occurred
   while( --timeout && !done ) {
      mdelay(1);  // Wait for the hardware

      IsCpiustat0 = (read_cci_csr(pdev, byte_offset_CCI_CH_STAT0) & 0x80000000);
      IsCpiustat1 = (read_cci_csr(pdev, byte_offset_CCI_CH_STAT1) & 0x80000000);
      done = IsCpiustat0 && IsCpiustat1;
   }

   // Don't take it out of reset if device not functioning
   if( 0 != timeout ){
      //De-assert reset
      csr = read_cci_csr(pdev, byte_offset_CCI_CH_CTRL);
      csr  &= ~0x01000000; // Clear overall csr to 0
      write_cci_csr32(pdev, byte_offset_CCI_CH_CTRL, csr);
   }
   return (timeout == 0);

} // spl2_cci_reset

//=============================================================================
// Name: cci3_init_ccidevice
// Description:  Initialize the CCI3 device
// Interface: private
// Returns: 0 - success
// Inputs: [IN/OUT] pspl2dev - Device structure
//         [IN/OUT] paaldevid - Device ID to assign to new device
// Outputs: none.
// Comments:
//=============================================================================
int cci3_init_ccidevice(struct spl2_device            *pspl2dev,
                         struct aal_device_id         *paaldevid)
{
   PVERBOSE("Verified device is running CCI3 interface protocol.\n");

   PNOTICE("Single AFU support. Assuming AFU0\n");

   // Assume AFU0 for now. Later may use AFU # as index. Possibly use address from spl2_alloc_next_afu_addr()
#if 0
   spl2_dev_phys_afu_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev) + CCI3_AFU0_CSR_OFFSET;
   spl2_dev_kvp_afu_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev) + CCI3_AFU0_CSR_OFFSET;
   spl2_dev_len_afu_csr(pspl2dev) = SPL3_CSR_SPACE_SIZE;
#endif

   // Temporarily the exposed CSRs (i.e., AFU CSRs) include QLP space
   spl2_dev_phys_afu_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev);
   spl2_dev_kvp_afu_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev);
   spl2_dev_len_afu_csr(pspl2dev) = 0x2000;

   PNOTICE("qpi device CSR Space starting at phys=0x%" PRIxPHYS_ADDR " kvp=0x%p len=%" PRIuSIZE_T "\n",
               spl2_dev_phys_afu_csr(pspl2dev),
               spl2_dev_kvp_afu_csr(pspl2dev),
               spl2_dev_len_afu_csr(pspl2dev));

   // Populate an AAL device ID for it.
   aaldevid_addr(*paaldevid)            = spl2_alloc_next_afu_addr();
   aaldevid_devaddr_bustype(*paaldevid) = aal_bustype_QPI;

   // Set the device type and its interface (ie. PIP)
   aaldevid_devtype(*paaldevid)         = aal_devtypeAFU;
   aaldevid_vendorid(*paaldevid)        = AAL_vendINTC;
   aaldevid_pipguid(*paaldevid)         = QPI_CCIAFUPIP_IID;
   aaldevid_ahmguid(*paaldevid)         = QPI_AHM_GUID;

   spl2_dev_board_type(pspl2dev)        = aal_bustype_QPI;
   return 0;
}

//=============================================================================
// Name: spl2_cci_device_init
// Description:  Initialize the CCI device
// Interface: private
// Returns: 0 - success
// Inputs: [IN/OUT] pspl2dev - Device structure
//         [IN/OUT] paaldevid - Device ID to assign to new device
// Outputs: none.
// Comments:
//=============================================================================
int
spl2_cci_device_init( struct spl2_device   *pspl2dev,
                      struct aal_device_id *paaldevid,
                      struct list_head     *pdevlist)
{
   spl2_dev_AFUDSM_type volatile *pAFU_DSM;

   int res = -EINVAL;

   PTRACEIN;

   // Make sure we initialize these first - if we error out, then
   // spl2_device_destroy() will need them.
   INIT_LIST_HEAD(&pspl2dev->m_session_list);
   kosal_mutex_init(&pspl2dev->m_session_sem);
   kosal_mutex_init(spl2_dev_semp(pspl2dev));
   kosal_mutex_init(spl2_dev_tran_done_semp(pspl2dev));

   spl2_dev_mem_sessionp(pspl2dev) = spl2mem_fops.alloc(); // get kmalloc'd memory context
   ASSERT(spl2_dev_mem_sessionp(pspl2dev));
   if ( NULL == spl2_dev_mem_sessionp(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   res = spl2mem_fops.open(spl2_dev_mem_sessionp(pspl2dev)); // initialize the memory sub-system
   ASSERT(0 == res);
   if ( res ) {
      goto ERR;
   }


   // Alloc the AFU DSM
   spl2_dev_AFUDSM(pspl2dev) = (spl2_dev_AFUDSM_type *)__get_free_pages(GFP_KERNEL, get_order(spl2_dev_AFUDSM_size));
   if ( NULL == spl2_dev_AFUDSM(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   // Reset the device
   if(1 == spl2_cci_reset(pspl2dev)){
      PDEBUG("Failed to reset CCI device\n");
      res = -EIO;
      goto ERR;
   }

   // Set up the CCI AFU DSM.
   res = spl2_set_afu_dsm(pspl2dev, byte_offset_CSR_AFU_DSM_BASE);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }


   pAFU_DSM = (spl2_dev_AFUDSM_type *)spl2_dev_AFUDSM(pspl2dev);

   // Get the AFU ID from the AFU DSM
   aaldevid_afuguidl(*paaldevid) = pAFU_DSM->vafu2.AFU_ID[0];
   aaldevid_afuguidh(*paaldevid) = pAFU_DSM->vafu2.AFU_ID[1];

   PVERBOSE("Saving AFUID 0x%" PRIx64 " 0x%" PRIx64 " to aaldevid struct\n",
               aaldevid_afuguidh(*paaldevid), aaldevid_afuguidl(*paaldevid));

   // Remember the configuration space
   PVERBOSE("Begin PCI configuration space storage in memory sub-system\n");
   spl2mem_fops.ioctl_put_config(spl2_dev_mem_sessionp(pspl2dev),
                                 spl2_dev_phys_config(pspl2dev),
                                 spl2_dev_len_config(pspl2dev),
                                 spl2_dev_kvp_config(pspl2dev));

   if ( NULL != pdevlist ) {
      list_add(&(pspl2dev->m_session_list), pdevlist);

      PDEBUG("Created SPL2 device 0x%p with list head at 0x%p and put on list 0x%p\n",
                pspl2dev,
                &(pspl2dev->m_session_list),
                pdevlist);
   } else {
      PDEBUG("Created SPL2 device 0x%p with list head at 0x%p\n",
                pspl2dev,
                &(pspl2dev->m_session_list));
   }

   res = spl2_create_discovered_afu(pspl2dev, paaldevid, &CCIAFUpip);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   res = 0;

ERR:
   PTRACEOUT_INT(res);
   return res;
} // spl2_cci_device_init


//=============================================================================
// Name: spl2_init_spl3device
// Description:  Initialize the SPL3 device
// Interface: private
// Returns: 0 - success
// Inputs: [IN/OUT] pspl2dev - Device structure
//         [IN/OUT] paaldevid - Device ID to assign to new device
// Outputs: none.
// Comments:
//=============================================================================
int spl2_init_spl3device(struct spl2_device           *pspl2dev,
                         struct aal_device_id         *paaldevid)
{
   PVERBOSE("Verified device is running SPL interface protocol.\n");

   PNOTICE("Single AFU support. Assuming AFU0\n");

   // Assume AFU0 for now. Later may use AFU # as index. Possibly use address from spl2_alloc_next_afu_addr()
   spl2_dev_phys_afu_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev) + SPL3_AFU0_CSR_OFFSET;
   spl2_dev_kvp_afu_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev) + SPL3_AFU0_CSR_OFFSET;
   spl2_dev_len_afu_csr(pspl2dev) = SPL3_CSR_SPACE_SIZE;

   PNOTICE("QPI AFU CSR Space starting at phys=0x%" PRIxPHYS_ADDR " kvp=0x%p len=%" PRIuSIZE_T "\n",
               spl2_dev_phys_afu_csr(pspl2dev),
               spl2_dev_kvp_afu_csr(pspl2dev),
               spl2_dev_len_afu_csr(pspl2dev));

   // Populate an AAL device ID for it.
   aaldevid_addr(*paaldevid)            = spl2_alloc_next_afu_addr();
   aaldevid_devaddr_bustype(*paaldevid) = aal_bustype_QPI;

   // Set the device type and its interface (ie. PIP)
   aaldevid_devtype(*paaldevid)         = aal_devtypeAFU;
   aaldevid_vendorid(*paaldevid)        = AAL_vendINTC;
   aaldevid_pipguid(*paaldevid)         = SPL2_AFUPIP_IID;
   aaldevid_ahmguid(*paaldevid)         = QPI_AHM_GUID;

   spl2_dev_board_type(pspl2dev)        = aal_bustype_QPI;
   return 0;
}

//=============================================================================
// Name: spl2_task_poller
// Description:
// Interface: private
// Returns: N/A
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
void
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
spl2_task_poller( struct work_struct *work )
{
   struct delayed_work * delayedWork = container_of(work, struct delayed_work, work);
   struct spl2_device *pdev = container_of( delayedWork, struct spl2_device, task_handler );
#else
spl2_task_poller(void *arg)
{
   struct spl2_device *pdev = (struct spl2_device *) arg;
#endif
   struct spl2_session      *pspl2sess;
   struct SPL2_DSM volatile *pDSM = spl2_dev_SPLDSM(pdev);
   static int count = 0;

   if ( ++count >= 1000 ) {
      count = 0;
      PVERBOSE("Checking AFU state");
   }

   // Is the AFU done
   if ( pDSM->Valid ) {
      struct aalui_AFUResponse               UIResponse;
      struct uidrv_event_afu_response_event *pafuresponse_evt;
      struct spl2_session                   *psess = spl2_dev_activesess(pdev);

      // The transaction completed - notify that we're done polling.
      up( spl2_dev_tran_done_semp(pdev) );

      PVERBOSE("Got Valid; clearing transaction.\n");

      // Ugly hack to retrieve VAFU2-specific counters.
      PVERBOSE("AFU_DSM_LATENCY     = 0x%p", (void *)pdev->m_AFUDSM->AFU_DSM_LATENCY);
      PVERBOSE("AFU_DSM_PERFORMANCE = 0x%p", (void *)pdev->m_AFUDSM->AFU_DSM_PERFORMANCE);

      spl2_trans_end(pdev);

      memset(&UIResponse, 0, sizeof(struct aalui_AFUResponse));
      UIResponse.respID  = uid_afurespTaskComplete;

      // Generate the task complete event
      pafuresponse_evt = uidrv_event_afutranstate_create(pdev,
                                                         &spl2_sessionp_currTran(psess),
                                                         spl2_sessionp_currContext(psess),
                                                         &UIResponse,
                                                         &spl2_dev_AFUDSM_WSMParms(pdev),
                                                         (pDSM->Error ? uid_errnumAFUTransaction : uid_errnumOK));
      if ( pDSM->Error ) {
         PERR("Error from DSM. Error_Status=%u Fault_Request_Type=%u Error_Address=0x%llx\n",
               pDSM->Error_Status,
               pDSM->Fault_Request_Type,
               pDSM->Error_Address);
      }

      // Free the internal workspace associated with the AFU DSM.

      down( spl2_sessionp_semaphore(psess) );

      ASSERT(NULL != spl2_dev_AFUDSM_wsid(pdev));
      spl2_sessionp_to_ownerSession(psess)->m_uiapi->freewsid(spl2_dev_AFUDSM_wsid(pdev));
      spl2_dev_AFUDSM_wsid(pdev) = NULL;

      up( spl2_sessionp_semaphore(psess) );

      // Send the event
      spl2_sessionp_to_ownerSession(psess)->m_uiapi->sendevent( spl2_sessionp_to_ownerSession(psess)->m_UIHandle,
                                                                spl2_dev_to_aaldev(pdev),
                                                                AALQIP(pafuresponse_evt),
                                                                spl2_sessionp_currMsgContext(psess));
      PVERBOSE("SPL2 Task DONE\n");
   } else {

      down( spl2_dev_semp(pdev) );

      pspl2sess = spl2_dev_activesess(pdev);

      if ( NULL != pspl2sess ) {
         if ( SPL2_SESS_STATE_CTXT_ACTIVE == spl2_sessionp_currState(pspl2sess) ) {
            // Continue polling.
            up( spl2_dev_semp(pdev) );
            queue_delayed_work(spl2_dev_workq(pdev), spl2_dev_task_handler(pdev), msecs_to_jiffies(spl2_dev_pollrate(pdev)));
            return;
         }
      }

      up( spl2_dev_semp(pdev) );

      PVERBOSE("SPL2 Task CANCELED\n");

      // The transaction was canceled - notify that we're done polling.
      up( spl2_dev_tran_done_semp(pdev) );
   }
} // spl2_task_poller

#define SPL_DSM_TIMEOUT 100
//=============================================================================
// Name: spl2_set_spl_dsm
// Description: Setup the SPL Device State Memory region
// Input: pdev - device
// Comment:
// Returns: 0 - Success
// Comments:
//=============================================================================
static
int
spl2_set_spl_dsm(struct spl2_device *pdev)
{
   struct CCIAFU_DSM volatile *pSPLDSM = (struct CCIAFU_DSM *)spl2_dev_SPLDSM(pdev);
   int                         timeout = SPL_DSM_TIMEOUT;

   // Clear and point HW at DSM
   memset((void*)pSPLDSM, 0, spl2_dev_SPLDSM_size);

   // Actual write -- First actual probe of HW
   PDEBUG("Writing SPL DSM Physical Base Address to config space\n");
   write_cci_csr64(pdev, byte_offset_SPL_DSM_BASE, virt_to_phys(pSPLDSM));

   // While not timed out check status for reset complete
   while( --timeout && (SPL2_ID != pSPLDSM->cci_afu_id) ) {
      mdelay(1);  // Wait for the hardware
   }

   PDEBUG("%s expected cci_afu_id (got 0x%x) at SPL DSM's physical address 0x%p\n",
               (SPL2_ID == pSPLDSM->cci_afu_id) ?  "Got" : "Didn't get",
               pSPLDSM->cci_afu_id, (void*)virt_to_phys(pSPLDSM));

   // biased toward late success
   ASSERT(SPL2_ID == pSPLDSM->cci_afu_id);
   return (SPL2_ID == pSPLDSM->cci_afu_id) ? 0 : -EIO;
} // spl2_set_spl_dsm

#define AFU_DSM_TIMEOUT 100
//=============================================================================
// Name: spl2_set_afu_dsm
// Description: Set the AFU Device State Memory region
// Input: pdev - device
// Comment:
// Returns: 0 - Success
// Comments:
//=============================================================================
static
int
spl2_set_afu_dsm(struct spl2_device *pdev, btCSROffset csroffset)
{
   spl2_dev_AFUDSM_type volatile *pAFU_DSM = (spl2_dev_AFUDSM_type *)spl2_dev_AFUDSM(pdev);
   int                            timeout  = AFU_DSM_TIMEOUT;

   // Clear and point HW at DSM
   memset((void*)pAFU_DSM, 0, spl2_dev_AFUDSM_size);

   // Actual write of AFU DSM address to SPL
   PVERBOSE("Writing AFU DSM Physical Base Address to config space\n");
   write_cci_csr64(pdev, csroffset, virt_to_phys(pAFU_DSM));

   // Wait for timepout or AFU ID to appear
   while ( ((0 == pAFU_DSM->vafu2.AFU_ID[1]) && (0 == pAFU_DSM->vafu2.AFU_ID[0]) ) && --timeout ) {
      mdelay(1);  // Wait for the hardware
      // While not timed out check status for reset complete
   }

   ASSERT(timeout > 0);
   if ( 0 != timeout ) {
      PVERBOSE("Read AFUID 0x%" PRIx64 " 0x%" PRIx64 " from AFU DSM's physical address 0x%p\n",
                  pAFU_DSM->vafu2.AFU_ID[1], pAFU_DSM->vafu2.AFU_ID[0], (void*)virt_to_phys(pAFU_DSM));
      return 0;
   } else {
      PVERBOSE("Timed out attempting to read AFUID.\n");
      return -EIO;
   }
} // spl2_set_afu_dsm

static inline
void
spl2_set_afu_context(struct spl2_device *pdev, bt64bitCSR wsuvaddr)
{
   write_cci_csr64(pdev, byte_offset_AFU_CNTXT_BASE, wsuvaddr);
}

static inline
void
spl2_set_spl_context(struct spl2_device *pdev)
{
   write_cci_csr64(pdev, byte_offset_SPL2_CNTXT_BASE, virt_to_phys(spl2_dev_SPLCTX(pdev)));
}


int
spl2_spl_device_init(struct spl2_device   *pspl2dev,
                     struct aal_device_id *paaldevid,
                     struct list_head     *pdevlist)
{
   spl2_dev_AFUDSM_type volatile *pAFU_DSM;

   int res = -EINVAL;

   PTRACEIN;

   // Make sure we initialize these first - if we error out, then
   // spl2_device_destroy() will need them.
   INIT_LIST_HEAD(&pspl2dev->m_session_list);
   kosal_mutex_init(&pspl2dev->m_session_sem);
   kosal_mutex_init(spl2_dev_semp(pspl2dev));
   kosal_mutex_init(spl2_dev_tran_done_semp(pspl2dev));

   spl2_dev_mem_sessionp(pspl2dev) = spl2mem_fops.alloc(); // get kmalloc'd memory context
   ASSERT(spl2_dev_mem_sessionp(pspl2dev));
   if ( NULL == spl2_dev_mem_sessionp(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   res = spl2mem_fops.open(spl2_dev_mem_sessionp(pspl2dev)); // initialize the memory sub-system
   ASSERT(0 == res);
   if ( res ) {
      goto ERR;
   }


   // Alloc the SPL Device Status Memory (DSM) region
   spl2_dev_SPLDSM(pspl2dev) = (spl2_dev_SPLDSM_type *)__get_free_pages(GFP_KERNEL, get_order(spl2_dev_SPLDSM_size));
   if ( NULL == spl2_dev_SPLDSM(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   // Alloc the SPL Context region
   spl2_dev_SPLCTX(pspl2dev) = (spl2_dev_SPLCTX_type *)__get_free_pages(GFP_KERNEL, get_order(spl2_dev_SPLCTX_size));
   if ( NULL == spl2_dev_SPLCTX(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   // Alloc the AFU DSM
   spl2_dev_AFUDSM(pspl2dev) = (spl2_dev_AFUDSM_type *)__get_free_pages(GFP_KERNEL, get_order(spl2_dev_AFUDSM_size));
   if ( NULL == spl2_dev_AFUDSM(pspl2dev) ) {
      res = -ENOMEM;
      goto ERR;
   }

   // Reset the device
   spl2_spl_reset(pspl2dev);

   // Set up the SPL DSM.
   res = spl2_set_spl_dsm(pspl2dev);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   // Set up the AFU DSM.
   res = spl2_set_afu_dsm(pspl2dev, byte_offset_AFU_DSM_BASE);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   pAFU_DSM = (spl2_dev_AFUDSM_type *)spl2_dev_AFUDSM(pspl2dev);

   // Get the AFU ID from the AFU DSM
   aaldevid_afuguidl(*paaldevid) = pAFU_DSM->vafu2.AFU_ID[0];
   aaldevid_afuguidh(*paaldevid) = pAFU_DSM->vafu2.AFU_ID[1];

   PVERBOSE("Saving AFUID 0x%" PRIx64 " 0x%" PRIx64 " to aaldevid struct\n",
               aaldevid_afuguidh(*paaldevid), aaldevid_afuguidl(*paaldevid));

   // Remember the configuration space
   PVERBOSE("Begin PCI configuration space storage in memory sub-system\n");
   spl2mem_fops.ioctl_put_config(spl2_dev_mem_sessionp(pspl2dev),
                                 spl2_dev_phys_config(pspl2dev),
                                 spl2_dev_len_config(pspl2dev),
                                 spl2_dev_kvp_config(pspl2dev));

   if ( NULL != pdevlist ) {
      list_add(&(pspl2dev->m_session_list), pdevlist);

      PDEBUG("Created SPL2 device 0x%p with list head at 0x%p and put on list 0x%p\n",
                pspl2dev,
                &(pspl2dev->m_session_list),
                pdevlist);
   } else {
      PDEBUG("Created SPL2 device 0x%p with list head at 0x%p\n",
                pspl2dev,
                &(pspl2dev->m_session_list));
   }

   res = spl2_create_discovered_afu(pspl2dev, paaldevid, &SPLAFUpip);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   res = 0;

ERR:
   PTRACEOUT_INT(res);
   return res;
} // spl2_device_init


//=============================================================================
// Name: spl2_device_destroy
// Description: Destroy the device
// Input: pspl2dev - device to destroy
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
spl2_device_destroy(struct spl2_device *pspl2dev)
{
   PDEBUG("Destroying struct spl2_device * 0x%p\n", pspl2dev);

   // TODO Change this when we enable device sharing.
   if(PCIE_FEATURE_HDR3_PROTOCOL_SPL == spl2_dev_protocol(pspl2dev) ){
      spl2_trans_end(pspl2dev);
   }else if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pspl2dev)){
      // Ignore return code as there is nothing we can do.
      spl2_cci_reset(pspl2dev);
   }

   PDEBUG("Synchronized with any pending tasks.\n");

   if ( spl2_dev_to_aaldev(pspl2dev) ) {
      PINFO("Removing AFU device\n");
      aalbus_get_bus()->unregister_device(spl2_dev_to_aaldev(pspl2dev));
      spl2_dev_to_aaldev(pspl2dev) = NULL;
   }

   if ( spl2_dev_is_simulated(pspl2dev) ) {
      PINFO("Removing Simulator AFU device\n");
      aalbus_get_bus()->unregister_device(spl2_dev_to_aalsimdev(pspl2dev));

      if ( NULL != spl2_dev_kvp_config(pspl2dev) ) {
         kfree(spl2_dev_kvp_config(pspl2dev));
         spl2_dev_kvp_config(pspl2dev) = NULL;
      }

      spl2_dev_clr_simulated(pspl2dev);
   } else if ( NULL != spl2_dev_kvp_config(pspl2dev) ) {
      iounmap(spl2_dev_kvp_config(pspl2dev));
      spl2_dev_kvp_config(pspl2dev) = NULL;
   }

   if ( NULL != spl2_dev_AFUDSM(pspl2dev) ) {
      free_pages((long unsigned int)spl2_dev_AFUDSM(pspl2dev), get_order(spl2_dev_AFUDSM_size));
      spl2_dev_AFUDSM(pspl2dev) = NULL;
   }

   if ( NULL != spl2_dev_SPLCTX(pspl2dev) ) {
      free_pages((long unsigned int)spl2_dev_SPLCTX(pspl2dev), get_order(spl2_dev_SPLCTX_size));
      spl2_dev_SPLCTX(pspl2dev) = NULL;
   }

   if( NULL != spl2_dev_SPLDSM(pspl2dev) ) {
      free_pages((long unsigned int)spl2_dev_SPLDSM(pspl2dev), get_order(spl2_dev_SPLDSM_size));
      spl2_dev_SPLDSM(pspl2dev) = NULL;
   }

   if ( NULL != spl2_dev_mem_sessionp(pspl2dev) ) {
      spl2mem_fops.close(spl2_dev_mem_sessionp(pspl2dev));
      spl2mem_fops.free(spl2_dev_mem_sessionp(pspl2dev));
      spl2_dev_mem_sessionp(pspl2dev) = NULL;
   }

   if ( spl2_dev_pci_dev_is_region_requested(pspl2dev) ) {
      pci_release_regions(spl2_dev_pci_dev(pspl2dev));
      spl2_dev_pci_dev_clr_region_requested(pspl2dev);
   }

   if ( spl2_dev_pci_dev_is_enabled(pspl2dev) ) {
      pci_disable_device(spl2_dev_pci_dev(pspl2dev));
      spl2_dev_pci_dev_clr_enabled(pspl2dev);
   }
} // spl2_device_destroy


/// Checks for an active session on the given pdev. If none is found..
/// 1) Claims the pdev for psess
/// 2) Issues an SPL2 reset
/// 3) Sets the SPL and AFU DSMs
/// 4) Updates the internal state flag of the session
///
/// @psess Session object pointer
/// @pdev Device pointer
///
/// @retval uid_errnumOK on success.
uid_errnum_e
spl2_trans_setup(struct spl2_session *psess,
                 struct spl2_device  *pdev)
{
   int retval;

   ASSERT(psess);
   ASSERT(pdev);

   PVERBOSE("Setting up an SPL2 Transaction.\n");

   down( spl2_sessionp_semaphore(psess) );

   // If there is an active session on the device AND it is NOT the one trying to claim it then
   //   fail. Its OK to reclaim a device already owned
   if ( spl2_dev_activesess(pdev) && (psess != spl2_dev_activesess(pdev)) ) {
      // There is already an active session for the device - this is a protocol violation.

      up( spl2_sessionp_semaphore(psess) );

      PDEBUG("Error: SPL2 dev 0x%p already has active session 0x%p\n",
                pdev,
                spl2_dev_activesess(pdev));

      return uid_errnumPermission;
   }

   // No active session or already owned - claim the device.
   spl2_sessionp_set_tranowner(psess);


   ASSERT(SPL2_SESS_STATE_NONE == spl2_sessionp_currState(psess));
   if(SPL2_SESS_STATE_NONE != spl2_sessionp_currState(psess)){
      PDEBUG("SPL device already has active task\n");
      up( spl2_sessionp_semaphore(psess) );
      return uid_errnumDeviceBusy;
   }

   spl2_sessionp_currPollrate(psess) = 0;
   spl2_sessionp_currState(psess) = SPL2_SESS_STATE_BEGIN;

   up( spl2_sessionp_semaphore(psess) );

   spl2_spl_reset(pdev);

   retval = spl2_set_spl_dsm(pdev);
   if ( retval ) {
      down( spl2_sessionp_semaphore(psess) );
      spl2_sessionp_currState(psess) = SPL2_SESS_STATE_NONE;
      spl2_sessionp_clear_tranowner(psess);
      up( spl2_sessionp_semaphore(psess) );
      return uid_errnumCouldNotClaimDevice;
   }

   retval = spl2_set_afu_dsm(pdev, byte_offset_AFU_DSM_BASE);
   if ( retval ) {
      down( spl2_sessionp_semaphore(psess) );
      spl2_sessionp_currState(psess) = SPL2_SESS_STATE_NONE;
      spl2_sessionp_clear_tranowner(psess);
      up( spl2_sessionp_semaphore(psess) );
      return uid_errnumCouldNotClaimDevice;
   }

   down( spl2_sessionp_semaphore(psess) );
   spl2_sessionp_currState(psess) = SPL2_SESS_STATE_DSMs_SET;
   up( spl2_sessionp_semaphore(psess) );

   return uid_errnumOK;
} // spl2_trans_setup



/// 1) Sets the AFU Context, using the buffer indicated by @pwsid
/// 2) Enables the SPL2 device
/// 3) Initializes and sets the SPL context, kicking off the transaction.
/// 4) Updates the internal state flag of the session
///
/// @psess Session object pointer
/// @pdev Device pointer
/// @pwsid Workspace object pointer of AFU Context
/// @pollrate Poll rate in milliseconds for checking the device status
///
/// @retval uid_errnumOK on success.
uid_errnum_e
spl2_trans_start(struct spl2_session *psess,
                 struct spl2_device  *pdev,
                 struct aal_wsid     *pwsid,
                 unsigned             pollrate)
{
   uid_errnum_e           err;
   struct memmgr_session *pmem_sess;
   xsid_t                 xsid;

   ASSERT(psess);
   ASSERT(pdev);

   ASSERT(pwsid);
   if ( NULL == pwsid ) {
      return uid_errnumBadParameter;
   }

   xsid = (xsid_t)pwsid->m_id;
   ASSERT(spl2_memsession_valid_xsid(xsid));
   if ( !spl2_memsession_valid_xsid(xsid) ) {
      return uid_errnumBadParameter;
   }

   pmem_sess = spl2_dev_mem_sessionp(pdev);
   ASSERT(pmem_sess);
   if ( NULL == pmem_sess ) {
      return uid_errnumInvalidRequest;
   }

   ASSERT( spl2_memsession_virtmem(pmem_sess, xsid).m_pte_array_uvaddr );
   if ( __UINT64_T_CONST(0) == spl2_memsession_virtmem(pmem_sess, xsid).m_pte_array_uvaddr ) {
      return uid_errnumInvalidRequest;
   }


   if ( SPL2_SESS_STATE_DSMs_SET != spl2_sessionp_currState(psess) ) {
      // The transaction has not been set up. Try now.
      err = spl2_trans_setup(psess, pdev);
      if ( uid_errnumOK != err ) {
         return err;
      }
   }

   ASSERT(psess == spl2_dev_activesess(pdev));
   ASSERT(SPL2_SESS_STATE_DSMs_SET == spl2_sessionp_currState(psess));

   PVERBOSE("Starting an SPL2 Transaction.\n");

   // Set up the SPL context
   spl2_dev_clrSPLCTX(pdev);
   spl2_dev_SPLCTX(pdev)->phys_addr_page_table  = spl2_memsession_virtmem(pmem_sess,xsid).m_pte_array_physaddr;
   spl2_dev_SPLCTX(pdev)->virt_addr_afu_context = spl2_memsession_virtmem(pmem_sess,xsid).m_pte_array_uvaddr;
   spl2_dev_SPLCTX(pdev)->page_size             = 1; // 1 = 2MB, only supported value currently
   spl2_dev_SPLCTX(pdev)->num_valid_ptes        = spl2_memsession_virtmem(pmem_sess,xsid).m_num_of_valid_pte;
   spl2_dev_SPLCTX(pdev)->control_flags         = 1; // 1 = virtual

   PVERBOSE("SPL2 Context at 0x%p contains phys_addr_page_table 0x%p virt_addr_afu_context 0x%p page_size %d num_valid_ptes %d control_flags %lx\n",
               (void*)virt_to_phys(spl2_dev_SPLCTX(pdev)),
               (void*)spl2_dev_SPLCTX(pdev)->phys_addr_page_table,
               (void*)spl2_dev_SPLCTX(pdev)->virt_addr_afu_context,
               spl2_dev_SPLCTX(pdev)->page_size,
               spl2_dev_SPLCTX(pdev)->num_valid_ptes,
               (unsigned long)spl2_dev_SPLCTX(pdev)->control_flags);

   // Enable the AFU's workspace and kick off the transaction.
   spl2_set_spl_context(pdev);
   spl2_set_afu_context(pdev, spl2_memsession_virtmem(pmem_sess, xsid).m_pte_array_uvaddr);
   spl2_enable(pdev, SPL2_ENABLE);

   down( spl2_sessionp_semaphore(psess) );

   spl2_sessionp_currPollrate(psess) = pollrate;
   spl2_sessionp_currState(psess)    = SPL2_SESS_STATE_CTXT_ACTIVE;

   up( spl2_sessionp_semaphore(psess) );

   return uid_errnumOK;
} // spl2_trans_start

/// 1) Disables the SPL2 device
/// 2) Issues a device reset
/// 3) Clears the 'Transaction Owner' from the active session
/// 4) Updates the internal state flag of the session
///
/// @pdev Device pointer
void
spl2_trans_end(struct spl2_device *pspl2dev)
{
   struct spl2_session *psess;

   ASSERT(pspl2dev);
   if ( NULL == pspl2dev ) {
      PERR("Got NULL spl2_device *\n");
      return;
   }

   down( spl2_dev_semp(pspl2dev) );

   psess = spl2_dev_activesess(pspl2dev);
   if ( NULL != psess ) {
      down( spl2_sessionp_semaphore(psess) );
      ASSERT(SPL2_SESS_STATE_CTXT_ACTIVE == spl2_sessionp_currState(psess));

      spl2_sessionp_currState(psess) = SPL2_SESS_STATE_NONE;
      spl2_sessionp_clear_tranowner(psess);

      up( spl2_sessionp_semaphore(psess) );
   }

   up( spl2_dev_semp(pspl2dev) );

   // Wait for any queued work to finish.
   down( spl2_dev_tran_done_semp(pspl2dev) );

   spl2_enable(pspl2dev, SPL2_DISABLE);
   spl2_spl_reset(pspl2dev);

   up( spl2_dev_tran_done_semp(pspl2dev) );

} // spl2_trans_end

//=============================================================================
// Name: create_discoveredAFU
// Description: Create and register an AAL device to represent the newly
//              discovered device.
// Input: pdev - device to attach simulator to.
// Comment: This function understands whether to register a
// Returns: 0 - Success
// Comments:
//=============================================================================
int
spl2_create_discovered_afu(struct spl2_device   *pdev,
                           struct aal_device_id *pdevid,
                           struct aal_ipip      *pafupip)
{
   // Used to send the create request to aalbus::afu_factory
   struct mafu_CreateAFU request;
   struct aal_device   **ppdev   = NULL;
   struct aal_ipip      *ppip    = NULL;

   // Pointer to the AAL Bus interface
   struct aal_bus       *AALbusp = aalbus_get_bus();

   ASSERT(pdev);
   ASSERT(pdevid);
   ASSERT(AALbusp);

   memset(&request, 0, sizeof(struct mafu_CreateAFU));

   // Set the ID for the AFU device to create
   request.device_id = *pdevid;

   // Name that will appear in sysfs
   strncpy(request.basename, DEVICE_BASENAME, sizeof(request.basename));

   // Not share-able
   request.maxshares = 1;

   // If we are creating a MAFU instantiate to the simdev
   if ( (SPL2_SIM_AFUIDL == aaldevid_afuguidl(*pdevid)) &&
        (SPL2_SIM_AFUIDH == aaldevid_afuguidh(*pdevid)) ) {
      // Point to the MAFU PIP
      ppip = spl2_MAFU_ppip_to_aalpipip(&MAFUpip);

      // Where to store the new aal_device
      //  if creating a MAFU store in m_simdev
      ppdev = &spl2_dev_to_aalsimdev(pdev);

      // Direct user mode CSR interface not supported
      request.enableDirectAPI = AAL_DEV_APIMAP_NONE;
      spl2_dev_clr_allow_map_csr_write_space(pdev);
      spl2_dev_clr_allow_map_csr_read_space(pdev);
   } else {
      // otherwise we are creating real afu
      ppip  = pafupip;

      // If creating the AFU use m_aaldev
      ppdev = &spl2_dev_to_aaldev(pdev);

      // Set up the unchanging parameters of the AFU DSM.
      spl2_dev_AFUDSM_WSMParms(pdev).size    = spl2_dev_AFUDSM_size;
      spl2_dev_AFUDSM_WSMParms(pdev).physptr = (btPhysAddr)virt_to_phys(spl2_dev_AFUDSM(pdev));

      if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pdev)){
         // Read and Write CSR is allowed.
         request.enableDirectAPI |= AAL_DEV_APIMAP_CSRWRITE;
         request.enableDirectAPI |= AAL_DEV_APIMAP_CSRREAD;
         spl2_dev_set_allow_map_csr_write_space(pdev);
         spl2_dev_set_allow_map_csr_read_space(pdev);

      }else{
         // For SPL Write CSR is allowed. Read CSR is not.
         request.enableDirectAPI |= AAL_DEV_APIMAP_CSRWRITE;
         request.enableDirectAPI &= ~AAL_DEV_APIMAP_CSRREAD;
         spl2_dev_set_allow_map_csr_write_space(pdev);
         spl2_dev_clr_allow_map_csr_read_space(pdev);

         // Initialize the worker thread
         spl2_dev_workq(pdev) = create_workqueue( request.basename );

   #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
         INIT_DELAYED_WORK( spl2_dev_task_handler(pdev), spl2_task_poller );
   #else
         INIT_WORK( spl2_dev_task_handler(pdev), spl2_task_poller, pdev );
   #endif
      }


   }
   ASSERT(ppip);
   ASSERT(ppdev);

   // Create the AAL AFU.
   //   Notes:
   //   When creating an AAL AFU a Physical Interface Protocol (PIP) is
   //   associated with the object. The PIP represents the command interface
   //   to the object. The PIP is often implemented as an AAL kernel service
   //   so that it may be shared by different device implementations. The ID
   //   of the PIP to associate with the AFU is specified in the aal_device_id
   //   (see above).  There are 2 ways that the PIP implementation can be associated
   //   (ie bound) to a new AFU.  If the PIP is implemented in a module (KSM)
   //   that is different from the module creating the AFU, then AAL kernel services
   //   will bind the PIP to the AFU in the kernel framework.  This will cause the
   //   module implementing the PIP to have its reference count incremented so that
   //   it cannot be removed while the PIP is in use.
   //
   //   If the PIP is implemented in the same module in which the AFU is being created,
   //   as in this example, then the PIP pointer should be directly provided
   //   to the AAL device factory at construction time. Not providing it at
   //   construction time will cause in THIS module's reference count to increment,
   //   which may prevent the module from being removed unless special provisions
   //   have been put in place by the module designer.

   // First arg is the request describing the AFU to create, 2nd is a pointer
   //  to call when the AFU is released (removed from the system) and the 3rd
   //  is a pointer to the device's PIP (see above explanation).

   // This will store the newly created AAL device in the appropriate variable
   //  as setup above.

   *ppdev = aaldev_factp(AALbusp).create(&request, AFUrelease_device, ppip);
   ASSERT(*ppdev);
   if ( NULL == *ppdev ) {
      return -ENODEV;
   }

   // The PIP uses the PIP context to get a handle to the AAL device.
   //  The generic (i.e., base class) AFU pointer is passed around in messages to the PIP
   //  typically through the owner session object.  .
   aaldev_pip_context(*ppdev) = (void*)pdev;

   PDEBUG("Factory Create AFU=struct aal_device * 0x%p dev=struct spl2_device * 0x%p\n",
             *ppdev, pdev);

   return 0;
} // spl2_create_discovered_afu

////////////////////////////////////////////////////////////////////////////////
//
// CCI CSR Accessor/Mutators
//
////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: read_cci_csr
// Description: read a csr value from the PCIe CCI configuration space
// Input: pdev - device
//        offset - to read
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef read_cci_csr
#undef read_cci_csr
#endif
bt32bitCSR
read_cci_csr(struct spl2_device *pspl2dev, btCSROffset offset)
{
#if (1 == SPL2PIP_USE_PCI_CONFIG)
   bt32bitCSR val = 0;
#endif // SPL2PIP_USE_PCI_CONFIG

   // If this is a simulated device then notify the simulator
   if ( spl2_dev_is_simulated(pspl2dev) ) {
      PVERBOSE("Reading simulated CSR.\n");
      spl2_dev_to_simpip(pspl2dev).read_cci_csr(pspl2dev, offset);
   }

#if (1 == SPL2PIP_USE_PCI_CONFIG)

   if ( pci_read_config_dword(spl2_dev_pci_dev(pspl2dev), offset, &val) ) {
      PERR("pci_read_config_dword(struct pci_dev *=0x%p, where=%u (0x%x), &val) failed\n",
              spl2_dev_pci_dev(pspl2dev),
              offset,
              offset);
      return -1;
   }

   return val;

#else

   ASSERT(spl2_dev_kvp_cci_csr(pspl2dev));
   if ( spl2_dev_kvp_cci_csr(pspl2dev) ) {
      char       volatile *p  = ((char volatile *)spl2_dev_kvp_cci_csr(pspl2dev)) + offset; // offset is in bytes
      bt32bitCSR volatile u;
      bt32bitCSR volatile *up = (bt32bitCSR volatile *)p;
      PVERBOSE("Reading from address 0x%p\n", p);
      u = *up;
      PVERBOSE("Returned value is 0x%x %u\n", u, u);
      return u;
   } else {
      PERR("pdev->m_kvp_cci_csr NULL, could not read CSR 0x%X. Returning -1 in lieu.\n",
           offset);
      return            -1;                          // typical value for undefined CSR
   }

#endif // SPL2PIP_USE_PCI_CONFIG
}  // read_cci_csr

//=============================================================================
// Name: write_cci_csr32
// Description: write csr value to the PCIe CCI configuration space
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef write_cci_csr32
#undef write_cci_csr32
#endif
void
write_cci_csr32(struct spl2_device *pspl2dev, btCSROffset offset, bt32bitCSR value)
{
#if (1 == SPL2PIP_USE_PCI_CONFIG)

   if ( pci_write_config_dword(spl2_dev_pci_dev(pspl2dev), offset, value) ) {
      PERR("pci_write_config_dword(struct pci_dev *=0x%p, where=%u (0x%x), value=%u (0x%x)) failed\n",
              spl2_dev_pci_dev(pspl2dev),
              offset, offset,
              value, value);
   }

#else
   ASSERT(spl2_dev_kvp_config(pspl2dev));

   if( spl2_dev_kvp_config(pspl2dev) ) {
      btVirtAddr  p  = ((btVirtAddr)spl2_dev_kvp_cci_csr(pspl2dev)) + offset; // offset is in bytes
      bt32bitCSR *up = (bt32bitCSR *)p;
      PVERBOSE("Writing value 0x%x %d to offset 0x%x %d at address 0x%p\n",
               value, value, offset, offset, p);
      *up            = value;
   } else {
      PERR("pdev->.m_kvp_config NULL, could not write CSR 0x%X with value 0x%X\n",
           offset, value);
   }
#endif // SPL2PIP_USE_PCI_CONFIG

   // If this is a simulated device then notify the simulator
   if ( spl2_dev_is_simulated(pspl2dev) ) {
      PVERBOSE("Writing simulated CSR.\n");
      spl2_dev_to_simpip(pspl2dev).write_cci_csr32(pspl2dev, offset, value);
   }
} // write_cci_csr32

//=============================================================================
// Name: write_cci_csr64
// Description: write a qword csr using our SPL2 convention of high dword
//              followed by low dword
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef write_cci_csr64
#undef write_cci_csr64
#endif
void
write_cci_csr64(struct spl2_device *pspl2dev, btCSROffset offset, bt64bitCSR value)
{
   // write high
   write_cci_csr32(pspl2dev, offset+4, (bt32bitCSR)(value>>32));
   // write low
   write_cci_csr32(pspl2dev, offset, (bt32bitCSR)value);
} // write_cci_csr64

////////////////////////////////////////////////////////////////////////////////
//
// AFU CSR Accessor/Mutators
//
////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: read_afu_csr
// Description: read a csr value from the AFU configuration space
// Input: pdev - device
//        offset - to read
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef read_afu_csr
#undef read_afu_csr
#endif
bt32bitCSR
read_afu_csr(struct spl2_device *pspl2dev, btCSROffset offset)
{
#if (1 == SPL2PIP_USE_PCI_CONFIG)
   bt32bitCSR val = 0;
#endif // SPL2PIP_USE_PCI_CONFIG

   // If this is a simulated device then notify the simulator - TODO No AFU variant
   if ( spl2_dev_is_simulated(pspl2dev) ) {
      PVERBOSE("Reading simulated CSR.\n");
      spl2_dev_to_simpip(pspl2dev).read_cci_csr(pspl2dev, offset);
   }

#if (1 == SPL2PIP_USE_PCI_CONFIG)

   if ( pci_read_config_dword(spl2_dev_pci_dev(pspl2dev), offset, &val) ) {
      PERR("pci_read_config_dword(struct pci_dev *=0x%p, where=%u (0x%x), &val) failed\n",
              spl2_dev_pci_dev(pspl2dev),
              offset,
              offset);
      return -1;
   }

   return val;

#else

   ASSERT(spl2_dev_kvp_afu_csr(pspl2dev));
   if ( spl2_dev_kvp_afu_csr(pspl2dev) ) {
      char       volatile *p  = ((char volatile *)spl2_dev_kvp_afu_csr(pspl2dev)) + offset; // offset is in bytes
       bt32bitCSR volatile u;
       bt32bitCSR volatile *up = (bt32bitCSR volatile *)p;
       PVERBOSE("Reading from address 0x%p\n", p);
       u = *up;
       PVERBOSE("Returned value is 0x%x %u\n", u, u);
       return u;
     } else {
      PERR("pdev->m_kvp_afu_csr NULL, could not read CSR 0x%X. Returning -1 in lieu.\n",
           offset);
      return            -1;                          // typical value for undefined CSR
   }

#endif // SPL2PIP_USE_PCI_CONFIG
}  // read_afu_csr

//=============================================================================
// Name: write_afu_csr32
// Description: write csr value to the AFU CSR space
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef write_afu_csr32
#undef write_afu_csr32
#endif
void
write_afu_csr32(struct spl2_device *pspl2dev, btCSROffset offset, bt32bitCSR value)
{
#if (1 == SPL2PIP_USE_PCI_CONFIG)

   if ( pci_write_config_dword(spl2_dev_pci_dev(pspl2dev), offset, value) ) {
      PERR("pci_write_config_dword(struct pci_dev *=0x%p, where=%u (0x%x), value=%u (0x%x)) failed\n",
              spl2_dev_pci_dev(pspl2dev),
              offset, offset,
              value, value);
   }

#else
   ASSERT(spl2_dev_kvp_config(pspl2dev));

   if( spl2_dev_kvp_config(pspl2dev) ) {
      btVirtAddr  p  = ((btVirtAddr)spl2_dev_kvp_afu_csr(pspl2dev)) + offset; // offset is in bytes
      bt32bitCSR *up = (bt32bitCSR *)p;
      *up            = value;
   } else {
      PERR("pdev->.m_kvp_config NULL, could not write CSR 0x%X with value 0x%X\n",
           offset, value);
   }
#endif // SPL2PIP_USE_PCI_CONFIG

   // If this is a simulated device then notify the simulator TODO - SIMULATOR DOES NOT HAVE AFU and CCI variants
   if ( spl2_dev_is_simulated(pspl2dev) ) {
      PVERBOSE("Writing simulated CSR.\n");
      spl2_dev_to_simpip(pspl2dev).write_cci_csr32(pspl2dev, offset, value);
   }
} // write_afu_csr32

//=============================================================================
// Name: write_afu_csr64
// Description: write a qword csr using our SPL2 convention of high dword
//              followed by low dword
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
#ifdef write_cci_csr64
#undef write_cci_csr64
#endif
void
write_afu_csr64(struct spl2_device *pspl2dev, btCSROffset offset, bt64bitCSR value)
{
   // write high
   write_afu_csr32(pspl2dev, offset+4, (bt32bitCSR)(value>>32));
   // write low
   write_afu_csr32(pspl2dev, offset, (bt32bitCSR)value);
} // write_afu_csr64

