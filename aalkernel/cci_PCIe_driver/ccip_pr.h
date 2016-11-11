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
/// @file ccip_port.h
/// @brief  Definitions for CCI Port.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_port.h
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel <ananda.ravuri@intel.com>
//              Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the definations of the CCIP Port
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __AALKERNEL_CCIP_PR_H_
#define __AALKERNEL_CCIP_PR_H_

#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/ccip_defs.h>
#include "cci_pcie_driver_internal.h"

#include "aalsdk/kernel/ccipdriver.h"

BEGIN_NAMESPACE(AAL)


// Maximum  PR timeout  15 seconds
#define  PR_OUTSTADREQ_TIMEOUT   15000000
#define  PR_OUTSTADREQ_DELAY     1
#define  PR_COUNTER_MAX_TRY      0xFFFFFF

// PR revoke Maximum try count
#define PR_AFU_REVOKE_MAX_TRY 400

// PR revoke work queue timeout in milliseconds
#define PR_WQ_REVOKE_TIMEOUT 10

// PR AFU Deactivated work queue timeout in milliseconds
#define PR_WQ_TIMEOUT 0

// AFU Resource Release queue timeout in milliseconds
#define AFU_RES_RELEASE_TIMEOUT 5

// Power Manager PR response  10 seconds
#define PWRMGR_RESPONSE_TIMEOUT  1000000

//#define PWRMGR   1;

// Green Bit stream Header
struct __attribute__((__packed__))  CCIP_GBS_HEADER {

   btByte                          m_digst[256];                   // Digital signature
   btByte                          m_mesgDist[32];                 // Message digest
   btByte                          m_intelpub_key[260];            // Intel public key
   btByte                          m_hash_pubkey[32];              // Hash of public key
   btByte                          m_md_afu[16];                   // GB meta Data structure
   btByte                          m_md_slot_type_uuid[16];        // Partial bitstream slot type UUID
   btUnsigned32bitInt              m_md_afu_power;                 // Partial bitstream power
   btByte                          m_md_port;                      // Partial bitstream port
   btUnsigned16bitInt              m_md_latency;                   // Partial bitstream latency
   btUnsigned32bitInt              m_md_clknum;                    // Partial bitstream clock number
   btUnsigned32bitInt              m_md_btlength;                  // Partial bitstream length *

};

///============================================================================
/// Name: pr_program_context
/// @brief PR worker thread device context
///
///============================================================================
struct pr_program_context
{
   struct kosal_work_object         m_workobject;
   btUnsigned64bitInt               m_cmd;
   struct port_device              *m_pportdev;
   struct cci_aal_device           *m_pPR_dev;
   struct cci_aal_device           *m_pAFU_dev;
   struct cci_aal_device           *m_pSigtap_dev;
   struct aaldev_ownerSession      *m_pownerSess;
   btVirtAddr                       m_kbufferptr;
   btWSSize                         m_bufferlen;
   int                              m_prregion_id ;
   btTime                           m_reconfTimeout;
   btUnsigned64bitInt               m_reconfAction;
   uid_afurespID_e                  m_respID;
   btUnsigned64bitInt               m_evt_data;
   uid_errnum_e                     m_eno;
   btBool                           m_leaveDeactivated;
   btUnsigned64bitInt               m_afuRevokeCount;
   btUnsigned64bitInt               m_sigtapRevokeCount;
   btUnsigned64bitInt               m_timeElapsed;
   stTransactionID_t                m_pwrReqTranID;
   struct CCIP_GBS_HEADER          *m_gbs_header;
};



/// Name: program_afu_callback
/// @brief Reconfigures  AFU with bitstream
///
/// @param[in] pr_context - pr program context
/// @return    void
void program_afu_callback(struct kosal_work_object * pwork);

/// Name: reconfigure_activateAFU
/// @brief Activates afu after reconfiguration
///
///
/// @param[in] pportdev - Port device pointer
/// @param[in] pdev - cci aal device pointer
/// @return    AFU activate status
btBool  reconfigure_activateAFU(struct port_device  *pportdev,struct cci_aal_device  *pdev );

/// Name: afu_request_release_revoke_sendevent
/// @brief Sends Release and Revoke event to Resource Owners
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void afu_request_release_revoke_sendevent(void* pr_context, btBool releaseAFU);

/// Name: sigtap_revoke_sendevent
/// @brief Sends Revoke event to Resource Owners
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void sigtap_revoke_sendevent(void* pr_context);

/// Name: deactiavted_afu_device
/// @brief unpublish and deactivates AFU
///
/// @param[in] pportdev -port device pointer
/// @return    void
void deactiavted_afu_device(struct port_device  *pportdev);

/// Name: afu_revoke_callback
/// @brief Revokes AFU from applications
///
/// @param[in] pr_context -pr configuration context
/// @param[in] ptr - null pointer.
/// @return    void
void afu_revoke_callback(struct kosal_work_object *pwork);

/// Name: afu_release_siganltap
/// @brief Releases AFU Resources and program afu
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void afu_release_siganltap(void* pr_context);

/// Name: sigtap_revoke_callback
/// @brief Revokes AFU from applications
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void sigtap_revoke_callback(struct kosal_work_object *pwork);

/// Name: afu_release_timeout_callback
/// @brief AFU Deactivate /reconfigure timeout callback
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void afu_release_timeout_callback(struct kosal_work_object *pwork);

/// Name: pwrmgr_timeout_callback
/// @brief Power manager  timeout callback
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void pwrmgr_timeout_callback(struct kosal_work_object * pwork);

/// Name: reconfigure_bitstream
/// @brief Core Idle is done Start PR
///
/// @param[in] pr_context -pr configuration context
/// @return    void
void reconfigure_bitstream(struct pr_program_context* ppr_context);


/// Name: reconfigure_bitstream_cancelPwrmgrTimer
/// @brief Core Idle is done Start PR
///
/// @param[in] pr_context -pr configuration context
/// @param[in] pr_pwrmgmt_status -Core Idle Status
/// @return    void
void  reconfigure_bitstream_cancelPwrmgrTimer(struct pr_program_context* ppr_program_ctx,
                                              int pr_pwrmgmt_status);

/// Name: send_event_to_pwrmgr
/// @brief Sends event ot power manager
///
/// @param[in] pr_context -pr configuration context
/// @return    error code if fails , 0 if pass
int send_event_to_pwrmgr(struct pr_program_context* ppr_context,int power_required);

/// Name: getport_device
/// @brief get port device  pointer
///
/// @param[in] pdev    -aal device pointer
/// @param[in] portId  -port index
/// @return    port device pointer
struct port_device * getport_device(struct cci_aal_device  *pdev , int portId);

/// Name: ccipdrv_event_afu_aysnc_pr_release_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
void ccipdrv_event_afu_aysnc_pr_release_send(struct pr_program_context *ppr_program_ctx,
                                             uid_errnum_e  eno);

/// Name: ccipdrv_event_reconfig_event_create_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
void ccipdrv_event_reconfig_event_create_send( struct pr_program_context *ppr_program_ctx,
                                               uid_afurespID_e            respID,
                                               btObjectType               devhandle,
                                               stTransactionID_t          *tranID,
                                               btObjectType               context,
                                               uid_errnum_e               errnum);

/// Name: ccipdrv_event_activationchange_event_create_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
void  ccipdrv_event_activationchange_event_create_send( struct pr_program_context *ppr_program_ctx,
                                                        uid_afurespID_e            respID,
                                                        btObjectType               devhandle,
                                                        stTransactionID_t          *tranID,
                                                        btObjectType               context,
                                                        uid_errnum_e               errnum);

END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_PR_H_ */
