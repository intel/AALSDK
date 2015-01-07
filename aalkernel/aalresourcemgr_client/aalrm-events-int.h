//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2009-2015, Intel Corporation.
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
//  Copyright(c) 2009-2015, Intel Corporation.
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
//        FILE: aalrm-events-int.h
//     CREATED: 01/08/2009
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains internal event definitions for the
//          AAL Resource Manager Client Service Module.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 01/08/09       JG       Initial version created
// 05/14/10       JG       Moved event methods into here
//****************************************************************************
#ifndef __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_EVENTS_INT_H__
#define __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_EVENTS_INT_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalrm_client.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////       RESOURCE MANAGER CLIENT EVENTS     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
// Name: reqdev_cmplt
// Description: Request Device Complete
//=============================================================================
struct reqdev_cmplt {
#define qi_to_reqdev_cmplt(pqi) (container_of( pqi, struct reqdev_cmplt, m_qitem ) )
   //-------------------------------------------
   // Including the macro effectively causes
   // this object to be derived from aal_q_item
   //-------------------------------------------
   _DECLARE_AALQ_TYPE;

   stTransactionID_t        tranID;    // transaction ID to identify result [IN]
   btAny 					context;   // optional token [IN]
   struct rsp_device        retdev;    // Return information [IN]

};

//=============================================================================
// Name: registrarreq_cmplt
// Description: Registrar Request Complete
//=============================================================================
struct registrarreq_cmplt {
#define qi_to_registrarreq_cmplt(pqi) (container_of( pqi, struct registrarreq_cmplt, m_qitem ) )
   //-------------------------------------------
   // Including the macro effectively causes
   // this object to be derived from aal_q_item
   //-------------------------------------------
   _DECLARE_AALQ_TYPE;
   btInt                     errno;    // Error code
   struct req_registrar     *resp;     // Return information
};


//=============================================================================
// Name: shutdownreq_cmplt
// Description: Shutdown Request Complete
//=============================================================================
struct shutdownreq_cmplt {
#define qi_to_shutdownreq_cmplt(pqi) (container_of( pqi, struct shutdownreq_cmplt, m_qitem ) )
   //-------------------------------------------
   // Including the macro effectively causes
   // this object to be derived from aal_q_item
   //-------------------------------------------
   _DECLARE_AALQ_TYPE;
   int                      errno;      // Error code
};



//=============================================================================
// Name: reqdev_cmplt_create
// Description: Request Device Event Constructor
//=============================================================================
static inline struct reqdev_cmplt *
reqdev_cmplt_create( struct rsp_device   *pretdev,
                     struct req_allocdev *origreq)
{
   // Allocate event with room for payload.
   struct reqdev_cmplt * This = (struct reqdev_cmplt *)kosal_kmalloc( sizeof(struct reqdev_cmplt) +
                                         	 	 	 	 	 	 	  pretdev->size);
   if(This == NULL){
      return NULL;
   }

   // Copy the rsp_device payload
   memcpy(&This->retdev, pretdev, pretdev->size + sizeof(struct rsp_device));

   // Carry forward request data
   This->tranID = origreq->tranID;
   This->context = origreq->context;

   AALQ_QID(This) = rspid_URMS_RequestDevice;
   AALQ_QLEN(This) = pretdev->size;    // Length of payload

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: reqdev_cmplt_destroy
// Description: Destructor
//=============================================================================
static inline void reqdev_cmplt_destroy(struct reqdev_cmplt *This)
{
   kfree(This);
}


//=============================================================================
// Name: registrar_cmplt_create
// Description: Constructor
//=============================================================================
static inline struct registrarreq_cmplt *
                          registrar_cmplt_create(int errno,
                                                 struct req_registrar *resp)
{
   struct registrarreq_cmplt * This = kmalloc(sizeof(struct registrarreq_cmplt),GFP_KERNEL);
   if(This == NULL){
      return NULL;
   }

   This->errno = errno;
   This->resp = resp;

   AALQ_QID(This) = rspid_RS_Registrar;
   AALQ_QLEN(This) = resp->size;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: registrar_cmplt_destroy
// Description: Destructor
//=============================================================================
static inline void registrar_cmplt_destroy(struct registrarreq_cmplt *This)
{

   kfree(This);
}


//=============================================================================
// Name: shutdown_cmplt_create
// Description: Constructor
//=============================================================================
static inline struct shutdownreq_cmplt *
                          shutdown_cmplt_create(int errno)
{
   struct shutdownreq_cmplt * This = kmalloc(sizeof(struct shutdownreq_cmplt),GFP_KERNEL);
   if(This == NULL){
      return NULL;
   }

   This->errno = errno;

   AALQ_QID(This) = rspid_Shutdown;
   AALQ_QLEN(This) = 0;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}


//=============================================================================
// Name: shutdown_cmplt_destroy
// Description: Destructor
//=============================================================================
static inline void shutdown_cmplt_destroy(struct shutdownreq_cmplt *This)
{
   kfree(This);
}


#endif // __AALKERNEL_AALRESOURCEMGR_CLIENT_AALRM_EVENTS_INT_H__
