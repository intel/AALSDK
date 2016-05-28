// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

ConsoleColorizer ConsoleColorizer::sm_Instance;
ConsoleColorizer & ConsoleColorizer::GetInstance()
{
   return ConsoleColorizer::sm_Instance;
}

bool ConsoleColorizer::HasColors(ConsoleColorizer::Stream s)
{
#if   defined( __AAL_WINDOWS__ )
   return _isatty((int)s) ? true : false;
#elif defined( __AAL_LINUX__ )
   return isatty((int)s) ? true : false;
#endif // OS
}

void ConsoleColorizer::Red(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT :

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Red;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR :
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Red;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Green(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT:

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Green;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR:
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Green;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Blue(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
#if defined( __AAL_WINDOWS__ )
      HANDLE                     h;
      CONSOLE_SCREEN_BUFFER_INFO buf_info;
#endif // __AAL_WINDOWS__

      switch ( s ) {
         case STD_COUT:

            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Blue;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStdoutAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#endif // OS
         break;

         case STD_CERR:
#if defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Blue;
#elif defined( __AAL_WINDOWS__ )
            h = GetStdHandle(STD_ERROR_HANDLE);
            GetConsoleScreenBufferInfo(h, &buf_info);
            m_OldStderrAttrs = buf_info.wAttributes;
            SetConsoleTextAttribute(h, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#endif // OS
         break;
      }
   }
}

void ConsoleColorizer::Reset(ConsoleColorizer::Stream s)
{
   if ( HasColors(s) ) {
      switch ( s ) {
         case STD_COUT:
            fflush(stdout);

#if   defined( __AAL_LINUX__ )
            std::cout << ConsoleColorizer::sm_Reset;
#elif defined( __AAL_WINDOWS__ )
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_OldStdoutAttrs);
#endif // OS

            fflush(stdout);
         break;

         case STD_CERR:
#if   defined( __AAL_LINUX__ )
            std::cerr << ConsoleColorizer::sm_Reset;
#elif defined( __AAL_WINDOWS__ )
            SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), m_OldStderrAttrs);
#endif // OS
         break;
      }
   }
}

ConsoleColorizer::ConsoleColorizer()
{
#if defined( __AAL_WINDOWS__ )
   CONSOLE_SCREEN_BUFFER_INFO bufinfo;

   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufinfo);
   m_OldStdoutAttrs = bufinfo.wAttributes;

   GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &bufinfo);
   m_OldStderrAttrs = bufinfo.wAttributes;
#endif // __AAL_WINDOWS__
}

#ifdef __AAL_LINUX__
const char ConsoleColorizer::sm_Red[]   = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char ConsoleColorizer::sm_Green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char ConsoleColorizer::sm_Blue[]  = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
const char ConsoleColorizer::sm_Reset[] = { 0x1b, '[', '0', 'm', 0 };
#endif // OS

