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
//        FILE: ccidrv-events.h
//     CREATED: Nov. 2, 2008
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE: This implements the external events for the Intel(R)
//          QuickAssist Technology Accelerator Abstraction Layer (AAL)
//          CCIP Debice Driver
// HISTORY:
// WHEN:          WHO:     WHAT:
// 11/02/2015     JG       Initial Version started
//****************************************************************************
#ifndef __AALSDK_KERNEL_CCIPDRV_EVENTS_H__
#define __AALSDK_KERNEL_CCIPDRV_EVENTS_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/aaldevice.h>
#include <aalsdk/kernel/aalqueue.h>

#include <aalsdk/kernel/ccipdriver.h>

BEGIN_NAMESPACE(AAL)

//=============================================================================
//=============================================================================
// Name: ccipdrv_event_afu_response_event
// @brief AFU Reponse event. Sent to report various AFU responses
//=============================================================================
//=============================================================================
struct ccipdrv_event_afu_response_event
{
#define qip_to_ui_evtp_afuresponse(pqi) kosal_container_of(pqi, struct ccipdrv_event_afu_response_event, m_qitem)
#define ui_evtp_afuresponse_to_qip(evt) ( &AALQI(evt) )
   //
   // Including the macro effectively causes this object to be derived from
   // aal_q_item
   //
   _DECLARE_AALQ_TYPE;

   btObjectType      m_devhandle;
   uid_errnum_e      m_errnum;
   stTransactionID_t m_tranID;
   btObjectType      m_context;
   btUnsignedInt     m_payloadsize;
   btByte            m_payload[1];

};

//=============================================================================
// Name: ccipdrv_event_shutdown_event_create
// Inputs: reason - reason for the shutdown
//         timeleft - If a timeout was given in the request how much is left
//                    for user space shutdown.(i.e.,timeout-timeused in kernel)
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_shutdown_event_create(ui_shutdownreason_e reason,
                                    btTime              timeleft,
                                    stTransactionID_t  *tranID,
                                    btObjectType        context,
                                    uid_errnum_e        errnum)
{
   struct aalui_Shutdown *shutdownnparms = NULL;

   // Allocate for event and payload
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_Shutdown) );

   if ( NULL == This ) {
      return NULL;
   }

   // Fill in generic stuff
   This->m_errnum             = errnum;
   This->m_context            = context;
   This->m_tranID             = *tranID;

   // Point at the payload
   shutdownnparms = (struct aalui_Shutdown*)This->m_payload;

   // Fill in the payload
   shutdownnparms->m_reason = reason;
   shutdownnparms->m_timeout = timeleft;

   AALQ_QLEN(This) = sizeof(struct aalui_Shutdown);
   AALQ_QID(This)  = rspid_UID_Shutdown;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: ccipdrv_event_activationchange_event_create
// Inputs: reason - reason for the shutdown
//         timeleft - If a timeout was given in the request how much is left
//                    for user space shutdown.(i.e.,timeout-timeused in kernel)
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_activationchange_event_create( uid_afurespID_e    respID,
                                             btObjectType       devhandle,
                                             stTransactionID_t *tranID,
                                             btObjectType       context,
                                             uid_errnum_e       errnum)
{
struct aalui_AFUResponse *response = NULL;
struct ccipdrv_event_afu_response_event *This =
(struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_AFUResponse));

if ( NULL == This ) {
return NULL;
}

This->m_devhandle = devhandle;
This->m_errnum    = errnum;
This->m_context   = context;
This->m_tranID    = *tranID;

// Point at the payload
response = (struct aalui_AFUResponse*)This->m_payload;

response->respID  = respID;
response->evtData = 0;
response->payloadsize = 0;

AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );
AALQ_QID(This) = rspid_AFU_Response;

// Initialize the queue item
kosal_list_init(&AALQ_QUEUE(This));

return This;
}

//=============================================================================
// Name: ccipdrv_event_reconfig_event_create
// Inputs:
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_reconfig_event_create( uid_afurespID_e    respID,
                                     btObjectType       devhandle,
                                     stTransactionID_t *tranID,
                                     btObjectType       context,
                                     uid_errnum_e       errnum)
{
struct aalui_AFUResponse *response = NULL;
struct ccipdrv_event_afu_response_event *This =
(struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_AFUResponse));

if ( NULL == This ) {
return NULL;
}

This->m_devhandle = devhandle;
This->m_errnum    = errnum;
This->m_context   = context;
This->m_tranID    = *tranID;

// Point at the payload
response = (struct aalui_AFUResponse*)This->m_payload;

response->respID  = respID;
response->evtData = 0;
response->payloadsize = 0;

AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );
AALQ_QID(This) = rspid_AFU_Response;

// Initialize the queue item
kosal_list_init(&AALQ_QUEUE(This));

return This;
}


//=============================================================================
// Name: ccipdrv_event_afu_afuinavlidrequest_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_afuinavlidrequest_create(btObjectType       devhandle,
                                           stTransactionID_t *tranID,
                                           btObjectType       context,
                                           uid_errnum_e       eno)
{
   struct aalui_AFUResponse *response = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_AFUResponse));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;
   This->m_tranID    = *tranID;

   // Point at the payload
   response = (struct aalui_AFUResponse*)This->m_payload;

   response->respID  = uid_afurespUndefinedRequest;
   response->evtData = 0;
   response->payloadsize = 0;

   AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );
   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));

   return This;
}

//=============================================================================
// Name: ccipdrv_event_afu_afucsrmap_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_afuallocws_create( btObjectType      devhandle,
                                     btWSID            wsid,
                                     btVirtAddr        ptr,
                                     btPhysAddr        physptr,
                                     btWSSize          size,
                                     stTransactionID_t tranID,
                                     btObjectType      context,
                                     uid_errnum_e      errnum)
{
   struct aalui_WSMEvent * WSEvent = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kmalloc(sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_WSMEvent));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle                = devhandle;
   This->m_errnum                   = errnum;
   This->m_context                  = context;
   This->m_tranID                   = tranID;

   // Point at the payload
   WSEvent = (struct aalui_WSMEvent*)This->m_payload;

   WSEvent->evtID            = uid_wseventAllocate;
   WSEvent->wsParms.wsid     = wsid;
   WSEvent->wsParms.ptr      = ptr;
   WSEvent->wsParms.physptr  = physptr;
   WSEvent->wsParms.size     = size;

   AALQ_QID(This)                   = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: ccipdrv_event_afu_afufreecws_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_afufreecws_create( btObjectType      devhandle,
                                     stTransactionID_t tranID,
                                     btObjectType      context,
                                     uid_errnum_e      eno)
{
   struct aalui_WSMEvent * WSEvent = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_response_event)+ sizeof(struct aalui_WSMEvent) );
   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle     = devhandle;
   This->m_errnum        = eno;
   This->m_context       = context;
   This->m_tranID        = tranID;

   // Point at the payload
   WSEvent = (struct aalui_WSMEvent*)This->m_payload;
   WSEvent->evtID = uid_wseventFree;

   // no payload
   AALQ_QID(This)  = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

///============================================================================
/// Name: ccipdrv_event_afu_aysnc_pr_revoke_create
/// @brief Creates AFU revoke event
///
/// @param[in] respID - Response ID
/// @param[in] devhandle -aal device handle.
/// @param[in] context - applicator context.
/// @param[in] eno - error id.
/// @return    afu Response event
///============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_aysnc_pr_revoke_create(uid_afurespID_e    respID,
                                         btObjectType       devhandle,
                                         btObjectType       context,
                                         uid_errnum_e       eno)
{
   struct aalui_AFUResponse *response = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_AFUResponse));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;

   // Point at the payload
   response = (struct aalui_AFUResponse*)This->m_payload;

   response->respID      = respID;
   response->evtData     = 0;
   response->payloadsize = 0;

   AALQ_QID(This)  = rspid_AFU_PR_Revoke_Event;
   AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));

   return This;
}

///============================================================================
/// Name: ccipdrv_event_afu_aysnc_pr_request_release_create
/// @brief Creates AFU release event
///
/// @param[in] respID - Response ID
/// @param[in] devhandle -aal device handle.
/// @param[in] context - applicator context.
/// @param[in] eno - error id.
/// @return    afu Response event
///============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_aysnc_pr_request_release_create(uid_afurespID_e    respID,
                                                  btObjectType       devhandle,
                                                  btObjectType       context,
                                                  btUnsigned64bitInt action,
                                                  btUnsigned64bitInt timeout,
                                                  uid_errnum_e       eno)
{
   UNREFERENCED_PARAMETER(action);

   struct aalui_PREvent *response = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_PREvent));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle  = devhandle;
   This->m_errnum     = eno;
   This->m_context    = context;

   // Point at the payload
   response = (struct aalui_PREvent*)This->m_payload;

   response->respID        = respID;
   response->evtData       = 0;
   response->reconfTimeout = timeout;

   AALQ_QID(This)  = rspid_AFU_PR_Release_Request_Event;
   AALQ_QLEN(This) = sizeof(struct aalui_PREvent );

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));

   return This;
}

///============================================================================
/// Name: ccipdrv_event_afu_aysnc_pr_release_create
/// @brief Creates PR release event
///
/// @param[in] respID - Response ID
/// @param[in] devhandle -aal device handle.
/// @param[in] context - applicator context.
/// @param[in] eno - error id.
/// @return    afu Response event
///============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_aysnc_pr_release_create(uid_afurespID_e    respID,
                                          btObjectType       devhandle,
                                          btObjectType       context,
                                          uid_errnum_e       eno)
{
   struct aalui_AFUResponse *response = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_AFUResponse));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;

   // Point at the payload
   response = (struct aalui_AFUResponse*)This->m_payload;

   response->respID      = respID;
   response->evtData     = 0;
   response->payloadsize = 0;

   AALQ_QID(This)  = rspid_AFU_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));

   return This;
}


//=============================================================================
// Name: ccipdrv_event_afu_afugetmmiomap_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_response_event *
ccipdrv_event_afu_afugetmmiomap_create( btObjectType      devhandle,
                                        btWSID            wsid,
                                        btPhysAddr        physptr,
                                        btWSSize          size,
                                        stTransactionID_t tranID,
                                        btObjectType      context,
                                        uid_errnum_e      errnum)
{
   struct aalui_WSMEvent * WSEvent = NULL;
   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_response_event) + sizeof(struct aalui_WSMEvent));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle                   = devhandle;
   This->m_errnum                      = errnum;
   This->m_context                     = context;
   This->m_tranID                      = tranID;

   // Point at the payload
   WSEvent = (struct aalui_WSMEvent*)This->m_payload;
   WSEvent->evtID               = uid_wseventMMIOMap;
   WSEvent->wsParms.wsid        = wsid;
   WSEvent->wsParms.ptr         = NULL;
   WSEvent->wsParms.physptr     = physptr;
   WSEvent->wsParms.size        = size;

   // Payload
   AALQ_QID(This)  = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: ccipdrv_event_bindcmplt_create
// Description: Constructor
//=============================================================================
static inline struct ccipdrv_event_afu_response_event *
                ccipdrv_event_bindcmplt_create( btObjectType devhandle,
                                                struct ccipdrv_DeviceAttributes *pattributes,
                                                uid_errnum_e errnum,
                                                struct ccipui_ioctlreq  *preq)
{
   struct ccipdrv_DeviceAttributes           *pattrib       = NULL;
   btWSSize                                   payloadsize   = 0;
   struct ccipdrv_event_afu_response_event   *This          = NULL;

   if( NULL != pattributes){
      // Payload size is the size of the attribute object minus 1 for the array entry
      //  plus the size of the variable data
      payloadsize = (sizeof(struct ccipdrv_DeviceAttributes)) + pattributes->m_size;
   }

   // Allocate object
   This =  (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc( sizeof( struct ccipdrv_event_afu_response_event) +
                                                                      payloadsize );
   if(This == NULL){
      return NULL;
   }

   This->m_devhandle = devhandle;
   This->m_errnum    = errnum;
   This->m_context   = preq->context;
   This->m_tranID    = preq->tranID;

   if(pattributes){

      // Point at payload
      pattrib = (struct ccipdrv_DeviceAttributes*)This->m_payload;

      // Copy the main body
      *pattrib = *pattributes;
      if( 0 != pattributes->m_size){
         // Now copy the payload
         memcpy(pattrib->m_devattrib,pattributes->m_devattrib, pattributes->m_size);
      }
   }

   AALQ_QID(This)    = rspid_UID_BindComplete;
   AALQ_QLEN(This)   = payloadsize;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: uidrv_event_unbindcmplt
// Description: Bind Complete
//=============================================================================
struct uidrv_event_unbindcmplt
{
#define qip_to_ui_evtp_unbindcmplt(pqi) (container_of( pqi, struct uidrv_event_unbindcmplt, m_qitem ) )
#define ui_evtp_unbindcmplt_to_qip(evt) ( &AALQI(evt) )
   //
   // Including the macro effectively causes this object to be derived from
   // aal_q_item
   //
   _DECLARE_AALQ_TYPE;

   uid_errnum_e                 m_errno;
   stTransactionID_t            m_tranID;
   void                        *m_context;
};

//=============================================================================
// Name: uidrv_event_Unbindcmplt_create
// Description: Constructor
//=============================================================================
static inline struct ccipdrv_event_afu_response_event *
                     ccipdrv_event_Unbindcmplt_create(uid_errnum_e errnum,
                                                    struct ccipui_ioctlreq  *preq)
{
   struct ccipdrv_event_afu_response_event * This =
                        (struct ccipdrv_event_afu_response_event *)kosal_kzmalloc(sizeof(struct ccipdrv_event_afu_response_event));
   if(This == NULL){
      return NULL;
   }

   This->m_context   = preq->context;
   This->m_tranID    = preq->tranID;
   This->m_errnum    = errnum;

   AALQ_QID(This)    = rspid_UID_UnbindComplete;
   AALQ_QLEN(This)   = 0;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: ccipdrv_event_afuresponse_destroy
// Description: Destructor
//=============================================================================
static inline
void
ccipdrv_event_afuresponse_destroy(struct ccipdrv_event_afu_response_event *This)
{
   kosal_kfree(This, sizeof(struct ccipdrv_event_afu_response_event) + AALQ_QLEN(This));
}


END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_CCIPDRV_EVENTS_H__

