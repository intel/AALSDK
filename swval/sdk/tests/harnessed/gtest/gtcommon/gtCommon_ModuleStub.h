// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_MODULESTUB_H__
#define __GTCOMMON_MODULESTUB_H__

/// ===================================================================
/// @brief        The custom service factory template.
///
/// @tparam       I     The service class / object type.
///

template <typename I>
class InModuleSvcsFact : public EmptyISvcsFact,
                                      public IAcceptsBuilders
{
public:
   IBase* CreateServiceObject( AALServiceModule* container,
                               IRuntime* pRuntime );

   btBool InitializeService( IBase* newService,
                             IBase* Client,
                             TransactionID const& rtid,
                             NamedValueSet const& optArgs );

   void DestroyServiceObject( IBase* pServiceBase );
   virtual void acceptBuilder( CAASBuilder const* pSB );

   // static functions had previously been in an anonymous namespace
   static AALServiceModule* GetServiceModule();
   static InModuleSvcsFact* GetServiceFactory();

   InModuleSvcsFact() : m_pBuilder( NULL )
   {
   }

   virtual ~InModuleSvcsFact()
   {
   }

private:
   CAASBuilder const* m_pBuilder;
   // static objects for use in static class functions
   static AALServiceModule* pModule;
   static InModuleSvcsFact<I>* pFactory;
};

// include the template implementation code
#include "gtCommon_ModuleStub.tpp"
#endif   // __GTCOMMON_MODULESTUB_H__
