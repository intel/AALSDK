//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2009-2014, Intel Corporation.
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
//  Copyright(c) 2009-2014, Intel Corporation.
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
//        FILE: aalmafu-events.h
//     CREATED: Oct. 26, 2008
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: This implements the external events for the Intel(R)
//          QuickAssist Technology Accelerator Abstraction Layer (AAL)
//          Universal Interface Driver
// HISTORY:
// WHEN:          WHO:     WHAT:
// 05/06/2009     HM       Removed inappropriate hard path coding of 
//                            aas/kernel/include
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALMAFU_EVENTS_H__
#define __AALSDK_KERNEL_AALMAFU_EVENTS_H__
#include <aalsdk/kernel/aalui-events.h>
#include <aalsdk/kernel/aalmafu.h>

//=============================================================================
// Name: uidrv_event_mafu_ConfigureAFU
// Description: Constructor
// Inputs:
//=============================================================================
static inline struct uidrv_event_afu_response_event *
   uidrv_event_mafu_ConfigureAFU_create( btObjectType             devhandle,
                                         afu_descriptor          *pafudec,
                                         btUnsigned32bitInt       respID,
                                         stTransactionID_t       *tranID,
                                         btObjectType             context,
                                         uid_errnum_e             eno)
{
   // Response buffer structure defined as Response followed by payload
   struct uidrv_event_afu_response_event * This =
      (struct uidrv_event_afu_response_event *)kosal_kmalloc(sizeof(struct uidrv_event_afu_response_event));
   if ( NULL == This ) {
      return NULL;
   }

   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_context   = context;
   This->m_tranID    = *tranID;
   AALQ_QLEN(This)   = 0;
   This->m_payload   = NULL;

   //-------------------------------------------------------------
   // Load the response data if present
   //  - Uses the payload field so that the generic event delivery
   //    code in uidrv_process_message() is consistent
   //-------------------------------------------------------------
   if ( NULL == pafudec ) {
      This->m_response = (struct aalui_AFUResponse *) kosal_kmalloc(sizeof(struct aalui_AFUResponse));
      if (unlikely((void*) This->m_response == NULL)) {
         This->m_errnum = uid_errnumNoMem;
      } else {
         memset(This->m_response, 0, sizeof(struct aalui_AFUResponse));
         This->m_response->respID = respID;
         AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse);
      }
   } else {
      // Allocate the response
      struct big {
         struct aalui_AFUResponse rsp;
         afu_descriptor           afu_desc;
      };

      // also implicitly sets m_response since m_response and m_payload are in a union
      struct big *pbig = (struct big *)kosal_kmalloc(sizeof(struct big));
      This->m_payload = (btVirtAddr)pbig;

      if ( unlikely( NULL == This->m_payload ) ) {
         This->m_errnum = uid_errnumNoMem;
      } else {
         memset(pbig, 0, sizeof(struct big));

         pbig->rsp.respID         = respID;
         pbig->rsp.payloadsize    = sizeof(afu_descriptor);

         // Copy in the contents of the aalui_taskComplete
         pbig->afu_desc           = *pafudec;
         pbig->afu_desc.cfgStatus = 
                     uid_afurespAFUCreateComplete == respID ? krms_ccfgUpdate_DevAdded : krms_ccfgUpdate_DevRemoved;;

         AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse) + sizeof(afu_descriptor);
      }
   }

   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: uidrv_event_mafu_DestroyAFU_create
// Description: Constructor
//=============================================================================
static inline struct uidrv_event_afu_response_event *
         uidrv_event_mafu_DestroyAFU_create( btObjectType              devhandle,
                                             stTransactionID_t        *tranID,
                                             btObjectType              context,
                                             uid_errnum_e              eno)
{
   // Response buffer structure defined as Response followed by payload
   struct uidrv_event_afu_response_event * This =
      (struct uidrv_event_afu_response_event *) kosal_kmalloc(sizeof(struct uidrv_event_afu_response_event));
   if ( NULL == This ) {
      return NULL;
   }
   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_errnum    = eno;
   This->m_tranID    = *tranID;
   This->m_context   = context;

   This->m_payload = (btVirtAddr) kosal_kmalloc(sizeof(struct aalui_AFUResponse ));
   if ( unlikely( NULL == This->m_payload ) ) {
      This->m_errnum = uid_errnumNoMem;
   } else {
      memset(This->m_payload, 0, sizeof(struct aalui_AFUResponse));
      This->m_response->respID = uid_afurespAFUDestroyComplete;
      AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse);
   }

   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: uidrv_event_mafu_afuactivate_create
// Description: Constructor
//=============================================================================
static inline struct uidrv_event_afu_response_event *
      uidrv_event_mafu_afuactivate_create( btObjectType              devhandle,
                                           unsigned                  subdev,
                                           stTransactionID_t        *tranID,
                                           btObjectType              context,
                                           uid_errnum_e              eno)
{
   // Response buffer structure defined as Response followed by payload
   struct uidrv_event_afu_response_event *This =
      (struct uidrv_event_afu_response_event *) kosal_kmalloc(sizeof(struct uidrv_event_afu_response_event));
   if ( NULL == This ) {
      return NULL;
   }

   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_tranID    = *tranID;
   This->m_context   = context;
   This->m_errnum    = eno;

   This->m_response = (struct aalui_AFUResponse *) kosal_kmalloc(sizeof(struct aalui_AFUResponse));
   if ( unlikely( NULL == This->m_response ) ) {
      This->m_errnum = uid_errnumNoMem;
   } else {
      memset(This->m_response, 0, sizeof(struct aalui_AFUResponse));

      This->m_response->respID  = uid_afurespActivateComplete;
      This->m_response->evtData = subdev;

      AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse);
   }

   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: uidrv_event_mafu_afudeactivate_create
// Description: Constructor
//=============================================================================
static inline struct uidrv_event_afu_response_event *
      uidrv_event_mafu_afudeactivate_create(btObjectType              devhandle,
                                            unsigned                  subdev,
                                            stTransactionID_t        *tranID,
                                            btObjectType              context,
                                            uid_errnum_e              eno)
{
   struct uidrv_event_afu_response_event * This =
      (struct uidrv_event_afu_response_event *) kosal_kmalloc(sizeof(struct uidrv_event_afu_response_event));
   if ( NULL == This ) {
      return NULL;
   }

   memset(This, 0, sizeof(struct uidrv_event_afu_response_event));

   This->m_devhandle = devhandle;
   This->m_tranID    = *tranID;
   This->m_context   = context;
   This->m_errnum    = eno;

   AALQ_QLEN(This) = 0;
   This->m_payload = NULL;

   This->m_response = (struct aalui_AFUResponse *) kosal_kmalloc(sizeof(struct aalui_AFUResponse));
   if ( unlikely( NULL == This->m_response ) ) {
      This->m_errnum = uid_errnumNoMem;
   } else {
      memset(This->m_response, 0, sizeof(struct aalui_AFUResponse));

      This->m_response->respID  = uid_afurespDeactivateComplete;
      This->m_response->evtData = subdev;

      AALQ_QLEN(This) = sizeof(struct aalui_AFUResponse);
   }

   AALQ_QID(This) = rspid_AFU_Response;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

#endif // __AALSDK_KERNEL_AALMAFU_EVENTS_H__

