// Copyright (c) 2007-2014, Intel Corporation
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
/// @file AALuAIA_UIDriverClient.h
/// @brief Defines the object wrapper for the AAL Universal Device Driver.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/08/2008     HM/JG    Implemented wrapping in SendAFUTransaction
/// 12/14/2008     HM       Added result-code accessor / mutator
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2009     HM       Added BindDev
/// 05/09/2009     HM/JG    Fixed a latent bug in UIDriverClient::uidrvManip
///                            UnBindDev(). The constructor of
///                            UIDriverClient::uidrvManip expects an object as
///                            the second parameter instead of a NULL.
/// 06/22/2009     JG       Made changes to marshaller to support UAIA
///                         accessor necessary to break build dependency that
///                         Interfered with plug-in model. Now marshaller is
///                         accessed from this library through a pointer.@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_AALUAIA_UIDRVERCLIENT_H__
#define __AALSDK_UAIA_AALUAIA_UIDRVERCLIENT_H__
#include <aalsdk/kernel/aalui.h>  // uid_msgIDs_e, uid_errnum_e, aalui_ioctlreq

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/uaia/uidrvMessage.h>      // uidrvMessage, uidrvMessageRoute
#include <aalsdk/uaia/AALuAIA_Messaging.h> // UIDriverClient_msgPayload, UIDriverClient_uidrvManip


#ifdef __AAL_UNKNOWN_OS__
# error Define UIDriverClient IPC for unknown OS.
#endif // __AAL_UNKNOWN_OS__


/// @todo Document UIDriverClient and related.


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


    //==========================================================================
    // Name: IUIDriverClientEvent
    // Description:
    //==========================================================================
    class UAIA_API IUIDriverClientEvent
    {
    public:
       virtual ~IUIDriverClientEvent();
       virtual btObjectType              DevHandle()  const         = 0;
       virtual uid_msgIDs_e              MessageID()  const         = 0;
       virtual btVirtAddr                Payload()    const         = 0;
       virtual btWSSize                  PayloadLen() const         = 0;
       virtual stTransactionID_t const & msgTranID()  const         = 0;
       virtual uidrvMessageRoute const & msgRoute()   const         = 0;
       virtual uid_errnum_e              ResultCode() const         = 0;
       virtual void                      ResultCode(uid_errnum_e e) = 0;
    };

    //==========================================================================
    // Name: IUIDriverClientEventExceptionEvent
    // Description:
    //==========================================================================
    class UAIA_API IUIDriverClientEventExceptionEvent
    {
    public:
       virtual ~IUIDriverClientEventExceptionEvent();
       virtual btInt AALErr() const = 0;
    };

    //==========================================================================
    // Name: UIDriverClient
    // Description: The UIDriverClient is a wrapper object around the
    //              low level driver interface.
    //==========================================================================
    class UAIA_API UIDriverClient : CriticalSection
    {
    public:
      // Constructor
      UIDriverClient();
      // Destructor
      virtual ~UIDriverClient();

      void   IsOK(btBool flag) { m_bIsOK = flag; }
      btBool IsOK()            { return m_bIsOK; }

      // Open the channel to the service
      void Open(const char *devName = NULL);

      btBool MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet);
      void UnMapWSID(btVirtAddr ptr, btWSSize Size);

      //=========================================================
      // Defines a manipulator that can take arguments
      // Since an operator takes at most 1 argument the multiple
      // arguments must be passed through a function object which
      // contains the actual manipulator pointer and its args
      //=========================================================
      UIDriverClient & operator << (const UIDriverClient_uidrvManip &m);
   
      // Polls for messages and returns when one is available
      btBool GetMessage(uidrvMessage *uidrvMessagep);

      // Sends a message down the UIDriver channel
      btBool SendMessage(btUnsigned64bitInt cmd, struct aalui_ioctlreq *reqp);

      UIDriverClient & Send(btUnsigned64bitInt cmd, struct aalui_ioctlreq *reqp)
      {
         m_bIsOK = SendMessage(cmd, reqp);
         return *this;
      }

      // Close the channel to the service
      void Close();

   private:
#if defined( __AAL_WINDOWS__ )
      HANDLE m_hClient;
#elif defined( __AAL_LINUX__ )
      btInt  m_fdClient;
#endif // OS

      btBool m_bIsOK;

      UIDriverClient(const UIDriverClient & );
      UIDriverClient & operator = (const UIDriverClient & );
   }; // class UIDriverClient{}


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_UAIA_AALUAIA_UIDRVERCLIENT_H__

