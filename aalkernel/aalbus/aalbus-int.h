//******************************************************************************
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
//        FILE: aalbus-int.h
//     CREATED: 02/15/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  Internal definitions for the AAL Logical Bus driver module
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-15-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
//****************************************************************************
#ifndef __AALKERNEL_AALBUS_AALBUS_INT_H__
#define __AALKERNEL_AALBUS_AALBUS_INT_H__
#include <aalsdk/kernel/kosal.h>
#include <aalsdk/kernel/aalbus.h>


#ifndef DRV_VERSION
   #define DRV_VERSION    "EXPERIMENTAL VERSION"
#endif

#define DRV_DESCRIPTION   "AAL Logical Bus Module"
#define DRV_AUTHOR        "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE       "GPL"
#define DRV_COPYRIGHT     "Copyright(c) 2008-2016, Intel Corporation"


//
// Attribute defaults
//

// Debug attributes
//#define AALBUS_DBG_MOD    (1 << 0)
//#define AALBUS_DBG_FILE   (1 << 1)
//#define AALBUS_DBG_MMAP   (1 << 2)
//#define AALBUS_DBG_IOCTL  (1 << 3)

#define AALBUS_DBG_ALL        (AALBUS_DBG_MOD | AALBUS_DBG_FILE | AALBUS_DBG_MMAP | AALBUS_DBG_IOCTL)
#define AALBUS_DBG_INVLID    ~(AALBUS_DBG_ALL)
#define AALBUS_DBG_DEFAULT    AALBUS_DBG_ALL

//========================
// Typedefs and constants
//========================

//=============================================================================
// Name: update_config_parms
// Description: Used in aalbus_config_update_event
//=============================================================================
struct update_config_parms
{
   krms_cfgUpDate_e             updateType;
   btPID                        pid;
   aalbus_event_config_update_t Handler;
};

//=============================================================================
// Name: aal_bus_type
// Description: AAL bus class
//=============================================================================
struct aal_bus_type
{
   struct aal_bus     m_bus;        // AAL Bus public interface

   struct {
      aalbus_event_config_update_t EventHandler;
      btObjectType                 context;
   } config_update_event_handler;
#if   defined( __AAL_LINUX__ )
   struct bus_type    m_bustype;    // Generic bus base
#endif
   kosal_list_head    m_servicelist;
   kosal_mutex        m_sem;        // Private mutex

#define AAL_BUS_TYPE_FLAG_REGISTERED 0x00000001
   btUnsigned16bitInt m_flags;

   // Head of all aal_device structs created by aalbus, with semaphore.
   // List items are in aal_device.alloc_list.
   kosal_list_head alloc_list_head;
   kosal_mutex     alloc_list_sem;
};

#define aal_bus_type_aal_busp(pbt) (&(pbt)->m_bus)

#define aal_bus_type_is_registered(pbt)  flag_is_set((pbt)->m_flags, AAL_BUS_TYPE_FLAG_REGISTERED)
#define aal_bus_type_set_registered(pbt) flag_setf((pbt)->m_flags,   AAL_BUS_TYPE_FLAG_REGISTERED)
#define aal_bus_type_clr_registered(pbt) flag_clrf((pbt)->m_flags,   AAL_BUS_TYPE_FLAG_REGISTERED)

#endif // __AALKERNEL_AALBUS_AALBUS_INT_H__
