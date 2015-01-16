// Copyright (c) 2013-2015, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file OSServiceModule.c
/// @brief OS Abstraction for Load-able Services (DLL's w/ well-known entry)
/// @ingroup ServiceModule
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/11/2013     TSW      Initial Version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <stdio.h>
#if HAVE_STRING_H
# include <string.h>
#endif // HAVE_STRING_H
#if HAVE_LTDL_H
# include <ltdl.h>
#endif // HAVE_LTDL_H
#if HAVE_PTHREAD_H
# ifndef __USE_GNU
#    define __USE_GNU 1
# endif // __USE_GNU
# include <pthread.h>
#endif // HAVE_PTHREAD_H

#include "aalsdk/osal/OSServiceModule.h"


/// @addtogroup ServiceModule
/// @{

#if HAVE_LTDL_H
# if HAVE_PTHREAD_H
// See http://www.gnu.org/software/libtool/manual/html_node/Thread-Safety-in-libltdl.html
// The summary is that it is up to us to provide thread safety for ltdl calls.
static pthread_mutex_t gltdlLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

int aalsdk_ltdl_lock(void)
{
   return pthread_mutex_lock(&gltdlLock);
}
int aalsdk_ltdl_unlock(void)
{
   return pthread_mutex_unlock(&gltdlLock);
}
# endif // HAVE_PTHREAD_H
#endif // HAVE_LTDL_H

#if defined ( __AAL_WINDOWS__ )                                               
#pragma warning( push)            
#pragma warning( disable : 4996) // destination of copy is unsafe            
#endif

void OSAL_API OSServiceModuleInit(OSServiceModule *p, btcString root_name)
{
   memset(p, 0, sizeof(OSServiceModule));
   // eg root_name = "libOSAL"
   strncpy(p->root_name,
           root_name,
           AAL_SVC_MOD_ROOT_NAME_MAX);
   // eg full_name = "libOSAL.so" or "libOSAL.dll"
   strncpy(p->full_name,
           p->root_name,
           AAL_SVC_MOD_FULL_NAME_MAX);
   strncat(p->full_name,
           AAL_SVC_MOD_EXT,
           AAL_SVC_MOD_FULL_NAME_MAX - strlen(p->full_name));
   // eg entry_point_name = "libOSALAALSvcMod"
   strncpy(p->entry_point_name,
           p->root_name,
           AAL_SVC_MOD_ROOT_NAME_MAX);
   strncat(p->entry_point_name,
           AAL_SVC_MOD_ENTRY_SUFFIX,
           AAL_SVC_MOD_ENTRY_NAME_MAX - strlen(p->entry_point_name));
}
#if defined ( __AAL_WINDOWS__ )                                               
#pragma warning( pop )                                                       
#endif

bt32bitInt OSServiceModuleOpen(OSServiceModule *p)
{
   bt32bitInt res = 0;
#if defined( __AAL_WINDOWS__ )

   if ( NULL != p->entry_point_fn ) {
      // Already open or not initialized..
      res = 1;
      p->error_msg = "Already / not Init";
# if DBG_DYN_LOAD
      fprintf(stderr, "[DBG_DYN_LOAD] Error : %s (%d)\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      return res;
   }

   p->error_msg = NULL;

   p->handle = LoadLibrary(p->full_name);

   if ( NULL == p->handle ) {
      res = 2;
      p->error_msg = "LoadLibrary()";
# if DBG_DYN_LOAD
      fprintf(stderr, "[DBG_DYN_LOAD] Error : %s (%d)\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      return res;
   }

# if DBG_DYN_LOAD
   fprintf(stderr, "[DBG_DYN_LOAD] Looking up module entry : %s ", p->entry_point_name);
# endif // DBG_DYN_LOAD

   p->entry_point_fn = (AALSvcEntryPoint) GetProcAddress(p->handle, p->entry_point_name);

   if ( NULL == p->entry_point_fn ) {
      res = 3;
      p->error_msg = "GetProcAddress()";
# if DBG_DYN_LOAD
      fprintf(stderr, "[Fail] : %s (%d)\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
   }
# if DBG_DYN_LOAD
   else {
      fprintf(stderr, "[OK].\n");
   }
# endif // DBG_DYN_LOAD

#elif HAVE_LTDL_H

   lt_dladvise advise;
   bt32bitInt  tmp;
   btByte      buf[AAL_SVC_MOD_FULL_NAME_MAX + 256];

   btBool      locked  = false;
   btBool      advised = false;

   tmp = aalsdk_ltdl_lock();
   if ( 0 != tmp ) {
      res = tmp;
      p->error_msg = strerror(res);
# if DBG_DYN_LOAD
      fprintf(stderr, "[DBG_DYN_LOAD] aalsdk_ltdl_lock() failed : %s (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      // yes, continue anyway.
   } else {
      locked = true;
   }

   if ( NULL != p->entry_point_fn ) {
      // Already open or not initialized..
      res = 1;
      p->error_msg = "Already / not Init";
      goto ERROR;
   }

   p->error_msg = NULL;

   tmp = lt_dladvise_init(&advise);
   if ( 0 != tmp ) {
      res = tmp;
      p->error_msg = lt_dlerror();
      if ( NULL == p->error_msg ) {
         p->error_msg = "lt_dladvise_init()";
      }
      goto ERROR;

      advised = true;
   }

   tmp = lt_dladvise_global(&advise);
   if ( 0 != tmp ) {
      res = tmp;
      p->error_msg = lt_dlerror();
      if ( NULL == p->error_msg ) {
         p->error_msg = "lt_dladvise_global()";
      }
      goto ERROR;
   }

# if DBG_DYN_LOAD
   fprintf(stderr, "[DBG_DYN_LOAD] Trying '%s' ", p->full_name);
# endif // DBG_DYN_LOAD

   // First, try the less-restrictive unqualified path (ie, module name only).

   p->handle = lt_dlopenadvise(p->full_name, advise);

   if ( NULL == p->handle ) {
      // Now try the fully-qualified installation path for the lib.
      snprintf(buf, sizeof(buf), "%s/%s", LIBDIR, p->full_name);

# if DBG_DYN_LOAD
      fprintf(stderr, " [Fail]. Trying '%s' ", buf);
# endif // DBG_DYN_LOAD

      p->handle = lt_dlopenadvise(buf, advise);

      if ( NULL == p->handle ) {
         res = 2;
         p->error_msg = lt_dlerror();
         if ( NULL == p->error_msg ) {
            p->error_msg = "lt_dlopenadvise()";
         }
         goto ERROR;
      }
   }

# if DBG_DYN_LOAD
   fprintf(stderr, "[OK]. Looking up module entry : '%s' ", p->entry_point_name);
# endif // DBG_DYN_LOAD

   p->entry_point_fn = (AALSvcEntryPoint) lt_dlsym(p->handle, p->entry_point_name);

   if ( NULL == p->entry_point_fn ) {
      res = 3;
      p->error_msg = lt_dlerror();
      if ( NULL == p->error_msg ) {
         p->error_msg = "lt_dlsym()";
      }
      goto ERROR;
   }
# if DBG_DYN_LOAD
   else {
      fprintf(stderr, "[OK].\n");
   }
# endif // DBG_DYN_LOAD

   goto CLEANUP;

ERROR:

# if DBG_DYN_LOAD
   fprintf(stderr, " [Fail] : %s (%d)", p->error_msg, res);
# endif // DBG_DYN_LOAD

CLEANUP:

   if ( advised ) {
      tmp = lt_dladvise_destroy(&advise);
      if ( 0 != tmp ) {
         res = tmp;
         p->error_msg = lt_dlerror();
         if ( NULL == p->error_msg ) {
            p->error_msg = "lt_dladvise_destroy()";
         }
# if DBG_DYN_LOAD
         fprintf(stderr, "[DBG_DYN_LOAD] %s failed (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      }
   }

   if ( locked ) {
      tmp = aalsdk_ltdl_unlock();
      if ( 0 != tmp ) {
         res = tmp;
         p->error_msg = strerror(res);
# if DBG_DYN_LOAD
         fprintf(stderr, "[DBG_DYN_LOAD] aalsdk_ltdl_unlock() failed : %s (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      }
   }

#endif // OS
   return res;
}

bt32bitInt OSServiceModuleClose(OSServiceModule *p)
{
   bt32bitInt res = 0;
#if defined( __AAL_WINDOWS__ )

   p->error_msg = NULL;
   if ( NULL != p->handle ) {

      if ( !FreeLibrary(p->handle) ) {
         res = 1;
         p->error_msg = "FreeLibrary()";
# if DBG_DYN_LOAD
         fprintf(stderr, "[DBG_DYN_LOAD] Error : %s ", p->error_msg);
# endif // DBG_DYN_LOAD
      }

      p->handle = NULL;
   }

#elif HAVE_LTDL_H
   btBool     locked = false;
   bt32bitInt tmp;

   tmp = aalsdk_ltdl_lock();
   if ( 0 != tmp ) {
      res = tmp;
      p->error_msg = strerror(res);
# if DBG_DYN_LOAD
      fprintf(stderr, "[DBG_DYN_LOAD] aalsdk_ltdl_lock() failed : %s (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      // yes, continue anyway.
   } else {
      locked = true;
   }

   p->error_msg = NULL;

   if ( NULL != p->handle ) {

      tmp = lt_dlclose(p->handle);
      p->handle = NULL;

      if ( 0 != tmp ) {
         res = tmp;
         p->error_msg = lt_dlerror();
         if ( NULL == p->error_msg ) {
            p->error_msg = "lt_dlclose()";
         }
# if DBG_DYN_LOAD
         fprintf(stderr, "[DBG_DYN_LOAD] %s failed (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      }
   }

   if ( locked ) {
      tmp = aalsdk_ltdl_unlock();
      if ( 0 != tmp ) {
         res = tmp;
         p->error_msg = strerror(res);
# if DBG_DYN_LOAD
         fprintf(stderr, "[DBG_DYN_LOAD] aalsdk_ltdl_unlock() failed : %s (%d).\n", p->error_msg, res);
# endif // DBG_DYN_LOAD
      }
   }

#endif // OS
   return res;
}

/// @} group ServiceModule

