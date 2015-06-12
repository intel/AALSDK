// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "swvalmod.h"

static const char *Something_in_the_mod = "This variable is in the mod.";

AAL_BEGIN_MOD(libswvalmod,
              SWVALMOD_API,
              SWVALMOD_VERSION,
              SWVALMOD_VERSION_CURRENT,
              SWVALMOD_VERSION_REVISION,
              SWVALMOD_VERSION_AGE)

   AAL_BEGIN_MOD_CMD(SWVALMOD_CMD_SAY_HELLO)
      strncpy((char *)arg, "hello", 5);
   AAL_END_MOD_CMD()

   AAL_BEGIN_MOD_CMD(SWVALMOD_CMD_MALLOC_OBJ)
      swvalmod_obj *p = (swvalmod_obj *) malloc(sizeof(swvalmod_obj));

      p->intfield = 3;
      strncpy(p->msg, "Message from libswvalmod.", sizeof(p->msg));
      p->something_in_the_mod = Something_in_the_mod;

      *(swvalmod_obj **)arg = p;
   AAL_END_MOD_CMD()

   AAL_BEGIN_MOD_CMD(SWVALMOD_CMD_FREE)
      swvalmod_obj *p = (swvalmod_obj *)arg;

      if ( 4 != p->intfield ) {
         res = 1;
      }
      if ( 0 != strncmp(p->msg, "goodbye", 7) ) {
         res = 2;
      }
      if ( 0 != strncmp(p->something_in_the_mod, "In the app.", 11) ) {
         res = 3;
      }

      free(arg);
   AAL_END_MOD_CMD()

AAL_END_MOD()
