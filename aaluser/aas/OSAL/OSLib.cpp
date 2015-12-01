// Copyright (c) 2003-2015, Intel Corporation
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
/// @file OSLib.cpp
/// @brief OSlib - Part of the OS utility abstraction library. Implements
///        DLL entry points (DLLmain) in Windows.  Also implements various
///        string conversion routines
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Charlie Lasswell, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 2003           CL       Initial version started as part of Tioga
///                         project.
/// 2007           JG       Modified and extended for AAL project
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 02/25/2008     HM       Removed all StringTo/FromNarrow functions by #ifdef 0
///                            Left them because may be needed in the future
///                            Difficult to port between Linux and Windows
/// 05/08/2008     HM       Cleaned up windows includes
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#if HAVE_LTDL_H
# include <ltdl.h>
#endif // HAVE_LTDL_H

#include "aalsdk/AALDefs.h"
#include "aalsdk/osal/OSALService.h"


#if   defined ( __AAL_LINUX__ )
# include <cstring>         /* ffsll() */
#elif defined ( __AAL_WINDOWS__ )
# include <intrin.h>        /* BitScanForwardxxx */
#else
# error TODO Update FindLowestBitSet* for unknown OS
#endif // OS

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks (MASTER)*/
//   #pragma warning( push)
//   #pragma warning( pop)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
//   #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
     #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif


#if HAVE_LTDL_H

BEGIN_C_DECLS

static AAL::bt32bitInt gAALltInitCount = 0;
static lt_dladvise     gAALltdladvise;

void __attribute__ ((constructor)) on_load(void)
{
   if ( 0 != aalsdk_ltdl_lock() ) {
      fprintf(stderr, "OSAL:on_load() : aalsdk_ltdl_lock() failed.\n");
   }

   if ( 0 == lt_dlinit() ) {
      ++gAALltInitCount;
   } else {
      fprintf(stderr, "OSAL:on_load() : lt_dlinit() failed.\n");
   }

   if ( 0 != aalsdk_ltdl_unlock() ) {
      fprintf(stderr, "OSAL:on_load() : aalsdk_ltdl_unlock() failed.\n");
   }
}

void __attribute__ ((destructor)) on_unload(void)
{
   if ( 0 != aalsdk_ltdl_lock() ) {
      fprintf(stderr, "OSAL:on_unload() : aalsdk_ltdl_lock() failed.\n");
   }

   if ( gAALltInitCount > 0 ) {
      --gAALltInitCount;
      lt_dlexit();
   }

   if ( 0 != aalsdk_ltdl_unlock() ) {
      fprintf(stderr, "OSAL:on_unload() : aalsdk_ltdl_unlock() failed.\n");
   }
}

END_C_DECLS

#endif // HAVE_LTDL_H


#ifdef __AAL_WINDOWS__
   BOOL APIENTRY DllMain(HANDLE hModule,
                         DWORD  ul_reason_for_call,
                         LPVOID lpReserved)
   {
      HANDLE hTemp  = hModule;
      LPVOID lpVoid = lpReserved;
      switch ( ul_reason_for_call ) {
         case DLL_PROCESS_ATTACH :
         case DLL_THREAD_ATTACH  :
         case DLL_THREAD_DETACH  :
         case DLL_PROCESS_DETACH :
            break;
      }
      return TRUE;
   }
#endif // __AAL_WINDOWS__


#if defined ( __AAL_WINDOWS__ )                                               
#pragma warning( push)            
#pragma warning( disable : 4996) // destination of copy is unsafe            
#endif

OSAL_BEGIN_MOD()
   /* Only default cmds at the moment. */
OSAL_END_MOD()

#if defined ( __AAL_WINDOWS__ )                                               
#pragma warning( pop )                                                       
#endif


BEGIN_NAMESPACE(AAL)

//=============================================================================
//=============================================================================
//=============================================================================
//==============              FindLowestBitSet64               ================
//=============================================================================
//=============================================================================
//=============================================================================

//=============================================================================
/// FindLowestBitSet64
/// @brief     Find the first bit set, scanning low to high.
/// @param[in] 64-bit entity to test
/// @return    0 if no bits set, otherwise bit position 1 to 64
///            NOT 0 to 63
///
/// @note   Windows intrinsic numbers bits 0 to 63, ffsll numbers
///         them 1 to 64. Using ffsll variation.
/// @note   Windows intrinsic must be set outside function
//
#if defined( __AAL_WINDOWS__ )    /* proxy for all Windows code */
# if (8 == sizeof_void_ptr)
#    pragma intrinsic(_BitScanForward64)
# elif (4 == sizeof_void_ptr)
#    pragma intrinsic(_BitScanForward)
# else
#    error Update for new sizeof_void_ptr
# endif
#endif
//=============================================================================
OSAL_API 
//unsigned long FindLowestBitSet64( __int64 value)
unsigned long FindLowestBitSet64(btUnsigned64bitInt value)
{
#if (8 == sizeof_void_ptr)
# if defined( __AAL_LINUX__ )
//      #pragma message ("linux 64")
      return ffsll(value);
# elif defined ( __AAL_WINDOWS__ )
//      #pragma message ("windows 64")
      unsigned long index [2];   // bug in windows implementation, eats 64-bits, not 32
      unsigned char retval = _BitScanForward64(&index[0], value);
      if( retval) {
         return index[0]+1;
      }
      else {
         return 0;
      }
# else
#    error Unknown OS
# endif
#elif (4 == sizeof_void_ptr)
# if defined( __AAL_LINUX__ )
#    pragma message ("linux 32")     /* never tested */
      return ffsll(value);
# elif defined ( __AAL_WINDOWS__ )
//      #pragma message ("windows 32")
      unsigned long index [2];   // bug in windows implementation, eats 64-bits, not 32
      unsigned char retval = _BitScanForward(&index[0], (unsigned long)value);   // do lower 32 bits
      if( retval) {
         return index[0]+1;
      }
      else {
         retval = _BitScanForward(&index[0], (unsigned long)(value >> 32));   // do upper 32 bits
         if( retval) {
            return index[0]+1+32;
         }
      }
      return 0;      // no bits found
# else
#    error Unknown OS
# endif
#else
# error Update for new sizeof_void_ptr
#endif
} // FindLowestBitSet64



#if 0 /* removed all of this code until need to deal with it - HM 2008.2.25 */
   #if defined( __AAL_WINDOWS__ )
      /* JG Changed
      void OSAL_API StringToNarrow(const String & strWide, NString & strNarrow)
      {
         strNarrow.resize(strWide.size());
      //JG	WideCharToMultiByte(CP_ACP, 0, strWide.c_str(), strWide.size(), strNarrow.begin(), strNarrow.size(), NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, strWide.c_str(), strWide.size(), strNarrow.c_str(), strNarrow.size(), NULL, NULL);
      }

      void OSAL_API StringToNarrow(const TCHAR * strWide, NString & strNarrow)
      {
         int nSize = wcslen(strWide);
         strNarrow.resize(nSize);
      //JG	WideCharToMultiByte(CP_ACP, 0, strWide, nSize, strNarrow.begin(), strNarrow.size(), NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, strWide, nSize, strNarrow.c_str(), strNarrow.size(), NULL, NULL);
      }

      void OSAL_API StringFromNarrow(String & strWide, const NString & strNarrow)
      {
         strWide.resize(strNarrow.size());
      //JG	MultiByteToWideChar(CP_ACP, 0, strNarrow.c_str(), strNarrow.size(), strWide.begin(), strWide.size());
            MultiByteToWideChar(CP_ACP, 0, strNarrow.c_str(), strNarrow.size(), strWide.c_str(), strWide.size());
      }

      void OSAL_API StringFromNarrow(String & strWide, const char * strNarrow)
      {
         int nSize = strlen(strNarrow);
         strWide.resize(nSize);
      //JG	MultiByteToWideChar(CP_ACP, 0, strNarrow, nSize, strWide.begin(), strWide.size());
            MultiByteToWideChar(CP_ACP, 0, strNarrow, nSize, strWide.c_str(), strWide.size());
      }

      void OSAL_API WideToNarrow(const wchar_t * strWide, NString & strNarrow)
      {
         int nSize = wcslen(strWide);
         strNarrow.resize(nSize);
      //JG	WideCharToMultiByte(CP_ACP, 0, strWide, nSize, strNarrow.begin(), strNarrow.size(), NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, strWide, nSize, strNarrow.c_str(), strNarrow.size(), NULL, NULL);
      }
      void OSAL_API WideToString(const wchar_t * strWide, String & strNarrow)
      {
         strNarrow = strWide;
      }
      */

      void OSAL_API StringToNarrow(const String & strWide, NString & strNarrow)
      {
         //Allocate a temp buff to convert to
         int BufSize = WideCharToMultiByte(CP_ACP, 0, strWide.c_str(), strWide.size(), NULL, 0, NULL, NULL);
         char *TempBuf = new char[BufSize+1];

         //JG Changed strNarrow.begin()
         WideCharToMultiByte(CP_ACP, 0, strWide.c_str(), strWide.size(), TempBuf, BufSize, NULL, NULL);
         TempBuf[BufSize]=NULL;
         strNarrow=TempBuf;
         delete [] TempBuf;
      }

      #ifdef UNICODE
         void OSAL_API StringToNarrow(const TCHAR * strWide, NString & strNarrow)
         {

            int nSize = wcslen(strWide);

            //Allocate a temp buff to convert to
            int BufSize  = WideCharToMultiByte(CP_ACP, 0, strWide, nSize, NULL, 0, NULL, NULL);
            char *TempBuf = new char[nSize+1];

            WideCharToMultiByte(CP_ACP, 0, strWide, nSize, TempBuf, BufSize, NULL, NULL);
            TempBuf[BufSize]=NULL;
            strNarrow=TempBuf;
            delete [] TempBuf;
         }
      #endif

      void OSAL_API StringFromNarrow(String & strWide, const NString & strNarrow)
      {

         //Allocate a temp buff to convert to
         int BufSize  = 	MultiByteToWideChar(CP_ACP, 0, strNarrow.c_str(), strNarrow.size(), NULL, 0);
         wchar_t *TempBuf = new wchar_t[BufSize+1];
         MultiByteToWideChar(CP_ACP, 0, strNarrow.c_str(), strNarrow.size(), TempBuf, BufSize);
         TempBuf[BufSize]=NULL;
         strWide=TempBuf;
         delete [] TempBuf;

      }

      void OSAL_API StringFromNarrow(String & strWide, const char * strNarrow)
      {
         int nSize = strlen(strNarrow);
         int BufSize  = 		MultiByteToWideChar(CP_ACP, 0, strNarrow, nSize, NULL, 0);
         wchar_t *TempBuf = new wchar_t[BufSize+1];

         MultiByteToWideChar(CP_ACP, 0, strNarrow, nSize, TempBuf, BufSize);
         TempBuf[BufSize]=NULL;
         strWide=TempBuf;
         delete [] TempBuf;
      }

      void OSAL_API WideToNarrow(const wchar_t * strWide, NString & strNarrow)
      {

         int nSize = wcslen(strWide);

         //Allocate a temp buff to convert to
         int BufSize  = WideCharToMultiByte(CP_ACP, 0, strWide, nSize, NULL, 0, NULL, NULL);
         char *TempBuf = new char[nSize+1];

         WideCharToMultiByte(CP_ACP, 0, strWide, nSize, TempBuf, BufSize, NULL, NULL);
         TempBuf[BufSize]=NULL;
         strNarrow=TempBuf;

         delete [] TempBuf;

      }

   #elif defined( __AAL_LINUX__ )

      #if !defined(WIDECHAR)

         void OSAL_API StringToNarrow(const String & strWide, NString & strNarrow)
         {
            strNarrow = strWide;
         }

         void OSAL_API StringToNarrow(const TCHAR * strWide, NString & strNarrow)
         {
            strNarrow = strWide;
         }

         void OSAL_API StringFromNarrow(String & strWide, const NString & strNarrow)
         {
            strWide = strNarrow;
         }

         void OSAL_API StringFromNarrow(String & strWide, const char * strNarrow)
         {
            strWide = strNarrow;
         }
         void OSAL_API WideToNarrow(const wchar_t * strWide, NString & strNarrow)
         {
            int nSize = wcslen(strWide);
            int nSizeBuffer = nSize+30;
            char * pBuffer = new char[nSizeBuffer];

            const wchar_t * pWide = strWide;

            mbstate_t         state;
            memset (&state, 0, sizeof (mbstate_t));

            wcsnrtombs(pBuffer, &pWide, nSize, nSizeBuffer-1, &state);
            pBuffer[nSize] = 0;
            strNarrow = pBuffer;
            delete []pBuffer;
         }
         void OSAL_API WideToString(const wchar_t * strWide, String & strNarrow)
         {
            WideToNarrow(strWide, strNarrow);
         }

      #else /* #if !defined(WIDECHAR), that is, WIDECHAR is defined */

         //#ifdef WIN32
         // Windows has a broken use_facet, so we use this macro to cover up the damage.
         //#define USE_FACET(TYPE, FACET) _USE (FACET, TYPE)
         //#else
         #define USE_FACET(TYPE, FACET) std::use_facet<TYPE> ((FACET))
         //#endif

         // We are using wide chars.  Do proper conversions.
         #include <locale>

         static inline void InitState (mbstate_t & state)
         // This is the official way to initialise a mbstate_t!!!
         {
            memset (&state, 0, sizeof (mbstate_t));
         }

         // The conversion facet that we use.
         typedef std::codecvt <wchar_t, char, mbstate_t> Converter;

         //+============================================================================
         // AL::StringFromNarrow (no error reporting).
         //
         // Convert a std::string to a std_string according to the current locale.
         //-============================================================================
         void OSAL_API WideToNarrow(const wchar_t * strWide, NString & strNarrow)
         {
         }

         void OSAL_API StringToNarrow(const TCHAR * strWide, NString & strNarrow)
         {
            try
            {
               int nSize = _tcslen(strWide);
               strNarrow.resize(nSize);

               mbstate_t         state;
               const Converter & cvt
                  = std::USE_FACET (Converter, std::locale::classic());

               TCHAR * from_next;
               char * to_next;
               InitState (state);

               cvt.out (state,
                  strWide, strWide + nSize, from_next,
                  &strNarrow[0], &strNarrow[0] + nSize, to_next);
            }
            catch (std::bad_alloc &) {
               strNarrow.erase();
            }
            catch (const std::exception &) {
               strNarrow.erase();
            }
         }
         void OSAL_API StringToNarrow(const String & strWide, NString & strNarrow)
         {
            StringToNarrow(strWide.c_str(), strNarrow);
         }

         void OSAL_API StringFromNarrow(String & strWide, const char * strNarrow)
         {
            try {
               int nSize = strlen(strNarrow);
               strWide.resize(nSize);

               mbstate_t         state;
               const Converter & cvt
                  = std::USE_FACET (Converter, std::locale::classic());

               char * from_next;
               TCHAR * to_next;
               InitState (state);

               cvt.in (state,
                  strNarrow, strNarrow + nSize, from_next,
                  &strWide[0], &strWide[0] + nSize, to_next);
            }
            catch (std::bad_alloc &) {
               strWide.erase();
            }
            catch (const std::exception &) {
               strWide.erase();
            }
         }

         void OSAL_API StringFromNarrow(String & strWide, const NString & strNarrow)
         {
            StringFromNarrow(strWide, strNarrow.c_str());
         }
         void OSAL_API WideToString(const wchar_t * strWide, String & strNarrow)
         {
            strNarrow = strWide;
         }

      #endif /* #if !defined(WIDECHAR) */

   #endif // OS

#endif // #if 0

END_NAMESPACE(AAL)

