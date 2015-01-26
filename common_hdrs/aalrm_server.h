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
//        FILE: aalrm_server.h
//     CREATED: 02/25/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file containe public definitions for the
//          AAL Resource Manager Server Kernel Module Interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/25/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALRM_SERVER_H__
#define __AALSDK_KERNEL_AALRM_SERVER_H__
#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/aalrm.h>
#include <aalsdk/kernel/aaldevice.h>

#define RMS_DEV_NAME "aalrms"

BEGIN_C_DECLS


//=============================================================================
//=============================================================================
//                  Resource Manager Service Messages
//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
// Messages structures -
//       Type -  Request - indicates that the message is requesting a service
//              Response - indicates that the message is in response to a
//                         service request
//                 Event - indicates that the messages is informational
//        DIR -  IN - indicates that the message is sent into the target.
//               OUT - indicates that the message is received from the target
//     Object - The destination [IN] or source [OUT] of the message
// Command ID - Constant value placed in the id field
//-----------------------------------------------------------------------------

//=============================================================================
// Name: Request device
// Type[Dir]: Request [IN]
// Object: User Resource Manager Service
// Command ID: reqid_URMS_RequestDevice
// Input:    Payload - aalrms_requestdevice
//        req_handle - Request handle. Returned in response
// Description: Request a device
//=============================================================================
struct aalrms_requestdevice
{
      btPID   pid;        // ID of the process requesting the device
      size_t  size;       // Size of this payload
      void   *manifest;   // Additional information
};


//=============================================================================
// Name: Request device response
// Type[Dir]: Response [OUT]
// Object: User Resource Manager Service
// Command ID: rspid_URMS_RequestDevice
// Input: res_handle - Resource handle
//        req_handle - handle of original reqid_URMS_RequestDevice
// Description: Response to a reqid_URMS_RequestDevice
//=============================================================================
//none



//=============================================================================
// Name: Registrar Request
// Type[Dir]: Request [IN]
// Object: Registrar Service
// Command ID: reqid_RS_Registrar
// Input:    Payload - Registrar request block
//        req_handle - Handle of request used in response
// Description: Registrar  request
//=============================================================================
//none



//=============================================================================
// Name: Registrar Request response
// Type[Dir]: Response [OUT]
// Object: User Resource Manager Service
// Command ID: rspid_URMS_RequestDevice
// Input:    Payload - Response payload
//        req_handle - handle of original reqid_RS_Registrar
// Description: Response to a reqid_RS_Registrar
//=============================================================================
//none



//=============================================================================
// Name: Set Configuration Update Events
// Type[Dir]: Request [IN]
// Object: Kernel Resource Manager Service
// Command ID: reqid_KRMS_SetConfigUpdates
// Input: data - Flags enabling mode (See KRMS_UPDATE_MODE_xxxx)
// Description: Enable/Disable config update events
//=============================================================================
//none
#define KRMS_UPDATE_MODE_EVENTS  (1 << 0)
#define KRMS_UPDATE_MODE_FILE    (1 << 1)

#define KRMS_UPDATE_MODE_NONE    (0)



//=============================================================================
// Name: Service shutdown request
// Type[Dir]: Request [IN]
// Object: Kernel Resource Manager Service
// Command ID: reqid_Shutdown
// Input:  none.
// Description: Signal that the service should shutdown. Causes the kernel
//              services to quiesce with respect to the user mode service.
//              Causes the message queues to be flushed.
//=============================================================================
//none



//=============================================================================
// Name: Service shutdown response
// Type[Dir]: Response [OUT]
// Object: Kernel Resource Manager Service
// Command ID: rspid_Shutdown
// Input: none.
// Description: Final event from the message delivery queue. Messages will no
//              longer be delivered from the service and all subsequent
//              requests to the KRMS will fail.
//=============================================================================
//none


//=============================================================================
// Name: Service restart request
// Type[Dir]: Request [IN]
// Object: Kernel Resource Manager Service
// Command ID: reqid_Restart
// Input:  none.
// Description: Request that the KRMS restart
//=============================================================================
//none


//=============================================================================
// Name: Service restart response
// Type[Dir]: Response [OUT]
// Object: Kernel Resource Manager Service
// Command ID: rspid_Started
// Input: none.
// Description: System restarted.  System is in the same state as an new open()
//=============================================================================
//none

END_C_DECLS

#endif // __AALSDK_KERNEL_AALRM_SERVER_H__

