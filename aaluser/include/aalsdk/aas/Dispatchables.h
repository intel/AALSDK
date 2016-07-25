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
   /// @brief ServiceAllocated constructor.
   ///
   /// @param pSvcClient   A pointer to the Service Client interface.
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pServiceBase A pointer to the IBase interface of the Service allocated.
   /// @param rTranID      A reference to the Transaction ID.
   /// @return void
   ServiceAllocated(IServiceClient      *pSvcClient,
                    IRuntimeClient      *pRTClient,
                    IBase               *pServiceBase,
                    TransactionID const &rTranID);
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief ServiceAllocateFailed constructor.
   ///
   /// @param pSvcClient   A pointer to the Service Client interface.
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   ServiceAllocateFailed(IServiceClient *pSvcClient,
                         IRuntimeClient *pRTClient,
                         const IEvent   *pEvent);
   ~ServiceAllocateFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief DestroyServiceObject constructor.
   ///
   /// @param pSvcsFact    A pointer to the Service Factory interface.
   /// @param pService     A pointer to the IBase interface of the Service.
   /// @returns void
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
   /// @brief ServiceReleased constructor.
   ///
   /// @param pSvcClient   A pointer to the Service Client interface.
   /// @param pServiceBase A pointer to the IBase interface of the Service allocated.
   /// @param rTranID      A reference to the Transaction ID.
   /// @returns void
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
   /// @brief ServiceReleaseFailed constructor.
   ///
   /// @param pSvcClient   A pointer to the Service Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   ServiceReleaseFailed(IServiceClient *pSvcClient,
                        const IEvent   *pEvent);
   ~ServiceReleaseFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void operator() ();
protected:
   IServiceClient *m_pSvcClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IServiceClient::serviceEvent(const IEvent & );
class AASLIB_API ServiceEvent : public IDispatchable
{
public:
   /// @brief ServiceReleaseFailed constructor.
   ///
   /// @param pSvcClient   A pointer to the Service Client interface.
   /// @param pEvent       A pointer to the Event.
   /// @returns void
   ServiceEvent(IServiceClient *pSvcClient,
                const IEvent   *pEvent);
   ~ServiceEvent();
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief RuntimeCreateOrGetProxyFailed constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   RuntimeCreateOrGetProxyFailed(IRuntimeClient *pRTClient,
                                 const IEvent   *pEvent);
   ~RuntimeCreateOrGetProxyFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief RuntimeStarted constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pRT          A pointer to the Runtime interface.
   /// @param rConfigParms A reference to the optional arguments used passed to the Service.
   /// @returns void
   RuntimeStarted(IRuntimeClient      *pRTClient,
                  IRuntime            *pRT,
                  const NamedValueSet &rConfigParms);
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief RuntimeStartFailed constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   RuntimeStartFailed(IRuntimeClient *pRTClient,
                      const IEvent   *pEvent);
   ~RuntimeStartFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeStopped(IRuntime * );
class AASLIB_API RuntimeStopped : public IDispatchable
{
public:
   /// @brief RuntimeStopped constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pRT          A pointer to the Runtime interface.
   /// @returns void
   RuntimeStopped(IRuntimeClient *pRTClient,
                  IRuntime       *pRT);
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   IRuntime       *m_pRT;
};

/// @brief Delivers IRuntimeClient::runtimeStopFailed(const IEvent & );
class AASLIB_API RuntimeStopFailed : public IDispatchable
{
public:
   /// @brief RuntimeStopFailed constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   RuntimeStopFailed(IRuntimeClient *pRTClient,
                     const IEvent   *pEvent);
   ~RuntimeStopFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief RuntimeAllocateServiceSucceeded constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pServiceBase A pointer to the IBase interface of the Service allocated.
   /// @param rTranID      A reference to the Transaction ID.
   /// @returns void
   RuntimeAllocateServiceSucceeded(IRuntimeClient      *pRTClient,
                                   IBase               *pServiceBase,
                                   TransactionID const &rTranID);
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief RuntimeAllocateServiceFailed constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event with information about the failure.
   /// @returns void
   RuntimeAllocateServiceFailed(IRuntimeClient *pRTClient,
                                const IEvent   *pEvent);
   ~RuntimeAllocateServiceFailed();
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void      operator() ();
protected:
   IRuntimeClient *m_pRTClient;
   const IEvent   *m_pEvent;
};

/// @brief Delivers IRuntimeClient::runtimeEvent(const IEvent & );
class AASLIB_API RuntimeEvent : public IDispatchable
{
public:
   /// @brief RuntimeEvent constructor.
   ///
   /// @param pRTClient    A pointer to the Runtime Client interface.
   /// @param pEvent       A pointer to the Event.
   /// @returns void
   RuntimeEvent(IRuntimeClient *pRTClient,
                const IEvent   *pEvent);
   ~RuntimeEvent();
   /// @brief Where the work happens.
   ///
   /// @returns void
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
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void operator() ();
protected:
   IServiceRevoke *m_pRevoke;
};

class AASLIB_API ReleaseServiceRequest : public IDispatchable
{
public:
   ReleaseServiceRequest(IBase *, const IEvent   *);
   ~ReleaseServiceRequest();
   /// @brief Where the work happens.
   ///
   /// @returns void
   virtual void operator() ();
protected:
   IBase          *m_pSvcBase;
   const IEvent   *m_pEvent;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_DISPATCHABLES_H__

