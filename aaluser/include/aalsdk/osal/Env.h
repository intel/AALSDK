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

#if   defined( __AAL_LINUX__ )
# include <cstdlib>
#elif defined( __AAL_WINDOWS__ )
# include <Windows.h>  //Processenv.h on Windows 8 and Windows Server 2012
#endif

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

END_C_DECLS

//=============================================================================
///Environment
/// @brief  Used for Accessing and mutating environment variables.
//=============================================================================
class Environment{
public:
   Environment():m_buf(NULL){}
   ~Environment()
   {
      if(NULL != m_buf ){
          delete m_buf;
          m_buf = NULL;
      }
   }

   //=============================================================================
   /// getVal
   /// Get the value of a variable
   /// @param[in] var - variable to query
   ///  @returns pointer to value or NULL if not set.
   /// Comments:
   //=============================================================================
   const char *getVal(const char* var) const{

#if defined( __AAL_LINUX__ )
      return  std::getenv(var);

   #elif defined( __AAL_WINDOWS__ )
      DWORD bufsize = GetEnvironmentVariable(var, NULL,0);
      if(0=bufsize){
         return NULL;
      }
      if(NULL != m_buf ){
         delete m_buf;
      }
      m_buf = new btString[bufsize];
      GetEnvironmentVariable(var, m_buf,bufsize);
      }
      return m_buf;

#endif
   }

//=============================================================================
///  setEnv
///  Set the value of a variable
/// @param[in] var - variable to query
/// @param[in] val - value to set it to
/// @param[in] overwrite - what to do if it already exists
/// @returns true - success
//=============================================================================
   btBool setEnv(btcString var, btcString val, btBool overwrite=true)
   {
#if defined( __AAL_LINUX__ )
      int ret = setenv(var, val, overwrite==true ? 1 : 0);
      return ret == 0 ? true : false;

#elif defined( __AAL_WINDOWS__ )
      if(true != overwite{
         if( NULL!= getVal() );
         // Already exists
         return false;
      }
      return SetEnvironmentVariable(var,val);
#endif
   }

private:
   btString       *m_buf;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_OSAL_ENV_H__
