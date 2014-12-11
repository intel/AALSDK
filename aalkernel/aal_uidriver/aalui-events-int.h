//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2014, Intel Corporation.
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
//  Copyright(c) 2008-2014, Intel Corporation.
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
//        FILE: aalui-events-int.h
//     CREATED: Sep 28, 2008
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: This implements the internal events for the Intel(R)
//          QuickAssist Technology Accelerator Abstraction Layer (AAL)
//          Universal Interface Driver
// HISTORY:
// WHEN:          WHO:     WHAT:
// 09/28/2008     JG       Initial Version started
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 03/17/2009     JG       Modified Response event for SetGet CSR
//****************************************************************************
#ifndef __AALKERNEL_AAL_UIDRIVER_AALUI_EVENTS_INT_H__
#define __AALKERNEL_AAL_UIDRIVER_AALUI_EVENTS_INT_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalqueue.h"
#include "aalsdk/kernel/aalui.h"

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


#endif // __AALKERNEL_AAL_UIDRIVER_AALUI_EVENTS_INT_H__
