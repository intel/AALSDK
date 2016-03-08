//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2016, Intel Corporation.
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
//  Copyright(c) 2008-2016, Intel Corporation.
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
//        FILE: aalui-events.h
//     CREATED: Dec. 3, 2008
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: This implements the external events for the
//          Accelerator Abstraction Layer (AAL)
//          Universal Interface Driver
// HISTORY:
// WHEN:          WHO:     WHAT:
// 12/03/2008     JG       Initial Version started
// 12/27/2008     JG       Support for AFU Response payload
// 01/04/2009     HM       Updated Copyright
// 03/20/2009     JG/HM    Global change to AFU_Response that generically puts
//                            payloads after the structure with a pointer to
//                            them. Ptr must be converted kernel to user.
// 03/27/2009     JG       Added support for MGMT AFU interface
// 05/15/2009     HM       Changed error code in uid_afurespID_e forced update
// 06/05/2009     JG       Added shutdown
// 06/27/2009     JG       Added timeout parameter to shutdown
//                         (unused but passed through)
// 08/06/2009     AC       Fixed a bug for payload buffer allocate failing
// 12/27/2009     JG       Added support for csrmap event
// 03/06/2012     JG       Put a check into uidrv_event_afutrancmplt_create
//                          to check for NULL ptaskComplete
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALUI_EVENTS_H__
#define __AALSDK_KERNEL_AALUI_EVENTS_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/aaldevice.h>
#include <aalsdk/kernel/aalqueue.h>
#include <aalsdk/kernel/aalui.h>

BEGIN_NAMESPACE(AAL)

//=============================================================================
//=============================================================================
// Name: uidrv_event_shutdown_event
// Description: UI driver shutdown event.
//=============================================================================
//=============================================================================
struct uidrv_event_shutdown_event
{
#define qip_to_ui_evtp_uishutdown(pqi) kosal_container_of(pqi, struct uidrv_event_shutdown_event, m_qitem)
#define ui_evtp_uishutdown_to_qip(evt) ( &AALQI(evt) )
   //
   // Including the macro effectively causes this object to be derived from
   // aal_q_item
   //
   _DECLARE_AALQ_TYPE;

   uid_errnum_e          m_errnum;
   stTransactionID_t     m_tranID;
   btObjectType          m_context;
   btVirtAddr            m_payload;
   struct aalui_Shutdown m_shutdown;
};

//=============================================================================
// Name: uidrv_event_shutdown_event_create
// Inputs: reason - reason for the shutdown
//         timeleft - If a timeout was given in the request how much is left
//                    for user space shutdown.(i.e.,timeout-timeused in kernel)
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_shutdown_event *
uidrv_event_shutdown_event_create(ui_shutdownreason_e reason,
                                  btTime              timeleft,
                                  stTransactionID_t  *tranID,
                                  btObjectType        context,
                                  uid_errnum_e        errnum)
{
   struct uidrv_event_shutdown_event *This =
      (struct uidrv_event_shutdown_event *)kosal_kmalloc( sizeof(struct uidrv_event_shutdown_event) );

   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_shutdown_event));

   This->m_errnum             = errnum;
   This->m_context            = context;
   This->m_tranID             = *tranID;
   This->m_shutdown.m_reason  = reason;
   This->m_shutdown.m_timeout = timeleft;
   This->m_payload            = (btVirtAddr)&This->m_shutdown;

   AALQ_QLEN(This) = sizeof(struct aalui_Shutdown);
   AALQ_QID(This)  = rspid_UID_Shutdown;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: uidrv_event_shutdown_event_destroy
// Description: Destructor
//=============================================================================
static inline
void
uidrv_event_shutdown_event_destroy(struct uidrv_event_shutdown_event *This)
{
   kosal_kfree(This, sizeof(struct uidrv_event_shutdown_event));
}



//=============================================================================
//=============================================================================
// Name: uidrv_event_afu_response_event
// Description: AFU Reponse event. Sent to report various AFU responses
//=============================================================================
//=============================================================================
struct uidrv_event_afu_response_event
{
#define qip_to_ui_evtp_afuresponse(pqi) kosal_container_of(pqi, struct uidrv_event_afu_response_event, m_qitem)
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

   union{
      struct aalui_AFUResponse *m_response;
      btVirtAddr                m_payload;
   };

};

//=============================================================================
// Name: uidrv_event_afu_afucsrgetset_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_response_event *
uidrv_event_afu_afucsrgetset_create(btObjectType        devhandle,
                                    csr_read_write_blk *pcsrrwb,
                                    btUnsigned64bitInt  index,
                                    stTransactionID_t  *tranID,
                                    btObjectType        context,
                                    uid_errnum_e        errnum)
{
   // Response buffer structure defined as Response followed by payload
   struct big
   {
     struct aalui_AFUResponse  rsp;
     struct csr_read_write_blk csrBlk;
   } *pbig;

   btWSSize size = sizeof(struct csr_offset_value) *  (pcsrrwb->num_to_get +  pcsrrwb->num_to_set);
   btWSSize payloadsize = (sizeof(struct big) + size);

   struct uidrv_event_afu_response_event *This =
      (struct uidrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_response_event) );
   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_errnum    = errnum;
   This->m_context   = context;
   This->m_tranID    = *tranID;

   AALQ_QLEN(This) = 0;
   This->m_payload = NULL;

   // Allocate the RWB (NOTE: m_payload is unioned with the m_response pointer)
   This->m_response = (struct aalui_AFUResponse *)kosal_kmalloc(payloadsize);
   pbig = (struct big *)This->m_response;

   if ( unlikely( NULL == This->m_payload ) ) {
      This->m_errnum = uid_errnumNoMem;
   } else {
      This->m_response->respID = uid_afurespSetGetCSRComplete;
      This->m_response->evtData = index;
      This->m_response->pcsrBlk = &pbig->csrBlk;

      This->m_response->pcsrBlk->num_to_get = pcsrrwb->num_to_get;
      This->m_response->pcsrBlk->num_to_set = pcsrrwb->num_to_set;
      kosal_printk("Num CSR get %" PRIu64 ", Num CSR Set %" PRIu64 "\n\n", This->m_response->pcsrBlk->num_to_get, This->m_response->pcsrBlk->num_to_set);
      memcpy(This->m_response->pcsrBlk->csr_array,pcsrrwb->csr_array, (size_t)size);
   }

   AALQ_QLEN(This) = payloadsize;
   AALQ_QID(This)  = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: uidrv_event_afutrancmplt_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_response_event *
uidrv_event_afutrancmplt_create(btObjectType               devhandle,
                                stTransactionID_t         *tranID,
                                btObjectType               context,
                                struct aalui_AFUResponse  *responsep,
                                struct aalui_taskComplete *taskcomplete,
                                btByteArray                pAFUDSMParms,
                                btUnsignedInt              parmsSize,
                                uid_errnum_e               eno)
{
   struct uidrv_event_afu_response_event *This =
      (struct uidrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_response_event) );

   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_errnum     = eno;
   This->m_context   = context;
   This->m_tranID    = *tranID;

   AALQ_QLEN(This) = 0;
   This->m_payload = NULL;

   //-------------------------------------------------------------
   // Load the response data if present
   //  - Uses the payload field so that the generic event delivery
   //    code in uidrv_process_message() is consistent
   //-------------------------------------------------------------
   if(responsep != NULL){
       // Allocate the response
      struct big
      {
         struct aalui_AFUResponse  rsp;
         struct aalui_taskComplete tcp;
         btByte                    dsmparms;
      };

      // also implicitly sets m_response
      struct big *pbig = (struct big *)kosal_kmalloc(sizeof(struct big)+parmsSize);
      This->m_payload  = (btVirtAddr)pbig;

      if ( unlikely( NULL == This->m_payload ) ) {
         This->m_errnum = uid_errnumNoMem;
      } else {

         // Copy in the aaluid_AFUResponse (pointer will have to be changed)
         pbig->rsp = *responsep;
         pbig->rsp.payloadsize = 0;

         // Copy in the contents of the aalui_taskComplete
         if(NULL != taskcomplete){
            pbig->tcp = *taskcomplete;
            pbig->rsp.payloadsize += sizeof(aalui_taskComplete);
            kosal_printk("context %p, mode %d delim=%d\n\n",
                         pbig->tcp.context,
                         pbig->tcp.mode,
                         pbig->tcp.delim);

         }

         // Copy in the contents of the aalui_taskComplete
         if(NULL != pAFUDSMParms){
            memcpy(&pbig->dsmparms, pAFUDSMParms, parmsSize);
            pbig->rsp.payloadsize += parmsSize;
         }
 
         kosal_printk("uidrv_event_afutrancmplt_create: pbig=%p, ptaskComplete=%p\n\n",
                         pbig,
                         &pbig->tcp);
         AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse) + pbig->rsp.payloadsize;
      }
   }
   AALQ_QID(This)    = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: uidrv_event_afutranstate_create
// Description: Constructor for SPL2 transaction state change.
//=============================================================================
static inline
struct uidrv_event_afu_response_event *
uidrv_event_afutranstate_create(btObjectType              devhandle,
                                stTransactionID_t        *tranID,
                                btObjectType              context,
                                struct aalui_AFUResponse *responsep,
                                struct aalui_WSMParms    *pAFUDSMParms,
                                uid_errnum_e              eno)
{
   struct uidrv_event_afu_response_event *This =
      (struct uidrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_response_event) );

   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;
   This->m_tranID    = *tranID;

   AALQ_QLEN(This) = 0;
   This->m_payload = NULL;

   //-------------------------------------------------------------
   // Load the response data if present
   //  - Uses the payload field so that the generic event delivery
   //    code in uidrv_process_message() is consistent
   //-------------------------------------------------------------
   if(responsep != NULL){
       // Allocate the response
      struct big
      {
         struct aalui_AFUResponse rsp;
         struct aalui_WSMParms    wsm;
      };

      // also implicitly sets m_response
      struct big *pbig = (struct big *)kosal_kmalloc(sizeof(struct big));
      This->m_payload  = (btVirtAddr)pbig;

      if ( unlikely( NULL == This->m_payload ) ) {
         This->m_errnum = uid_errnumNoMem;
      } else {

         // Copy in the aaluid_AFUResponse (pointer will have to be changed)
         pbig->rsp = *responsep;
         pbig->rsp.payloadsize = sizeof(struct aalui_WSMParms);
         pbig->wsm = *pAFUDSMParms;

         kosal_printk("uidrv_event_afutranstate_create(): pbig=%p, payload=%p\n\n",
                         pbig, &pbig->wsm);
         AALQ_QLEN(This) = sizeof(struct big);
      }
   }
   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: uidrv_event_afu_afuinavlidrequest_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_response_event *
uidrv_event_afu_afuinavlidrequest_create(btObjectType       devhandle,
                                         stTransactionID_t *tranID,
                                         btObjectType       context,
                                         uid_errnum_e       eno)
{

   struct uidrv_event_afu_response_event *This =
      (struct uidrv_event_afu_response_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_response_event) );

   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

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
   return This;
}

//=============================================================================
// Name: uidrv_event_afuresponse_destroy
// Description: Destructor
//=============================================================================
static inline
void
uidrv_event_afuresponse_destroy(struct uidrv_event_afu_response_event *This)
{
   if ( NULL != This->m_payload ) {
      kosal_kfree(This->m_payload, AALQ_QLEN(This));
   }
   kosal_kfree(This, sizeof(struct uidrv_event_afu_response_event));
}



//=============================================================================
//=============================================================================
// Name: uidrv_event_afu_workspace_event
// Description: Event sent by the PIP to signal that an AFU workspace operation
//              has  completed
//=============================================================================
//=============================================================================
struct uidrv_event_afu_workspace_event
{
#define qip_to_ui_evtp_afuwsevent(pqi)  kosal_container_of(pqi, struct uidrv_event_afu_workspace_event, m_qitem)
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
// Name: uidrv_event_afu_afucsrmap_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_workspace_event *
uidrv_event_afu_afuallocws_create(btObjectType      devhandle,
                                  btWSID            wsid,
                                  btVirtAddr        ptr,
                                  btPhysAddr        physptr,
                                  btWSSize          size,
                                  stTransactionID_t tranID,
                                  btObjectType      context,
                                  uid_errnum_e      errnum)
{
   struct uidrv_event_afu_workspace_event *This =
      (struct uidrv_event_afu_workspace_event *)kosal_kmalloc(sizeof(struct uidrv_event_afu_workspace_event));

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
// Name: uidrv_event_afu_afufreecws_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_workspace_event *
uidrv_event_afu_afufreecws_create(btObjectType      devhandle,
                                  stTransactionID_t tranID,
                                  btObjectType      context,
                                  uid_errnum_e      eno)
{
   struct uidrv_event_afu_workspace_event *This =
      (struct uidrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_workspace_event) );
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
// Name: uidrv_event_afu_afugetphysws_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_workspace_event *
uidrv_event_afu_afugetphysws_create(btObjectType      devhandle,
                                    btPhysAddr        ptr,
                                    stTransactionID_t tranID,
                                    btObjectType      context,
                                    uid_errnum_e      errnum)
{
   struct uidrv_event_afu_workspace_event *This =
      (struct uidrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_workspace_event) );
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
// Name: uidrv_event_afu_afugetcsrmap_create
// Description: Constructor
//=============================================================================
static inline
struct uidrv_event_afu_workspace_event *
uidrv_event_afu_afugetcsrmap_create(btObjectType      devhandle,
                                    btWSID            wsid,
                                    btPhysAddr        physptr,
                                    btWSSize          size,
                                    btWSSize          csrsize,
                                    btWSSize          csrspacing,
                                    stTransactionID_t tranID,
                                    btObjectType      context,
                                    uid_errnum_e      errnum)
{
   struct uidrv_event_afu_workspace_event *This =
      (struct uidrv_event_afu_workspace_event *)kosal_kmalloc( sizeof(struct uidrv_event_afu_workspace_event) );

   if ( NULL == This ) {
      return NULL;
   }

   This->m_devhandle                   = devhandle;
   This->m_WSEvent.evtID               = uid_wseventCSRMap;
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
// Name: uidrv_event_afucwsevent_destroy
// Description: Destructor
//=============================================================================
static inline
void
uidrv_event_afucwsevent_destroy(struct uidrv_event_afu_workspace_event *This)
{
   kosal_kfree(This, sizeof(struct uidrv_event_afu_workspace_event));
}


END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALUI_EVENTS_H__

