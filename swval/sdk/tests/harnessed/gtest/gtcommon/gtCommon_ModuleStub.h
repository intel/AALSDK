#ifndef __GTCOMMON_MODULESTUB_H__
#define __GTCOMMON_MODULESTUB_H__

#include "gtCommon.h"
#include "gtCommon_SMocks.h"
#include "gtCommon_DoWorker.h"

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
class GTCOMMON_API InModuleSvcsFact : public ISvcsFact
{
public:
   IBase* CreateServiceObject(AALServiceModule* container, IRuntime* pRuntime)
   {
      I* pService = new (std::nothrow) I(container, pRuntime);
      if(NULL == pService) {
         return NULL;
      }
      return dynamic_cast<IBase*>(pService);
   }

   btBool InitializeService(IBase* newService, IBase* Client, TransactionID const& rtid, NamedValueSet const& optArgs)
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
      pModule = new (std::nothrow) AALServiceModule(reinterpret_cast<ISvcsFact*>(pFactory));
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
