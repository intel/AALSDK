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
//        FILE: aalinterface.h
//     CREATED: 02/21/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  External definitions for the AAL Interface object
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-21-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALINTERFACE_H__
#define __AALSDK_KERNEL_AALINTERFACE_H__
#include <aalsdk/kernel/kosal.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            AAL Interface
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//=============================================================================
// Name: aal_interface
// Description: AAL Interface represents an opaque, typically dynamically
//              bindable, interface
//=============================================================================
struct aal_interface {
#define AAL_INTERFACE_FLAG_IS_REGISTERED 0x00000001
   btUnsigned32bitInt m_flags;
   kosal_ownermodule  m_owner;    // Module that owns this interface
   bt32bitInt         m_count;    // Number of outstanding get's
   btIID              m_iid;      // Interface ID
   void              *m_iptr;     // Pointer to the interface. Must be cast
   kosal_list_head    m_list;     // Used when interface is in a list
};

#define aalinterface_is_registered(p)  ((p)->m_flags & AAL_INTERFACE_FLAG_IS_REGISTERED)
#define aalinterface_set_registered(p) ((p)->m_flags |= AAL_INTERFACE_FLAG_IS_REGISTERED)
#define aalinterface_clr_registered(p) ((p)->m_flags &= ~AAL_INTERFACE_FLAG_IS_REGISTERED)

#define aalinterface_count(p)          ((p)->m_count)
#define aalinterface_get(p)            (++((p)->m_count))
#define aalinterface_put(p)            (--((p)->m_count))

// Casting macro for aal_interfaces. Returns NULL if improper cast
#define cast_aal_interface(t,i,p) ( (t*) ((i==p->m_iid) ? p->m_iptr : NULL) )
#define aalinterface_iptr(p)      (p->m_iptr)

//=============================================================================
// Name: aal_interface_init
// Description: Initialize and interface
//=============================================================================
#define aal_interface_init(i,p,id) \
do                                 \
{  i.m_flags = 0;                  \
   i.m_owner = THIS_MODULE;        \
   i.m_count = 0;                  \
   i.m_iid   = id;                 \
   i.m_iptr  = p;                  \
   kosal_list_init(&i.m_list);     \
}while(0)

//=============================================================================
// Name: aal_interface_find
// Description: Finds and returns an interface of the specified type
// Interface: inline
// Inputs: list - list of interfaces
//         iid - ID of interface to find
// Outputs: NULL - Failed. Interface if successful
// Comments: This function does not increment the module reference count
//           If this interface is to be used outside the owning module
//           the client should take the necessary precautions such as
//           issuing a try_module_get() on the owner prior to using.
//=============================================================================
static inline
struct aal_interface *
aal_interface_find(kosal_list_head *list, btIID iid)
{
   struct aal_interface *tmp = NULL;

   kosal_printk("Looking for interface %" PRIuIID " on %p\n", iid, list);

   kosal_list_for_each_entry(tmp, list, m_list, struct aal_interface) {
      kosal_printk("Looking at ID %" PRIuIID "\n", tmp->m_iid);
      if ( tmp->m_iid == iid ) {
         kosal_printk("Interface found at %p\n", tmp);
         return tmp;
      }
   }
   kosal_printk("Interface not found\n");
   return NULL;
}

//=============================================================================
// Name: aal_interface_add
// Description: Adds an interface
// Interface: inline
// Inputs: list - list of interfaces
//         pinterface - interface to add
// Outputs: 0 - success.
//=============================================================================
static inline
int
aal_interface_add(kosal_list_head      *list,
                  kosal_semaphore      *sem,
                  struct aal_interface *pinterface)
{
   if ( unlikely( kosal_sem_get_krnl_alertable(sem) ) ) {
      return -2;
   }

   if ( aal_interface_find(list, pinterface->m_iid) ) {
      kosal_sem_put(sem);
      // Already registered
      kosal_printk("%p already registered\n", pinterface);
      return -1;
   }
   kosal_printk("Adding Interface ID %" PRIuIID "\n", pinterface->m_iid);
   kosal_list_add(&pinterface->m_list, list);
   kosal_sem_put(sem);

   return 0;
}

//=============================================================================
// Name: aal_interface_del
// Description: Finds and returns an interface of the specified type
// Interface: inline
// Inputs: list - list of interfaces
//         pinterface - interface to delete
// Outputs: 0 - success.
//=============================================================================
static inline
int
aal_interface_del(kosal_list_head      *list,
                  kosal_semaphore      *sem,
                  struct aal_interface *pinterface)
{
   if ( unlikely( kosal_sem_get_krnl_alertable(sem) ) ) {
      return -2;
   }

   if ( NULL == aal_interface_find(list, pinterface->m_iid) ) {
      kosal_sem_put(sem);
      // Not found
      return -1;
   }

   kosal_list_del_init(&pinterface->m_list);
   kosal_sem_put(sem);

   return 0;
}

#endif // __AALSDK_KERNEL_AALINTERFACE_H__
