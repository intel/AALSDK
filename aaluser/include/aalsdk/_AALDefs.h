// Copyright(c) 2005-2016, Intel Corporation
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
/// @file _AALDefs.h
/// @brief Definitions for basic OS/Compiler/C/C++ abstractions.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///      AUTHOR: Joseph Grecco, Intel Corporation.
///              Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/15/2005     JG       Initial version started
/// 03/19/2007     JG       Linux Port
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 04/22/2012     HM       Added EXPIMP_TEMPLATE for exporting templates
/// 10/24/2012     TSW      Cleanup for faplib.@endverbatim
//****************************************************************************
#ifndef __AALSDK__AALDEFS_H__
#define __AALSDK__AALDEFS_H__

// Compile decisions based on C vs C++.
#ifdef __cplusplus
# ifndef BEGIN_C_DECLS
#    define BEGIN_C_DECLS extern "C" {
# endif // BEGIN_C_DECLS
# ifndef END_C_DECLS
#    define END_C_DECLS   }
# endif // END_C_DECLS
# ifndef BEGIN_NAMESPACE
#    define BEGIN_NAMESPACE(__n) namespace __n {
# endif // BEGIN_NAMESPACE
# ifndef END_NAMESPACE
#    define END_NAMESPACE(__n)   }
# endif // END_NAMESPACE
# ifndef USING_NAMESPACE
#    define USING_NAMESPACE(__n) using namespace __n ;
# endif // USING_NAMESPACE
# ifndef NULL
#    define NULL 0
# endif // NULL
#else
# ifndef BEGIN_C_DECLS
#    define BEGIN_C_DECLS
# endif // BEGIN_C_DECLS
# ifndef END_C_DECLS
#    define END_C_DECLS
# endif // END_C_DECLS
# ifndef BEGIN_NAMESPACE
#    define BEGIN_NAMESPACE(__n)
# endif // BEGIN_NAMESPACE
# ifndef END_NAMESPACE
#    define END_NAMESPACE(__n)
# endif // END_NAMESPACE
# ifndef USING_NAMESPACE
#    define USING_NAMESPACE(__n)
# endif // USING_NAMESPACE
# ifndef NULL
#    define NULL ((void *)0)
# endif // NULL
#endif // __cplusplus


// Compile decisions based on target OS.
#ifndef __AAL_OS_DEFINED
#define __AAL_OS_DEFINED 1
# if defined( _WIN32 ) || defined( _WIN64 )
// Windows
#    ifdef __AAL_WINDOWS__
#       undef __AAL_WINDOWS__
#    endif // __AAL_WINDOWS__
#    define __AAL_WINDOWS__ 1
#    ifdef __AAL_ANDROID__
#       undef __AAL_ANDROID__
#    endif // __AAL_ANDROID__
#    ifdef __AAL_LINUX__
#       undef __AAL_LINUX__
#    endif // __AAL_LINUX__
#    ifdef __AAL_APPLE__
#       undef __AAL_APPLE__
#    endif // __AAL_APPLE__
#    ifdef __AAL_UNKNOWN_OS__
#       undef __AAL_UNKNOWN_OS__
#    endif // __AAL_UNKNOWN_OS__
#    if !defined( __cplusplus ) && !defined( inline )
#       define inline _inline
#    endif // inline
#    ifndef __iomem
#       define __iomem
#    endif // __iomem

// TODO: Android

# elif defined( __linux__ )
#    ifdef __AAL_WINDOWS__
#       undef __AAL_WINDOWS__
#    endif // __AAL_WINDOWS__
#    ifdef __AAL_ANDROID__
#       undef __AAL_ANDROID__
#    endif // __AAL_ANDROID__
#    ifdef __AAL_LINUX__
#       undef __AAL_LINUX__
#    endif // __AAL_LINUX__
#    define __AAL_LINUX__ 1
#    ifdef __AAL_APPLE__
#       undef __AAL_APPLE__
#    endif // __AAL_APPLE__
#    ifdef __AAL_UNKNOWN_OS__
#       undef __AAL_UNKNOWN_OS__
#    endif // __AAL_UNKNOWN_OS__
#    define memcpy_s(d,l1,s,l2) memcpy(d,s,l1)
# elif defined( __APPLE__ )
// Apple
#    ifdef __AAL_WINDOWS__
#       undef __AAL_WINDOWS__
#    endif // __AAL_WINDOWS__
#    ifdef __AAL_ANDROID__
#       undef __AAL_ANDROID__
#    endif // __AAL_ANDROID__
#    ifdef __AAL_LINUX__
#       undef __AAL_LINUX__
#    endif // __AAL_LINUX__
#    ifdef __AAL_APPLE__
#       undef __AAL_APPLE__
#    endif // __AAL_APPLE__
#    define __AAL_APPLE__ 1
#    ifdef __AAL_UNKNOWN_OS__
#       undef __AAL_UNKNOWN_OS__
#    endif // __AAL_UNKNOWN_OS__
# else
// Unknown
#    ifdef __AAL_WINDOWS__
#       undef __AAL_WINDOWS__
#    endif // __AAL_WINDOWS__
#    ifdef __AAL_ANDROID__
#       undef __AAL_ANDROID__
#    endif // __AAL_ANDROID__
#    ifdef __AAL_LINUX__
#       undef __AAL_LINUX__
#    endif // __AAL_LINUX__
#    ifdef __AAL_APPLE__
#       undef __AAL_APPLE__
#    endif // __AAL_APPLE__
#    ifdef __AAL_UNKNOWN_OS__
#       undef __AAL_UNKNOWN_OS__
#    endif // __AAL_UNKNOWN_OS__
#    define __AAL_UNKNOWN_OS__ 1
# endif // target OS
#endif // __AAL_OS_DEFINED


#ifndef __AAL_USER_VS_KERNEL_DEFINED
#define __AAL_USER_VS_KERNEL_DEFINED 1
// Compile decisions based on user space vs. kernel space
# if defined( __AAL_WINDOWS__ ) && defined( __KERNEL__ )
#    if !defined( __AAL_KERNEL__ ) || ( 0 == __AAL_KERNEL__ )
#       ifdef __AAL_KERNEL__
#          undef __AAL_KERNEL__
#       endif
#       define __AAL_KERNEL__ 1
#    endif // __AAL_KERNEL__
#    if defined( __AAL_USER__ )
#       undef __AAL_USER__
#    endif // __AAL_USER__

// TODO: Android

# elif defined( __AAL_LINUX__ ) && defined( __KERNEL__ )
#    if !defined( __AAL_KERNEL__ ) || ( 0 == __AAL_KERNEL__ )
#       ifdef __AAL_KERNEL__
#          undef __AAL_KERNEL__
#       endif
#       define __AAL_KERNEL__ 1
#    endif // __AAL_KERNEL__
#    if defined( __AAL_USER__ )
#       undef __AAL_USER__
#    endif // __AAL_USER__
# elif defined( __AAL_APPLE__ ) && defined( KERNEL )
#    if !defined( __AAL_KERNEL__ ) || ( 0 == __AAL_KERNEL__ )
#       ifdef __AAL_KERNEL__
#          undef __AAL_KERNEL__
#       endif
#       define __AAL_KERNEL__ 1
#    endif // __AAL_KERNEL__
#    if defined( __AAL_USER__ )
#       undef __AAL_USER__
#    endif // __AAL_USER__
# else
// Assume user (non-kernel) mode
#    if !defined( __AAL_USER__ ) || ( 0 == __AAL_USER__ )
#       ifdef __AAL_USER__
#          undef __AAL_USER__
#       endif
#       define __AAL_USER__ 1
#    endif // __AAL_USER__
#    if defined( __AAL_KERNEL__ )
#       undef __AAL_KERNEL__
#    endif // __AAL_KERNEL__
# endif // user vs. kernel
#endif // __AAL_USER_VS_KERNEL_DEFINED


// dll magic for Windows dll's
#if defined( __AAL_USER__ ) && defined( __AAL_WINDOWS__ )
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XYZ_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// OSAL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
# ifdef OSAL_EXPORTS
#    define OSAL_API                      __declspec(dllexport)
# else
#    define OSAL_API                      __declspec(dllimport)
# endif // OSAL_EXPORTS
# ifdef AALRUNTIME_EXPORTS
#    define AALRUNTIME_API                __declspec(dllexport)
# else
#    define AALRUNTIME_API                __declspec(dllimport)
# endif // AALRUNTIME_EXPORTS
# ifdef AASLIB_EXPORTS
#    define AASLIB_API                    __declspec(dllexport)
# else
#    define AASLIB_API                    __declspec(dllimport)
# endif // AASLIB_EXPORTS
# ifdef AIASERVICE_EXPORTS
#    define AIASERVICE_API                __declspec(dllexport)
# else
#    define AIASERVICE_API                __declspec(dllimport)
# endif // AIASERVICE_EXPORTS
# ifdef UAIA_EXPORTS
#    define UAIA_API                      __declspec(dllexport)
# else
#    define UAIA_API                      __declspec(dllimport)
# endif // UAIA_EXPORTS
# ifdef AASEDS_EXPORTS
#    define AASEDS_API                    __declspec(dllexport)
# else
#    define AASEDS_API                    __declspec(dllimport)
# endif // AASEDS_EXPORTS
# ifdef AALRESMGR_EXPORTS
#     define AALRESOURCEMANAGERCLIENT_API __declspec(dllexport)
#     define AALRESOURCEMANAGER_API       __declspec(dllexport)
#     define RESMGR_SERVICE_API           __declspec(dllexport)
# else
#     define AALRESOURCEMANAGERCLIENT_API __declspec(dllimport)
#     define AALRESOURCEMANAGER_API       __declspec(dllimport)
#     define RESMGR_SERVICE_API           __declspec(dllimport)
# endif // Resource Manager
# ifdef RRM_EXPORTS
#    define RRM_API                       __declspec(dllexport)
# else
#    define RRM_API                       __declspec(dllimport)
# endif // RRM_EXPORTS
# ifdef RRMBROKER_EXPORTS
#    define RRMBROKER_API                 __declspec(dllexport)
# else
#    define RRMBROKER_API                 __declspec(dllimport)
# endif // RRMBROKER_EXPORTS
#else
# define __declspec(x)
// OSAL
# define OSAL_API                         __declspec(0)
// AALRUNTIME
# define AALRUNTIME_API                   __declspec(0)
// AASLib
# define AASLIB_API                       __declspec(0)
// AIAService
# define AIASERVICE_API                   __declspec(0)
// uAIA
# define UAIA_API                         __declspec(0)
// AASEDS
# define AASEDS_API                       __declspec(0)
// Resource Manager Client
# define AALRESOURCEMANAGERCLIENT_API     __declspec(0)
// Resource Manager
# define AALRESOURCEMANAGER_API           __declspec(0)
# define RESMGR_SERVICE_API               __declspec(0)
# define RRM_API                          __declspec(0)
# define RRMBROKER_API                    __declspec(0)
#endif // OS


// Full path to current source file.
#ifndef __AAL_FULL_FILE__
# define __AAL_FULL_FILE__     __FILE__
#endif // __AAL_FULL_FILE__


// Compile decisions based on toolchain.
#if defined( __INTEL_COMPILER )
// Intel C/C++
# ifndef __AAL_FUNC__
#    define __AAL_FUNC__       __func__
# endif // __AAL_FUNC__
# ifndef __AAL_FUNCSIG__
#    define __AAL_FUNCSIG__    __func__
# endif // __AAL_FUNCSIG__
# ifndef __AAL_SHORT_FILE__
#    define __AAL_SHORT_FILE__ __BASE_FILE__
# endif // __AAL_SHORT_FILE__
#elif defined ( _MSC_VER )
// MS C/C++
# ifndef __AAL_FUNC__
#    define __AAL_FUNC__       __FUNCTION__
# endif // __AAL_FUNC__
# ifndef __AAL_FUNCSIG__
#    define __AAL_FUNCSIG__    __FUNCSIG__
# endif // __AAL_FUNCSIG__
# ifndef __AAL_SHORT_FILE__
static inline
const char * AALWinShortFile(const char *f) {
   const char *p = f;
   while ( *p ) { ++p; }
   while ( (p > f)  &&
           ('/'  != *p) &&
           ('\\' != *p) ) { --p; }
   if ( p > f ) { ++p; }
   return p;
}
#    define __AAL_SHORT_FILE__ AALWinShortFile( __FILE__ )
# endif // __AAL_SHORT_FILE__
#elif defined( __GNUC__ ) && !defined( __cplusplus )
// gcc
// __func__ is part of the C99 standard.
# ifndef __AAL_FUNC__
#    define __AAL_FUNC__       __func__
# endif // __AAL_FUNC__
// For C, __PRETTY_FUNCTION__ is an alias for __func__.
# ifndef __AAL_FUNCSIG__
#    define __AAL_FUNCSIG__    __PRETTY_FUNCTION__
# endif // __AAL_FUNCSIG__
# ifndef __AAL_SHORT_FILE__
#define __AAL_SHORT_FILE__         \
({ const char *file = __FILE__;    \
   const char *p    = file;        \
   while ( *p ) { ++p; }           \
   while ( (p > file)  &&          \
           ('/'  != *p) &&         \
           ('\\' != *p) ) { --p; } \
   if ( p > file ) { ++p; }        \
   p;                              \
})
# endif // __AAL_SHORT_FILE__
#elif defined ( __GNUG__ )
// g++
# ifndef __AAL_FUNC__
#    define __AAL_FUNC__       __func__
# endif // __AAL_FUNC__
// For C++, __PRETTY_FUNCTION__ expands to the function signature.
# ifndef __AAL_FUNCSIG__
#    define __AAL_FUNCSIG__    __PRETTY_FUNCTION__
# endif // __AAL_FUNCSIG__
# ifndef __AAL_SHORT_FILE__
#define __AAL_SHORT_FILE__         \
({ const char *file = __FILE__;    \
   const char *p    = file;        \
   while ( *p ) { ++p; }           \
   while ( (p > file)  &&          \
           ('/'  != *p) &&         \
           ('\\' != *p) ) { --p; } \
   if ( p > file ) { ++p; }        \
   p;                              \
})
# endif // __AAL_SHORT_FILE__
#endif // toolchain


#if defined( __AAL_WINDOWS__ )
# if defined( __AAL_KERNEL__ )
#    include <ntddk.h>
#    ifndef __cplusplus
#       ifndef true
#          define true TRUE
#       endif // true
#       ifndef false
#          define false FALSE
#       endif // false
#    endif // __cplusplus
#    include <ntdef.h> // Unicode (defines __TEXT)
#    ifdef _T
#       undef _T
#    endif // _T
#    define _T(__str_literal) __TEXT(__str_literal)
# else
#    ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN 1
#    endif // WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <winsock2.h>
#    ifndef __cplusplus
#       ifndef true
#          define true TRUE
#       endif // true
#       ifndef false
#          define false FALSE
#       endif // false
#    endif // __cplusplus
#    include <tchar.h> // Unicode (defines _T)
# endif // user vs. kernel
#elif defined( __AAL_LINUX__ )
# if defined( __AAL_KERNEL__ )
#    include <linux/version.h>
#    include <linux/kernel.h>
#    include <linux/stddef.h> // true, false
#    include <linux/module.h>
#    include <linux/moduleparam.h>
#    include <linux/init.h>
#    include <linux/types.h>
#    include <linux/mm.h>
#    include <linux/highmem.h>
#    include <linux/fs.h>
#    include <linux/delay.h>
#    include <linux/wait.h>
#    include <linux/sched.h>
#    include <linux/poll.h>
#    include <linux/pci.h>
#    include <linux/aer.h>
# else
#    ifndef __cplusplus
#       ifndef true
#          define true 1
#       endif // true
#       ifndef false
#          define false 0
#       endif // false
#    else
#       include <memory.h>
#    endif // __cplusplus
#    include <sys/time.h> // gettimeofday, struct timeval, struct timespec.
#    include <unistd.h>   // close
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <limits.h>
#    include <sys/ioctl.h>
#    include <sys/socket.h>
#    include <netinet/in.h>
#    include <netdb.h>
#    include <poll.h>
#    include <sys/mman.h>
#    include <pthread.h>
# endif // user vs. kernel
# ifdef _T
#    undef _T
# endif // _T
# ifdef _UNICODE
#    define _T(__str_literal) L##__str_literal
# else
#    define _T(__str_literal) __str_literal
# endif // _UNICODE
#endif // OS


#if !defined(ENABLE_ASSERT) || (0 == ENABLE_ASSERT)
# ifndef ENABLE_ASSERT
#    define ENABLE_ASSERT 0
# endif // ENABLE_ASSERT
#else
# undef ENABLE_ASSERT
# define ENABLE_ASSERT 1
#endif // ENABLE_ASSERT


#ifndef __CASSERT_DEFINED
#define __CASSERT_DEFINED 1
# if (0 == ENABLE_ASSERT)
#    define CASSERT(__expr) extern char __CASSERT_DISABLED__[1]
# else
     BEGIN_C_DECLS
#    define CASSERT(__expr) extern char __CASSERT_FAILED__[!(__expr) ? -1 : 1]
     END_C_DECLS
# endif // ENABLE_ASSERT
#endif // __CASSERT_DEFINED


#if defined( __AAL_USER__ )
# define __user
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# ifdef __cplusplus
#    include <algorithm>
#    include <ctime>
#    include <fstream>
#    include <iostream>
#    include <iomanip>
#    include <list>
#    include <map>
#    include <queue>
#    include <sstream>
#    include <string>
#    include <vector>
# else
#    include <stdio.h>
# endif // __cplusplus
# ifndef __AAL_USER_ASSERT_DEFINED
# define __AAL_USER_ASSERT_DEFINED 1
#    ifdef ASSERT_HERE
#       undef ASSERT_HERE
#    endif // ASSERT_HERE
#    ifdef ASSERT
#       undef ASSERT
#    endif // ASSERT
#    if (0 == ENABLE_ASSERT)
#       define ASSERT_HERE(__file, __line, __fn, __expr) do{}while(0)
#       define ASSERT(__expr) do{}while(0)
#       ifdef __cplusplus
#          define __ASSERT_HERE_PROTO_VOID
#       else
#          define __ASSERT_HERE_PROTO_VOID void
#       endif // __cplusplus
#       define __ASSERT_HERE_PROTO
#       define __ASSERT_HERE_ARGS_VOID
#       define __ASSERT_HERE_ARGS
#       define __ASSERT_HERE_IN_FN(__expr) do{}while(0)
#    else
static inline
void
AALUserAssertFail(const char *__file, int __line, const char *__fn, const char *__expr)
{
#ifdef __cplusplus
   std::cerr << "AAL ASSERT( " << __expr << " ) FAILED " <<
                __fn << "():" << __file << ":" << __line << std::endl;
#else
   fprintf(stderr, "AAL ASSERT( %s ) FAILED %s():%s:%d\n",
           __expr, __fn, __file, __line);
#endif // __cplusplus
}

#       define ASSERT_HERE(__file, __line, __fn, __expr) \
do                                                       \
{                                                        \
   if ( !(__expr) ) {                                    \
      AALUserAssertFail(__file, __line, __fn, #__expr);  \
   }                                                     \
}while(0)

#       define ASSERT(__expr)                                                 \
do                                                                            \
{                                                                             \
   if ( !(__expr) ) {                                                         \
      AALUserAssertFail(__AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__, #__expr); \
   }                                                                          \
}while(0)

#       define __ASSERT_HERE_PROTO_VOID    const char *__file, int __line, const char *__fn
#       define __ASSERT_HERE_PROTO         const char *__file, int __line, const char *__fn,
#       define __ASSERT_HERE_ARGS_VOID     __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__
#       define __ASSERT_HERE_ARGS          __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__,
#       define __ASSERT_HERE_IN_FN(__expr) ASSERT_HERE(__file, __line, __fn, __expr)
#    endif // ENABLE_ASSERT
# endif // __AAL_USER_ASSERT_DEFINED
#endif // __AAL_USER__


#if defined( __AAL_KERNEL__ )
# ifndef __AAL_KERNEL_ASSERT_DEFINED
# define __AAL_KERNEL_ASSERT_DEFINED 1
#    ifdef ASSERT_HERE
#       undef ASSERT_HERE
#    endif // ASSERT_HERE
#    ifdef ASSERT
#       undef ASSERT
#    endif // ASSERT
#    if (0 == ENABLE_ASSERT)
#       define ASSERT_HERE(__file, __line, __fn, __expr) do{}while(0)
#       define ASSERT(__expr) do{}while(0)
#       ifdef __cplusplus
#          define __ASSERT_HERE_PROTO_VOID
#       else
#          define __ASSERT_HERE_PROTO_VOID void
#       endif // __cplusplus
#       define __ASSERT_HERE_PROTO
#       define __ASSERT_HERE_ARGS_VOID
#       define __ASSERT_HERE_ARGS
#       define __ASSERT_HERE_IN_FN(__expr) do{}while(0)
#    elif defined( __AAL_WINDOWS__ )
#       define ASSERT_HERE(__file, __line, __fn, __expr)                 \
do                                                                       \
{                                                                        \
   if ( !(__expr) ) {                                                    \
      DbgPrint("%s(%d)%s(): Soft assertion failed\n   Expression: %s\n", \
                  __file, __line, __fn, #__expr);                        \
   }                                                                     \
}while(0)
#       define ASSERT(__expr) RTL_SOFT_ASSERT(__expr)
#    elif defined( __AAL_LINUX__ )
static inline
void
AALKernelAssertFail(const char *__file, int __line, const char *__fn, const char *__expr)
{
   printk(KERN_EMERG "A:[%d] AAL ASSERT( %s ) FAILED %s():%s:%d\n",
          current->tgid, __expr, __fn, __file, __line);
}

#       define ASSERT_HERE(__file, __line, __fn, __expr)  \
do                                                        \
{                                                         \
   if ( !(__expr) ) {                                     \
      AALKernelAssertFail(__file, __line, __fn, #__expr); \
   }                                                      \
}while(0)

#       define ASSERT(__expr)                                                   \
do                                                                              \
{                                                                               \
   if ( !(__expr) ) {                                                           \
      AALKernelAssertFail(__AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__, #__expr); \
   }                                                                            \
}while(0)

#    endif // ENABLE_ASSERT && __AAL_LINUX__
#    if (1 == ENABLE_ASSERT)
#       define __ASSERT_HERE_PROTO_VOID    const char *__file, int __line, const char *__fn
#       define __ASSERT_HERE_PROTO         const char *__file, int __line, const char *__fn,
#       define __ASSERT_HERE_ARGS_VOID     __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__
#       define __ASSERT_HERE_ARGS          __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__,
#       define __ASSERT_HERE_IN_FN(__expr) ASSERT_HERE(__file, __line, __fn, __expr)
#    endif // ENABLE_ASSERT
# endif // __AAL_KERNEL_ASSERT_DEFINED
#endif // __AAL_KERNEL__


// We let DEBUG, _DEBUG, and NDEBUG override ENABLE_DEBUG here, in order to satisfy
//  external projects that rely on these.
#if defined(DEBUG) || defined(_DEBUG)
# if defined(NDEBUG)
#    error Both (DEBUG or _DEBUG) and NDEBUG are defined.
# endif // NDEBUG
# if defined(ENABLE_DEBUG)
// override ENABLE_DEBUG
#    undef  ENABLE_DEBUG
#    define ENABLE_DEBUG 1
# else
#    define ENABLE_DEBUG 1
# endif // ENABLE_DEBUG
#elif defined(NDEBUG)
# if defined(ENABLE_DEBUG)
// override ENABLE_DEBUG
#    undef  ENABLE_DEBUG
#    define ENABLE_DEBUG 0
# else
#    define ENABLE_DEBUG 0
# endif // ENABLE_DEBUG
#else
// None of DEBUG, _DEBUG, NDEBUG are defined - allow ENABLE_DEBUG to control their definition.
# if defined(ENABLE_DEBUG) && (0 == ENABLE_DEBUG)
#    define NDEBUG 1
# elif defined(ENABLE_DEBUG)
#    define DEBUG 1
#    define _DEBUG 1
#    undef  ENABLE_DEBUG
#    define ENABLE_DEBUG 1
# else
#    define NDEBUG 1
#    define ENABLE_DEBUG 0
# endif // ENABLE_DEBUG
#endif // DEBUG || _DEBUG


#define ENABLE_CANARIES (0 && (1 == ENABLE_DEBUG))
# if (1 == ENABLE_CANARIES)
// struct X
// {
// #if (1 == ENABLE_CANARIES)
// #define _struct_X_canary_size              11
// #define _struct_X_start_canary_const       "s:struct X"
// #define _struct_X_define_start_canary      char _struct_X_start_canary[_struct_X_canary_size]
// #define _struct_X_end_canary_const         "e:struct X"
// #define _struct_X_define_end_canary        char _struct_X_end_canary[_struct_X_canary_size]
// #define _struct_X_start_canary_initializer { 's', ':', 's', 't', 'r', 'u', 'c', 't', ' ', 'X', 0 },
// #define _struct_X_end_canary_initializer   { 'e', ':', 's', 't', 'r', 'u', 'c', 't', ' ', 'X', 0 }
//    _struct_X_define_start_canary;
// #else
// #define _struct_X_start_canary_initializer
// #define _struct_X_end_canary_initializer
// #endif // ENABLE_CANARIES
//
// (normal contents of struct X here)
//
// #if (1 == ENABLE_CANARIES)
//    _struct_X_define_end_canary;
// #endif // ENABLE_CANARIES
// } x = { _struct_X_start_canary_initializer (normal initializers here), _struct_X_end_canary_initializer };
//

// strncpy((ptr)->_##type##_start_canary, _##type##_start_canary_const, _##type##_canary_size);
// ex)
//   canary_start_init(struct_X, &x);
#define canary_start_init(type, ptr)                       \
do                                                         \
{  const char *_canary = _##type##_start_canary_const;     \
   char       *_p      = (ptr)->_##type##_start_canary;    \
   char       *_end    = (_p + _##type##_canary_size) - 1; \
   while ( *_canary && (_p < _end) ) {                     \
      *_p = *_canary; ++_p; ++_canary;                     \
   }                                                       \
   *_p = 0;                                                \
}while(0)

// strncpy((ptr)->_##type##_end_canary, _##type##_end_canary_const, _##type##_canary_size);
// ex)
//   canary_end_init(struct_X, &x);
#define canary_end_init(type, ptr)                         \
do                                                         \
{  const char *_canary = _##type##_end_canary_const;       \
   char       *_p      = (ptr)->_##type##_end_canary;      \
   char       *_end    = (_p + _##type##_canary_size) - 1; \
   while ( *_canary && (_p < _end) ) {                     \
      *_p = *_canary; ++_p; ++_canary;                     \
   }                                                       \
   *_p = 0;                                                \
}while(0)

// ex)
//   canaries_init(struct_X, &x);
#define canaries_init(type, ptr) \
do                               \
{                                \
   canary_start_init(type, ptr); \
   canary_end_init(type, ptr);   \
}while(0)


// memset((ptr)->_##type##_start_canary, 0, _##type##_canary_size);
// ex)
//   canary_start_clear(struct_X, &x);
#define canary_start_clear(type, ptr)          \
do                                             \
{  char *_p   = (ptr)->_##type##_start_canary; \
   char *_end = _p + _##type##_canary_size;    \
   while ( _p < _end ) {                       \
      *_p = 0; ++_p;                           \
   }                                           \
}while(0)

// memset((ptr)->_##type##_end_canary, 0, _##type##_canary_size);
// ex)
//   canary_end_clear(struct_X, &x);
#define canary_end_clear(type, ptr)          \
do                                           \
{  char *_p   = (ptr)->_##type##_end_canary; \
   char *_end = _p + _##type##_canary_size;  \
   while ( _p < _end ) {                     \
      *_p = 0; ++_p;                         \
   }                                         \
}while(0)

// ex)
//   canaries_clear(struct_X, &x);
#define canaries_clear(type, ptr) \
do                                \
{                                 \
   canary_start_clear(type, ptr); \
   canary_end_clear(type, ptr);   \
}while(0)


// bool_res = (0 == strncmp(_##type##_start_canary_const, (ptr)->_##type##_start_canary, _##type##_canary_size));
// ex)
//   btBool valid;
//   canary_start_is_valid(struct_X, &x, valid);
//   if ( valid ) ...
#define canary_start_is_valid(type, ptr, bool_res)      \
do                                                      \
{  const char *_canary = _##type##_start_canary_const;  \
   char       *_p      = (ptr)->_##type##_start_canary; \
   char       *_end    = _p + _##type##_canary_size;    \
   unsigned char _valid = 1;                            \
   while ( _p < _end ) { /* verify 0-terminator, too */ \
      if ( *_p != *_canary ) {                          \
         _valid = 0;                                    \
         break;                                         \
      }                                                 \
      ++_p; ++_canary;                                  \
   }                                                    \
   bool_res = _valid;                                   \
}while(0)

// bool_res = (0 == strncmp(_##type##_end_canary_const, (ptr)->_##type##_end_canary, _##type##_canary_size));
//   btBool valid;
//   canary_end_is_valid(struct_X, &x, valid);
//   if ( valid ) ...
#define canary_end_is_valid(type, ptr, bool_res)        \
do                                                      \
{  const char *_canary = _##type##_end_canary_const;    \
   char       *_p      = (ptr)->_##type##_end_canary;   \
   char       *_end    = _p + _##type##_canary_size;    \
   unsigned char _valid = 1;                            \
   while ( _p < _end ) { /* verify 0-terminator, too */ \
      if ( *_p != *_canary ) {                          \
         _valid = 0;                                    \
         break;                                         \
      }                                                 \
      ++_p; ++_canary;                                  \
   }                                                    \
   bool_res = _valid;                                   \
}while(0)

//   btBool valid;
//   canaries_are_valid(struct_X, &x, valid);
//   if ( valid ) ...
#define canaries_are_valid(type, ptr, bool_res)    \
do                                                 \
{  unsigned char _start_valid;                     \
   unsigned char _end_valid;                       \
   canary_start_is_valid(type, ptr, _start_valid); \
   canary_end_is_valid(type, ptr, _end_valid);     \
   bool_res = (_start_valid && _end_valid);        \
}while(0)

#else

# define canary_start_init(type, ptr)
# define canary_end_init(type, ptr)
# define canaries_init(type, ptr)

# define canary_start_clear(type, ptr)
# define canary_end_clear(type, ptr)
# define canaries_clear(type, ptr)

# define canary_start_is_valid(type, ptr, bool_res)
# define canary_end_is_valid(type, ptr, bool_res)
# define canaries_are_valid(type, ptr, bool_res)
#endif // ENABLE_CANARIES


// Must always evaluate to 0.
#ifdef DEPRECATED
# undef DEPRECATED
#endif // DEPRECATED
#define DEPRECATED 0


// Set 1+ flags
#ifndef flag_setf
# define flag_setf(flags, f)     ( (flags) |= (f) )
#endif // flag_setf
// Clear 1+ flags
#ifndef flag_clrf
# define flag_clrf(flags, f)     ( (flags) &= ~(f) )
#endif // flag_clrf
// 1+ flags from f are set?
#ifndef flag_is_set
# define flag_is_set(flags, f)   ( (flags) & (f) )
#endif // flag_is_set
// 1+ flags from f are clear?
#ifndef flag_is_clr
# define flag_is_clr(flags, f)   ( ((flags) & (f)) != (f) )
#endif // flag_is_clr
// All flags from f are set?
#ifndef flags_are_set
# define flags_are_set(flags, f) ( ((flags) & (f)) == (f) )
#endif // flags_are_set
// All flags from f are clear?
#ifndef flags_are_clr
# define flags_are_clr(flags, f) ( ((flags) & (f)) == 0 )
#endif // flags_are_clr


#if defined( __AAL_USER__ )
# ifndef __NO_PRAGMA_BUILD_MSG__
#    define BUILD_MSG(s) __FILE__ " COMMENT: " #s
# else
#    define BUILD_MSG(s)
#    define message
# endif // __NO_PRAGMA_BUILD_MSG__
#endif // __AAL_USER__

#endif // __AALSDK__AALDEFS_H__

