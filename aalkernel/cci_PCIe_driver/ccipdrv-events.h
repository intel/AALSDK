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
//=============================================================================
// Name: ccipdrv_event_afu_response_event
// Description: AFU Reponse event. Sent to report various AFU responses
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

/*
   union{
      struct aalui_AFUResponse *m_response;
      btVirtAddr                m_payload;
   };
*/

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

   struct ccipdrv_event_afu_response_event *This =
      (struct ccipdrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_response_event) );

   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct ccipdrv_event_afu_response_event));
#if 0
   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;
   This->m_tranID    = *tranID;

   // Allocate the RWB (NOTE: m_payload is union-ed with the m_response pointer)
   This->m_response = (struct aalui_AFUResponse *)kosal_kmalloc(sizeof(struct aalui_AFUResponse));
   if ( unlikely( NULL == This->m_response ) ) {
       This->m_errnum = uid_errnumNoMem;
   } else {
       This->m_response->respID  = uid_afurespUndefinedRequest;
       This->m_response->evtData = 0;
       This->m_response->payloadsize = 0;
       AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse );
   }

   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
#endif
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

//=============================================================================
//=============================================================================
// Name: ccipdrv_event_afu_workspace_event
// Description: Event sent by the PIP to signal that an AFU workspace operation
//              has  completed
//=============================================================================
//=============================================================================
struct ccipdrv_event_afu_workspace_event
{
#define qip_to_ui_evtp_afuwsevent(pqi)  kosal_container_of(pqi, struct ccipdrv_event_afu_workspace_event, m_qitem)
#define ui_evtp_afucwsevent_to_qip(evt) ( &AALQI(evt) )
   //
   // Including the macro effectively causes this object to be derived from
   // aal_q_item
   //
   _DECLARE_AALQ_TYPE;

   btObjectType          m_devhandle;
   btVirtAddr            m_payload;
   struct aalui_WSMEvent m_WSEvent;
   uid_errnum_e          m_errnum;
   stTransactionID_t     m_tranID;
   btObjectType          m_context;
};


//=============================================================================
// Name: uidrv_event_bindcmplt
// Description: Bind Complete
//=============================================================================
struct uidrv_event_bindcmplt {
#define qip_to_ui_evtp_bindcmplt(pqi) (container_of( pqi, struct uidrv_event_bindcmplt, m_qitem ) )
#define ui_evtp_bindcmplt_to_qip(evt) ( &AALQI(evt) )
   //
   // Including the macro effectively causes this object to be derived from
   // aal_q_item
   //
   _DECLARE_AALQ_TYPE;

   struct aalui_extbindargs     m_extargs;
   btObjectType                *m_devhandle;
   uid_errnum_e                 m_errno;
   stTransactionID_t            m_tranID;
   btAny                        m_context;

};

//=============================================================================
// Name: uidrv_event_bindcmplt_create
// Description: Constructor
//=============================================================================
static inline struct uidrv_event_bindcmplt *
                uidrv_event_bindcmplt_create( btObjectType m_devhandle,
                                              struct aalui_extbindargs *extargs,
                                              uid_errnum_e errno,
                                              struct aalui_ioctlreq  *preq)
{
   struct uidrv_event_bindcmplt * This =
                        kmalloc(sizeof(struct uidrv_event_bindcmplt),GFP_KERNEL);
   if(This == NULL){
      return NULL;
   }

   This->m_devhandle = m_devhandle;
   This->m_errno = errno;
   if(extargs){
      This->m_extargs = *extargs;
   }

   This->m_context = preq->context;
   This->m_tranID = preq->tranID;

   AALQ_QID(This) = rspid_UID_BindComplete;

   // The payload for this message will be the bindargs
   AALQ_QLEN(This) = sizeof(struct aalui_extbindargs);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: uidrv_event_bindcmplt_destroy
// Description: Destructor
//=============================================================================
static inline void uidrv_event_bindcmplt_destroy(struct uidrv_event_bindcmplt *This)
{
   kfree(This);
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
static inline struct uidrv_event_unbindcmplt *
                     uidrv_event_Unbindcmplt_create(uid_errnum_e errno,
                                                    struct aalui_ioctlreq  *preq)
{
   struct uidrv_event_unbindcmplt * This =
                        kmalloc(sizeof(struct uidrv_event_bindcmplt),GFP_KERNEL);
   if(This == NULL){
      return NULL;
   }

   This->m_context   = preq->context;
   This->m_tranID    = preq->tranID;


   This->m_errno     = errno;

   AALQ_QID(This)    = rspid_UID_UnbindComplete;
   AALQ_QLEN(This)   = 0;


   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: uidrv_event_Unbindcmplt_destroy
// Description: Destructor
//=============================================================================
static inline void uidrv_event_Unbindcmplt_destroy(struct uidrv_event_unbindcmplt *This)
{
   kfree(This);
}


//=============================================================================
// Name: ccipdrv_event_afu_afucsrmap_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_workspace_event *
ccipdrv_event_afu_afuallocws_create(btObjectType      devhandle,
                                  btWSID            wsid,
                                  btVirtAddr        ptr,
                                  btPhysAddr        physptr,
                                  btWSSize          size,
                                  stTransactionID_t tranID,
                                  btObjectType      context,
                                  uid_errnum_e      errnum)
{
   struct ccipdrv_event_afu_workspace_event *This =
      (struct ccipdrv_event_afu_workspace_event *)kosal_kmalloc(sizeof(struct ccipdrv_event_afu_workspace_event));

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle                = devhandle;
   This->m_WSEvent.evtID            = uid_wseventAllocate;
   This->m_WSEvent.wsParms.wsid     = wsid;
   This->m_WSEvent.wsParms.ptr      = ptr;
   This->m_WSEvent.wsParms.physptr  = physptr;
   This->m_WSEvent.wsParms.size     = size;
   This->m_payload                  = (btVirtAddr)&This->m_WSEvent;
   This->m_errnum                   = errnum;
   This->m_context                  = context;
   This->m_tranID                   = tranID;


   AALQ_QID(This)                   = rspid_WSM_Response;

   // Payload
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
struct ccipdrv_event_afu_workspace_event *
ccipdrv_event_afu_afufreecws_create(btObjectType      devhandle,
                                  stTransactionID_t tranID,
                                  btObjectType      context,
                                  uid_errnum_e      eno)
{
   struct ccipdrv_event_afu_workspace_event *This =
      (struct ccipdrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_workspace_event) );
   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle     = devhandle;
   This->m_WSEvent.evtID = uid_wseventFree;
   This->m_payload       = (btVirtAddr)&This->m_WSEvent;
   This->m_errnum        = eno;
   This->m_context       = context;
   This->m_tranID        = tranID;

   // no payload
   AALQ_QID(This)  = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: ccipdrv_event_afu_afugetphysws_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_workspace_event *
ccipdrv_event_afu_afugetphysws_create(btObjectType      devhandle,
                                    btPhysAddr        ptr,
                                    stTransactionID_t tranID,
                                    btObjectType      context,
                                    uid_errnum_e      errnum)
{
   struct ccipdrv_event_afu_workspace_event *This =
      (struct ccipdrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_workspace_event) );
   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle               = devhandle;
   This->m_WSEvent.evtID           = uid_wseventGetPhys;
   This->m_WSEvent.wsParms.physptr = ptr;
   This->m_payload                 = (btVirtAddr)&This->m_WSEvent;
   This->m_errnum                  = errnum;
   This->m_context                 = context;
   This->m_tranID                  = tranID;

   // no payload
   AALQ_QID(This)  = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: ccipdrv_event_afu_afugetcsrmap_create
// Description: Constructor
//=============================================================================
static inline
struct ccipdrv_event_afu_workspace_event *
ccipdrv_event_afu_afugetcsrmap_create( btObjectType      devhandle,
                                       btWSID            wsid,
                                       btPhysAddr        physptr,
                                       btWSSize          size,
                                       btWSSize          csrsize,
                                       btWSSize          csrspacing,
                                       stTransactionID_t tranID,
                                       btObjectType      context,
                                       uid_errnum_e      errnum)
{
   struct ccipdrv_event_afu_workspace_event *This =
      (struct ccipdrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct ccipdrv_event_afu_workspace_event) );

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle                   = devhandle;
   This->m_WSEvent.evtID               = uid_wseventMMIOMap;
   This->m_WSEvent.wsParms.wsid        = wsid;
   This->m_WSEvent.wsParms.ptr         = NULL;
   This->m_WSEvent.wsParms.physptr     = physptr;
   This->m_WSEvent.wsParms.size        = size;
   This->m_WSEvent.wsParms.itemsize    = csrsize;
   This->m_WSEvent.wsParms.itemspacing = csrspacing;
   This->m_payload                     = (btVirtAddr)&This->m_WSEvent;
   This->m_errnum                      = errnum;
   This->m_context                     = context;
   This->m_tranID                      = tranID;

   // Payload
   AALQ_QID(This)  = rspid_WSM_Response;
   AALQ_QLEN(This) = sizeof(struct aalui_WSMEvent);

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: ccipdrv_event_afucwsevent_destroy
// Description: Destructor
//=============================================================================
static inline
void
ccipdrv_event_afucwsevent_destroy(struct ccipdrv_event_afu_workspace_event *This)
{
   kosal_kfree(This, sizeof(struct ccipdrv_event_afu_workspace_event));
}


END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_CCIPDRV_EVENTS_H__

