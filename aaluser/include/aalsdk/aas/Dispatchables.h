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
//        FILE: aas/Dispatchables.h
//     CREATED: May 15, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: This file implements convenient IDispatchble based classes.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 05/15/2015     JG       Initial Version
//****************************************************************************///
#ifndef __AALSDK_DISPATCHABLES_H__
#define __AALSDK_DISPATCHABLES_H__
#include <aalsdk/AALDefs.h>
#include <aalsdk/osal/IDispatchable.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/aas/IServiceRevoke.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/aas/AALService.h>

BEGIN_NAMESPACE(AAL)

//============================================================================
// AAL Service Client
//============================================================================

/// @brief Delivers IServiceClient::serviceAllocated(IBase               * ,
///                                                  TransactionID const & );
class AASLIB_API ServiceAllocated : public IDispatchable
{
public:
   ServiceAllocated(IServiceClient      *pSvcClient,
                    IRuntimeClient      *pRTClient,
                    IBase               *pServiceBase,
                    TransactionID const &rTranID);
   virtual void operator() ();
protected:
   IServiceClient      *m_pSvcClient;
   IRuntimeClient      *m_pRTClient;
   IBase               *m_pServiceBase;
   const TransactionID  m_TranID;
};

/// @brief Delivers IServiceClient::serviceAllocateFailed(const IEvent & );
class AASLIB_API ServiceAllocateFailed : public IDispatchable
{
public:
   ServiceAllocateFailed(IServiceClient *pSvcClient,
                         IRuntimeClient *pRTClient,
                         const IEvent   *pEvent);
   ~ServiceAllocateFailed();
   virtual void operator() ();
protected:
   IServiceClient *m_pSvcClient;
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

// @brief Causes a Service Object to be detrsoyed
class AASLIB_API DestroyServiceObject : public IDispatchable
{
public:
   DestroyServiceObject(ISvcsFact *pSvcsFact,
                        IBase     *pService);

   virtual void operator() ();
protected:
   ISvcsFact *m_pSvcsFact;
   IBase     *m_pService;
};

/// @brief Delivers IServiceClient::serviceReleased(TransactionID const & );
class AASLIB_API ServiceReleased : public IDispatchable
{
public:
   ServiceReleased(IServiceClient      *pSvcClient,
                   IBase               *pServiceBase,
                   TransactionID const &rTranID);
   virtual void operator() ();
protected:
   IServiceClient      *m_pSvcClient;
   IBase               *m_pServiceBase;
   const TransactionID  m_TranID;
};

/// @brief Delivers IServiceClient::serviceReleaseFailed(const IEvent & );
class AASLIB_API ServiceReleaseFailed : public IDispatchable
{
public:
   ServiceReleaseFailed(IServiceClient *pSvcClient,
                        const IEvent   *pEvent);
   ~ServiceReleaseFailed();
   virtual void operator() ();
protected:
   IServiceClient *m_pSvcClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IServiceClient::serviceEvent(const IEvent & );
class AASLIB_API ServiceEvent : public IDispatchable
{
public:
   ServiceEvent(IServiceClient *pSvcClient,
                const IEvent   *pEvent);
   ~ServiceEvent();
   virtual void operator() ();
protected:
   IServiceClient *m_pSvcClient;
   const IEvent   *m_pEvent;
};

//============================================================================
// AAL Runtime
//============================================================================

/// @brief Delivers IRuntimeClient::runtimeCreateOrGetProxyFailed(IEvent const & );
class AASLIB_API RuntimeCreateOrGetProxyFailed : public IDispatchable
{
public:
   RuntimeCreateOrGetProxyFailed(IRuntimeClient *pRTClient,
                                 const IEvent   *pEvent);
   ~RuntimeCreateOrGetProxyFailed();
   virtual void       operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeStarted(IRuntime            * ,
///                                                const NamedValueSet & );
class AASLIB_API RuntimeStarted : public IDispatchable
{
public:
   RuntimeStarted(IRuntimeClient      *pRTClient,
                  IRuntime            *pRT,
                  const NamedValueSet &rConfigParms);
   virtual void operator() ();
protected:
   IRuntimeClient      *m_pRTClient;
   IRuntime            *m_pRT;
   const NamedValueSet  m_rConfigParms;
};

/// @brief Delivers IRuntimeClient::runtimeStartFailed(const IEvent & );
class AASLIB_API RuntimeStartFailed : public IDispatchable
{
public:
   RuntimeStartFailed(IRuntimeClient *pRTClient,
                      const IEvent   *pEvent);
   ~RuntimeStartFailed();
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeStopped(IRuntime * );
class AASLIB_API RuntimeStopped : public IDispatchable
{
public:
   RuntimeStopped(IRuntimeClient *pRTClient,
                  IRuntime       *pRT);
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   IRuntime       *m_pRT;
};

/// @brief Delivers IRuntimeClient::runtimeStopFailed(const IEvent & );
class AASLIB_API RuntimeStopFailed : public IDispatchable
{
public:
   RuntimeStopFailed(IRuntimeClient *pRTClient,
                     const IEvent   *pEvent);
   ~RuntimeStopFailed();
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeAllocateServiceSucceeded(IBase * ,
///                                                                 TransactionID const & );
class AASLIB_API RuntimeAllocateServiceSucceeded : public IDispatchable
{
public:
   RuntimeAllocateServiceSucceeded(IRuntimeClient      *pRTClient,
                                   IBase               *pServiceBase,
                                   TransactionID const &rTranID);
   virtual void operator() ();
protected:
   IRuntimeClient      *m_pRTClient;
   IBase               *m_pServiceBase;
   const TransactionID  m_rTranID;
};

/// @brief Delivers IRuntimeClient::runtimeAllocateServiceFailed(const IEvent & );
class AASLIB_API RuntimeAllocateServiceFailed : public IDispatchable
{
public:
   RuntimeAllocateServiceFailed(IRuntimeClient *pRTClient,
                                const IEvent   *pEvent);
   ~RuntimeAllocateServiceFailed();
   virtual void      operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeEvent(const IEvent & );
class AASLIB_API RuntimeEvent : public IDispatchable
{
public:
   RuntimeEvent(IRuntimeClient *pRTClient,
                const IEvent   *pEvent);
   ~RuntimeEvent();
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IServiceRevoke::serviceRevoked(const IEvent & );
class AASLIB_API ServiceRevoke : public IDispatchable
{
public:
   ServiceRevoke(IServiceRevoke *pRevoke);
   virtual void operator() ();
protected:
   IServiceRevoke *m_pRevoke;
};

class AASLIB_API ReleaseServiceRequest : public IDispatchable
{
public:
   ReleaseServiceRequest(IServiceClient *, const IEvent   *);
   ~ReleaseServiceRequest();
   virtual void operator() ();
protected:
   IServiceClient *m_pSvcClient;
   const IEvent   *m_pEvent;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_DISPATCHABLES_H__

