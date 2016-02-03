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
//       FILE: CAALogger.h
//    CREATED: 09/19/2008
//     AUTHOR: Henry Mitchel, Intel Corporation.
//     		   Alvin Chen, Intel Corporation.
//
//    PURPOSE: Public Interface to Logger Facility
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 09/19/2008     HM       Initial version started
// 11/04/2008     HM       Made logLevel an array to track the mask bits
// 12/14/2008     HM       Spell check
// 01/04/2009     HM       Updated Copyright
// 03/22/2009     HM       Logger now sets Start Of Time to be when it is
//                            constructed, to the microsecond. Will show more
//                            consistent results across runs.
// 09/07/2009     AC       Added the auto flush feature
// 04/22/2012     HM       Disabled irrelevant warning about export
//                            of CAALEvent::m_InterfaceMap for _WIN32
//****************************************************************************
#ifndef __AALSDK_CAALLOGGER_H__
#define __AALSDK_CAALLOGGER_H__

/// @todo Document ILogger, CLogger, and related.

#include <aalsdk/AALLogger.h>  // includes <syslog.h>, <string>,
//                             // <sstream>, <AALIDDefs.h>

#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/osal/Sleep.h>
#include <aalsdk/AALDefs.h>

BEGIN_NAMESPACE(AAL)


enum LoggerConstants {
   TempStringLength = 128       // length for a buffer for strerror_r
};
/*
 * ostringstream record for the PIDossMap, below.
 * pointer should be smart reference counter, but expect that each is created and
 *    inserted into the map once. Real copying should not occur, if it does this implementation
 *    will not work, as when a copied ossRec is destructed it will delete the referenced
 *    ostringstream out from under the original. No problem until the original is used
 *    or destructed, resulting in double-free.
 * Since in fact these things are copied all around, the key distinction is that only
 *    one of them is actually used and destructed. Specifically, the last one standing
 *    in the map.
 * However, the constructor for this is now spread out between the class ossRec proper
 *    and the GetOss and Getpsz functions of the PIDossMap class and, so it is getting
 *    to the point of needing a proper smart pointer implementation.
 */
class AASLIB_API ossRec
{
   std::ostringstream*  m_poss;              // the oss associated with the index PID
   char*                m_pszTemp;           // For use by thread-specific char* functions
public:
                        ossRec();
               virtual ~ossRec();
   std::ostringstream*  GetOss()            {return m_poss;}
   void                 SetOss(std::ostringstream* poss) {m_poss = poss;}
   char*                Getpsz()            {return m_pszTemp;}
   void                 Setpsz(char* psz)   {m_pszTemp = psz;}
   void                 Destroy();          // Call then when done with the stream
}; // End of ossRec

// MSVC can't handle exporting template classes from dlls other than vector<>.
// Just uncomment this to see the mess that results, although defined as appropo in
// http://support.microsoft.com/default.aspx?scid=KB;EN-US;168958
//
//#if defined( __AAL_WINDOWS__ )
//   EXPIMP_TEMPLATE template class AASLIB_API std::map<int, ossRec>;
//#endif 
//
// Only option is to disable C4251

/*
 * Map of Process (or Thread) ID to ostringstream
 */
class AASLIB_API PIDossMap
{
#if defined( _MSC_VER )
# pragma warning( push )
# pragma warning( disable:4251 )  // Cannot export template definitions
#endif

   typedef std::map<int, ossRec> ossMap_t;
   typedef ossMap_t::iterator    ossMap_itr_t;
   ossMap_t                      m_ossMap;   // the map being manipulated

#if defined( _MSC_VER )
# pragma warning( pop )
#endif

public:
   PIDossMap();
   virtual ~PIDossMap();
   std::ostringstream* GetOss(int index);    // Return a ostringstream pointer based on index
   char*               Getpsz(int index);    // Return a temporary string based on index
}; // End of PIDossMap

//=============================================================================
// Name:          CLogger
// Description:   Concrete implementation of ILogger
// Comment:       Expected to be just one instantiation per load module, e.g.
//                   in a .so or .dll.
//                Can be declared static.
//=============================================================================
class AASLIB_API CLogger : public ILogger, public CriticalSection
{
private:
   eLogTo               m_eDest;       // Type of output file
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string          m_sFile;       // Name of the output file, if there is one
                                       // Number of elements in m_rgLogLevel
                                       //    (i.e. 1 to 64, one for each bit, plus 0th special case)
                                       //    having 65 elements allows direct indexing by ffsll()
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   static const int     m_numElementsInLogLevel = (sizeof(LogMask_t)*8)+1;
                                       // Only pass through levels =< than m_rgLogLevel[]
                                       //    Array of levels indexed by lowest set bit in mask
   int                  m_rgLogLevel[m_numElementsInLogLevel];
   LogMask_t            m_LogMask;     // Logger mask to put against mask passed in
   btBool               m_bLogPID;     // Whether to log the Process ID
   btBool               m_bTimeStamp;  // Whether to log the time stamp
   btBool               m_bErrorLevel; // Whether to log the error level itself
   btBool               m_bFlush;      // Whether to flush the stream after every output
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::ofstream        m_ofstream;    // Output file stream
   std::string          m_sPrepend;    // Prepended string
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   PIDossMap            m_PIDossMap;	// map of ThreadID/PID to ostringstream
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::ostringstream   m_oss;         // backup ostringstream because must return a reference
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   char                 m_szTemp[TempStringLength]; // string for use by thread-specific char* functions
#ifdef __AAL_LINUX__
   struct timeval       m_tvZero;      // Start of time definition
#endif // __AAL_LINUX__

   // Prepend the ostringstream with time-stamp and thread-id etc.
   void                 PreloadOss     (std::ostringstream* poss, int errLevel);

  // Return a temporary thread-specific string based on index.
  // Will never fail. Return ptr will always be valid. Buffer Length is TempStringLength.
   char*                Getpsz();

   OSLThread           *m_pFlushThread;
   CSemaphore          	m_flushEvent;
   volatile btBool      m_bExitFlushThread;
   volatile btBool      m_bFlushThreadIsExiting;
   volatile btBool      m_needFlush;
   unsigned	int			m_autoFlushTime;

   void StartFlushThread() {
      AutoLock(this);
      if ( NULL == m_pFlushThread ) {
         m_flushEvent.Create(0, 1);

         m_bExitFlushThread      = false;
         m_bFlushThreadIsExiting = false;
         m_needFlush             = false;

         m_pFlushThread = new OSLThread(CLogger::FileFlushThread, OSLThread::THREADPRIORITY_NORMAL, this);
         if ( NULL == m_pFlushThread ) {
            std::cerr << __AAL_FUNC__ <<
                     "(): create flush thread failed. Using manual flush." << std::endl;
            m_flushEvent.Destroy();
         }
      }
   }

   void StopFlushThread()
   {
      AAL::btBool JoinFlushThr = false;

      {
         AutoLock(this);
         if ( NULL != m_pFlushThread ) {
            m_needFlush  = false;
            JoinFlushThr = true;
         }
      }

      if ( JoinFlushThr ) {

         while ( !m_bFlushThreadIsExiting ) {
            m_bExitFlushThread = true;
            m_flushEvent.Post(1);
            SleepZero();
         }

         m_pFlushThread->Join();

         {
            AutoLock(this);
            delete m_pFlushThread;
            m_pFlushThread = NULL;
            m_flushEvent.Destroy();
         }

      }
   }

   btBool SetFlushEvent() {
      AutoLock(this);
      if ( NULL == m_pFlushThread ) {
         return false;
      }

      if( !m_needFlush ) { // Max of 1 pending flush event.
         m_needFlush = true;
         m_flushEvent.Post(1);
      }
      return true;
   }

   enum FlushEvent
   {
      FLUSH_NONE = 0,
      FLUSH_EXIT,
      FLUSH_EVENT
   };
   FlushEvent GetFlushEvent() {
      // Don't AutoLock here, as we cannot Wait() with an AutoLock, else deadlock.
      m_flushEvent.Wait();

      FlushEvent e = FLUSH_NONE;

      if ( m_bExitFlushThread ) {
         e = FLUSH_EXIT;
      } else if ( m_needFlush ) {
         e = FLUSH_EVENT;
         m_needFlush = false;
      }

      return e;
   }

public:
   // Ctor/Dtor
   CLogger();
   ~CLogger();

   // Ask whether the log should be executed at all
   btBool      IfLog                (LogMask_t mask, int errlevel) const;

   // Get an ostringstream to write it to, based on PID
   std::ostringstream& GetOss       (int errLevel=-1); // defaults to NOT including the Error Level

   // Return the perror string for an errno defined in errno.h
   //    or a string version of the error and message that it was not found
   char*      GetErrorString        (int errNum);

   // Log a string
   void        Log                  (int errlevel, const char* psz);
   void        Log                  (int errlevel, std::ostringstream& ros);
   void        Log                  (int errlevel, std::basic_ostream<char, std::char_traits<char> > &rbos);

   // if eDest is FILE, there better be a filename
   void        SetDestination       (eLogTo eDest=COUT, std::string sFile="");
   eLogTo      GetDestinationEnum   () const;
   std::string GetDestinationFile   () const;

   // Set the global mask that determines what is logged
   void        AddToMask            (LogMask_t mask, int errLevel);
   void        RemoveFromMask       (LogMask_t mask);
   LogMask_t   GetMask              () const;

   // Set the global debug level to be accepted, one of LOG_EMERG to LOG_DEBUG, from syslog.h
//   void        SetLevel             (int errLevel, LogMask_t mask=0);
   int         GetLevel             (LogMask_t mask) const;

   // Tell the Logger to prepend the PID of the caller (for multi-threaded applications)
   void        SetLogPID            (btBool fLogPID);
   btBool      GetLogPID            () const;

   // Tell the Logger to prepend a wall-clock timestamp
   void        SetLogTimeStamp      (btBool fLogTimeStamp);
   btBool      GetLogTimeStamp      () const;

   // Tell the Logger to always prepend a string
   void        SetLogPrepend        (std::string sPrepend);
   std::string GetLogPrepend        () const;

   // Tell the Logger whether to prepend the error Level
   void        SetLogErrorLevelPrepend (btBool errLevel);
   btBool      GetLogErrorLevelPrepend () const;

   // Tell the Logger whether to flush on EVERY full write
   void        SetFlush (btBool flush);
   btBool      GetFlush () const;

   void		   	SetAutoFlushTime( unsigned int seconds );
   unsigned int GetAutoFlushTime( void ) const;

   static void FileFlushThread(OSLThread *pThread, void *pContext);

   // No copying allowed
   CLogger(const CLogger & );
   CLogger & operator = (const CLogger & );
}; // class ILogger


END_NAMESPACE(AAL)


#endif // __AALSDK_CAALLOGGER_H__

