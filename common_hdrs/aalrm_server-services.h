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
//        FILE: aalrm_server-services.h
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains public definitions for the
//          AAL Resource Manager Kernel Module Service Interfaces
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/20/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 02/09/2009     JG       Added support for RMSS cancel transaction
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALRM_SERVER_SERVICES_H__
#define __AALSDK_KERNEL_AALRM_SERVER_SERVICES_H__
#include <aalsdk/kernel/aalbus.h>
#include <aalsdk/kernel/aalrm_client.h>
#include <aalsdk/kernel/AALTransactionID_s.h>
#include <aalsdk/kernel/aalinterface.h>

//-----------------------------------------------------------------------------
// Public Interface
//-----------------------------------------------------------------------------
#define  AALRMS_API_MAJVERSION     (0x00000001)
#define  AALRMS_API_MINVERSION     (0x00000000)
#define  AALRMS_API_RELEASE        (0x00000000)

#define  AALRMS_API_INTC           (0x0000000000002000)

// API IIDs TODO should come from aal ids
#define AAL_RMSAPI_IID_01         (0x2)


//=============================================================================
//=============================================================================
//                 Resource Manager Server Service interface
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrms_req_tranID
// Description:
//=============================================================================
struct aalrms_req_tranID{
   void                             *m_context;
};


//=============================================================================
// Name: aalrms_reqdev_cmplt_t
// Description: Request device completion callback and completion object.
//              Defines the callback signature for the completion function
//              called when request_device completes.
// Inputs: preqdev_cmplt - pointer to completion object
//         tranID - Transaction ID for the request
//=============================================================================
typedef void (*aalrms_reqdev_cmplt_t)( int result,
                                       struct rsp_device   *pretdev,
                                       struct req_allocdev *origreq,
                                       struct aalrms_req_tranID tranID );

//=============================================================================
// Name: aalrms_registrarreq_cmplt_t
// Description: Request completion callback and completion object.
//              Defines the callback signature for the completion function
//              called when request_device completes.
// Inputs: preqdev_cmplt - pointer to completion object
//         tranID - Transaction ID for the request
//=============================================================================
typedef void (*aalrms_registrarreq_cmplt_t)( int result,
                                             struct req_registrar *resp,
                                             struct req_registrar *origreq,
                                             struct aalrms_req_tranID tranID  );



//=============================================================================
// Name: aalrm_server_cmplt_t
// Description: General purpose callback function signature
//=============================================================================
typedef void (*aalrm_server_cmplt_t)( int errno,
                                      struct req_registrar *resp,
                                      struct req_registrar *origreq,
                                      struct aalrms_req_tranID tranID  );


//=============================================================================
// Name: aalrm_server_service
// Description: Service interface for the AAL RMS service. This
//              interface defines the methods used by clients of the RMS
//              service.
//=============================================================================
struct aalrm_server_service{

   //==========================================================================
   // Name: request_device
   // Description: Request a device be allocated by RMS
   // Inputs: req_allocdev - request with manifest
   //         tranID - Transaction ID for the request
   //==========================================================================
   int (*request_device)( struct req_allocdev* req,
                          aalrms_reqdev_cmplt_t completionfcn,
                          struct aalrms_req_tranID tranID );

   //==========================================================================
   // Name: registrar_request
   // Description: Send a request to Registrar
   // Inputs: req_allocdev - request with manifest
   //         pSession - owner
   //         tranID - Transaction ID for the request
   //==========================================================================
   int (*registrar_request)(  struct req_registrar *preq,
                              aalrms_registrarreq_cmplt_t completionfcn,
                              struct aalrms_req_tranID tranID  );

   //==========================================================================
   // Name: cancel_all_requests
   // Description: Cancel all requests that match transaction ID context
   // Inputs: tranID - Transaction ID for the request(s) to cancel
   //==========================================================================
   void (*cancel_all_requests)(  struct aalrms_req_tranID *tranID  );
};


// Convenient macros for using the interface
#define i_rmserver(i) (*((struct aalrm_server_service*)((struct aal_interface *)i)->m_iptr))
#define rms_reqdev(i) ((struct aalrm_server_service*)((struct aal_interface *)i)->m_iptr)->request_device
#define rms_regdev(i) ((struct aalrm_server_service*)((struct aal_interface *)i)->m_iptr)->registrar_request
#define rms_reldev(i) ((struct aalrm_server_service*)((struct aal_interface *)i)->m_iptr)->release_device
#define rms_cancelreqs(i) ((struct aalrm_server_service*)((struct aal_interface *)i)->m_iptr)->cancel_all_requests

#endif // __AALSDK_KERNEL_AALRM_SERVER_SERVICES_H__

