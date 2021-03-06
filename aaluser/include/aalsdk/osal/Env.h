// Copyright(c) 2014-2016, Intel Corporation
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
/// Accelerator Abstraction Layer
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
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/CUnCopyable.h>

/// @addtogroup OSAL
/// @{

BEGIN_NAMESPACE(AAL)

//=============================================================================
///Environment
/// @brief  Used for Accessing and mutating environment variables.
//=============================================================================
class OSAL_API Environment : public CUnCopyable
{
public:
   virtual ~Environment() {}

   //=============================================================================
   // GetObj
   /// Get an instance of the Environment class.
   /// @returns A pointer to an instance of Environment class.
   // Comments:
   //=============================================================================
   static Environment * GetObj();

   //=============================================================================
   // ReleaseObj
   /// Release instance of the Environment class.
   /// @return void
   // Comments:
   //=============================================================================
   static void ReleaseObj();

   //=============================================================================
   // Get
   /// Get the value of an environment variable.
   /// @param[in] var - The variable to query.
   /// @param[in] val - Reference variable to hold the value.
   /// @retval true  - success.
   /// @retval false - failure.
   // Comments:
   //=============================================================================
   btBool Get(std::string var, std::string &val);

   //=============================================================================
   //  Set
   ///  Set the value of an environment variable.
   /// @param[in] var - The variable to set.
   /// @param[in] val - The value to set it to.
   /// @param[in] overwrite - What to do if it already exists.
   /// @retval true  - success.
   /// @retval false - failure.
   //==============================================================================
   btBool Set(std::string var, std::string val, btBool overwrite=true);

private:
   Environment();

   Environment(Environment const & ); //copy constructor is made private so that singleton cannot be copied
   Environment & operator = (const Environment & ); //Assignment operator is made private

   static Environment    *sm_EnvObj;
   static CriticalSection sm_Lock;
};

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_OSAL_ENV_H__

