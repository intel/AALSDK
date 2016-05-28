// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#if defined( __AAL_LINUX__ )

// Make sure that the given path appears in LD_LIBRARY_PATH, preventing duplication.
// Return non-zero on error.
int RequireLD_LIBRARY_PATH(const char *path)
{
   int   res  = 1;
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL == pvar ) {
      // not found, so set it.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   char *pcopyvar  = strdup(pvar);
   char *psavecopy = pcopyvar;

   if ( NULL == pcopyvar ) {
      return res;
   }

   char *pcolon;
   while ( NULL != (pcolon = strchr(pcopyvar, ':')) ) {

      *pcolon = 0;

      if ( 0 == strcmp(pcopyvar, path) ) {
         // path already found in LD_LIBRARY_PATH
         res = 0;
         goto _DONE;
      }

      pcopyvar = pcolon + 1;

   }

   if ( 0 == strcmp(pcopyvar, path) ) {
      // path already found in LD_LIBRARY_PATH
      res = 0;
      goto _DONE;
   }

   // LD_LIBRARY_PATH exists, but does not contain path.

   free(psavecopy);

   if ( 0 == strcmp(pvar, "") ) {
      // LD_LIBRARY_PATH is defined, but empty.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   psavecopy = (char *) malloc(strlen(pvar) + strlen(path) + 2);
   if ( NULL == psavecopy ) {
      return res;
   }

   sprintf(psavecopy, "%s:%s", pvar, path);

   res = setenv("LD_LIBRARY_PATH", psavecopy, 1);

_DONE:
   free(psavecopy);

   return res;
}

int UnRequireLD_LIBRARY_PATH(const char *path)
{
   int   res  = 0;
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL == pvar ) {
      // not found, so nothing can possibly be removed.
      return 1;
   }

   if ( NULL == strchr(pvar, ':') ) {
      // There is only one path present.
      if ( 0 == strcmp(pvar, path) ) {
         // The variable is set to the one path to remove. Unset LD_LIBRARY_PATH.
         return unsetenv("LD_LIBRARY_PATH");
      } else {
         // path is not found in LD_LIBRARY_PATH. Nothing to do.
         return 2;
      }
   }

   // LD_LIBRARY_PATH contains a list of paths separated by :

   char *pnewval = (char *) malloc(strlen(pvar) + 1);
   *pnewval = 0;

   char *pcopyvar  = strdup(pvar);
   char *psavecopy = pcopyvar;

   int   cnt = 0;
   char *pcolon;

   if ( NULL == pcopyvar ) {
      res = 3;
      goto _DONE;
   }

   while ( NULL != (pcolon = strchr(pcopyvar, ':')) ) {

      *pcolon = 0;

      if ( 0 != strcmp(pcopyvar, path) ) {
         // This one is not an instance of path, so copy it into the new value..

         if ( cnt > 0 ) {
            strcat(pnewval, ":");
         }

         strcat(pnewval, pcopyvar);

         ++cnt;
      }

      pcopyvar = pcolon + 1;

   }

   // Check the last instance..
   if ( 0 != strcmp(pcopyvar, path) ) {
      if ( cnt > 0 ) {
         strcat(pnewval, ":");
      }
      strcat(pnewval, pcopyvar);
   }

   if ( 0 == strlen(pnewval) ) {
      // There were multiple copies of path, but nothing else.
      res = unsetenv("LD_LIBRARY_PATH");
   } else {
      res = setenv("LD_LIBRARY_PATH", pnewval, 1);
   }

_DONE:
   if ( NULL != psavecopy ) {
      free(psavecopy);
   }
   if ( NULL != pnewval ) {
      free(pnewval);
   }
   return res;
}

// Print streamer for LD_LIBRARY_PATH.
// Ex.
//   cout << LD_LIBRARY_PATH << endl;
std::ostream & LD_LIBRARY_PATH(std::ostream &os)
{
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL != pvar ) {
      os << pvar;
   }

   return os;
}

#endif // __AAL_LINUX__

