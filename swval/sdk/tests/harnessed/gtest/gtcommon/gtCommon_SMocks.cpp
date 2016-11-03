// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

/// ===================================================================
/// @internal        CMockDispatchable
///
CMockDispatchable::CMockDispatchable( IMockWorkClient* pClient,
                                      IBase* pMockDoWorker,
                                      TransactionID const& rTranID )
       : m_pWorkClient( pClient )
       , m_pService( pMockDoWorker )
       , m_TranID( rTranID )
{
}

// Provides the work completion callback event to clients.
void CMockDispatchable::operator()()
{
   ASSERT( m_pWorkClient );
   if ( m_pWorkClient != NULL ) {
      m_pWorkClient->workComplete( m_TranID );
   }
   delete this;
}

/// ===================================================================
/// @internal        CMockWorkClient
///

/// ===================================================================
/// @internal    Service configuration and allocation.  The
/// ConfigRecord for the module library is left out as both service and
/// client are in the same module.  This circumvents the service module
/// creation and load-library code, allowing us to test without relying
/// on a separate binary.
///
int CMockWorkClient::aquireServiceResource()
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add( AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true );

   Manifest.Add( AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord );

   Manifest.Add( AAL_FACTORY_CREATE_SERVICENAME, "Mock DoWorker" );

   IRuntime* pRuntime = m_pRuntimeClient->getRuntime();
   pRuntime->allocService(
      static_cast<IBase*>( this ), Manifest, TransactionID() );

   m_pLock->wait();   // wait for OnRuntimeAllocateServiceSucceeded
   return m_Result;
}

void CMockWorkClient::serviceAllocated( IBase* pServiceBase,
                                        TransactionID const& rTranID )
{
   m_pListener->OnServiceAllocated( dynamic_cast
                                    <ServiceBase*>( pServiceBase ) );
}

void CMockWorkClient::serviceAllocateFailed( const IEvent& rEvent )
{
   m_Result++;
   m_pListener->OnServiceAllocateFailed( rEvent );
}

void CMockWorkClient::serviceReleaseFailed( const IEvent& rEvent )
{
   m_pListener->OnServiceReleaseFailed( rEvent );
}

void CMockWorkClient::serviceReleased( TransactionID const& rTranID )
{
   m_pListener->OnServiceReleased( rTranID );
}

void CMockWorkClient::serviceReleaseRequest( IBase* pServiceBase,
                                             const IEvent& rEvent )
{
   m_pListener->OnServiceReleaseRequest( pServiceBase, rEvent );
}

void CMockWorkClient::serviceEvent( const IEvent& rEvent )
{
   m_pListener->OnServiceEvent( rEvent );
}

void CMockWorkClient::workComplete( TransactionID const& rTranID )
{
   m_pListener->OnWorkComplete( rTranID );
}
