// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __MYPTHREADHOOKS_H__
#define __MYPTHREADHOOKS_H__

#define MAX_PTHREAD_HOOKS 3
#define PTHREAD_HOOKS_STATIC_INITIALIZER { NULL, NULL, NULL }

#ifdef PTHREAD_HOOKS_LIB
# define PTHREAD_HOOKS_ATTR
# define PTHREAD_HOOKS_EXTERN
#else
# define PTHREAD_HOOKS_ATTR   __attribute__((weak))
# define PTHREAD_HOOKS_EXTERN extern
#endif // PTHREAD_HOOKS_LIB

////////////////////////////////////////////////////////////////////////////////
// pthread_create
//
// extern int pthread_create (pthread_t *__restrict __newthread,
//                            const pthread_attr_t *__restrict __attr,
//                            void *(*__start_routine) (void *),
//                            void *__restrict __arg) __THROWNL __nonnull ((1, 3));

typedef int (*pthread_create_fn)(void *__restrict __newthread,
                                 const void *__restrict __attr,
                                 void *(*__start_routine) (void *),
                                 void *__restrict __arg);

// Hook called before the real pthread_create is called.
typedef void (*pthread_create_pre_hook)(void *__restrict __newthread,
                                        const void *__restrict __attr,
                                        void *(*__start_routine) (void *),
                                        void *__restrict __arg);

// Hook called after the real pthread_create is called.
typedef void (*pthread_create_post_hook)(int res,
                                         void *__restrict __newthread,
                                         const void *__restrict __attr,
                                         void *(*__start_routine) (void *),
                                         void *__restrict __arg);
BEGIN_C_DECLS
PTHREAD_HOOKS_EXTERN pthread_create_pre_hook  PTHREAD_HOOKS_ATTR  gpthread_create_pre_hooks[MAX_PTHREAD_HOOKS];
PTHREAD_HOOKS_EXTERN pthread_create_post_hook PTHREAD_HOOKS_ATTR gpthread_create_post_hooks[MAX_PTHREAD_HOOKS];
END_C_DECLS

////////////////////////////////////////////////////////////////////////////////
// pthread_join
//
// extern int pthread_join (pthread_t __th, void **__thread_return);

typedef int (*pthread_join_fn)(unsigned long int __th, void **__thread_return);

// Hook called before the real pthread_join is called.
typedef void (*pthread_join_pre_hook)(unsigned long int __th,
                                      void **__thread_return);

// Hook called after the real pthread_join is called.
typedef void (*pthread_join_post_hook)(int res,
                                       unsigned long int __th,
                                       void **__thread_return);

BEGIN_C_DECLS
PTHREAD_HOOKS_EXTERN pthread_join_pre_hook  PTHREAD_HOOKS_ATTR  gpthread_join_pre_hooks[MAX_PTHREAD_HOOKS];
PTHREAD_HOOKS_EXTERN pthread_join_post_hook PTHREAD_HOOKS_ATTR gpthread_join_post_hooks[MAX_PTHREAD_HOOKS];
END_C_DECLS

#define DECLARE_PTHREAD_HOOKS                                                                              \
pthread_create_pre_hook  gpthread_create_pre_hooks[MAX_PTHREAD_HOOKS]  = PTHREAD_HOOKS_STATIC_INITIALIZER; \
pthread_create_post_hook gpthread_create_post_hooks[MAX_PTHREAD_HOOKS] = PTHREAD_HOOKS_STATIC_INITIALIZER; \
pthread_join_pre_hook    gpthread_join_pre_hooks[MAX_PTHREAD_HOOKS]    = PTHREAD_HOOKS_STATIC_INITIALIZER; \
pthread_join_post_hook   gpthread_join_post_hooks[MAX_PTHREAD_HOOKS]   = PTHREAD_HOOKS_STATIC_INITIALIZER

#endif // __MYPTHREADHOOKS_H__

