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
//        FILE: aalbus_events.h
//     CREATED: 02/26/2015
//      AUTHOR: Joseph Grecco - Intel Corporation
//
// PURPOSE: This file contains definitions for the internal AAL Bus Service 
//          Events.
// HISTORY:
// COMMENTS: AAL Bus Service Events are kernel mode only. I.e., they flow
//           between kernel modules.  Events that appear in User Mode are
//           translated as they are marshalled up.  User mode representation
//           is defined with the user mode interface.
// SCOPE: Linux and Windows
// WHEN:          WHO:     WHAT:
// 02/26/15       JG       Initial version created based off 
//                          aalrms_server-int.h (DEPRECTATED)
//****************************************************************************
#ifndef __AALKERNEL_AALBUS_EVENTS_H__
#define __AALKERNEL_AALBUS_EVENTS_H__
#include <aalsdk/kernel/kosal.h>                   // KOSAL functions
#include <aalsdk/kernel/aaldevice.h>               // Device Attributes
#include <aalsdk/kernel/aalqueue.h>                // For queue functions
#include "aalsdk/kernel/aalbus_iconfigmonitor.h"   // For config_update event

BEGIN_NAMESPACE( AAL )

//----------------------------
// Casting and accessor macros
//----------------------------
#define AALBUSEVTQI(p)       (p->m_qitem)
#define AALBUSEVTQIP(p)      (&(p->m_qitem))


//=============================================================================
// Name: aalbus_config_update_event
// Description: Notification that the device resource configuration has 
//              changed.  E.g., New device added/removed or owner changed.
//=============================================================================
struct aalbus_config_update_event
{
#define qip_to_updateevent(pqi) (kosal_container_of( pqi, struct aalbus_config_update_event, m_qitem ) )
#define CONF_EVTP_MSG(p) (&(p->m_msg))
#define CONF_EVT_MSG(p) (p->m_msg)

   struct aal_q_item                      m_qitem;          // Derive from q_item
   struct aalbus_configmonitor_message    m_msg;            // The message
};


//=============================================================================
// Name: aalbus_config_update_event_create
// Inputs:  updateType - Typer of update (e.g., DeviceAdded)
//          pid - In the case of owneradded or removed the PID of the owner
//          pdevattrs - Pointer to the device attributes
// Returns: Pointer to event
// Description: Constructor
//=============================================================================
static inline struct aalbus_config_update_event *
        aalbus_config_update_event_create( struct device_attributes *pdevattrs,
                                           krms_cfgUpDate_e  updateType )
{
   btWSSize size = 0;

   if(NULL == pdevattrs){
      return NULL;
   }

   size = sizeof( struct aalbus_config_update_event ) + pdevattrs->size;
   
   // Allocate the event
   struct aalbus_config_update_event * This = (struct aalbus_config_update_event *)kosal_kmalloc( size );
   if(This == NULL){
      return NULL;
   }
   
   // Must set size for the aalbus_configmon_msg_attributes() macro to work properly
   CONF_EVT_MSG(This).attribute_size = pdevattrs->size;

   // Copy in the device attributes
   memcpy( ( aalbus_configmon_msg_attributes(btByteArray, CONF_EVTP_MSG( This ) )), pdevattrs, pdevattrs->size );

   CONF_EVT_MSG( This ).id = updateType;

   AALQ_QLEN( This ) = size;
   AALBUSEVTQI( This ).m_id = evtid_bus_ConfigUpdate;

   // Initialize the queue item
   kosal_list_init(&AALQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: aalbus_config_update_event_destroy
// Description: Destructor
//=============================================================================
static inline void 
   aalbus_config_update_event_destroy( struct aalbus_config_update_event *This )
{
   kosal_kfree( This, AALQ_QLEN( This ) );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if 0
//=============================================================================
// Name: rms_reqq_Shutdown
// Description: Shutdown request
//=============================================================================
struct rms_reqq_Shutdown {
#define qi_to_shutdownreq(pqi) (container_of( pqi, struct rms_reqq_Shutdown, m_rmsqitem.m_qitem ) )
#define SHUTDOWNREQ_REASON(p) ((p->m_reason))

   //-----------------------------------------------------------------------
   // Including this macro effectively causes this object to be derived from
   // aal_q_item
   //-----------------------------------------------------------------------
   _DECLARE_RMSSQ_TYPE;

   // Reason
   rms_shutdownreason_e  m_reason;
};


//=============================================================================
// Name: rms_reqq_Shutdown
// Description: Constructor
//=============================================================================
static inline struct rms_reqq_Shutdown *
     rms_reqq_Shutdown_create(rms_shutdownreason_e reason)
{
#define qi_to_shutdownevent(pqi) (container_of( pqi, struct rms_reqq_Shutdown, m_rmsqitem.m_qitem ) )

   struct rms_reqq_Shutdown * This
                        = (struct rms_reqq_Shutdown * )kosal_kmalloc(sizeof(struct rms_reqq_Shutdown));
   if(This == NULL){
      return NULL;
   }
   This->m_reason = reason;

   RMSSQ_QID(This)    = reqid_Shutdown;
   RMSSQ_QLEN(This)   = 0;
   RMSSQ_TRANID(This).m_context = NULL;

   // Initialize the queue item
   kosal_list_init(&RMSSQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: rms_reqq_Shutdown_destroy
// Description: Destructor
//=============================================================================
static inline void rms_reqq_Shutdown_destroy(struct rms_reqq_Shutdown *This)
{
   kfree(This);
}
#endif

//
// Prototypes
//
int aalbus_process_configmon_event( struct aalbus_session *,
                                    struct aal_q_item*,
                                    struct aalbus_configmonitor_message *,
                                    btWSSize *);

btInt Bus_IResMon_SendEvent( struct aal_q_item * );

END_NAMESPACE(AAL)

#endif // __AALKERNEL_AALBUS_EVENTS_H__

