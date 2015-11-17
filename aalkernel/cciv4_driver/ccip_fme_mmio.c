//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
/// @file ccip_def.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_fme_mmio.c
//     CREATED: Sept 24, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the definations of the CCIP FME
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#include "aalsdk/kernel/kosal.h"

#include "ccip_def.h"
#include "ccip_fme_mmio.h"

#define MODULE_FLAGS CCIV4_DBG_MOD

/// @brief   Writes 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @param[in]  value value  going to be write in CSR.
/// @return   void
void write_ccip_csr64(btVirtAddr baseAddress, btCSROffset offset,bt64bitCSR value)
{

   if( baseAddress ) {
      btVirtAddr  p  = (btVirtAddr)baseAddress + offset; // offset is in bytes
      bt64bitCSR *up = (bt64bitCSR *)p;
      *up            = value;
   } else {
      //PERR("baseAddress is NULL, could not write CSR 0x%X with value 0x%X\n",offset, value);
   }
}

/// @brief   read 64 bit control and status registers.
///
/// @param[in]  baseAddress base CSR address.
/// @param[in]  offset offset of CSR  .
/// @return    64 bit CSR value
bt64bitCSR read_ccip_csr64(btVirtAddr baseAddress ,  btCSROffset offset )
{
   if(baseAddress)
   {
      char       volatile *p  = ((char volatile *) baseAddress) + offset; // offset is in bytes
      bt64bitCSR volatile u;
      bt64bitCSR volatile *up = (bt64bitCSR volatile *)p;
      u = *up;
      return u;
   } else {

      PERR("baseAddress is NULL, \n");

      return  -1;                          // typical value for undefined CSR
   }

}

/// @brief   read 64 bit control and status registers.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_mmio(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{
   bt32bitInt res =0;
   PTRACEIN;
   PINFO(" get_fme_mmio ENTER\n");

   if((NULL == pfme_dev) || (NULL ==  pkvp_fme_mmio))
   {
   	res = -EINVAL;
   	goto ERR;
   }

   // get  FPGA Management Engine Header
   res =  get_fme_dev_header(pfme_dev,pkvp_fme_mmio );
   if(0 != res)
   {
   	PERR("FME Header Error %d \n" ,res);
   	goto ERR;
   }

   // get  FPGA Management Engine Device feature list
   res =  get_fme_dev_featurelist(pfme_dev,pkvp_fme_mmio );
   if(0 != res )
   {
   	PERR("Port Header Error %d \n", res);
   	goto ERR;
   }

   PINFO(" get_fme_mmio EXIT\n");
   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return res;
}

/// @brief   reads FME header from MMIO.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_header(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

	struct CCIP_PORT_AFU_OFFSET  port_afu_offset;
   bt32bitInt res =0;
   bt32bitInt offset =0;
   btBool next_afu = true;
   bt32bitInt i =0;

   PTRACEIN;
   PINFO(" get_fme_header ENTER\n");

   ccip_fme_hdr(pfme_dev) =  (struct CCIP_FME_HDR*) kosal_kzmalloc(sizeof(struct CCIP_FME_HDR));

   if (  NULL == ccip_fme_hdr(pfme_dev)  ) {
       PERR("Unable to allocate system memory for pfme_dev->pfme_header object\n");
       res = -ENOMEM;
       goto ERR;
     }

   // Read FME Header
   ccip_fme_hdr(pfme_dev)->ccip_dfh.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Read FME AFU ID low
   offset = offset + byte_offset_CSR;
   ccip_fme_hdr(pfme_dev)->ccip_afu_id_l.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Read FME AFU ID high
   offset = offset + byte_offset_CSR;
   ccip_fme_hdr(pfme_dev)->ccip_afu_id_h.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Read  next afu offset
   offset = offset + byte_offset_CSR;
   ccip_fme_hdr(pfme_dev)->ccip_next_afu.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Rsvd
   offset = offset + byte_offset_CSR;

   // Read  scratchpad csr
   offset = offset + byte_offset_CSR;
   ccip_fme_hdr(pfme_dev)->ccip_fme_scratchpad.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Read  FME Capability csr
   offset = offset + byte_offset_CSR;
   ccip_fme_hdr(pfme_dev)->ccip_fme_capability.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // Read list of ports


   while (next_afu)
   {
   	offset = offset + byte_offset_CSR;
		port_afu_offset.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

		// if port implemented
		// List of ports TBD
		if(port_afu_offset .port_imp == 0x1)
		{
			pfme_dev->m_port_afu[i] =  (struct CCIP_PORT_AFU_OFFSET*) kosal_kzmalloc(sizeof(struct CCIP_PORT_AFU_OFFSET));

			if ( NULL == pfme_dev->m_port_afu[i]) {
				 PERR("Unable to allocate system memory for port next afu offset object\n");
				 res = -ENOMEM;
				 return res;
			  }
			pfme_dev->m_port_afu[i]->csr = port_afu_offset.csr;
			i++;

		}
		else
		{
			next_afu = false;
		}
   }

   PINFO(" get_fme_header EXIT \n");
   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);

   return res;
}


/// @brief   reads FME header from MMIO.
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_featurelist(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

   bt32bitInt res =0;
   bt32bitInt offset =0;
   struct CCIP_DFH fme_dfh;
   btVirtAddr pkvp_fme;
   btBool ValidFeature = true;

   PTRACEIN;
   PINFO(" get_fme_dev_featurelist ENTER \n");

   // if Device feature list is 0  , NO FME  features list
   if( 0 == ccip_fme_hdr(pfme_dev)->ccip_dfh.next_DFH_offset )
   {
      PERR("NO FME features are available \n");
      res = -EINVAL;
      goto ERR;
   }

   pkvp_fme = pkvp_fme_mmio + ccip_fme_hdr(pfme_dev)->ccip_dfh.next_DFH_offset;
   fme_dfh.csr = read_ccip_csr64(pkvp_fme,offset);


   while (ValidFeature)
   {

   	// check for Device type
   	// Type == CCIP_DFType_private
   	if(fme_dfh.Type  != CCIP_DFType_private )
   	{
   		PERR(" invalid FME Feature Type \n");
   		res = -EINVAL;
   		goto ERR;
   	}

   	// Device feature revision
      switch (fme_dfh.Feature_rev)
        {

           case CCIP_DFL_rev0 :
           {

         	    // Feature ID
                switch(fme_dfh.Feature_ID)
                {

                   case CCIP_FME_TMP_DFLID:
                   {
                     res = get_fme_dev_tmp_rev0(pfme_dev, pkvp_fme );
                   }
                   break;

                   case CCIP_FME_PM_DFLID:
                   {
                      res = get_fme_dev_pm_rev0(pfme_dev, pkvp_fme );
                   }
                   break;

                   case CCIP_FME_GP_DFLID:
                   {
                      res = get_fme_dev_fpmon_rev0(pfme_dev, pkvp_fme );
                   }
                   break;

                   case CCIP_FME_GE_DFLID:
                   {
                      res = get_fme_dev_gerr_rev0(pfme_dev, pkvp_fme );
                   }
                   break;

                   case CCIP_FME_PR_DFLID:
                   {
                      res = get_fme_dev_pr_rev0(pfme_dev, pkvp_fme );
                   }
                   break;

                   default :
                   {
                  	 PERR(" invalid FME Feature ID\n");
                  	 res = -EINVAL;
                  	 goto ERR;
                   }
                      break ;
                } // end switch


           }
           break ;

           case CCIP_DFL_rev1:
           {

           }
           break ;

           default :
           {
          	 PERR(" invalid FME Feature Revision ID \n");
          	 res = -EINVAL;
          	 goto ERR;
           }
           break ;
        }; // end switch

      // if next DFH is not 0 , enumerate next feature
      if( 0 != fme_dfh.next_DFH_offset)
      {
         pkvp_fme = pkvp_fme + fme_dfh.next_DFH_offset;
         fme_dfh.csr = read_ccip_csr64(pkvp_fme,0);

      }
      else
      {  // End of feature List  exit from loop
         ValidFeature = false;
      }


   } // end while


   PINFO(" get_fme_dev_featurelist EXIT \n");
   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return res;
}

/// @brief   reads FME Temperature Management CSR
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_tmp_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

	bt32bitInt res =0;
	bt32bitInt offset =0;

	PINFO(" get_fme_dev_tmp_rev0 ENTER \n");

	PTRACEIN;
	ccip_fme_tmp(pfme_dev) =  (struct CCIP_FME_TMP_DFL*) kosal_kzmalloc(sizeof(struct CCIP_FME_TMP_DFL));

	if ( NULL == ccip_fme_tmp(pfme_dev)) {
		 PERR("Unable to allocate system memory for FME Power bject\n");
		 res = -ENOMEM;
		 goto ERR;
	  }

	// read Temperature Management  header
	ccip_fme_tmp(pfme_dev)->ccip_fme_tmp_dflhdr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read Temperature Management  threshold csr
	offset = offset + byte_offset_CSR;
	ccip_fme_tmp(pfme_dev)->ccip_tmp_threshold.csr= read_ccip_csr64(pkvp_fme_mmio,offset);

	// read Temperature Sensor Read values  csr
	offset = offset + byte_offset_CSR;
	ccip_fme_tmp(pfme_dev)->ccip_tmp_rdssensor_fm1.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read Temperature Sensor Read values  csr
	offset = offset + byte_offset_CSR;
	ccip_fme_tmp(pfme_dev)->ccip_tmp_rdssensor_fm2.csr = read_ccip_csr64(pkvp_fme_mmio,offset);


	PINFO(" get_fme_dev_tmp_rev0 EXIT \n");
	PTRACEOUT_INT(res);
	return res;
	
ERR:
	PTRACEOUT_INT(res);

   return res;
}

/// @brief   reads FME Power Management CSR
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_pm_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

	bt32bitInt res =0;
	bt32bitInt offset =0;

	PINFO(" get_fme_dev_pm_rev0 ENTER \n");

	PTRACEIN;
	ccip_fme_pm(pfme_dev) =  (struct CCIP_FME_PM_DFL*) kosal_kzmalloc(sizeof(struct CCIP_FME_PM_DFL));

	if ( NULL ==  ccip_fme_pm(pfme_dev)) {
		 PERR("Unable to allocate system memory for FME Temperature object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }

	// read  Power Management header
	ccip_fme_pm(pfme_dev)->ccip_fme_pm_dflhdr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  Power Threshold csr
	offset = offset + byte_offset_CSR;
	ccip_fme_pm(pfme_dev)->ccip_pm_threshold.csr= read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  Power  Management Read & write voltage Regulator csr
	offset = offset + byte_offset_CSR;
	ccip_fme_pm(pfme_dev)->ccip_pm_rdvr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  Power  Management Record max VR values csr
	offset = offset + byte_offset_CSR;
	ccip_fme_pm(pfme_dev)->ccip_pm_mrdvr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	PINFO(" get_fme_dev_pm_rev0 EXIT \n");
	PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);
	return res;
}

/// @brief   reads FME Global performance CSR
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_fpmon_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

   bt32bitInt res =0;
   bt32bitInt offset =0;

   PTRACEIN;

   PINFO(" get_fme_dev_fpmon_rev0 ENTER \n");

   ccip_fme_fpmon(pfme_dev) =  (struct CCIP_FME_FPMON_DFL*) kosal_kzmalloc(sizeof(struct CCIP_FME_FPMON_DFL));

   if ( NULL ==  ccip_fme_fpmon(pfme_dev)) {
       PERR("Unable to allocate system memory for FME FPMON object\n");
       res = -ENOMEM;
       goto ERR;
     }

   // read Global performance header
   ccip_fme_fpmon(pfme_dev)->ccip_fme_fpmon_dflhdr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // read cache control csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_ch_ctl.csr= read_ccip_csr64(pkvp_fme_mmio,offset);

   // read cache counter 0 csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_ch_ctr_0.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // read cache counter 1 csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_ch_ctr_1.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // read fabric control csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_fab_ctl.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // read fabric counter csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_fab_ctr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   // read afu clock csr
   offset = offset + byte_offset_CSR;
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_clk_ctrs.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

   PINFO(" get_fme_dev_fpmon_rev0 EXIT \n");
   PTRACEOUT_INT(res);
   return res;

ERR:

   PTRACEOUT_INT(res);
   return res;
}

/// @brief   reads FME Global error CSR
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_gerr_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

	bt32bitInt res =0;
	bt32bitInt offset =0;

	PTRACEIN;
	PINFO(" get_fme_dev_gerr_rev0 ENTER \n");

	ccip_fme_gerr(pfme_dev) =  (struct CCIP_FME_GERROR_DFL*) kosal_kzmalloc(sizeof(struct CCIP_FME_GERROR_DFL));

	if (  NULL ==  ccip_fme_gerr(pfme_dev)) {
		 PERR("Unable to allocate system memory for FME GERROR object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }
	// read Global error header
	ccip_fme_gerr(pfme_dev)->ccip_gerror_dflhdr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  error mask csr
	offset = offset + byte_offset_CSR;
	ccip_fme_gerr(pfme_dev)->ccip_fme_error_mask.csr= read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  error  csr
	offset = offset + byte_offset_CSR;
	ccip_fme_gerr(pfme_dev)->ccip_fme_error.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read  first error  csr
	offset = offset + byte_offset_CSR;
	ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	PINFO(" get_fme_dev_gerr_rev0 EXIT \n");
	PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);
	return res;
}

/// @brief   reads FME PR CSR
///
/// @param[in] fme_device fme device .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_pr_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio )
{

	bt32bitInt res =0;
	bt32bitInt offset =0;

	PTRACEOUT;
	PINFO(" get_fme_dev_pr_rev0 ENTER \n");

	ccip_fme_pr(pfme_dev) =  (struct CCIP_FME_PR_DFL*) kosal_kzmalloc(sizeof(struct CCIP_FME_PR_DFL));

	if (  NULL ==  ccip_fme_pr(pfme_dev)) {
		 PERR("Unable to allocate system memory for  FME PR  object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }

	// read PR header
	ccip_fme_pr(pfme_dev)->ccip_pr_dflhdr.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read PR control CSR
	offset = offset + byte_offset_CSR;
	ccip_fme_pr(pfme_dev)->ccip_fme_pr_control.csr= read_ccip_csr64(pkvp_fme_mmio,offset);

	// read PR status CSR
	offset = offset + byte_offset_CSR;
	ccip_fme_pr(pfme_dev)->ccip_fme_pr_status.csr = read_ccip_csr64(pkvp_fme_mmio,offset);

	// read PR data CSR
	offset = offset + byte_offset_CSR;
	ccip_fme_pr(pfme_dev)->ccip_fme_pr_data.csr = read_ccip_csr64(pkvp_fme_mmio,offset);


	PINFO(" get_fme_dev_pr_rev0 EXIT \n");
        PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);

	return res;
}

/// @brief   freee FME Device feature list memory
///
/// @param[in] fme_device fme device .
/// @return    void
void ccip_fme_mem_free(struct fme_device *pfme_dev )
{

	int i =0;


	PINFO(" ccip_fme_mem_free ENTER \n");

	PTRACEOUT;
	if(pfme_dev)	{

		PINFO(" ccip_fme_mem_free  pfme_dev \n");

		if(pfme_dev->m_pfme_hdr) {
			kosal_kfree(pfme_dev->m_pfme_hdr, sizeof(struct CCIP_FME_HDR));
		}

		if(pfme_dev->m_pfme_tmp) {
			kosal_kfree(pfme_dev->m_pfme_tmp, sizeof(struct CCIP_FME_TMP_DFL));
		}

		if(pfme_dev->m_pfme_pm)	{
			kosal_kfree(pfme_dev->m_pfme_pm, sizeof(struct CCIP_FME_FPMON_DFL));
		}

		if(pfme_dev->m_pfme_gerror) {
			kosal_kfree(pfme_dev->m_pfme_gerror, sizeof(struct CCIP_FME_GERROR_DFL));
		}

		if(pfme_dev->m_pfme_fpmon)	{
			kosal_kfree(pfme_dev->m_pfme_fpmon, sizeof(struct CCIP_FME_GERROR_DFL));
		}

		if(pfme_dev->m_pfme_pr)	{
			kosal_kfree(pfme_dev->m_pfme_pr, sizeof(struct CCIP_FME_PR_DFL));
		}

		// TBD list
		for( i=0;i<5;i++) {

			if(pfme_dev->m_port_afu[i]){

				kosal_kfree(pfme_dev->m_port_afu[i], sizeof(struct CCIP_PORT_AFU_OFFSET));
			}
		}

		// iounmap  BAR0      TBD
	}

	PINFO(" ccip_fme_mem_free EXIT \n");

	PTRACEOUT;
}




















































