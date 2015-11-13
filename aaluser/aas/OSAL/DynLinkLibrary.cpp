// Copyright (c) 2007-2015, Intel Corporation
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
/// @file DynLinkLibrary.cpp
/// @brief Implements Abstractions for Dynamic Link Libraries
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 03/21/2007     JG       Initial version started
/// 11/27/2007     JG/HM    Fixed leaks
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 06/21/2009     JG       Added RTLD_GLOBAL flag to allow plug-in model to
///                            work correctly
/// 07/23/2009     HM       DynLinkLibrary dtor added error check
/// 02/11/2010     JG       Added support for glib version 4.4@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#if HAVE_LTDL_H
# include <ltdl.h>
#endif // HAVE_LTDL_H

#include "aalsdk/osal/DynLinkLibrary.h"

#if HAVE_LTDL_H && HAVE_PTHREAD_H
BEGIN_C_DECLS
int aalsdk_ltdl_lock(void);
int aalsdk_ltdl_unlock(void);
END_C_DECLS
#else
# define aalsdk_ltdl_lock()   0
# define aalsdk_ltdl_unlock() 0
#endif // HAVE_LTDL_H && HAVE_PTHREAD_H

BEGIN_NAMESPACE(AAL)

DynLinkLibrary::DynLinkLibrary() {/*empty*/}
DynLinkLibrary::DynLinkLibrary(const DynLinkLibrary & ) {/*empty*/}
DynLinkLibrary & DynLinkLibrary::operator=(const DynLinkLibrary & ) { return *this; }

//=============================================================================
// Name:  DynLinkLibrary
// Description: Constructor
// Interface: public
// Inputs: LibraryName - Path to the library
// Comments:
//=============================================================================
DynLinkLibrary::DynLinkLibrary(const std::string &LibraryName) :
   m_hDLL(NULL)
{
//Load the library
#ifdef __AAL_WINDOWS__
   const char *ErrMsg = "LoadLibrary() failed";
   m_hDLL = LoadLibrary( LibraryName.c_str() );
//#else
//   // RTLD_GLOBAL to make all of the service's symbols visible and
//   // RTLD_LAZY to allow late symbol look-up
//   m_hDLL = dlopen(LibraryName.c_str(), RTLD_GLOBAL | RTLD_LAZY);
//   const char *Errmsg = dlerror();
#endif // __AAL_WINDOWS__

#if HAVE_LTDL_H
   const char *ErrMsg = NULL;
   lt_dladvise advise;

   if ( 0 != aalsdk_ltdl_lock() ) {
      std::cerr << "DynLinkLibrary::DynLinkLibrary() : aalsdk_ltdl_lock() failed." << std::endl;
   }

   if ( 0 != lt_dladvise_init(&advise) ) {
      std::cerr << "DynLinkLibrary::DynLinkLibrary() : lt_dladvise_init() failed";
      ErrMsg = lt_dlerror();
      if ( NULL != ErrMsg ) {
         std::cerr << " : " << ErrMsg;
      }
      std::cerr << std::endl;
   }

   if ( 0 != lt_dladvise_global(&advise) ) {
      std::cerr << "DynLinkLibrary::DynLinkLibrary() : lt_dladvise_global() failed";
      ErrMsg = lt_dlerror();
      if ( NULL != ErrMsg ) {
         std::cerr << " : " << ErrMsg;
      }
      std::cerr << std::endl;
   }

# if DBG_DYN_LOAD
   std::cerr << "[DBG_DYN_LOAD] Trying " << LibraryName << " ";
# endif // DBG_DYN_LOAD

   // First, try the less-restrictive unqualified path (ie, module name only).

   m_hDLL = (btObjectType)lt_dlopenadvise( LibraryName.c_str(), advise );

   if ( NULL == m_hDLL ) {
      // Now try the fully-qualified installation path for the lib.
      const std::string FullPath = std::string(LIBDIR "/") + LibraryName;

# if DBG_DYN_LOAD
      std::cerr << " [Fail]. Trying " << FullPath << " ";
# endif // DBG_DYN_LOAD

      m_hDLL = (btObjectType)lt_dlopenadvise( FullPath.c_str(), advise );

# if DBG_DYN_LOAD
      if ( NULL != m_hDLL ) {
         std::cerr << "[OK]." << std::endl;
      }
# endif // DBG_DYN_LOAD

   }
# if DBG_DYN_LOAD
   else {
      std::cerr << "[OK]." << std::endl;
   }
# endif // DBG_DYN_LOAD

   ErrMsg = lt_dlerror();

   if ( 0 != lt_dladvise_destroy(&advise) ) {
      std::cerr << "DynLinkLibrary::DynLinkLibrary() : lt_dladvise_destroy() failed." << std::endl;
   }

   if ( 0 != aalsdk_ltdl_unlock() ) {
      std::cerr << "DynLinkLibrary::DynLinkLibrary() : aalsdk_ltdl_unlock() failed." << std::endl;
   }

#endif // HAVE_LTDL_H

   if ( NULL == m_hDLL ) {
      // TODO use logger here?
      std::cerr << "DynLinkLibrary::DynLinkLibrary() Error";
      if ( NULL != ErrMsg ) {
         std::cerr << " : File [" << LibraryName << "] " << ErrMsg << " (or unresolved symbol in library)";
      }
      std::cerr << std::endl;
      return;
   }

//   cerr << "DynLinkLibrary::DynLinkLibrary: m_hDLL is " << (void*)m_hDLL << ". pLibrary is " << (void*)this << endl;
}

//=============================================================================
// Name:  ~DynLinkLibrary
// Description: Destructor
// Interface: public
// Inputs: none
// Comments:
//=============================================================================
DynLinkLibrary::~DynLinkLibrary()
{
   // Free the library
   if ( NULL != m_hDLL ) {
//      cerr << "DynLinkLibrary::~DynLinkLibrary: m_hDLL is " << (void*)m_hDLL << ". pLibrary is " << (void*)this << endl;

#ifdef __AAL_WINDOWS__
      FreeLibrary((HMODULE)m_hDLL);
//#else
//      if ( dlclose(m_hDLL) ) {
//         // TODO use logger here?
//         std::cerr << "DynLinkLibrary::~DynLinkLibrary: dlclose error: " << dlerror();
//      }
#endif // __AAL_WINDOWS__

#if HAVE_LTDL_H
      const char *ErrMsg;

      if ( 0 != aalsdk_ltdl_lock() ) {
         std::cerr << "DynLinkLibrary::~DynLinkLibrary() : aalsdk_ltdl_lock() failed." << std::endl;
      }

      if ( lt_dlclose( (lt_dlhandle)m_hDLL ) ) {
         // TODO use logger here?
         ErrMsg = lt_dlerror();
         std::cerr << "DynLinkLibrary::~DynLinkLibrary : lt_dlclose() error";
         if ( NULL != ErrMsg ) {
            std::cerr << ": " << ErrMsg;
         }
         std::cerr << std::endl;
      }

      if ( 0 != aalsdk_ltdl_unlock() ) {
         std::cerr << "DynLinkLibrary::~DynLinkLibrary() : aalsdk_ltdl_unlock() failed." << std::endl;
      }

#endif // HAVE_LTDL_H

      m_hDLL = NULL;
   }
}

//=============================================================================
// Name: GetSymAddress
// Description: Gets the address of the named symbol
// Interface: public
// Inputs: SymbolName - Symbol name
// Outputs: pointer to symbol
// Comments:
//=============================================================================
void * DynLinkLibrary::GetSymAddress(const std::string &SymbolName)
{
   if ( NULL == m_hDLL ) {
      return NULL;
   }

#ifdef __AAL_WINDOWS__
   // Get the entry point
   return GetProcAddress((HMODULE)m_hDLL, SymbolName.c_str());
//#else
//   return dlsym(m_hDLL, SymbolName.c_str());
#endif // __AAL_WINDOWS__

#if HAVE_LTDL_H
   void *res;

# if DBG_DYN_LOAD
   const char *ErrMsg = NULL;
   std::cerr << "[DBG_DYN_LOAD] Looking up " << SymbolName;
# endif // DBG_DYN_LOAD

   if ( 0 != aalsdk_ltdl_lock() ) {
      std::cerr << "DynLinkLibrary::GetSymAddress() : aalsdk_ltdl_lock() failed." << std::endl;
   }

   res = (void *) lt_dlsym((lt_dlhandle)m_hDLL, SymbolName.c_str());

   if ( 0 != aalsdk_ltdl_unlock() ) {
      std::cerr << "DynLinkLibrary::GetSymAddress() : aalsdk_ltdl_unlock() failed." << std::endl;
   }

# if DBG_DYN_LOAD
   if ( NULL == res ) {
      ErrMsg = lt_dlerror();
      std::cerr << " [Fail]";
      if ( NULL != ErrMsg ) {
         std::cerr << " : " << ErrMsg << std::endl;
      }
      std::cerr << std::endl;
   } else {
      std::cerr << " [OK]." << std::endl;
   }
# endif // DBG_DYN_LOAD

   return res;
#endif // HAVE_LTDL_H
}

END_NAMESPACE(AAL)

