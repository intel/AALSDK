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
/// @file RegDBProxy.cpp
/// @brief External char*-based interface to a Database Daemon
///        Provides utility functions used by both sides, will likely be
///        included in multiple executables, or become a .so by itself.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///         Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/19/2008     HM       To hold implementation of "class" functions for
///                            RegistrarCmdResp_t
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 08/09/2008     HM       Integrating with new kernel driver mechanism
/// 08/10/2008     HM       Moved to AASResourceManager as dedicated file
/// 09/02/2008     HM       Added pContext to operator << RegistrarCmdResp_t
/// 09/12/2008     HM       Major checkin for new remote database
/// 11/09/2008     HM       Added eRegCannotWriteOpenFile
/// 11/14/2008     HM       Fixed read_nvs_from_RCP to handle embedded nulls
///                            in the NamedValueSet being read in.
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/registrar/CAASRegistrar.h"   // pulls in RegDBProxy.h


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)


//=============================================================================
// Name: RegistrarCmdResp_Destroy
// Description: Dtor for struct RegistrarCmdResp_t
//=============================================================================
pRegistrarCmdResp_t RegistrarCmdResp_Destroy(pRegistrarCmdResp_t pRC)
{
   delete [] reinterpret_cast<char*>(pRC);
   return NULL;
}  // End of RegistrarCmdResp_Destroy

//=============================================================================
// Name: operator << on RegistrarCmdResp_t
// Description: Prints a RegistrarCmdResp_t
//=============================================================================
std::ostream & operator << (std::ostream &s, const RegistrarCmdResp_t &rc)
{
   if (!strcmp(rc.szSignature, RegistrarCmdCommandSignature)) {  // If it is a valid RegistrarCommand
      s << std::uppercase << std::hex << std::showbase <<
           "RegistrarCommand @ " << reinterpret_cast<const void*>(&rc) <<
           "\n\tCommand: " << rc.Command << std::endl;
   }
   else if (!strcmp(rc.szSignature, RegistrarCmdResponseSignature)) {
      s << std::uppercase << std::hex << std::showbase <<
           "RegistrarResponse @ " << reinterpret_cast<const void*>(&rc) <<
           "\n\tCommand: " << rc.Command <<
           "\n\tReturned Value: " << rc.Response << std::endl;
   }
   else {
      s << "Is not a valid RegistrarCmdResp!\n";
      return s;
   }
   s << std::uppercase << std::hex << std::showbase <<
      "\t" << rc.TranID <<
      "\tpFunc:            " << rc.pFunc          <<
      "\tLength of NVS:    " << rc.LengthOfNVS    <<
      "\tLength of Struct: " << rc.LengthOfStruct <<
      "\tPrimaryKey:       " << rc.Key            <<
      "\tLock:             " << rc.Lock           << std::endl;
   if( rc.LengthOfNVS ){
      s << "\tNVS:\n" << rc.szNVS;
   } else {
      s << "\tNo NVS in structure.\n";
   }

   return s;
}

//=============================================================================
// Name: operator << on an eReg
// Description: Prints an eReg
//=============================================================================
std::ostream & operator << (std::ostream &s, const eReg &Error)
{
   struct eRetToString {
      eReg           eCode;
      std::string    sCode;
   };
   const struct eRetToString eRetToStringMap[] = {
      {eRegOK,                   " Operation succeeded. "},
      {eRegDBNotLoaded,          " Database not loaded. "},
      {eRegDBAlreadyLoaded,      " Database already allocated, but should not be. "},
      {eRegOpenNoPath,           " No path either passed in or available from construction. "},
      {eRegOpenScanError,        " Scandir could not find or open the directory. "},
      {eRegOpenNoStartup,        " Open could not create the default state record. "},
      {eRegOpenBadFile,          " Cannot open a scandir'd file. "},
      {eRegOpenCannotReadNVSRec, " Cannot read an NVS record from the file, possibly corrupt. "},
      {eRegNoInsertAtKey,        " Could not insert at the provided key. "},
      {eRegCannotWriteFile,      " Could not open file for writing, file not written. "},
      {eRegDuplicateRecord,      " Attempt to insert a record that already exists. "},
      {eRegKeyNotNeeded,         " Attempt to match valid key against unlocked record. "},
      {eRegNeedKey,              " Attempt to access locked record with no key. "},
      {eRegWrongKey,             " Attempt to access locked record with wrong key. "},
      {eRegDoubleLock,           " Attempt to lock an already locked record. "},
      {eRegNoFindByKey,          " Could not find the expected key. "},
      {eRegNoFindByPattern,      " Could not find the expected pattern. "},
      {eRegCannotWriteOpenFile,  " File is open, but could not write to it. "},
      {eRegEndNoComma,           ""}
   };

   if (Error >= eRegOK && Error < eRegEndNoComma) {
      s << eRetToStringMap[Error].sCode;
   }
   else {
      s << " eReg Error code is " << static_cast<unsigned>(Error) << ", but eReg operator << is out of date or the field is invalid. ";
   }

   return s;
}

//=============================================================================
// Name: operator << on an eCmd
// Description: Prints an eCmd
//=============================================================================
std::ostream & operator << (std::ostream &s, const eCmd &Command)
{
   struct eCmdToString {
      eCmd           eCode;
      std::string    sCode;
   };
   const struct eCmdToString eCmdToStringMap[] = {
      {eCmdNoOp,                 " No Op "},
      {eCmdOpen,                 " Open "},
      {eCmdClose,                " Close "},
      {eCmdRegister,             " Register "},
      {eCmdDelete,               " Delete "},
      {eCmdGetByKey,             " GetByKey "},
      {eCmdGetByPattern,         " GetByPattern "},
      {eCmdCommit,               " Commit "},
      {eCmdFindExactBegin,       " FindExactBegin "},
      {eCmdFindExactNext,        " FindExactNext "},
      {eCmdFindSubsetBegin,      " FindSubsetBegin "},
      {eCmdFindSubsetNext,       " FindSubsetNext "},
      {eCmdDumpDatabase,         " DumpDatabase "},
      {eCmdIsOK,                 " IsOK "},
      {eCmdEndNoComma,           ""}
   };

   if (Command >= eCmdNoOp && Command < eCmdEndNoComma) {
      s << eCmdToStringMap[Command].sCode;
   }
   else {
      s << " eCmd Command code is " << static_cast<unsigned>(Command) << ", but not found. ";
   }

   return s;
}

//=============================================================================
// Name: read_nvs_from_RCP
// Description: read an NVS from a RegistrarCmdResp
// Use like this:
//    NamedValueSet nvs;
//    pRegistrarCmdResp_t p;
//    AAL::AAS::read_nvs_from_RCP (p, &nvs)
//=============================================================================
void read_nvs_from_RCP(const pRegistrarCmdResp_t pRCR, NamedValueSet *pnvs)
{
   if (pRCR->LengthOfNVS) {
//      std::istringstream iss (pRCR->szNVS);
//      iss >> *pnvs;
       std::string s(pRCR->szNVS, pRCR->LengthOfNVS); // length provided constructor allows embedded nulls
       std::istringstream iss (s);
       iss >> *pnvs;
   }
}


   END_NAMESPACE(AAS) 
END_NAMESPACE(AAL) 


