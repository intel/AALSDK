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
/// @file ccip_fme.c
/// @brief  Implements the FPGA Management Engine (FME).
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_fme.c
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel <ananda.ravuri@intel.com>
//              Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the definations of the CCIP FME
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/AALTransactionID_s.h"
#include "aalsdk/kernel/aalbus-ipip.h"

#include "aalsdk/kernel/ccipdriver.h"
#include "ccipdrv-events.h"
#include "ccip_perfmon.h"
#include "aalsdk/kernel/ccip_defs.h"
#include "ccip_fme.h"
#include "cci_pcie_driver_PIPsession.h"



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          AAL SUPPORT FUNCTIONS           ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int CommandHandler( struct aaldev_ownerSession *,
                           struct aal_pipmessage*);
extern int cci_mmap( struct aaldev_ownerSession *pownerSess,
                     struct aal_wsid *wsidp,
                     btAny os_specific);

//=============================================================================
// cci_FMEpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip cci_FMEpip = {
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

   // UI Driver message
   struct aalui_CCIdrvMessage *pmsg = (struct aalui_CCIdrvMessage *) Message->m_message;

   // Save original response buffer size in case we return something
   btWSSize         respBufSize     = Message->m_respbufSize;


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
         }
         PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
         Message->m_errcode = uid_errnumOK;

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

      } break;

      AFU_COMMAND_CASE(ccipdrv_getPerfMonitor) {

         bt32bitInt res;
         struct CCIP_PERF_COUNTERS perfcounter;
         PVERBOSE("ccipdrv_getPerfMonitor \n");

         res= get_perfmon_counters(cci_aaldev_pfme(pdev) ,&perfcounter);
         if(0 != res )
         {
            Message->m_errcode = uid_errnumPermission;
                   break;
         }

        if(respBufSize >= sizeof(struct CCIP_PERF_COUNTERS)){
           PVERBOSE("ccipdrv_getPerfMonitor  Valid Buffer\n");
           *((struct CCIP_PERF_COUNTERS*)Message->m_response) = perfcounter;
           Message->m_respbufSize = sizeof(struct CCIP_PERF_COUNTERS);
        }

         Message->m_errcode = uid_errnumOK;

      } break; // case ccipdrv_getPerfMonitor

      AFU_COMMAND_CASE(ccipdrv_getFMEError) {

          bt32bitInt res               = 0;
          struct CCIP_ERROR fme_error  = {0};

          PVERBOSE("ccipdrv_getFmeError \n");

          res= get_fme_error(cci_aaldev_pfme(pdev) ,&fme_error);
          if(0 != res ) {
             Message->m_errcode = uid_errnumBadParameter;
             break;
          }

         if(respBufSize >= sizeof(struct CCIP_ERROR)){
            PVERBOSE("ccipdrv_getFmeErrors  Valid Buffer\n");
            *((struct CCIP_ERROR*)Message->m_response) = fme_error;
            Message->m_respbufSize = sizeof(struct CCIP_ERROR);
         }

          Message->m_errcode = uid_errnumOK;

       } break; // case ccipdrv_getFMEError

       AFU_COMMAND_CASE(ccipdrv_SetFMEErrorMask) {

          struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
          PVERBOSE("ccipdrv_fmeSetErrorMask \n");

          if(NULL == preq) {
             Message->m_errcode = uid_errnumBadParameter;
             break;
          }

          PVERBOSE("Port Error mask CSR:%llx \n",ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error_mask.csr);
          PVERBOSE("preq->ahmreq.u.error_csr.error:%llx \n",preq->ahmreq.u.error_csr.error);

          ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error_mask.csr = preq->ahmreq.u.error_csr.error;

          //Success Event
          Message->m_errcode = uid_errnumOK;

        } break; // case ccipdrv_SetFMEErrorMask


        AFU_COMMAND_CASE(ccipdrv_ClearFMEError) {

           struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
           PVERBOSE("ccipdrv_fmeClearError \n");

           if(NULL == preq) {
              Message->m_errcode = uid_errnumBadParameter;
              break;
           }

           PVERBOSE("Port Error CSR:%llx \n",ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error.csr);
           PVERBOSE("Port First Error CSR:%llx \n",ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_first_error.csr);
           PVERBOSE("preq->ahmreq.u.error_csr.error:%llx \n",preq->ahmreq.u.error_csr.error);

           ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error.csr = preq->ahmreq.u.error_csr.error;
           ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_first_error.csr = preq->ahmreq.u.error_csr.error;

           //Success Event
           Message->m_errcode = uid_errnumOK;

         } break; // case ccipdrv_ClearFMEError

        AFU_COMMAND_CASE(ccipdrv_ClearAllFMEErrors) {

           PVERBOSE("ccipdrv_ClearAllFMEError \n");

           PVERBOSE("Port Error CSR:%llx \n",ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error.csr);
           PVERBOSE("Port First Error CSR:%llx \n",ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_first_error.csr);

           ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_error.csr = 0x0;
           ccip_fme_gerr(cci_aaldev_pfme(pdev))->ccip_fme_first_error.csr = 0x0;

           //Success Event
           Message->m_errcode = uid_errnumOK;

       } break; // case ccipdrv_ClearAllFMEErrors

       AFU_COMMAND_CASE(ccipdrv_getPower) {

          bt32bitInt res                        = 0;
          struct CCIP_THERMAL_PWR thermal_power = {0};
          PVERBOSE("ccipdrv_getPower \n");

          res= get_fme_power(cci_aaldev_pfme(pdev) ,&thermal_power);
          if(0 != res ) {
              Message->m_errcode = uid_errnumBadParameter;
              break;
          }

          if(respBufSize >= sizeof(struct CCIP_THERMAL_PWR)){
             PVERBOSE("ccipdrv_getPower  Valid Buffer\n");
             *((struct CCIP_THERMAL_PWR*)Message->m_response) = thermal_power;
             Message->m_respbufSize = sizeof(struct CCIP_THERMAL_PWR);
          }

          //Success Event
          Message->m_errcode = uid_errnumOK;

        } break; // case ccipdrv_getPower

        AFU_COMMAND_CASE(ccipdrv_gertThermal) {

          bt32bitInt res                        = 0;
          struct CCIP_THERMAL_PWR thermal_power = {0};
          PVERBOSE("ccipdrv_gertThermal \n");

          res= get_fme_thermal(cci_aaldev_pfme(pdev) ,&thermal_power);
          if(0 != res ) {
             Message->m_errcode = uid_errnumBadParameter;
             break;
          }

          if(respBufSize >= sizeof(struct CCIP_THERMAL_PWR)){
            PVERBOSE("ccipdrv_gertThrermal  Valid Buffer\n");
            *((struct CCIP_THERMAL_PWR*)Message->m_response) = thermal_power;
            Message->m_respbufSize = sizeof(struct CCIP_THERMAL_PWR);
          }

          //Success Event
          Message->m_errcode = uid_errnumOK;

         } break; // case ccipdrv_gertThrermal

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////           GENERIC FME FUNCTIONS           ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///============================================================================
/// Name: write_ccip_csr64
/// @brief   Writes 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @param[in]  value value  going to be write in CSR.
/// @return   void
///============================================================================
int write_ccip_csr64( btVirtAddr baseAddress, 
                      btUnsigned64bitInt offset, 
                      bt64bitCSR value )
{

   ASSERT(baseAddress);
   if( baseAddress ) {
      btVirtAddr  p  = (btVirtAddr)baseAddress + offset; // offset is in bytes
      bt64bitCSR *up = (bt64bitCSR *)p;
      *up            = value;
   } else {
      //PERR("baseAddress is NULL, could not write CSR 0x%X with value 0x%X\n",offset, value);
      return 0;
   }
   return 1;
}

///============================================================================
/// Name: read_ccip_csr64
/// @brief   read 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @return    64 bit CSR value
///============================================================================
bt64bitCSR read_ccip_csr64( btVirtAddr baseAddress, btUnsigned64bitInt offset )
{
   if(baseAddress)
   {
      char       volatile *p  = ((char volatile *) baseAddress) + offset; // offset is in bytes
      bt64bitCSR volatile  u;
      bt64bitCSR volatile *up = (bt64bitCSR volatile *)p;
      u = *up;
      return u;
   } else {

      PERR("baseAddress is NULL, \n");

      return  ( bt64bitCSR )- 1;          // typical value for undefined CSR
   }

}

///============================================================================
/// Name:get_fme_mmio_dev
/// @brief Construct the FME MMIO object.
///
/// @param[in] fme_device fme device object .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
///============================================================================
struct fme_device * get_fme_mmio_dev(btVirtAddr pkvp_fme_mmio )
{
   bt32bitInt            res        = 0;
   struct fme_device    *pfme_dev   = NULL;
   PTRACEIN;


   // Validate inputs parameters
   ASSERT(pkvp_fme_mmio);
   if(NULL ==  pkvp_fme_mmio){
      return NULL;
   }

   // Construct the new object
   pfme_dev =(struct fme_device*) kosal_kmalloc(sizeof(struct fme_device));
   ASSERT(pfme_dev);
   if(NULL == pfme_dev){
      return NULL;
   }

   // Get the FPGA Management Engine Header
   ccip_fme_hdr(pfme_dev) =  get_fme_dev_header(pkvp_fme_mmio );
   if(NULL == ccip_fme_hdr(pfme_dev)){
      PERR("Error reading FME header %d \n" ,res);
      goto ERR;
   }

   // Get the FPGA Management Engine Device feature list
   res =  get_fme_dev_featurelist(pfme_dev,pkvp_fme_mmio );
   if(0 != res ){
      PERR("Error reading FME feature list %d \n", res);
      goto ERR;
   }

   PTRACEOUT_INT(res);
   return pfme_dev;

   ERR:
      kosal_kfree(pfme_dev,sizeof(struct fme_device));
      PTRACEOUT_INT(res);
      return NULL;
}

///============================================================================
/// ccip_destroy_fme_mmio_dev
/// @brief Destroy the FME MMIO object.
///
/// @param[in] fme_device fme device object .
/// @return    None
///============================================================================
void ccip_destroy_fme_mmio_dev(struct fme_device *pfme_dev)
{
   PVERBOSE("Destroying fme_device");
   kosal_kfree(pfme_dev,sizeof(struct fme_device));
}

///============================================================================
/// Name: get_fme_dev_header
/// @brief   reads FME header from MMIO.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
///============================================================================
struct CCIP_FME_HDR* get_fme_dev_header(btVirtAddr pkvp_fme_mmio )
{

   // FME registers are mapped into the PF MMIO address space. FME DFH will
   //   be the first register at address offset 0
   return (struct CCIP_FME_HDR* )pkvp_fme_mmio;
}

///============================================================================
/// Name: get_fme_dev_featurelist
/// @brief Enumerates the FME features.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_featurelist( struct fme_device *pfme_dev,
                                    btVirtAddr pkvp_fme_mmio )
{

   bt32bitInt res       = 0;
   btVirtAddr pkvp_fme  = NULL;
   struct CCIP_DFH fme_dfh;

   PTRACEIN;

   PDEBUG(" get_fme_dev_featurelist ENTER \n");

   // if Device feature list is 0  , NO FME  features list
   if( 0 == ccip_fme_hdr(pfme_dev)->dfh.next_DFH_offset ){
      PERR("NO FME features are available \n");
      res = -EINVAL;
      goto ERR;
   }

   // Start at the first Device Feature Header
   pkvp_fme = pkvp_fme_mmio + ccip_fme_hdr(pfme_dev)->dfh.next_DFH_offset;

   // Walk the Feature list storing the pointer to each feature header
   do {

      // Peek at the Header
      fme_dfh.csr = read_ccip_csr64(pkvp_fme,0);

      // check for Device type
      // Type == CCIP_DFType_private
      if(fme_dfh.Type  != CCIP_DFType_private ){
         PERR(" invalid FME Feature Type \n");
         res = -EINVAL;
         goto ERR;
      }
      if(0 != fme_dfh.Feature_rev){
         PVERBOSE("Found FME feature with invalid revision %d. IGNORING!\n",fme_dfh.Feature_rev);
         continue;
      }

      // Feature ID
      switch(fme_dfh.Feature_ID)
      {
         // Thermal Management
         case CCIP_FME_DFLID_THERM:
         {
            ccip_fme_therm(pfme_dev) = (struct CCIP_FME_DFL_THERM *)pkvp_fme;
         }
         break;

         // Power Management
         case CCIP_FME_DFLID_POWER:
         {
            ccip_fme_power(pfme_dev) = (struct CCIP_FME_DFL_PM *)pkvp_fme;
         }
         break;

         // Global Performance
         case CCIP_FME_DFLID_GPERF:
         {
            ccip_fme_perf(pfme_dev) = (struct CCIP_FME_DFL_FPMON *)pkvp_fme;
         }
         break;

         // Global Error
         case CCIP_FME_DFLID_GERR:
         {
            ccip_fme_gerr(pfme_dev) = (struct CCIP_FME_DFL_GERROR *)pkvp_fme;
         }
         break;

         // FME Partial Reconfiguration Management
         case CCIP_FME_DFLID_PR:
         {
            ccip_fme_pr(pfme_dev) = (struct CCIP_FME_DFL_PR *)pkvp_fme;
         }
         break;

         default :
         {
            PWARN(" Invalid FME Feature ID %d  0x%X \n",fme_dfh.Feature_ID,fme_dfh.Feature_ID);
         }
         break ;
      } // end switch

      // Point at next feature header.
      pkvp_fme = pkvp_fme + fme_dfh.next_DFH_offset;

   }while(0 != fme_dfh.next_DFH_offset );  // end while

   PINFO(" get_fme_dev_featurelist EXIT \n");
 ERR:
   PTRACEOUT_INT(res);
   return res;
}


///============================================================================
/// Name:    get_fme_error
/// @brief   get fpga global error
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pfme_error ccip error structure  pointer
/// @return    error code
///============================================================================
bt32bitInt get_fme_error(struct fme_device  *pfme_dev,
                         struct CCIP_ERROR  *pfme_error)
{
   int res = 0;

   PTRACEIN;

   if( (NULL == pfme_dev) && (NULL == pfme_error))  {
      PERR("Invalid input pointers \n");
      res =-EINVAL;
      goto ERR;
   }

   memset(pfme_error,0,sizeof(struct CCIP_ERROR));

   pfme_error->error_mask    = ccip_fme_gerr(pfme_dev)->ccip_fme_error_mask.csr;
   pfme_error->error         = ccip_fme_gerr(pfme_dev)->ccip_fme_error.csr;
   pfme_error->first_error   = ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr;

   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return  res;
}

///============================================================================
/// Name:    get_fme_power
/// @brief   power consumed values
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pfme_power ccip power structure  pointer
/// @return    error code
///============================================================================
bt32bitInt get_fme_power(struct fme_device       *pfme_dev,
                         struct CCIP_THERMAL_PWR *pthermal_power)
{
   int res = 0;

   PTRACEIN;

   if( (NULL == pfme_dev) && (NULL == pthermal_power))  {
      PERR("Invalid input pointers \n");
      res =-EINVAL;
      goto ERR;
   }

   memset(pthermal_power,0,sizeof(struct CCIP_THERMAL_PWR));

   pthermal_power->pwr_status    = 0;

   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return  res;
}

///============================================================================
/// Name:    get_fme_thermal
/// @brief   get  fpga  thermal values
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf ccip thermal structure  pointer
/// @return    error code
///============================================================================
bt32bitInt get_fme_thermal(struct fme_device        *pfme_dev,
                           struct CCIP_THERMAL_PWR  *pthermal_power)
{
   int res = 0;

   PTRACEIN;

   if( (NULL == pfme_dev) && (NULL == pthermal_power))  {
      PERR("Invalid input pointers \n");
      res =-EINVAL;
      goto ERR;
   }

   memset(pthermal_power,0,sizeof(struct CCIP_THERMAL_PWR));

   pthermal_power->tmp_threshold    = ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.csr;
   pthermal_power->tmp_rdsensor1    = ccip_fme_therm(pfme_dev)->ccip_tmp_rdssensor_fm1.csr;
   pthermal_power->tmp_rdsensor2    = ccip_fme_therm(pfme_dev)->ccip_tmp_rdssensor_fm2.csr;

   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return  res;
}
