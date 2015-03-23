// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __HOOKSBRIDGE_H__
#define __HOOKSBRIDGE_H__
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif // HAVE_STDBOOL_H

#include "mypthreadhooks.h"

BEGIN_C_DECLS

void  install_pre_pthread_create_hook(pthread_create_pre_hook  fn, bool install);
void install_post_pthread_create_hook(pthread_create_post_hook fn, bool install);

void  install_pre_pthread_join_hook(pthread_join_pre_hook  fn, bool install);
void install_post_pthread_join_hook(pthread_join_post_hook fn, bool install);

END_C_DECLS

#endif // __HOOKSBRIDGE_H__

