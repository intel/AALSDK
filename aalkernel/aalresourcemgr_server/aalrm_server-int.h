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
//        FILE: aalrm_server-int.h
//     CREATED: 02/13/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file containe internal definetions for the
//          Accelerator Hardware Module Emulator (AHME) driver module.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/13/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/23/2009     JG       Initial code cleanup
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 04/28/2010     HM       Added return value checks to kosal_sem_get_krnl_alertable()
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
//****************************************************************************
#ifndef __AALKERNEL_AALRESOURCEMGR_SERVER_AALRM_SERVER_INT_H__
#define __AALKERNEL_AALRESOURCEMGR_SERVER_AALRM_SERVER_INT_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalrm_server-services.h"
#include "aalsdk/kernel/aalrm_server.h"


#ifndef DRV_VERSION
# define DRV_VERSION      "EXPERIMENTAL VERSION"
#endif

#define DRV_DESCRIPTION   "Resource Manager Server Kernel Module"
#define DRV_AUTHOR        "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE       "GPL"
#define DRV_COPYRIGHT     "Copyright (c) 2008-2014 Intel Corporation"

extern btUnsignedInt debug;

#define AALRMS_DBG_ALL        AALRMS_DBG_MOD | AALRMS_DBG_FILE | AALRMS_DBG_MMAP | AALRMS_DBG_IOCTL
#define AALRMS_DBG_INVLID    ~(AALRMS_DBG_ALL)
#define AALRMS_DBG_DEFAULT    AALRMS_DBG_ALL

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////      RESOURCE MANAGER SERVER SERVICE     ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Typedefs and constants
//=============================================================================
#define  KRMS_DEFAULT_EVENTS   0


extern struct aalrm_server rmserver;

//=============================================================================
// Name: aalrm_server_session
// Description: Session structure holds state and other context for a user
//              session with the RMS.
//=============================================================================
struct aalrm_server_session {
   // Event flags
   unsigned int          m_eventflgs;

   // AAL Bus
   struct aal_bus       *m_aalbus;

   // The resource manager
   struct aalrm_server  *m_rmserver;

   // Queue of sessions
   kosal_list_head       m_sessq;

  // Wait queue used for poll
   kosal_poll_object     m_waitq;

   // Private semaphore
   kosal_semaphore       m_sem;
};

//=============================================================================
// Name: aalrm_server
// Description: RMS Service class. This is the class definition for the
//              RMS kernel service.
//=============================================================================
struct aalrm_server {
   // Public Methods
   void (*register_sess)(kosal_list_head *psession);
   void (*unregister_sess)(kosal_list_head *psession);

   // RMS driver
   struct aal_driver       *m_driver;

   // RMS class device
   struct aal_classdevice  *m_class;

   // List of current sessions
   kosal_list_head          m_sessq;

   // Private semaphore
   kosal_semaphore          m_sem;

   // Request queue
   aal_queue_t              m_reqq;

   // Pending queue
   aal_queue_t              m_pendq;

   // Wait queue used to unblock poll
   kosal_poll_object        m_reqq_wq;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          RMSS MESSAGE CLASSES            ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//----------------------------------------------------------------------------
// The following classes are used to wrap Resource Manager Service INterface
// calls into messages that can be queued for the handling by the RMSS.
// The RMSS presents a Proxy interface to clients (e.g., RMCS) via the AALBus
// Service Interface Broker.  The RMSS API is asynchronous. Method calls into
// the RMSS service interface are converted into request messages that are
// queued for scheduled processing
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//--------------------------------------------
// Base class for all RMSS request queue items
//--------------------------------------------
struct rms_reqq_item {
   struct aal_q_item              m_qitem;
   struct aalrms_req_tranID       m_tranid;
};
#define _DECLARE_RMSSQ_TYPE  struct rms_reqq_item m_rmsqitem

//----------------------------
// Casting and accessor macros
//----------------------------
#define qi_to_rmsqi(pqi)   (container_of( pqi, struct rms_reqq_item, m_qitem ))
#define RMSSQI(p)       (p->m_rmsqitem.m_qitem)
#define RMSSQIP(p)      (&(p->m_rmsqitem.m_qitem))
#define RMSSQ_QID(p)    ( AALQ_QID( &(p->m_rmsqitem )) )
#define RMSSQ_QLEN(p)   (AALQ_QLEN(&(p->m_rmsqitem )))
#define RMSSQ_QUEUE(p)  (AALQ_QUEUE(&(p->m_rmsqitem )))
#define RMSSQ_TRANID(p) (p->m_rmsqitem.m_tranid)

//=============================================================================
// Name: rms_reqq_reqdev
// Description: Requests a Device allocation from the Resource Manager Service
//=============================================================================
struct rms_reqq_reqdev{
#define qi_to_reqdev(pqi) (container_of( pqi, struct rms_reqq_reqdev, m_rmsqitem.m_qitem ))

   //-----------------------------------------------------------------------
   // Including this macro effectively causes this object to be derived from
   // rms_reqq_item
   //-----------------------------------------------------------------------
   _DECLARE_RMSSQ_TYPE;

   // Request specific
   struct req_allocdev           *m_reqdev;
   void                          *m_powner;
   aalrms_reqdev_cmplt_t          m_completionfcn;
   void                          *m_context;
};

//=============================================================================
// Name: rms_reqq_reqdev_create
// Description: Constructor
//=============================================================================
static inline struct rms_reqq_reqdev *
      rms_reqq_reqdev_create( struct req_allocdev       *req,
                              aalrms_reqdev_cmplt_t      completionfcn,
                              struct aalrms_req_tranID   tranid)
{
   struct rms_reqq_reqdev * This = (struct rms_reqq_reqdev * )kosal_kmalloc(sizeof(struct rms_reqq_reqdev));
   if(This == NULL){
      return NULL;
   }

   This->m_reqdev = req;
   This->m_completionfcn = completionfcn;
   RMSSQ_QID(This) = reqid_URMS_RequestDevice;
   RMSSQ_QLEN(This) = req->size;
   RMSSQ_TRANID(This) = tranid;

   // Initialize the queue item
   kosal_list_init(&RMSSQ_QUEUE(This));
   return This;
}

//----------
// Accessors
//----------
#define REQDEV_ALLOCDEV(preqdev)    (*(preqdev->m_reqdev))
#define REQDEV_ALLOCDEVP(preqdev)   (preqdev->m_reqdev)


//=============================================================================
// Name: rms_reqq_reqdev_create
// Description: Destructor
//=============================================================================
static inline void rms_reqq_reqdev_destroy(struct rms_reqq_reqdev *This)
{
   kfree(This);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//=============================================================================
// Name: rms_reqq_registrar
// Description: Registrar request message
//=============================================================================
struct rms_reqq_registrar {
#define qi_to_regreq(pqi) (container_of( pqi, struct rms_reqq_registrar, m_rmsqitem.m_qitem ) )
   //-----------------------------------------------------------------------
   // Including this macro effectively causes this object to be derived from
   // aal_q_item
   //-----------------------------------------------------------------------
   _DECLARE_RMSSQ_TYPE;

   // Request specific
   struct req_registrar          *m_regreq;
   aalrms_registrarreq_cmplt_t    m_completionfcn;
};

//----------
// Accessors
//----------
#define REGREQ_REGREQ(p)    (*(p->m_regreq))
#define REGREQ_REGREQP(p)   (p->m_regreq)


#define CALL_COMPLETION(p)    (p->m_completionfcn)

//=============================================================================
// Name: rms_reqq_registrar_create
// Description: Constructor
//=============================================================================
static inline struct rms_reqq_registrar *
         rms_reqq_registrar_create( struct req_registrar          *req,
                                    aalrms_registrarreq_cmplt_t   completionfcn,
                                    struct aalrms_req_tranID      tranid)
{
   struct rms_reqq_registrar * This = (struct rms_reqq_registrar *)kosal_kmalloc(sizeof(struct rms_reqq_registrar));
   if(This == NULL){
      return NULL;
   }

   This->m_regreq = req;
   This->m_completionfcn = completionfcn;

   RMSSQ_QID(This) = reqid_RS_Registrar;
   RMSSQ_QLEN(This) = req->size;
   RMSSQ_TRANID(This) = tranid;

   // Initialize the queue item
   kosal_list_init(&RMSSQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: rms_reqq_regreq_destroy
// Description: Destructor
//=============================================================================
static inline void rms_reqq_regreq_destroy(struct rms_reqq_registrar *This)
{
   kfree(This);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//=============================================================================
// Name: rms_reqq_config_update_event
// Description: Config update event
//=============================================================================
struct rms_reqq_config_update_event
{
#define qi_to_updateevent(pqi) (container_of( pqi, struct rms_reqq_config_update_event, m_rmsqitem.m_qitem ) )
#define CONF_EVTP_EVENT(p) (*(p->m_pupdateEvt))

   //-----------------------------------------------------------------------
   // Including this macro effectively causes this object to be derived from
   // aal_q_item
   //-----------------------------------------------------------------------
   _DECLARE_RMSSQ_TYPE;

   // Request specific
   struct aalrms_configUpDateEvent          *m_pupdateEvt;
};

//----------
// Accessors
//----------


//=============================================================================
// Name: rms_reqq_config_update_event_create
// Description: Constructor
//=============================================================================
static inline struct rms_reqq_config_update_event *
     rms_reqq_config_update_event_create(struct aalrms_configUpDateEvent  *pupdateEvt)
{
   struct rms_reqq_config_update_event * This
                        = (struct rms_reqq_config_update_event * )kosal_kmalloc(sizeof(struct rms_reqq_config_update_event));
   if(This == NULL){
      return NULL;
   }
   This->m_pupdateEvt = pupdateEvt;

   DPRINTF(AALRMS_DBG_IOCTL,"ID %d and %d\n",  This->m_pupdateEvt->id ,pupdateEvt->id );

   RMSSQ_QID(This) = evtid_KRMS_ConfigUpdate;
   RMSSQ_QLEN(This) = offsetof(struct aalrms_configUpDateEvent,devattrs.ownerlist) + (pupdateEvt->devattrs.numOwners * sizeof(btPID));
   RMSSQ_TRANID(This).m_context = NULL;


   // Initialize the queue item
   kosal_list_init(&RMSSQ_QUEUE(This));
   return This;
}

//=============================================================================
// Name: rms_reqq_config_update_event_destroy
// Description: Destructor
//=============================================================================
static inline void rms_reqq_config_update_event_destroy(struct rms_reqq_config_update_event *This)
{
   kfree(This->m_pupdateEvt);
   kfree(This);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

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


//=============================================================================
//=============================================================================
//                            Resource Manager Server
//=============================================================================
//=============================================================================
extern int aalrm_server_open(struct inode *, struct file *);
extern int aalrm_server_close(struct inode *, struct file *);

#if HAVE_UNLOCKED_IOCTL
extern long aalrm_server_ioctl(struct file *,
                               unsigned int,
                               unsigned long);
#else
extern int aalrm_server_ioctl(struct inode *,
                              struct file *,
                              unsigned int,
                              unsigned long);
#endif

extern unsigned int aalrm_server_poll(struct file *, poll_table *);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          RMSS QUEUE MANAGEMENT           ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//----------------------------------------------------------------------------
// The following functions are used to interact with the RMSS queues
// They are here primarily for legacy reasons and for the most part simply
// wrap the AAL generic functions.
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//=============================================================================
// Name: aalrms_queue_req
// Description: Queues a request onto the request queue
// Interface: private
// Inputs: pqitem - item
// Outputs: none.
// Comments: Causes any threads waiting on this event to wake up
//=============================================================================
static inline void aalrms_queue_req(struct aal_q_item * pqitem)
{
   _aal_q_enqueue( pqitem, &rmserver.m_reqq);

   // Unblock select() calls.
   wake_up_interruptible (&rmserver.m_reqq_wq);
}

//=============================================================================
// Name: aalrms_queue_push_req
// Description: Queues a request onto the request queue head
// Interface: private
// Inputs: pqitem - item
// Outputs: none.
// Comments: Causes any threads waiting on this event to wake up
//=============================================================================
static inline void aalrms_queue_push__req(struct aal_q_item * pqitem)
{
   _aal_q_pushqueue( pqitem, &rmserver.m_reqq);

   // Unblock select() calls.
   wake_up_interruptible (&rmserver.m_reqq_wq);
}

//=============================================================================
// Name: aalrms_dequeue_req
// Description: Get the next item from the request queue
// Interface: private
// Inputs: None
// Returns: next item; NULL - empty.
// Comments:
//=============================================================================
static inline struct aal_q_item * aalrms_dequeue_req(void)
{
   return _aal_q_dequeue(&rmserver.m_reqq);
}

//=============================================================================
// Name: aalrms_peek_req
// Description: Get the next item from the request queue without dequeuing
// Interface: private
// Inputs: None
// Returns: next item; NULL - empty.
// Comments:
//=============================================================================
static inline struct aal_q_item * aalrms_peek_req(void)
{
   return _aal_q_peek(&rmserver.m_reqq);
}


//=============================================================================
// Name: aalrms_reqq_empty
// Description: return status of request queue
// Interface: private
// Inputs: None
// Returns: >0 - empty
// Comments:
//=============================================================================
static inline int aalrms_reqq_empty(void)
{
   return _aal_q_empty(&rmserver.m_reqq);
}


//=============================================================================
// Name: aalrms_reqq_find
// Description: if the item is found
// Interface: private
// Inputs: pqitem - item to find
// Returns: NULL = Not foind
// Comments:
//=============================================================================
static inline int aalrms_reqq_find(struct aal_q_item * pqitem)
{
   return (_aal_q_find(&QI_QUEUE(pqitem), &rmserver.m_reqq) != NULL);
}

//=============================================================================
// Name: aalrms_queue_move_to_pend
// Description: Moves a request to the pending queue atomically
// Interface: private
// Inputs: pqitem - item
// Outputs: none.
// Comments:
//=============================================================================
static inline void aalrms_queue_move_to_pend(struct aal_q_item * pqitem)
{
   _aal_q_swapqueue( pqitem, &rmserver.m_reqq, &rmserver.m_pendq);
}

//=============================================================================
// Name: aalrms_dequeue_pend
// Description: Get the next item from the pending queue
// Interface: private
// Inputs: None
// Returns: next item; NULL - empty.
// Comments:
//=============================================================================
static inline struct aal_q_item * aalrms_dequeue_pend(void)
{
   return _aal_q_dequeue(&rmserver.m_pendq);
}

//=============================================================================
// Name: aalrms_peek_pend
// Description: Get the next item from the pending queue without dequeuing
// Interface: private
// Inputs: None
// Returns: next item; NULL - empty.
// Comments:
//=============================================================================
static inline struct aal_q_item * aalrms_peek_pend(void)
{
   return _aal_q_peek(&rmserver.m_pendq);
}


//=============================================================================
// Name: aalrms_pendq_empty
// Description: return status of pending queue
// Interface: private
// Inputs: None
// Returns: >0 - empty
// Comments:
//=============================================================================
static inline int aalrms_pendq_empty(void)
{
   return _aal_q_empty(&rmserver.m_pendq);
}

//=============================================================================
// Name: aalrms_pendq_find
// Description: if the item is found
// Interface: private
// Inputs: pqitem - item to find
// Returns: NULL = Not found
// Comments:
//=============================================================================
static inline int aalrms_pendq_find(struct aal_q_item * pqitem)
{
   return (_aal_q_find(&QI_QUEUE(pqitem), &rmserver.m_pendq) != NULL);
}

//=============================================================================
// Name: aalrms_pendq_remove
// Description: if the item is found
// Interface: private
// Inputs: pqitem - item to find
// Returns: NULL = Not found
// Comments: TODO these should use qitems not list_head entries
//=============================================================================
static inline int aalrms_pendq_remove(struct aal_q_item * pqitem)
{
   kosal_list_head *entry = NULL;

   entry = _aal_q_find(&QI_QUEUE(pqitem),
                      &rmserver.m_pendq);
   if(entry != NULL){
      // NOTE: It is possible for this to fail due to not being able to lock the queue.
      //       If something other than ignoring needs to be done, test the result code here
      _aal_q_remove(entry, &rmserver.m_pendq );
      return 1;
   }
   return 0;
}


//=============================================================================
// Name: aalrms_queue_remove
// Description: remove from any queue
// Interface: private
// Inputs: pqitem - item to find
// Returns: NULL = Not found
// Comments:
//=============================================================================
static inline int aalrms_queue_remove(aal_queue_t *pqueue,
                                      struct aal_q_item * pqitem)
{
   kosal_list_head *entry = NULL;

   entry = _aal_q_find(&QI_QUEUE(pqitem),
                      pqueue);
   if(entry != NULL){
      // NOTE: It is possible for this to fail due to not being able to lock the queue.
      //       If something other than ignoring needs to be done, test the result code here
      _aal_q_remove(entry, pqueue);
      return 1;
   }
   return 0;
}

//=============================================================================
// Name: aalrms_queue_doForAllQueuedItems
// Description: For each item on the Request queue and pending queue call the
//              function passing the context and queue  info.
// Interface: private
// Inputs: pqitem - item to find
// Returns: NULL = Not found
// Comments:
//=============================================================================
static inline void aalrms_queue_doForAllQueuedItems( aal_q_ProcessItem_t fcn,
                                                     void *context)
{
   // Request queue first
   _aal_q_doForEachItem( &rmserver.m_reqq,
                         fcn,
                         context);
   // Now pending queue
   _aal_q_doForEachItem( &rmserver.m_pendq,
                         fcn,
                         context);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                  MACROS                  ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

// Macros to help with deep fields. Act like casts
#define rms_sess_to_rms_server(sess)         (*(sess)->m_rmserver)

#define rms_drv_to_ibus(drv)                 (*(drv)->m_bus)
#define rms_sess_to_ibus(s)                  (*((s)->m_rmserver->m_driver->m_bus))


// Singleton RMS
extern struct aalrm_server rmserver;


// External implementations
extern int aalrms_request_device( struct req_allocdev* preqdev,
                                  aalrms_reqdev_cmplt_t completionfcn,
                                  struct aalrms_req_tranID tranID);

extern int aalrms_registrar_request( struct req_registrar* preqdev,
                                     aalrms_registrarreq_cmplt_t completionfcn,
                                     struct aalrms_req_tranID tranID);

extern void aalrms_cancel_all_requests(  struct aalrms_req_tranID *);

#endif // __AALKERNEL_AALRESOURCEMGR_SERVER_AALRM_SERVER_INT_H__

