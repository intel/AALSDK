// Copyright(c) 2015-2016, Intel Corporation
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
/// @file IPwrMgr.h
/// @brief Power Manager Interface
/// @ingroup IPwrMgr
/// @verbatim
/// Accelerator Abstraction Layer
/// Power Manager Interface (IPwrMgr) definition
///
/// This header defines the various interfaces exposed by the FPGA Power
/// Interface. There is no specific separate IPwrMgr class. The individual
/// interfaces are:
///
///    IPwrMgr   Functions for accessing Power manager Resource and
///              Power manager response
///    IPwrMgr_Client   Functions for power Request callback.
///
/// }
/// @endcode
///
/// AUTHORS: Ananda Ravuri, Intel Corporation

///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/2015     AR       Initial version derived from CCI
//****************************************************************************
#ifndef __AALSDK_SERVICE_IPWRMGR_H__
#define __AALSDK_SERVICE_IPWRMGR_H__
#include <aalsdk/OSAL.h>
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/INTCDefs.h>
#include <aalsdk/AASLib.h>
#include <aalsdk/service/IALIAFU.h>

BEGIN_NAMESPACE(AAL)

//-----------------------------------------------------------------------------
// IPwrMgr Interface GUID.
//-----------------------------------------------------------------------------
#define CCIP_PWR_GUID "93136515-D527-451A-99DE-979A70CEE6C9"

//-----------------------------------------------------------------------------
// IPwrMgr Interface IIDs.
//-----------------------------------------------------------------------------
#define iid_PWRMGR_Service            __INTC_IID(INTC_sysAFULinkInterface,0x0100)
#define iid_PWRMGR_Service_Client     __INTC_IID(INTC_sysAFULinkInterface,0x0101)

//-----------------------------------------------------------------------------
// IPwrMgr interface.
//-----------------------------------------------------------------------------
/// @brief  Allocates Power Resource and responds to power response .
///
/// @note   This service interface is obtained from an IBase via iid_PWRMGR_Service
/// @code
///         m_pPwrMgrService = dynamic_ptr<IPwrMgr>(iid_PWRMGR_Service, pServiceBase);
/// @endcode
///
class IPwrMgr
{
public:

   #define PWRMGR_DATATYPE                  btInt
   #define PWRMGMT_STATUS                  "Reconf Power Management Status"
   virtual ~IPwrMgr() {}

   /// @brief sends reconfiguration power reauest's response .
   ///
   ///
   /// @param[in]  rTranID     Power response Transaction ID.
   /// @param[in]  rInputArgs  rInputArgs  input Arguments.
   /// @return     btBool      returns status of Transaction .
   virtual btBool reconfPowerResponse(TransactionID const &rTranID ,
                                      NamedValueSet const &rInputArgs) = 0;
};

//-----------------------------------------------------------------------------
// IPwrMgr_Client service client interface.
//-----------------------------------------------------------------------------
/// @brief  PR power Callback
///
/// @note   This interface is implemented by the client and set in the IBase
///         of the client object as an iid_PWRMGR_Service_Client.
/// @code
///         SetInterface(iid_PWRMGR_Service_Client, dynamic_cast<IPwrMgr_Client *>(this));
/// @endcode
///
class IPwrMgr_Client
{
public:

   #define PWRMGRCLIENT_DATATYPE             btInt
   #define PWRMGR_SOCKETID                  "SocketID"
   #define PWRMGR_BUSID                     "BusID"
   #define PWRMGR_DEVICEID                  "DeviceID"
   #define PWRMGR_FUNID                     "FunctionID"
   #define PWRMGR_RECONF_PWRREQUIRED        "Reconf_PwrRequired"

   virtual ~IPwrMgr_Client() {}

   /// @brief Configuration successful.
   ///
   /// @param[in]   rTranID PR Power Transition ID
   /// @param[in]   rEvent An  event  describing the pr power request .
   /// @param[in]   rResult power manager required information.
   /// @return      void.
   ///
   virtual void reconfPowerRequest( TransactionID &rTranID,IEvent const &rEvent ,INamedValueSet &rInputArgs) = 0;

};

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_IPWRMGR_H__

