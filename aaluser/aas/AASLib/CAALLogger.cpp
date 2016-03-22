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
/// @file CAALLogger.cpp
/// @brief Contains CLogger, PIDossMap, and ossRec implementations.
/// @ingroup Debugging
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Alvin Chen, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/19/2008     HM       Initial version started
/// 10/02/2008     HM       Initial version of Logger implementation completed
/// 11/04/2008     HM       Made logLevel an array to track the mask bits
/// 01/04/2009     HM       Updated Copyright
/// 01/07/2009     HM       Added ThreadID when printing Pid
/// 02/08/2009     HM       Modified SetDestination to print at the current
///                            LogLevel. Logic is a little funky; it will log
///                            at the level of the high-bit of the current
///                            mask. That will typically be what is set by
///                            the application or most recent subsystem, so
///                            should generally work well. No back door at
///                            this time, but easy to add if needed later.
/// 02/20/2009     HM       Set ThreadID width to 10 for uniformity
/// 02/22/2009     HM       Set max debug level sent to SysLog to LOG_DEBUG in
///                            CLogger::Log(int errLevel, std::ostringstream& ross)
///                            because that is the highest SysLog will accept
/// 02/22/2009     HM       Converted C headers to C++ headers
/// 03/22/2009     HM       Logger now sets Start Of Time to be when it is
///                            constructed, to the microsecond. Will show more
///                            consistent results across runs.
/// 04/03/2009     HM       Fixed bug in code added 3/22/2009
/// 09/07/2009     AC       Added the auto flush feature
/// 10/21/2011     JG       Added code for Windows compatibility
/// 04/22/2012     HM       Added usage of new OSAL functionality for ffsll()
///                            written for Windows: FindLowestBitSet64()
/// 04/25/2012     HM       Forced new OSAL functionality for Linux bitscan
/// 04/30/2012     HM       Default bitmask to ANY (LM_Any) instead of NONE so
///                            even if not initialized all WARNINGs will print.
/// 06/13/2012     HM       Move prepend string BEFORE the Level Notice@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"
#define AAL_LOGGER_INSTANCE
#include "aalsdk/AALLoggerExtern.h"    // Includes AALLogger.h, AALIDDefs.h
                                       // <sys/time.h>
#include "aalsdk/CAALLogger.h"         // Includes <AALLogger.h>, <syslog.h>,
                                       // <string>, <sstream>, <AALIDDefs.h>,
                                       // <fstream>, <map>, <aas/CriticalSection.h>

#include "aalsdk/utils/Utilities.h"    // NUM_ELEMENTS()
#include "aalsdk/OSAL.h"               // GetThreadID(), FindLowestBitSet64()


BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name:          ILoggerFactory
// Description:   Create and return an instance of an object that implements
//                   an ILogger
//=============================================================================
ILogger * ILoggerFactory()
{
   return new CLogger();
}

ILogger::~ILogger() {}

//=============================================================================
// Name:          ossRec::ossRec
// Description:   Default Constructor
// Comments:      DO NOT initialize the m_poss here with an allocated object.
//                See destructor for why.
// NOTE:          Real constructor is in PIDossMap::GetOss
//=============================================================================
ossRec::ossRec() :
   m_poss(NULL)/*, m_bInUse(false)*/
{
// UNCOMMENT THESE LINES FOR DEBUGGING
//   std::cerr << "LOG_DEBUG:ossRec::Constructor &object=" << static_cast<void*>(this)
//             << " &oss = " << static_cast<void*>(m_poss) << std::endl;
}

//=============================================================================
// Name:          ossRec::~ossRec
// Description:   Default Destructor
// Comments:      DO NOT clean up the m_poss here. These things are created and
//                   deleted many times by the map, and if you eat the pointer
//                   here it will be used from elsewhere. Indeed, this dtor
//                   is called without the ctor being called when it is inserted
//                   into the map (YEAH, WIERD)
//=============================================================================
ossRec::~ossRec()
{
// UNCOMMENT THESE LINES FOR DEBUGGING
//   std::cerr << "LOG_DEBUG:ossRec::Destructor  &object=" << static_cast<void*>(this)
//             << " &oss = " << static_cast<void*>(m_poss) << std::endl;
}

//=============================================================================
// Name:          ossRec::Destroy
// Description:   Real Destructor
// Comments:      See default destructor for why it cannot be used. So instead
//                   do the cleanup here.
//=============================================================================
void ossRec::Destroy()
{
   if ( NULL != m_poss ) {
      delete m_poss;
      m_poss = NULL;
   }
   if ( NULL != m_pszTemp ) {
      delete[] m_pszTemp;
      m_pszTemp = NULL;
   }
   // DO NOT then do "delete this" to remove it from memory. That will be done
   // by clearing the map.
}

//=============================================================================
// Name:          PIDossMap::PIDossMap
// Description:   Default Constructor
// Comments:      Just instantiate the ossMap with nothing in it.
//=============================================================================
PIDossMap::PIDossMap() :
   m_ossMap()
{}

//=============================================================================
// Name:          PIDossMap::~PIDossMap
// Description:   Destructor
//=============================================================================
PIDossMap::~PIDossMap()
{
   ossMap_itr_t itr;
   // iterate through map, deleting the ostreamstring entries of the ossRec's
   for ( itr = m_ossMap.begin() ; itr != m_ossMap.end() ; ++itr ) {
      (*itr).second.Destroy();      // Note this does not remove the element, merely cleans it
   }
   // then clear the map
   m_ossMap.clear();
} // PIDossMap::~PIDossMap

//=============================================================================
// Name:          PIDossMap::GetOss
// Description:   Given an index (ThreadID), retrieve the associated ostringstream,
//                   or add one if a new ThreadID.
// Returns:       The associated ostringstream*, or NULL if it is not there
//                   or cannot be created
// NOTE:          Callers need to be locked.
//=============================================================================
std::ostringstream * PIDossMap::GetOss(int index)
{
//   this does not seem to work correctly
//   ossRec& rossRec = m_ossMap[index];      // Retrieve existing or create new

   ossMap_itr_t itr = m_ossMap.find(index);
   if ( itr != m_ossMap.end() ) {              // Found it
      return (*itr).second.GetOss();         // retrieve ostringstream*
   } else {
      ossRec ossRecNil;                                  // copy this into the map
      std::pair <ossMap_itr_t, btBool> insertRetVal;     // for call to insert

      std::ostringstream *poss = new std::ostringstream;
      ASSERT(NULL != poss);
      if ( NULL == poss ) {
         std::cerr << "PIDossMap::GetOss unable to allocate an ostringstream\n";
         return NULL;
      }

      char *pszTemp = new char[TempStringLength];
      ASSERT(NULL != pszTemp);
      if ( NULL == pszTemp ) {
         std::cerr << "PIDossMap::GetOss unable to allocate a string\n";
         delete poss;
         return NULL;
      }

      insertRetVal = m_ossMap.insert(std::pair<int, ossRec>(index, ossRecNil));

      // if successful insertion, return pointer to oss
      if ( insertRetVal.second ) {
         ossRec &rossRec = (*insertRetVal.first).second; // get the ossRec inserted
         rossRec.SetOss(poss);                           // set the pointer to the new oss
         rossRec.Setpsz(pszTemp);                        // set the pointer to the new string
         return poss;
         // Parse: insertRetVal.first is the iterator that is returned
         //        *insertRetVal.first is the map pair being pointed to
         //        (*insertRetVal.first).first is the key
         //        (*insertRetVal.first).second is the ossRec
         //        (*insertRetVal.first).second.m_poss is the pointer to the ostringstream inside the ossRec
         //        &((*insertRetVal.first).second.m_poss) is the address of the pointer ...
      } else {
         std::cerr << "PIDossMap::GetOss unable to add to map\n";
         delete[] pszTemp;
         delete   poss;
         return NULL;
      }
   }
}  // PIDossMap::GetOss

//=============================================================================
// Name:          PIDossMap::Getpsz
// Description:   Given an index (ThreadID), retrieve the associated C-string
//                   or add one if a new ThreadID.
// Returns:       The associated char*, or NULL if it is not there
//                   or cannot be created
// NOTE:          Callers need to be locked.
//=============================================================================
char * PIDossMap::Getpsz(int index)
{
   ossMap_itr_t itr = m_ossMap.find(index);
   if ( itr != m_ossMap.end() ) {            // Found it
      return (*itr).second.Getpsz();         // retrieve char*
   } else {                                  // have to create it, use GetOss
      std::ostringstream *poss = GetOss(index);
      if ( NULL == poss ) {                  // could not create it, fail
         return NULL;
      }
      return Getpsz(index);                  // now created, try it again
   }
}  // PIDossMap::Getpsz

//=============================================================================
// Name:          CLogger::CLogger
// Description:   Ctor
//=============================================================================
CLogger::CLogger() :
   CriticalSection(),
   m_eDest(COUT),
   m_sFile(),
   m_LogMask(LM_Any),
   m_bLogPID(true),
   m_bTimeStamp(true),
   m_bErrorLevel(true),
   m_bFlush(false),
   m_ofstream(),
   m_sPrepend(),
   m_PIDossMap(),
   m_oss(),
#ifdef __AAL_LINUX__
   m_tvZero(),
#endif // __AAL_LINUX__
   m_pFlushThread(NULL),
   m_bExitFlushThread(false),
   m_bFlushThreadIsExiting(false),
   m_needFlush(false),
   m_autoFlushTime(1) // Default auto flush time is 1 second
{
   //Autolock(this); //compiler says this is out of scope, need to investigate?
#ifdef __AAL_LINUX__
   // Get the beginning of time
   gettimeofday(&m_tvZero, NULL);
#endif // __AAL_LINUX__

   int var;
   for ( var = 0 ; var < m_numElementsInLogLevel ; ++var ) {
      m_rgLogLevel[var] = LOG_WARNING;
   }
//   memset(m_rgLogLevel, LOG_WARNING, sizeof(m_rgLogLevel));
} // CLogger::CLogger

//=============================================================================
// Name:          CLogger::~CLogger
// Description:   Dtor
// Comment:       Need to clean up the open file if there is one
//                The PIDossMap is cleaned up automatically as it is a member
//=============================================================================
CLogger::~CLogger()
{
   AutoLock(this);

   StopFlushThread();

   if ( FILE == m_eDest ) {
      m_ofstream.flush();
      m_ofstream.close();
      m_sFile.clear();
   }

   if ( SYSLOG == m_eDest ) {
#ifdef __AAL_LINUX__
      closelog();
#endif // __AAL_LINUX__
   }

} // CLogger::~CLogger


//=============================================================================
// Name:          CLogger::IfLog
// Description:   Ask whether the log should be executed at all
// Comment:       Need speed here, may someday be inlined
//=============================================================================
btBool CLogger::IfLog(LogMask_t mask, int errlevel) const
{
   /*
    * Don't lock. If another thread tweaks the level and masks while this is
    * executing, the worst that will happen is that a line may not get tested
    * with the right level and mask. That could be unpleasant, but this function
    * must be fast, and setting a lock seems unnecessary
    */
   // AutoLock(this);

   return ( mask & m_LogMask) && (errlevel <= m_rgLogLevel[FindLowestBitSet64(mask)]);
} // CLogger::IfLog

//=============================================================================
// Name:          CLogger::GetOss
// Description:   Return an ostringstream for the client to write upon
// Comment:       Based on the threads PID
//=============================================================================
std::ostringstream & CLogger::GetOss(int errLevel)
{
   AutoLock(this);                     // manipulating the map
   std::ostringstream *poss = m_PIDossMap.GetOss((int)GetThreadID());
   if ( NULL == poss ) {
      poss = &m_oss;                   // backup string if something goes wrong
   }

   PreloadOss(poss, errLevel);         // Preload standard stuff into the stream

   return *poss;
} // CLogger::GetOss

//=============================================================================
// Name:          CLogger::Getpsz
// Description:   Return a char* for the client to write upon
// Comment:       Based on the threads PID
// NOTE:          Will never fail. Return ptr will always be valid.
//                   Buffer Length is TempStringLength
//=============================================================================
char * CLogger::Getpsz()
{
   AutoLock(this);                     // manipulating the map
   char *psz = m_PIDossMap.Getpsz((int)GetThreadID());
   if ( NULL == psz ) {
      psz = m_szTemp;                 // backup string if something goes wrong
   }
   return psz;
} // CLogger::Getpsz

//=============================================================================
// Name:          CLogger::GetErrorString
// Description:   Return a char* for the client to write upon
// Comment:       Return the perror string for an errno defined in errno.h
//                   or a string version of the error and a message that it
//                   was not found
//=============================================================================
char * CLogger::GetErrorString(int errNum)
{
   AutoLock(this);                     // manipulating the map
#ifdef __AAL_LINUX__
   return strerror_r(errNum, Getpsz(), TempStringLength);
#else
   return NULL;
#endif // OS
} // CLogger::GetErrorString

//=============================================================================
// Name:          CLogger::PreloadOss
// Description:   Preload an ostringstream with decorations
//=============================================================================
void CLogger::PreloadOss(std::ostringstream *poss, int errLevel)
{
   // should not need to lock, called internally

   // retain flag state
   // flags gets/sets entire field, setf tweaks individual
   // fmtflags: for setf
   //       basefield   hex, dec
   //       adjustfield left, right
   // width
   // fillchar

   static std::ios::fmtflags defaultFlags;
   static char               defaultFillChar;
#ifdef __AAL_LINUX__
   static std::ios::fmtflags PIDFlags;
   static std::ios::fmtflags TSFlags;
#endif // __AAL_LINUX__
   static btBool             Initialized = false;

   // defined in syslog.h
   const std::string LevelToStringMap[] = {           // map syslog levels 0-7 to strings
      "EMERG ",   /* system is unusable */
      "ALERT ",   /* action must be taken immediately */
      "CRITL ",   /* critical conditions */
      "ERROR ",   /* error conditions */
      "WARNG ",   /* warning conditions */
      "NOTIC ",   /* normal but significant condition */
      "INFO  ",   /* informational */
      "DEBUG ",   /* debug-level messages */
      "VBOSE "
   };

   // A combined set with the higher ones being 5, lower 3, to emphasize the higher ones
   //   "EMERG ",   /* system is unusable */
   //   "ALERT ",   /* action must be taken immediately */
   //   "CRITL ",   /* critical conditions */
   //   "ERROR ",   /* error conditions */
   //   "WRN ",   /* warning conditions */
   //   "NOT ",   /* normal but significant condition */
   //   "INF ",   /* informational */
   //   "DBG "    /* debug-level messages */

   // A set of 5 char wide variants
   //   "EMERG ",   /* system is unusable */
   //   "ALERT ",   /* action must be taken immediately */
   //   "CRITL ",   /* critical conditions */
   //   "ERROR ",   /* error conditions */
   //   "WARNG ",   /* warning conditions */
   //   "NOTIC ",   /* normal but significant condition */
   //   "INFOR ",   /* informational */
   //   "DEBUG "    /* debug-level messages */

   // A set of 4 char wide variants
   //   "EMRG ",   /* system is unusable */
   //   "ALRT ",   /* action must be taken immediately */
   //   "CRIT ",   /* critical conditions */
   //   "ERRR ",   /* error conditions */
   //   "WARN ",   /* warning conditions */
   //   "NOTE ",   /* normal but significant condition */
   //   "INFO ",   /* informational */
   //   "DBUG "    /* debug-level messages */

   // A set of 3 char wide variants
   //   "EMG ",   /* system is unusable */
   //   "ALT ",   /* action must be taken immediately */
   //   "CRT ",   /* critical conditions */
   //   "ERR ",   /* error conditions */
   //   "WRN ",   /* warning conditions */
   //   "NOT ",   /* normal but significant condition */
   //   "INF ",   /* informational */
   //   "DBG "    /* debug-level messages */

   // Only executed once
   if ( !Initialized ) {
      // get default flags
      Initialized     = true;
      defaultFlags    = poss->flags();
      defaultFillChar = poss->fill();

      // get PID flags
      poss->setf(std::ios::right, std::ios::adjustfield);
      poss->setf(std::ios::dec, std::ios::basefield);
      poss->setf(std::ios::showbase);
#ifdef __AAL_LINUX__
      PIDFlags = poss->flags();
#endif // __AAL_LINUX__
      poss->flags(defaultFlags); // reset

      // get TimeStamp flags
      poss->setf(std::ios::right, std::ios::adjustfield);
      poss->setf(std::ios::dec, std::ios::basefield);
#ifdef __AAL_LINUX__
      TSFlags = poss->flags();
#endif // __AAL_LINUX__
      poss->flags(defaultFlags); // reset
   }

   // This block is where normal execution starts
   // Preload pre-pend string into the stream
   *poss << m_sPrepend;

   // Preload error level into the stream
   // NOTE: errLevel == -1 is a valid number, meaning don't bother with Error Level
   if ( m_bErrorLevel && (errLevel >= LOG_EMERG) && (errLevel <= LOG_VERBOSE) ) {
      *poss << LevelToStringMap[errLevel];
   }

#ifdef __AAL_LINUX__

   if ( m_bLogPID ) {
      poss->fill('0');
      poss->flags(PIDFlags);
      *poss << "[" << GetProcessID() << ":" << std::hex << std::setw(10)
            << GetThreadID() << "] ";
   }

   if (m_bTimeStamp) {
      struct timeval tv;
      gettimeofday( &tv, NULL);
      poss->fill('0');
      poss->flags(TSFlags);
      *poss << std::setw(4) << (tv.tv_sec - m_tvZero.tv_sec) << ":"
            << std::setw(6) << ((tv.tv_usec >= m_tvZero.tv_usec) ?
                                                                (tv.tv_usec - m_tvZero.tv_usec) :
                                                                (1000000 - m_tvZero.tv_usec + tv.tv_usec) ) << " ";
   }
#endif // __AAL_LINUX__
   // reset flag state
   poss->flags(defaultFlags);
   poss->fill(defaultFillChar);

} // CLogger::PreloadOss

//=============================================================================
// Name:          CLogger::Log(int errLevel, const char* psz)
// Description:   Actually perform the logging.
// Comment:       Need speed here as well - not too fast yet ;-)
//=============================================================================
void CLogger::Log(int errLevel, const char* psz)
{
   AutoLock(this);

   std::ostringstream oss;

   PreloadOss(&oss, errLevel);
   oss << psz;

   Log(errLevel, oss);
} // CLogger::Log(int errLevel, const char* psz)

//=============================================================================
// Name:          CLogger::Log (int errlevel, std::ostringstream& ross)
// Description:   Actually perform the logging.
// Comment:       Need speed here as well - not fast yet ;-)
//=============================================================================
void CLogger::Log(int errLevel, std::ostringstream& ross)
{
   AutoLock(this);

   switch ( m_eDest ) {

      case FILE : {
         m_ofstream << ross.str();
         //if ( m_bFlush || (errLevel <= LOG_ERR) )
         if ( m_bFlush ) {
            m_ofstream.flush();
         } else if( !SetFlushEvent() && (errLevel <= LOG_ERR) ) {
            m_ofstream.flush();
         }
      } break;

      case CERR : {
         std::cerr << ross.str();
      } break;

      case COUT : {
         std::cout << ross.str();
         if ( m_bFlush || (errLevel <= LOG_ERR) ) {
            std::cout.flush();
         }
      } break;

      case SYSLOG : {
#ifdef __AAL_LINUX__
         syslog( std::min( errLevel, LOG_DEBUG), "%s", ross.str().c_str());
#endif // __AAL_LINUX__
      } break;
   }

   ross.str("");
} // CLogger::Log (int errlevel, std::ostringstream& oss)

void CLogger::Log(int errlevel, std::basic_ostream<char, std::char_traits<char> > &rbos)
{
   Log(errlevel, static_cast<std::ostringstream &>(rbos));
}

//=============================================================================
// Name:          CLogger::SetDestination
// Description:   Mutator: Set destination type and filename
// Comment:       if eDest is FILE, there better be a filename
//=============================================================================
void CLogger::SetDestination(eLogTo eDest, std::string sFile)
{
   AutoLock(this);
#ifdef __AAL_LINUX__
   // if currently in syslog, close it
   if ( SYSLOG == m_eDest ) {
      closelog();
   }
#endif // __AAL_LINUX__
   // if currently in a file, close it
   if ( FILE == m_eDest ) {
      StopFlushThread();
      m_ofstream.close();
   }

   // clear the earlier filename, if any
   m_sFile.clear();

   m_eDest = eDest;

   switch ( eDest ) {

      case COUT : {
//         m_rostream = std::cout;
      } break;

      case CERR : {
//         m_rostream = std::cerr;
      } break;

      case SYSLOG : {
#ifdef __AAL_LINUX__
         openlog(NULL, m_bLogPID ? LOG_PID : 0, LOG_USER);
#else
         m_eDest = CERR;
#endif // OS
      } break;

      case FILE : {

         m_ofstream.open(sFile.c_str(), std::ios_base::app | std::ios_base::out);

         if ( m_ofstream.good() ) {
//            m_rostream = m_ofstream;
            m_sFile = sFile;

            if( !m_bFlush ) {
               StartFlushThread();
		      }

         } else {
//            m_rostream = std::cerr;
            std::cerr << "CLogger::SetDestination could not open " << sFile
                      << "using cerr instead\n";
            m_eDest = CERR;
         }

      } break;

   }  // switch

   if ( IfLog(m_LogMask, LOG_INFO) ) {
      const time_t timet = time(NULL);
      std::ostringstream &ross = GetOss(LOG_INFO);

#if defined( _MSC_VER )
# pragma warning( push )
# pragma warning( disable:4996 )  // ctime is exactly what we want, not ctime_s
#endif // _MSC_VER
      ross << "AAL Logger-Destination set: " << ctime(&timet);
#if defined( _MSC_VER )
# pragma warning( pop )
#endif // _MSC_VER

      Log(LOG_INFO, ross);
   }

   //   Log(LOG_INFO, ctime(&timet));

} // CLogger::SetDestination

//=============================================================================
// Name:          CLogger::GetDestinationEnum
// Description:   Accessor
//=============================================================================
ILogger::eLogTo CLogger::GetDestinationEnum() const
{
   AutoLock(this);
   return m_eDest;
} // CLogger::GetDestinationEnum

//=============================================================================
// Name:          CLogger::GetDestinationFile
// Description:   Accessor
//=============================================================================
std::string CLogger::GetDestinationFile() const
{
   AutoLock(this);
   return m_sFile;
} // CLogger::GetDestinationFile

//=============================================================================
// Name:          CLogger::AddToMask
// Description:   Mutator: Set mask bits set in
// Comment:       If internal mask & mask passed in to IfLog, then will log
//=============================================================================
void CLogger::AddToMask(LogMask_t mask, int errLevel)
{
   AutoLock(this);

   m_LogMask |= mask;

   LogMask_t tMask = 1;
   int       index;

   for ( index = 1 ; index < m_numElementsInLogLevel ; ++index, tMask <<= 1 ) {
      if (mask & tMask) {
         m_rgLogLevel[index] = errLevel;
      }
   }

} // CLogger::AddToMask

//=============================================================================
// Name:          CLogger::RemoveFromMask
// Description:   Mutator: Clear mask bits passed in
// Comment:       If internal mask & mask passed in to IfLog, then will log
//=============================================================================
void CLogger::RemoveFromMask(LogMask_t mask)
{
   AutoLock(this);
   m_LogMask &= ~mask;
} // CLogger::RemoveFromMask

//=============================================================================
// Name:          CLogger::GetMask
// Description:   Accessor
//=============================================================================
LogMask_t CLogger::GetMask() const
{
   AutoLock(this);
   return m_LogMask;
} // CLogger::GetMask

//=============================================================================
// Name:          CLogger::GetLevel
// Description:   Accessor
//=============================================================================
int CLogger::GetLevel(LogMask_t mask) const
{
   AutoLock(this);
#ifdef __AAL_LINUX__
   return m_rgLogLevel[ffsll(mask)];
#endif // __AAL_LINUX__
   return 0;
} // CLogger::GetLevel

//=============================================================================
// Name:          CLogger::SetLogPID
// Description:   Mutator: Set whether or not process ID will be printed
//=============================================================================
// Tell the Logger to prepend the PID of the caller (for multi-threaded applications)
void CLogger::SetLogPID(btBool fLogPID)
{
   AutoLock(this);
   m_bLogPID = fLogPID;
} // CLogger::SetLogPID

//=============================================================================
// Name:          CLogger::GetLogPID
// Description:   Accessor
//=============================================================================
btBool CLogger::GetLogPID() const
{
   AutoLock(this);
   return m_bLogPID;
} // CLogger::GetLogPID

//=============================================================================
// Name:          CLogger::SetLogTimeStamp
// Description:   Mutator: Set whether or not to prepend a timestamp
//=============================================================================
void CLogger::SetLogTimeStamp(btBool fLogTimeStamp)
{
   AutoLock(this);
   m_bTimeStamp = fLogTimeStamp;
} // CLogger::SetLogTimeStamp

//=============================================================================
// Name:          CLogger::GetLogTimeStamp
// Description:   Accessor
//=============================================================================
btBool CLogger::GetLogTimeStamp() const
{
   AutoLock(this);
   return m_bTimeStamp;
} // CLogger::GetLogTimeStamp

//=============================================================================
// Name:          CLogger::SetLogTimeStamp
// Description:   Mutator: Set whether or not to prepend a string
//=============================================================================
void CLogger::SetLogPrepend (std::string sPrepend)
{
   AutoLock(this);
   m_sPrepend = sPrepend;
   //m_bPrepend = !sPrepend.empty();
}

//=============================================================================
// Name:          CLogger::GetLogTimeStamp
// Description:   Accessor
//=============================================================================
std::string CLogger::GetLogPrepend() const
{
   AutoLock(this);
   return m_sPrepend;
}

//=============================================================================
// Name:          CLogger::SetLogErrorLevelPrepend
// Description:   Mutator: Set whether or not to prepend the Error Level
//=============================================================================
void CLogger::SetLogErrorLevelPrepend(btBool errLevel)
{
   AutoLock(this);
   m_bErrorLevel = errLevel;
}

//=============================================================================
// Name:          CLogger::GetLogErrorLevelPrepend
// Description:   Accessor
//=============================================================================
btBool CLogger::GetLogErrorLevelPrepend() const
{
   AutoLock(this);
   return m_bErrorLevel;
}

//=============================================================================
// Name:          CLogger::SetFlush
// Description:   Mutator: Set whether or not to flush on EVERY full write
//=============================================================================
void CLogger::SetFlush(btBool flush)
{
   AutoLock(this);

   m_bFlush = flush;

   if ( m_bFlush ) { // We're handling flush ourselves.
      StopFlushThread();
   } else {          // Flush handled in separate thread.
      StartFlushThread();
   }
}

//=============================================================================
// Name:          CLogger::GetLogErrorLevelPrepend
// Description:   Accessor
//=============================================================================
btBool CLogger::GetFlush() const
{
   AutoLock(this);
   return m_bFlush;
}

////////////////////////////////////////////////////////////////////////////////
void CLogger::SetAutoFlushTime(unsigned int seconds)
{
   AutoLock(this);
   m_autoFlushTime = seconds;
}

//=============================================================================
// Name:          CLogger::GetLogErrorLevelPrepend
// Description:   Accessor
//=============================================================================
unsigned int CLogger::GetAutoFlushTime() const
{
	AutoLock(this);
	return m_autoFlushTime;
}
//=============================================================================
void CLogger::FileFlushThread(OSLThread *pThread, void *pContext)
{
   CLogger *This = static_cast<CLogger *>(pContext);

   ASSERT(NULL != This);

   while( true ) {

      switch ( This->GetFlushEvent() ) {
         case FLUSH_EXIT  : {
            This->m_bFlushThreadIsExiting = true;
         } return;

         case FLUSH_EVENT : {
            This->m_ofstream.flush();
            SleepSec(This->m_autoFlushTime);
         } break;

         case FLUSH_NONE : // Fall through
         default         : break;
      }

   }
}


END_NAMESPACE(AAL)


