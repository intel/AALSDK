
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
//        FILE: ccip_port_mmio.c
//     CREATED: Sept 24, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the definations of the CCIP port
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#include "aalsdk/kernel/kosal.h"
#include "ccip_def.h"
#include "ccip_port_mmio.h"

#define MODULE_FLAGS CCIV4_DBG_MOD

#define  CCIP_PORT_OUTREQ_TIMEOUT  1000
#define  CCIP_PORT_OUTREQ_COMPLETE 0x1

/// @brief   reads PORT mmio region
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_mmio(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio )
{
   bt32bitInt res =0;

   PTRACEIN;
   PINFO(" get_port_mmio ENTER\n");

   if((NULL == pport_dev) || (NULL ==  pkvp_port_mmio)) {
   	res = -EINVAL;
   	goto ERR;
   }

   // get port header
   res =  get_port_header(pport_dev,pkvp_port_mmio );
   if(res !=0) {
   	PERR("Port Header Error %d\n", res);
   	goto ERR;
   }

   // get port feature list
   res =  get_port_featurelist(pport_dev,pkvp_port_mmio );
   if(res !=0) {
   	PERR("Port device feature list Error %d \n",res);
   	goto ERR;
   }

   PINFO(" get_port_mmio EXIT \n");
ERR:
   PTRACEOUT_INT(res);
   return res;
}

/// @brief   reads PORT Header
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_header(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio )
{
	bt32bitInt res =0;
	bt32bitInt offset =0;

   PTRACEIN;
	PINFO(" get_port_header ENTER\n");

	ccip_port_hdr(pport_dev) =  (struct CCIP_PORT_HDR*) kosal_kzmalloc(sizeof(struct CCIP_PORT_HDR));

	if ( NULL == ccip_port_hdr(pport_dev)) {
		 PERR("Unable to allocate system memory for PORT Header object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }
	// Read port Header
	ccip_port_hdr(pport_dev)->ccip_port_dfh.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port AFU ID low
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_afuidl.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port AFU ID hight
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_afuidh.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	 // Read  next afu offset
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_next_afu.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Rsvd
	offset = offset + byte_offset_CSR;

	 // Read  scratchpad csr
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_scratchpad.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read  PORT Capability csr
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_capability.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read  PORT control csr
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_control.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read  PORT status csr
	offset = offset + byte_offset_CSR;
	ccip_port_hdr(pport_dev)->ccip_port_status.csr = read_ccip_csr64(pkvp_port_mmio,offset);


	PINFO(" get_port_header EXIT \n");
	PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);
	return res;
}


/// @brief   reads PORT feature list
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_featurelist(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio )
{

   bt32bitInt res =0;
   bt32bitInt offset =0;
   struct CCIP_DFH port_dfh;
   btVirtAddr pkvp_port;
   btBool validFeature = true;

   PTRACEIN;
   PINFO(" get_port_featurelist ENTER\n");

   if( ccip_port_hdr(pport_dev)->ccip_port_dfh.next_DFH_offset ==0)   {
      PERR("NO PORT features are available \n");
      res = -EINVAL;
      goto ERR;
   }
   // read PORT Device feature Header
   pkvp_port = pkvp_port_mmio + ccip_port_hdr(pport_dev)->ccip_port_dfh.next_DFH_offset;
   port_dfh.csr = read_ccip_csr64(pkvp_port,offset);

   while (validFeature)
   {

		// check for Device type
		// Type == CCIP_DFType_private
		if(port_dfh.Type  != CCIP_DFType_private ) {
			PERR(" invalid PORT Feature Type \n");
			res = -EINVAL;
			goto ERR;

		}
		// Device feature revision
      switch (port_dfh.Feature_rev)
        {

           case CCIP_DFL_rev0 :
           {
         	  // Device feature ID
                switch(port_dfh.Feature_ID)
                {

                   case CCIP_PORT_ERROR_DFLID:
                   {
                     res = get_port_err_rev0(pport_dev, pkvp_port );
                   }
                   break;

                   case CCIP_PORT_USMG_DFLID:
                   {
                      res = get_port_umsg_rev0(pport_dev, pkvp_port );
                   }
                   break;

                   case CCIP_PORT_PR_DFLID:
                   {
                      res = get_port_pr_rev0(pport_dev, pkvp_port );
                   }
                   break;

                   case CCIP_PORT_STP_DFLID:
                   {
                      res = get_port_stap_rev0(pport_dev, pkvp_port );
                   }
                   break;


                   default :
                   {
                  	 PERR(" invalid PORT Feature ID\n");
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
           break ;
        }; // end switch

      if(port_dfh.next_DFH_offset !=0) {

         pkvp_port = pkvp_port + port_dfh.next_DFH_offset;
         port_dfh.csr = read_ccip_csr64(pkvp_port,0);

      }
      else
         validFeature = false;



   } // end while


   PINFO(" get_port_featurelist EXIT \n");
   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return res;
}


/// @brief   reads PORT error  CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_err_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio)
{

   bt32bitInt res =0;
   bt32bitInt offset =0;

   PTRACEIN;
   PINFO(" get_port_err_rev0 ENTER\n");

   ccip_port_err(pport_dev) =  (struct CCIP_PORT_ERR_DFL*) kosal_kzmalloc(sizeof(struct CCIP_PORT_ERR_DFL));

   if ( NULL == ccip_port_err(pport_dev)) {
       PERR("Unable to allocate system memory for PORT ERROR object\n");
       res = -ENOMEM;
       goto ERR;
     }
   // Read port error header
   ccip_port_err(pport_dev)->ccip_port_err_dflhdr.csr = read_ccip_csr64(pkvp_port_mmio,offset);

   // Read port error mask csr
   offset = offset + byte_offset_CSR;
   ccip_port_err(pport_dev)->ccip_port_error_mask.csr= read_ccip_csr64(pkvp_port_mmio,offset);

   // Read port error  csr
   offset = offset + byte_offset_CSR;
   ccip_port_err(pport_dev)->ccip_port_error.csr = read_ccip_csr64(pkvp_port_mmio,offset);

   // Read port first error  csr
   offset = offset + byte_offset_CSR;
   ccip_port_err(pport_dev)->ccip_port_first_error.csr = read_ccip_csr64(pkvp_port_mmio,offset);

   // Read port malformed request  csr
   offset = offset + byte_offset_CSR;
   ccip_port_err(pport_dev)->ccip_port_malformed_req_0.csr = read_ccip_csr64(pkvp_port_mmio,offset);

   // Read port malformed request  csr
   offset = offset + byte_offset_CSR;
   ccip_port_err(pport_dev)->ccip_port_malformed_req_1.csr = read_ccip_csr64(pkvp_port_mmio,offset);

   PINFO(" get_port_err_rev0 EXIT\n");
   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return res;
}

/// @brief   reads PORT UMSG CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_umsg_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio)
{

	bt32bitInt res =0;
	bt32bitInt offset =0;

	PTRACEIN;
	PINFO(" get_port_umsg_rev0 ENTER\n");

	ccip_port_umsg(pport_dev) =  (struct CCIP_PORT_UMSG_DFL*) kosal_kzmalloc(sizeof(struct CCIP_PORT_UMSG_DFL));

	if (  NULL == ccip_port_umsg(pport_dev)) {
		 PERR("Unable to allocate system memory for PORT USMG object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }
	// Read port USMG header
	ccip_port_umsg(pport_dev)->ccip_port_umsg_dflhdr.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port USMG  capability csr
	offset = offset + byte_offset_CSR;
	ccip_port_umsg(pport_dev)->ccip_umsg_capability.csr= read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port USMG  base address csr
	offset = offset + byte_offset_CSR;
	ccip_port_umsg(pport_dev)->ccip_umsg_base_address.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port USMG  mode csr
	offset = offset + byte_offset_CSR;
	ccip_port_umsg(pport_dev)->ccip_umsg_mode.csr = read_ccip_csr64(pkvp_port_mmio,offset);


	PINFO(" get_port_umsg_rev0 EXIT\n");
	PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);
	return res;
}

/// @brief   reads PORT PR CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_pr_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio)
{

	bt32bitInt res =0;
	bt32bitInt offset =0;
	PTRACEIN;
   PINFO(" get_port_pr_rev0 ENTER\n");

	ccip_port_pr(pport_dev) =  (struct CCIP_PORT_PR_DFL*) kosal_kzmalloc(sizeof(struct CCIP_PORT_PR_DFL));

	if (  NULL == ccip_port_pr(pport_dev)) {
		 PERR("Unable to allocate system memory for PORT PR object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }
	// Read port PR header
	ccip_port_pr(pport_dev)->ccip_port_pr_dflhdr.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port PR control csr
	offset = offset + byte_offset_CSR;
	ccip_port_pr(pport_dev)->ccip_port_pr_control.csr= read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port PR status csr
	offset = offset + byte_offset_CSR;
	ccip_port_pr(pport_dev)->ccip_port_pr_status.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port PR data csr
	offset = offset + byte_offset_CSR;
	ccip_port_pr(pport_dev)->ccip_port_pr_data.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port PR power budget csr
	offset = offset + byte_offset_CSR;
	ccip_port_pr(pport_dev)->ccip_port_pr_pbudget.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port PR user clock frequency csr
	offset = offset + byte_offset_CSR;
	ccip_port_pr(pport_dev)->ccip_usr_clk_freq.csr = read_ccip_csr64(pkvp_port_mmio,offset);


	PINFO(" get_port_pr_rev0 EXIT\n");
	PTRACEOUT_INT(res);
	return res;
ERR:
	PTRACEOUT_INT(res);
	return res;
}

/// @brief   reads PORT signaltap CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_stap_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio)
{

	bt32bitInt res =0;
	bt32bitInt offset =0;
	PTRACEIN;
	PINFO(" get_port_stap_rev0 ENTER\n");

	ccip_port_stap(pport_dev) =  (struct CCIP_PORT_STAP_DFL*) kosal_kzmalloc(sizeof(struct CCIP_PORT_STAP_DFL));

	if (  NULL == ccip_port_stap(pport_dev)) {
		 PERR("Unable to allocate system memory for PORT Signaltap object\n");
		 res = -ENOMEM;
		 goto ERR;
	  }
	// Read port signaltap header
	ccip_port_stap(pport_dev)->ccip_port_stap_dflhdr.csr = read_ccip_csr64(pkvp_port_mmio,offset);

	// Read port signaltap csr
	offset = offset + byte_offset_CSR;
	ccip_port_stap(pport_dev)->ccip_port_stap.csr= read_ccip_csr64(pkvp_port_mmio,offset);

	PINFO(" get_port_stap_rev0 EXIT \n");
	PTRACEOUT_INT(res);
	return res;

ERR:
	PTRACEOUT_INT(res);
	return res;
}
/// @brief   freee Port feature list memory
///
/// @param[in] pport_dev port device pointer .
/// @return    void
void ccip_port_mem_free(struct port_device *pport_dev )
{

	PTRACEIN;
	PINFO(" ccip_port_mem_free ENTER\n");

	if(pport_dev)	{

		PINFO(" ccip_port_mem_free  pport_dev \n");

		if(pport_dev->m_pport_hdr) {

			kosal_kfree(pport_dev->m_pport_hdr, sizeof(struct CCIP_PORT_HDR));
		}

		if(pport_dev->m_pport_err) {

			kosal_kfree(pport_dev->m_pport_err, sizeof(struct CCIP_PORT_ERR_DFL));
		}

		if(pport_dev->m_pport_umsg) {

			kosal_kfree(pport_dev->m_pport_umsg, sizeof(struct CCIP_PORT_UMSG_DFL));
		}

		if(pport_dev->m_pport_pr)	{

			kosal_kfree(pport_dev->m_pport_pr, sizeof(struct CCIP_PORT_PR_DFL));
		}

		if(pport_dev->m_pport_stap) 	{

			kosal_kfree(pport_dev->m_pport_stap, sizeof(struct CCIP_PORT_STAP_DFL));
		}

		// iounmap  BAR1 TBD
	}
	PINFO(" ccip_port_mem_free EXIT\n");

	PTRACEOUT;
}


/// @brief   Port Re Enable
///
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_re_enable(btVirtAddr pkvp_port_mmio )
{

   bt32bitInt res =0;
   struct CCIP_PORT_CONTROL port_control;

   PINFO(" Enter \n");

   if(NULL == pkvp_port_mmio)  {

      res = -EINVAL;
      return  res;
   }

   port_control.csr = read_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL);
   port_control.port_sftreset_control = 0x0;

   // Re enable port /afu
   write_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL,port_control.csr);

   PINFO(" Exit \n");

   return res;
}

/// @brief   Reset port
///
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_port_reset(btVirtAddr pkvp_port_mmio )
{
   bt32bitInt res =0;
   struct CCIP_PORT_CONTROL port_control;

   PINFO(" Enter \n");

   if(NULL == pkvp_port_mmio)  {

      res = -EINVAL;
      return  res;
   }

   // afu/port Quiesce reset
   res = afu_quiesce_reset(pkvp_port_mmio );
   if(0 != res) {
      goto ERR;
   }

   // afu/port enable
   res = afu_re_enable(pkvp_port_mmio );
   if(0 != res) {
      goto ERR;
   }

   PINFO(" Exit \n");

ERR:
   PINFO(" Error Exit \n");
   return res;
}



/// @brief   Port Quiesce Reset
///
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_quiesce_reset(btVirtAddr pkvp_port_mmio )
{
   bt32bitInt res =0;
   btTime delay =10;
   btTime totaldelay =0;
   struct CCIP_PORT_CONTROL port_control;

   PINFO(" Enter \n");

   if(NULL == pkvp_port_mmio)  {

   res = -EINVAL;
   return  res;
   }

   // Reset Port
   port_control.csr = read_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL);
   port_control.port_sftreset_control = 0x1;
   write_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL,port_control.csr);

   // read Port control CSR
   port_control.csr = read_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL);

   // All CCI-P request at port complete
   // Set to 1 When all outstanding requests initiated bu this port have been drained
   while (CCIP_PORT_OUTREQ_COMPLETE != port_control.ccip_outstaning_request)
   {
      // Sleep
      kosal_udelay(delay);

      // total dealy
      totaldelay = totaldelay + delay;

      // if total delay is more then 1 millisecond , return erroor
      if(totaldelay > CCIP_PORT_OUTREQ_TIMEOUT)   {
         res = -ETIME;
         break ;
      }

      port_control.csr = read_ccip_csr64(pkvp_port_mmio,byte_offset_PORT_CONTROL);

   }

   PINFO(" Exit \n");
   return  res;
}



