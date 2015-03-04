// Copyright (c) 2014-2015, Intel Corporation
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
/// @file Env.h
/// @brief Object for manipulating the environment variables.
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Sadruta Chandrashekar, Intel Corporation
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 9/29/2014      JG       Created @endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_ENV_H__
#define __AALSDK_OSAL_ENV_H__
#include <aalsdk/AALDefs.h>

BEGIN_NAMESPACE(AAL)

//=============================================================================
///Environment
/// @brief  Used for Accessing and mutating environment variables.
//=============================================================================
class Environment
{
public:
   Environment() :
      m_buf(NULL)
   {}

   virtual ~Environment()
   {
      if ( NULL != m_buf ) {
          delete m_buf;
          m_buf = NULL;
      }
   }

   //=============================================================================
   /// Get
   /// Get the value of a variable
   /// @param[in] var - variable to query
   ///  @returns pointer to value or NULL if not set.
   /// Comments:
   //=============================================================================

   // TODO Test case: Environment::Get() behaves robustly when var is NULL.
   // TODO Test case: thread safety for Environment::Get().
   btcString Get(btcString var)
   {
#if defined( __AAL_LINUX__ )

      return  std::getenv(var);

#elif defined( __AAL_WINDOWS__ )

      DWORD bufsize = GetEnvironmentVariable(var, NULL, 0);
      if ( 0 == bufsize ) {
         // variable doesn't exist.
         return NULL;
      }

      // TODO this isn't thread safe.

      if ( NULL != m_buf ) {
         delete m_buf;
      }

      m_buf = new btByte[bufsize];

      // TODO NULL check for above new?

      GetEnvironmentVariable(var, m_buf,bufsize);

      // TODO check return code.

      return m_buf;
#endif
   }

//=============================================================================
///  Set
///  Set the value of a variable
/// @param[in] var - variable to query
/// @param[in] val - value to set it to
/// @param[in] overwrite - what to do if it already exists
/// @returns true - success
//=============================================================================

   // TODO Test case: Environment::Set() behaves robustly when var is NULL.
   // TODO Test case: Environment::Set() behaves robustly when val is NULL.
   // TODO Test case: thread safety for Environment::Set().

   btBool Set(btcString var, btcString val, btBool overwrite=true)
   {
#if defined( __AAL_LINUX__ )

      int ret = setenv(var, val, overwrite ? 1 : 0);
      return 0 == ret;

#elif defined( __AAL_WINDOWS__ )

      if ( !overwite ) {

         // TODO This is inefficient - when getting the same var twice, we will
         // delete / new m_buf each time.

         if ( NULL!= getVal() );
         // Already exists
         return false;
      }
      return SetEnvironmentVariable(var,val);
#endif
   }

private:
   btString m_buf;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_OSAL_ENV_H__

