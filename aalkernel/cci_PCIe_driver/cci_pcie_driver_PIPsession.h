//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2016, Intel Corporation.
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
//  Copyright(c) 2012-2016, Intel Corporation.
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
//        FILE: cci_PIPsession.h
//     CREATED: 07/28/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE: Internal private definitions and constants for the CCCIV4 PIP Session.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALKERNEL_CCI_PIP_SESSION_H__
#define __AALKERNEL_CCI_PIP_SESSION_H__
#include "cci_pcie_driver_internal.h"

//=============================================================================
// Name: cci_PIPsession
// Description: Object that represents an instance of a session between an
//              owner of a device and the device itself. It holds state such
//              as the task list and PIP interface. The PIP interface
//              holds the owner session context and the PIP function interfaces.
// Comments: This object is specific to the PIP. The ownerSession contains
//           the generic session context shared between the PIP and the
//           AAL kernel services.
//=============================================================================
struct cci_PIPsession {

   // PIP contains all of the interfaces we use for communications
   struct aal_device               *paaldev;
   struct cci_aal_device           *pCCIdev;

   // Current transaction
   stTransactionID_t              currTranID;

   // Owner Session hold shared session instance information
   struct aaldev_ownerSession    *pownerSess;
   kosal_semaphore                session_sem;

};
#define cci_PIPsessionp_to_ccidev(s)              ( (s)->pCCIdev )
#define cci_PIPsessionp_to_aal_afup(s)            ((s)->paaldev)
#define cci_PIPsessionp_to_ownerSession(s)        ((s)->pownerSess)
#define cci_PIPsessionp_semaphore(s)              (&(s)->session_sem)

//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
struct cci_PIPsession *session_create(struct aaldev_ownerSession *pownerSess);
int BindSession(struct aaldev_ownerSession *pownerSess);
int session_destroy(struct cci_PIPsession *sess);
int UnbindSession(struct aaldev_ownerSession *pownerSess);
void cci_flush_all_wsids( struct cci_PIPsession *);

#endif // __AALKERNEL_CCIV4_PIP_SESSION_H__

