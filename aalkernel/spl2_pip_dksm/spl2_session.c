//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
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
//  Copyright(c) 2012-2015, Intel Corporation.
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
//        FILE: spl2_mafu_pip.c
//     CREATED: 02/17/2012
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          SPL2 device Session.
// HISTORY:
// COMMENTS: TheSPL2 Device Session is an an object which maintains state and
//           context between an SPL2 aal_device and a user mode application
//           that has bound to it.
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_DEV

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/spl2defs.h"
#include "spl2mem-kern.h"
#include "spl2_session.h"
#include "aalsdk/kernel/aalmafu-events.h"


//=============================================================================
// Name: session_create
// Description: Creates a private session object
//              The session holds PIP context specific to the device and its
//              owner.
// Inputs: pownerSession - AAL Object used to maintain context between the PIP
//                        and its owning process.
// Outputs: encoder_session
// Comments:
//=============================================================================
struct spl2_session *
session_create(struct aaldev_ownerSession *pownerSess)
{
   struct spl2_session *sessp;

   // Allocate the session object
   sessp = kmalloc(sizeof(struct spl2_session), GFP_KERNEL);
   if ( NULL == sessp ){
      return NULL;
   }
   memset(sessp, 0, sizeof(struct spl2_session));

   // Initialize the session
   spl2_sessionp_to_ownerSession(sessp) = pownerSess;
   spl2_sessionp_to_aal_afup(sessp) = aalsess_aaldevicep(pownerSess);

   kosal_mutex_init(spl2_sessionp_semaphore(sessp));


   // Save the device for this AFU. (Saved when the AFU was created)
   spl2_sessionp_to_spl2dev(sessp) = aaldev_pip_context_to_obj(struct spl2_device *, sessp->paaldev);

   PDEBUG("SPL2 Created Session.\n");

   return sessp;
}

//=============================================================================
// Name: AFUbindSession
// Description: Bind the application session to the PIP
// Inputs: powerSession - AAL Object used to maintain context between the PIP
//                        and its owning process.
// Returns: success = 1
//=============================================================================
int
AFUbindSession(struct aaldev_ownerSession *pownerSess)
{
   struct aal_device   *paaldev = NULL;
   struct spl2_device  *pdev    = NULL;
   struct spl2_session *sessp   = NULL;

   PDEBUG("Binding UI Session with SPL2 Simulated MAFU PIP %p Context %p\n",
             pownerSess->m_device, pownerSess->m_device->m_pipContext);

   // Get the AAL device
   paaldev = aalsess_aaldevicep(pownerSess);

   // Make sure there is a device to bind to
   if ( unlikely( NULL == paaldev ) ) {
      PDEBUG( "No device!\n" );
      return -ENODEV;
   }

   // Get the device from the AAL device's PIP context (setup when device was created)
   pdev = aaldev_pip_context_to_obj(struct spl2_device*, paaldev);

   // Create a PIP session and save it.  The session uses the ENCODER device
   sessp = session_create( pownerSess );
   if( unlikely( NULL == sessp ) ){
      PDEBUG( "Create session failed.\n" );
      return -ENODEV;
   }

   // Return the session as a handle. This is used later in AFUcommand
   pownerSess->m_PIPHandle = sessp;

   // Save the session in the device
   spl2_dev_to_simsession(pdev) = sessp;

   return 1;
}

//=============================================================================
// Name: session_destroy
// Description: Destroy the private session
// Interface: public
// Inputs: .
// Outputs: none.
// Comments:
//=============================================================================
int session_destroy(struct spl2_session *sess)
{
   PDEBUG( "Destroying SPL2 Session; %p.\n", sess );

   // Do any cleanup like unblocking any transactions

   // Final free
   kfree(sess);
   return 0;
}

//=============================================================================
// Name: AFUunbindSession
// Description:Called when the user process unbinds from the device
// Interface: public
// Inputs: Pointer to the ownerSession attributes
// Returns: success = 1
// Comments:  Host AFU need  not do anything
//=============================================================================
int AFUunbindSession(struct aaldev_ownerSession *pownerSess)
{
   struct spl2_session *sessp = (struct spl2_session *)pownerSess->m_PIPHandle;
   struct spl2_device  *pdev  = spl2_sessionp_to_spl2dev(sessp);

   PDEBUG("UnBinding UI Session\n");

   // Stop all on-going processing
   down( spl2_dev_semp(pdev) );

   if ( spl2_dev_activesess(pdev) ) {
      spl2_sessionp_currState(spl2_dev_activesess(pdev)) = SPL2_SESS_STATE_NONE;
   }

   up( spl2_dev_semp(pdev) );

   // Wait for the task poller to finish, if it was active.
   down( spl2_dev_tran_done_semp(pdev) );

   spl2_dev_activesess(pdev)    = NULL;
   spl2_dev_to_simsession(pdev) = NULL;

   up( spl2_dev_tran_done_semp(pdev) );

   // Free all allocated workspaces not yet freed
   flush_all_wsids(sessp);

   session_destroy(sessp);

   return 1;
}


