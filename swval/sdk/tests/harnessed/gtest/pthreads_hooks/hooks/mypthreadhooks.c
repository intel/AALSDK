// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <stdlib.h>

//#ifdef HAVE_DLFCN_H
# define __USE_GNU 1
# include <dlfcn.h>
//#endif // HAVE_DLFCN_H
//#ifdef HAVE_ASSERT_H
# include <assert.h>
//#endif // HAVE_ASSERT_H

#ifndef BEGIN_C_DECLS
# define BEGIN_C_DECLS
#endif // BEGIN_C_DECLS
#ifndef END_C_DECLS
# define END_C_DECLS
#endif // END_C_DECLS

#define PTHREAD_HOOKS_LIB 1
#include "mypthreadhooks.h"

DECLARE_PTHREAD_HOOKS;

// The faux pthread_create
int pthread_create (void *__restrict __newthread,
                    const void *__restrict __attr,
                    void *(*__start_routine) (void *),
                    void *__restrict __arg)
{
   static pthread_create_fn real_pthread_create = NULL;

   unsigned i;
   int      res;

   // resolve the real pthread_create.
   if ( NULL == real_pthread_create ) {
      real_pthread_create = (pthread_create_fn) dlsym(RTLD_NEXT, "pthread_create");
   }

   assert(NULL != real_pthread_create);
   if ( NULL == real_pthread_create ) {
      return -1;
   }

   // call the pre hooks.
   for ( i = 0 ; i < MAX_PTHREAD_HOOKS ; ++i ) {
      if ( NULL == gpthread_create_pre_hooks[i] ) {
         continue;
      }
      gpthread_create_pre_hooks[i](__newthread, __attr, __start_routine, __arg);
   }

   // call the real pthread_create.
   res = real_pthread_create(__newthread, __attr, __start_routine, __arg);

   // call the post hooks.
   for ( i = 0 ; i < MAX_PTHREAD_HOOKS ; ++i ) {
      if ( NULL == gpthread_create_post_hooks[i] ) {
         continue;
      }
      gpthread_create_post_hooks[i](res, __newthread, __attr, __start_routine, __arg);
   }

   return res;
}

// The faux pthread_join
int pthread_join (unsigned long int __th, void **__thread_return)
{
   static pthread_join_fn real_pthread_join = NULL;

   unsigned i;
   int      res;

   // resolve the real pthread_join.
   if ( NULL == real_pthread_join ) {
      real_pthread_join = (pthread_join_fn) dlsym(RTLD_NEXT, "pthread_join");
   }

   assert(NULL != real_pthread_join);
   if ( NULL == real_pthread_join ) {
      return -1;
   }

   // call the pre hooks.
   for ( i = 0 ; i < MAX_PTHREAD_HOOKS ; ++i ) {
      if ( NULL == gpthread_join_pre_hooks[i] ) {
         continue;
      }
      gpthread_join_pre_hooks[i](__th, __thread_return);
   }

   // call the real pthread_join.
   res = real_pthread_join(__th, __thread_return);

   // call the post hooks.
   for ( i = 0 ; i < MAX_PTHREAD_HOOKS ; ++i ) {
      if ( NULL == gpthread_join_post_hooks[i] ) {
         continue;
      }
      gpthread_join_post_hooks[i](res, __th, __thread_return);
   }

   return res;
}

