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
/// @file AASRegistrar.h
/// @brief AASRegistrar - Public Interface to Registrar.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/02/2007     HM       Initial version started
/// 08/22/2007     HM       Removed most events, corrected the ones left
/// 08/23/2007     HM       More corrections
/// 08/25/2007     HM       Heavy changes to DBRecord to deal with non-constness
/// 08/26/2007     HM       Ambiguated DBRecord.NVS to make the interface cleaner
/// 10/18/2007     HM       Added AAL_AAS_RECORD_NAME
/// 05/08/2008     HM       Comments & License
/// 06/14/2008     HM       Added DEBUG_REGISTRAR & DumpDatabase enabled by it
/// 07/03/2008     HM       Removed restrictions on DumpDatabase
/// 07/03/2008     HM       Added GetCopyOfFindResult(), GetCopyOfDBRecord()
/// 01/04/2009     HM       Updated Copyright
/// 04/25/2012     HM       Added AASREGISTRAR_API to virtual interfaces@endverbatim
//****************************************************************************
#ifndef __AALSDK_REGISTRAR_AASREGISTRAR_H__
#define __AALSDK_REGISTRAR_AASREGISTRAR_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

const char RegistrarRecordName[] = "__RecordName";          // Internal use, to be deprecated
const char RegistrarPathKeyName[] = "RegistrarPathKeyName"; // Pass in optArgs of Open() to change the registry path
                                                            // DEPRECATED: no longer externally available

//=============================================================================
// Name: IFindResult
// Description: Represents the record found when probing the Registrar database
//    and the state required to get the next matching one
//=============================================================================
class AASREGISTRAR_API IFindResult
{
public:
   // Retrieve an R/O reference to record
   virtual const NamedValueSet & NVS()         const = 0;
   // Create a new copy of the IFindResult.
   // Use this to access the IFindResult outside of the Event Handler function.
   // Must delete the result when done with it.
   virtual IFindResult * GetCopyOfFindResult() const = 0;
   // Delete the iterator
   virtual ~IFindResult() {}
}; // class IFindResult


//=============================================================================
// Name: IDBRecord
// Description: Represents the LOCKED record found when probing the Registrar
//    database
//=============================================================================
class AASREGISTRAR_API IDBRecord
{
public:
   // Get R/W reference to record
   virtual NamedValueSet & NVS() = 0;
   // Overwrite the record
   virtual NamedValueSet & NVS(const NamedValueSet &rRecord) = 0;
   // Create a new copy of the IDBRecord.
   // Use this to access the IDBRecord outside of the Event Handler function.
   // Must delete the result when done with it.
   virtual IDBRecord * GetCopyOfDBRecord() const = 0;
   // Unlock the record and delete the reference
   virtual ~IDBRecord() {}
}; // class IDBRecord

//=============================================================================
// Constants and Events for methods in IRegistrar
//=============================================================================

// tranevtRegistrarRegister
class AASREGISTRAR_API IRegisterTransactionEvent
{
public:
   virtual btBool fRegisterSucceeded() const = 0;
   virtual ~IRegisterTransactionEvent() {}
};

// tranevtRegistrarOpen
class AASREGISTRAR_API IOpenTransactionEvent
{
public:
   virtual btBool fOpenSucceeded() const = 0;
   virtual ~IOpenTransactionEvent() {}
};

// tranevtRegistrarFind
class AASREGISTRAR_API IFindTransactionEvent
{
public:
   virtual       btBool        fFindSucceeded() const = 0;
   virtual const IFindResult & FindResult()     const = 0;
   virtual ~IFindTransactionEvent() {}
};

// tranevtRegistrarGet
class AASREGISTRAR_API IGetTransactionEvent
{
public:
   virtual btBool      fGetSucceeded() const = 0;
   virtual IDBRecord & DBRecord()            = 0;  // retrieve mutable reference
   virtual ~IGetTransactionEvent() {}
};

// tranevtRegistrarCommit
class AASREGISTRAR_API ICommitTransactionEvent
{
public:
   virtual       btBool        fCommitSucceeded() const = 0;
   virtual const IFindResult & FindResult()       const = 0;
   virtual ~ICommitTransactionEvent() {}
};

//=============================================================================
// Name: IRegistrar
// Description: Public interface to the Registrar
//=============================================================================
class AASREGISTRAR_API IRegistrar
{
public:
   virtual btBool IsOK() const = 0;

   // TO BE DEPRECATED - do not use
   virtual void Open(const NamedValueSet &rnvsOptArgs,
                     const TransactionID &rTransID=TransactionID()) = 0;

   // Close the database
   // TO BE DEPRECATED - do not use
   virtual void Close(const TransactionID &rTransID=TransactionID()) = 0;

   // Insert a new record in the Database
   virtual void Register(const NamedValueSet &rRecord,
                         const TransactionID &rTransID=TransactionID()) = 0;

   // Find an existing record in the Database
   virtual void Find(const NamedValueSet &rPattern,
                     const TransactionID &rTransID=TransactionID()) const = 0;

   // Find next existing record in the Database
   virtual void FindNext(const NamedValueSet &rPattern,
                         const IFindResult   &rFindResult,
                         const TransactionID &rTransID=TransactionID()) const = 0;

   // Retrieve a record for update based on a pattern
   virtual void Get(const NamedValueSet &rPattern,
                    const TransactionID &rTransID=TransactionID()) const = 0;

   // Retrieve a record for update based on
   //    the result of a previous find
   virtual void Get(const IFindResult   &rFindResult,
                    const TransactionID &rTransID=TransactionID()) const = 0;

   // Store a record after getting it
   virtual void Commit(const IDBRecord     &rRecord,
                       const TransactionID &rTransID=TransactionID()) = 0;

   // Remove an existing record in the Database
   //    after retrieving for update
   virtual void DeRegister(const IDBRecord     &rRecord,
                           const TransactionID &rTransID=TransactionID()) = 0;

   virtual ~IRegistrar() {}

   virtual void DumpDatabase() const = 0;
}; // class IRegistrar


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_REGISTRAR_AASREGISTRAR_H__

