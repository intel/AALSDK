// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_DOWORKER_H__
#define __GTCOMMON_DOWORKER_H__

/// ===================================================================
/// @brief        The service event listener interface, used for definition
///               of adapter classes to facilitate event delegation.
///
class GTCOMMON_API IServiceListener : public Listener
{
public:
   virtual void OnServiceAllocated(ServiceBase*) = 0;
   virtual void OnServiceAllocateFailed(IEvent const&) = 0;
   virtual void OnServiceReleaseFailed(IEvent const&) = 0;
   virtual void OnServiceReleased(TransactionID const&) = 0;
   virtual void OnServiceReleaseRequest(IBase*, IEvent const&) = 0;
   virtual void OnWorkComplete(TransactionID const&) = 0;
   virtual void OnWorkComplete2(TransactionID const&) = 0;
   virtual void OnServiceEvent(IEvent const&) = 0;

   virtual ~IServiceListener()
   {
   }
};

/// ===================================================================
/// @brief        The primary custom service interface.
///
/// @details      Clients access the service through this interface in
///               order to "do work".
///

#define iidMockDoWorker __INTC_IID(INTC_sysSampleAFU, 0x0001)
// @todo Investigate whether or not custom interfaces should be deriving
// from IBase
//
class GTCOMMON_API IMockDoWorker
{
public:
   virtual void dispatchWorkComplete(TransactionID const& rTranID) = 0;
   virtual void doWork() = 0;
};

/// ===================================================================
/// @brief        The primary custom service client interface.
///
/// @details      Clients of the service implement this interface to get
///               callback notifications.
///
#define iidMockWorkClient __INTC_IID(INTC_sysSampleAFU, 0x0002)

class GTCOMMON_API IMockWorkClient
{
public:
   virtual void setListener(IServiceListener*) = 0;
   virtual void workComplete(TransactionID const& rTranID) = 0;

   virtual ~IMockWorkClient()
   {
   }
};

/// ===================================================================
/// @brief        The custom service.
///
/// @details      Provides the work dispatch functions.
///
class GTCOMMON_API CMockDoWorker : public EmptyServiceBase,
                                   public IMockDoWorker,
                                   public IAcceptsVisitors
{
public:
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CMockDoWorker, EmptyServiceBase),
      m_pSvcClient(NULL), m_pWorkClient(NULL), m_pVisitingWorker(NULL)
   {
      SetInterface(iidMockDoWorker, dynamic_cast<IMockDoWorker*>(this));
   }

   // allow deletion from a base pointer
   virtual ~CMockDoWorker()
   {
   }

   virtual btBool init(IBase*, NamedValueSet const&, TransactionID const&);

   //<overrides from ServiceBase, IServiceBase>
   virtual btBool
      Release(TransactionID const&, btTime timeout = AAL_INFINITE_WAIT);
   virtual btBool initComplete(TransactionID const&);
   virtual AALServiceModule* getAALServiceModule() const;
   virtual IServiceClient* getServiceClient() const;
   //<overrides from ServiceBase, IServiceBase>

   virtual void doWork();
   virtual void dispatchWorkComplete(TransactionID const&);
   virtual void acceptVisitor(IVisitingWorker*);

protected:
   IServiceClient* m_pSvcClient;
   IMockWorkClient* m_pWorkClient;
   TransactionID m_CurrTranID;
   IVisitingWorker* m_pVisitingWorker;
};

/// ===================================================================
/// @brief        The primary custom service functor.
///
/// @details      The operator()() overload invokes work site(s) to
///               do work, perform tasks, etc.
///
class GTCOMMON_API CMockDispatchable : public IDispatchable
{
public:
   CMockDispatchable(IMockWorkClient*, IBase*, TransactionID const&);

   virtual void operator()();

   // allow deletion from a base pointer
   virtual ~CMockDispatchable()
   {
   }

protected:
   IMockWorkClient* m_pWorkClient;
   IBase* m_pService;
   TransactionID const& m_TranID;
};

#endif   // __GTCOMMON_DOWORKER_H__
