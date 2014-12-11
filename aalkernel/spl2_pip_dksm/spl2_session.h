//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2014, Intel Corporation.
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
//  Copyright(c) 2012-2014, Intel Corporation.
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
//        FILE: spl2_session.h
//     CREATED: 02/17/2012
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE: Internal private definitions and constants for the Intel(R)
//          Intel QuickAssist Technology SPL2 device Session.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_SPL2_SESSION_H__
#define __AALKERNEL_SPL2_PIP_DKSM_SPL2_SESSION_H__
#include "spl2_pip_internal.h"

//=============================================================================
// Name: spl2_session
// Description: Object that represents an instance of a session between an
//              owner of a device and the device itself. It holds state such
//              as the task list and PIP interface. The PIP interface
//              holds the owner session context and the PIP function interfaces.
// Comments: This object is specific to the PIP. The ownerSession contains
//           the generic session context shared between the PIP and the
//           AAL kernel services.
//=============================================================================
#define SPL2_SESS_STATE_NONE        0
#define SPL2_SESS_STATE_BEGIN       1
#define SPL2_SESS_STATE_DSMs_SET    2
#define SPL2_SESS_STATE_CTXT_ACTIVE 3
struct spl2_session {

   // PIP contains all of the interfaces we use for communications
   struct aal_device             *paaldev;
   struct spl2_device            *pspl2dev;

   // Current transaction
   stTransactionID_t              currTranID;
   void                          *currContext;
   unsigned                       pollrate;
   void                          *messageContext;
   unsigned                       sessstate;

   // Owner Session hold shared session instance information
   struct aaldev_ownerSession    *pownerSess;
   struct semaphore               session_sem;

};
#define spl2_sessionp_to_spl2dev(s)             ((s)->pspl2dev)
#define spl2_sessionp_to_aal_afup(s)            ((s)->paaldev)
#define spl2_sessionp_to_ownerSession(s)        ((s)->pownerSess)
#define spl2_sessionp_is_tranowner(s)           ((s) == spl2_dev_activesess(s->pspl2dev))
#define spl2_sessionp_set_tranowner(s)          (spl2_dev_activesess(s->pspl2dev) = s)
#define spl2_sessionp_clear_tranowner(s)        (spl2_dev_activesess(s->pspl2dev) = NULL)
#define spl2_sessionp_semaphore(s)              (&(s)->session_sem)
#define spl2_sessionp_currTran(s)               ((s)->currTranID)
#define spl2_sessionp_currContext(s)            ((s)->currContext)
#define spl2_sessionp_currMsgContext(s)         ((s)->messageContext)
#define spl2_sessionp_currPollrate(s)           ((s)->pollrate)
#define spl2_sessionp_currState(s)              ((s)->sessstate)
#define spl2_sessionp_clearTran(s)              (memset(&(s)->currTranID,0,sizeof(stTransactionID_t) ))

//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
struct spl2_session *session_create(struct aaldev_ownerSession *pownerSess);
int AFUbindSession(struct aaldev_ownerSession *pownerSess);
int session_destroy(struct spl2_session *sess);
int AFUunbindSession(struct aaldev_ownerSession *pownerSess);
void flush_all_wsids( struct spl2_session *);

#endif // __AALKERNEL_SPL2_PIP_DKSM_SPL2_SESSION_H__

