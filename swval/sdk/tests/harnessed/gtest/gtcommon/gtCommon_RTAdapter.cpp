// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

/// ===================================================================
/// @internal        CRuntimeListener
///
CRuntimeListener::CRuntimeListener( CListenerLock* pLock ) : m_pLock( pLock )
{
}

CRuntimeListener::~CRuntimeListener()
{
}

void CRuntimeListener::OnRuntimeCreateOrGetProxyFailed( IEvent const& rEvent )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeStarted( IRuntime* pRuntime,
                                         const NamedValueSet& rConfigParams )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeStopped( IRuntime* pRuntime )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeStartFailed( IEvent const& rEvent )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeStopFailed( IEvent const& rEvent )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeAllocateServiceFailed( IEvent const& rEvent )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeAllocateServiceSucceeded( IBase* pBase,
                                                          TransactionID const
                                                          & rTranID )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

void CRuntimeListener::OnRuntimeEvent( IEvent const& rEvent )
{
   MSG( "runtime adapter" );
   m_pLock->signal();
}

/// ===================================================================
/// @internal        CRuntimeAdapter
///

CRuntimeAdapter::CRuntimeAdapter( IRuntimeClient* pRCA )
       : m_pRuntimeDelegate( NULL ), m_pRCA( pRCA ), m_pLock( NULL )
{
   m_pRuntimeDelegate = new ( std::nothrow ) Runtime( pRCA );
   SetInterface( iidRuntime, m_pRuntimeDelegate );
}

void CRuntimeAdapter::setListenerLock( CListenerLock* pLock )
{
   m_pLock = pLock;
}

CRuntimeAdapter::~CRuntimeAdapter()
{
   delete m_pRuntimeDelegate;
   m_pRuntimeDelegate = NULL;
}

/// ===================================================================
/// @internal     Runtime interface implementation, forwarding to
/// delegate.  Modeled after the internal SDK Runtime wrapper class. The
/// only reason for this class to exist is to override the allocService
/// function in order to invoke the custom in-module service factory.
///

btBool CRuntimeAdapter::start( const NamedValueSet& rconfigParms )
{
   return m_pRuntimeDelegate->start( rconfigParms );
}

void CRuntimeAdapter::stop()
{
   m_pRuntimeDelegate->stop();
}

btBool CRuntimeAdapter::schedDispatchable( IDispatchable* pDispatchable )
{
   return m_pRuntimeDelegate->schedDispatchable( pDispatchable );
}

IRuntime* CRuntimeAdapter::getRuntimeProxy( IRuntimeClient* pClient )
{
   return m_pRuntimeDelegate->getRuntimeProxy( pClient );
}

btBool CRuntimeAdapter::releaseRuntimeProxy()
{
   return m_pRuntimeDelegate->releaseRuntimeProxy();
}

IRuntimeClient* CRuntimeAdapter::getRuntimeClient()
{
   return m_pRuntimeDelegate->getRuntimeClient();
}

btBool CRuntimeAdapter::IsOK()
{
   return m_pRuntimeDelegate->IsOK();
}

/// ===================================================================
/// @internal     Custom override of allocService so we can use the
/// custom in-module service factory.
///
void CRuntimeAdapter::allocService( IBase* pServiceClient,
                                    NamedValueSet const& rManifest,
                                    TransactionID const& rTranID )
{
   InModuleSvcsFact<CMockDoWorker>* pFact = InModuleSvcsFact
      <CMockDoWorker>::GetServiceFactory();

   ASSERT( NULL != pFact );
   if ( pFact != NULL ) {
      IBase* pService = pFact->CreateServiceObject(
         InModuleSvcsFact<CMockDoWorker>::GetServiceModule(),
         static_cast<IRuntime*>( this ) );
      ASSERT( NULL != pService );
      if ( pService != NULL ) {
         pFact->InitializeService(
            pService, pServiceClient, rTranID, rManifest );
      }
      ASSERT( NULL != pServiceClient );
      if ( pServiceClient != NULL ) {
         dynamic_ptr<ServiceBase>( iidServiceBase, pService )
            ->init( pServiceClient, rManifest, rTranID );
      }
   }
}
