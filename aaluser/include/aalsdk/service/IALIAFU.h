// Copyright (c) 2015, Intel Corporation
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
/// @file IALIAFU.h
/// @brief IALIAFU Service definition.
/// @ingroup IALIAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Ananda Ravuri, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     AR       Initial version derived from CCI
/// 08/XX/2015     HM       New material@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_IALIAFU_H__
#define __AALSDK_SERVICE_IALIAFU_H__
#include <aalsdk/AAL.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup IALIAFU
/// @{

#define iidALI_MMIO_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0001)
#define iidALI_UMSG_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0002)
#define iidALI_BUFF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0003)
#define iidALI_BUFF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0004)
#define iidALI_PERF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0005)
#define iidALI_RSET_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0006)


/// @file
/// @brief AFU Link Interface (ALI).
///
/// Defines the functionality available to ALI AFU's.
/// ALI Service defines a suite of interfaces. There is no specific separate IALIAFU class.
///
/// All ALI IID's will derive from Intel-specific sub-system INTC_sysAFULinkInterface.
///
/// An ALI Service will support zero to all of the following Services Interfaces:
///   iidALI_MMIO_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0001)
///   iidALI_UMSG_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0002)
///	  iidALI_BUFF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0003)
///   iidALI_BUFF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0004)
///	  iidALI_PERF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0005)
///   iidALI_RSET_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0006)
/// <TODO: LIST INTERFACES HERE>
///
/// If an ALI Service Client needs any particular Service Interface, then it must check at runtime
///    that the returned Service pointer supports the needed Interface and take appropriate action,
///    e.g. possibly failing to start and issuing an appropriate error message, or taking other
///    corrective action if possible.
/// For example:
/// @code
/// void serviceAllocated( IBase *pServiceBase, TransactionID const &rTranID) {
///    ASSERT( pServiceBase );         // if false, then Service threw a bad pointer
///
///    IAALService *m_pAALService;     // used to call Release on the Service
///    m_pAALService = dynamic_ptr<IAALService>( iidService, pServiceBase);
///    ASSERT( m_pAALService );
///
///    IALIMMIO *m_pMMIOService;       // used to call MMIO methods on the Service
///    m_pMMIOService = dynamic_ptr<IALIMMIO>( iidALI_MMIO_Service, pServiceBase);
///    ASSERT( m_pMMIOService );
///
///    IALIBUFFER *m_pBufferService;   // used to call BUFF methods on the Service
///    m_pBUFFERService = dynamic_ptr<IALIBuffer>( iidALI_BUFFER_Service, pServiceBase);
///    ASSERT( m_pBUFFERService );
///
///    <TODO: ADD EXAMPLES HERE>
/// }
/// @endcode

/// @brief  Provide access to the MMIO region exposed by the AFU to the Application.
/// @note   There is no client for this Service Interface because all of its methods
///            are synchronous (and fast)
///
class IALIMMIO
{
public:
   virtual ~IALIMMIO() {}

   /// @brief Obtain the user virtual address of the mmapped MMIO region.
   virtual btVirtAddr   mmioGetAddress( void ) = 0;

   /// @brief Obtain the user virtual address of the mmapped MMIO region.
   virtual btCSROffset  mmioGetLength( void ) = 0;

   /// Convenience functions for those who organize an MMIO space as a set of Registers

   /// @brief      Read an MMIO address (or register) as a 32-bit value.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to read.
   /// @param[out] pValue Where to place value read.
   /// @return     True if the read was successful.
   virtual btBool  mmioRead32( const btCSROffset Offset, btUnsigned32bitInt * const pValue) = 0;

   /// @brief      Write an MMIO address (or register) as a 32-bit value.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to write.
   /// @param[in]  Value  Value to write.
   /// @return     True if the write was successful.
   virtual btBool  mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value) = 0;

   /// @brief      Read an MMIO address (or register) as a 64-bit value.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to read.
   /// @param[out] pValue Where to place value read.
   /// @return     True if the read was successful.
   virtual btBool  mmioRead64( const btCSROffset Offset, btUnsigned64bitInt * const pValue) = 0;

   /// @brief      Write an MMIO address (or register) as a 64-bit value.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to write.
   /// @param[in]  Value  Value to write.
   /// @return     True if the write was successful.
   virtual btBool  mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value) = 0;
};

/// @brief  Provide access to the UMsg region(s) exposed by the AFU to the Application.
/// @note   Consider splitting into two interfaces. Do not need the more complex
///            Transaction oriented interface for most use cases. Alternatively have
///            asynchronous response to rarely used umsgSetAttributes call
///            IServiceClient::serviceEvent().
///
class IALIUMsg
{
public:
   virtual ~IALIUMsg() {}

   /// @brief     Obtain the number of UMsgs available to this AFU
   /// @note      Synchronous
   /// @return    The number of UMsgs available to the AFU.
   virtual btUnsignedInt umsgGetNumber( void ) = 0;

   /// @brief     Obtain the user virtual address of a particular UMsg.
   /// @note      Synchronous
   /// @param[in] Index of UMsg. Index starts from 0 and runs to umsgGetNumber()-1
   /// @return    The virtual address of the cache line which, if written, sends a UMsg
   virtual btVirtAddr    umsgGetAddress( const btUnsignedInt UMsgNumber ) = 0;

   /// @brief  Set attributes associated with the UMsg region and/or
   ///            individual UMsgs, depending on the arguments.
   /// @param[in] nvsArgs Defines the bitmask that will be set. Each bit
   ///	          in the mask corresponds to a UMsg. If set, then the UMsg has
   ///            hint as well as data.
   /// @note   Placeholder for now. Arguments TBD. E.g., for hint information.
   ///            Expectation is that hints will be defined for allocService
   ///            and used there, as well as here. This is only needed for changes.
   /// @note   #define UMSG_HINT_MASK_KEY       "UMsg-Hint-Mask-Key"
   ///         #define UMSG_HINT_MASK_DATATYPE  btUnsigned64bitInt
   ///         Value of bit mask is:
   ///            1 for hint, 0 for not.
   ///            Bit 0 = UMsg[0]
   ///            Bit 1 = UMsg[1]
   ///            etc.
   /// @note   Implication of synchronous is that the UMsg Device Feature Area
   ///            must be mapped in to user space.
   /// @return True of worked. At this point, no reason it would every fail.
   virtual bool umsgSetAttributes( NamedValueSet const &nvsArgs) = 0;
};

/// @brief  Buffer Allocation Service Interface of IALI
///
class IALIBuffer
{
public:
   virtual ~IALIBuffer() {}

   /// @brief Allocate a Workspace.
   ///
   /// @param[in]  Length   Requested length, in bytes.
   /// @param[in]  TranID   Returned in the notification event.
   /// @param[in]  pNVS     Pointer to Optional Arguments if needed. Defaults to NULL.
   ///
   /// On success, the workspace parameters are notified via IALIBUFFER::bufferAllocated.
   /// On failure, an error notification is sent via IALIBUFFER::bufferAllocateFailed.
   virtual void bufferAllocate( btWSSize             Length,
                                TransactionID const &TranID,
                                NamedValueSet       *pOptArgs = NULL ) = 0;

   /// @brief Free a previously-allocated Workspace.
   ///
   /// The provided workspace Address must have been acquired previously by IALIBUFFER::bufferAllocate.
   ///
   /// @param[in]  Address  User virtual address of the workspace.
   /// @param[in]  TranID   Returned in the notification event.
   ///
   /// On success, a notification is sent via IALIBUFFER::bufferFreed.
   /// On failure, an error notification is sent via IALIBUFFER::bufferFreeFailed.
   virtual void bufferFree( btVirtAddr           Address,
                            TransactionID const &TranID) = 0;

   /// @brief Retrieve the location at which the AFU can access the passed in virtual address.
   ///
   /// The user virtual address that the application uses to access a buffer may or
   ///    may not be directly usable by the AFU. The general assumption is that it is not.
   ///
   /// @param[in]  Address User virtual address to be converted to AFU-addressable location
   /// @return     A value that can be passed to the AFU such that when the AFU uses it,
   ///                the AFU will be accessing the byte at the address that was passed in.
   virtual btPhysAddr bufferGetIOVA( btVirtAddr Address) = 0;

};

/// @brief  Buffer Allocation Service Client Interface of IALI
///
class IALIBuffer_Client
{
public:
   virtual ~IALIBuffer_Client() {}

   /// @brief Notification callback for workspace allocation success.
   ///
   /// Sent in response to a successful workspace allocation request (IALIBUFFER::bufferAllocate).
   ///
   /// @param[in]  TranID     The transaction ID provided in the call to IALIBUFFER::bufferAllocate.
   /// @param[in]  WkspcVirt  The user virtual address of the newly-allocated workspace.
   /// @param[in]  WkspcSize  The size in bytes of the allocation.
   ///
   virtual void bufferAllocated( TransactionID const &rTranID,
                                 btVirtAddr           WkspcVirt,
                                 btWSSize             WkspcSize ) = 0;

   /// @brief Notification callback for workspace allocation failure.
   ///
   /// Sent in response to a failed workspace allocation request (IALIBUFFER::bufferAllocate).
   ///
   /// @param[in]  Event  An IExceptionTransactionEvent describing the failure.
   ///
   virtual void bufferAllocateFailed( IEvent const &rEvent ) = 0;

   /// @brief Notification callback for workspace free success.
   ///
   /// Sent in response to a successful free workspace request (IALIBUFFER::bufferFree).
   ///
   /// @param[in]  TranID  The transaction ID provided in the call to IALIBUFFER::bufferFree.
   ///
   virtual void bufferFreed( TransactionID const &rTranID ) = 0;

   /// @brief Notification callback for workspace free failure.
   ///
   /// Sent in response to a failed free workspace request (IALIBUFFER::bufferFree).
   ///
   /// @param[in]  Event  An IExceptionTransactionEvent describing the failure.
   ///
   virtual void bufferFreeFailed( IEvent const &rEvent ) = 0;
};

/// @brief  Obtain Global Performance Data (not AFU-specific)
///
class IALIPerf
{
public:
   virtual ~IALIPerf() {}

   /// @brief Request the Global Performance Data.
   ///
   /// The global performance counters relate to the traffic of all AFUs.
   /// They are 64-bit wrapping counters, and are read-only. Thus, one can
   ///    take a snapshot, perform an action, take another snapshot, subtract
   ///    one from the other (taking into account a possible wrap), and thereby
   ///    obtain the values associated with that operation (or operations if
   ///    across multiple AFUs).
   /// @note One needs to ensure that the operations being measured would not cause
   ///    a 64-bit wrap.
   ///
   /// @param[out]  ppResult  Returns the Performance Counter Values defined below.
   ///                           NULL if a problem.
   /// @param[in]   pOptArgs  Pointer to Optional Arguments if needed. Defaults to NULL.
   ///
   #define AALPERF_DATATYPE         btUnsigned64bitInt
   #define AALPERF_VERSION          "Version"           // Start with Version 0
   #define AALPERF_PORT0_READ_HIT   "Port0_Read_Hit"
   #define AALPERF_PORT0_WRITE_HIT  "Port0_Write_Hit"
   #define AALPERF_PORT0_READ_MISS  "Port0_Read_Miss"
   #define AALPERF_PORT0_WRITE_MISS "Port0_Write_Miss"
   #define AALPERF_PORT0_EVICTIONS  "Port0_Evictions"
   #define AALPERF_PORT1_READ_HIT   "Port1_Read_Hit"
   #define AALPERF_PORT1_WRITE_HIT  "Port1_Write_Hit"
   #define AALPERF_PORT1_READ_MISS  "Port1_Read_Miss"
   #define AALPERF_PORT1_WRITE_MISS "Port1_Write_Miss"
   #define AALPERF_PORT1_EVICTIONS  "Port1_Evictions"
   ///
   /// @code
   /// INamedValueSet const *pResult = NULL;
   /// performanceCountersGet( &pResult );
   /// if ( NULL != pResult ) {
   ///    // retrieve results
   /// }
   /// @endcode

   virtual void performanceCountersGet ( INamedValueSet const **ppResult,
                                         NamedValueSet const  *pOptArgs = NULL) = 0;
};

/// @brief  Reset the AFU Link Interface to this AFU
///
class IALIReset
{
public:
   virtual ~IALIReset() {}

   /// @brief Initiate a Reset.
   ///
   /// Only the Link to this AFU will be reset.
   /// By resetting the link, all outstanding transactions will quiesced, the
   ///    processing of memory transactions will be disabled,
   ///    the AFU will be sent a Reset signal.
   ///
   /// TODO: Making this synchronous requires the mapping into user space of
   ///          the Reset Device Feature List
   ///
   /// @param[in]  pNVS     Pointer to Optional Arguments if ever needed. Defaults to NULL.
   /// @return     True if succeeded, False if not. False would imply that transactions
   ///                did not quiesce within timeout value.
   ///
   virtual void afuDisableAndReset( NamedValueSet const *pOptArgs = NULL) = 0;

   /// @brief Re-enable the AFU after a Reset.
   ///
   /// Only the Link to this AFU will be enabled.
   /// It is an error to do anything other than strictly alternate afuDisableAndReset
   ///    and afuReEnable.
   ///
   /// TODO: Making this synchronous requires the mapping into user space of
   ///          the Reset Device Feature List
   ///
   /// @param[in]  pNVS     Pointer to Optional Arguments if ever needed. Defaults to NULL.
   /// @return     True if succeeded, False if not. False would imply that the alternating
   ///                state machine was not followed.
   ///
   virtual void afuReEnable( NamedValueSet const *pOptArgs = NULL) = 0;

   /// @brief Request a complete Reset. Convenience function combining other two.
   ///
   /// The Link to this AFU will be reset.
   /// By resetting the link, all outstanding transactions will quiesced, the
   ///    processing of memory transactions will be disabled,
   ///    the AFU will be sent a Reset signal,
   ///    and transactions will be re-enabled.
   /// TODO: Is there a positive affirmation of AFU Reset, now that DSM is gone? [Yes]
   /// TODO: Check that quiescence is destructive. E.g., one could could not
   ///       quiesce/enable without destroying state. Thus the need for reset. Correct? [Yes]
   /// TODO: Assuming quiesce is destructive, might one not want to split this into
   ///       two parts; Quiesce+Reset, then Enable Link? [Yes]
   ///
   /// TODO: Making this synchronous requires the mapping into user space of
   ///          the Reset Device Feature List
   ///
   /// @param[in]  pNVS     Pointer to Optional Arguments if ever needed. Defaults to NULL.
   ///
   /// Response is via IALIReset_Client::()
   ///
   virtual void afuReset( NamedValueSet const *pOptArgs = NULL) = 0;
};

// TODO:
// MAFU: Reconfigure (Deactivate, Activate?)


/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_IALIAFU_H__

