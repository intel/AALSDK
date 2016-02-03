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
/// @file data_model.c
/// @brief Display the Data Model employed by the current compiler.
/// @ingroup data_model
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing
///     commercially-deployable applications.
///
/// AUTHORS: Tim Whisonant, Intel Corporation.@endverbatim
/**
@addtogroup data_model
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

Report the data model used by the current compiler.

1 Summary of Operation

As the addressable memory space and CPU register size of PCs have evolved over time, C/C++ compilers
have also evolved in support of operating systems for these platforms. The scalar types described
in the C/C++ language standards are given names that reflect the kinds of values they are likely to
contain, rather than the amount of memory occupied by instances of these types.

<ul>
  <li>(unsigned) char</li>
  <li>(unsigned) short int</li>
  <li>(unsigned) int</li>
  <li>(unsigned) long int</li>
  <li>(unsigned) long long int</li>
</ul>
 
Compilers choose the sizes for these scalar types based on the needs of the current system.
For example, in the original days of DOS, int and long were constrained to 16 bits, matching
the prevelant address space size of PCs from that time. Modern PCs implement 64-bit addressing,
yet operating systems continue to differ in the sizes associated with the scalar types.

The choice of sizes for the scalar types is refered to as the "Data Model" and is easily
discovered using the C/C++ sizeof() operator. This is what data_model does.

Further information on Data Models can be found at http://en.wikipedia.org/wiki/64-bit_computing

2 Running the application

@verbatim
$ data_model
lp64
The compiler defines __LP64__@endverbatim

@}
*/
//****************************************************************************
#include <stdio.h>

#define __AALSDK_DATA_MODEL_LP32  0
#define __AALSDK_DATA_MODEL_ILP32 1
#define __AALSDK_DATA_MODEL_ILP64 2
#define __AALSDK_DATA_MODEL_LLP64 3
#define __AALSDK_DATA_MODEL_LP64  4

#if 1
int main(int argc, char *argv[])
{
   if ( ( 2 == sizeof(int)    ) &&
        ( 4 == sizeof(long)   ) &&
        ( 4 == sizeof(void *) ) ) {

      printf("lp32\n");

#ifdef __LP32__
      printf("The compiler defines __LP32__\n");
#else
      printf("The compiler does not define __LP32__\n");
#endif // __LP32__

      return __AALSDK_DATA_MODEL_LP32;

   } else if ( ( 4 == sizeof(int)    ) &&
               ( 4 == sizeof(long)   ) &&
               ( 4 == sizeof(void *) ) ) {

      printf("ilp32\n");

#ifdef __ILP32__
      printf("The compiler defines __ILP32__\n");
#else
      printf("The compiler does not define __ILP32__\n");
#endif // __ILP32__

      return __AALSDK_DATA_MODEL_ILP32;

   } else if ( ( 8 == sizeof(int)    ) &&
               ( 8 == sizeof(long)   ) &&
               ( 8 == sizeof(void *) ) ) {

      printf("ilp64\n");

#ifdef __ILP64__
      printf("The compiler defines __ILP64__\n");
#else
      printf("The compiler does not define __ILP64__\n");
#endif // __ILP64__

      return __AALSDK_DATA_MODEL_ILP64;

   } else if ( ( 4 == sizeof(int)    ) &&
               ( 4 == sizeof(long)   ) &&
               ( 8 == sizeof(void *) ) ) {

      printf("llp64\n");

#ifdef __LLP64__
      printf("The compiler defines __LLP64__\n");
#else
      printf("The compiler does not define __LLP64__\n");
#endif // __LLP64__

      return __AALSDK_DATA_MODEL_LLP64;

   } else if ( ( 4 == sizeof(int)    ) &&
               ( 8 == sizeof(long)   ) &&
               ( 8 == sizeof(void *) ) ) {

      printf("lp64\n");

#ifdef __LP64__
      printf("The compiler defines __LP64__\n");
#else
      printf("The compiler does not define __LP64__\n");
#endif // __LP64__

      return __AALSDK_DATA_MODEL_LP64;

   } else {

      printf("Unable to determine the data model.\n");

   }

   return 99;
}
#else
int main(int argc, char *argv[])
{
   if ( ( 2 == sizeof(int)    ) &&
        ( 4 == sizeof(long)   ) &&
        ( 4 == sizeof(void *) ) ) {
      return 0;
   }

   if ( ( 4 == sizeof(int)    ) &&
        ( 4 == sizeof(long)   ) &&
        ( 4 == sizeof(void *) ) ) {
      return 1;
   }

   if ( ( 8 == sizeof(int)    ) &&
        ( 8 == sizeof(long)   ) &&
        ( 8 == sizeof(void *) ) ) {
      return 2;
   }

   if ( ( 4 == sizeof(int)    ) &&
        ( 4 == sizeof(long)   ) &&
        ( 8 == sizeof(void *) ) ) {
      return 3;
   }
 
   if ( ( 4 == sizeof(int)    ) &&
        ( 8 == sizeof(long)   ) &&
        ( 8 == sizeof(void *) ) ) {
      return 4;
   }
 
   return 99;
}
#endif

