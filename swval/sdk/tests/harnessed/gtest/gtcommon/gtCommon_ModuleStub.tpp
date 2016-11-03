// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// This is the template implementation code, included at the end of
// gtCommon_ModuleStub.h
//

template <typename I>
IBase* InModuleSvcsFact
   <I>::CreateServiceObject( AALServiceModule* container, IRuntime* pRuntime )
{
   I* pService = NULL;

   // Create the complex object when a builder is present
   if ( NULL != m_pBuilder ) {

      pService = new ( std::nothrow )
         I( container,
            pRuntime,
            (IAALTransport*)m_pBuilder->Interface( iidAALTransport ),
            (IAALMarshaller*)m_pBuilder->Interface( iidAALMarshaller ),
            (IAALUnMarshaller*)m_pBuilder->Interface( iidAALUnMarshaller ) );

   } else {
      // else create the standard object
      pService = new ( std::nothrow )
         I( container, pRuntime, NULL, NULL, NULL );
   }
   // NULL the builder pointer when finished to avoid re-use
   // Leave clean-up to the owner / creator
   m_pBuilder = NULL;
   if ( NULL == pService ) {
      return NULL;
   }
   return dynamic_cast<IBase*>( pService );
}

template <typename I>
btBool InModuleSvcsFact<I>::InitializeService( IBase* newService,
                                               IBase* Client,
                                               TransactionID const& rtid,
                                               NamedValueSet const& optArgs )
{
   I* pobj = dynamic_cast<I*>( newService );
   if ( NULL != pobj ) {
      return pobj->_init( Client, rtid, optArgs, NULL );
   }
   return false;
}

template <typename I>
void InModuleSvcsFact<I>::DestroyServiceObject( IBase* pServiceBase )
{
   // delegate clean-up to base classes
   EmptyISvcsFact::DestroyServiceObject( pServiceBase );
   // I* pobj = dynamic_cast<I*>(pServiceBase);
   // delete pobj;
   // pobj = NULL;
}

// Consumes a builder
// Must be set to NULL after use, clean-up being left to the provider / owner
template <typename I>
void InModuleSvcsFact<I>::acceptBuilder( CAASBuilder const* pSB )
{
   m_pBuilder = pSB;
}

// static factory and module, causing instance variables to persist
// which must be voided (set to NULL) for proper control-flow logic in
// the CreateServiceObject function
template <typename I>
AALServiceModule* InModuleSvcsFact<I>::pModule = NULL;

template <typename I>
AALServiceModule* InModuleSvcsFact<I>::GetServiceModule()
{
   if ( NULL == InModuleSvcsFact<I>::pModule ) {
      InModuleSvcsFact<I>::pModule = new ( std::nothrow ) AALServiceModule(
         reinterpret_cast<ISvcsFact*>( InModuleSvcsFact<I>::pFactory ) );
      ASSERT( InModuleSvcsFact::pModule );
   }
   return InModuleSvcsFact<I>::pModule;
}

template <typename I>
InModuleSvcsFact<I>* InModuleSvcsFact<I>::pFactory = NULL;

template <typename I>
InModuleSvcsFact<I>* InModuleSvcsFact<I>::GetServiceFactory()
{
   if ( NULL == InModuleSvcsFact<I>::pFactory ) {
      InModuleSvcsFact<I>::pFactory = new ( std::nothrow ) InModuleSvcsFact
         <I>();
      ASSERT( InModuleSvcsFact<I>::pFactory );
   }
   return InModuleSvcsFact<I>::pFactory;
}
