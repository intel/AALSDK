//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2016, Intel Corporation.
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
//  Copyright(c) 2008-2016, Intel Corporation.
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
//        FILE: aalqueue.h
//     CREATED: 09/15/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  External definitions for the AAL queue utilities
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09-15-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 02/03/2009     JG       Added _aal_q_swapqueue
//                           Removed unused fields in queue struct.
// 02/04/2009     JG       Added _aal_q_doForEachItem
// 04/28/2010     HM       Added return value checks to kosal_sem_get_krnl_alertable()
// 08/06/2010     HM       Added #include <asm/uaccess.h>, previously included
//                            only implicitly
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
// 09/12/2010     HM       Added LINUX_VERSION_CODE for semaphore.h
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALQUEUE_H__
#define __AALSDK_KERNEL_AALQUEUE_H__
#include <aalsdk/kernel/kosal.h>


BEGIN_NAMESPACE(AAL)

//-----------------------------------------------------------------------------
//                         Queue Primitives
//-----------------------------------------------------------------------------

//=============================================================================
// Name: rm_q_item
// Description: Base class for queuable requests
//=============================================================================
struct aal_q_item {
   btID            m_id;
   btWSSize        m_length;
   kosal_list_head m_queue;
};

//Macro used to embed this type in a subclass structure
#define _DECLARE_AALQ_TYPE struct aal_q_item  m_qitem

// Macros for accessing base members
#define QI_QUEUE(pqi)      ((pqi)->m_queue)
#define QI_QID(pqi)        ((pqi)->m_id)
#define QI_LEN(pqi)        (NULL == (pqi) ? 0 : (pqi)->m_length)

// Macros for accessing base members
#define AALQI(pqi)         ((pqi)->m_qitem)
#define AALQIP(pqi)        (&(pqi)->m_qitem)
#define AALQ_QUEUE(pqi)    ((pqi)->m_qitem.m_queue)
#define AALQ_QID(pqi)      ((pqi)->m_qitem.m_id)
#define AALQ_QLEN(pqi)     ((pqi)->m_qitem.m_length)


//=============================================================================
// Name: aal_queue_t
// Description: AAL queue object
//=============================================================================
typedef struct {
   kosal_mutex     m_sem;
   kosal_list_head m_queue;
} aal_queue_t;

//=============================================================================
// Name: aal_queue_t
// Description: AAL queue object
//=============================================================================
inline static void aal_queue_init(aal_queue_t *thisQueue)
{
   // Initialize base members
   kosal_list_init(&thisQueue->m_queue);
   kosal_mutex_init(&thisQueue->m_sem);
}



//=============================================================================
// Name: _aal_q_enqueue
// Description: Queues a request
// Interface: private
// Inputs: pqitem - item
//         pqueue - queue
// Outputs: none.
// Comments:
//=============================================================================
static inline void _aal_q_enqueue(struct aal_q_item *pqitem,
                                  aal_queue_t       *pqueue)
{
   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted in.\n");
      return;
   }
   kosal_list_add_tail(&pqitem->m_queue, &pqueue->m_queue);
   kosal_sem_put(&pqueue->m_sem);
}


//=============================================================================
// Name: _aal_q_pushqueue
// Description: Queues a request on head
// Interface: private
// Inputs: pqitem - item
//         pqueue - queue
// Outputs: none.
// Comments:
//=============================================================================
static inline void _aal_q_pushqueue(struct aal_q_item *pqitem,
                                    aal_queue_t       *pqueue)
{
   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return;
   }
   kosal_list_add_head( &pqitem->m_queue, &pqueue->m_queue );
   kosal_sem_put(&pqueue->m_sem);
}


//=============================================================================
// Name: _aal_q_dequeue
// Description: Get the next item from the queue
// Interface: private
// Inputs: pqueue - queue
// Returns: next item; NULL - empty.
// Comments:
//=============================================================================
static inline struct aal_q_item * _aal_q_dequeue(aal_queue_t *pqueue)
{
   struct aal_q_item *pqitem;

   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted in.\n");
      return NULL;
   }

   if ( kosal_list_is_empty(&pqueue->m_queue) ) {
      kosal_sem_put(&pqueue->m_sem);
      return NULL;
   }

   // Get a pointer to the item
   pqitem = kosal_list_get_object(kosal_list_next(&pqueue->m_queue), 
                                  struct aal_q_item, 
                                  m_queue);

   kosal_list_del_init(&pqitem->m_queue);

   kosal_sem_put(&pqueue->m_sem);

   return pqitem;
}

//=============================================================================
// Name: _aal_q_swapqueue
// Description: Swap item from one queue to another atomically
// Interface: private
// Inputs: pfrom - queue the item should be on
//         pto - queue the item will be placed on
// Returns: none
// Comments: Note that for performance reasons the from queue is not checked
//           to see if the item is actually present. If it is not then the
//           result is the same as if an enqueue was done to pto while pfrom
//           is protected from change.
//=============================================================================
static inline void _aal_q_swapqueue(struct aal_q_item *pqitem,
                                    aal_queue_t       *pfrom,
                                    aal_queue_t       *pto)
{

//   kosal_sem_get_user_alertable(&pfrom->m_sem);
//   kosal_sem_get_user_alertable(&pto->m_sem);

   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pfrom->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable of &pfrom->m_sem interrupted.\n");
      return;
   }

   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pto->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable of &pto->m_sem interrupted.\n");
      kosal_sem_put(&pfrom->m_sem);
      return;
   }

   // Remove the item
   kosal_list_del_init(&pqitem->m_queue);

   // Add to target queue
   kosal_list_add_tail(&pqitem->m_queue, &pto->m_queue);

   kosal_sem_put(&pto->m_sem);
   kosal_sem_put(&pfrom->m_sem);
}

//=============================================================================
// Name: _aal_q_peek
// Description: Get the next item from the request queue without dequeing
// Interface: private
// Inputs: pqueue - queue
// Returns: next item; NULL - empty.
// Comments: returns NULL if cannot acquire mutex, even if not empty
//=============================================================================
static inline struct aal_q_item * _aal_q_peek(aal_queue_t *pqueue)
{
   struct aal_q_item *pqitem;

   // FIXME 20100428 -- Joe, you need to look at this error return value
   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return NULL;
   }

   if ( kosal_list_is_empty(&pqueue->m_queue) ) {
      kosal_sem_put(&pqueue->m_sem);
      return NULL;
   }

   // Get a pointer to the item
   pqitem = kosal_list_get_object(kosal_list_next(&pqueue->m_queue), struct aal_q_item, m_queue);

   kosal_sem_put(&pqueue->m_sem);

   return pqitem;
}


//=============================================================================
// Name: _aal_q_empty
// Description: return status of  queue
// Interface: private
// Inputs: pqueue - queue
// Returns: >0 - empty
// Comments: Returns "empty" if cannot lock semaphore
//=============================================================================
static inline int _aal_q_empty(aal_queue_t *pqueue)
{
   int ret = 0;

   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return 0;
   }

   ret = kosal_list_is_empty(&pqueue->m_queue);
   kosal_sem_put(&pqueue->m_sem);
   return ret;
}

//=============================================================================
// Name: _aal_q_find
// Description: return whether an item is on a specified queue
// Interface: private
// Inputs: pqitem - item
//         pqueue - queue
// Returns: aal_q_item * pqitem - found item
// Comments:
//=============================================================================
static inline kosal_list_head * _aal_q_find(pkosal_list_head pitem,
                                            aal_queue_t     *pqueue)
{

   pkosal_list_head pitr;
   pkosal_list_head result = NULL;

   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return NULL;
   }

   //Loop through the list looking for a match
   kosal_list_for_each(pitr, &pqueue->m_queue) {
      if( pitr == pitem ) {
         result = pitr;
         break;
      }
   }
   kosal_sem_put(&pqueue->m_sem);
   return result;
}

//=============================================================================
// Name: _aal_q_remove
// Description: remove an entry from the queue
// Interface: private
// Inputs: pitem - item
// Comments:
//=============================================================================
static inline void _aal_q_remove(pkosal_list_head pitem,
                                 aal_queue_t     *pqueue)
{
   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return;
   }

   kosal_list_del_init(pitem);

   kosal_sem_put(&pqueue->m_sem);
}



//=============================================================================
// Name: _aal_q_doForEachItem
// Description: Calls the function pointed to by fcn for each item on the
//              queue pointed to by pqueue. Continues until queue is empty or
//              the callback returns non-zero. The context is used to provide
//              the callback with additional arguments it needs.
// Interface: public
// Inputs: pqueue - queue to process
//         fcn - pointer to callback
//         context - context to pass
// Returns: pointer to last item processed or NULL if list completed
// Comments:
//=============================================================================
//-----------------------------------------------------------------------------
// Prototype for call back used in doForeachOwner
typedef int (*aal_q_ProcessItem_t)(struct aal_q_item *,
                                   aal_queue_t *,
                                   btAny );
//-----------------------------------------------------------------------------
static inline struct aal_q_item * _aal_q_doForEachItem(aal_queue_t        *pqueue,
                                                       aal_q_ProcessItem_t fcn,
                                                       btAny               context)
{
   pkosal_list_head   pitr;
   pkosal_list_head   temp;
   struct aal_q_item *pqitem = NULL;

   // kosal_sem_get_user_alertable(&pqueue->m_sem);
   // Try to acquire lock. If cannot, then abort.
   if ( unlikely( kosal_sem_get_user_alertable(&pqueue->m_sem) ) ) {
      kosal_printk("kosal_sem_get_user_alertable interrupted.\n");
      return NULL;
   }
   
   UNREFERENCED_PARAMETER(temp);
   //Loop through the list looking for a match
   kosal_list_for_each_safe(pitr, temp, &pqueue->m_queue) {
      pqitem = kosal_get_object_containing(pitr, struct aal_q_item, m_queue);
      // If the function returns non-zero return current item
      if ( fcn(pqitem, pqueue, context) != 0 ) {
         kosal_sem_put(&pqueue->m_sem);
         return pqitem;
      }
   }

   kosal_sem_put(&pqueue->m_sem);
   return NULL;
}

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALQUEUE_H__

