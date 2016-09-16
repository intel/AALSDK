// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_SMOCKS_H__
#define __GTCOMMON_SMOCKS_H__

using namespace std;

/// ===================================================================
/// @brief        Google mock service client class, providing implementation
///               for the attendent, (and required) derived client and
///               service client interfaces.
///
/// @details      The interface includes the custom work-completion
///               callback, used to recieve notification from the custom
///               service class, which implements the custom service
///               interface.
///

class GTCOMMON_API CMockWorkClient : public CAASBase, public IServiceClient, public IMockWorkClient
{

public:
   /// ================================================================
   /// @brief        Main object constructor, taking a Runtime client to use
   ///               for callback notifications.
   ///
   /// @param        pRCA    The Runtime client adapter pointer.
   ///

   CMockWorkClient(CRuntimeClientAdapter* pRCA)
      : m_pAALService(NULL)
      , m_pRuntimeClient(pRCA)
      , m_Result(0)
      , m_pListener(NULL)
      , m_pLock(pRCA->getListenerLock())
   {
      SetInterface(iidServiceClient, dynamic_cast<IServiceClient*>(this));
      SetInterface(iidMockWorkClient, dynamic_cast<IMockWorkClient*>(this));
   }

   // virtual desctructor to allow deletion from a base pointer
   virtual ~CMockWorkClient()
   {
   }

   int aquireServiceResource();
   void setListener(IServiceListener* pListener)
   {
      m_pListener = pListener;
   }

   /// ================================================================
   /// @brief        Custom service work completion callback.
   ///
   /// @param        rTranID    Read-only transaction ID reference.
   ///
   virtual void workComplete(TransactionID const& rTranID);
   virtual void workComplete2(TransactionID const& rTranID);

   /// @internal    <begin IServiceClient interface>
   virtual void serviceAllocated(IBase* pServiceBase, TransactionID const& rTranID);
   virtual void serviceAllocateFailed(const IEvent& rEvent);
   virtual void serviceReleaseFailed(const IEvent& rEvent);
   virtual void serviceReleased(TransactionID const& rTranID);
   virtual void serviceReleaseRequest(IBase* pServiceBase, const IEvent& rEvent);
   virtual void serviceEvent(const IEvent& rEvent);
   /// @internal   <end IServiceClient interface>

protected:
   IBase* m_pAALService;
   CRuntimeClientAdapter* m_pRuntimeClient;
   IServiceListener* m_pListener;
   CListenerLock* m_pLock;
   int m_Result;
};

#endif   // __GTCOMMON_SMOCKS_H__
