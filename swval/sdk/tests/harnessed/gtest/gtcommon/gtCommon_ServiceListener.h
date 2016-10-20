// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_SERVICELISTENER_H__
#define __GTCOMMON_SERVICELISTENER_H__

/// ===================================================================
/// @brief        Custom implementation of the service listener interface,
///               used for test cases starting with aal0813.
///
class GTCOMMON_API CServiceListener : public IServiceListener
{
public:
   /// ================================================================
   /// @brief        Main constructor, taking a shared lock to synchronize
   ///               callback events.
   ///
   /// @param        pRCAdapter    Pointer to the runtime client
   ///                             adapter.
   /// @param        pLock         Pointer to the callback
   ///                             synchronization lock.
   /// @param[in]    msg           The message
   ///
   CServiceListener(CRuntimeClientAdapter* pRCAdapter, CListenerLock* pLock, string msg)
      : m_pServiceBase(NULL)
      , m_Message(msg)
      , m_pSvcClient(NULL)
      , m_pLock(pLock)
      , m_pRCA(pRCAdapter)
      , m_Result(0)
   {
   }

   virtual ~CServiceListener()
   {
   }

   /// ================================================================
   /// @brief        Service allocation complete event handler, invoked by
   ///               service client.
   ///
   /// @details      Get the service client pointer from the service
   ///               base pointer, then signal the lock.
   ///
   /// @param        pBase    Service IBase pointer.
   ///
   void OnServiceAllocated(ServiceBase* pBase)
   {
      MOCKDEBUG(m_Message);
      m_pServiceBase = pBase;
      m_pSvcClient = pBase->getServiceClient();
      ASSERT_NONNULL(m_pServiceBase);
      ASSERT_NONNULL(m_pSvcClient);
      m_pLock->signal();
   }

   /// ================================================================
   /// @brief        Service allocation failure event handler, invoked by
   ///               service client.
   ///
   /// @param        rEvent    The event read-only reference.
   ///
   void OnServiceAllocateFailed(IEvent const& rEvent)
   {
      MOCKDEBUG(m_Message);
      --m_Result;                                   // decrement the result, indicating a failure.
      IEvent& rRef = const_cast<IEvent&>(rEvent);   // cast away the constness
      // in order to construct the exception transaction event.
      IExceptionTransactionEvent& rTEE = dynamic_cast<IExceptionTransactionEvent&>(rRef);
      MSG("debug " << rTEE.Description());
      m_pLock->signal();
   }

   /// ================================================================
   /// @brief        Service release failure event handler, invoked by service
   ///               client.
   ///
   /// @param        rEvent    Read-only event reference.
   ///
   void OnServiceReleaseFailed(IEvent const& rEvent)
   {
      MOCKDEBUG(m_Message);
      m_pLock->signal();   // Signal semaphore to indicate callback completion.
   }

   /// ================================================================
   /// @brief        Service released event handler, invoked by service client.
   ///
   /// @param        rTranID    Read-only transaction ID reference.
   ///
   void OnServiceReleased(TransactionID const& rTranID)
   {
      MOCKDEBUG(m_Message);
      m_pLock->signal();
   }

   /// ================================================================
   /// @brief        Service release request event handler, invoked by service
   ///               client.
   ///
   /// @param        pBase     Service IBase pointer.
   /// @param        rEvent    Read-only event reference.
   ///
   void OnServiceReleaseRequest(IBase* pBase, IEvent const& rEvent)
   {
      MOCKDEBUG(m_Message);
      m_pLock->signal();   // Signal semaphore to indicate callback completion.
   }

   /// ================================================================
   /// @brief        Service event event handler, invoked by service client.
   ///
   /// @param        rEvent    Read-only event reference.
   ///
   void OnServiceEvent(IEvent const& rEvent)
   {
      MOCKDEBUG(m_Message);
      m_pLock->signal();   // Signal semaphore to indicate callback completion.
   }

   /// ================================================================
   /// @brief        Work complete event handler, invoked by service client.
   ///
   /// @details      Indicates service work completion.
   ///
   /// @param        rTranID    Read-only TransactionID reference.
   ///
   void OnWorkComplete(TransactionID const& rTranID)
   {
      MOCKDEBUG(m_Message);
      //      dynamic_cast<CMockDoWorker*>(m_pServiceBase)->Release(TransactionID(), AAL_INFINITE_WAIT);
      m_pLock->signal();   // Signal semaphore to indicate callback completion.
   }

   /// ================================================================
   /// @brief        Additional work complete event handler, invoked by service
   ///               client.
   ///
   /// @details      Indicates service work completion for task number
   ///               2, showing how individual services can expose an
   ///               arbitrary number of functions or tasks.
   ///
   /// @param        rTranID    Read-only TransactionID reference.
   ///
   void OnWorkComplete2(TransactionID const& rTranID)
   {
      MOCKDEBUG(m_Message);
      m_pLock->signal();   // Signal semaphore to indicate callback completion.
   }

private:
   // members
   IServiceBase* m_pServiceBase;
   IServiceClient* m_pSvcClient;
   CRuntimeClientAdapter* m_pRCA;
   CListenerLock* m_pLock;
   int m_Result;
   string m_Message;

public:
   IServiceBase* getService()
   {
      ASSERT(m_pServiceBase);
      return m_pServiceBase;
   }

   IServiceClient* getClient()
   {
      ASSERT(m_pSvcClient);
      return m_pSvcClient;
   }
};

#endif   // __GTCOMMON_SERVICELISTENER_H__
