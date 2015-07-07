// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __SWVALSVCMOD_H__
#define __SWVALSVCMOD_H__
#include <aalsdk/AAL.h>
#include <aalsdk/AALBase.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/aas/AALRuntimeModule.h>

#if defined( __AAL_USER__ ) && defined( __AAL_WINDOWS__ )
# ifdef SWVALSVCMOD_EXPORTS
#    define SWVALSVCMOD_API __declspec(dllexport)
# else
#    define SWVALSVCMOD_API __declspec(dllimport)
# endif // SWVALSVCMOD_EXPORTS
#else
# define __declspec(x)
# define SWVALSVCMOD_API __declspec(0)
#endif // Windows / User Mode

AAL_DECLARE_SVC_MOD(libswvalsvcmod, SWVALSVCMOD_API)

// Define some custom commands for the module.
#define SWVALSVCMOD_CMD_SAY_HELLO     (AAL_SVC_USER_CMD_BASE + 0)

#define SWVALSVCMOD_CMD_MALLOC_STRUCT (AAL_SVC_USER_CMD_BASE + 1)
typedef struct _swvalsvcmod_obj
{
   const char *something_in_the_mod;
   char        msg[32];
   int         intfield;
} swvalsvcmod_obj;

#define SWVALSVCMOD_CMD_FREE          (AAL_SVC_USER_CMD_BASE + 2)

#define SWVALSVCMOD_CMD_NEW_OBJ       (AAL_SVC_USER_CMD_BASE + 3)
#define SWVALSVCMOD_CMD_DELETE_OBJ    (AAL_SVC_USER_CMD_BASE + 4)


#define iidSwvalSvc __INTC_IID(INTC_sysSampleAFU, 0x00fe)
class ISwvalSvcMod
{
public:
   virtual ~ISwvalSvcMod() {}
   virtual int DoSomething(int ) = 0;
};

#endif // __SWVALSVCMOD_H__

