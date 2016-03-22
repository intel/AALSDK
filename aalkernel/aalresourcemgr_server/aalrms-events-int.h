//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2009-2016, Intel Corporation.
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
//  Copyright(c) 2009-2016, Intel Corporation.
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
//        FILE: aalrms-events-int.h
//     CREATED: 03/04/2009
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains internal event definitions for the
//          AAL Resource Manager Server Service Module.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 03/04/09       JG       Initial version created
//****************************************************************************
#ifndef __AALKERNEL_AALRESOURCEMGR_SERVER_AALRMS_EVENTS_INT_H__
#define __AALKERNEL_AALRESOURCEMGR_SERVER_AALRMS_EVENTS_INT_H__
#include "aalsdk/kernel/aalbus.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      RESOURCE MANAGER SERVICE EVENTS     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: devreq_cmplt
// Description: Device Request Complete
//=============================================================================
struct rms_event_devreq_cmplt {
#define qi_to_devreq_cmplt(pqi) (container_of( pqi, struct rms_event_devreq_cmplt, m_qitem ) )
   //-------------------------------------------
   // Including the macro effectively causes
   // this object to be derived from aal_q_item
   //-------------------------------------------
   _DECLARE_AALQ_TYPE;
   rms_result_e             result_code;  // Result code [IN/OUT]
   stTransactionID_t        tranID;       // transaction ID to identify result [IN]
   void                    *context;      // optional token [IN]
};

//=============================================================================
// Name: rms_event_devreq_cmplt_create
// Description: Constructor
//=============================================================================
static inline struct rms_event_devreq_cmplt *
               rms_event_devreq_cmplt_create( rms_result_e           result_code,
                                              stTransactionID_t      tranID,
                                              void                  *context)
{
   struct rms_event_devreq_cmplt * This =
                        ( struct rms_event_devreq_cmplt * )kosal_kmalloc(sizeof(struct rms_event_devreq_cmplt));
   if(This == NULL){
      return NULL;
   }

   This->result_code = result_code;
   This->tranID = tranID;
   This->context = context;

   AALQ_QID(This) = rspid_RM_DeviceRequest;
   AALQ_QLEN(This) = 0; // No payload

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: rms_event_devreq_cmplt_destroy
// Description: Destructor
//=============================================================================
static inline void rms_event_devreq_cmplt_destroy(struct rms_event_devreq_cmplt *This)
{
   kfree(This);
}


#endif // __AALKERNEL_AALRESOURCEMGR_SERVER_AALRMS_EVENTS_INT_H__
