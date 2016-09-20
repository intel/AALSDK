// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H

#ifndef HAVE_COMMON_H
#include "gtCommon.h"
#endif

/// ===================================================================
/// @internal        CMockDoWorker
///

btBool CMockDoWorker::init(IBase* pClientBase, NamedValueSet const& optArgs, TransactionID const& rtid)
{
   m_pWorkClient = dynamic_ptr<IMockWorkClient>(iidMockWorkClient, pClientBase);
   m_pSvcClient = dynamic_ptr<IServiceClient>(iidServiceClient, pClientBase);

   if(NULL == m_pWorkClient || NULL == m_pSvcClient) {
      initFailed(new CExceptionTransactionEvent(
         NULL, rtid, errBadParameter, reasMissingInterface, "Client did not publish IMockWorkClient Interface"));
      return false;
   }

   initComplete(rtid);
   return true;
}

AALServiceModule* CMockDoWorker::getAALServiceModule() const
{
   return GetServiceModule();
}

IServiceClient* CMockDoWorker::getServiceClient() const
{
   return dynamic_cast<IServiceClient*>(m_pSvcClient);
}

void CMockDoWorker::dispatchWorkFunctor(TransactionID const& rTranID)
{
   CMockDispatchable* pDisp = new (std::nothrow) CMockDispatchable(m_pWorkClient, static_cast<IBase*>(this), rTranID);

   ASSERT(pDisp);
   getRuntime()->schedDispatchable(pDisp);
}

void CMockDoWorker::dispatchWorkFunctor2(TransactionID const& rTranID)
{
   CMockDispatchable2* pDisp = new (std::nothrow) CMockDispatchable2(m_pWorkClient, static_cast<IBase*>(this), rTranID);

   ASSERT(pDisp);
   getRuntime()->schedDispatchable(pDisp);
}

btBool CMockDoWorker::Release(TransactionID const& rTranID, btTime timeout)
{
   return ServiceBase::Release(rTranID, timeout);
}

btBool CMockDoWorker::initComplete(TransactionID const& rtid)
{
   return ServiceBase::initComplete(rtid);
}

/// ===================================================================
/// @internal        CMockDispatchable
///
/// @param        pClient          The client
/// @param        pMockDoWorker    The mock do worker
/// @param        rTranID          The r tran id
///

CMockDispatchable::CMockDispatchable(IMockWorkClient* pClient, IBase* pMockDoWorker, TransactionID const& rTranID)
   : m_pWorkClient(pClient)
   , m_pService(pMockDoWorker)
   , m_TranID(rTranID)
{
}

void CMockDispatchable::operator()()
{
   m_pWorkClient->workComplete(m_TranID);
   delete this;
}

CMockDispatchable2::CMockDispatchable2(IMockWorkClient* pClient, IBase* pMockDoWorker, TransactionID const& rTranID)
   : m_pWorkClient(pClient)
   , m_pService(pMockDoWorker)
   , m_TranID(rTranID)
{
}

void CMockDispatchable2::operator()()
{
   m_pWorkClient->workComplete2(m_TranID);
   delete this;
}

/// ===================================================================
/// @internal        CMockWorkClient
///

/// ===================================================================
/// @interanal    Service configuration and allocation.  The
/// ConfigRecord for the module library is left out as both service and
/// client are in the same module.  This circumvents the service module
/// creation and load-library code, allowing us to test without relying
/// on a separate binary.
///

int CMockWorkClient::aquireServiceResource()
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Mock DoWorker");

   IRuntime* pRuntime = m_pRuntimeClient->getRuntime();
   pRuntime->allocService(static_cast<IBase*>(this), Manifest, TransactionID());

   m_pLock->wait();   // wait for OnRuntimeAllocateServiceSucceeded
   return m_Result;
}

void CMockWorkClient::serviceAllocated(IBase* pServiceBase, TransactionID const& rTranID)
{
   m_pListener->OnServiceAllocated(dynamic_cast<ServiceBase*>(pServiceBase));
}

void CMockWorkClient::serviceAllocateFailed(const IEvent& rEvent)
{
   m_Result++;
   m_pListener->OnServiceAllocateFailed(rEvent);
}

void CMockWorkClient::serviceReleaseFailed(const IEvent& rEvent)
{
   m_pListener->OnServiceReleaseFailed(rEvent);
}

void CMockWorkClient::serviceReleased(TransactionID const& rTranID)
{
   m_pListener->OnServiceReleased(rTranID);
}

void CMockWorkClient::serviceReleaseRequest(IBase* pServiceBase, const IEvent& rEvent)
{
   m_pListener->OnServiceReleaseRequest(pServiceBase, rEvent);
}

void CMockWorkClient::serviceEvent(const IEvent& rEvent)
{
   m_pListener->OnServiceEvent(rEvent);
}

void CMockWorkClient::workComplete(TransactionID const& rTranID)
{
   m_pListener->OnWorkComplete(rTranID);
}

void CMockWorkClient::workComplete2(TransactionID const& rTranID)
{
   m_pListener->OnWorkComplete2(rTranID);
}
