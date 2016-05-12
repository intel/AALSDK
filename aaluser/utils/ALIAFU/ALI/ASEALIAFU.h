// Copyright(c) 2015-2016, Intel Corporation
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
/// @file HWALIASE.h
/// @brief Definitions for ALI Hardware AFU Service.
/// @ingroup ALI
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/11/2016     HM       Initial version.@endverbatim
//****************************************************************************

#ifndef __ASEALIAFU1000_H__
#define __ASEALIAFU1000_H__

#include "ALIBase.h"
#include "aalsdk/kernel/ccip_defs.h"

// Buffer information structure
struct buffer_t_ase                   //  Descriptiion                    Computed by
{                                 // --------------------------------------------
  int fd_app;                     // File descriptor                 |   APP
  int fd_ase;                     // File descriptor                 |   SIM
  int index;                      // Tracking id                     | INTERNAL
  int valid;                      // Valid buffer indicator          | INTERNAL
  int metadata;                   // MQ marshalling command          | INTERNAL
  char memname[40]; // Shared memory name              | INTERNAL
  uint32_t memsize;               // Memory size                     |   APP
  uint64_t vbase;                 // SW virtual address              |   APP
  uint64_t pbase;                 // SIM virtual address             |   SIM
  uint64_t fake_paddr;            // unique low FPGA_ADDR_WIDTH addr |   SIM
  uint64_t fake_paddr_hi;         // unique hi FPGA_ADDR_WIDTH addr  |   SIM
  int is_privmem;                 // Flag memory as a private memory |
  int is_csrmap;                  // Flag memory as DSM              |
  int is_umas;                    // Flag memory as UMAS region      |
  struct buffer_t_ase *next;
};



BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///

class  CASEALIAFU : public CALIBase,
                    public IALIMMIO,
                    public IALIBuffer,
                    public IALIUMsg,
                    public IALIReset
{
public :

   CASEALIAFU( IBase *pSvcClient,
               IServiceBase *pServiceBase,
               TransactionID transID);


   ~CASEALIAFU()  { };

   btBool ASEInit();
   btBool ASERelease();

   // <IALIMMIO>
   virtual btVirtAddr   mmioGetAddress( void );
   virtual btCSROffset  mmioGetLength( void );

   virtual btBool  mmioRead32( const btCSROffset Offset,       btUnsigned32bitInt * const pValue);
   virtual btBool  mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value);
   virtual btBool  mmioRead64( const btCSROffset Offset,       btUnsigned64bitInt * const pValue);
   virtual btBool  mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value);
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                          NamedValueSet const &rInputArgs,
                                          NamedValueSet       &rOutputArgs );
   // overloaded version without rOutputArgs
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                          NamedValueSet const &rInputArgs );
   virtual btBool  mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                         NamedValueSet const &rInputArgs,
                                         NamedValueSet       &rOutputArgs );
   // overloaded version without rOutputArgs
   virtual btBool  mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                         NamedValueSet const &rInputArgs );
   // </IALIMMIO>

   // <IALIBuffer>
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr ) { return bufferAllocate(Length, pBufferptr, AAL::NamedValueSet()); }
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr,
                                             NamedValueSet const &rInputArgs )
   {
      NamedValueSet temp = NamedValueSet();
      return bufferAllocate(Length, pBufferptr, rInputArgs, temp);
   }
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr,
                                             NamedValueSet const &rInputArgs,
                                             NamedValueSet       &rOutputArgs );
   virtual AAL::ali_errnum_e bufferFree( btVirtAddr           Address);
   virtual btPhysAddr bufferGetIOVA( btVirtAddr Address);
   // </IALIBuffer>

   // <IALIUMsg>
   virtual btUnsignedInt umsgGetNumber( void );
   virtual btVirtAddr   umsgGetAddress( const btUnsignedInt UMsgNumber );
   virtual void          umsgTrigger64( const btVirtAddr pUMsg,
                                        const btUnsigned64bitInt Value );
   virtual bool      umsgSetAttributes( NamedValueSet const &nvsArgs);
   // </IALIUMsg>

   // <IALIReset>
   virtual e_Reset afuQuiesceAndHalt( void ) { return afuQuiesceAndHalt(NamedValueSet()); }
   virtual e_Reset afuQuiesceAndHalt( NamedValueSet const &rInputArgs );
   virtual e_Reset afuEnable( void ) { return afuEnable(NamedValueSet()); }
   virtual e_Reset afuEnable( NamedValueSet const &rInputArgs);
   virtual e_Reset afuReset( void ) { return afuReset(NamedValueSet()); }
   virtual e_Reset afuReset( NamedValueSet const &rInputArgs );
   // </IALIReset>

protected:
   typedef std::map<btVirtAddr, buffer_t_ase> map_t;
   typedef map_t::iterator                map_iter;
   typedef map_t::const_iterator          const_map_iter;

   btVirtAddr           m_MMIORmap;
   btUnsigned32bitInt   m_MMIORsize;
   btVirtAddr           m_uMSGmap;
   btUnsigned32bitInt   m_uMSGsize;

   btCSRValue             m_Last3c4;
   btCSRValue             m_Last3cc;
   map_t                  m_WkspcMap;

   // Map to store workspace parameters
   typedef std::map<btVirtAddr, struct aalui_WSMParms> mapWkSpc_t;
   mapWkSpc_t m_mapWkSpc;

   // List to cache device feature metadata
   typedef struct {
      btCSROffset        offset;    //< MMIO offset of feature
      struct CCIP_DFH    dfh;       //< Associated device feature header
      btUnsigned64bitInt guid[2];   //< GUID (for BBBs/private features)
   } FeatureDefinition;
   typedef std::vector<FeatureDefinition> FeatureList;
   FeatureList m_featureList;

   static CriticalSection sm_ASEMtx;

private:
   btBool _discoverFeatures();
   btBool _validateDFL();
   void _printDFH( const struct CCIP_DFH &dfh );

};

/// @}

END_NAMESPACE(AAL)

#endif // __ASEALIAFU1_H__

