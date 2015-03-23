// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <stdlib.h>

#ifndef BEGIN_C_DECLS
# define BEGIN_C_DECLS
#endif // BEGIN_C_DECLS
#ifndef END_C_DECLS
# define END_C_DECLS
#endif // END_C_DECLS

#include "hooksbridge.h"

#define INSTALL_HOOK(__arr, __fn)                      \
do                                                     \
{  unsigned __i;                                       \
   for ( __i = 0 ; __i < MAX_PTHREAD_HOOKS ; ++__i ) { \
      if ( NULL == __arr[__i] ) {                      \
         __arr[__i] = __fn;                            \
         break;                                        \
      }                                                \
   }                                                   \
}while(0)

#define REMOVE_HOOK(__arr, __fn)                       \
do                                                     \
{  unsigned __i;                                       \
   for ( __i = 0 ; __i < MAX_PTHREAD_HOOKS ; ++__i ) { \
      if ( __fn == __arr[__i] ) {                      \
         __arr[__i] = NULL;                            \
      }                                                \
   }                                                   \
}while(0)


void install_pre_pthread_create_hook(pthread_create_pre_hook  fn, bool install)
{
   if ( NULL == gpthread_create_pre_hooks ) {
      return;
   }
   if ( install ) {
      INSTALL_HOOK(gpthread_create_pre_hooks, fn);
   } else {
      REMOVE_HOOK(gpthread_create_pre_hooks, fn);
   }
}

void install_post_pthread_create_hook(pthread_create_post_hook fn, bool install)
{
   if ( NULL == gpthread_create_post_hooks ) {
      return;
   }
   if ( install ) {
      INSTALL_HOOK(gpthread_create_post_hooks, fn);
   } else {
      REMOVE_HOOK(gpthread_create_post_hooks, fn);
   }
}

void install_pre_pthread_join_hook(pthread_join_pre_hook  fn, bool install)
{
   if ( NULL == gpthread_join_pre_hooks ) {
      return;
   }
   if ( install ) {
      INSTALL_HOOK(gpthread_join_pre_hooks, fn);
   } else {
      REMOVE_HOOK(gpthread_join_pre_hooks, fn);
   }
}

void install_post_pthread_join_hook(pthread_join_post_hook fn, bool install)
{
   if ( NULL == gpthread_join_post_hooks ) {
      return;
   }
   if ( install ) {
      INSTALL_HOOK(gpthread_join_post_hooks, fn);
   } else {
      REMOVE_HOOK(gpthread_join_post_hooks, fn);
   }
}

