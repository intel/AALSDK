// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "swvalsvcmod.h"

class CDidSomethingDisp : public IDispatchable
{
public:
   CDidSomethingDisp(ISwvalSvcClient          *pClient,
                     const AAL::TransactionID &tid,
                     int                       i) :
      m_pClient(pClient),
      m_tid(tid),
      m_i(i)
   {}

   virtual void operator() ()
   {
      m_pClient->DidSomething(m_tid, m_i);
      delete this;
   }

protected:
   ISwvalSvcClient          *m_pClient;
   const AAL::TransactionID &m_tid;
   int                       m_i;
};

////////////////////////////////////////////////////////////////////////////////

static const char *Something_in_the_mod = "This variable is in the service mod.";

class CSwvalSvcMod : public ISwvalSvcMod,
                     public AAL::ServiceBase
{
public:
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CSwvalSvcMod, AAL::ServiceBase)
   {
      if ( AAL::EObjOK != SetInterface(iidSwvalSvc, dynamic_cast<ISwvalSvcMod *>(this)) ) {
         m_bIsOK = false;
      }
   }

   virtual void DoSomething(const AAL::TransactionID &tid, int i)
   {
      ISwvalSvcClient *pSwvalClient = AAL::dynamic_ptr<ISwvalSvcClient>(iidSwvalSvcClient, m_pclientbase);
      ASSERT(NULL != pSwvalClient);

      getRuntime()->schedDispatchable( new CDidSomethingDisp(pSwvalClient, tid, i + 1) );
   }

   virtual ~CSwvalSvcMod() {}


   virtual AAL::btBool init(AAL::IBase               *pclientBase,
                            AAL::NamedValueSet const &optArgs,
                            AAL::TransactionID const &tid)
   {
      ASSERT(NULL != m_pclient); // iidServiceClient / IServiceClient *
      ASSERT(NULL != AAL::dynamic_ptr<ISwvalSvcClient>(iidSwvalSvcClient, pclientBase));
      return initComplete(tid);
   }

   virtual AAL::btBool Release(AAL::TransactionID const &tid, AAL::btTime timeout=AAL_INFINITE_WAIT)
   {
      return AAL::ServiceBase::Release(tid, timeout);
   }

   // <IServiceBase>

   virtual AAL::btBool initComplete(AAL::TransactionID const &rtid)
   {
      return AAL::ServiceBase::initComplete(rtid);
   }

   virtual AAL::btBool initFailed(AAL::IEvent const *ptheEvent)
   {
      return AAL::ServiceBase::initFailed(ptheEvent);
   }

   virtual AAL::btBool ReleaseComplete()
   {
      return AAL::ServiceBase::ReleaseComplete();
   }

   // </IServiceBase>

   // <IRuntimeClient>

   virtual void runtimeCreateOrGetProxyFailed(AAL::IEvent const &e)
   {

   }

   virtual void runtimeStarted(AAL::IRuntime            * ,
                               const AAL::NamedValueSet & )
   {

   }

   virtual void runtimeStopped(AAL::IRuntime * )
   {

   }

   virtual void runtimeStartFailed(const AAL::IEvent & )
   {

   }

   virtual void runtimeStopFailed(const AAL::IEvent & )
   {

   }

   virtual void runtimeAllocateServiceFailed(AAL::IEvent const & )
   {

   }

   virtual void runtimeAllocateServiceSucceeded(AAL::IBase               * ,
                                                AAL::TransactionID const & )
   {

   }

   virtual void runtimeEvent(const AAL::IEvent & )
   {

   }

   // </IRuntimeClient>
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

