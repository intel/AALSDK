// Copyright(c) 2008-2016, Intel Corporation
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
/// @file AALLogger.h
/// @brief Public Interface to Logger Facility
/// @ingroup Debugging
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///     AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/19/2008     HM       Initial version started
/// 10/02/2008     HM       Initial version finished
/// 10/05/2008     HM       Added pointer option, since multi-use items will
///                            typically be using more than the singleton
/// 11/04/2008     HM       Added LogMasks for every known subsystem
/// 11/04/2008     HM       Made logLevel an array to track the mask bits
/// 11/13/2009     HM       Added Note to user to include AALLoggerExtern.h
///                            instead of this file, in general
/// 01/03/2009     HM       Updated subsystem LM_* flags
/// 01/04/2009     HM       Updated Copyright
/// 03/22/2009     HM       Sped up AAL_##__VA_ARGS__X macros by caching the logger
///                            service pointer in the macro
/// 07/15/2009     HM       Added LM_Shutdown
/// 10/22/2009     AC       Added LM_ASM
/// 10/21/2011     JG       Added code for Windows compatibility
/// 01/16/2012     JG       Changed macros that used ##__VA_ARGS__ since in
///                           gcc 4.5.1 the preprocessor fails when passing
///                           arguments of the type "one" << "two" << "three"
///                           _VA_ARGS__ works but means that the variable
///                           argument must always be present in the macro.
///                           This is OK since the var arg is the message.
/// 04/22/2012     HM       Added LIBAAL_API to AAL::ILogger@endverbatim
//****************************************************************************

/// NOTE: Generally, you want to include AALLoggerExtern.h rather than this
///    file directly.

/**
 * USAGE:
 * There are a lot of options, so it is best to just construct it with the
 *    defaults, and set things from there.
 * The defaults are:
 *    Output to COUT - change with SetDestination
 *    No prepended string
 *    Accepts all bitmask values
 *    Accept syslog level LOG_WARNING and above
 *    Do not prepend PID
 *    Do not prepend timestamp
 *
 * Because syslog() needs to be set with PID option,
 *    then IF one is going to set the destination to syslog(),
 *    it will be most efficient to call SetLogPID()
 *    BEFORE calling SetDestination(SYSLOG)
 *
 * Note: The Prepend String is not passed to syslog as it really wants a truly
 *    constant string (e.g. a literal that will be available for the rest of the
 *    life of the program), and the library cannot guarantee that, so prepending
 *    is done brute force in the Logger.
 *
 * Note: if the program is a daemon, one would not want to SetDestination
 *    to either COUT or CERR. Use SYSLOG or FILE in this case.
 *
 * The values of the syslog levels are: (from sys/syslog.h)
 *    #define  LOG_EMERG   0  // system is unusable
 *    #define  LOG_ALERT   1  // action must be taken immediately
 *    #define  LOG_CRIT    2  // critical conditions
 *    #define  LOG_ERR     3  // error conditions
 *    #define  LOG_WARNING 4  // warning conditions
 *    #define  LOG_NOTICE  5  // normal but significant condition
 *    #define  LOG_INFO    6  // informational
 *    #define  LOG_DEBUG   7  // debug-level messages
 *
 * LogMasks for various AAL subsystems are defined below. Use other masks
 *    for applications.
 */

/* ---------------  INTERNAL NOTES ON DESIGN --------------------------------
 * Requirements:
 * MUST:
 *    Provide run-time selection of log entries
 *    Provide as light-weight (time and space) a process for run-time selection
 *       of log entries as reasonable
 *    Log entry selection based on a capability bitmask (e.g. trace, error,
 *       sub-system-specific, etc.) that is set by the client [the client in
 *       this case is the framework itself, although additional bits could
 *       be used by the framework's client]
 *    Log entry selection compatible with system logging functions as defined
 *       in errno.h, e.g. LOG_ERROR, LOG_WARNING levels supported
 *    Provide the ability to log process/thread ID
 *    Provide the ability to time-stamp
 *    Provide the ability to define output file or facility (e.g. cout, cerr,
 *       a file, or syslog())
 *    Assuming multiple threads calling the logger at the same time, reduce
 *       the resultant interleaving of the output to a minimum. Preferably, no lines
 *       will be intermixed.
 *
 * SHOULD:
 *    Provide compile-time removal of log entries (e.g. via a macro or
 *       code that compiles to nothing when the removal flag is turned on
 *    Provide extensibility hooks so that output can be filtered or reformatted
 *       by a user-provided routine
 *    Be dynamically loaded so that an implementation can be swapped out trivially
 *       (Questions: speed of access relevant? multiple instantiations cause difficulty
 *           with interleaved log output? Ease of use?)
 *
 * ASSUMPTIONS:
 *    Can efficiently use 64-bit quantities
 *
 * USE CASES:
 * 1) Tracing - put a trace on each routine as it is called and exits
 *    Typically this is DEBUG level information. When debugging, however, one wants
 *    trace information only sometimes.
 *    Two ways to approach this: AND and OR
 *    AND: Only log when the client has set the logger to accept the trace bitmask
 *       AND the logger error level is LOG_DEBUG
 *    OR: Log traces whenever the logger has been set to trace OR is at DEBUG level.
 * ---In this case, the AND case seems preferable
 *
 * SPEED AND FLEXIBILITY TRADEOFFS:
 * 1) The client code needs a compact and rapid way to test whether or not to proceed.
 *    It should go to all the trouble of formatting a string, retrieving a time-stamp
 *    and process id, and sending all of this to the logger only to have the logger
 *    drop it all on the floor because it is only interested in certain things.
 *    Rather, the code should be structured so that there is a small fast test done
 *    up front to determine whether or not to continue. This is then wrapped up
 *    into a macro or other easy to use client code. Two ways to do this:
 *    TEST IN CLIENT CODE:
 *       Perform the test right in the client code. Larger code, likely faster,
 *       especially if logger is virtual. Most time-consuming part is fetching the
 *       global logger settings from memory.
 *    TEST IN LOGGER CODE:
 *       Provide a function for the client to call, passing the relevant test info,
 *       the returns btBool. Fairly small - load parameter is load constant into register
 *       followed by a call and a test of RAX. Still have to fetch global logger
 *       settings from memory, plus potentially have to vector through a vtbl.
 *    IMPLEMENTED METHOD:
 *       Make a call into the logger. Make that function tight. If the client desires,
 *       that function can be inlined (with caveats), providing a build-time switch
 *       between the two alternatives.
 *
 * DESIGN:
 *    LOG ENTRY SELECTION:
 *       SINGLE BITMASK IMPLEMENTATION:
 *       Basing log entry selection on a bitmask allows it to proceed EITHER on
 *          the classic levels defined in errno.h, or via an applied bitmask.
 *          The classic levels of errno are mapped onto the bitmask as individual
 *          bits (e.g. LOG_EMERG is 0x01, LOG_ALERT is 0x02, etc).
 *       The idea is to associate a bitmask with the entry itself, and then set
 *          a logger bitmask. AND'ing them together tells the logger whether or
 *          not to print the string.
 *       The tricky part is that the classic errno values are compared against, not
 *          bitmasked, so if the global value is set to LOG_CRIT, then all messages
 *          of that priority or higher (e.g. LOG_ALERT, LOG_EMERG) are also included.
 *          To emulate this mechanism using bitmasks, the logger bitmask is set up
 *          for the errno flags by OR'ing together a given level's flag with all
 *          those higher than it. So, the global ALLOW_LOG_CRIT flag would consist
 *          of the bits not just for LOG_CRIT, but also for LOG_ALERT and LOG_EMERG.
 *       If the bitmask is used straightforwardly, then the client can get OR
 *          functionality.
 *       If the bitmask is first cleaved into two sections, each tested, and the
 *          result AND'ed, then the client can get OR functionality.
 *       DUAL VALUE IMPLEMENTATION:
 *       Just pass a mask for selection and a value for errno level. Generally both
 *          are passed in registers, which will be loaded from constants. Two loads
 *          versus one for the single bitmask implementation, but no field-splitting.
 */
#ifndef __AALSDK_AALLOGGER_H__
#define __AALSDK_AALLOGGER_H__
#include <aalsdk/AALTypes.h> // for LogMasks and AASLIB_API

#ifdef __AAL_LINUX__
# include <syslog.h>
#else
# define LOG_EMERG   0	
# define LOG_ALERT   1	
# define LOG_CRIT    2	
# define LOG_ERR     3
# define LOG_WARNING 4
# define LOG_NOTICE  5
# define LOG_INFO    6	
# define LOG_DEBUG   7
#endif // __AAL_LINUX__


/*
 * Use these macros for your output
 */

// Typical usage
// AAL_LOG(LOG_DEBUG,LM_ResMgr,"9999" << argv[0] << endl)
// where: LOG_##__VA_ARGS__X is the debug level of this message, from syslog.h
//        LM_##__VA_ARGS__xx is the MASK associated with this subsystem (global
//           ones defined below. Add your own as needed.
//        And the rest of it is standard stuff like you would send to COUT
//           e.g. cout << "9999" << argv[0] << endl
//           Except that the 'cout <<' is already there ...
//
// Even Better: for the above example, use:
// AAL_DEBUG(LM_ResMgr,"9999" << argv[0] << endl)
//
// If instead of using the AAL global, you want to use a pointer from the
//    ILoggerFactory, use one of the forms:
// MY_LOG(myLogger(),LOG_DEBUG,LM_ResMgr,"9999" << argv[0] << endl)
//    or:
// MY_DEBUG(myLogger(),LM_ResMgr,"9999" << argv[0] << endl)

#if defined( AAL_LOG_LEVEL )                    /* top-level compile-time switch */
                                                // set it to a SYSLOG level
/* Generic Call through a Logger pointer */
#  define AAL_ANY_LOGP(LOGP,ERRLVL,MASK,...) do { \
      AAL::ILogger* __logptr=(LOGP); \
      if (__logptr->IfLog( (MASK), (ERRLVL))) { \
         __logptr->Log( (ERRLVL), __logptr->GetOss((ERRLVL)) << __VA_ARGS__); \
      } } while (0)

/* original version. new version caches the (LOGP). A bit faster.
#  define AAL_ANY_LOGP(LOGP,ERRLVL,MASK,...) do { \
      if ((LOGP)->IfLog( (MASK), (ERRLVL))) { \
         (LOGP)->Log( (ERRLVL), (LOGP)->GetOss((ERRLVL)) << ##__VA_ARGS__); \
      } } while (0)
*/

/* Generic Call to a Logger reference */
#  define AAL_ANY_LOGR(LOGR, ERRLVL, MASK, ...) do { \
      AAL::ILogger& __logref=(LOGP); \
      if (__logref.IfLog( (MASK), (ERRLVL))) { \
         __logref.Log((ERRLVL), __logref.GetOss((ERRLVL)) << __VA_ARGS__); \
      } } while (0)

// AAL-specific version, uses AAL static global pointer
#  define AAL_LOG(ERRLVL, MASK, ...) AAL_ANY_LOGP(pAALLogger(), (ERRLVL), (MASK), __VA_ARGS__)

// Generic version using pointer - default
#  define MY_LOG(LOGP, ERRLVL, MASK, ...) AAL_ANY_LOGP((LOGP), (ERRLVL), (MASK), __VA_ARGS__)

// Generic version using reference
#  define MY_LOGR(LOGR, ERRLVL, MASK, ...) AAL_ANY_LOGR((LOGR), (ERRLVL), (MASK), __VA_ARGS__)

#else
# define AAL_ANY_LOGP(LOGP, ERRLVL, MASK, ...) do{}while(0)
# define AAL_ANY_LOGR(LOGR, ERRLVL, MASK, ...) do{}while(0)
# define AAL_LOG(ERRLVL, MASK, ...)            do{}while(0)
# define MY_LOGR(LOGR, ERRLVL, MASK, ...)      do{}while(0)
# define MY_LOGP(LOGP, ERRLVL, MASK, ...)      do{}while(0)
#endif // #if defined(AAL_LOG_LEVEL)


#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_EMERG)
#  define AAL_EMERG(mask,...)     AAL_LOG(LOG_EMERG,(mask),__VA_ARGS__)
#  define MY_EMERG(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_EMERG,(mask),__VA_ARGS__)
#else
#  define AAL_EMERG(mask,...) do { } while (0)
#  define MY_EMERG(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_EMERG)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_ALERT)
#  define AAL_ALERT(mask,...)     AAL_LOG(LOG_ALERT,(mask),__VA_ARGS__)
#  define MY_ALERT(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_ALERT,(mask),__VA_ARGS__)
#else
#  define AAL_ALERT(mask,...) do { } while (0)
#  define MY_ALERT(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_ALERT)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_CRIT)
#  define AAL_CRIT(mask,...)     AAL_LOG(LOG_CRIT,(mask),__VA_ARGS__)
#  define MY_CRIT(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_CRIT,(mask),__VA_ARGS__)
#else
#  define AAL_CRIT(mask,...) do { } while (0)
#  define MY_CRIT(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_CRIT)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_ERR)
#  define AAL_ERR(mask,...)     AAL_LOG(LOG_ERR,(mask),__VA_ARGS__)
#  define MY_ERR(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_ERR,(mask),__VA_ARGS__)
#else
#  define AAL_ERR(mask,...) do { } while (0)
#  define MY_ERR(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_ERR)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_WARNING)
#  define AAL_WARNING(mask,...)     AAL_LOG(LOG_WARNING,(mask),__VA_ARGS__)
#  define MY_WARNING(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_WARNING,(mask),__VA_ARGS__)
#else
#  define AAL_WARNING(mask,...) do { } while (0)
#  define MY_WARNING(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_WARNING)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_NOTICE)
#  define AAL_NOTICE(mask,...)     AAL_LOG(LOG_NOTICE,(mask),__VA_ARGS__)
#  define MY_NOTICE(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_NOTICE,(mask),__VA_ARGS__)
#else
#  define AAL_NOTICE(mask,...) do { } while (0)
#  define MY_NOTICE(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_NOTICE)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_INFO)
#  define AAL_INFO(mask,...)     AAL_LOG(LOG_INFO,(mask),__VA_ARGS__)
#  define MY_INFO(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_INFO,(mask),__VA_ARGS__)
#else
#  define AAL_INFO(mask,...) do { } while (0)
#  define MY_INFO(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_INFO)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_DEBUG)
#  define AAL_DEBUG(mask,...)     AAL_LOG(LOG_DEBUG,(mask),__VA_ARGS__)
#  define MY_DEBUG(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_DEBUG,(mask),__VA_ARGS__)
#else
#  define AAL_DEBUG(mask,...) do { } while (0)
#  define MY_DEBUG(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_DEBUG)

#if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_VERBOSE)
#  define AAL_VERBOSE(mask,...)     AAL_LOG(LOG_VERBOSE,(mask),__VA_ARGS__)
#  define MY_VERBOSE(LOGP,mask,...) AAL_ANY_LOGP((LOGP),LOG_VERBOSE,(mask),__VA_ARGS__)
#else
#  define AAL_VERBOSE(mask,...) do { } while (0)
#  define MY_VERBOSE(LOGP,mask,...) do { } while (0)
#endif // #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_VERBOSE)


/*
 * Define the Logger parts
 */

BEGIN_NAMESPACE(AAL)

   typedef btUnsigned64bitInt LogMask_t;        // 64-bits - capability mask used by the client

   const LogMask_t LM_None          = (LogMask_t)0;        // LM_All selects no bits in the bitmask
   const LogMask_t LM_All           = (LogMask_t)-1;       // LM_All selects all bits in the bitmask
   const LogMask_t LM_Any           = (LogMask_t)1 << AAL_sysAny;
   const LogMask_t LM_AAL           = (LogMask_t)1 << AAL_sysAAL;
   const LogMask_t LM_AAS           = (LogMask_t)1 << AAL_sysAAS;
   const LogMask_t LM_AIA           = (LogMask_t)1 << AAL_sysAIA;
   const LogMask_t LM_AFUFactory    = (LogMask_t)1 << AAL_sysAFUFactory;
   const LogMask_t LM_AFU           = (LogMask_t)1 << AAL_sysAFU;
   const LogMask_t LM_Registrar     = (LogMask_t)1 << AAL_sysRegistrar;
   const LogMask_t LM_Factory       = (LogMask_t)1 << AAL_sysFactory;
   const LogMask_t LM_Database      = (LogMask_t)1 << AAL_sysDatabase;
   const LogMask_t LM_EDS           = (LogMask_t)1 << AAL_sysEDS;
   const LogMask_t LM_ResMgr        = (LogMask_t)1 << AAL_sysResMgr;
   const LogMask_t LM_ResMgrClient  = (LogMask_t)1 << AAL_sysResMgrClient;
   const LogMask_t LM_ManagementAFU = (LogMask_t)1 << AAL_sysManagementAFU;
   const LogMask_t LM_UAIA          = (LogMask_t)1 << AAL_sysUAIA;
   const LogMask_t LM_Shutdown      = (LogMask_t)1 << AAL_sysShutdown;

   //=============================================================================
   // Name: ILogger
   // @brief Main logger interface
   //=============================================================================
   class AASLIB_API ILogger
   {
   public:
      /// @brief Sets the output destination of the logger
      enum eLogTo {
         COUT,
         CERR,
         SYSLOG,
         FILE
      };
      /// @brief Ctor of implicit, Dtor of standard for abstract interface class
      virtual            ~ILogger();

      /// @brief Asks whether the log should be executed at all
      virtual btBool      IfLog              (LogMask_t mask, btInt errlevel) const = 0;

      /// @brief Gets an ostringstream to write to
      virtual std::ostringstream& GetOss     (btInt errLevel=-1) = 0;

      /// @brief Returns the perror string for an errno defined in errno.h
      ///    or a string version of the error and message that it was not found
      virtual char*       GetErrorString     (btInt errNum) = 0;

      /// @brief Log a string
      virtual void        Log                (btInt errlevel, btcString psz) = 0;
      virtual void        Log                (btInt errlevel, std::ostringstream &ros) = 0;
      virtual void        Log                (btInt errlevel, std::basic_ostream<char, std::char_traits<char> >& ros) = 0;

      /// @brief if eDest is FILE, there better be a no-null filename
      virtual void        SetDestination     (eLogTo eDest=COUT, std::string sFile="") = 0;
      virtual eLogTo      GetDestinationEnum () const = 0;
      virtual std::string GetDestinationFile () const = 0;

      /// @brief Sets the mask that determines what is logged, and for each bit in it the threshold of what is to be logged.
      virtual void        AddToMask          (LogMask_t mask, btInt errLevel) = 0;
      virtual void        RemoveFromMask     (LogMask_t mask) = 0;
      virtual LogMask_t   GetMask            () const = 0;

      /// @brief Gets the threshold level associated with a particular bit in the mask.
      virtual int         GetLevel           (LogMask_t mask) const = 0;

      /// @brief Tells the Logger to prepend the PID of the caller (for multi-threaded applications)
      virtual void        SetLogPID          (btBool fLogPID) = 0;
      virtual btBool      GetLogPID          () const = 0;

      /// @brief Tells the Logger to prepend a wall-clock timestamp
      virtual void        SetLogTimeStamp    (btBool fLogTimeStamp) = 0;
      virtual btBool      GetLogTimeStamp    () const = 0;

      /// @brief Tells the Logger to always prepend a string
      virtual void        SetLogPrepend      (std::string sPrepend) = 0;
      virtual std::string GetLogPrepend      () const = 0;

      /// @brief Tells the Logger whether to prepend the error Level
      virtual void        SetLogErrorLevelPrepend (btBool errLevel) = 0;
      virtual btBool      GetLogErrorLevelPrepend () const = 0;

      /// @brief Tells the Logger whether to flush on EVERY full write
      virtual void        SetFlush (btBool flush) = 0;
      virtual btBool      GetFlush () const = 0;

   }; // class ILogger

   // AAL scope, get a heap-allocated instance of an object that implements ILogger
   AASLIB_API ILogger * ILoggerFactory();

   //#define AALOUT(level,mask,...)

END_NAMESPACE(AAL)

#endif // __AALSDK_AALLOGGER_H__

