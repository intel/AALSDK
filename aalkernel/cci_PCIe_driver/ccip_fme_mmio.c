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
/// @file ccip_def.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_fme_mmio.c
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

#include "ccip_defs.h"
#include "ccip_fme_mmio.h"



///============================================================================
/// Name: write_ccip_csr64
/// @brief   Writes 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @param[in]  value value  going to be write in CSR.
/// @return   void
///============================================================================
int write_ccip_csr64(btVirtAddr baseAddress, btCSROffset offset,bt64bitCSR value)
{

   ASSERT(baseAddress);
   if( baseAddress ) {
      btVirtAddr  p  = (btVirtAddr)baseAddress + offset; // offset is in bytes
      bt64bitCSR *up = (bt64bitCSR *)p;
      *up            = value;
   } else {
      //PERR("baseAddress is NULL, could not write CSR 0x%X with value 0x%X\n",offset, value);
      return false;
   }
   return true;
}

///============================================================================
/// Name: read_ccip_csr64
/// @brief   read 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @return    64 bit CSR value
///============================================================================
bt64bitCSR read_ccip_csr64(btVirtAddr baseAddress ,  btCSROffset offset )
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

      return  -1;                          // typical value for undefined CSR
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
   ccip_fme_hdr(pfme_dev) =  get_fme_dev_header(pfme_dev,pkvp_fme_mmio );
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

   PTRACEOUT_INT(res);
   return NULL;
}

///============================================================================
/// Name: get_fme_dev_header
/// @brief   reads FME header from MMIO.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
///============================================================================
struct CCIP_FME_HDR* get_fme_dev_header( struct fme_device *pfme_dev,
      btVirtAddr pkvp_fme_mmio )
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
      if(fme_dfh.Type  != CCIP_DFType_private )
      {
         PERR(" invalid FME Feature Type \n");
         res = -EINVAL;
         goto ERR;
      }
      if(0 != fme_dfh.Feature_rev){
         PVERBOSE("Found feature with invalid revision %d. IGNORING!\n",fme_dfh.Feature_rev);
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
            PERR(" invalid FME Feature ID\n");
         }
         break ;
      } // end switch

      // Point at next feature header.
      pkvp_fme = pkvp_fme + fme_dfh.next_DFH_offset;

   }while(0 != fme_dfh.next_DFH_offset );  // end while


   PINFO(" get_fme_dev_featurelist EXIT \n");
   PTRACEOUT_INT(res);
   return res;

   ERR:
   PTRACEOUT_INT(res);
   return res;
}

