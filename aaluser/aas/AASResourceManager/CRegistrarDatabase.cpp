// Copyright (c) 2007-2015, Intel Corporation
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
/// @file CRegistrarDatabase.cpp
/// @brief Database for Registrar.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/01/2007     HM       Initial version started
/// 08/12/2007     HM       Finally got to the implementation
/// 08/19/2007     JG       Fixed unportable _fileno and _chsize
///                            function references
/// 08/26/2007     HM       Added record locking
/// 10/31/2007     HM       #ifndef __GNUC__ away various #pragmas
/// 11/??/2007     AC       Removed calls to flockfile as they lock at the
///                            thread level instead of the process level
/// 11/17/2007     HM       Removed no longer used #defines
/// 11/27/2007     HM       Fixed bad delete of m_pdbrList
/// 12/16/2007     JG       Changed include path to expose aas/
/// 03/22/2008     HM       Removed reference to aas/AALCNamedValueSet.h
/// 05/08/2008     HM       Comments & License
/// 06/01/2008     HM       Rework to make system work off a map, using
///                            PrimaryKey as the iterator
/// 06/11/2008     HM       Fixes to go with RegistrarDatabase updates
/// 06/12/2008     HM       Make sure a Get cannot get a second lock on an already locked record
///                            And other fixes associated with regression testing
/// 06/16/2008     HM       Robustified Delete by ensuring returned iterator
///                            is at end() instead of dangling
/// TODO: What happens if a Get occurs without a Commit?
/// TODO: Do not allow normal operations from clients on the State Record.
///       Perhaps the state record should be kept separately.
/// 06/20/2008     HM       Separated eReg printing from general error logging
/// 06/27/2008     HM       Changed Open argument from path to nvsoptArgs,
///                         Moved eReg printing to Proxy
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 09/12/2008     HM       Major check-in for new remote database
/// 10/06/2008     HM       Initial modification to use Logger
/// 11/09/2008     HM       Deprecating Open/Close, and adding individual
///                            record flushing. Added FlushRecordToDisk and
///                            added ios_base::trunc to the open mode.
/// 01/04/2009     HM       Updated Copyright
/// 01/14/2009     HM       Fixed leak associated with getcwd()
/// 02/15/2009     HM       Add keyRegRecordNum to Register@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/registrar/AASRegistrar.h" // for RegistrarPathKeyName
#include "aalsdk/rm/CRegistrarDatabase.h"  // This implementation's header
#include "aalsdk/utils/Utilities.h"        // likely, unlikely
#include "aalsdk/AALLoggerExtern.h"

#if defined( __AAL_LINUX__ )
# include <dirent.h>                    // for scandir() - need a windows functions
# include <unistd.h>                    // for getcwd()
#endif // __AAL_LINUX__

USING_NAMESPACE(std)

// Declarations for Internal functions
// NOTE: declaring the functions in here, and defining them later, causes the GNU compiler to
//       be confused and think there are two distinct function declarations. When the function
//       is called, it cannot disambiguate them (because they have exactly the same signatures),
//       and fails.
// Net:  The functions are defined as well as declared here, however significantly inconvenient
//       that may be.

namespace {
   // Filter function for scandir(), that matches filetype of nvs
   // TODO: Check filename format, e.g. only # of a certain length, or maybe hex
   // An example of an easily parsable and checkable format would be 0xNNNNNNNNNNNNNNNN.nvs
   // There are exactly 16 hex digits

   int DatabaseFilter(const struct dirent *p) {
      int len = strlen(p->d_name);
      if (len>=4 && !memcmp(".nvs", &p->d_name[len-4], 4)) {  // matches filetype of .nvs
         return 1;
      }
      else {
         return 0;
      }
   }
}  // end of unnamed namespace



BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

DBRec::DBRec() :
   m_Lock(Unlocked),
   m_bModified(false)
{}

DBRec::~DBRec() {}

//=============================================================================
// Name: operator << on DBRec
// Description: prints a DBRec to a stream
//=============================================================================
std::ostream & operator << (std::ostream &s, const DBRec &dbr)
{
   s << "DBRec    " << &dbr << std::endl;
   s << "Lock     " << dbr.m_Lock << std::endl;
   s << "Modified " << dbr.m_bModified << std::endl;
   s << "NVS \n"    << dbr.m_nvs ;
   return s;
}

CRegDB::CRegDB() :
   m_PrimaryKey(NoKey),
   m_LockKey(Unlocked),
   m_fValid(false)
{}

CRegDB::~CRegDB() {}

//=============================================================================
// Name: operator << on CRegDB
// Description: prints a CRegDB to a stream
//=============================================================================
std::ostream & operator << (std::ostream &s, const CRegDB &dbr)
{
   s << "CDBReg      " << &dbr << std::endl;
   s << "Primary Key " << dbr.m_PrimaryKey << std::endl;
   s << "Lock        " << dbr.m_LockKey << std::endl;
   s << "Valid Flag  " << dbr.m_fValid << std::endl;
   return s;
}
#if 0
//=============================================================================
// Name: operator << on CRegDB, with an indent value
// Description: prints a CRegDB to a stream
//=============================================================================
std::ostream& AAL::AAS::operator << (std::ostream& s, const AAL::AAS::CRegDB& dbr, int indent) {
   std::string tabs;
   while (indent--) tabs += "\t";

   s << tabs << "CDBReg      " << &dbr << endl;
   s << tabs << "Primary Key " << dbr.m_PrimaryKey << std::endl;
   s << tabs << "Lock        " << dbr.m_LockKey << std::endl;
   s << tabs << "Valid Flag  " << dbr.m_fValid << std::endl;
   return s;
}
#endif // 0


CRegistrarDatabase::CRegistrarDatabase(const CRegistrarDatabase & ) {/*empty*/}
CRegistrarDatabase & CRegistrarDatabase::operator=(const CRegistrarDatabase & ) { return *this; }


//=================================================
// CRegistrarDatabase::CRegistrarDatabase
// Constructor: Default
// Parameters:    pOptArgs defaults to NULL
//=================================================
CRegistrarDatabase::CRegistrarDatabase(NamedValueSet *pOptArgs) :
   CriticalSection(),
   m_DatabasePath(),
   m_pdbrMap(NULL),
   m_bIsOK(false),
   m_NextDatabaseKey(),
   m_NextLockKey(),
   m_OptArgs()
{
   AutoLock(this);

   if (pOptArgs) {
      m_OptArgs = *pOptArgs;
   }

   /*
    * Get the database location
    */
   btcString DBPath = NULL;

   if ( m_OptArgs.Has(nidSystemRegistrarPath) ) {                           // Try the Optional Arguments first
      m_OptArgs.Get(nidSystemRegistrarPath, &DBPath);
   } else if ( NULL == (DBPath = getenv("AAL_REGISTRAR_DATABASE_PATH")) ) { // Try the Environment
                                                                            // If not in the Environment, use build-time defaults
#ifdef REGISTRAR_DATABASE_PATH
      DBPath = REGISTRAR_DATABASE_PATH;
#endif // REGISTRAR_DATABASE_PATH

#if defined( __AAL_WINDOWS__ )
      DBPath = "..\\RegistrarRepository\\win";
#endif // __AAL_WINDOWS__
   }

   m_DatabasePath = DBPath;

   LogReturnCode(eRegOK, "CRegistrarDatabase pDatabasePath = '" + m_DatabasePath + "'.", LOG_INFO);

   // In separate mode, there can be only one database, so open it here
   NamedValueSet nil;
   eReg retval = Open(nil);               // use default databasepath just set

   m_bIsOK = (eRegOK == retval);          // set for top-level constructor success determination
}  // End of CRegistrarDatabase::CRegistrarDatabase

//=================================================
// CRegistrarDatabase::~CRegistrarDatabase
//=================================================
CRegistrarDatabase::~CRegistrarDatabase()
{
   AutoLock(this);
   if ( NULL != m_pdbrMap ) {
      delete m_pdbrMap;
      m_pdbrMap = NULL;
   }
}  // End of CRegistrarDatabase::~CRegistrarDatabase

//=============================================================================
// CRegistrarDatabase::DumpDatabase
//=============================================================================
void CRegistrarDatabase::DumpDatabase() const
{
   AutoLock(this);

   // Dump the database path
   cout << "DUMP DATABASE: " << m_DatabasePath << endl;

   // Is there already a database loaded?
   if (!m_pdbrMap) {                    // Is there no database loaded?
      cerr << "No database loaded\n";
      return;
   }

   for (dbrMap_itr_t itr = m_pdbrMap->begin(); itr != m_pdbrMap->end(); ++itr) {
      cout << "Key: " << (*itr).first << "\n" << (*itr).second << endl;    // (*itr).second is a DBRec
   }
}  // End of CRegistrarDatabase::DumpDatabase

//=============================================================================
// CRegistrarDatabase::DumpDatabase, alternate version called by operator <<
//=============================================================================
std::ostream & CRegistrarDatabase::DumpDatabase(std::ostream &ross) const
{
   AutoLock(this);

   // Dump the database path
   ross << "DUMP DATABASE: " << m_DatabasePath << endl;

   // Is there already a database loaded?
   if (!m_pdbrMap) {                    // Is there no database loaded?
      ross << "No database loaded\n";
   }
   else {
      for (dbrMap_itr_t itr = m_pdbrMap->begin(); itr != m_pdbrMap->end(); ++itr) {
         ross << "Key: " << (*itr).first << "\n" << (*itr).second << endl;    // (*itr).second is a DBRec
      }
   }
   return ross;
}

//=============================================================================
// operator << on CRegistrarDatabase, not a member function
//=============================================================================
std::ostream & operator << (std::ostream &s, const CRegistrarDatabase &theDatabase)
{
   return theDatabase.DumpDatabase(s);
}

//=============================================================================
// CRegistrarDatabase::FullFilePathFromDBKey
//    Worker function to convert key value into filename
//=============================================================================
std::string CRegistrarDatabase::FullFilePathFromDBKey(DatabaseKey_t dbKey)
{
   char numbuf[32];                          // for filename
   sprintf ( numbuf, "%llu", dbKey);
   return m_DatabasePath + numbuf + ".nvs";
}  // end of CRegistrarDatabase::FullFilePathFromDBKey

//=============================================================================
// CRegistrarDatabase::LoadFromDisk
//    Replaces CRegistrarDatabase::LoadAndLockDatabase for Daemon
// Flow:
//    If there is already a database loaded, signal an error and do nothing
//       In the future, could write the current one back, clean up, and then open the new one
//    If the PassedInDatabasePath is null, use the default set during construction
//       If both are null, signal and error and do nothing
//    Remember the directory used, and normalize it to always end in '/'
//    Find the directory and get a list of files
//    Create the map, and load the files
//    Set database parameters from the special file (primary key 0). If it
//       does not exist, create it
// After execution:
//    m_pdbrMap will be pointing to a map of primary key indexed DBRec's
//=============================================================================
eReg CRegistrarDatabase::LoadFromDisk(struct dirent **pNameList, int nNameList)
{
   fstream instream;                         // single .nvs file
   pair <dbrMap_itr_t, btBool> insertRetVal; // for call to insert
   DatabaseKey_t dbKey;                      // the key for the call to insert
   const std::string fName("LoadFromDisk");
   eReg eRetVal(eRegOK);

   DBRec DBRecNil;                           // empty DBRec to hold the record
   DBRecNil.m_Lock = Unlocked;               // These fields will be copied into every record
   DBRecNil.m_bModified = false;

   while(nNameList--) {                      // iterate through the files. Backwards works as well as forwards.

      // open the file in pNameList[nNameList]
      string FullFileName = m_DatabasePath + pNameList[nNameList]->d_name;

      AAL_DEBUG(LM_Database, "CRegistrarDatabase::LoadFromDisk - Processing file: " << FullFileName << endl);

      // Open file
      instream.open( FullFileName.c_str(), ios_base::binary | ios_base::in);
      if (!instream) {
         eRetVal = LogReturnCode(eRegOpenBadFile, fName + " - file '" + FullFileName + "'", LOG_WARNING);
         free(pNameList[nNameList]);
      }
      else {                                    // have an open file
         // Get the key
         dbKey = strtoull(pNameList[nNameList]->d_name, NULL, 0);
         // TODO: Error check: 0 is a bad return, means 0.nvs cannot be the magic one. Also, check format
         //       in scandir callback

         AAL_DEBUG(LM_Database, "CRegistrarDatabase::LoadFromDisk - Key is " << dbKey << endl);

         // Read the nvs into the DBRec
         insertRetVal = m_pdbrMap->insert(pair<DatabaseKey_t, DBRec>(dbKey, DBRecNil));

         if (insertRetVal.second) {
            ENamedValues eretval = NVSReadNVS (instream, &(*insertRetVal.first).second.m_nvs); // read the NVS
            if (eretval != ENamedValuesOK) {       // if got a bad nvs, remove it from the map
               m_pdbrMap->erase( insertRetVal.first );
               LogReturnCode(eRegOpenCannotReadNVSRec, fName + " '" + FullFileName+ "' ", LOG_ERR);
            }
         }
         else {
            ostringstream oss;
            oss << dbKey;
            LogReturnCode(eRegNoInsertAtKey, fName + " - " + oss.str(), LOG_WARNING);
         }

         // Done with the file, filename, and nvs
         free(pNameList[nNameList]);
         instream.close();
         DBRecNil.m_nvs.Empty();

      }  // End if (!instream) else
   }  // End while(nNameList--)
   return eRetVal;
#if 0       /* keep around for useful reference when decoding map and iterator references */
   cout << "Insertion result is " << insertRetVal.second << endl;
                 dbrMap_itr_t itr = insertRetVal.first;
                 DatabaseKey_t dbKey = (*itr).first;                   // Useful reference
            //DatabaseKey_t dbKey = (*insertRetVal.first).first;  // Useful reference
                 cout << "Inserted Key is " << dbKey << endl;
                 DBRec dbrTemp = (*itr).second;                        // Useful reference
            //DBRec dbrTemp = (*insertRetVal.first).second;     // Useful reference
                 cout << "Inserted DBRec is " << dbrTemp << endl;
                 cout << "Inserted NVS is " << dbrTemp.m_nvs << endl;
#endif
}  // End of LoadFromDisk

//=============================================================================
// CRegistrarDatabase::Open
//    Replaces CRegistrarDatabase::LoadAndLockDatabase for Daemon
// Flow:
//    If there is already a database loaded, signal an error and do nothing
//       In the future, could write the current one back, clean up, and then open the new one
//    If the PassedInDatabasePath is null, use the default set during construction
//       If both are null, signal and error and do nothing
//    Remember the directory used, and normalize it to always end in '/'
//    Find the directory and get a list of files
//    Create the map, and load the files
//    Set database parameters from the special file (primary key 0). If it
//       does not exist, create it
// After execution:
//    m_pdbrMap will be pointing to a map of primary key indexed DBRec's
// Return value:
//    0 on success, errno on file failure, or one of the below
//       -1: Database already loaded
//       -2: No database path available - none from constructor and none passed in
//       -3: Database Path provided does not exist or cannot be created
//       -4: Out of memory
//       -5: Could not create new database
//=============================================================================
eReg CRegistrarDatabase::Open(const NamedValueSet &rnvsOptArgs)
{
   AutoLock(this);
   const std::string fName("Open");

   // Is there already a database loaded?
   if (m_pdbrMap) {                    // Is there already a database loaded?
      return LogReturnCode(eRegDBAlreadyLoaded, fName, LOG_ERR);
   }

   // Get the database path, part 1
   btcString pPathName;
   if ( rnvsOptArgs.Has(RegistrarPathKeyName) ) {
      rnvsOptArgs.Get(RegistrarPathKeyName, &pPathName);
   } else {
      pPathName = NULL;
   }

   // Get the database path, part 2
   std::string DatabasePath;
   if (pPathName) {              // If path is passed in, use it
      DatabasePath = pPathName;
   } else {                                    // Otherwise use the default one
      DatabasePath = m_DatabasePath;
   }

   if ( DatabasePath.empty() ) {               // No path passed in
      return LogReturnCode(eRegOpenNoPath, fName, LOG_ERR);
   }

   // Append '/' if not already there
   if ('/' != DatabasePath[DatabasePath.length()-1]) {
      DatabasePath += '/';
   }

   // Update the default path to the one actually used
   m_DatabasePath = DatabasePath;

   // Get a filelist of the records in the database
   struct dirent **pNameList;
   int nNameList;

#if AAL_LOG_LEVEL >= LOG_INFO
   char* pcwd = getcwd(NULL,0);
   AAL_INFO(LM_Database, "CRegistrarDatabase::Open CWD='" << pcwd <<
                         "' and DatabasePath='" << DatabasePath << "'\n");
   free( pcwd);
#endif

   nNameList = scandir(DatabasePath.c_str(), &pNameList, DatabaseFilter, alphasort);
   if ( nNameList < 0 ) {                      // no need to free pNameList, not allocated on error
      return LogReturnCode(eRegOpenScanError, fName, LOG_ERR);
   }

   // Allocate storage for the database
   m_pdbrMap = new dbrMap_t;
   if (!m_pdbrMap) {
      free(pNameList);                       // allocated by scandir
      return LogReturnCode(eRegDBNotLoaded, fName, LOG_ERR);
   }

   if ( !nNameList ) {                       // Directory okay but no files
      NamedValueSet nvsBase;                 // Establish a default database
      nvsBase.Add(NextLockKeyName, static_cast<LockKey_t>(DefaultNextLock));
      nvsBase.Add(NextDatabaseKeyName, static_cast<DatabaseKey_t>(DefaultNextPrimaryKey));
      CRegDB rdb;
      rdb.m_PrimaryKey = StateNVSPrimaryKey;
      rdb.m_LockKey = Unlocked;
      if (eRegOK != InsertByKey( nvsBase, rdb)) {
         free(pNameList);                    // allocated by scandir
         return LogReturnCode(eRegOpenNoStartup, fName, LOG_ERR);
      }
   } else {
      LoadFromDisk( pNameList, nNameList);
      free(pNameList);                       // allocated by scandir
   }

   // Set the database internal variables from StateNVSPrimaryKey.nvs
   (*m_pdbrMap)[StateNVSPrimaryKey].m_nvs.Get( NextLockKeyName,     &m_NextLockKey);
   (*m_pdbrMap)[StateNVSPrimaryKey].m_nvs.Get( NextDatabaseKeyName, &m_NextDatabaseKey);

   return eRegOK;                            // so far as can tell, things are okay

}  // End of CRegistrarDatabase::Open

//=============================================================================
// CRegistrarDatabase::FlushRecordToDisk
//    Worker function for Close.
//    Broken out so can be called separately if needed
//    Constraints:
//       Upon entry m_pdbrMap must be valid.
//    Flow:
//       Write all modified entries in the database to disk
//       At entry, database must be loaded as represented by m_pdbrMap
//       At exit, all modified entries in the database have been flushed to disk,
//          but m_pdbrMap is still valid
//=============================================================================
eReg CRegistrarDatabase::FlushRecordToDisk(const dbrMap_itr_t &itr)
{
   fstream outstream;                        // single .nvs file
   std::string FullFileName;                 // The database path is in m_DatabasePath
   eReg eRetVal(eRegOK);                     // Return value

   // Retrieve the record
   if ((*itr).second.m_bModified) {          // only write out if modified
      DatabaseKey_t dbKey = (*itr).first;
      FullFileName = FullFilePathFromDBKey (dbKey);

      AAL_DEBUG(LM_Database,"CRegistrarDatabase::FlushRecordToDisk - Writing to Filename " << FullFileName << endl);

      outstream.open( FullFileName.c_str(), ios_base::binary | ios_base::out | ios_base::trunc);
      if (outstream) {
         ENamedValues eNVRetVal = NVSWriteOneNVSToFile (outstream, (*itr).second.m_nvs, 0);   // write the nvs
         if (!outstream || (ENamedValuesOK != eNVRetVal)) {
            // record error and do not update modified flag
            eRetVal = LogReturnCode(eRegCannotWriteOpenFile, "FlushRecordToDisk - file '" + FullFileName + "'", LOG_WARNING);
         }
         else {   // successful write, clear modified flag
            ((*itr).second.m_bModified) = false;
         }
         outstream.close();
      }
      else {
         eRetVal = LogReturnCode(eRegOpenBadFile, "FlushRecordToDisk - file '" + FullFileName + "'", LOG_WARNING);
      }
   }
   return eRetVal;
}  // End of CRegistrarDatabase::FlushRecordToDisk

//=============================================================================
// CRegistrarDatabase::FlushDatabaseToDisk
//    Worker function for Close.
//    Broken out so can be called separately if needed
//    Constraints:
//       Upon entry m_pdbrMap must be valid.
//    Flow:
//       Write all modified entries in the database to disk
//       At entry, database must be loaded as represented by m_pdbrMap
//       At exit, all modified entries in the database have been flushed to disk,
//          but m_pdbrMap is still valid
//=============================================================================
eReg CRegistrarDatabase::FlushDatabaseToDisk()
{
   fstream outstream;                        // single .nvs file
   std::string FullFileName;                 // The database path is in m_DatabasePath
   dbrMap_itr_t itr;                         // iterator across list
   eReg eRetVal(eRegOK);                     // Return value

   // For each record in the database, retrieve it, check for modification, and write it out
   for (itr=m_pdbrMap->begin(); itr!=m_pdbrMap->end(); ++itr) {
      FlushRecordToDisk(itr);
   }
   return eRetVal;
}  // End of CRegistrarDatabase::FlushDatabaseToDisk

//=============================================================================
// CRegistrarDatabase::Close
//    Constraints:
//       Write all modified entries in the database to disk
//       At entry, database should be loaded as represented by m_pdbrMap
//       At exit, database is flushed to disk and destroyed internally as represented
//          by m_pdbrMap being NULL
//=============================================================================
eReg CRegistrarDatabase::Close()
{
   AutoLock(this);

   if (!m_pdbrMap) {                         // Error if no database loaded
      return LogReturnCode(eRegDBNotLoaded, "Close", LOG_WARNING);
   }

   AAL_DEBUG(LM_Database, "From CRegistrarDatabase::Close, DumpDatabase() before FlushDatabaseToDisk "
                            << *this);

   eReg eRetVal = FlushDatabaseToDisk();     // Flush all modified entries to disk

   delete m_pdbrMap;                         // Remove the internal database
   m_pdbrMap = NULL;

   return eRetVal;
}  // End of CRegistrarDatabase::Close

//=============================================================================
// CRegistrarDatabase::FindByKey
//    Worker routing to find an entry in the database by key
//    No checking performed
//    If found, returns true and iterator reference is valid.
//    If not found, returns false and iterator reference is end()
//=============================================================================
btBool CRegistrarDatabase::FindByKey(DatabaseKey_t dbKey, dbrMap_itr_t &itr)
{
   itr = m_pdbrMap->find(dbKey);
   return (itr != m_pdbrMap->end());
}

//=============================================================================
// CRegistrarDatabase::Find
//    Find a match of rPattern, and return a description of it in the iterator
//       passed in
//    CRegDB will contain an iterator for use and return
//    Return value specifies if operation succeeded, and iterator points to result
//    fSubset TRUE means find a subset match, FALSE means find Exact match -
//       default to TRUE
//    fBegin TRUE means start a new search, FALSE means start from PrimaryKey -
//       default to TRUE
//=============================================================================
eReg CRegistrarDatabase::Find(NamedValueSet const &rPattern,
                              CRegDB              &rRegDBRet,
                              btBool               fSubset,
                              btBool               fBegin)
{
   AutoLock(this);

   if ( NULL == m_pdbrMap ) {          // passed in bad parameter. Never expected, just defensive
      rRegDBRet.SetInvalid();
      return LogReturnCode( eRegDBNotLoaded, "Find", LOG_ERR );
   }
   AAL_DEBUG(LM_Database,
              "CRegistrarDatabase::Find: rPattern     = \n" << rPattern
         <<   "CRegistrarDatabase::Find: rRegDBRet    = \n" << rRegDBRet
         <<   "CRegistrarDatabase::Find: fSubset      = " << fSubset
         << "\nCRegistrarDatabase::Find: fBegin       = " << fBegin
         << "\nCRegistrarDatabase::Find: DumpDatabase = \n" << *this
         );

   // Set starting position
   if ( fBegin ) {                                          // start at the beginning
      rRegDBRet.m_itr = m_pdbrMap->begin();
   } else {                                                 // or start at PrimaryKey+1
      // Find the element
      rRegDBRet.m_itr = m_pdbrMap->find( rRegDBRet.m_PrimaryKey );
      if ( rRegDBRet.m_itr != m_pdbrMap->end() ) {          // If found, go to the next one
         ++rRegDBRet.m_itr;
      } else {                                              // If not found, find the next larger one to start at
         rRegDBRet.m_itr = m_pdbrMap->lower_bound( rRegDBRet.m_PrimaryKey );
      }
   }

   // Scan for match from starting position set above
   for ( ; rRegDBRet.m_itr != m_pdbrMap->end(); ++rRegDBRet.m_itr ) {
      AAL_DEBUG(LM_Database, "CRegistrarDatabase::Find: Iterated Record Key = " <<
                             (*rRegDBRet.m_itr).first <<
                             "\nCRegistrarDatabase::Find: Iterated NVS to match against rPattern\n" <<
                             (*rRegDBRet.m_itr).second.m_nvs);
      if ( fSubset                                                   // Look for a match
            ? rPattern.Subset( ( *rRegDBRet.m_itr ).second.m_nvs )   // match subset?
            : rPattern == ( *rRegDBRet.m_itr ).second.m_nvs ){       // match exact?
         // Found it. rRegDBRet.m_itr contains the iterator pointing to the found record
         rRegDBRet.m_PrimaryKey = ( *rRegDBRet.m_itr ).first;        // Remember the Key
         rRegDBRet.m_LockKey = Unlocked;                             // Do NOT want to give access to someone else, so do NOT copy the lock
         rRegDBRet.SetFlag(m_pdbrMap);
         return eRegOK;
      }
   }
   // Did NOT find a match
   rRegDBRet.SetFlag(m_pdbrMap);
   return eRegNoFindByPattern;
}  // End of CRegistrarDatabase::Find

//=============================================================================
// CRegistrarDatabase::GetByKey
//    Retrieve a lock for the passed in PrimaryKey, returning both the
//    LockKey and the iterator pointing to the NVS in the CRegDB
//=============================================================================
eReg CRegistrarDatabase::GetByKey(CRegDB &rRegDBRet)
{
   AutoLock(this);
   const char fName[] = "GetByKey";

   AAL_DEBUG(LM_Database, "CRegistrarDatabase::GetByKey rRegDBRet =\n" << rRegDBRet);

   if ( NULL == m_pdbrMap ) {          // passed in bad parameter. Never expected, just defensive
      rRegDBRet.SetInvalid();
      return LogReturnCode( eRegDBNotLoaded, fName, LOG_ERR );
   }

   btBool fFoundIt = FindByKey(rRegDBRet.m_PrimaryKey, rRegDBRet.m_itr);
   rRegDBRet.SetFlag(m_pdbrMap);

   if (fFoundIt) {                                    // if found, check that record is unlocked,
      if ( Unlocked == (*rRegDBRet.m_itr).second.m_Lock ){
         LockKey_t newKey = m_NextLockKey++;          // Generate the lock & save it in the local database state
         (*rRegDBRet.m_itr).second.m_Lock = newKey;   // Set it in the record
         rRegDBRet.m_LockKey = newKey;                // Set it in the RegDB
         UpdateStateRecord();                         // Save the state
         return eRegOK;
      } else if ( rRegDBRet.m_LockKey == (*rRegDBRet.m_itr).second.m_Lock ){ // or locked with the current key
         return eRegOK;                                                    // no need to relock, already locked
      } else {
         return LogReturnCode( eRegDoubleLock, fName, LOG_WARNING );       // Nope, already locked
      }
   } else {
      return LogReturnCode( eRegNoFindByKey, fName, LOG_WARNING );         // Nope, couldn't find it
   }
}  // End of CRegistrarDatabase::GetByKey

//=============================================================================
// CRegistrarDatabase::GetByPattern
//    Retrieve a lock for the passed in PrimaryKey
//=============================================================================
eReg CRegistrarDatabase::GetByPattern(NamedValueSet const &rPattern, CRegDB &rRegDBRet)
{
   AutoLock(this);
   const char fName[] = "GetByPattern";

   AAL_DEBUG(LM_Database,"CRegistrarDatabase::GetByPattern: rPattern =\n" << rPattern <<
                         "CRegistrarDatabase::GetByPattern: rRegDBRet =\n" << rRegDBRet <<
                         "CRegistrarDatabase::GetByPattern: DumpDatabase =\n" << *this);

   if ( NULL == m_pdbrMap ) {          // passed in bad parameter. Never expected, just defensive
      rRegDBRet.SetInvalid();
      return LogReturnCode( eRegDBNotLoaded, fName, LOG_ERR );
   }

   if (eRegOK == FindSubsetBegin(rPattern, rRegDBRet)) {   // If found, just lock it
      return GetByKey(rRegDBRet);
   }

   return LogReturnCode( eRegNoFindByPattern, fName, LOG_DEBUG );
}  // End of CRegistrarDatabase::GetByPattern

//=============================================================================
// CRegistrarDatabase::InsertByKey
//    rRegDBRet.m_PrimaryKey is the key used, no checking provided
//    The lock is set to rRegDBRet.m_Lock
//    Writes new record to disk
// Returns:
//    Return value specifies if the insertion worked at a mechanical level
//    rRegDBRet.m_itr points at the inserted record. If the insertion did not
//       work, rRegDBRet.m_itr points to the end()
//=============================================================================
eReg CRegistrarDatabase::InsertByKey(NamedValueSet const &rNew, CRegDB &rRegDBRet)
{
   AutoLock(this);
   pair <dbrMap_itr_t, btBool> insertRetVal; // for call to insert()
   DBRec DBRecNil;                           // empty DBRec to hold the record
                                             //    constructor sets Lock & Modified to Unlocked,false
   // Insert record
   insertRetVal = m_pdbrMap->insert(pair<DatabaseKey_t, DBRec>(rRegDBRet.m_PrimaryKey, DBRecNil));

   // Load record with information if insertion worked
   if ( insertRetVal.second ) {
      rRegDBRet.m_itr = insertRetVal.first;  // insertRetVal.first is the iterator
                                             // (*insertRetVal.first).second is the inserted DBRec
      (*insertRetVal.first).second.m_nvs = rNew;
      (*insertRetVal.first).second.m_Lock = rRegDBRet.m_LockKey;
      (*insertRetVal.first).second.m_bModified = true;
      rRegDBRet.SetFlag(m_pdbrMap);
      return FlushRecordToDisk(insertRetVal.first);
   } else {
      rRegDBRet.m_itr = m_pdbrMap->end();    // Bad insertion
      rRegDBRet.SetInvalid();
      ostringstream oss;
      oss << rRegDBRet.m_PrimaryKey;
      return LogReturnCode(eRegNoInsertAtKey, "InsertByKey - " + oss.str(), LOG_WARNING);
   }
}  // End of CRegistrarDatabase::InsertByKey

//=============================================================================
// CRegistrarDatabase::Insert
//    Insert a record.
//       If new, create a key for it
//       If already exists, the lock's keys must match - either both locked
//          with the same key, or both unlocked
// Details:
//    dbKey is either the key to use or NeedNewKey, in which case a new key is
//       created and recorded
//    If a record with the passed in primary key already exists, delete it
//    Insert the record with its lock set to Unlocked
//       This implies UNLOCKING the record,
//       If it turns out that is not the desired behavior (implementation routine
//          implementing policy), then the fix is to either pass the lock state
//          into InsertByKey, or reset the lock state later.
// What does it mean to insert?
//    1) Primary key already exists
//       Only allowed if the record has been locked, and this Insert operation
//          (or Put operation) has the correct lock
//    2) Primary key of inserted record (not NeedNewKey) does not exist in the database
//       Should never happen - either it is a record,
//          in which case it is in the database, or it is not,
//          in which case it should come in via Register (new Primary Key)
//       Get/Commit cannot modify the primary key
//       If it does happen, put it in a new record, getting a new key and using it
//    3) Insert and assign a new Primary key - e.g. this is a true Insert New
//       Assign new primary key - increment in member variable, and increment
//          in database - could do it lazily
// Returns:
//    rRegDBRet.m_PrimaryKey is the key that was used to insert the record
//    rRegDBRet.m_LockKey set to
//=============================================================================
eReg CRegistrarDatabase::Insert(NamedValueSet const &rNew, CRegDB &rRegDBRet)
{
   AutoLock(this);

   // Currently a placeholder. Most of the documented functionality belongs
   //    elsewhere, so it is not clear that it is needed here.
   // NeedNewKey functionality is provided by Register
   // Overwriting functionality requires a Get/Commit
   return InsertByKey(rNew, rRegDBRet);
}  // End of CRegistrarDatabase::Insert

//=============================================================================
// CRegistrarDatabase::Register
//    Insert a NEW record.
// Flow:
//    If record is a duplicate, log error and do not insert it.
//    Otherwise, create a new key and directly insert it unlocked
// Returns:
//    rRegDBRet.m_PrimaryKey is the key that was used to insert the record
//    rRegDBRet.m_LockKey set to
//=============================================================================
eReg CRegistrarDatabase::Register(NamedValueSet const &rconstNew, CRegDB &rRegDBRet)
{
   AutoLock(this);
   const char fName[] = "Register";

   if (NULL == m_pdbrMap){                   // passed in bad parameter. Never expected, just defensive
      rRegDBRet.SetInvalid();
      return LogReturnCode(eRegDBNotLoaded, fName, LOG_ERR);
   }

   NamedValueSet rNew = rconstNew;           // get a copy

   // If there is already a Record Number, remove it to avoid searching on it.
   // Also, there just should not be one.
   if (rNew.Has( keyRegRecordNum)) {
      AAL_WARNING(LM_Database," CRegistrarDatabase::Register NVS contains a keyRegRecordNum,"
            " but should not, and it is removed. NVS in error is:" << rNew);
      rNew.Delete( keyRegRecordNum);
   }

   if (fFindExactBegin(rNew, rRegDBRet)) {   // If it's value is already in the database, do not insert. Duplicates not allowed.
      rRegDBRet.SetFlag(m_pdbrMap);
      return LogReturnCode(eRegDuplicateRecord, fName, LOG_NOTICE);
   }

   // To get a new key just use m_NextDatabaseKey, and increment it.
   // However, someday it might wrap (unlikely, 64 bits, but have to guard against it).
   // If it wraps have to:
   // 1) Make sure never uses 0 - not a valid number due to file read in restrictions
   // 2) Make sure it does not conflict with other keys already in the database
   while( FindByKey(m_NextDatabaseKey, rRegDBRet.m_itr) ){
      ++m_NextDatabaseKey;
      if ( 0 == m_NextDatabaseKey ){         // Don't allow zero
         ++m_NextDatabaseKey;
      }
   }

   rRegDBRet.m_PrimaryKey = m_NextDatabaseKey++;   // Note post-increment
   rRegDBRet.m_LockKey    = Unlocked;              // Start out unlocked
   UpdateStateRecord();                            // Remember new key values

   // Put the record number in the record
   rNew.Add( keyRegRecordNum, rRegDBRet.m_PrimaryKey);

   return InsertByKey( rNew, rRegDBRet);           // Insert the record, and update the validity flag in the RegDB

}  // End of CRegistrarDatabase::Register

//=============================================================================
// CRegistrarDatabase::CheckLock
//    Worker function to check various key situations
// Return value:
//    eRegOK if the keys are the same (both Unlocked, or both Locked with the same key)
//    Other values of eReg otherwise
//=============================================================================
eReg CRegistrarDatabase::CheckLock(LockKey_t lockToCheck, LockKey_t lockOnRecord)
{
   if ( lockOnRecord == lockToCheck ) {                  // Normal case, locks match
      return eRegOK;
   }
   // the locks did not match - it might be a security violation or a logic error

   const char fName[] = "CheckLock";
   ostringstream oss;

   // if the record was not locked, then a lock was passed in to delete an already unlocked record.
   if ( Unlocked == lockOnRecord ) {
      oss << " Checked Key: " << lockToCheck;
      return LogReturnCode(eRegKeyNotNeeded, fName + oss.str(), LOG_WARNING);
   } else if ( Unlocked == lockToCheck ) {
      // if the record was locked but the key was not, then someone was trying to delete a locked record
      oss << " Record's Key: " << lockOnRecord;
      return LogReturnCode(eRegNeedKey, fName + oss.str(), LOG_WARNING);
   }

   // if the key and record lock were both non-zero and did not match, someone was trying to delete
   oss << " Checked Key: " << lockToCheck << "Record's Key" << lockOnRecord;
   return LogReturnCode(eRegWrongKey, fName + oss.str(), LOG_WARNING);
}  // End of CRegistrarDatabase::CheckLock

//=============================================================================
// CRegistrarDatabase::Delete
//    Delete the NVS pointed to by the PrimaryKey.
//    The iterator will be set to end(), and is not valid
//    The lock in pRegDBRet must match the lock on the record. In particular,
//       if the record is not locked, then the pRegDBRet key must be 0.
// Return value:
//    True if the entry was found and deleted, false otherwise
//    False could be because it was not found, or it was locked and the
//       was not correct
//=============================================================================
eReg CRegistrarDatabase::Delete(CRegDB &rRegDBRet)
{
   AutoLock(this);
   const char fName[] = "Delete";

   if ( NULL == m_pdbrMap ) {                      // passed in bad parameter. Never expected, just defensive
      rRegDBRet.SetInvalid();
      return LogReturnCode( eRegDBNotLoaded, fName, LOG_ERR );
   }

   DatabaseKey_t PrimaryKey = rRegDBRet.m_PrimaryKey; // Remember for deletion, below
   rRegDBRet.m_itr = m_pdbrMap->find( PrimaryKey );   // find the entry by primary key

   if ( rRegDBRet.m_itr != m_pdbrMap->end() ) {    // if the entry exists

      eReg eRetVal = CheckLock( rRegDBRet.m_LockKey, ( *rRegDBRet.m_itr ).second.m_Lock );

      if ( eRegOK == eRetVal) {                    // Normal case, locks match
         m_pdbrMap->erase( rRegDBRet.m_itr );
         rRegDBRet.m_LockKey = Unlocked;           // Unlock the reference
         rRegDBRet.m_itr = m_pdbrMap->end();       // Initialize the now invalid iterator to a safe value
         rRegDBRet.SetInvalid();
         std::string FullFileName = FullFilePathFromDBKey (PrimaryKey);
         remove(FullFileName.c_str());             // Delete the file
      }
      return eRetVal;                              // Locks did not match, record the problem and get out
   } else {
      // did not find it at all
      ostringstream oss;
      oss << " - Key: " << rRegDBRet.m_PrimaryKey;
      rRegDBRet.SetInvalid();
      return LogReturnCode( eRegNoFindByKey, fName + oss.str(), LOG_WARNING);
   }
}  // End of CRegistrarDatabase::Delete

//=============================================================================
// CRegistrarDatabase::Commit
//    Replace DBRec pointed to by rRegDBRet.m_PrimaryKey
// Flow
//    Find and delete the record. All handled by Delete:
//       Find the record
//       Check that lock keys match. If not - error like delete.
//       Delete the NVS pointed to by the PrimaryKey.
//    Insert the new NVS - no possibility of collision by key
//    However, the passed in value could be a duplicate of a record already
//       in the database. In that case, we cannot have duplicates, so
//       return the other record with the returned key unlocked.
// Return value:
//    eRegOK if all went well, an appropriate error code if not, associated
//       deleting or inserting
//=============================================================================
eReg CRegistrarDatabase::Commit(NamedValueSet const &rNew, CRegDB &rRegDBRet)
{
   eReg eRetVal = Delete(rRegDBRet);               // If found by key and locks match, delete

   if (eRegOK == eRetVal) {                        // If successfully found and deleted
      if (!fFindExactBegin( rNew, rRegDBRet)) {    // And if the value is not a duplicate
         eRetVal = InsertByKey( rNew, rRegDBRet);  // then insert it and write it to disk
      }
      rRegDBRet.m_LockKey = Unlocked;              // The lock has been used up
   }
   return eRetVal;
}  // End of CRegistrarDatabase::Commit

//=============================================================================
// CRegistrarDatabase::InitRegDB
//    Initialize the RegDB iterator. Not required, but safest because
//       code cannot detect an uninitialized RegDB
//=============================================================================
// void CRegistrarDatabase::InitRegDB(CRegDB* pRegDBRet)
// {
//    pRegDBRet->m_itr = m_pdbrMap->end();
// }  // End of CRegistrarDatabase::InitRegDB

//=============================================================================
// CRegistrarDatabase::NVS
//    Return a copy of the record pointed to by CRegDB
//=============================================================================
//NamedValueSet CRegistrarDatabase::NVS ( const CRegDB& rRegDB)
//{
//   return (*rRegDB.m_itr).second.m_nvs;
//}  // End of CRegistrarDatabase::Record

//=============================================================================
// CRegistrarDatabase::UpdateStateRecord
//    Update the state record in the database with the relevant member variables.
//    Need a special case for internal records, e.g. the state record. Otherwise,
//       internal logic will have to lock it, update it, insert (which is delete+insert).
// Suggest special update routing that:
//    Get the iterator of the state record
//    Directly update its internal fields (see Open code for example)
//    Mark it modified
//    Write it to disk
//=============================================================================
btBool CRegistrarDatabase::UpdateStateRecord()
{
   // Find the state record
   dbrMap_itr_t itr;                         // points to a DBRec
   if ( !FindByKey(StateNVSPrimaryKey, itr) ) {
      return false;
   }
   (*itr).second.m_nvs.Delete(NextLockKeyName);
   (*itr).second.m_nvs.Add(NextLockKeyName, m_NextLockKey);

   (*itr).second.m_nvs.Delete(NextDatabaseKeyName);
   (*itr).second.m_nvs.Add(NextDatabaseKeyName, m_NextDatabaseKey);

   (*itr).second.m_bModified = true;
   FlushRecordToDisk(itr);

   return true;
}  // End of CRegistrarDatabase::UpdateStateRecord

//=============================================================================
// LogReturnCode
//    cerr or syslog the errors found
// Inputs:
//    Error is from eReg typedef
//    sComment is the name of the routine and any other info needed
//    Level is the syslog level
//=============================================================================
eReg CRegistrarDatabase::LogReturnCode(eReg Error, std::string sComment, unsigned Level)
{
   AAL_LOG(Level,LM_Database,"CRegistrarDatabase::" << sComment << Error << endl);
   return Error;
} // End of CRegistrarDatabase::LogReturnCode


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


