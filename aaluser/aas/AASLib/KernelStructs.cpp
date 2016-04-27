// Copyright(c) 2008-2016, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file KernelStructs.cpp
/// @brief Print Utilities used when working with the Kernel Structures.
/// @ingroup ResMgr
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/10/2008     HM       Initial version started
/// 11/11/2008     HM       Extensive modifications
/// 11/13/2008     HM       Cosmetic changes for easier to read dumps
/// 12/13/2008     HM       Added some of the UserInterface structs and enums to
///                            the previous ResourceManager ones
/// 01/04/2009     HM       Updated Copyright
/// 02/07/2009     HM       Added LogLevel_t
/// 03/06/2009     HM       Updated operator << for rms_msgIDs_e
/// 03/20/2009     HM       Added more kernel structures
/// 03/27/2009     JG       Added support for MGMT AFU interface
///                         changed  some enums to match external changes
/// 05/15/2009     HM       Changed error code in uid_afurespID_e forced update
///                            as well as others
/// 11/19/2009     JG       Added support for more uid_errnum_e types
/// 12/09/2009     JG       Pulled out AFU PIP commands aalui_afucmd_e
///                            and moved it to fappip.h and defined them as FAP
///                            pip specific.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"

#include "aalsdk/kernel/KernelStructs.h"
#include "aalsdk/utils/ResMgrUtilities.h"   // for GUIDStructFromU64, GUIDStringFromStruct

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////                                        ////////////////////
//////////                      A A L T Y P E S                      //////////
///////////////////                                        ////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name:        std::ostream& operator << of btVirtAddr
// Description: btVirtAddr is a char* but needs to print as a void*
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const btVirtAddr &pVirt)
{
   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << static_cast<void*>(pVirt);

   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of btVirtAddr

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////                                        ////////////////////
//////////                         L I N U X                         //////////
///////////////////                                        ////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:        std::ostream& operator << of LogLevel_t
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const LogLevel_t &loglevel)
{
#define LogLevel_t_CASE(x, msg) case x : s << msg << static_cast<int>(loglevel); break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (loglevel) {
      // Note: order is an attempt to optimize.

      // NOT a LINUX level. AAL for really verbose output.
      LogLevel_t_CASE(eLogLevel_Verbose,       "LOG_VERBOSE = ");
      LogLevel_t_CASE(eLogLevel_Debug,         "LOG_DEBUG = "  );
      LogLevel_t_CASE(eLogLevel_Warning,       "LOG_WARNING = ");
      LogLevel_t_CASE(eLogLevel_Error,         "LOG_ERR = "    );
      LogLevel_t_CASE(eLogLevel_Notice,        "LOG_NOTICE = " );
      LogLevel_t_CASE(eLogLevel_Emergency,     "LOG_EMERG = "  );
      LogLevel_t_CASE(eLogLevel_Alert,         "LOG_ALERT = "  );
      LogLevel_t_CASE(eLogLevel_Critical,      "LOG_CRIT = "   );
      LogLevel_t_CASE(eLogLevel_Informational, "LOG_INFO = "   );

      default:
         s << "LogLevel_t is " << static_cast<int>(loglevel) <<
            ": but operator<< for LogLevel_t is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of LogLevel_t

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////                                        ////////////////////
//////////             R E S O U R C E   M A N A G E R               //////////
///////////////////                                        ////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:        std::ostream& operator << of stTransactionID_t
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const stTransactionID_t &tranid)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase << std::boolalpha <<
         "stTransactionID_t at " << reinterpret_cast<const void*>(&tranid) <<
         "\n\tContextPtr:      " << reinterpret_cast<const void*>(tranid.m_Context) <<
         "\n\tEventHandler:    " << reinterpret_cast<const void*>(tranid.m_Handler) <<
         "\n\tFilter:          " << tranid.m_Filter <<
         "\n\tId:              " << tranid.m_intID <<
         std::endl;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of stTransactionID_t

//=============================================================================
// Name:        std::ostream& operator << of aal_bus_types_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aal_bus_types_e &bustype)
{
#define aal_bus_types_e_CASE(x, msg) case x : s << msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();


   switch (bustype) {

      // Note: order is an attempt to optimize.
      aal_bus_types_e_CASE(aal_bustype_QPI,     "QPI "        );
      aal_bus_types_e_CASE(aal_bustype_PCIe,    "PCIe "       );
      aal_bus_types_e_CASE(aal_bustype_Host,    "Host "       );
      aal_bus_types_e_CASE(aal_bustype_ASM,     "ASM "        );
      aal_bus_types_e_CASE(aal_bustype_FSB,     "FSB "        );
      aal_bus_types_e_CASE(aal_bustype_Prop,    "Proprietary ");
      aal_bus_types_e_CASE(aal_bustype_unknown, "Unknown "    );

      default:
         s << "aal_bus_types_e is " << static_cast<unsigned>(bustype) <<
            ": but operator<< for aal_bus_types_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aal_bus_types_e

//=============================================================================
// Name:        std::ostream& operator << of aal_device_type_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aal_device_type_e &devtype)
{
#define aal_device_type_e_CASE(x, msg) case x : s << msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (devtype) {

      // Note: order is an attempt to optimize.
      aal_device_type_e_CASE(aal_devtypeMgmtAFU, "Management AFU ");
      aal_device_type_e_CASE(aal_devtypeAHM,     "AHM "           );
      aal_device_type_e_CASE(aal_devtypeAFU,     "AFU "           );

      case aal_devtype_unknown: /* FALL THROUGH */
      default:
         s << "aal_device_type_e is " << static_cast<unsigned>(devtype) <<
            ": but operator<< for aal_device_type_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aal_device_type_e

//=============================================================================
// Name:        std::ostream& operator << of aal_device_addr
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aal_device_addr &devaddr)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
         "aal_device_addr at ->  " << reinterpret_cast<const void*>(&devaddr) << "\n\t"
                 "bustype:       " << devaddr.m_bustype << "\n\t"
                 "busnum:        " << devaddr.m_busnum << "\n\t"
                 "devicenum:     " << devaddr.m_devicenum << "\n\t"
                 "functnum:      " << devaddr.m_functnum<< "\n\t"
                 "subdevnum:     " << devaddr.m_subdevnum << "\n\t"
                 "instance:      " << devaddr.m_instanceNum << "\n";

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aal_device_addr

//=============================================================================
// Name:        std::ostream& operator << of aal_device_id
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aal_device_id &devid)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
         "aal_device_id at --->" << reinterpret_cast<const void*>(&devid) << "\n\t"
                 "DeviceType:  " << devid.m_devicetype << "\n\t"
                 "vendor:      " << devid.m_vendor << "\n\t"
                 "pipGUID:     " << devid.m_pipGUID << "\n\t"
                 "ahmGUID:     " << devid.m_ahmGUID << "\n\t"
                 "afuGUID:     " << devid.m_afuGUIDh << " " << devid.m_afuGUIDl <<
                 "\t"            << GUIDStringFromStruct(GUIDStructFrom2xU64(devid.m_afuGUIDh, devid.m_afuGUIDl)) <<
//                 "\t"            << GUIDStringFromStruct(GUIDStructFromU64(devid.m_afuGUID)) <<
         "\n" << devid.m_devaddr;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aal_device_id

//=============================================================================
// Name:        std::ostream& operator << of device_attrib
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const device_attrib &devattr)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
         "device_attrib at --->" << reinterpret_cast<const void*>(&devattr) << "\n\t"
                 "Handle:      " << devattr.Handle << "\n\t"
                 "State:       " << (devattr.state ? "Activated" : "Quiescent") << "\n\t"
                 "MaxOwners:   " << devattr.maxOwners << "\n\t"
                 "NumOwners:   " << devattr.numOwners << "\n";

   for (unsigned i = 0; i < devattr.numOwners; ++i) {
      s << "\t\tPID: " << std::setw(10) << devattr.ownerlist[i] << std::dec << std::setw(10) << devattr.ownerlist[i] << "\n";
   }

   s << devattr.devid;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of device_attrib

//=============================================================================
// Name:        std::ostream& operator << of krms_cfgUpDate_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const krms_cfgUpDate_e &update)
{
#define krms_cfgUpDate_e_CASE(x, msg) case x : s << msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (update) {

      // Note: order is an attempt to optimize.
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevAdded,        "DevAdded "       );
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevRemoved,      "DevRemoved "     );
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevOwnerAdded,   "DevOwnerAdded "  );
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevOwnerUpdated, "DevOwnerUpdated ");
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevOwnerRemoved, "DevOwnerRemoved ");
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevActivated,    "DevActivated "   );
      krms_cfgUpDate_e_CASE(krms_ccfgUpdate_DevQuiesced,     "DevQuiesced "    );

      default:
         s << "krms_cfgUpDate_e is " << static_cast<unsigned>(update) <<
            ": but operator<< for krms_cfgUpDate_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of krms_cfgUpDate_e

//=============================================================================
// Name:        std::ostream& operator << of aalrms_configUpDateEvent
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aalrms_configUpDateEvent &event)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
         "aalrms_configUpDateEvent at " << reinterpret_cast<const void*>(&event) <<
         "\n\tUpdate Type: " << event.id <<
         "\n\tPID:         " << event.pid << " " << std::dec << event.pid <<
         "\n\tDevice Attr: " << event.devattrs <<
         std::endl;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aalrms_configUpDateEvent

//=============================================================================
// Name:        std::ostream& operator << of rms_msgIDs_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const rms_msgIDs_e &enumeration)
{
#define rms_msgIDs_e_CASE(x, msg) case x : s << #x ": " msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      rms_msgIDs_e_CASE(reqid_URMS_RequestDevice,    "Device Allocation Request "                 );
      rms_msgIDs_e_CASE(reqid_URMS_ReleaseDevice,    "Device Release Request "                    );
      rms_msgIDs_e_CASE(reqid_RS_Registrar,          "Registrar Request "                         );
      rms_msgIDs_e_CASE(reqid_RM_DeviceRequest,      "Request configuration of a device "         );
      rms_msgIDs_e_CASE(reqid_Shutdown,              "Request to shutdown Service session "       );
      rms_msgIDs_e_CASE(reqid_Restart,               "Request to restart KRMS without close/open ");
      rms_msgIDs_e_CASE(reqid_KRMS_SetConfigUpdates, "Used to set configuration updates "         );
      rms_msgIDs_e_CASE(rspid_URMS_RequestDevice,    "Device Allocation Response "                );
      rms_msgIDs_e_CASE(rspid_RS_Registrar,          "Registrar Response "                        );
      rms_msgIDs_e_CASE(rspid_RM_DeviceRequest,      "Response "                                  );
      rms_msgIDs_e_CASE(evtid_KRMS_ConfigUpdate,     "Configuration Update "                      );
      rms_msgIDs_e_CASE(rspid_Shutdown,              "Service is shutdown "                       );
      rms_msgIDs_e_CASE(rspid_Started,               "Service is restarted "                      );

      default:
         s << "rms_msgIDs_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for rms_msgIDs_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of rms_msgIDs_e

//=============================================================================
// Name:        std::ostream& operator << of rms_result_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const rms_result_e &enumeration)
{
#define rms_result_e_CASE(x) case x : s << #x " "; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      rms_result_e_CASE(rms_resultOK);
      rms_result_e_CASE(rms_resultMaxOwnersErr);
      rms_result_e_CASE(rms_resultDuplicateOwnerErr);
      rms_result_e_CASE(rms_resultNotOwnerErr);
      rms_result_e_CASE(rms_resultInvalidDevice);
      rms_result_e_CASE(rms_resultErrno);
      rms_result_e_CASE(rms_resultBadParm);
      rms_result_e_CASE(rms_resultCancelled);
      rms_result_e_CASE(rms_resultDeviceHasNoPIPAssigned);
      rms_result_e_CASE(rms_resultNoAppropriateInterface);
      rms_result_e_CASE(rms_resultSystemErr);

      default:
         s << "rms_result_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for rms_result_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of rms_result_e

//=============================================================================
// Name:        std::ostream& operator << of aalrms_requestdevice
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aalrms_requestdevice &reqdev)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
         "aalrms_requestdevice at " << reinterpret_cast<const void*>(&reqdev) <<
         "\n\tPID:          " << reqdev.pid << " " << std::dec << reqdev.pid << std::hex <<
         "\n\tSize:         " << reqdev.size <<
         "\n\tManifest at   " << reqdev.manifest <<
         "\n\tManifest data:\n" << *(static_cast<NamedValueSet *>(reqdev.manifest)) <<
         std::endl;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of aalrms_requestdevice

//=============================================================================
// Name:        std::ostream& operator << of aalrm_ioctlreq
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const aalrm_ioctlreq &reqdev)
{
   // remember flag and fill state
   std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << std::hex << std::uppercase << std::showbase <<
   "aalrm_ioctlreq at " << reinterpret_cast<const void*> ( &reqdev ) <<
   "\n\tMessage Type:       " << reqdev.id <<
   "\n\tResult  Code:       " << reqdev.result_code <<
   "\n\tRequest Handle:     " << reqdev.req_handle <<
   "\n\tPayload Pointer:    " << (void*)reqdev.payload <<
   "\n\tPayload Size:       " << reqdev.size << " " << std::dec << reqdev.size <<
   "\n\tUnion   Data:       " << reqdev.data <<
   "\n\tUnion   Res Handle: " << std::hex << reqdev.data <<
   "\n\t" << reqdev.tranID <<
   std::endl;

   // reset flag and fill state
   // s.fill(defaultFillChar);
   s.flags ( defaultFlags );
   return s;
} // std::ostream& operator << of aalrm_ioctlreq

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////   a a l u i . h  //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:        std::ostream& operator << of uid_msgIDs_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const uid_msgIDs_e &enumeration)
{
#define uid_msgIDs_e_CASE(x, msg) case x : s << #x ": " msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      uid_msgIDs_e_CASE(reqid_UID_Bind,             "Bind, and use default API Version "        );
      uid_msgIDs_e_CASE(reqid_UID_ExtendedBindInfo, "Pass additional Bind parms "               );
      uid_msgIDs_e_CASE(reqid_UID_UnBind,           "Release a device "                         );
      uid_msgIDs_e_CASE(reqid_UID_Activate,         "Activate the device "                      );
      uid_msgIDs_e_CASE(reqid_UID_Deactivate,       "Deactivate the device "                    );
      uid_msgIDs_e_CASE(reqid_UID_Shutdown,         "Request that the Service session shutdown ");
      uid_msgIDs_e_CASE(reqid_UID_SendAFU,          "Send AFU a message "                       );
      uid_msgIDs_e_CASE(rspid_UID_Shutdown,         "Service is shutdown "                      );
      uid_msgIDs_e_CASE(rspid_UID_UnbindComplete,   "Release Device Response "                  );
      uid_msgIDs_e_CASE(rspid_UID_BindComplete,     "Bind has completed "                       );
      uid_msgIDs_e_CASE(rspid_UID_Activate,         "Activate the device "                      );
      uid_msgIDs_e_CASE(rspid_UID_Deactivate,       "Deactivate the device "                    );
      uid_msgIDs_e_CASE(rspid_AFU_Response,         "Response from AFU request "                );
      uid_msgIDs_e_CASE(rspid_AFU_Event,            "Event from AFU "                           );

      uid_msgIDs_e_CASE(rspid_AFU_PR_Revoke_Event,  "Event form PR to Revoke AFU"               );

      uid_msgIDs_e_CASE(rspid_PIP_Event,            "Event from PIP "                           );
      uid_msgIDs_e_CASE(rspid_WSM_Response,         "Event from Workspace manager "             );
      uid_msgIDs_e_CASE(rspid_UID_Response,         "Generic Response "                         );
      uid_msgIDs_e_CASE(rspid_UID_Event,            "Generic Event "                            );

      default:
         s << "uid_msgIDs_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for uid_msgIDs_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of uid_msgIDs_e

//=============================================================================
// Name:        print_uid_errnum_e
// Description: worker for operator << for uid_errnum_e
//=============================================================================
static
std::ostream & print_uid_errnum_e(std::ostream &s, const uid_errnum_e &enumeration)
{
#define uid_errnum_e_CASE(x, msg) case x : s << #x " (" << ((int)x) << "): " msg; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      uid_errnum_e_CASE(uid_errnumOK,                                "");
      uid_errnum_e_CASE(uid_errnumBadDevHandle,                      "");
      uid_errnum_e_CASE(uid_errnumCouldNotClaimDevice,               "");
      uid_errnum_e_CASE(uid_errnumNoAppropriateInterface,            "");
      uid_errnum_e_CASE(uid_errnumDeviceHasNoPIPAssigned,            "");
      uid_errnum_e_CASE(uid_errnumCouldNotBindPipInterface,          "");
      uid_errnum_e_CASE(uid_errnumCouldNotUnBindPipInterface,        "");
      uid_errnum_e_CASE(uid_errnumNotDeviceOwner,                    "");
      uid_errnum_e_CASE(uid_errnumSystem,                            "");
      uid_errnum_e_CASE(uid_errnumAFUTransaction,                    "");
      uid_errnum_e_CASE(uid_errnumAFUTransactionNotFound,            "");
      uid_errnum_e_CASE(uid_errnumDuplicateStartingAFUTransactionID, "");
      uid_errnum_e_CASE(uid_errnumBadParameter,                      "");
      uid_errnum_e_CASE(uid_errnumNoMem,                             "could not allocate memory "                               );
      uid_errnum_e_CASE(uid_errnumNoMap,                             "mmap failed "                                             );
      uid_errnum_e_CASE(uid_errnumBadMapping,                        "the result of mmap() overlapped another workspace "       );
      uid_errnum_e_CASE(uid_errnumPermission,                        "Management-specific Operation sent to Non-Management AFU ");
      uid_errnum_e_CASE(uid_errnumInvalidOpOnMAFU,                   "Management AFU could not handle Operation "               );
      uid_errnum_e_CASE(uid_errnumPointerOutOfWorkspace,             "pointer to descriptor is out of range "                   );
      uid_errnum_e_CASE(uid_errnumNoAFUBindToChannel,                "No AFU bind to the Channel "                              );
      uid_errnum_e_CASE(uid_errnumCopyFromUser,                      "Copying data from the user space failed "                 );
      uid_errnum_e_CASE(uid_errnumDescArrayEmpty,                    "The descriptor array submitted to kernel is empty "       );
      uid_errnum_e_CASE(uid_errnumCouldNotCreate,                    "Could not create object "                                 );
      uid_errnum_e_CASE(uid_errnumInvalidRequest,                    "The request was unknown or not supported "                );
      uid_errnum_e_CASE(uid_errnumInvalidDeviceAddr,                 "");
      uid_errnum_e_CASE(uid_errnumCouldNotDestroy,                   "");
      uid_errnum_e_CASE(uid_errnumDeviceBusy,                        "");

      default:
         s << "uid_errnum_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for uid_errnum_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
}  // print_uid_errnum_e


//=============================================================================
// Name:        std::ostream& operator << of uid_errnum_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const uid_errnum_e &enumeration)
{
   return print_uid_errnum_e(s, enumeration);
} // std::ostream& operator << of uid_errnum_e

#if 0
//=============================================================================
// Name:        std::ostream& operator << of uid_errnum_e
// Description: writes a description of the object to the ostream
//=============================================================================
std::ostream& operator << (std::ostream& s, const uid_errnum_e& enumeration)
{
   return print_uid_errnum_e( s, enumeration);
} // std::ostream& operator << of uid_errnum_e
#endif

#if 0
//=============================================================================
// Name:        std::ostream& operator << of fappip_afuCmdID_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const fappip_afuCmdID_e &enumeration)
{
#define fappip_afuCmdID_e_CASE(x) case x : s << #x ": "; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      fappip_afuCmdID_e_CASE(fappip_afucmdWKSP_ALLOC);
      fappip_afuCmdID_e_CASE(fappip_afucmdWKSP_VALLOC);
      fappip_afuCmdID_e_CASE(fappip_afucmdWKSP_FREE);
      fappip_afuCmdID_e_CASE(fappip_afucmdWKSP_VFREE);
      fappip_afuCmdID_e_CASE(fappip_afucmdWKSP_GET_PHYS);
      fappip_afuCmdID_e_CASE(fappip_afucmdDESC_SUBMIT);
      fappip_afuCmdID_e_CASE(fappip_afucmdDESC_QUERY);
      fappip_afuCmdID_e_CASE(fappip_afucmdTASK_ABORT);
      fappip_afuCmdID_e_CASE(fappip_afucmdSTART_SPL2_TRANSACTION);
      fappip_afuCmdID_e_CASE(fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE);
      fappip_afuCmdID_e_CASE(fappip_afucmdCSR_GETSET);
      fappip_afuCmdID_e_CASE(fappip_afucmdDESC_MULTISUBMIT);
      fappip_afuCmdID_e_CASE(fappip_getCSRmap);

      default:
         s << "fappip_afuCmdID_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for fappip_afuCmdID_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of fappip_afuCmdID_e
#endif

//=============================================================================
// Name:        std::ostream& operator << of uid_mgtAfuCmdID_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const uid_mgtAfuCmdID_e &enumeration)
{
#define uid_mgtAfuCmdID_e_CASE(x) case x : s << #x ": "; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      uid_mgtAfuCmdID_e_CASE(aalui_mafucmd);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdActvateAFU);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdDeactvateAFU);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdInitializeAFU);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdFreeAFU);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdCreateAFU);
      uid_mgtAfuCmdID_e_CASE(aalui_mafucmdDestroyAFU);

      default:
         s << "uid_mgtAfuCmdID_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for uid_mgtAfuCmdID_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of uid_mgtAfuCmdID_e

//=============================================================================
// Name:        std::ostream& operator << of uid_afurespID_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const uid_afurespID_e &enumeration)
{
#define uid_afurespID_e_CASE(x) case x : s << #x ": "; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {
         // uid_afurespUndefinedResponse used in case more generic code cannot
         // tell what this AFUResponse is and returns an error. This is just filler.
      uid_afurespID_e_CASE(uid_afurespUndefinedResponse);
      uid_afurespID_e_CASE(uid_afurespInputDescriptorComplete);
      uid_afurespID_e_CASE(uid_afurespOutputDescriptorComplete);
      uid_afurespID_e_CASE(uid_afurespEndofTask);
      uid_afurespID_e_CASE(uid_afurespTaskStarted);
      uid_afurespID_e_CASE(uid_afurespSetContext);
      uid_afurespID_e_CASE(uid_afurespTaskComplete);
      uid_afurespID_e_CASE(uid_afurespSetGetCSRComplete);
      uid_afurespID_e_CASE(uid_afurespAFUCreateComplete);
      uid_afurespID_e_CASE(uid_afurespAFUDestroyComplete);
      uid_afurespID_e_CASE(uid_afurespActivateComplete);
      uid_afurespID_e_CASE(uid_afurespDeactivateComplete);
      uid_afurespID_e_CASE(uid_afurespInitializeComplete);
      uid_afurespID_e_CASE(uid_afurespFreeComplete);
      uid_afurespID_e_CASE(uid_afurespUndefinedRequest);

      default:
         s << "uid_afurespID_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for uid_afurespID_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of uid_afurespID_e

//=============================================================================
// Name:        std::ostream& operator << of uid_wseventID_e
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const uid_wseventID_e &enumeration)
{
#define uid_wseventID_e_CASE(x) case x : s << #x ": "; break

   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   switch (enumeration) {

      uid_wseventID_e_CASE(uid_wseventAllocate);
      uid_wseventID_e_CASE(uid_wseventFree);
      uid_wseventID_e_CASE(uid_wseventGetPhys);
//      uid_wseventID_e_CASE(uid_wseventCSRMap);

      default:
         s << "uid_wseventID_e is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for uid_wseventID_e is out of date or the field is invalid";
         ASSERT(false);
      break;

   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of uid_wseventID_e

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////    A A L W o r k s p a c e . h    ////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:        std::ostream& operator << of TTASK_MODE
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const TTASK_MODE &taskmode)
{
#define TTASK_MODE_CASE(x, msg) case x : s << msg; break
   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << "AAL::";

   switch (taskmode) {

      TTASK_MODE_CASE(MASTER_PHYS_MODE, "Physical Master Mode "  );
      TTASK_MODE_CASE(MASTER_VIRT_MODE, "Virtual Master Mode "   );
      TTASK_MODE_CASE(SLAVE_MODE,       "Slave (or Serial) Mode ");

      default:
         s << "TTASK_MODE is " << static_cast<unsigned>(taskmode) <<
            ": but operator<< for TTASK_MODE is out of date or the field is invalid";
         ASSERT(false);
      break;
   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of TTASK_MODE

//=============================================================================
// Name:        std::ostream& operator << of TDESC_TYPE
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const TDESC_TYPE &enumeration)
{
#define TDESC_TYPE_CASE(x, msg) case x : s << msg; break
   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << "AAL::";

   switch (enumeration) {

      TDESC_TYPE_CASE(INPUT_DESC,  "Input Buffer " );
      TDESC_TYPE_CASE(OUTPUT_DESC, "Output Buffer ");

      default:
         s << "TDESC_TYPE is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for TDESC_TYPE is out of date or the field is invalid";
         ASSERT(false);
      break;
   }
   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of TDESC_TYPE

//=============================================================================
// Name:        std::ostream& operator << of TDESC_POSITION
// Description: writes a description of the object to the ostream
//=============================================================================
AASLIB_API
std::ostream & operator << (std::ostream &s, const TDESC_POSITION &enumeration)
{
#define TDESC_POSITION_CASE(x, msg) case x : s << msg; break
   // remember flag and fill state
   // std::ios::fmtflags defaultFlags = s.flags();
   // char defaultFillChar = s.fill();

   s << "AAL::";

   switch (enumeration) {

      TDESC_POSITION_CASE(START_OF_TASK,  "SOT "        );
      TDESC_POSITION_CASE(MIDDLE_OF_TASK, "MOT "        );
      TDESC_POSITION_CASE(END_OF_TASK,    "EOT "        );
      TDESC_POSITION_CASE(COMPLETE_TASK,  "SOT MOT EOT ");

      default:
         s << "TDESC_POSITION is " << static_cast<unsigned>(enumeration) <<
            ": but operator<< for TDESC_POSITION is out of date or the field is invalid";
         ASSERT(false);
      break;
   }

   // reset flag and fill state
   // s.fill(defaultFillChar);
   // s.flags(defaultFlags);
   return s;
} // std::ostream& operator << of TDESC_POSITION

END_NAMESPACE(AAL)


AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::btVirtAddr &x)               { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::LogLevel_t &x)               { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::stTransactionID_t &x)        { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aal_bus_types_e &x)          { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aal_device_type_e &x)        { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aal_device_addr &x)          { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aal_device_id &x)            { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::device_attrib &x)            { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::krms_cfgUpDate_e &x)         { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aalrms_configUpDateEvent &x) { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::rms_msgIDs_e &x)             { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::rms_result_e &x)             { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aalrms_requestdevice &x)     { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::aalrm_ioctlreq &x)           { return AAL::operator << (s, x); }
//AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::fappip_afuCmdID_e &x)        { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::uid_msgIDs_e &x)             { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::uid_errnum_e &x)             { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::uid_mgtAfuCmdID_e &x)        { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::uid_afurespID_e &x)          { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::uid_wseventID_e &x)          { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::TTASK_MODE &x)               { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::TDESC_TYPE &x)               { return AAL::operator << (s, x); }
AASLIB_API std::ostream & operator << (std::ostream &s, const AAL::TDESC_POSITION &x)           { return AAL::operator << (s, x); }

