// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "swvalsvcmod.h"

static const char *Something_in_the_mod = "This variable is in the service mod.";

class CSwvalSvcMod : public ISwvalSvcMod,
                     public AAL::ServiceBase
{
public:
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CSwvalSvcMod, AAL::ServiceBase)
   {
      SetSubClassInterface(iidSwvalSvc, dynamic_cast<ISwvalSvcMod *>(this));
   }

   virtual ~CSwvalSvcMod() {}
   virtual int DoSomething(int i) { return i + 1; }

   virtual void init(AAL::TransactionID const &tid)
   {
      allocService(AAL::dynamic_ptr<AAL::IBase>(iidBase, this),
                   AAL::NamedValueSet(),
                   AAL::TransactionID());
   }

   virtual AAL::btBool Release(AAL::TransactionID const &tid, AAL::btTime timeout=AAL_INFINITE_WAIT)
   {
      return AAL::ServiceBase::Release(tid, timeout);
   }

   virtual AAL::btBool Release(AAL::btTime timeout=AAL_INFINITE_WAIT)
   {
      return AAL::ServiceBase::Release(timeout);
   }
};


AAL_BEGIN_SVC_MOD(AAL::InProcSvcsFact< CSwvalSvcMod >,
                  libswvalsvcmod,
                  SWVALSVCMOD_API,
                  SWVALSVCMOD_VERSION,
                  SWVALSVCMOD_VERSION_CURRENT,
                  SWVALSVCMOD_VERSION_REVISION,
                  SWVALSVCMOD_VERSION_AGE)

   AAL_BEGIN_SVC_MOD_CMD(SWVALSVCMOD_CMD_SAY_HELLO)
      strncpy((char *)arg, "hello", 5);
   AAL_END_SVC_MOD_CMD()

   AAL_BEGIN_SVC_MOD_CMD(SWVALSVCMOD_CMD_MALLOC_STRUCT)
      swvalsvcmod_obj *p = (swvalsvcmod_obj *) malloc(sizeof(swvalsvcmod_obj));

      p->something_in_the_mod = Something_in_the_mod;
      strncpy(p->msg, "Message from libswvalsvcmod.", sizeof(p->msg));
      p->intfield = 7;

      *(swvalsvcmod_obj **)arg = p;
   AAL_END_SVC_MOD_CMD()

   AAL_BEGIN_SVC_MOD_CMD(SWVALSVCMOD_CMD_FREE)
      swvalsvcmod_obj *p = (swvalsvcmod_obj *) arg;

      if ( 0 != strncmp(p->something_in_the_mod, "In the test.", 12) ) {
         res = 1;
      }
      if ( 0 != strncmp(p->msg, "abc", 3) ) {
         res = 2;
      }
      if ( 6 != p->intfield ) {
         res = 3;
      }

      free(arg);
   AAL_END_SVC_MOD_CMD()

AAL_END_SVC_MOD()

