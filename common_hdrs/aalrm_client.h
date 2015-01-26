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
//        FILE: aalrm-client.h
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains public definitions for the
//          AAL Resource Manager Client Service Module Interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/20/08       JG       Initial version created
// 11/10/08       JG       Renamed aalrm_client to separate out rmc specifics
//                            from common definitions
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALRM_CLIENT_H__
#define __AALSDK_KERNEL_AALRM_CLIENT_H__
#include <aalsdk/kernel/aalrm.h>

#define RESMGR_DEV_NAME "aalresmgr"

BEGIN_C_DECLS


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     RESOURCE MANAGER CLIENT INTERFACE    ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Message headers - These headers are used by the user space clients
//=============================================================================


//=============================================================================
// Name: req_allocdev
// Description: Request a device allocation using a manifest
//=============================================================================
// Request device
struct req_allocdev
{
   stTransactionID_t  tranID;   // transaction ID to identify result [IN]
   void              *context;  // optional token [IN]v
   size_t             size;     // size of payload [IN]
   btByte             buf;      // first char of variably length payload [IN]
};

#define ALLOC_DEVHDRSZ  ( offsetof(struct req_allocdev, buf) )

//=============================================================================
// Name: req_registrar
// Description: Send a request to the registrar
//=============================================================================
// Request device
struct req_registrar
{
   stTransactionID_t  tranID;   // transaction ID to identify result [IN]
   void              *context;  // optional token [IN]
   size_t             size;     // size of payload [IN]
   btByte             buf;      // first char of variably length payload [IN]
};
#define REGISTRAR_REQ_HDRSZ  ( offsetof(struct req_registrar, buf) )

//=============================================================================
// Name: rsp_device
// Description: Response to a device specific request
//=============================================================================
struct rsp_device
{
   btInt              result;    // result code [OUT]
   void              *devHandle; // device handle. NULL if no device [OUT]
   size_t             size;      // size of payload [IN]
   btByte             buf;       // first char of variably length payload [IN]
};
#define REQDEVICE_RSP_HDRSZ  ( offsetof(struct rsp_device, buf) )

//=============================================================================
// Name: req_reldev
// Description: Request to release a device allocated via an alloc
// COMMENTS:
//=============================================================================
struct req_reldev
{
   void  *tranID;       // transaction ID to identify result [IN/OUT]
   void  *data;         // optional payload [IN/OUT]
   btInt  result;       // result code [OUT]
   void  *devHandle;    // device handle [IN]
};


END_C_DECLS

#endif // __AALSDK_KERNEL_AALRM_CLIENT_H__
