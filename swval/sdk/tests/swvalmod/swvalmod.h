// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __SWVALMOD_H__
#define __SWVALMOD_H__
#include <aalsdk/osal/OSServiceModule.h>

#if defined( __AAL_USER__ ) && defined( __AAL_WINDOWS__ )
# ifdef SWVALMOD_EXPORTS
#    define SWVALMOD_API __declspec(dllexport)
# else
#    define SWVALMOD_API __declspec(dllimport)
# endif // SWVALMOD_EXPORTS
#else
# define __declspec(x)
# define SWVALMOD_API __declspec(0)
#endif // Windows / User Mode

AAL_DECLARE_MOD(libswvalmod, SWVALMOD_API)

// Define some custom commands for the module.
#define SWVALMOD_CMD_SAY_HELLO  (AAL_SVC_USER_CMD_BASE + 0)

#define SWVALMOD_CMD_MALLOC_OBJ (AAL_SVC_USER_CMD_BASE + 1)
typedef struct _swvalmod_obj
{
   int         intfield;
   char        msg[32];
   const char *something_in_the_mod;
} swvalmod_obj;

#define SWVALMOD_CMD_FREE       (AAL_SVC_USER_CMD_BASE + 2)

#endif // __SWVALMOD_H__

