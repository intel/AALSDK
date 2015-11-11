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
/// @file OSServiceModule.h
/// @brief Implements Abstractions for Dynamically-loaded Modules and Service Modules
/// @ingroup ServiceModule
///
/// Implements macros and definitions to simplify implementing AAL-compatible
/// library Modules and Service Modules. All modules in AAL are version stamped
/// and contain a well-defined default interface that, among other things,
/// allows software to query the version of the Module.
///
/// Modules may be loaded via the DynLinkLibrary class or by using the low-level
/// Module API described in this file.
///
/// Service Modules are a subset of Modules that provide AAL Services.
/// Service Modules use the SVC_ version of the macros.
///
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/11/2013     TSW      Initial version@endverbatim
//****************************************************************************
#ifndef __OSAL_OSSERVICEMODULE_H__
#define __OSAL_OSSERVICEMODULE_H__
#include <aalsdk/AALTypes.h>
#ifdef __cplusplus
# include <aalsdk/AALBase.h>
# include <aalsdk/osal/CriticalSection.h>
# include <aalsdk/aas/AALServiceModule.h>
#endif // __cplusplus

#if defined( __AAL_LINUX__ )
# include <ltdl.h>
#endif // __AAL_LINUX__

/// @addtogroup ServiceModule
/// @{

// This file may be included by C or C++ applications..
#ifdef __cplusplus
# define BT32I  AAL::bt32bitInt
# define BT32U  AAL::btUnsigned32bitInt
# define BTANY  AAL::btAny
# define BTSTR  AAL::btString
# define BTCSTR AAL::btcString
# define BTBYTE AAL::btByte
#else
// C has no comprehension of C++ namespace's.
# define BT32I  bt32bitInt
# define BT32U  btUnsigned32bitInt
# define BTANY  btAny
# define BTSTR  btString
# define BTCSTR btcString
# define BTBYTE btByte
#endif // __cplusplus

BEGIN_C_DECLS

/// Signature of the Service Module Entry Point.
typedef BT32I (*AALSvcEntryPoint)(BT32U , BTANY );


#ifndef _AAL_SVC_INTERNAL_CMD_BASE
/// AALSDK-specific Service commands start here.
# define _AAL_SVC_INTERNAL_CMD_BASE    0
#endif // _AAL_SVC_INTERNAL_CMD_BASE

#ifndef AAL_SVC_USER_CMD_BASE
/// User-defined Service commands start here.
# define AAL_SVC_USER_CMD_BASE         1000
#endif // AAL_SVC_USER_CMD_BASE

#ifndef AAL_SVC_CMD_VER_STR
/// Retrieve the Service Module version as a char *. Common to all AALSDK Modules.
# define AAL_SVC_CMD_VER_STR           (_AAL_SVC_INTERNAL_CMD_BASE + 0)
#endif // AAL_SVC_CMD_VER_STR
#ifndef AAL_SVC_CMD_VER_CURRENT
/// Retrieve the "Current" field of the Service Module version (libtool scheme) as a u32. Common to all AALSDK Modules.
# define AAL_SVC_CMD_VER_CURRENT       (_AAL_SVC_INTERNAL_CMD_BASE + 1)
#endif // AAL_SVC_CMD_VER_CURRENT
#ifndef AAL_SVC_CMD_VER_REVISION
/// Retrieve the "Revision" field of the Service Module version (libtool scheme) as a u32. Common to all AALSDK Modules.
# define AAL_SVC_CMD_VER_REVISION      (_AAL_SVC_INTERNAL_CMD_BASE + 2)
#endif // AAL_SVC_CMD_VER_REVISION
#ifndef AAL_SVC_CMD_VER_AGE
/// Retrieve the "Age" field of the Service Module version (libtool scheme) as a u32. Common to all AALSDK Modules.
# define AAL_SVC_CMD_VER_AGE           (_AAL_SVC_INTERNAL_CMD_BASE + 3)
#endif // AAL_SVC_CMD_VER_AGE
#ifndef AAL_SVC_CMD_GET_PROVIDER
/// Allocate the Service Provider (Factory) for the Services provided by the Service Module. Common to all AALSDK Service Modules.
# define AAL_SVC_CMD_GET_PROVIDER      (_AAL_SVC_INTERNAL_CMD_BASE + 4)
#endif // AAL_SVC_CMD_GET_FACTORY
#ifndef AAL_SVC_CMD_FREE_PROVIDER
/// Free the Service Provider (Factory) for the Services provided by the Service Module. Common to all AALSDK Service Modules.
# define AAL_SVC_CMD_FREE_PROVIDER     (_AAL_SVC_INTERNAL_CMD_BASE + 5)
#endif // AAL_SVC_CMD_FREE_FACTORY

#if   defined( __AAL_WINDOWS__ )
# define AAL_SVC_MOD_EXT               ".dll"
# define AAL_SVC_MOD_ENTRY_SUFFIX      "AALSvcMod"
# define AAL_SVC_MOD_ENTRY_POINT(__x)  __x##AALSvcMod
# define aalsdk_ltdl_lock()            0
# define aalsdk_ltdl_unlock()          0
#elif defined( __AAL_LINUX__ )
# define AAL_SVC_MOD_EXT               ".so"
# define AAL_SVC_MOD_ENTRY_SUFFIX      "AALSvcMod"
# define AAL_SVC_MOD_ENTRY_POINT(__x)  __x##_LTX_##__x##AALSvcMod

// Because ltdl provides no thread safety guarantees.
int aalsdk_ltdl_lock(void);
int aalsdk_ltdl_unlock(void);
#endif // OS

#ifdef __cplusplus

/// @brief Declare that a library implements an AAL Module.
///
/// Declares the appropriate C or C++ function signature for the Module entry point, accounting
/// for the current OS. Typically appears in an outward-facing header file.
/// 
/// @param[in]  __rtnamesym   the module root name (libtool) of the loadable module, typically,
///                            the name of the shared library binary without any file extension.
/// @param[in]  __storagecls  the Windows storage class for the dll.
///
/// Ex.@code
/// #if defined(__AAL_USER__) && defined(__AAL_WINDOWS__)
/// # ifdef OSAL_EXPORTS
/// #    define OSAL_API __declspec(dllexport)
/// # else
/// #    define OSAL_API __declspec(dllimport)
/// # endif // OSAL_EXPORTS
/// #else
/// # define __declspec(x)
/// # define OSAL_API __declspec(0)
/// #endif // Windows User Mode
///
/// AAL_DECLARE_MOD(libOSAL, OSAL_API)@endcode
# define AAL_DECLARE_MOD(__rtnamesym, __storagecls)                                                              \
extern "C" {                                                                                                     \
extern __storagecls AAL::bt32bitInt AAL_SVC_MOD_ENTRY_POINT(__rtnamesym)(AAL::btUnsigned32bitInt , AAL::btAny ); \
}

#else

# define AAL_DECLARE_MOD(__rtnamesym, __storagecls)                                             \
extern __storagecls bt32bitInt AAL_SVC_MOD_ENTRY_POINT(__rtnamesym)(btUnsigned32bitInt, btAny );

#endif // __cplusplus

/// @brief Declare that a library implements an AAL Service Module.
///
/// Declares the appropriate C or C++ function signature for the Service Module entry point, accounting
/// for the current OS. Typically appears in an outward-facing header file.
/// 
/// @param[in]  __rtnamesym   the module root name (libtool) of the loadable module, typically,
///                            the name of the shared library binary without any file extension.
/// @param[in]  __storagecls  the Windows storage class for the dll.
///
/// Ex.@code
/// #if defined(__AAL_USER__) && defined (__AAL_WINDOWS__)
/// # ifdef SPLAFU_EXPORTS
/// #    define SPLAFU_API __declspec(dllexport)
/// # else
/// #    define SPLAFU_API __declspec(dllimport)
/// # endif // SPLAFU_EXPORTS
/// #else
/// # define __declspec(x)
/// # define SPLAFU_API __declspec(0)
/// #endif // Windows User Mode
///
/// AAL_DECLARE_SVC_MOD(libSPLAFU, SPLAFU_API)@endcode
#define AAL_DECLARE_SVC_MOD(__rtnamesym, __storagecls) AAL_DECLARE_MOD(__rtnamesym, __storagecls)


/// Max length of string copied for AAL_SVC_CMD_VER_STR.
#define AAL_SVC_MOD_VER_STR_MAX    256
/// Max length of module root name (no file extension).
#define AAL_SVC_MOD_ROOT_NAME_MAX  256
/// Max length of module file name.
#define AAL_SVC_MOD_FULL_NAME_MAX  256
/// Max length of module entry point name.
#define AAL_SVC_MOD_ENTRY_NAME_MAX 256

/// @brief Default interface (command list) supported by all AALSDK Modules.
/// @note Used internally by the AAL_BEGIN_xxx_MOD macros. Not to be used directly.
///
/// @param[in]  __arg     the Module entry point's command argument.
/// @param[in]  __verstr  the libtool version of the module in C:R:A form. (char *)
/// @param[in]  __vercur  the Current component of the C:R:A. (u32)
/// @param[in]  __verrev  the Revision component of the C:R:A. (u32)
/// @param[in]  __verage  the Age component of the C:R:A. (u32)
#define AAL_MOD_DEFAULT_IMPL(__arg, __verstr, __vercur, __verrev, __verage) \
case AAL_SVC_CMD_VER_STR : {                                                \
   strncpy((BTSTR)(__arg), __verstr, AAL_SVC_MOD_VER_STR_MAX);              \
} return 0;                                                                 \
case AAL_SVC_CMD_VER_CURRENT : {                                            \
   *(BT32U *)(__arg) = __vercur;                                            \
} return 0;                                                                 \
case AAL_SVC_CMD_VER_REVISION : {                                           \
   *(BT32U *)(__arg) = __verrev;                                            \
} return 0;                                                                 \
case AAL_SVC_CMD_VER_AGE : {                                                \
   *(BT32U *)(__arg) = __verage;                                            \
} return 0


/// @brief Default interface (command list) supported by all AALSDK Service Modules.
/// @note Used internally by the AAL_BEGIN_xxx_MOD macros. Not to be used directly.
///
/// @param[in]  __svcfactory  the C++ type of the Service Module's Factory class.
/// @param[in]  __arg         the Module entry point's command argument.
/// @param[in]  __verstr      the libtool version of the module in C:R:A form. (char *)
/// @param[in]  __vercur      the Current component of the C:R:A. (u32)
/// @param[in]  __verrev      the Revision component of the C:R:A. (u32)
/// @param[in]  __verage      the Age component of the C:R:A. (u32)
#define AAL_SVC_MOD_DEFAULT_IMPL(__svcfactory, __arg, __verstr, __vercur, __verrev, __verage) \
case AAL_SVC_CMD_VER_STR : {                                                                  \
   strncpy((BTSTR)(__arg), __verstr, AAL_SVC_MOD_VER_STR_MAX);                                \
} return 0;                                                                                   \
case AAL_SVC_CMD_VER_CURRENT : {                                                              \
   *(BT32U *)(__arg) = __vercur;                                                              \
} return 0;                                                                                   \
case AAL_SVC_CMD_VER_REVISION : {                                                             \
   *(BT32U *)(__arg) = __verrev;                                                              \
} return 0;                                                                                   \
case AAL_SVC_CMD_VER_AGE : {                                                                  \
   *(BT32U *)(__arg) = __verage;                                                              \
} return 0;                                                                                   \
case AAL_SVC_CMD_GET_PROVIDER : {                                                             \
   AutoLock(&cs);                                                                             \
   if ( NULL == pServiceFactory ) {                                                           \
      pServiceFactory = new __svcfactory();                                                   \
      if ( NULL == pServiceFactory ) {                                                        \
         return 1;                                                                            \
      }                                                                                       \
   }                                                                                          \
   if ( NULL == pServiceProvider ) {                                                          \
      pServiceProvider = new AAL::AALServiceModule(pServiceFactory);                          \
      if ( NULL == pServiceProvider ) {                                                       \
         return 2;                                                                            \
      }                                                                                       \
   }                                                                                          \
   *((AAL::IServiceModule **)(arg)) =                                                         \
      AAL::dynamic_ptr<AAL::IServiceModule>(iidServiceProvider, pServiceProvider);            \
} return 0;                                                                                   \
case AAL_SVC_CMD_FREE_PROVIDER : {                                                            \
   AutoLock(&cs);                                                                             \
   if ( NULL != pServiceProvider ) {                                                          \
      delete pServiceProvider;                                                                \
      pServiceProvider = NULL;                                                                \
   }                                                                                          \
   if ( NULL != pServiceFactory ) {                                                           \
      delete pServiceFactory;                                                                 \
      pServiceFactory = NULL;                                                                 \
   }                                                                                          \
} return 0


/// @brief Beginning of an application-defined Module command.
///
/// Use within an enclosing AAL_BEGIN_MOD/AAL_END_MOD pair to
/// define a custom Module command.
///
/// @param[in]  __cmd  Command identifier constant.
///
/// Ex.@code
/// #define MY_AAL_CMD_A (AAL_SVC_USER_CMD_BASE + 0)
/// #define MY_AAL_CMD_B (AAL_SVC_USER_CMD_BASE + 1)
///
/// AAL_BEGIN_MOD(libMyModule, MYMODULE_API, "1.0.0", 1, 0, 0)
///
///    AAL_BEGIN_MOD_CMD(MY_AAL_CMD_A)
///
///       <C/C++ statements for MY_AAL_CMD_A>
///
///    AAL_END_MOD_CMD() // MY_AAL_CMD_A
///
///    AAL_BEGIN_MOD_CMD(MY_AAL_CMD_B)
///
///       <C/C++ statements for MY_AAL_CMD_B>
///
///    AAL_END_MOD_CMD() // MY_AAL_CMD_B
///
/// AAL_END_MOD()@endcode
#define AAL_BEGIN_MOD_CMD(__cmd) case __cmd : {

/// @brief End of an application-defined Module command.
///
/// Paired with AAL_BEGIN_MOD_CMD when defining a custom AALSDK Module command.
#define AAL_END_MOD_CMD()        } break;


/// @brief Beginning of an application-defined Service Module command.
///
/// Use within an enclosing AAL_BEGIN_SVC_MOD/AAL_END_SVC_MOD pair to
/// define a custom Service Module command.
///
/// @param[in]  __cmd  Command identifier constant.
///
/// Ex.@code
/// #define SERVICE_FACTORY AAL::InProcSvcsFact< MyService >
///
/// #define MY_AAL_SVC_CMD_A (AAL_SVC_USER_CMD_BASE + 2)
/// #define MY_AAL_SVC_CMD_B (AAL_SVC_USER_CMD_BASE + 3)
///
/// AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libMyService, MYSERVICE_API, "1.0.0", 1, 0, 0)
///
///    AAL_BEGIN_SVC_MOD_CMD(MY_AAL_SVC_CMD_A)
///
///       <C/C++ statements for MY_AAL_SVC_CMD_A>
///
///    AAL_END_SVC_MOD_CMD() // MY_AAL_SVC_CMD_A
///
///    AAL_BEGIN_SVC_MOD_CMD(MY_AAL_SVC_CMD_B)
///
///       <C/C++ statements for MY_AAL_SVC_CMD_B>
///
///    AAL_END_SVC_MOD_CMD() // MY_AAL_SVC_CMD_B
///
/// AAL_END_SVC_MOD()@endcode
#define AAL_BEGIN_SVC_MOD_CMD(__cmd) case __cmd : {

/// @brief End of an application-defined Service Module command.
///
/// Paired with AAL_BEGIN_SVC_MOD_CMD when defining a custom AALSDK Service Module command.
#define AAL_END_SVC_MOD_CMD()        } break;


#ifdef __cplusplus

/// @brief Instantiates the Module entry point and default command implementations.
///
/// Used in a Module source file to provide the appropriate C or C++ function
/// for the Module entry point, accounting for the current OS.
///
/// AAL_BEGIN_MOD is used for non-Service Modules like OSAL.
///
/// @param[in]  __rtnamesym   the module root name (libtool) of the loadable module, typically,
///                            the name of the shared library binary without any file extension.
/// @param[in]  __storagecls  the Windows storage class for the dll.
/// @param[in]  __verstr      the libtool version of the module in C:R:A form. (char *)
/// @param[in]  __vercur      the Current component of the C:R:A. (u32)
/// @param[in]  __verrev      the Revision component of the C:R:A. (u32)
/// @param[in]  __verage      the Age component of the C:R:A. (u32)
///
/// Ex.@code
/// AAL_BEGIN_MOD(libMyModule, MYMODULE_API, "1.0.0", 1, 0, 0)
///
/// AAL_END_MOD()@endcode
# define AAL_BEGIN_MOD(__rtnamesym, __storagecls, __verstr, __vercur, __verrev, __verage)      \
extern "C" {                                                                                   \
__storagecls AAL::bt32bitInt AAL_SVC_MOD_ENTRY_POINT(__rtnamesym)(AAL::btUnsigned32bitInt cmd, \
                                                                  AAL::btAny              arg) \
{                                                                                              \
   AAL::bt32bitInt res = 0;                                                                    \
   switch( cmd ) {                                                                             \
      AAL_MOD_DEFAULT_IMPL(arg, __verstr, __vercur, __verrev, __verage);

/// @brief Completes the Module entry point and default command implementations.
# define AAL_END_MOD()                                                                         \
      default : res = -1; break;                                                               \
   }                                                                                           \
   return res;                                                                                 \
}                                                                                              \
} /* extern "C" */

#else

# define AAL_BEGIN_MOD(__rtnamesym, __storagecls, __verstr, __vercur, __verrev, __verage) \
__storagecls bt32bitInt AAL_SVC_MOD_ENTRY_POINT(__rtnamesym)(btUnsigned32bitInt cmd,      \
                                                             btAny              arg)      \
{                                                                                         \
   bt32bitInt res = 0;                                                                    \
   switch( cmd ) {                                                                        \
      AAL_MOD_DEFAULT_IMPL(arg, __verstr, __vercur, __verrev, __verage);


# define AAL_END_MOD()                                                                    \
      default : res = -1; break;                                                          \
   }                                                                                      \
   return res;                                                                            \
}

#endif // __cplusplus


#ifdef __cplusplus

/// @brief Instantiates the Service Module entry point and default command implementations.
///
/// Used in a Service Module source file to provide the appropriate C++ function
/// for the Service Module entry point, accounting for the current OS.
///
/// Implements the default Module commands as well as the Service Module commands:
/// <ul>
///   <li>AAL_SVC_CMD_GET_PROVIDER</li>
///   <li>AAL_SVC_CMD_FREE_PROVIDER</li>
/// </ul>
///
/// @param[in]  __svcfactory  the C++ type of the Service Module's Factory class.
/// @param[in]  __rtnamesym   the module root name (libtool) of the loadable module, typically,
///                            the name of the shared library binary without any file extension.
/// @param[in]  __storagecls  the Windows storage class for the dll.
/// @param[in]  __verstr      the libtool version of the module in C:R:A form. (char *)
/// @param[in]  __vercur      the Current component of the C:R:A. (u32)
/// @param[in]  __verrev      the Revision component of the C:R:A. (u32)
/// @param[in]  __verage      the Age component of the C:R:A. (u32)
///
/// Ex.@code
/// #define SERVICE_FACTORY AAL::InProcSvcsFact< MyService >
///
/// AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libMyService, MYSERVICE_API, "1.0.0", 1, 0, 0)
///
/// AAL_END_SVC_MOD()@endcode
# define AAL_BEGIN_SVC_MOD(__svcfactory, __rtnamesym, __storagecls, __verstr, __vercur, __verrev, __verage) \
extern "C" {                                                                                                \
__storagecls AAL::bt32bitInt AAL_SVC_MOD_ENTRY_POINT(__rtnamesym)(AAL::btUnsigned32bitInt cmd,              \
                                                                  AAL::btAny              arg)              \
{                                                                                                           \
   static __svcfactory          *pServiceFactory  = NULL;                                                   \
   static AAL::AALServiceModule *pServiceProvider = NULL;                                                   \
   static CriticalSection        cs;                                                                        \
   AAL::bt32bitInt               res = 0;                                                                   \
   switch( cmd ) {                                                                                          \
      AAL_SVC_MOD_DEFAULT_IMPL(__svcfactory, arg, __verstr, __vercur, __verrev, __verage);

/// @brief Completes the Service Module entry point and default command implementations.
# define AAL_END_SVC_MOD()                                                                                  \
      default : res = -1; break;                                                                            \
   }                                                                                                        \
     return res;                                                                                            \
  }                                                                                                         \
  } /* extern "C" */

#endif // __cplusplus



////////////////////////////////////////////////////////////////////////////////
# define AAL_BUILTIN_SVC_MOD_ENTRY_POINT(__x )  __##__y##BuiltInAALSvcMod

//=============================================================================
// Name: AAL_DECLARE_BUILTIN_MOD
// Description: Declare an AAL Module entry point.
// Inputs: __rtnamesym - Typically the name of the file containig the module
//                       e.g. libOSAL
// Comments:
//=============================================================================
#ifdef __cplusplus

# define AAL_DECLARE_BUILTIN_MOD(__rtnamesym, __storagecls)                                                              \
extern "C" {                                                                                                             \
extern __storagecls AAL::bt32bitInt AAL_BUILTIN_SVC_MOD_ENTRY_POINT(__rtnamesym)(AAL::btUnsigned32bitInt , AAL::btAny ); \
}

#else

# define AAL_DECLARE_BUILTIN_MOD(__rtnamesym, __storagecls)                                             \
extern __storagecls bt32bitInt AAL_BUILTIN_SVC_MOD_ENTRY_POINT(__rtnamesym)(btUnsigned32bitInt, btAny );

#endif // __cplusplus

#define AAL_DECLARE_BUILTIN_SVC_MOD(__rtnamesym, __storagecls) AAL_DECLARE_BUILTIN_MOD(__rtnamesym, __storagecls)

//=============================================================================
// Name: AAL_BEGIN_BUILTIN_SVC_MOD
// Description: Implements the common module entry points and commands for
//              built-in services.
// Inputs:   __rtnamesym - module Root Name Symbol. eg libOSAL
//           __verstr - (string) The libtool version of the module in C:R:A form.
//           __vercur - (u32)    The Current component of the C:R:A.
//           __verrev - (u32)    The Revision component of the C:R:A.
//           __verage - (u32)    The Age component of the C:R:A.
// Comments: AAL_BEGIN_SVC_MOD is used for Service modules and includes the
//           command to access the Service Factory.
//=============================================================================

#ifdef __cplusplus

# define AAL_BEGIN_BUILTIN_SVC_MOD(__svcfactory, __rtnamesym, __storagecls, __verstr, __vercur, __verrev, __verage) \
extern "C" {                                                                                                \
__storagecls AAL::bt32bitInt AAL_BUILTIN_SVC_MOD_ENTRY_POINT(__rtnamesym)(AAL::btUnsigned32bitInt cmd,              \
                                                                          AAL::btAny              arg)              \
{                                                                                                           \
   static __svcfactory          *pServiceFactory  = NULL;                                              \
   static AAL::AALServiceModule *pServiceProvider = NULL;                                              \
   static CriticalSection        cs;                                                                   \
   AAL::bt32bitInt               res = 0;                                                              \
   switch( cmd ) {                                                                                          \
      AAL_SVC_MOD_DEFAULT_IMPL(__svcfactory, arg, __verstr, __vercur, __verrev, __verage);


#endif // __cplusplus



/// @brief Low-level Service Module handle used by OSServiceModuleInit, OSServiceModuleOpen, and OSServiceModuleClose.
///
/// Refer to @ref aalscan "aalscan" for example usage.
typedef struct OSAL_API _OSServiceModule
{
#if   defined( __AAL_WINDOWS__ )
   HMODULE          handle;                                       ///< hDll for the Service Module
#elif defined( __AAL_LINUX__ )
   lt_dlhandle      handle;                                       ///< ltdl handle for the Service Module
#endif // OS
   BTBYTE           root_name[AAL_SVC_MOD_ROOT_NAME_MAX];         ///< root name of module. (ie, libtool -module name. eg "libOSAL")
   BTBYTE           full_name[AAL_SVC_MOD_FULL_NAME_MAX];         ///< full name of module. (OS-specific. eg, "libOSAL.so")
   BTBYTE           entry_point_name[AAL_SVC_MOD_ENTRY_NAME_MAX]; ///< module entry point name. (eg. "libOSALAALSvcMod")
   AALSvcEntryPoint entry_point_fn;                               ///< address of the module entry point.
} OSServiceModule;

/// @brief Initialize the given OSServiceModule, preparing it to open the Service Module whose module root name is root_name.
///
/// @param[out]  p          Address of the OSServiceModule to initialize.
/// @param[in]   root_name  The module root name of the desired Service Module.
void OSAL_API OSServiceModuleInit(OSServiceModule *p, BTCSTR root_name);

/// @brief Open (load) the Service Module described by p and resolve its entry point.
///
/// @param[in]  p  Address of the initialized (OSServiceModuleInit) Service Module data structure.
///
/// @retval 0 Service Module was successfully loaded and its entry point resolved.
/// @retval 1 OSServiceModule data structure already open or not initialized.
/// @retval 2 Service Module not found.
/// @retval 3 Service Module entry point not found within Service Module.
/// @retval (other non-zero error code) if another internal error occurs.
///
/// Sets p->error_msg on failure.
BT32I OSAL_API OSServiceModuleOpen(OSServiceModule *p);

/// @brief Close (unload) the Service Module described by p.
///
/// @param[in]  p  Address of the opened (OSServiceModuleOpen) Service Module data structure.
///
/// @retval 0 Service Module was successfully closed.
/// @retval non-0 and set p->error_msg on failure.
BT32I OSAL_API OSServiceModuleClose(OSServiceModule *p);

END_C_DECLS

/// @}

#endif // __OSAL_OSSERVICEMODULE_H__

