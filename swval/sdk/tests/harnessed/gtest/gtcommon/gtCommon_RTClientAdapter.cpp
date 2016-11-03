// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

/// ===================================================================
/// @internal        CRuntimeClientAdapter
///

CRuntimeClientAdapter::CRuntimeClientAdapter( CListenerLock* pLock )
       : m_pRuntimeAdapter( new ( std::nothrow ) CRuntimeAdapter(
            dynamic_cast<IRuntimeClient*>( this ) ) )
       , m_isOK( false )
       , m_pLock( pLock )
{
   m_pRTListener = dynamic_cast
      <IRuntimeListener*>( new ( std::nothrow ) CRuntimeListener( pLock ) );

   SetInterface( iidRuntimeClient, dynamic_cast<IRuntimeClient*>( this ) );

   NamedValueSet nvs;
   ASSERT( m_pRuntimeAdapter->start( nvs ) );
}

IRuntime* CRuntimeClientAdapter::getRuntimeAdapter()
{
   return dynamic_cast<IRuntime*>( m_pRuntimeAdapter );
}

btBool CRuntimeClientAdapter::isOK()
{
   return m_isOK;
}

CListenerLock* CRuntimeClientAdapter::getListenerLock()
{
   return m_pLock;
}

void CRuntimeClientAdapter::end()
{
   m_pRuntimeAdapter->stop();
}

IRuntime* CRuntimeClientAdapter::getRuntime()
{
   return getRuntimeAdapter();
}

void CRuntimeClientAdapter::runtimeCreateOrGetProxyFailed( IEvent const
                                                           & rEvent )
{
   m_isOK = false;
   m_pRTListener->OnRuntimeCreateOrGetProxyFailed( rEvent );
}

void CRuntimeClientAdapter::runtimeStarted( IRuntime* pRuntime,
                                            const NamedValueSet& rConfigParms )
{
   m_isOK = true;
   m_pRTListener->OnRuntimeStarted( pRuntime, rConfigParms );
}

void CRuntimeClientAdapter::runtimeStopped( IRuntime* pRuntime )
{
   m_isOK = false;
   m_pRTListener->OnRuntimeStopped( pRuntime );
}

void CRuntimeClientAdapter::runtimeStartFailed( const IEvent& rEvent )
{
   m_isOK = false;
   m_pRTListener->OnRuntimeStartFailed( rEvent );
}

void CRuntimeClientAdapter::runtimeStopFailed( const IEvent& rEvent )
{
   m_pRTListener->OnRuntimeStopFailed( rEvent );
}

void CRuntimeClientAdapter::runtimeAllocateServiceFailed( IEvent const& rEvent )
{
   m_isOK = false;
   m_pRTListener->OnRuntimeAllocateServiceFailed( rEvent );
}

void CRuntimeClientAdapter::runtimeAllocateServiceSucceeded( IBase* pBase,
                                                             TransactionID const
                                                             & rTranID )
{
   m_isOK = true;
   m_pRTListener->OnRuntimeAllocateServiceSucceeded( pBase, rTranID );
}

void CRuntimeClientAdapter::runtimeEvent( IEvent const& rEvent )
{
   m_pRTListener->OnRuntimeEvent( rEvent );
}
