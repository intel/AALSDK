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
//        FILE: cciv4_PIPsession.c
//     CREATED: 07/28/2015
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          CCIv4 PIP Session.
// HISTORY:
// COMMENTS: The CCIv4 PIP Session is an an object which maintains state and
//           context between an CCIv4 AAL Device and a user mode application
//           that has been bound to it.
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIV4_DBG_MOD

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/spl2defs.h"

#include "cciv4_PIPsession.h"
#include "cciv4_driver_internal.h"

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
struct cciv4_PIPsession *
session_create(struct aaldev_ownerSession *pownerSess)
{
   struct cciv4_PIPsession *pSess;

   // Allocate the session object
   pSess = (struct cciv4_PIPsession *)kosal_kzmalloc(sizeof(struct cciv4_PIPsession));
   ASSERT(NULL != pSess);
   if( NULL == pSess ){
      return NULL;
   }

   // Initialize the session
   cciv4_PIPsessionp_to_ownerSession(pSess)  = pownerSess;
   cciv4_PIPsessionp_to_aal_afup(pSess)      = aalsess_aaldevicep(pownerSess);

   kosal_mutex_init(cciv4_PIPsessionp_semaphore(pSess));


   // Save the device for this AFU. (Saved when the AFU was created)
   cciv4_PIPsessionp_to_cciv4dev(pSess) = aaldev_pip_context_to_obj(struct cciv4_device *, pSess->paaldev);
   //pSess->pCCIV4dev = aaldev_pip_context_to_obj(struct cciv4_device *, pSess->paaldev);
   PDEBUG("CCIv4 PIP Session Created.\n");

   return pSess;
}

//=============================================================================
// Name: BindSession
// Description: Bind the application session to the PIP
// Inputs: powerSession - AAL Object used to maintain context between the PIP
//                        and its owning process.
// Returns: success = 1
//=============================================================================
int
BindSession(struct aaldev_ownerSession *pownerSess)
{
   struct aal_device       *paaldev = NULL;
   struct cciv4_device     *pdev    = NULL;
   struct cciv4_PIPsession *pSess   = NULL;

   PVERBOSE("Binding UI Session with CCIv4 Device\n");

   // Get the AAL device
   paaldev = aalsess_aaldevicep(pownerSess);

   // Make sure there is a device to bind to
   if ( unlikely( NULL == paaldev ) ) {
      PDEBUG( "No device!\n" );
      return -ENODEV;
   }

   // Get the device from the AAL device's PIP context (setup when device was created)
   pdev = aaldev_pip_context_to_obj(struct cciv4_device*, paaldev);

   // Create a PIP session and save it.
   pSess = session_create( pownerSess );
   if( unlikely( NULL == pSess ) ){
      PDEBUG( "Create session failed.\n" );
      return -ENODEV;
   }

   // Return the session as a handle. This is used later in PIP Command Handler
   pownerSess->m_PIPHandle = pSess;

   // Save the session in the device
   cciv4_dev_to_PIPsessionp(pdev) = pSess;

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
int session_destroy(struct cciv4_PIPsession *pSess)
{
   PDEBUG( "Destroying CCIv4 PIP Session.\n");

   // Final free
   kosal_kfree(pSess, sizeof(struct cciv4_PIPsession) );
   return 0;
}

//=============================================================================
// Name: UnbindSession
// Description:Called when the user process unbinds from the device
// Interface: public
// Inputs: Pointer to the ownerSession attributes
// Returns: success = 1
// Comments:  Host AFU need  not do anything
//=============================================================================
int UnbindSession(struct aaldev_ownerSession *pownerSess)
{
   struct cciv4_PIPsession *pSess = (struct cciv4_PIPsession *)pownerSess->m_PIPHandle;
   struct cciv4_device     *pdev  = cciv4_PIPsessionp_currContext(pSess);

   PDEBUG("UnBinding UI Session\n");

   // Stop all on-going processing
   kosal_sem_get_krnl( cciv4_dev_psem(pdev) );
   // TODO STOP STUFF HERE
   kosal_sem_put( cciv4_dev_psem(pdev) );

   // Free all allocated workspaces not yet freed
   cciv4_flush_all_wsids(pSess);

   session_destroy(pSess);

   return 1;
}


