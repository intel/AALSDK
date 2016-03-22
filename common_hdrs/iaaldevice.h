//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
//        FILE: iaaldevice.h
//     CREATED: 10/14/2014
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains API definitions for AAL device objects
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALDEVICE_INTERFACE_H__
#define __AALSDK_KERNEL_AALDEVICE_INTERFACE_H__
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
   union {
	   btUnsigned32bitInt   m_busnum;      // Domain:Bus number
	   struct {
		   btUnsigned16bitInt	m_domain;
		   btUnsigned16bitInt	m_bus;
	   };
   };
   btUnsigned16bitInt   m_devicenum;   // device number
   btUnsigned16bitInt   m_functnum;    // function number
   btUnsigned16bitInt   m_subdevnum;   // Sub-device/channel number
   btUnsigned16bitInt   m_instanceNum; // instance number
};
#define aaldevaddr_bustype(devaddr)         ((devaddr).m_bustype)
#define aaldevaddr_busnum(devaddr)          ((devaddr).m_busnum)
#define aaldevaddr_busdom(devaddr)          ((devaddr).m_domain)
#define aaldevaddr_bus(devaddr)             ((devaddr).m_bus)
#define aaldevaddr_devnum(devaddr)          ((devaddr).m_devicenum)
#define aaldevaddr_fcnnum(devaddr)          ((devaddr).m_functnum)
#define aaldevaddr_subdevnum(devaddr)       ((devaddr).m_subdevnum)
#define aaldevaddr_instanceNum(devaddr)     ((devaddr).m_instanceNum)

//=============================================================================
// Name: AAL Device_ID
// Description: AAL device ID used to match a discovered device with a driver
//=============================================================================
struct aal_device_id {
   struct aal_device_addr m_devaddr;     // Device address
   enum aal_device_type_e m_devicetype;  // Device type

   btUnsigned32bitInt     m_vendor;      // Vendor ID

   btUnsigned64bitInt     m_pipGUID;     // PIP GUID  TODO must reconcile with btIID
#define AAL_DEVICE_MAX_GUID_STRING_SIZE 80
   char                   m_deviguid[AAL_DEVICE_MAX_GUID_STRING_SIZE]; // Should replace PIP Guid

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

#define aaldevid_addr(devid)              	((devid).m_devaddr)
#define aaldevid_devaddr_bustype(devid)   	((aaldevid_addr(devid)).m_bustype)
#define aaldevid_devaddr_busnum(devid)    	((aaldevid_addr(devid)).m_busnum)
#define aaldevid_devaddr_busdom(devid)    	((aaldevid_addr(devid)).m_busnum)
#define aaldevid_devaddr_bus(devid)    	    ((aaldevid_addr(devid)).m_bus)
#define aaldevid_devaddr_devnum(devid)    	((aaldevid_addr(devid)).m_devicenum)
#define aaldevid_devaddr_fcnnum(devid)    	((aaldevid_addr(devid)).m_functnum)
#define aaldevid_devaddr_subdevnum(devid) 	((aaldevid_addr(devid)).m_subdevnum)
#define aaldevid_devaddr_instanceNum(devid)	((aaldevid_addr(devid)).m_instanceNum)

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
// Name:        device_attributes
// Description: 
// Comments:  
//=============================================================================
struct device_attributes {
   btWSSize                 size;                      // size with attributes
   struct aal_device_id     device_id;                 // AAL Bus device Identifer
   btObjectType            *Handle;                    // Device Handle

#define DEVICE_MAX_BASENAME_LEN   256                  // Maximum size of name base
   btByte                  basename[DEVICE_MAX_BASENAME_LEN + 1]; // Base name of device

   btUnsignedInt           maxOwners;                 // Max number of owners

#define MAFU_CONFIGURE_UNLIMTEDSHARES  (~0U)
   btUnsigned16bitInt      enableDirectAPI;
   btWSSize                extended_attrib_size;   // Length of optional attributes
   btWSSize                pub_attrib_size;        // Length of public attributes
};

#define AALBUS_INIT_DEVICE_ATTIBUTES(p)  memset( p, 0, sizeof( struct device_attributes));

#if defined( __AAL_KERNEL__ )
#define aalbus_destroy_device_attrib(p)  kosal_kfree(p, p->size) 
#else 
#define aalbus_destroy_device_attrib(p) free(p) 
#endif

#define aalbus_config_dev_pattributes(d) ( WdfObjectGetAALContext( d )->m_pdevAttributes )

// Extended attributes follow the base attributes and is a variable length object.  Returns NULL if none
#define aalbus_config_dev_pextattributes(t,p) (   p->extended_attrib_size != 0 ? ((t)((btByteArray)p+sizeof(struct device_attributes))) : NULL )

// Public attributes follow the extended attributes (if any) and is a variable length object. Returns NULL if none.
#define aalbus_config_dev_ppubattributes(t,p) ( p->pub_attrib_size != 0 ? ((t)( (btByteArray)p+sizeof(struct device_attributes) + p->extended_attrib_size) ) : NULL)


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
typedef enum aaldev_AddOwner_e{
   aaldev_addowner_OK = 0,
   aaldev_addowner_MaxOwners,
   aaldev_addowner_DupOwner,
   aaldev_addowner_NotOwner,
   aaldev_addowner_InvalidDevice,
   aaldev_addowner_Interrupted,        // a down_interruptible returned from a signal instead of acquiring the mutex
   aaldev_addowner_SysErr = -1
} aaldev_AddOwner_e;

// Config update event types
typedef enum krms_cfgUpDate_e{
   krms_ccfgUpdate_DevAdded,
   krms_ccfgUpdate_DevRemoved,
   krms_ccfgUpdate_DevOwnerAdded,
   krms_ccfgUpdate_DevOwnerUpdated,
   krms_ccfgUpdate_DevOwnerRemoved,
   krms_ccfgUpdate_DevActivated,
   krms_ccfgUpdate_DevQuiesced
}krms_cfgUpDate_e;

struct aalrms_configUpDateEvent {
   krms_cfgUpDate_e        id;         // Type of update
   btPID                   pid;        // Used in owner Added/Removed
   struct device_attrib    devattrs;   // Device attributes
};

#if defined( __AAL_KERNEL__ )
// Forward Reference
struct aal_device;
struct aaldev_owner;

// Prototype for call back used in doForeachOwner
typedef btInt (*aaldev_OwnerProcessor_t)(struct aal_device *,
                                         struct aaldev_owner *);

//=============================================================================
// Name: aaldev_ownerSession
// Description: Represents an instance of a session between a device and its
//              owner. This is the primary object shared between PIP and UIDrv.
//=============================================================================
struct aaldev_ownerSession {
   // UI Message Adaptor
   struct aal_uiapi  *m_uiapi;            // Message handler interface
   btObjectType       m_UIHandle;         // UI Handle
   btAny              m_ownerContext;     // Specific to binding owner


   // PIP
   btObjectType       m_PIPHandle; // PIP Handle
   kosal_list_head    m_wshead;    // Head of the workspace list for this device
   struct aal_device *m_device;    // Device owned by the session
};

#define aalsess_pipSendMessage(os)  (aaldev_pipmsgHandlerp((os)->m_device)->sendMessage)
#define aalsess_pipmsgID(os)        ((aaldev_pipid((os)->m_device)))
#define aalsess_uimsgHandlerp(os)   ((os)->m_uiapi)
#define aalsess_aaldevicep(os)      ((os)->m_device)
#define aalsess_aalpipp(os)         (aaldev_pipp( aalsess_aaldevicep(os) ) )
#define aalsess_pipHandle(os)       ((os)->m_PIPHandle)
#define aalsess_uiHandle(os)        ((os)->m_UIHandle)
#define aalsess_add_ws(os,ih)       kosal_list_add_head(&ih, &(os)->m_wshead);

//=============================================================================
// Name: aaldevice_interface
// Description: Public interface to aal_device
// Interface: public
// Comments:
//=============================================================================
typedef struct aaldevice_interface {
#if defined( __AAL_WINDOWS__ )
   INTERFACE      InterfaceHeader;        // TODO converge with Linux aal_interface

   struct device_attributes     *( *getDevAttributes )( WDFDEVICE );  // Fix this too
#endif
   btAny( *getExtendedAttributes )( void );
   btAny( *getPublicAttributes )( void );
   btBool( *sendDeviceUpdate )( void );

   aaldev_AddOwner_e    (*addOwner)(struct aal_device * ,
                                    btPID ,
                                    btObjectType ,
                                    btAny,
                                    pkosal_list_head );
   aaldev_AddOwner_e     (*isOwner)(struct aal_device * ,
                                    btPID );
   aaldev_AddOwner_e (*removeOwner)(struct aal_device * ,
                                    btPID );
   aaldev_AddOwner_e (*updateOwner)(struct aal_device * ,
                                    btPID ,
                                    struct aaldev_ownerSession * ,
                                    pkosal_list_head );
   struct aaldev_ownerSession *
                 (*getOwnerSession)(struct aal_device * ,
                                    btPID );

   struct aaldev_owner *
                  (*doForeachOwner)(struct aal_device * ,
                                    aaldev_OwnerProcessor_t );
   btInt                 (*quiesce)(struct aal_device * ,
                                    struct aaldev_owner * );
   btInt                  (*remove)(struct aal_device * );
   void            (*releasedevice)(pkosal_os_dev );
}AAL_DEVICE_INTERFACE, *PAAL_DEVICE_INTERFACE;


// Convenience macros
#define dev_addOwner(p,id,m,o,s)    (p)->i.addOwner(p,id,m,o,s)
#define dev_isOwner(p,id,m,s)     (p)->i.addOwner(p,id)
#define dev_removeOwner(p,id)     (p)->i.removeOwner(p,id)
#define dev_updateOwner(p,o,s,l)  (p)->i.updateOwner(p,o,s,l)
#define dev_OwnerSession(p,o)     (p)->i.getOwnerSession(p,o)
#define dev_doForeachOwner(p,f)   (p)->i.doForeachOwner(p,f)
#define dev_quiesce(p,o)          (p)->i.quiesce(p,o)
#define dev_removedevice(p)       (p)->i.remove(p)
#define dev_hasRelease(p)         (NULL != (p)->i.releasedevice)
#define dev_setrelease(p,f)       (p)->i.releasedevice=f
#define dev_release(p,d)          (p)->i.releasedevice(d)
#endif



END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALDEVICE_INTERFACE_H__

