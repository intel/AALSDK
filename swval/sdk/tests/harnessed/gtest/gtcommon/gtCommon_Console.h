// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_CONSOLE_H__
#define __GTCOMMON_CONSOLE_H__

// Linux/Windows abstraction for changing the text foreground color of either stdout or
// stderr to red, green, blue, or resetting it to the default foreground color.
class GTCOMMON_API ConsoleColorizer
{
public:
   enum Stream
   {
      STD_COUT = 1,
      STD_CERR
   };

   static ConsoleColorizer & GetInstance();

   bool HasColors(Stream );

   void   Red(Stream );
   void Green(Stream );
   void  Blue(Stream );
   void Reset(Stream );

protected:
   ConsoleColorizer();

   static ConsoleColorizer sm_Instance;

#if   defined( __AAL_LINUX__ )
   static const char sm_Red[];
   static const char sm_Green[];
   static const char sm_Blue[];
   static const char sm_Reset[];
#elif defined( __AAL_WINDOWS__ )
   WORD m_OldStdoutAttrs;
   WORD m_OldStderrAttrs;
#endif // OS
};

#endif // __GTCOMMON_CONSOLE_H__

