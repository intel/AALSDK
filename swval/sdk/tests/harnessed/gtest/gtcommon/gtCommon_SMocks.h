// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_SMOCKS_H__
#define __GTCOMMON_SMOCKS_H__

/// ===================================================================
/// @brief        The custom service client.
///
/// @details      The custom service client (work client) interface
///               includes work-completion callbacks, which are
///               recieved here.
///
class GTCOMMON_API CMockWorkClient : public EmptyIServiceClient,
                                     public IMockWorkClient
{

public:
   /// ================================================================
   /// @brief        The main service client constructor, taking a runtime
   ///               client adapter to recieve callback notifications from the
   ///               shared singlton runtime instance through a proxy.
   ///
   /// @param        pRCA    The runtime client adapter pointer.
   ///

   CMockWorkClient( CRuntimeClientAdapter* pRCA )
          : m_pAALService( NULL )
          , m_pRuntimeClient( pRCA )
          , m_Result( 0 )
          , m_pListener( NULL )
          , m_pLock( pRCA->getListenerLock() )
   {
      SetInterface( iidServiceClient, dynamic_cast<IServiceClient*>( this ) );
      SetInterface( iidMockWorkClient, dynamic_cast<IMockWorkClient*>( this ) );
   }

   /// ================================================================
   /// @brief        The custom service work completion callback.
   ///
   /// @param        rTranID    A read-only transaction ID reference.
   ///
   virtual void workComplete( TransactionID const& rTranID );

   /// @internal    <begin IServiceClient interface>
   virtual void serviceAllocated( IBase* pServiceBase,
                                  TransactionID const& rTranID );
   virtual void serviceAllocateFailed( const IEvent& rEvent );
   virtual void serviceReleaseFailed( const IEvent& rEvent );
   virtual void serviceReleased( TransactionID const& rTranID );
   virtual void serviceReleaseRequest( IBase* pServiceBase,
                                       const IEvent& rEvent );
   virtual void serviceEvent( const IEvent& rEvent );
   /// @internal   <end IServiceClient interface>

   // virtual destructor to allow deletion from a base pointer
   virtual ~CMockWorkClient()
   {
   }

   int aquireServiceResource();
   void setListener( IServiceListener* pListener )
   {
      m_pListener = pListener;
   }

protected:
   IBase* m_pAALService;
   CRuntimeClientAdapter* m_pRuntimeClient;
   IServiceListener* m_pListener;
   CListenerLock* m_pLock;
   int m_Result;
};

#endif   // __GTCOMMON_SMOCKS_H__
