// Copyright (c) 2008-2014, Intel Corporation
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
/// @file RegDBProxy.h
/// @brief External char*-based interface to a Database Daemon.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2008     HM       Initial version started
/// 06/11/2008     HM       Fixes to go with RegistrarDatabase updates
/// 06/19/2008     HM       Renamed to RegDBProxy as top of stub/skeleton tree
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 08/07/2008     HM       Modifications for Resource Manager Daemon
/// 09/02/2008     HM       Added pContext to RegistrarCmdResp_t
/// 09/12/2008     HM       Major checkin for new remote database
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_REGISTRAR_REGDBPROXY_H__
#define __AALSDK_REGISTRAR_REGDBPROXY_H__

//#define DEBUG_REGISTRAR /* if debugging Registrar operations, enables DumpDatabase*/
//#define DEBUG_DATABASE  /* if debugging Database operations */

#include <aalsdk/AALTransactionID.h>    // Pulls in AALTypes.h, also used herein
#include <aalsdk/AALNamedValueSet.h>

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

      const char RegistrarCmdCommandSignature[]  = "RegCmd\n";     // These two must be the same length, and fit into
      const char RegistrarCmdResponseSignature[] = "RegRsp\n";     // the RegistrarCmdResp::szSignature[]
      const size_t lenRegistrarCmdRespSignature  = sizeof(RegistrarCmdCommandSignature);

      // Registrar Database internal types that the Proxy must be able to marshal
      typedef btUnsigned64bitInt    DatabaseKey_t;
      typedef btUnsigned64bitInt    LockKey_t;
      // Special values of these types
      const LockKey_t      Unlocked              = 0;                // When a lock value == 0, the lock or key is turned off
      const DatabaseKey_t  NoKey                 = 0;                // Not a valid primary key value

      // Various shared constants
      const btNumberKey RegistrarDatabaseOpenPathName = 1;

      // Available RegistrarDatabase Commands
      typedef enum eCmd {
         eCmdNoOp=0,                // Place holder
         eCmdOpen,                  // Open(pszPathName)
         eCmdClose,
         eCmdRegister,
         eCmdDelete,
         eCmdGetByKey,
         eCmdGetByPattern,
         eCmdCommit,
         eCmdFindExactBegin,
         eCmdFindExactNext,
         eCmdFindSubsetBegin,
         eCmdFindSubsetNext,
         eCmdDumpDatabase,
         eCmdIsOK,
         eCmdEndNoComma             // INSERT BEFORE THIS to handle the trailing non-comma
      } eCmd;
      std::ostream& operator << (std::ostream& s, const eCmd& Command);  // printout of an eCmd

      // Return codes from various RegistrarDatabase routines. Logged in LogReturnCode.
      typedef enum eReg {
         eRegOK=0,                  // Good return
         eRegDBNotLoaded,           // Database memory not allocated
         eRegDBAlreadyLoaded,       // Database already allocated, but should not be
         eRegOpenNoPath,            // No path either passed in or available from construction
         eRegOpenScanError,         // Scandir could not find or open the directory
         eRegOpenNoStartup,         // Open could not create the base state record
         eRegOpenBadFile,           // Cannot open a scandir'd file
         eRegOpenCannotReadNVSRec,  // Cannot read an NVS from the record, possibly corrupt file
         eRegNoInsertAtKey,         // Could not insert at the provided key
         eRegCannotWriteFile,       // Could not open file for writing, file not written
         eRegDuplicateRecord,       // Attempt to insert a record that already exists.
         eRegKeyNotNeeded,          // Attempt to match valid key against unlocked record.
         eRegNeedKey,               // Attempt to access locked record with no key
         eRegWrongKey,              // Attempt to access locked record with wrong key
         eRegDoubleLock,            // Attempt to lock an already locked record
         eRegNoFindByKey,           // Could not find the expected key
         eRegNoFindByPattern,       // Could not find the expected pattern
         eRegCannotWriteOpenFile,   // File is open, but could not write to it
         eRegEndNoComma             // INSERT BEFORE THIS to handle the trailing non-comma
      } eReg;
      std::ostream& operator << (std::ostream& s, const eReg& Error);     // printout of an eReg


      //=============================================================================
      // RegistrarCmdResp_t is the target of a serialization for marshalling.
      //    It cannot be a true class because if it is then it cannot have the
      //    terminating szNVS. As a non-class, it does not have ctor/dtor & member
      //    functions (otherwise it needs a function table pointer, and as a struct
      //    that would hose up the binary interface). Net result, class-type ctor &
      //    dtor and member functions are declared and implemented without being
      //    scoped by a class - thus the strange naming.
      // Changes to this structure may need to be reflected in RegistrarCommand_New
      //    and RegistrarResponse_New. If, e.g., padding were added by the compiler
      //    after szSignature, those routines would break. Similarly if the ordering
      //    of szSignature as the first item and TranID as the second were changed.
      //=============================================================================
      typedef struct RegistrarCmdResp* pRegistrarCmdResp_t;          // forward reference
      class CRegistrar;                                              // forward reference
      typedef void (CRegistrar::* pRegRetFunc)(pRegistrarCmdResp_t); // pointer to member function

      typedef struct RegistrarCmdResp {
         char           szSignature[lenRegistrarCmdRespSignature];   // 8, Identification, human readable
         TransactionID  TranID;                                      // A true (binary) AAL TransactionID
         pRegRetFunc    pFunc;                                       // Pointer to Function to call upon return
         DatabaseKey_t  Key;                                         // Database Key
         LockKey_t      Lock;                                        // Database Lock
//         void          *pContext;                                    // Extra pointer for use when this is passed through an adapter layer
//         int            iContext;                                    // Union disambiguator for the pContext, defines type of extra pointer
         unsigned       LengthOfNVS;                                 // Once allocated, specifies length of szNVS[], in bytes
         unsigned       LengthOfStruct;                              // Length of this structure
         eCmd           Command;                                     // Command to execute
         eReg           Response;                                    // Response to that Command

#if defined( _MSC_VER )
# pragma warning( push )
# pragma warning( disable:4200 )                          // zero-sized array, no default copy/assignment ctor possible
#endif // _MSC_VER

         char           szNVS[];                                     // Serialized (human readable) NVS

#if defined( _MSC_VER )
# pragma warning( pop )
#endif // _MSC_VER

      } RegistrarCmdResp_t/*, *pRegistrarCmdResp_t*/;
      pRegistrarCmdResp_t    RegistrarCmdResp_Destroy   (pRegistrarCmdResp_t pRC);   // dtor for a struct RegistrarCmdResp. always returns NULL
      std::ostream&          operator << (std::ostream& s, const RegistrarCmdResp_t& dbr);     // printout of a RegistrarCmdResp

      //=============================================================================
      // NamedValueSet I/O routines for RegistrarCmdResp
      //=============================================================================
      void read_nvs_from_RCP (const pRegistrarCmdResp_t pRCR, NamedValueSet* pnvs);


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif // __AALSDK_REGISTRAR_REGDBPROXY_H__

