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
//        FILE: aalui.h
//     CREATED: Sep 28, 2008
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Definitions for the AAL Universal Interface Driver
// HISTORY:
// WHEN:          WHO:     WHAT:
// 9/28/2008      JG       Initial version started
// 11/11/2008     JG       Added legal header
// 12/10/2008     JG       Added support for bound wsmgr
// 12/13/2008     HM       Adde uid_errnumNoMap to uid_errnum_e
// 12/16/2008     JG       Began support for abort and shutdown
//                         Added Support for WSID object
//                         Major interface changes.
// 12/18/2008     HM       Added aalui_PIPmessage
// 12/27/2008     JG       Added AFU response payloads
// 01/04/2009     HM       Updated Copyright
// 01/05/2009     JG       Support for additional information in AFUReponse
//                         event.
// 03/17/2009     JG       Added GetSet CSR
// 03/20/2009     JG/HM    Global change to AFU_Response that generically puts
//                            payloads after the structure with a pointer to
//                            them. Ptr must be converted kernel to user.
// 03/27/2009     JG       Added support for MGMT AFU interface
// 05/11/2009     HM       Tweaked definition of MGMT AFU interface structure
// 05/15/2009     HM       Changed error code in uid_afurespID_e
// 05/19/2009     HM       Added values to uid_afurespID_e, and changed
//                            mafu_request.payload to char*
// 06/05/2009     JG       Added shutdown
// 11/05/2009     HM       Changed size field in struct aalui_WSMParms from
//                            int to size_t. Needs to be big enough to handle
//                            any sized object.
// 12/09/2009     JG       Pulled out AFU PIP commands aalui_afucmd_e
//                         and moved it to fappip,h and defined them as FAP
//                         pip specific.
// 12/27/2009     JG       Added support for CSR Map
// 03/05/2012     JG       Added support for uid_afurespTaskStarted for SPL2
// 03/12/2012     JG       Removed linked payloads from AFUResponse
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALUI_H__
#define __AALSDK_KERNEL_AALUI_H__
#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/AALTransactionID_s.h>
#include <aalsdk/kernel/AALWorkspace.h>
#include <aalsdk/kernel/aaldevice.h>

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

//-----------------------------------------------------------------------------
// Request message IDs
//-----------------------------------------------------------------------------
typedef enum
{
   // Management
   reqid_UID_Bind=1,                // Use default API Version
   reqid_UID_ExtendedBindInfo,      // Pass additional Bind parms
   reqid_UID_UnBind,                // Release a device

   // Provision
   reqid_UID_Activate,              // Activate the device
   reqid_UID_Deactivate,            // Deactivate the device

   // Administration
   reqid_UID_Shutdown,              // Request that the Service session shutdown

   reqid_UID_SendAFU,               // Send AFU a message
   reqid_UID_SendPIP,               // Send PIP a message
   reqid_UID_SendWSM,               // Send to Workspace Manager

   // Response and Event IDs
   rspid_UID_Shutdown=0xF000,       // Service is shutdown
   rspid_UID_UnbindComplete,        // Release Device Response
   rspid_UID_BindComplete,          // Bind has completed

   rspid_UID_Activate,              // Activate the device
   rspid_UID_Deactivate,            // Deactivate the device

   rspid_AFU_Response,              // Response from AFU request
   rspid_AFU_Event,                 // Event from AFU

   rspid_PIP_Event,                 // Event from PIP

   rspid_WSM_Response,              // Event from Workspace manager

   rspid_UID_Response,
   rspid_UID_Event,

} uid_msgIDs_e;

typedef enum
{
   uid_errnumOK = 0,
   uid_errnumBadDevHandle,                       // 1
   uid_errnumCouldNotClaimDevice,                // 2
   uid_errnumNoAppropriateInterface,             // 3
   uid_errnumDeviceHasNoPIPAssigned,             // 4
   uid_errnumCouldNotBindPipInterface,           // 5
   uid_errnumCouldNotUnBindPipInterface,         // 6
   uid_errnumNotDeviceOwner,                     // 7
   uid_errnumSystem,                             // 8
   uid_errnumAFUTransaction,                     // 9
   uid_errnumAFUTransactionNotFound,             // 10
   uid_errnumDuplicateStartingAFUTransactionID,  // 11
   uid_errnumBadParameter,                       // 12
   uid_errnumNoMem,                              // 13
   uid_errnumNoMap,                              // 14
   uid_errnumBadMapping,                         // 15
   uid_errnumPermission,                         // 16
   uid_errnumInvalidOpOnMAFU,                    // 17
   uid_errnumPointerOutOfWorkspace,              // 18
   uid_errnumNoAFUBindToChannel,                 // 19
   uid_errnumCopyFromUser,                       // 20
   uid_errnumDescArrayEmpty,                     // 21
   uid_errnumCouldNotCreate,                     // 22
   uid_errnumInvalidRequest,                     // 23
   uid_errnumInvalidDeviceAddr,                  // 24
   uid_errnumCouldNotDestroy,                    // 25
   uid_errnumDeviceBusy                          // 26
} uid_errnum_e;



//=============================================================================
// Name: aalui_extbindargs
// TYpe pDir]; Request[IN] Response[OUT]
// Description: Extended bind arguments.
// Comments: This object is used to specify arguments to the PIP during the
//           bind operation and to receive attributes back in the bind complete
//=============================================================================
struct aalui_extbindargs
{
   btUnsigned64bitInt m_apiver;        // Version of message handler
   btUnsigned64bitInt m_pipver;        // Version of PIP interface
   btUnsigned32bitInt m_mappableAPI;   // Permits direct CSR mapping
   btObjectType       m_pipattrib;     // Attribute block (TBD)
};



//=============================================================================
// Name: aalui_Shutdown
// Type[Dir]: Request[IN]
// Object: UI Driver
// Description: Request to the AFU
// Comments:
//=============================================================================
typedef enum
{
   ui_shutdownReasonNormal=0,
   ui_shutdownReasonMaint,
   ui_shutdownFailure,
   ui_shutdownReasonRestart
} ui_shutdownreason_e;

struct aalui_Shutdown {
   ui_shutdownreason_e m_reason;
   btTime              m_timeout;
};


//=============================================================================
// Name: aalui_AFUmessage
// Type[Dir]: Request [IN]
// Object: AFU Engine
// Command ID: reqid_UID_SendAFU
// Input: apiver - Version (ID) of the message protocol using
//        pipver - Version (ID) of PIP expected
//           cmd - Command
//       payload - Payload of message
// Description: Used when sending a message to an AFU
// Comments: The API and PIP versions are provided as an additional check
//           that the message sent is using the appropriate protocol.
//=============================================================================
#define AALUI_AFUCMD_STARTMAFU   0xF001

struct aalui_AFUmessage
{
   btUnsigned64bitInt  apiver;     // Version of message handler [IN]
   btUnsigned64bitInt  pipver;     // Version of PIP interface [IN]
   btUnsigned64bitInt  cmd;        // Command [IN]
   btWSSize            size;       // size of payload [IN]
   btVirtAddr          payload;    // data [IN/OUT]
};

//=============================================================================
// Name: aalui_taskComplete
// Type[Dir]: Request[IN] Event/Response [OUT]
// Object: AFU engine
// Description: Single or mult0 descriptor AFU task completed
// Comments:
//=============================================================================
typedef struct aalui_taskComplete
{
   btObjectType       context;
   stTransactionID_t  afutskTranID;
   btByte             data[32];
   TTASK_MODE         mode;
   btUnsigned16bitInt delim;
} aalui_taskComplete;

//=============================================================================
// Name: csr_offset_value
// Type[Dir]: Request[IN] Event/Response [OUT]
// Object: AFU engine
// Description: Describes a CSR object and its Value
// Comments: Used for setting and getting AFU CSRs
//=============================================================================
typedef struct csr_offset_value
{
   btCSROffset csr_offset; // CSR index: 0, 1, 2, ...
   btCSRValue  csr_value;  // Value
} csr_offset_value;

//=============================================================================
// Name: csr_read_write_blk
// Type[Dir]: Request[IN] Event/Response [OUT]
// Object: AFU engine
// Description: Block of csr_offset_value objects used for getting and setting
//              blocks of CSRs
// Comments: Used for setting and getting AFU CSRs
//=============================================================================
typedef struct csr_read_write_blk
{
   btWSSize         num_to_get;   // length of get array. if 0, do not get.
   btWSSize         num_to_set;   // length of set array. if 0, do not set.
   csr_offset_value csr_array[1]; // array of csr_ofset_value's, GET first, then SET
} csr_read_write_blk;
#define csr_rwb_getarray(p) (p)->csr_array
#define csr_rwb_setarray(p) ( (p)->csr_array + (p)->num_to_get )


//=============================================================================
// Name: aalui_AFUResponse
// Type[Dir]: Event/Response [OUT]
// Object: AFU engine
// Command ID: reqid_UID_SendAFU
// fields: context - user defined data associated with descriptor
//         afutskTranID - transaction ID of AFU task
//         respID - ID code of response
//         mode - mode the descriptor ran
//         data - 32 buyte data returned by AFU
// Description: Returned AFU response to a request
// Comments:
//=============================================================================
typedef enum
{
   uid_afurespUndefinedResponse=0,
   uid_afurespInputDescriptorComplete,
   uid_afurespOutputDescriptorComplete,
   uid_afurespEndofTask,
   uid_afurespTaskStarted,
   uid_afurespTaskStopped,
   uid_afurespSetContext,
   uid_afurespTaskComplete,
   uid_afurespSetGetCSRComplete,
   uid_afurespAFUCreateComplete,
   uid_afurespAFUDestroyComplete,
   uid_afurespActivateComplete,
   uid_afurespDeactivateComplete,
   uid_afurespInitializeComplete,
   uid_afurespFreeComplete,
   uid_afurespUndefinedRequest,
   uid_afurespFirstUserResponse = 0xf000
} uid_afurespID_e;

struct aalui_AFUResponse
{
   btUnsigned32bitInt  respID;
   btUnsigned64bitInt  evtData;
   btUnsignedInt       payloadsize;
   csr_read_write_blk *pcsrBlk;
};
#define aalui_AFURespPayload(__ptr) ( ((btVirtAddr)(__ptr)) + sizeof(struct aalui_AFUResponse) )


//=============================================================================
// Name:        aalui_PIPmessage
// Type[Dir]:   Request [IN]
// Object:      PIP
// Command ID:  reqid_UID_SendPIP
// Input:       apiver - Version (ID) of the message protocol using
//              pipver - Version (ID) of PIP expected
//              cmd - Command
//              payload - Payload of message
// Description: Used when sending a message to a PIP
// Comments:    The API and PIP versions are provided as an additional check
//              that the message sent is using the appropriate protocol.
//=============================================================================
struct aalui_PIPmessage
{
   btUnsigned64bitInt apiver;     // Version of message handler [IN]
   btUnsigned64bitInt pipver;     // Version of PIP interface [IN]
   btUnsigned64bitInt cmd;        // Command [IN]
   btVirtAddr         payload;    // data [IN/OUT]
};


//=============================================================================
// Name: aalui_WSMParms
// Object: Kernel Workspace Manager Service
// Command ID: reqid_UID_SendWSM
// Input: wsid  - Workspace ID
//        ptr   - Pointer to start of workspace
//        physptr - Physical address
//        size  - size in bytes of workspace
// Description: Parameters describing a workspace
// Comments: Used in WSM interface
//=============================================================================
struct aalui_WSMParms
{
   btWSID     wsid;        // Workspace ID
   btVirtAddr ptr;         // Virtual Workspace pointer
   btPhysAddr physptr;     // Depends on use
   btWSSize   size;        // Workspace size
   btWSSize   itemsize;    // Workspace item size
   btWSSize   itemspacing; // Workspace item spacing
   TTASK_MODE type;        // Task mode this workspace is compatible with
};


//=============================================================================
// Name: aalui_WSMEvent
// Type[Dir]: Event/Response [OUT]
// Object: Kernel Workspace Manager Service
// Command ID: reqid_UID_SendWSM
// fields: wsid - Workspace ID
//         ptr  - Pointer tostart of workspace
//         size - size in bytes of workspace
// Description: Parameters describing a workspace
// Comments: Used in WSM interface
//=============================================================================
typedef enum
{
   uid_wseventAllocate=0,
   uid_wseventFree,
   uid_wseventGetPhys,
   uid_wseventCSRMap
} uid_wseventID_e;

struct aalui_WSMEvent
{
   uid_wseventID_e       evtID;
   struct aalui_WSMParms wsParms;
};


//=============================================================================
// Name: aalui_ioctlreq
// Description: IOCTL message block
//=============================================================================
struct aalui_ioctlreq
{
   uid_msgIDs_e       id;      // ID of UI request [IN]
   stTransactionID_t  tranID;  // Transaction ID to identify result [IN]
   btObjectType       context; // Optional token [IN]
   uid_errnum_e       errcode; // Driver specific error number
   btHANDLE           handle;  // Device handle
   btWSSize           size;    // Size of payload section [IN]
//   btVirtAddr         payload; // Pointer to optional payload [IN]
};

// TODO CASSERT( sizeof(struct aalui_ioctlreq) 

#define aalui_ioAFUmessagep(i)  	(struct aalui_AFUmessage *)(((char *)i) + sizeof(struct aalui_ioctlreq) )
#define aalui_ioctlPayload(i)   	(void *)(((char *)i) + sizeof(struct aalui_ioctlreq) )
#define aalui_ioctlPayloadSize(i)   ((i)->size)

#if   defined( __AAL_LINUX__ )
# define AALUID_IOCTL_SENDMSG       _IOR ('x', 0x00, struct aalui_ioctlreq)
# define AALUID_IOCTL_GETMSG_DESC   _IOR ('x', 0x01, struct aalui_ioctlreq)
# define AALUID_IOCTL_GETMSG        _IOWR('x', 0x02, struct aalui_ioctlreq)
# define AALUID_IOCTL_BINDDEV       _IOWR('x', 0x03, struct aalui_ioctlreq)
# define AALUID_IOCTL_ACTIVATEDEV   _IOWR('x', 0x04, struct aalui_ioctlreq)
# define AALUID_IOCTL_DEACTIVATEDEV _IOWR('x', 0x05, struct aalui_ioctlreq)
#elif defined( __AAL_WINDOWS__ )
# ifdef __AAL_USER__
#    include <winioctl.h>
# endif // __AAL_USER__

// CTL_CODE() bits:
//     Common[31]
// DeviceType[30:16]
//     Access[15:14]
//     Custom[13]
//   Function[12:2] (or [10:0] << 2)
//     Method[1:0]

# define UAIA_DRV_ID                  0x2
# define UAIA_DEVICE_TYPE             FILE_DEVICE_BUS_EXTENDER
# define UAIA_ACCESS                  FILE_ANY_ACCESS
# define UAIA_CUSTOM                  1
# define UAIA_METHOD                  METHOD_BUFFERED

// field extractors
# define AAL_IOCTL_DRV_ID(__ctl_code) (((__ctl_code) >> 10) & 0x7)
# define AAL_IOCTL_FN(__ctl_code)     (((__ctl_code) >> 2) & 0xff)

                  // 0 - 0xff
# define UAIA_IOCTL(__index)          CTL_CODE(UAIA_DEVICE_TYPE, (UAIA_CUSTOM << 11) | (UAIA_DRV_ID << 8) | (__index), UAIA_METHOD, UAIA_ACCESS)

# define AALUID_IOCTL_SENDMSG         UAIA_IOCTL(0x00)
# define AALUID_IOCTL_GETMSG_DESC     UAIA_IOCTL(0x01)
# define AALUID_IOCTL_GETMSG          UAIA_IOCTL(0x02)
# define AALUID_IOCTL_BINDDEV         UAIA_IOCTL(0x03)
# define AALUID_IOCTL_ACTIVATEDEV     UAIA_IOCTL(0x04)
# define AALUID_IOCTL_DEACTIVATEDEV   UAIA_IOCTL(0x05)
# define AALUID_IOCTL_POLL            UAIA_IOCTL(0x06)
# define AALUID_IOCTL_MMAP            UAIA_IOCTL(0x07)

#endif // OS

END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALUI_H__

