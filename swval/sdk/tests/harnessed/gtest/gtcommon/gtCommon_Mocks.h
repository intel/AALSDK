// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_MOCKS_H__
#define __GTCOMMON_MOCKS_H__

// Means for tracking that a member function (or method) on an object instance has been called.
// Saves the method name along with zero or more parameters and their types.
// Preserves the order in which parameters were added to the entry via AddParam(), making the order
// available via ParamName(unsigned ) and ParamType(unsigned ).
class GTCOMMON_API MethodCallLogEntry
{
public:
   MethodCallLogEntry(btcString method, Timer timestamp=Timer());

   btcString MethodName() const;

   void AddParam(btcString , btBool                );
   void AddParam(btcString , btByte                );
   void AddParam(btcString , bt32bitInt            );
   void AddParam(btcString , btUnsigned32bitInt    );
   void AddParam(btcString , bt64bitInt            );
   void AddParam(btcString , btUnsigned64bitInt    );
   void AddParam(btcString , btFloat               );
   void AddParam(btcString , btcString             );
   void AddParam(btcString , btObjectType          );
   void AddParam(btcString , INamedValueSet *      );
   void AddParam(btcString , const TransactionID & );

   unsigned       Params()           const;
   std::string ParamName(unsigned i) const;
   eBasicTypes ParamType(unsigned i) const;

   void GetParam(btcString , btBool                * ) const;
   void GetParam(btcString , btByte                * ) const;
   void GetParam(btcString , bt32bitInt            * ) const;
   void GetParam(btcString , btUnsigned32bitInt    * ) const;
   void GetParam(btcString , bt64bitInt            * ) const;
   void GetParam(btcString , btUnsigned64bitInt    * ) const;
   void GetParam(btcString , btFloat               * ) const;
   void GetParam(btcString , btcString             * ) const;
   void GetParam(btcString , btObjectType          * ) const;
   void GetParam(btcString , INamedValueSet const ** ) const;
   void GetParam(btcString , TransactionID &         ) const;

protected:
   Timer         m_TimeStamp;
   NamedValueSet m_NVS;

   struct TracksParamOrder
   {
      TracksParamOrder(btcString ParamName, eBasicTypes Type) :
         m_ParamName(ParamName),
         m_Type(Type)
      {}
      std::string m_ParamName;
      eBasicTypes m_Type;
   };

   std::list<TracksParamOrder> m_Order;
};

// Tracks a sequence of member function (method) calls on an interface in the order they were
// received. Allows for querying and printing the logged sequence.
class GTCOMMON_API MethodCallLog : public CriticalSection
{
public:
   MethodCallLog() {}

   MethodCallLogEntry *    AddToLog(btcString method) const;
   unsigned              LogEntries()                 const;
   const MethodCallLogEntry & Entry(unsigned i)       const;
   void                    ClearLog();

protected:
   typedef std::list<MethodCallLogEntry> LogList;
   typedef LogList::iterator             iterator;
   typedef LogList::const_iterator       const_iterator;

   mutable LogList m_LogList;
};

GTCOMMON_API std::ostream & operator << (std::ostream & , const MethodCallLog & );

////////////////////////////////////////////////////////////////////////////////

// For the mock classes below, it's valuable to control what value an interface function
// returns. For a given mock class, this macro defines two new member functions that enable
// us to set and get the return value for an interface method.
// ex).
//
//   class MyMockClass : public ISomeInterface
//   {
//   public:
//
//   // Method from ISomeInterface. We would like to be able to control the value returned
//   // by MyMethod() when it is called. Some test cases might require true, while others false.
//   virtual btBool MyMethod(NamedValueSet const & );
//
//   DECLARE_RETVAL_ACCESSORS(MyMethod, btBool)
//
//   // The result is these two function signatures: an accessor and a mutator.
//   // btBool MyMethodReturnsThisValue() const;
//   // void MyMethodReturnsThisValue(btBool );
//
//   protected:
//   // One then obviously needs to also provide a member variable to back the accessor and mutator:
//      btBool m_MyMethod_returns;
//   };
#define DECLARE_RETVAL_ACCESSORS(__membfn, __rettype) \
__rettype __membfn##ReturnsThisValue() const;         \
void __membfn##ReturnsThisValue(__rettype );

// In the implementation file, this macro is used to declare the accessor and mutator
// implementations.
// ex).
//
//   IMPLEMENT_RETVAL_ACCESSORS(MyMockClass, MyMethod, btBool, m_MyMethod_returns)
//
//   // The result is these two function implementations:
//   // btBool MyMockClass::MyMethodReturnsThisValue() const { return m_MyMethod_returns; }
//   // void MyMockClass::MyMethodReturnsThisValue(btBool x) { m_MyMethod_returns = x;    }
//
#define IMPLEMENT_RETVAL_ACCESSORS(__cls, __membfn, __rettype, __membvar) \
__rettype __cls::__membfn##ReturnsThisValue() const { return __membvar; } \
void __cls::__membfn##ReturnsThisValue(__rettype x) { __membvar = x;    }


// Naming Convention for mock classes:
//
//  EmptyIfc -         interface Ifc is implemented, but its virtual routines do nothing.
//                     They are empty.
//
//  CallTrackingIfc -  inherits EmptyIfc, but additionally implements MethodCallLog. Each of
//                     the virtual methods of Ifc make call log entries via
//                     MethodCallLog::AddToLog().
//
//  SynchronizingIfc - inherits CallTrackingIfc. Calls the CallTrackingIfc implementation of
//                     the virtual method, then calls Post(). Waiters may call Wait() to
//                     synchronize.
//

////////////////////////////////////////////////////////////////////////////////
// IAALTransport

class GTCOMMON_API EmptyIAALTransport : public AAL::IAALTransport
{
public:
   EmptyIAALTransport();
   virtual ~EmptyIAALTransport() {}

   virtual btBool  connectremote(NamedValueSet const & );
   virtual btBool waitforconnect(NamedValueSet const & );
   virtual btBool     disconnect();
   virtual btcString      getmsg(btWSSize * );
   virtual int            putmsg(btcString , btWSSize );

   DECLARE_RETVAL_ACCESSORS(connectremote,  btBool    )
   DECLARE_RETVAL_ACCESSORS(waitforconnect, btBool    )
   DECLARE_RETVAL_ACCESSORS(disconnect,     btBool    )
   DECLARE_RETVAL_ACCESSORS(getmsg,         btcString )
   DECLARE_RETVAL_ACCESSORS(putmsg,         int       )

protected:
   btBool    m_connectremote_returns;
   btBool    m_waitforconnect_returns;
   btBool    m_disconnect_returns;
   btcString m_getmsg_returns;
   int       m_putmsg_returns;
};

////////////////////////////////////////////////////////////////////////////////
// IAALMarshaller

class GTCOMMON_API EmptyIAALMarshaller : public AAL::IAALMarshaller
{
public:
   EmptyIAALMarshaller();
   virtual ~EmptyIAALMarshaller() {}

   virtual ENamedValues   Empty();
   virtual btBool           Has(btStringKey )                   const;
   virtual ENamedValues  Delete(btStringKey );
   virtual ENamedValues GetSize(btStringKey   , btWSSize    * ) const;
   virtual ENamedValues    Type(btStringKey   , eBasicTypes * ) const;
   virtual ENamedValues GetName(btUnsignedInt , btStringKey * ) const;

   virtual ENamedValues Add(btNumberKey Name, btBool                Value);
   virtual ENamedValues Add(btNumberKey Name, btByte                Value);
   virtual ENamedValues Add(btNumberKey Name, bt32bitInt            Value);
   virtual ENamedValues Add(btNumberKey Name, btUnsigned32bitInt    Value);
   virtual ENamedValues Add(btNumberKey Name, bt64bitInt            Value);
   virtual ENamedValues Add(btNumberKey Name, btUnsigned64bitInt    Value);
   virtual ENamedValues Add(btNumberKey Name, btFloat               Value);
   virtual ENamedValues Add(btNumberKey Name, btcString             Value);
   virtual ENamedValues Add(btNumberKey Name, btObjectType          Value);
   virtual ENamedValues Add(btNumberKey Name, const INamedValueSet *Value);

   virtual ENamedValues Add(btNumberKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btNumberKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements);

   virtual ENamedValues Add(btStringKey Name, btBool                Value);
   virtual ENamedValues Add(btStringKey Name, btByte                Value);
   virtual ENamedValues Add(btStringKey Name, bt32bitInt            Value);
   virtual ENamedValues Add(btStringKey Name, btUnsigned32bitInt    Value);
   virtual ENamedValues Add(btStringKey Name, bt64bitInt            Value);
   virtual ENamedValues Add(btStringKey Name, btUnsigned64bitInt    Value);
   virtual ENamedValues Add(btStringKey Name, btFloat               Value);
   virtual ENamedValues Add(btStringKey Name, btcString             Value);
   virtual ENamedValues Add(btStringKey Name, btObjectType          Value);
   virtual ENamedValues Add(btStringKey Name, const INamedValueSet *Value);

   virtual ENamedValues Add(btStringKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements);
   virtual ENamedValues Add(btStringKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements);

   virtual char const * pmsgp(btWSSize *len);

   DECLARE_RETVAL_ACCESSORS(pmsgp, char const *)

protected:
   const char   *m_pmsgp_returns;
   NamedValueSet m_NVS;
};

////////////////////////////////////////////////////////////////////////////////
// IAALUnMarshaller

class GTCOMMON_API EmptyIAALUnMarshaller : public AAL::IAALUnMarshaller
{
public:
   EmptyIAALUnMarshaller();
   virtual ~EmptyIAALUnMarshaller() {}

   virtual ENamedValues   Empty();
   virtual btBool           Has(btStringKey )                   const;
   virtual ENamedValues  Delete(btStringKey );
   virtual ENamedValues GetSize(btStringKey   , btWSSize    * ) const;
   virtual ENamedValues    Type(btStringKey   , eBasicTypes * ) const;
   virtual ENamedValues GetName(btUnsignedInt , btStringKey * ) const;

   virtual ENamedValues Get(btNumberKey Name, btBool                   *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btByte                   *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt32bitInt               *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitInt       *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt64bitInt               *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitInt       *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btFloat                  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btcString                *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btObjectType             *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, INamedValueSet const    **pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btByteArray              *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt32bitIntArray          *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, bt64bitIntArray          *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray  *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btFloatArray             *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btStringArray            *pValue) const;
   virtual ENamedValues Get(btNumberKey Name, btObjectArray            *pValue) const;

   virtual ENamedValues Get(btStringKey Name, btBool                   *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btByte                   *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt32bitInt               *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitInt       *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt64bitInt               *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitInt       *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btFloat                  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btcString                *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btObjectType             *pValue) const;
   virtual ENamedValues Get(btStringKey Name, INamedValueSet const    **pValue) const;
   virtual ENamedValues Get(btStringKey Name, btByteArray              *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt32bitIntArray          *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, bt64bitIntArray          *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray  *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btFloatArray             *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btStringArray            *pValue) const;
   virtual ENamedValues Get(btStringKey Name, btObjectArray            *pValue) const;

   virtual void   importmsg(char const *pmsg, btWSSize len);

protected:
   NamedValueSet m_NVS;
};

////////////////////////////////////////////////////////////////////////////////
// IServiceClient

class GTCOMMON_API EmptyIServiceClient : public AAL::IServiceClient,
                                         public AAL::CAASBase
{
public:
   EmptyIServiceClient();
   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & )    {}
   virtual void serviceAllocateFailed(const IEvent & )           {}
   virtual void       serviceReleased(TransactionID const & )    {}
   virtual void serviceReleaseRequest(IBase *, const IEvent & )  {}
   virtual void  serviceReleaseFailed(const IEvent & )           {}
   virtual void          serviceEvent(const IEvent & )           {}
};

class GTCOMMON_API CallTrackingIServiceClient : public EmptyIServiceClient,
                                                public MethodCallLog
{
public:
   CallTrackingIServiceClient();
   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & );
   virtual void serviceAllocateFailed(const IEvent & );
   virtual void       serviceReleased(TransactionID const & );
   virtual void  serviceReleaseFailed(const IEvent & );
   virtual void          serviceEvent(const IEvent & );
};

class GTCOMMON_API SynchronizingIServiceClient : public CallTrackingIServiceClient
{
public:
   SynchronizingIServiceClient();

   virtual void      serviceAllocated(IBase               * ,
                                      TransactionID const & );
   virtual void serviceAllocateFailed(const IEvent & );
   virtual void       serviceReleased(TransactionID const & );
   virtual void  serviceReleaseFailed(const IEvent & );
   virtual void          serviceEvent(const IEvent & );

   btBool Wait(btTime        Timeout=AAL_INFINITE_WAIT);
   btBool Post(btUnsignedInt Count=1);

protected:
   Barrier m_Bar;
};

////////////////////////////////////////////////////////////////////////////////
// IRuntimeClient

class GTCOMMON_API EmptyIRuntimeClient : public AAL::IRuntimeClient,
                                         public AAL::CAASBase
{
public:
   EmptyIRuntimeClient();
   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & )        {}
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & ) {}
   virtual void                  runtimeStopped(IRuntime * )            {}
   virtual void              runtimeStartFailed(const IEvent & )        {}
   virtual void               runtimeStopFailed(const IEvent & )        {}
   virtual void    runtimeAllocateServiceFailed(IEvent const & )        {}
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & ) {}
   virtual void                    runtimeEvent(const IEvent & )        {}
};

class GTCOMMON_API CallTrackingIRuntimeClient : public EmptyIRuntimeClient,
                                                public MethodCallLog
{
public:
   CallTrackingIRuntimeClient();
   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & );
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & );
   virtual void                  runtimeStopped(IRuntime * );
   virtual void              runtimeStartFailed(const IEvent & );
   virtual void               runtimeStopFailed(const IEvent & );
   virtual void    runtimeAllocateServiceFailed(IEvent const & );
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & );
   virtual void                    runtimeEvent(const IEvent & );
};

class GTCOMMON_API SynchronizingIRuntimeClient : public CallTrackingIRuntimeClient
{
public:
   SynchronizingIRuntimeClient();

   virtual void   runtimeCreateOrGetProxyFailed(IEvent const & );
   virtual void                  runtimeStarted(IRuntime            * ,
                                                const NamedValueSet & );
   virtual void                  runtimeStopped(IRuntime * );
   virtual void              runtimeStartFailed(const IEvent & );
   virtual void               runtimeStopFailed(const IEvent & );
   virtual void    runtimeAllocateServiceFailed(IEvent const & );
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & );
   virtual void                    runtimeEvent(const IEvent & );

   btBool Wait(btTime        Timeout=AAL_INFINITE_WAIT);
   btBool Post(btUnsignedInt Count=1);

protected:
   Barrier m_Bar;
};

////////////////////////////////////////////////////////////////////////////////
// ISvcsFact

class GTCOMMON_API EmptyISvcsFact : public AAL::ISvcsFact
{
public:
   EmptyISvcsFact();

   virtual IBase * CreateServiceObject(AALServiceModule * ,
                                       IRuntime         * );
   virtual void   DestroyServiceObject(IBase * ) { }
   virtual btBool    InitializeService(IBase               * ,
                                       IBase               * ,
                                       TransactionID const & ,
                                       NamedValueSet const & );
   DECLARE_RETVAL_ACCESSORS(CreateServiceObject, IBase *)
   DECLARE_RETVAL_ACCESSORS(InitializeService,   btBool)
protected:
   IBase  *m_CreateServiceObject_returns;
   btBool  m_InitializeService_returns;
};

class GTCOMMON_API CallTrackingISvcsFact : public EmptyISvcsFact,
                                           public MethodCallLog
{
public:
   CallTrackingISvcsFact() {}
   virtual IBase * CreateServiceObject(AALServiceModule * ,
                                       IRuntime         * );
   virtual void   DestroyServiceObject(IBase * );
   virtual btBool    InitializeService(IBase               * ,
                                       IBase               * ,
                                       TransactionID const & ,
                                       NamedValueSet const & );
};

////////////////////////////////////////////////////////////////////////////////
// IRuntime

class GTCOMMON_API EmptyIRuntime : public AAL::IRuntime,
                                   public AAL::CAASBase
{
public:
   EmptyIRuntime();

   virtual btBool                     start(const NamedValueSet & );
   virtual void                        stop() {}
   virtual void                allocService(IBase               * ,
                                            NamedValueSet const & ,
                                            TransactionID const & = TransactionID()) {}
   virtual btBool         schedDispatchable(IDispatchable * );
   virtual IRuntime *       getRuntimeProxy(IRuntimeClient * );
   virtual btBool       releaseRuntimeProxy();
   virtual IRuntimeClient *getRuntimeClient();
   virtual btBool                      IsOK();

   DECLARE_RETVAL_ACCESSORS(start,               btBool           )
   DECLARE_RETVAL_ACCESSORS(schedDispatchable,   btBool           )
   DECLARE_RETVAL_ACCESSORS(getRuntimeProxy,     IRuntime *       )
   DECLARE_RETVAL_ACCESSORS(releaseRuntimeProxy, btBool           )
   DECLARE_RETVAL_ACCESSORS(getRuntimeClient,    IRuntimeClient * )
   DECLARE_RETVAL_ACCESSORS(IsOK,                btBool           )

protected:
   btBool          m_start_returns;
   btBool          m_schedDispatchable_returns;
   IRuntime       *m_getRuntimeProxy_returns;
   btBool          m_releaseRuntimeProxy_returns;
   IRuntimeClient *m_getRuntimeClient_returns;
   btBool          m_IsOK_returns;
};

class GTCOMMON_API CallTrackingIRuntime : public EmptyIRuntime,
                                          public MethodCallLog
{
public:
   CallTrackingIRuntime();
   virtual btBool                     start(const NamedValueSet & );
   virtual void                        stop();
   virtual void                allocService(IBase                * ,
                                            NamedValueSet const &rManifest,
                                            TransactionID const &rTranID = TransactionID());
   virtual btBool         schedDispatchable(IDispatchable * );
   virtual IRuntime *       getRuntimeProxy(IRuntimeClient * );
   virtual btBool       releaseRuntimeProxy();
   virtual IRuntimeClient *getRuntimeClient();
   virtual btBool                      IsOK();
};

////////////////////////////////////////////////////////////////////////////////
// IServiceBase

class GTCOMMON_API EmptyIServiceBase : public AAL::IServiceBase,
                                       public AAL::CAASBase
{
public:
   EmptyIServiceBase();

   virtual btBool                     initComplete(TransactionID const &rtid);
   virtual btBool                       initFailed(IEvent const *ptheEvent);
   virtual btBool                  ReleaseComplete();
   virtual NamedValueSet const &           OptArgs() const;
   virtual IServiceClient *       getServiceClient() const;
   virtual IBase *            getServiceClientBase() const;
   virtual IRuntime *                   getRuntime() const;
   virtual IRuntimeClient *       getRuntimeClient() const;
   virtual AALServiceModule *  getAALServiceModule() const;

   DECLARE_RETVAL_ACCESSORS(initComplete,         btBool)
   DECLARE_RETVAL_ACCESSORS(initFailed,           btBool)
   DECLARE_RETVAL_ACCESSORS(ReleaseComplete,      btBool)
   DECLARE_RETVAL_ACCESSORS(OptArgs,              NamedValueSet const &)
   DECLARE_RETVAL_ACCESSORS(getServiceClient,     IServiceClient *)
   DECLARE_RETVAL_ACCESSORS(getServiceClientBase, IBase *)
   DECLARE_RETVAL_ACCESSORS(getRuntime,           IRuntime *)
   DECLARE_RETVAL_ACCESSORS(getRuntimeClient,     IRuntimeClient *)
   DECLARE_RETVAL_ACCESSORS(getAALServiceModule,  AALServiceModule *)

protected:
   btBool            m_initComplete_returns;
   btBool            m_initFailed_returns;
   btBool            m_ReleaseComplete_returns;
   IServiceClient   *m_getServiceClient_returns;
   IBase            *m_getServiceClientBase_returns;
   IRuntime         *m_getRuntime_returns;
   IRuntimeClient   *m_getRuntimeClient_returns;
   AALServiceModule *m_getAALServiceModule_returns;
   NamedValueSet     m_OptArgs_returns;
};

class GTCOMMON_API CallTrackingIServiceBase : public EmptyIServiceBase,
                                              public MethodCallLog
{
public:
   CallTrackingIServiceBase();

   virtual btBool                     initComplete(TransactionID const &rtid);
   virtual btBool                       initFailed(IEvent const *ptheEvent);
   virtual btBool                  ReleaseComplete();
   virtual NamedValueSet const &           OptArgs() const;
   virtual IServiceClient *       getServiceClient() const;
   virtual IBase *            getServiceClientBase() const;
   virtual IRuntime *                   getRuntime() const;
   virtual IRuntimeClient *       getRuntimeClient() const;
   virtual AALServiceModule *  getAALServiceModule() const;
};

////////////////////////////////////////////////////////////////////////////////
// ServiceBase

class GTCOMMON_API EmptyServiceBase : public AAL::ServiceBase
{
public:
   EmptyServiceBase(AALServiceModule *container,
                    IRuntime         *pAALRUNTIME,
                    IAALTransport    *ptransport,
                    IAALMarshaller   *marshaller,
                    IAALUnMarshaller *unmarshaller);

   virtual btBool init(IBase               * ,
                       NamedValueSet const & ,
                       TransactionID const & );

   DECLARE_RETVAL_ACCESSORS(init, btBool)

   void     ServiceClient(IServiceClient * );
   void ServiceClientBase(IBase * );
   void                RT(IRuntime * );

protected:
   btBool m_init_returns;
};

class GTCOMMON_API CallTrackingServiceBase : public EmptyServiceBase,
                                             public MethodCallLog
{
public:
   CallTrackingServiceBase(AALServiceModule *container,
                           IRuntime         *pAALRUNTIME,
                           IAALTransport    *ptransport,
                           IAALMarshaller   *marshaller,
                           IAALUnMarshaller *unmarshaller);

   virtual btBool  init(IBase               *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &rtid);
   virtual btBool _init(IBase               *pclientBase,
                        TransactionID const &rtid,
                        NamedValueSet const &optArgs,
                        CAALEvent           *pcmpltEvent=NULL);

   // <IAALService>
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);
   // </IAALService>

   // <IServiceBase>
   virtual btBool                    initComplete(TransactionID const &rtid);
   virtual btBool                      initFailed(IEvent const        *ptheEvent);
   virtual btBool                 ReleaseComplete();
   virtual NamedValueSet const &          OptArgs() const;
   virtual IServiceClient *      getServiceClient() const;
   virtual IBase *           getServiceClientBase() const;
   virtual IRuntime *                  getRuntime() const;
   virtual IRuntimeClient *      getRuntimeClient() const;
   virtual AALServiceModule * getAALServiceModule() const;
   // </IServiceBase>
};

////////////////////////////////////////////////////////////////////////////////
// IServiceModule / IServiceModuleCallback

class GTCOMMON_API EmptyServiceModule : public AAL::IServiceModule,
                                        public AAL::IServiceModuleCallback
{
public:
   EmptyServiceModule();

   // <IServiceModule>
   virtual btBool Construct(IRuntime            *pAALRUNTIME,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet());
   virtual void     Destroy();
   // </IServiceModule>

   // <IServiceModuleCallback>

   virtual void      ServiceReleased(IBase * );
   virtual btBool ServiceInitialized(IBase * , TransactionID const & );
   virtual btBool  ServiceInitFailed(IBase * , IEvent        const * );

   // </IServiceModuleCallback>

   DECLARE_RETVAL_ACCESSORS(Construct,          btBool)
   DECLARE_RETVAL_ACCESSORS(ServiceInitialized, btBool)
   DECLARE_RETVAL_ACCESSORS(ServiceInitFailed,  btBool)

protected:
   btBool m_Construct_returns;
   btBool m_ServiceInitialized_returns;
   btBool m_ServiceInitFailed_returns;
};

class GTCOMMON_API CallTrackingServiceModule : public EmptyServiceModule,
                                               public MethodCallLog
{
public:
   CallTrackingServiceModule();

   // <IServiceModule>
   virtual btBool Construct(IRuntime            *pAALRUNTIME,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet());
   virtual void     Destroy();
   // </IServiceModule>

   // <IServiceModuleCallback>

   virtual void      ServiceReleased(IBase * );
   virtual btBool ServiceInitialized(IBase * , TransactionID const & );
   virtual btBool  ServiceInitFailed(IBase * , IEvent        const * );

   // </IServiceModuleCallback>
};

////////////////////////////////////////////////////////////////////////////////
// sw validation module / service module

GTCOMMON_API void    AllocSwvalMod(AAL::IRuntime * ,
                                   AAL::IBase    * ,
                                   const AAL::TransactionID & );

GTCOMMON_API void AllocSwvalSvcMod(AAL::IRuntime * ,
                                   AAL::IBase    * ,
                                   const AAL::TransactionID & );

class GTCOMMON_API EmptySwvalSvcClient : public ISwvalSvcClient,
                                         public EmptyIServiceClient
{
public:
   EmptySwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

class GTCOMMON_API CallTrackingSwvalSvcClient : public ISwvalSvcClient,
                                                public CallTrackingIServiceClient
{
public:
   CallTrackingSwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

class GTCOMMON_API SynchronizingSwvalSvcClient : public ISwvalSvcClient,
                                                 public SynchronizingIServiceClient
{
public:
   SynchronizingSwvalSvcClient();
   virtual void DidSomething(const AAL::TransactionID & , int );
};

#endif // __GTCOMMON_MOCKS_H__

