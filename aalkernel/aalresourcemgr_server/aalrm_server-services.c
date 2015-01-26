//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2015, Intel Corporation.
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
//  Copyright(c) 2008-2015, Intel Corporation.
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
//        FILE: aalrm_server-services.c
//     CREATED: 02/29/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE:  Implementation of RMS kernel services
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-29-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/23/2009     JG       Initial code cleanup
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMS_DBG_MOD

#include "aalsdk/kernel/aalrm_server.h"
#include "aalsdk/kernel/aalrm_server-services.h"
#include "aalrm_server-int.h"

//
// Protoypes
void aalrms_queue_req(struct aal_q_item * pqitem);
struct aal_q_item * aalrms_dequeue_req(void);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////       RESOURCE MANAGER CLIENT EVENTS     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// The following methods implement the RMS Service interface. These functions
// are not called directly but rather through the the virtual RMSS interface
// that is registered with the AALBus Service Interface Broker
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//=============================================================================
// Name: aalrms_request_device
// Description: builds and queues an allocate request
// Interface: public
// Inputs: preqdev - pointer to request object
//         tranID - transaction ID
// Outputs: none.
// Comments:
//=============================================================================
int aalrms_request_device(struct req_allocdev     *preqdev,
                          aalrms_reqdev_cmplt_t    completionfcn,
                          struct aalrms_req_tranID tranID )
{
   //----------------------------
   // Create queue request object
   //----------------------------
   struct rms_reqq_reqdev *newreq = rms_reqq_reqdev_create(preqdev,
                                                           completionfcn,
                                                           tranID);
   if ( NULL == newreq ) {
      return -ENOMEM;
   }

   // Queue the item
   aalrms_queue_req(&RMSSQI(newreq));
   return 0;
}

//=============================================================================
// Name: aalrms_registrar_request
// Description: Created and queues a request for the Registrar
// Interface: public
// Inputs: preqdev - pointer to request object
//         tranID - Transaction ID for the request
// Outputs: none.
// Comments:
//=============================================================================
int aalrms_registrar_request(struct req_registrar       *preqdev,
                             aalrms_registrarreq_cmplt_t completionfcn,
                             struct aalrms_req_tranID    tranID)
{

   //----------------------------
   // Create queue request object
   //----------------------------
   struct rms_reqq_registrar *newreq = rms_reqq_registrar_create(preqdev,
                                                                 completionfcn,
                                                                 tranID);
   if ( NULL == newreq ) {
      return -ENOMEM;
   }

   // Queue the item
   aalrms_queue_req(&RMSSQI(newreq));
   return 0;
}

//=============================================================================
// Name: cancel_all_requests
// Description: Cancel all requests that match transaction ID context
// Inputs: tranID - Transaction ID for the request(s) to cancel
// Outputs: none.
// Comments:
//=============================================================================
//-----------------------------------------------------------------------------
int aalrms_process_cancel_request(struct aal_q_item *pqitem,
                                  aal_queue_t *pqueue,
                                  void *pcontext)
{
   struct rms_reqq_reqdev     *preqdev;   // "Request Device"
   struct rms_reqq_registrar  *pregreq;   // "Registrar Request"

   struct aalrms_req_tranID *tranID = (struct aalrms_req_tranID *)pcontext;



   // Get the RMSS queue item from the AAL qitem (downcast)
   struct rms_reqq_item *rms_qitem = qi_to_rmsqi(pqitem);
   DPRINTF(AALRMS_DBG_IOCTL, ": TranID context  callback %p %p \n",tranID->m_context,rms_qitem->m_tranid.m_context );

   // Is this a request to cancel?
   if( tranID->m_context == rms_qitem->m_tranid.m_context ){
      DPRINTF(AALRMS_DBG_IOCTL, ": Cancelling callback %p %p \n",pqitem,pqueue );

      // Remove it
      aalrms_queue_remove(pqueue,pqitem);

      //---------------------------------------------
      // Each message is processed differently
      //  Call the completion
      //---------------------------------------------
      // TODO Would be nice to make this normalized like  AAL user mode
      switch (pqitem->m_id){
         case reqid_RS_Registrar:
         case rspid_RS_Registrar: {
            DPRINTF(AALRMS_DBG_IOCTL, ": rspid_RS_Registrar\n" );
            // Get the original request from queue item
            pregreq = qi_to_regreq(pqitem);
            CALL_COMPLETION(pregreq)(rms_resultCancelled,
                                     NULL,
                                     REGREQ_REGREQP(pregreq),
                                     RMSSQ_TRANID(pregreq));
            //Destroy original request message
            rms_reqq_regreq_destroy(pregreq);

            break;
         }

         case reqid_URMS_RequestDevice:
         case rspid_URMS_RequestDevice: {
            DPRINTF(AALRMS_DBG_IOCTL, ": URMS_RequestDevice\n" );
            // Get the original request from queue item
            preqdev = qi_to_reqdev(pqitem);

            // Send the response event back
            CALL_COMPLETION( preqdev)(rms_resultCancelled,
                                      NULL,
                                      REQDEV_ALLOCDEVP(preqdev),
                                      RMSSQ_TRANID(preqdev));
            // Done so destroy the request
            rms_reqq_reqdev_destroy(preqdev);
         }
         break;
         default:
            DPRINTF(AALRMS_DBG_IOCTL, ": Cancelling unknown type 0x%" PRIx64 "\n", pqitem->m_id);
            break;
      } // switch (pqitem->m_id)
   } // if( tranID->m_context == rms_qitem->m_tranid.m_context )

   return 0;
}

//=============================================================================
// Name: cancel_all_requests
// Description: Cancel all requests that match transaction ID context
// Inputs: tranID - Transaction ID for the request(s) to cancel
// Outputs: none.
// Comments: For each entry in the queue the callback
//           aalrms_process_cancel_request is called.
//=============================================================================
void aalrms_cancel_all_requests(  struct aalrms_req_tranID *tranID  )
{

   if (kosal_sem_get_krnl_alertable(&rmserver.m_sem)) { /* FIXME */ }
   DPRINTF(AALRMS_DBG_IOCTL, ": Cancelling all\n");

   aalrms_queue_doForAllQueuedItems( aalrms_process_cancel_request,
                                     tranID);

   kosal_sem_put(&rmserver.m_sem);
}

