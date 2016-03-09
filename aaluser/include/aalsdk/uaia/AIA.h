// Copyright(c) 2007-2016, Intel Corporation
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
/// @file AIA.h
/// @brief AIA - Defines for AIA packages.
/// @ingroup uAIA
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/25/2007     JG       Initial version started
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Comments & License
/// 11/14/2008     JG       Redesigned for AAL Version 1.0
/// 11/25/2008     JG       Added support for IAFUdev
/// 12/05/2008     HM       Added AFUTransaction members GetTranIDPtr() and
///                            GetCommand()
/// 12/15/2008     HM       Added GetPIPVer() & GetAPIVer() to IAFUTransaction
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2009     HM       Modified signature of UnBind
/// 03/16/2009     HM       Added comments
/// 06/22/2009     JG       Massive changes to support new proxy mechaism
/// 12/27/2009     JG       Added atomic CSR functions
/// 09/01/2011     JG       Removed Proxys
/// 09/02/2011     JG       Removed IAIA
/// 10/26/2011     JG       Removed CAIA moved to uAIA.h and redefined@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_AIA_H__
#define __AALSDK_UAIA_AIA_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/kernel/aalui.h> 				// uid_msgIDs_e
#include <aalsdk/utils/AALWorkSpaceUtilities.h> // WorkSpaceMapper
#include <aalsdk/CUnCopyable.h>

// Remove once Autotooled and placed in aaldefs.h
#if defined ( __AAL_WINDOWS__ )
# ifdef AIASERVICE_SERVICE_EXPORTS
#    define AIA_API __declspec(dllexport)
# else
#    define AIA_API __declspec(dllimport)
# endif // AIASERVICE_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define AIA_API    __declspec(0)
#endif // __AAL_WINDOWS__

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: IAFUTransaction
// Description: Interface to IAFUTransaction object which abstracts the
//              and AFU Transaction message.
// Comments: Because the structure of the message is AFU/PIP specific the
//           interface does not define nor expose any details.
//=============================================================================
class UAIA_API IAFUTransaction : public CUnCopyable
{
public:
   virtual ~IAFUTransaction();
   virtual  btVirtAddr     GetPayloadPtr() = 0;
   virtual  btWSSize       GetSize()       = 0;
   virtual  TransactionID *GetTranIDPtr()  = 0;  // NULL means no embedded TranID
   virtual  uid_msgIDs_e   GetCommand()    = 0;  // e.g. reqid_UID_SendAFU
   virtual  btID           GetPIPVer()     = 0;
   virtual  btID           GetAPIVer()     = 0;
};

//=============================================================================
// Name:          IAFUCSRMap
// Description:   Interface to a direct-mapped CSR space, if available on this
//                   AFUDev
// Comments:      Obtained by a function call on IAFUDev
//                NOT YET IMPLEMENTED - PLACEHOLDER
//=============================================================================
class UAIA_API IAFUCSRMap
{
public:
   virtual ~IAFUCSRMap();
   virtual void SetCSR(btCSROffset CSR,     // Number of CSR to set, starting at 0
                       btCSRValue  Value    // Value to set it to. FAP uses 64-bit interface, QPI/PCIe will use 32-bit
                      ) = 0;
   virtual void GetCSR(btCSROffset CSR      // Number of CSR to get, starting at 0
                      ) = 0;
   virtual void Destroy() = 0;
};

//=============================================================================
// Name: IAFUDev
// Description: Interface to AFUDev object which abstracts the AFU device
//=============================================================================
class UAIA_API IAFUDev
{
public:
   virtual ~IAFUDev();

   // Send a message to the device
   virtual btBool SendTransaction(IAFUTransaction * , TransactionID const & ) = 0;

   // CSR methods
   // Normal CSR access is provided by standard AFUTransactions and SendTransaction
   // This provides a Direct Mapping from user space to the AFU's registers - very fast
   // NOT YET IMPLEMENTED - PLACEHOLDER, currently returns NULL
   virtual IAFUCSRMap * GetCSRMAP() = 0;

   virtual btBool atomicSetCSR(btCSROffset CSR, btCSRValue  Value) = 0;
   virtual btBool atomicGetCSR(btCSROffset CSR, btCSRValue *Value) = 0;
   virtual btVirtAddr getMMIOR()                                   = 0;
   virtual btUnsigned32bitInt getMMIORsize()                       = 0;

   virtual btVirtAddr getUMSG()                                    = 0;
   virtual btUnsigned32bitInt getUMSGsize()                        = 0;
   virtual void Destroy(TransactionID const & ) = 0;

   // Temporary extension to give HWALIAFU service access to workspace mapper
   // Will be replaced by different method on redesign of AFUDev
   virtual WorkSpaceMapper & WSM()                                         = 0;
};


//=============================================================================
// Name: IManagement
// Description: AIA Management interface
//=============================================================================
class UAIA_API IManagement
{
public:
   virtual ~IManagement();

   virtual void Bind(btObjectType         Handle,
                     NamedValueSet        nvsOptions,
                     TransactionID const &tid) = 0;

   virtual void UnBind(btObjectType         Handle,
                       TransactionID const &tid) = 0;

   virtual void ReBind(IAFUDev             *DevObject,
                       NamedValueSet        nvsOptions,
                       TransactionID const &tid) = 0;

};

//=============================================================================
// Name: IProvisioning
// Description: AIA Device Provisioning interface
//=============================================================================
class UAIA_API IProvisioning
{
public:
   virtual ~IProvisioning();

   virtual void ActivateDevice(btObjectType         Handle,
                               NamedValueSet        nvsOptions,
                               TransactionID const &tid) = 0;

   virtual void DeActivateDevice(btObjectType         Handle,
                                 TransactionID const &tid) = 0;

   virtual void SetDeviceState(btObjectType         Handle,
                               TransactionID const &tid,
                  /*DevState*/ int                  eDevState) = 0;

};

END_NAMESPACE(AAL)

#endif // __AALSDK_UAIA_AIA_H__

