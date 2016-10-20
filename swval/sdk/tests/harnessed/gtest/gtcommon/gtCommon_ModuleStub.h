// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_MODULESTUB_H__
#define __GTCOMMON_MODULESTUB_H__

/// ===================================================================
/// @brief        The custom service factory template.
///
/// @details      Copied from the SDK source code for InProcSvcsFact and
///               modified. For use when a single module will contain
///               both client and service.
///
/// @tparam       I     The service class / object type.
///
template <typename I>
// ServiceBase::ServiceBase(AALServiceModule *container,
//                          IRuntime         *pAALRuntime,
//                          IAALTransport    *ptransport,
//                          IAALMarshaller   *marshaller,
//                          IAALUnMarshaller *unmarshaller) :
class GTCOMMON_API InModuleSvcsFact : public EmptyISvcsFact,
                                      public IAcceptsBuilders
{
public:
   IServiceBuilder* m_pBuilder;
   IBase* CreateServiceObject(AALServiceModule* container,
                              IRuntime* pRuntime,
                              IAALTransport* ptransport = NULL,
                              IAALMarshaller* marshaller = NULL,
                              IAALUnMarshaller* unmarshaller = NULL)
   {
      I* pService = NULL;
      /// =============================================================
      /// @todo The builder (to be done right) should ultimately be
      /// derived from CAASBase and use the interface map mechanism.
      ///
      if(NULL != m_pBuilder) {
         pService = new (std::nothrow)
            I(container,
              pRuntime,
              dynamic_cast<IAALTransport*>(m_pBuilder),
              dynamic_cast<IAALMarshaller*>(m_pBuilder),
              dynamic_cast<IAALUnMarshaller*>(m_pBuilder));
      } else {
         pService = new (std::nothrow)
            I(container, pRuntime, ptransport, marshaller, unmarshaller);
      }
      if(NULL == pService) {
         return NULL;
      }
      return dynamic_cast<IBase*>(pService);
   }

   btBool InitializeService(IBase* newService,
                            IBase* Client,
                            TransactionID const& rtid,
                            NamedValueSet const& optArgs)
   {
      I* pobj = dynamic_cast<I*>(newService);
      if(NULL != pobj) {
         return pobj->_init(Client, rtid, optArgs, NULL);
      }
      return false;
   }

   void DestroyServiceObject(IBase* pServiceBase)
   {
      I* pobj = dynamic_cast<I*>(pServiceBase);
      if(NULL != pobj) {
         delete pobj;
      }
   }

   void acceptBuilder(IServiceBuilder* pBuilder)
   {
      m_pBuilder = pBuilder;
   }
};

namespace
{
/// ===================================================================
/// @internal Anonymous namespace to avoid use of global functions.
///
AALServiceModule* pModule = NULL;
InModuleSvcsFact<CMockDoWorker>* pFactory = NULL;

AALServiceModule* GetServiceModule()
{
   if(NULL == pModule) {
      pModule = new (std::nothrow)
         AALServiceModule(reinterpret_cast<ISvcsFact*>(pFactory));
      ASSERT(pModule);
   }
   return pModule;
}

InModuleSvcsFact<CMockDoWorker>* GetServiceFactory()
{
   if(NULL == pFactory) {
      pFactory = new (std::nothrow) InModuleSvcsFact<CMockDoWorker>();
      ASSERT(pFactory);
   }
   return pFactory;
}
}   // end anonymous namespace

#endif   // __GTCOMMON_MODULESTUB_H__
