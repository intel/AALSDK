// Copyright(c) 2007-2016, Intel Corporation
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
/// @file CRegistrarDatabase.h
/// @brief Define AALRegistrar-specific interface to a Database object.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/01/2007     HM       Initial version started
/// 08/26/2007     HM       Added record locking
/// 10/31/2007     HM       #ifndef __GNUC__ away various #pragmas
/// 12/16/2007     JG       Changed include path to expose aas/
/// 05/08/2008     HM       Comments & License
/// 06/11/2008     HM       Fixes to go with RegistrarDatabase updates
/// 06/27/2008     HM       Splitting Registrar from Database
/// 11/09/2008     HM       Deprecating Open/Close, and adding individual
///                            record flushing
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_RM_CREGISTRARDATABASE_H__
#define __AALSDK_RM_CREGISTRARDATABASE_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/AALEvent.h>


/// @todo Document CRegDB, CRegistrarDatabase, and related.

#if defined( __AAL_LINUX__ )
struct dirent;                      // Forward reference for use of scandir() parts
#endif // __AAL_LINUX__


BEGIN_NAMESPACE(AAL)


// Definition of NVS containing stored Database State information
const DatabaseKey_t  StateNVSPrimaryKey    = 1;                // This is the NVS record that will store the primary state
const char           NextLockKeyName[]     = "NextLock";       // Locks start here if not implemented as nonces
const LockKey_t      DefaultNextLock       = 1;
const char           NextDatabaseKeyName[] = "NextPrimaryKey"; // Primary key starts after StateNVSPrimaryKey
const DatabaseKey_t  DefaultNextPrimaryKey = 2;

class DBRec                               // A database record in the in-memory database, Primary Key is separate
{
public:
         DBRec();                         // NVS default constructor works here
virtual ~DBRec();
   NamedValueSet  m_nvs;                  // NamedValueSet constituting the record
   LockKey_t      m_Lock;                 // Unlocked means not locked, otherwise a nonce
   btBool         m_bModified;            // True if the record has been modified

   std::ostream & Print(std::ostream & ) const;
};

typedef std::map<DatabaseKey_t, DBRec> dbrMap_t;           // Map [primary key, NVS Database record]
typedef dbrMap_t::iterator             dbrMap_itr_t;

// Encapsulation of arguments to and return value from RegistrarDatabase functions.
// This is exposed to the clients of the database, and so they should not need to know
//    about database internals to use it. However, most properly it should be initialized
//    using the m_pdbrMap in the database.
// It is reasonable for all constructions of CRegDB to be passed the database that it
//    is associated with, but the below 'todo' shows another way that is more dynamic
//    and correct.
// TODO: make CRegDB friend of CRegistrarDatabase, and CRegistrarDatabase::m_pdbrMap protected
//    instead of private, and remove calls to SetFlag/SetInvalid and remove m_fValid.
//    Instead, only return NVS or NULL, where NVS is returned only if: m_pDB (from constructor)
//    is not NULL, and m_pDB->IsOK, and m_pDB->m_pdbrMap is not NULL,
//    and m_itr != m_pDB->m_pdbrMap->end(). Pretty verbose, but seems safer than expecting code
//    to correctly set the validity flag everywhere - although that code is already in.
// TODO: make it explicitly uncopyable
class CRegDB
{
public:
   DatabaseKey_t  m_PrimaryKey;
   LockKey_t      m_LockKey;
   btBool         m_fValid;      // True only when m_itr points to a valid NVS
   dbrMap_itr_t   m_itr;         // Used ONLY to provide retrieval of NVS within RegistrarDatabase for Find & Get
                                       // Constructor
         CRegDB();
virtual ~CRegDB();

   std::ostream & Print(std::ostream & ) const;

                                       // Determine if the pointer is valid, call only after setting m_itr
   void SetFlag(dbrMap_t *pdbrMap) { m_fValid = (m_itr != pdbrMap->end()); }
                                       // Force the flag to be invalid without involving the pdbrMap, e.g. if pdbrMap==NULL
   void SetInvalid() { m_fValid = false; }
                                       // Get a pointer to the NVS in the dbrMap
   const NamedValueSet * NVS() { return m_fValid ? &(*m_itr).second.m_nvs : NULL; }
};

//      std::ostream& operator << (std::ostream& s, const CRegDB& dbr, int indent);

//=============================================================================
// Name: CRegistrarDatabase Interface definition
// Description: Concrete class - Registrar's interface to the Database
// NOTE:
//    The protocol for manipulating the database follows:
//       Open()
//       Register inserts completely new records.
//       Find finds records by value. Client should never be retrieving by Key.
//       Get finds records by value, or can use Key invisibly returned by Find.
//          Get locks the record for update or deletion.
//       Delete only works after a Get.
//       Commit is update, only works after a Get
//       Close().
//=============================================================================
class CRegistrarDatabase: public CriticalSection
{
private:
   std::string    m_DatabasePath;   // Database directory path
   dbrMap_t      *m_pdbrMap;        // In-memory database, if NULL, then no database loaded
   btBool         m_bIsOK;          // If constructor worked
   DatabaseKey_t  m_NextDatabaseKey;
   LockKey_t      m_NextLockKey;    // Nonce would be better, but this will suffice
   NamedValueSet  m_OptArgs;        // Optional Arguments passed to Constructor

   // Disallow copy and assignment
   CRegistrarDatabase(const CRegistrarDatabase & );
   CRegistrarDatabase & operator = (const CRegistrarDatabase & );

   // Internal worker routines
   eReg     CheckLock          (LockKey_t lockToCheck, LockKey_t lockOnRecord);    // check keys and log problems
   eReg     Find               (NamedValueSet const &rPattern, CRegDB &rRegDBRet,  // General find by NVS value, others are calls to this
                                btBool fSubset = true, btBool fBegin = true);
   btBool   FindByKey          (DatabaseKey_t dbKey, dbrMap_itr_t &itr);           // simple retrieval
   eReg     FlushRecordToDisk  (const dbrMap_itr_t &itr);                          // Write a specific modified record to disk
   eReg     FlushDatabaseToDisk();                                                 // Write all modified records to disk
   eReg     Insert             (NamedValueSet const &rNew, CRegDB &rRegDBRet);     // Insert a record that has a key, without deletion
   eReg     InsertByKey        (NamedValueSet const &rNew, CRegDB &rRegDBRet);     // no checking
   eReg     LoadFromDisk       (struct dirent **pNameList, int nNameList);
   eReg     LogReturnCode      (eReg Error, std::string sComment, unsigned Level); // log errors
   //NamedValueSet NVS     ( const CRegDB& rRegDB);                                // Return a copy of the record pointed to by CRegDB
   btBool   UpdateStateRecord  ();                                                 // write RegistrarDatabase state to record in database for saving
   btBool   fFindExactBegin    (NamedValueSet const &rPattern, CRegDB &rRegDBRet)
                               { return eRegOK == FindExactBegin(rPattern,rRegDBRet); }
   std::string FullFilePathFromDBKey(DatabaseKey_t dbKey);

public:
   CRegistrarDatabase(NamedValueSet* pOptArgs = NULL);     // default is empty path, but can carry anything
   ~CRegistrarDatabase();
   btBool IsOK() { return m_bIsOK; }

   eReg Open           (const NamedValueSet &rnvsOptArgs); // Load the database from directory
   eReg Close          ();                                                 // Flush modified records to disk and remove all records from memory
   eReg Register       (NamedValueSet const &rNew, CRegDB &rRegDBRet);     // Insert a completely new record with a new key
   eReg Delete         (CRegDB &rRegDBRet);                                // Delete DBRec pointed to by rRegDBRet.m_PrimaryKey
   eReg GetByKey       (CRegDB &rRegDBRet);                                // Get a Lock on DBRec pointed to by rRegDBRet.m_PrimaryKey
   eReg GetByPattern   (NamedValueSet const &rPattern, CRegDB &rRegDBRet); // Get a Lock on DBRec found to match rPattern
   eReg Commit         (NamedValueSet const &rNew, CRegDB &rRegDBRet);     // Replace DBRec pointed to by rRegDBRet.m_PrimaryKey

   eReg FindExactBegin (NamedValueSet const &rPattern, CRegDB &rRegDBRet)
                       { return Find(rPattern, rRegDBRet, false, true ); }
   eReg FindExactNext  (NamedValueSet const &rPattern, CRegDB &rRegDBRet)
                       { return Find(rPattern, rRegDBRet, false, false); }
   eReg FindSubsetBegin(NamedValueSet const &rPattern, CRegDB &rRegDBRet)
                       { return Find(rPattern, rRegDBRet, true , true ); }
   eReg FindSubsetNext (NamedValueSet const &rPattern, CRegDB &rRegDBRet)
                       { return Find(rPattern, rRegDBRet, true , false); }
   // Debug functions
   void DumpDatabase() const;                                              // Print database to cout
   std::ostream & DumpDatabase(std::ostream &ross) const;                  // Print database to stream
}; // class CRegistrarDatabase

std::ostream & operator << (std::ostream & , const DBRec & );
std::ostream & operator << (std::ostream & , const CRegDB & );
std::ostream & operator << (std::ostream & , const CRegistrarDatabase & );

END_NAMESPACE(AAL)

std::ostream & operator << (std::ostream & , const AAL::DBRec & );
std::ostream & operator << (std::ostream & , const AAL::CRegDB & );
//std::ostream & operator << (std::ostream & , const AAL::CRegistrarDatabase & );

#endif // __AALSDK_RM_CREGISTRARDATABASE_H__

