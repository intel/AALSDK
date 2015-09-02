// Copyright (c) 2008-2015, Intel Corporation
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
/// @file AALLoggerExtern.h
/// @brief Provide extern declaration or static definition for a static
///        instanced of the Logger.
/// @ingroup Debugging
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///      AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/21/2008     HM       Initial version of AALLoggerExtern.h created
/// 09/28/2008     HM       #define AAL_LOGGER_INSTANCE to create a static global
/// 10/01/2008     HM       Instantiation of BeginningOfTime
/// 10/02/2008     HM       Set up fancy macros
/// 10/05/2008     HM       Added pointer option, since multi-use items will
///                            typically be using more than the singleton
/// 11/28/2008     HM       Fixed legal header
/// 12/10/2008     HM       Added LOG_VERBOSE level
/// 01/04/2009     HM       Updated Copyright
/// 03/22/2009     HM       Deprecated GetStartSecond() in favor of an internal
///                            Start of Time counter inside each logger.
/// 07/15/2009     HM       Allowed over-ride of AAL_LOG_LEVEL, which globally
///                            enables/disables logging levels at compile time.
///                         Also added DEBUG_BEYOND_LOGGER for logging when
///                            the logger cannot run (e.g. after program exit)
/// 10/21/2011     JG       Added code for Windows compatibility
///                            Changed macros to use ##VA_ARGS
/// 01/16/2012     JG       Added back sys/time.  Logger not functioning in
///                            Windows.
/// 04/10/2012     HM       Added AAL_LOGGER_LEVEL_KEYNAME/TYPE to allow copy
///                            of Log Level through an NVS-based Manifest@endverbatim
//***************************************************************************/
#ifndef __AALSDK_AALLOGGEREXTERN_H__
#define __AALSDK_AALLOGGEREXTERN_H__
#include <aalsdk/AALDefs.h>

/**
 * Use it by including it directly for an extern declaration of
 * theLogger, a CLogger instance, and a reference to it via the
 * ILogger interface, rLogger.
 *
 * Preface the #include with:
 *    #define AAL_LOGGER_INSTANCE
 * in a single compilation unit in order to declare the objects for
 * use globally through the objects linked to that compilation unit.
*/

/**
 * To enable Logging at compile-time, #define AAL_LOG_LEVEL to one of the
 * syslog levels, e.g. one of:
 *    LOG_EMERG   0  system is unusable
 *    LOG_ALERT   1  action must be taken immediately
 *    LOG_CRIT    2  critical conditions
 *    LOG_ERR     3  error conditions
 *    LOG_WARNING 4  warning conditions
 *    LOG_NOTICE  5  normal but significant condition
 *    LOG_INFO    6  informational
 *    LOG_DEBUG   7  debug-level messages
 *
 * All messages at the set level and all messages of a higher priority (lower
 *    numerical value) will have code generated that will allow them to be
 *    printed if the run-time level is sufficiently high.
 *
 * To turn all Logging OFF at compile-time, #undef AAL_LOG_LEVEL
 */

/*
 * The possible variations are provided below
 */

//#undef  AAL_LOG_LEVEL

//#define AAL_LOG_LEVEL LOG_EMERG      /* system is unusable */
//#define AAL_LOG_LEVEL LOG_ALERT      /* action must be taken immediately */
//#define AAL_LOG_LEVEL LOG_CRIT       /* critical conditions */
//#define AAL_LOG_LEVEL LOG_ERR        /* error conditions */
//#define AAL_LOG_LEVEL LOG_WARNING    /* warning conditions */
//#define AAL_LOG_LEVEL LOG_NOTICE     /* normal but significant condition */
//#define AAL_LOG_LEVEL LOG_INFO       /* informational */
//#define AAL_LOG_LEVEL LOG_DEBUG      /* debug-level messages */
#define LOG_VERBOSE 8                  /* for really verbose output */




// Defining AAL_LOG_LEVEL will globally remove ALL CODE AT COMPILE TIME
//    for logging at greater than that level. E.g. setting it to LOG_DEBUG
//    would remove at compile time all LOG_VERBOSE-level logging code.

// This is the current AAL Default compilation logging level
#ifndef AAL_LOG_LEVEL
# define AAL_LOG_LEVEL LOG_VERBOSE   /* really verbose debug-level messages */
#endif




// Enable DEBUG_BEYOND_LOGGER in order to debug events that occur when the Logger
// is not available, e.g. in the Assassin

//#define DEBUG_BEYOND_LOGGER

#ifdef DEBUG_BEYOND_LOGGER
# define DEBUG_CERR(...) std::cerr << "SHUTDOWN: " << ##__VA_ARGS__
#else
# define DEBUG_CERR(...) do{}while(0)
#endif // DEBUG_BEYOND_LOGGER





#include <aalsdk/AALLogger.h>       // See this for usage suggestions and bitmasks
                                    //    Note that it pre-includes AALIDDefs.h

#ifdef AAL_LOGGER_INSTANCE          /* Only defined in one file, currently CAALLogger.cpp */
   #include <aalsdk/CAALLogger.h>   // lots of includes :(
   AASLIB_API AAL::ILogger & theAALLogger()     // Static singleton of Logger
   {                                // Will only be instantiated if used
      static AAL::CLogger theCLogger;
      return theCLogger;
   }

   AASLIB_API AAL::ILogger * pAALLogger() // Another Static singleton of Logger
   {                                      // Will only be instantiated if used
      static AAL::CLogger anotherCLogger;
      return &anotherCLogger;
   }

//   __time_t GetStartSecond ()       // Record the second the Logger started
//   {
//      static __time_t BeginningOfTime;
//      if (!BeginningOfTime) {
//         struct timeval tv;
//         gettimeofday( &tv, NULL);
//         BeginningOfTime = tv.tv_sec;
//      }
//      return BeginningOfTime;
//   }  // End of GetStartSecond
#else
   extern AAL::ILogger & theAALLogger();
   extern AAL::ILogger * pAALLogger();
#endif // AAL_LOGGER_INSTANCE

   // Define macros for carrying Log Level in NVS as the default int
   #define AAL_LOGGER_LEVEL_KEYNAME   "AAL_LOGGER_LEVEL_KEYNAME"
   #define AAL_LOGGER_LEVEL_TYPE      btInt


#endif // __AALSDK_AALLOGGEREXTERN_H__
