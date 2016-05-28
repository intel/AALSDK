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
/// @file HWALIReconf.h
/// @brief Definitions for ALI Hardware AFU Service.
/// @ingroup ALI
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/11/2016     HM       Initial version.@endverbatim
//****************************************************************************
#ifndef __HWALIRECONF_H__
#define __HWALIRECONF_H__

#include "HWALIBase.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///
class CHWALIReconf : public CHWALIBase,
                     public IALIReconfigure
{
public :

   CHWALIReconf( IBase *pSvcClient,
                 IServiceBase *pServiceBase,
                 TransactionID transID,
                 IAFUProxy *pAFUProxy);

   ~CHWALIReconf()  { };

   // <ALIReconfigure>
   virtual void reconfDeactivate( TransactionID const &rTranID,
                                  NamedValueSet const &rInputArgs );
   virtual void reconfConfigure( TransactionID const &rTranID,
                                 NamedValueSet const &rInputArgs );
   virtual void reconfActivate( TransactionID const &rTranID,
                                NamedValueSet const &rInputArgs );
   // </ALIReconfigure>

   // sets Reconfigure  Client interface
   btBool setReconfClientInterface();

   // AFU Event Handler
   virtual void AFUEvent(AAL::IEvent const &theEvent);

private:
   IALIReconfigure_Client *m_pReconClient;
};

// ===========================================================================
//
// Dispatchables for client callbacks
//
// ===========================================================================
class AFUDeactivated : public IDispatchable
{
public:
   AFUDeactivated(  IALIReconfigure_Client   *pSvcClient,
                    TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->deactivateSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};

class AFUDeactivateFailed : public IDispatchable
{
public:
   AFUDeactivateFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->deactivateFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
};

class AFUActivated : public IDispatchable
{
public:
   AFUActivated(  IALIReconfigure_Client   *pSvcClient,
                    TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->activateSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};

class AFUActivateFailed : public IDispatchable
{
public:
   AFUActivateFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->activateFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
};

class AFUReconfigured : public IDispatchable
{
public:
   AFUReconfigured(  IALIReconfigure_Client   *pSvcClient,
                     TransactionID const      &rTranID)
   : m_pSvcClient(pSvcClient),
     m_TranID(rTranID){}

   virtual void operator() ()
   {
      m_pSvcClient->configureSucceeded(m_TranID);
   }


protected:
   IALIReconfigure_Client      *m_pSvcClient;
   const TransactionID          m_TranID;
};


class AFUReconfigureFailed : public IDispatchable
{
public:
   AFUReconfigureFailed( IALIReconfigure_Client   *pSvcClient,
                         const IEvent   *pEvent)
   : m_pSvcClient(pSvcClient),
     m_pEvent(pEvent){}

   virtual void operator() ()
   {
      m_pSvcClient->configureFailed(*m_pEvent);
   }


protected:
   IALIReconfigure_Client        *m_pSvcClient;
   const IEvent                  *m_pEvent;
};

/// @}
END_NAMESPACE(AAL)

#endif // __HWALIRECONF_H__

