// Copyright(c) 2003-2016, Intel Corporation
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
/// @file Env.cpp
/// @brief Env interface.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Sadruta Chandrashekar, Intel Corporation
/// 		 Tim Whisonant, Intel Corporation
///
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/07/2015     SC
/// @endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/Env.h"

BEGIN_NAMESPACE(AAL)

Environment * Environment::GetObj()
{
   AutoLock(&Environment::sm_Lock);

   if ( NULL == Environment::sm_EnvObj ) {
      Environment::sm_EnvObj = new(std::nothrow) Environment();
   }

   return Environment::sm_EnvObj;
}

void Environment::ReleaseObj()
{
   AutoLock(&Environment::sm_Lock);

   if ( NULL != Environment::sm_EnvObj ) {
      delete Environment::sm_EnvObj;
      Environment::sm_EnvObj = NULL;
   }
}

btBool Environment::Get(std::string var, std::string &val)
{
   AutoLock(&Environment::sm_Lock);

#if   defined( __AAL_LINUX__ )

   const char* temp_var =  var.c_str();
   char* temp_val = std::getenv(temp_var);

   if ( NULL != temp_val ) {
      val.assign(temp_val);
   }

   return NULL != temp_val;

#elif defined( __AAL_WINDOWS__ )

   char* temp_val = NULL;
   const char* temp_var;

   temp_var = var.c_str();

   DWORD bufsize = GetEnvironmentVariable(temp_var, NULL, 0);

   if (( 0 == bufsize ) && ( ERROR_ENVVAR_NOT_FOUND == GetLastError())) {
      // variable doesn't exist.
      return false;
   }

   if ( 0 == bufsize ) {
      val.assign("");
      return true;
   }

   temp_val = new char[bufsize];

   if ( NULL == temp_val ) {
      return false;
   }
   GetEnvironmentVariable(temp_var, temp_val, bufsize);

   val.assign(temp_val);

   delete temp_val;

   return true;
#endif // OS
}

btBool Environment::Set(std::string var, std::string val, btBool overwrite)
{
   AutoLock(&Environment::sm_Lock);

#if   defined( __AAL_LINUX__ )

   const char* temp_var = var.c_str();
   const char* temp_val = val.c_str();

   int ret = setenv(temp_var, temp_val, overwrite ? 1 : 0);

   return 0 == ret;

#elif defined( __AAL_WINDOWS__ )

   if ( !overwite ) {
      if ( Environment::Get(var, val) ) {
         // Already exists
         return false;
      }
   }

   btBool ret = SetEnvironmentVariable(var,val);

   return ret;
#endif // OS
}

Environment::Environment() {}
Environment::Environment(Environment const & ) {}
Environment & Environment::operator = (Environment const & ) { return *this; }

Environment *   Environment::sm_EnvObj = NULL;
CriticalSection Environment::sm_Lock;

END_NAMESPACE(AAL)
