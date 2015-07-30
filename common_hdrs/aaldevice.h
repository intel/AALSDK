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
//        FILE: aaldevice.h
//     CREATED: 09/18/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains public definitions for AAL device objects
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09/18/08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 11/13/2008     HM       Added AAL_GUID_t, standard GUID
// 01/04/2009     HM       Updated Copyright
// 02/26/2009     JG       Began dynamic config implementation
// 10/09/2009     JG       Added support for Host Based AFUs
// 10/22/2009     JG       Added aaldev_AddOwner_e in support of modified
//                         device methods
// 04/28/2010     HM       Added return value checks to kosal_sem_get_krnl_alertable()
// 03/06/2011     HM       Added comment showing expansion of aal_device_id
// 06/25/2013     JG       Added device address macros
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALDEVICE_H__
#define __AALSDK_KERNEL_AALDEVICE_H__
#include <aalsdk/kernel/kosal.h>

#ifdef __AAL_USER__
# include <aalsdk/AALTypes.h>
# define __user
#endif

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

//-----------------------------------------------------------------------------
// Typedefs and constants
//-----------------------------------------------------------------------------

// Defines for AFU mask
#define   AAL_DEVIDMASK_AFU(n)  (0x1 <<n)
#define   AAL_DEVID_MAFU        64

//=============================================================================
// Name: 
// Description: mappable API modes
//=============================================================================
// XXX These must match fappip.h:WSID_CSRMAP_*AREA
#define AAL_DEV_APIMAP_NONE     0x00000000
#define AAL_DEV_APIMAP_CSRREAD  0x00000001
#define AAL_DEV_APIMAP_CSRWRITE 0x00000002
#define AAL_DEV_APIMAP_MMIOR    0x00000004
#define AAL_DEV_APIMAP_UMSG     0x00000008
#define AAL_DEV_APIMAP_CSRRW    ( AAL_DEV_APIMAP_CSRREAD | AAL_DEV_APIMAP_CSRWRITE )

//=============================================================================
// Standard Format for GUID is either binary or a modified hex-encoded string
// NOTE: See http://en.wikipedia.org/wiki/Globally_Unique_Identifier
//=============================================================================
typedef struct {
   btUnsigned32bitInt Data1;
   btUnsigned16bitInt Data2;
   btUnsigned16bitInt Data3;
   btByte             Data4[8];
} AAL_GUID_t;

//=============================================================================
// Name: aal_bus_types_e
// Description: AAL Bus types
//=============================================================================
enum aal_bus_types_e {
   aal_bustype_unknown=0,
   aal_bustype_FSB,
   aal_bustype_QPI,
   aal_bustype_PCIe,
   aal_bustype_ASM,
   aal_bustype_Prop,
   aal_bustype_Host
};

//=============================================================================
// Name: aal_device_type_e
// Description: AAL device types
//=============================================================================
enum aal_device_type_e {
   aal_devtype_unknown=0,
   aal_devtypeAHM=1,
   aal_devtypeAFU,
   aal_devtypeMgmtAFU
};


//=============================================================================
// Name: aal_device_request_e
// Description: AAL common device requests
//=============================================================================
enum aal_device_request_e {
   aaldev_reqActivate=1,
   aaldev_reqDeactivate,
   aaldev_reqCreateDevice,
   aaldev_reqDestroyDevice,
};

//=============================================================================
// Name: aal_device_addr
// Description: Device address structure
//=============================================================================
struct aal_device_addr {
   enum aal_bus_types_e m_bustype;     // Type of bus E.g., FSB, QPI, PCIe
   btUnsigned32bitInt   m_busnum;      // Bus number
   btUnsigned32bitInt   m_devicenum;   // device/channel number
   bt32bitInt           m_subdevnum;   // Sub-device/channel number
};
#define aaldevaddr_bustype(devaddr)         ((devaddr).m_bustype)
#define aaldevaddr_busnum(devid)    		((devaddr).m_busnum)
#define aaldevaddr_devnum(devid)    		((devaddr).m_devicenum)
#define aaldevaddr_subdevnum(devid) 		((devaddr).m_subdevnum)


//=============================================================================
// Name: AAL Device_ID
// Description: AAL device ID used to match a discovered device with a driver
//=============================================================================
struct aal_device_id {
   struct aal_device_addr m_devaddr;     // Device address
   enum aal_device_type_e m_devicetype;  // Device type

   btUnsigned32bitInt     m_vendor;      // Vendor ID

   btUnsigned64bitInt     m_pipGUID;     // PIP GUID  TODO must reconcile with btIID
   btUnsigned64bitInt     m_ahmGUID;     // AHM GUID

   btUnsigned64bitInt     m_afuGUIDl;    // AFU GUID low order
   btUnsigned64bitInt     m_afuGUIDh;    // AFU GUID high order
#define m_afuGUID m_afuGUIDl // AFU GUID low order (backward compat)
};

#define aal_device_id_terminator                                                        \
{  .m_devaddr = { .m_bustype = 0, .m_busnum = 0, .m_devicenum = 0, .m_subdevnum = 0, }, \
   .m_devicetype = aal_devtype_unknown,                                                 \
   .m_vendor   = 0,                                                                     \
   .m_pipGUID  = 0,                                                                     \
   .m_ahmGUID  = 0,                                                                     \
   .m_afuGUIDl = 0,                                                                     \
   .m_afuGUIDh = 0, }

#define aaldevid_addr(devid)              ((devid).m_devaddr)
#define aaldevid_devaddr_bustype(devid)   ((aaldevid_addr(devid)).m_bustype)
#define aaldevid_devaddr_busnum(devid)    ((aaldevid_addr(devid)).m_busnum)
#define aaldevid_devaddr_devnum(devid)    ((aaldevid_addr(devid)).m_devicenum)
#define aaldevid_devaddr_subdevnum(devid) ((aaldevid_addr(devid)).m_subdevnum)

#define aaldevid_afuguidl(devid) ((devid).m_afuGUIDl)
#define aaldevid_afuguidh(devid) ((devid).m_afuGUIDh)
#define aaldevid_devtype(devid)  ((devid).m_devicetype)
#define aaldevid_pipguid(devid)  ((devid).m_pipGUID)
#define aaldevid_vendorid(devid) ((devid).m_vendor)
#define aaldevid_ahmguid(devid)  ((devid).m_ahmGUID)



// EXPANDED IN-PLACE DEFINITION of AAL Device_ID as of 2011.3.6
// struct aal_device_id devid;   // Device ID
// {
//    struct aal_device_addr     m_devaddr;     // Device address
//    {
//       enum aal_bus_types_e    m_bustype;     // Type of bus E.g., FSB, QPI, PCIe
//       {
//          aal_bustype_unknown=0,
//          aal_bustype_FSB,
//          aal_bustype_QPI,
//          aal_bustype_PCIe,
//          aal_bustype_ASM,
//          aal_bustype_Prop,
//          aal_bustype_Host
//       }
//       u_int32_t               m_busnum;      // Bus number
//       u_int32_t               m_devicenum;   // device/channel number
//       int32_t                 m_subdevnum;   // Sub-device/channel number
//    }
//
//    enum aal_device_type_e     m_devicetype;  // Device type
//    {
//       aal_devtype_unknown=0,
//       aal_devtypeAHM=1,
//       aal_devtypeAFU,
//       aal_devtypeMgmtAFU
//    }
//
//    u_int32_t    m_vendor;      // Vendor ID
//
//    u_int64_t    m_pipGUID;     // PIP GUID  TODO must reconcile with btIID
//    u_int64_t    m_ahmGUID;     // AHM GUID
//    u_int64_t    m_afuGUID;     // AFU GUID
// }


//=============================================================================
// Name: Configuration Update Event
// Type[Dir]: Event [OUT]
// Object: Kernel Resource Manager Service
// Command ID: evtid_KRMS_ConfigUpdate
// Input: Payload - aalrms_configUpDateEvent
// Description: Event notification of a change in configuration state
//=============================================================================



//=============================================================================
// Name: device_attrib
// Description: Device attributes
//=============================================================================
struct device_attrib {
   void                *Handle;       // Device Handle
   btInt                state;        // 1 - if activated 0 - if quiescent

   struct aal_device_id devid;        // Device ID

   btUnsignedInt        numOwners;    // Number of owners
   btUnsignedInt        maxOwners;    // Max number of owners
   btPID                ownerlist[1]; // List of owning pids
};

// Device Add Owner result codes
typedef enum {
   aaldev_addowner_OK = 0,
   aaldev_addowner_MaxOwners,
   aaldev_addowner_DupOwner,
   aaldev_addowner_NotOwner,
   aaldev_addowner_InvalidDevice,
   aaldev_addowner_Interrupted,        // a down_interruptible returned from a signal instead of acquiring the mutex
   aaldev_addowner_SysErr = -1
} aaldev_AddOwner_e;

// Config update event types
typedef enum {
   krms_ccfgUpdate_DevAdded,
   krms_ccfgUpdate_DevRemoved,
   krms_ccfgUpdate_DevOwnerAdded,
   krms_ccfgUpdate_DevOwnerUpdated,
   krms_ccfgUpdate_DevOwnerRemoved,
   krms_ccfgUpdate_DevActivated,
   krms_ccfgUpdate_DevQuiesced
} krms_cfgUpDate_e;

struct aalrms_configUpDateEvent {
   krms_cfgUpDate_e        id;         // Type of update
   btPID                   pid;        // Used in owner Added/Removed
   struct device_attrib    devattrs;   // Device attributes
};


END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALDEVICE_H__

