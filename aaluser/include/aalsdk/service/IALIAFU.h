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
/// @file IALIAFU.h
/// @brief AFU Link Interface (ALI)
/// @ingroup IALIAFU
/// @verbatim
/// Accelerator Abstraction Layer
/// AFU Link Interface (ALI) definition
///
/// This header defines the various interfaces exposed by the AFU Link
/// Interface. There is no specific separate IALIAFU class. The individual
/// interfaces are:
///
///    IALIMMIO   Functions for accessing AFU MMIO space and discovering
///               features used by the AFU, such as MPF
///    IALIUMsg   Functions for sending UMessages
///    IALIBuffer Functions for allocating shared buffers between software
///               and the AFU
///    IALIPerf   Functions for accessing performance counters
///    IALIReset  Functions for enabling, disabling, quiescing, and resetting
///               the AFU
///    IALIReconfigure
///               Functions for partial reconfiguration
///    IALISignalTap
///               Functions for remote debugging
///
/// Applications or services that need to interact with FPGA-implemented
/// functions (i.e AFUs) will typically allocate an ALI service and obtain
/// one or more of the above interfaces.
//////
/// All ALI IID's will derive from Intel-specific sub-system INTC_sysAFULinkInterface.
///
/// An ALI Service will support zero to all of the following Services Interfaces:
///   iidALI_MMIO_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0001)
///   iidALI_UMSG_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0002)
///   iidALI_BUFF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0003)
///   iidALI_BUFF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0004)
///   iidALI_PERF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0005)
///   iidALI_RSET_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0006)
///   iidALI_CONF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0007)
///   iidALI_CONF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0008)
///   iidALI_STAP_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0009)
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
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Ananda Ravuri, Intel Corporation
///          Enno Luebbers, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     AR       Initial version derived from CCI
/// 08/XX/2015     HM       New material@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_IALIAFU_H__
#define __AALSDK_SERVICE_IALIAFU_H__
#include <aalsdk/OSAL.h>
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/INTCDefs.h>
#include <aalsdk/AASLib.h>

BEGIN_NAMESPACE(AAL)

// FIXME: declare this where it should be declared...
#define ALI_MMAP_TARGET_VADDR_KEY        "ALIMmapTargetVAddr"
#define ALI_MMAP_TARGET_VADDR_DATATYPE   void *
#define ALI_GETFEATURE_ID_KEY            "ALIGetFeatureID"
#define ALI_GETFEATURE_ID_DATATYPE       btUnsigned64bitInt
#define ALI_GETFEATURE_TYPE_KEY          "ALIGetFeatureTYPE"
#define ALI_GETFEATURE_TYPE_DATATYPE     btUnsigned64bitInt
#define ALI_GETFEATURE_GUID_KEY          "ALIGetFeatureGUID"
#define ALI_GETFEATURE_GUID_DATATYPE     btcString

// CCIP DFH header types
#define ALI_DFH_TYPE_RSVD    0
#define ALI_DFH_TYPE_AFU     1
#define ALI_DFH_TYPE_BBB     2
#define ALI_DFH_TYPE_PRIVATE 3



/// @addtogroup IALIAFU
/// @{


//-----------------------------------------------------------------------------
// Request message IDs. Subject to change.
//-----------------------------------------------------------------------------
typedef enum
{
   ali_errnumOK = 0,
   ali_errnumBadDevHandle,                       // 1
   ali_errnumCouldNotClaimDevice,                // 2
   ali_errnumNoAppropriateInterface,             // 3
   ali_errnumDeviceHasNoPIPAssigned,             // 4
   ali_errnumCouldNotBindPipInterface,           // 5
   ali_errnumCouldNotUnBindPipInterface,         // 6
   ali_errnumNotDeviceOwner,                     // 7
   ali_errnumSystem,                             // 8
   ali_errnumAFUTransaction,                     // 9
   ali_errnumAFUTransactionNotFound,             // 10
   ali_errnumDuplicateStartingAFUTransactionID,  // 11
   ali_errnumBadParameter,                       // 12
   ali_errnumNoMem,                              // 13
   ali_errnumNoMap,                              // 14
   ali_errnumBadMapping,                         // 15
   ali_errnumPermission,                         // 16
   ali_errnumInvalidOpOnMAFU,                    // 17
   ali_errnumPointerOutOfWorkspace,              // 18
   ali_errnumNoAFUBindToChannel,                 // 19
   ali_errnumCopyFromUser,                       // 20
   ali_errnumDescArrayEmpty,                     // 21
   ali_errnumCouldNotCreate,                     // 22
   ali_errnumInvalidRequest,                     // 23
   ali_errnumInvalidDeviceAddr,                  // 24
   ali_errnumCouldNotDestroy,                    // 25
   ali_errnumDeviceBusy,                         // 26
   ali_errnumTimeout,                            // 27
   ali_errnumNoAFU,                              // 28
   ali_errnumAFUNotActivated,                    // 29
   ali_errnumDeActiveTimeout,                    // 30
   ali_errnumPRTimeout,                          // 31
   ali_errnumPROperation,                        // 32
   ali_errnumPRCRC,                              // 33
   ali_errnumPRIncompatibleBitstream,            // 34
   ali_errnumPRIPProtocal,                       // 35
   ali_errnumPRFIFO,                             // 36
   ali_errnumAFUActivationFail,                  // 37
   ali_errnumPRDeviceBusy,                       // 38
   ali_errnumPRSecureLoad,                       // 39
   ali_errnumPRPowerMgrTimeout,                  // 40
   ali_errnumPRPowerMgrCoreIdleFail,             // 41
   ali_errnumNoPRPowerMgrDemon,                  // 42
   ali_errnumSigtapRevokeTimeout,                // 43
   ali_errnumBadSocket,                          // 44
   ali_errnumRdMsrCmdFail,                       // 45
   ali_errnumFPGAPowerRequestTooLarge            // 46

} ali_errnum_e;


//-----------------------------------------------------------------------------
// ALI Interface IIDs.
//-----------------------------------------------------------------------------
#define iidALI_MMIO_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0001)
#define iidALI_UMSG_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0002)

#define iidALI_BUFF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0003)
#define iidALI_BUFF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0004)

#define iidALI_PERF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0005)
#define iidALI_RSET_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0006)

#define iidALI_CONF_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0007)
#define iidALI_CONF_Service_Client  __INTC_IID(INTC_sysAFULinkInterface,0x0008)
#define ALI_AFUID_UAFU_CONFIG       "A3AAB285-79A0-4572-83B5-4FD5E5216870"

#define iidALI_STAP_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0009)
#define CCIP_STAP_AFUID             "022F85B1-2CC2-4C9D-B6B0-3A385883AB8D"

#define iidALI_PORTERR_Service      __INTC_IID(INTC_sysAFULinkInterface,0x0010)
#define iidALI_FMEERR_Service       __INTC_IID(INTC_sysAFULinkInterface,0x0011)
#define iidALI_POWER_Service        __INTC_IID(INTC_sysAFULinkInterface,0x0012)
#define iidALI_TEMP_Service         __INTC_IID(INTC_sysAFULinkInterface,0x0013)


// FME GUID
#define CCIP_FME_AFUID              "BFAF2AE9-4A52-46E3-82FE-38F0F9E17764"
// PORT GUID
#define CCIP_PORT_AFUID             "3AB49893-138D-42EB-9642-B06C6B355B87"

/// Key for selecting an AFU delegate.
// TODO: consolidate naming scheme for NVS keys and datatypes
#define ALIAFU_NVS_KEY_TARGET "ALIAFUTarget"
/// Key for selecting ASECCIAFU
# define ALIAFU_NVS_VAL_TARGET_ASE   "ALIAFUTarget_ASE"
/// Key for selecting HWCCIAFU
# define ALIAFU_NVS_VAL_TARGET_FPGA  "ALIAFUTarget_FPGA"
/// Key for selecting SWSimCCIAFU
# define ALIAFU_NVS_VAL_TARGET_SWSIM "ALIAFUTarget_SWSim"


//-----------------------------------------------------------------------------
// AFU Target type.
//-----------------------------------------------------------------------------
typedef enum
{
   ali_afu_hwfpaga  = 0,      //Target Type Hardware FPGA
   ali_afu_ase,               //Target Type ASE
   ali_afu_swswim             //Target Type Software simulation

} ali_afu_target_e;


//-----------------------------------------------------------------------------
// IALIMMIO interface.
//-----------------------------------------------------------------------------
/// @brief  Provide the Application access to the MMIO region exposed by the AFU.
/// @note   There is no client for this Service Interface because all of its methods
///            are synchronous (and fast)
/// @note   This service interface is obtained from an IBase via iidALI_MMIO_Service
/// @code
///         m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
/// @endcode
class IALIMMIO
{
public:
   virtual ~IALIMMIO() {}

   /// @brief Obtain the user virtual address of the mmapped MMIO region.
   /// @returns The virtual address.
   virtual btVirtAddr   mmioGetAddress( void ) = 0;

   /// @brief Obtain the length of the mmapped MMIO region.
   /// @returns The length of the region.
   virtual btCSROffset  mmioGetLength( void ) = 0;

   /// @brief      Read an MMIO address (or register) as a 32-bit value.
   ///
   /// Convenience function for those who organize an MMIO space as a set of Registers.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to read.
   /// @param[out] pValue Where to place the value read.
   /// @retval     True if the read was successful.
   /// @retval     False if the read was not successful.
   virtual btBool  mmioRead32( const btCSROffset Offset, btUnsigned32bitInt * const pValue) = 0;

   /// @brief      Write an MMIO address (or register) as a 32-bit value.
   ///
   /// Convenience function for those who organize an MMIO space as a set of Registers.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to write.
   /// @param[in]  Value  Value to write.
   /// @retval     True if the write was successful.
   /// @retval     False if the write was not successful.
   virtual btBool  mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value) = 0;

   /// @brief      Read an MMIO address (or register) as a 64-bit value.
   ///
   /// Convenience function for those who organize an MMIO space as a set of Registers.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to read.
   /// @param[out] pValue Where to place value read.
   /// @retval     True if the read was successful.
   /// @retval     False if the read was not successful.
   virtual btBool  mmioRead64( const btCSROffset Offset, btUnsigned64bitInt * const pValue) = 0;

   /// @brief      Write an MMIO address (or register) as a 64-bit value.
   ///
   /// Convenience function for those who organize an MMIO space as a set of Registers.
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[in]  Offset Byte offset into the MMIO region at which to write.
   /// @param[in]  Value  Value to write.
   /// @retval     True if the write was successful.
   /// @retval     False if the write was not successful.
   virtual btBool  mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value) = 0;

   /// @brief      Request a pointer to a device feature header (DFH).
   ///
   /// Will deposit in *pFeatureAddr the base address of the device feature
   /// MMIO space (aka the address of the respective feature header) for a
   /// particular device feature that was requested. Users can request specific
   /// features by ID, type, and/or GUID, by supplying a NamedValueSet in
   /// rInputArgs. Possible keys are:
   ///
   /// ALI_GETFEATURE_ID      to request a feature with a specific feature ID
   /// ALI_GETFEATURE_GUID    to request a feature with a specific GUID ID
   /// ALI_GETFEATURE_TYPE    to request a feature with a specific feature type
   ///                        (BBB, private feature, …?)
   ///
   /// The function will return the ID, GUID, and TYPE of the first matching
   /// feature in rOutputArgs, if supplied (optional argument via overloading).
   ///
   /// Note that mmioGetFeature*() expects the values for the keys listed above
   /// to be of a specific datatype, namely:
   ///
   ///    ALI_GETFEATURE_ID_DATATYPE
   ///    ALI_GETFEATURE_TYPE_DATATYPE
   ///    ALI_GETFEATURE_GUID_DATATYPE
   ///
   /// The function will return an error if it encounters an unexpected
   /// datatype.
   ///
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[out] pFeature    Where to place the address of the feature header.
   /// @param[in]  rInputArgs  Reference to arguments specifying which feature to search for.
   /// @param[out] rOutputArgs Reference to additional properties of the returned feature.
   /// @return     True if requested feature was found in DFH space.
   // TODO: do we want this to return a valid pointer, or an offset to be used with the above functions?
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeature,
                                          NamedValueSet const &rInputArgs,
                                          NamedValueSet       &rOutputArgs ) = 0;

   // overloaded version without rOutputArgs
   /// @brief      Request a pointer to a device feature header (DFH).
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[out] pFeature    Where to place the address of the feature header.
   /// @param[in]  rInputArgs  Reference to arguments specifying which feature to search for.
   /// @return     True if requested feature was found in DFH space.
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeature,
                                          NamedValueSet const &rInputArgs ) = 0;

   // version that returns an MMIO offset instead of an address
   /// @brief      Request MMIO offset to a device feature header (DFH).
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[out] pFeatureOffset  Where to place the MMIO offset of the feature header.
   /// @param[in]  rInputArgs  Reference to arguments specifying which feature to search for.
   /// @param[out] rOutputArgs Reference to additional properties of the returned feature.
   /// @return     True if requested feature was found in DFH space.
   virtual btBool  mmioGetFeatureOffset( btCSROffset        *pFeatureOffset,
                                         NamedValueSet const &rInputArgs,
                                         NamedValueSet       &rOutputArgs ) = 0;

   // overloaded version without rOutputArgs
   /// @brief      Request MMIO offset to a device feature header (DFH).
   /// @note       Synchronous function; no TransactionID. Generally very fast.
   /// @param[out] pFeatureOffset  Where to place the MMIO offest of the feature header.
   /// @param[in]  rInputArgs  Reference to arguments specifying which feature to search for.
   /// @return     True if requested feature was found in DFH space.
   virtual btBool  mmioGetFeatureOffset( btCSROffset        *pFeatureOffset,
                                         NamedValueSet const &rInputArgs ) = 0;




}; // class IALIMMIO

/// Key for selecting the UMsg-Hint-Mask-Key.
#define UMSG_HINT_MASK_KEY "UMSG_HINT_MASK_KEY"

//-----------------------------------------------------------------------------
// IALIUMsg interface.
//-----------------------------------------------------------------------------
/// @brief  Provide access to the UMsg region(s) exposed by the AFU to the Application.
/// @note   This service interface is obtained from an IBase via iidALI_UMSG_Service.
/// @note   Once you have a pointer from umsgGetAddress(), you can write anything
///            to it, for up to 64-bytes, in any manner whatsoever. E.g. if you want
///            to atomically write a 16-byte value, you can do so using an SSE
///            intrinsic. If the processor supports AVX512, then one could atomically
///            write 64-bytes at once.
/// @note   If you write to a UMsg atomically, you should get one UMsg signal. If you
///            write to a UMsg atomically N times very quickly, the FPGA might get less
///            than N UMsg signals.
/// @note   If you write a single value that ends up being multiple writes from the point
///            of the view of the processor, then the FPGA will probably see multiple writes,
///            although it might not. An example of this would where memcpy() is used
///            to copy 64 bytes to the UMsg address. This might optimize to 4 pipelined
///            16-byte writes in a row. What the FPGA sees in such a situation is
///            non-deterministic.
/// @code
///         m_pALIUMsgService = dynamic_ptr<IALIUMsg>(iidALI_UMSG_Service, pServiceBase);
/// @endcode
class IALIUMsg
{
public:
   virtual ~IALIUMsg() {}

   /// @brief     Obtain the number of UMsgs available to this AFU.
   /// @note      Synchronous.
   /// @return    The number of UMsgs available to the AFU.
   virtual btUnsignedInt umsgGetNumber( void ) = 0;

   /// @brief     Obtain the user virtual address of a particular UMsg.
   /// @note      Synchronous.
   /// @param[in] UMsgNumber Index of UMsg. Index starts from 0 and runs to umsgGetNumber()-1.
   /// @return    The virtual address of the cache line which, if written, sends a UMsg.
   virtual btVirtAddr    umsgGetAddress( const btUnsignedInt UMsgNumber ) = 0;

   /// @brief     Convenience function to write a 64-bit entity to UMsg.
   /// @note      This is the only uMsg triggering method supported by ASE, so
   ///               use it for ASE compatibility.
   /// @note      This is intended to be fast, so there is no check. Passing a bad
   ///               address will probably result in a GPF.
   /// @param[in] pUMsg is the pointer returned from umsgGetAddress().
   /// @param[in] Value is a 64-bit value to write.
   /// @return    The virtual address of the cache line which, if written, sends a UMsg.
   virtual void    umsgTrigger64( const btVirtAddr pUMsg,
                                  const btUnsigned64bitInt Value ) = 0;

   /// @brief  Set attributes associated with the UMsg region and/or
   ///            individual UMsgs, depending on the arguments.
   /// @param[in] nvsArgs defines the bitmask that will be set. Each bit
   ///	          in the mask corresponds to a UMsg. If set, then the UMsg has
   ///            hint information as well as data.
   /// @note   Placeholder for now. Arguments TBD. E.g., for hint information.
   ///            Expectation is that hints will be defined for allocService
   ///            and used there, as well as here. This is only needed for changes.
   /// @note   \#define UMSG_HINT_MASK_KEY       "UMsg-Hint-Mask-Key".
   ///         \#define UMSG_HINT_MASK_DATATYPE  btUnsigned64bitInt.
   ///         Value of bit mask is:
   ///            1 for hint, 0 for not.
   ///            Bit 0 = UMsg[0]
   ///            Bit 1 = UMsg[1]
   ///            etc.
   /// @note   Implication of synchronous is that the UMsg Device Feature Area
   ///            must be mapped into user space.
   /// @retval True if successful. At this point, no reason it would ever fail.
   virtual bool umsgSetAttributes( NamedValueSet const &nvsArgs) = 0;

}; // class IALIUMsg


//-----------------------------------------------------------------------------
// IALIBuffer interface.
//-----------------------------------------------------------------------------
/// @brief  Buffer Allocation Service Interface of IALI.
///
/// @note   This service interface is obtained from an IBase via iidALI_BUFF_Service.
/// @code
///         m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
/// @endcode
class IALIBuffer
{
public:
   virtual ~IALIBuffer() {}

   /// @brief Allocate a Workspace.
   ///
   /// @param[in]  Length       Requested length, in bytes.
   /// @param[out] pBufferptr    Buffer Pointer.
   ///
   /// @return On success, ali_errnumOK.
   /// @return On failure, ali_errnumSystem.
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr ) = 0;
   /// @brief Allocate a Workspace using additional input arguments.
   ///
   /// @param[in]  Length       Requested length, in bytes.
   /// @param[out] pBufferptr    Buffer Pointer.
   /// @param[in]  rInputArgs   Reference to optional input arguments if needed.
   /// @return On success, ali_errnumOK.
   /// @return On failure, ali_errnumSystem.
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr,
                                             NamedValueSet const &rInputArgs ) = 0;
   /// @brief Allocate a Workspace using additional input arguments and accepting
   ///        return arguments.
   ///
   /// @param[in]  Length       Requested length, in bytes.
   /// @param[out] pBufferptr    Buffer Pointer.
   /// @param[in]  rInputArgs   Reference to optional input arguments if needed.
   /// @param[out] rOutputArgs  Reference to optional return arguments if needed.
   /// @return On success, ali_errnumOK.
   /// @return On failure, ali_errnumSystem.
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr,
                                             NamedValueSet const &rInputArgs,
                                             NamedValueSet       &rOutputArgs ) = 0;

   /// @brief Free a previously-allocated Workspace.
   ///
   /// The provided workspace Address must have been acquired previously by IALIBUFFER::bufferAllocate.
   ///
   /// @param[in]  Address  User virtual address of the workspace.
   ///
   /// @return On success, ali_errnumOK.
   /// @return On failure, ali_errnumBadParameter or ali_errnumSystem.
   virtual AAL::ali_errnum_e bufferFree( btVirtAddr           Address) = 0;

   /// @brief Retrieve the location at which the AFU can access the passed in virtual address.
   ///
   /// The user virtual address that the application uses to access a buffer may or
   ///    may not be directly usable by the AFU. The general assumption is that it is not.
   ///
   /// NOTE: The mapping from virtual address to IOVA is not particularly fast. The user
   ///    of this interface is urged to allocate large buffers, obtain the IOVA once, and
   ///    then sub-allocate. Within a buffer, the virtual and IOVA addresses track each other.
   ///    E.g. if the virtual address is 0x6000 and the IOVA is 0x1000, then 5 bytes into 
   ///    the buffer will be at virtual address 0x6005 and at IOVA 0x1005.
   ///
   /// @param[in]  Address User virtual address to be converted to AFU-addressable location
   /// @return     A value that can be passed to the AFU such that when the AFU uses it,
   ///                the AFU will be accessing the byte at the address that was passed in.
   virtual btPhysAddr bufferGetIOVA( btVirtAddr Address) = 0;

}; // class IALIBuffer


//-----------------------------------------------------------------------------
// IALIPerf interface.
//-----------------------------------------------------------------------------
/// @brief  Obtain Global Performance Data (not AFU-specific) (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_PERF_Service.
/// @code
///         m_pALIPerfService = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, pServiceBase);
/// @endcode
///
class IALIPerf
{
public:
   virtual ~IALIPerf() {}

   #define AALPERF_DATATYPE         btUnsigned64bitInt
   #define AALPERF_VERSION          "version"           // Start with Version 0
   #define AALPERF_READ_HIT         "Read_Hit"
   #define AALPERF_WRITE_HIT        "Write_Hit"
   #define AALPERF_READ_MISS        "Read_Miss"
   #define AALPERF_WRITE_MISS       "Write_Miss"
   #define AALPERF_EVICTIONS        "Evictions"

   #define AALPERF_PCIE0_READ       "PCIe0 Read"
   #define AALPERF_PCIE0_WRITE      "PCIe0 Write"
   #define AALPERF_PCIE1_READ       "PCIe1 Read"
   #define AALPERF_PCIE1_WRITE      "PCIe1 Write"
   #define AALPERF_UPI_READ         "UPI Read"
   #define AALPERF_UPI_WRITE        "UPI Write"

   #define AALPERF_VTD_AFU_MEMREAD_TRANS            "VT-d AFU Memory Read Transaction"
   #define AALPERF_VTD_AFU_MEMWRITE_TRANS           "VT-d AFU Memory Write Transaction"
   #define AALPERF_VTD_AFU_DEVTLBREAD_HIT           "VT-d AFU DevTLB Read Hit"
   #define AALPERF_VTD_AFU_DEVTLBWRITE_HIT          "VT-d AFU DevTLB Write Hit"

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
   /// @param[out]  pResult  Returns the Performance Counter Values defined below.
   ///                           NULL if a problem.
   ///
   /// @code
   /// INamedValueSet const *pResult = NULL;
   /// performanceCountersGet( &pResult );
   /// if ( NULL != pResult ) {
   ///    // retrieve results
   /// }
   /// @endcode
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult ) = 0;
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
   /// @param[out]  pResult  Returns the Performance Counter Values defined below.
   ///                           NULL if a problem.
   /// @param[in]   pOptArgs  Pointer to Optional Arguments if needed. Defaults to NULL.
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult,
                                           NamedValueSet    const &pOptArgs ) = 0;
}; // class IALIPerf

//-----------------------------------------------------------------------------
// IALIReset interface.
//-----------------------------------------------------------------------------
/// @brief  Reset the AFU Link Interface to this AFU (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_RSET_Service.
/// @code
///         m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
/// @endcode
class IALIReset
{
public:
   virtual ~IALIReset() {}

   /// @brief       Definitions of errors returned  when resetting an AFU.
   enum e_Reset {
      e_OK,                      ///< Everything is okay
      e_Internal,                ///< Internal failure
      e_Error_Quiesce_Timeout    ///< Could not disable completely, issued reset anyway
   };

   /// @brief Initiate an AFU Reset.
   ///
   /// Only the Link to this AFU will be reset.
   /// By resetting the link, all outstanding transactions will be quiesced, the
   ///    processing of memory transactions will be disabled,
   ///    the AFU will be sent a Reset signal.
   /// There is a positive affirmation of Quiesce in the form of number of outstanding
   ///    transactions going to 0.
   /// Quiesce is destructive, e.g. outstanding transactions are lost. So state must
   ///    be Reset afterwards.
   ///
   /// @retval     e_Reset::e_OK if succeessful.
   /// @retval     e_Reset::e_Error_Quiesce_Timeout indicates that the link did not quiesce
   ///                        within the provided timeout. (Currently no way to set timeout).
   /// @retval     e_Reset::e_Internal if any problem other than a timeout occurred.
   virtual e_Reset afuQuiesceAndHalt( void ) = 0;

   /// @brief Initiate an AFU Reset.
   ///
   /// Only the Link to this AFU will be reset.
   /// By resetting the link, all outstanding transactions will be quiesced, the
   ///    processing of memory transactions will be disabled,
   ///    the AFU will be sent a Reset signal.
   /// There is a positive affirmation of Quiesce in the form of number of outstanding
   ///    transactions going to 0.
   /// Quiesce is destructive, e.g. outstanding transactions are lost. So state must
   ///    be Reset afterwards.
   ///
   /// @param[in]  rInputArgs      Reference to Optional Arguments if ever needed. Defaults to NULL.
   /// @retval     e_Reset::e_OK if succeessful.
   /// @retval     e_Reset::e_Error_Quiesce_Timeout indicates that the link did not quiesce
   ///                        within the provided timeout. (Currently no way to set timeout).
   /// @retval     e_Reset::e_Internal if any problem other than a timeout occurred.
   virtual e_Reset afuQuiesceAndHalt( NamedValueSet const &rInputArgs ) = 0;

   /// @brief Re-enable the AFU after a Reset.
   ///
   /// Only the Link to this AFU will be enabled.
   /// It is an error to do anything other than strictly alternate afuQuiesceAndHalt()
   ///    and afuReEnable().
   ///
   /// @retval     e_Reset::e_OK if succeeded. No errors expected.
   virtual e_Reset afuEnable( void ) = 0;

   /// @brief Re-enable the AFU after a Reset.
   ///
   /// Only the Link to this AFU will be enabled.
   /// It is an error to do anything other than strictly alternate afuQuiesceAndHalt
   ///    and afuReEnable.
   ///
   /// @param[in]  rInputArgs  Pointer to Optional Arguments if ever needed. Defaults to NULL.
   /// @retval     e_Reset::e_OK if successful. No errors expected.
   virtual e_Reset afuEnable( NamedValueSet const &rInputArgs) = 0;

   /// @brief Request a complete Reset. Convenience function combining other two.
   ///
   /// The Link to this AFU will be reset.
   /// By resetting the link:
   ///    - All outstanding transactions will quiesced.
   ///    - The processing of memory transactions will be disabled.
   ///    - The AFU will be sent a Reset signal.
   ///    - Transactions will be re-enabled.
   ///
   /// @retval     e_Reset::e_OK if successful.
   /// @retval     e_Reset::e_Error_Quiesce_Timeout indicates that the link did not quiesce within
   ///                                     the provided timeout. (Currently no way to set timeout).
   /// @retval     e_Reset::e_Internal if any problem other than a timeout occurred.
   virtual e_Reset afuReset( void ) = 0;

   /// @brief Request a complete Reset. Convenience function combining other two.
   ///
   /// The Link to this AFU will be reset.
   /// By resetting the link:
   ///    - All outstanding transactions will quiesced.
   ///    - The processing of memory transactions will be disabled.
   ///    - The AFU will be sent a Reset signal.
   ///    - Transactions will be re-enabled.
   ///
   /// @param[in]  rInputArgs              Pointer to Optional Arguments if ever needed. Defaults to NULL.
   /// @retval     e_Reset::e_OK if successful.
   /// @retval     e_Reset::e_Error_Quiesce_Timeout indicates that the link did not quiesce within
   ///                                     the provided timeout. (Currently no way to set timeout).
   /// @retval     e_Reset::e_Internal if any problem other than a timeout occurred.
   virtual e_Reset afuReset( NamedValueSet const &rInputArgs ) = 0;
}; // class IALIReset


/// NOTE: This will be a service that is not typically exported by the ALI Service. Rather,
///       it will be allocated by requesting a PR_ID (an AFU_ID associated with a PR),
///       along with if necessary additional meta information such as bus:function:number of
///       the PCIe device.
//
//-----------------------------------------------------------------------------
// IALIReconfigure interface.
//-----------------------------------------------------------------------------
/// @brief  Provides Reconfiguration Services (asynchronous and controlled by driver)
///
/// @note   This service interface is obtained from an IBase via iidALI_CONF_Service
/// @code
///         m_pALIReconfigureService = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
/// @endcode
class IALIReconfigure
{
public:
   virtual ~IALIReconfigure() {}

   #define AALCONF_FILENAMEKEY            "BitStreamFile"
   #define AALCONF_FILENAMETYPE            btString
   #define AALCONF_BUFPTRKEY              "BufPointer"
   #define AALCONF_BUFSIZE                "BufSize"


   // Approximate duration to wait for device to be deactivated
   // and reconfigure timeout
   #define AALCONF_MILLI_TIMEOUT          "DeactivatedTimeout"

   // Reconfiguration action value specifies what is expected
   // action that should be taken if the AFU is in use
   #define AALCONF_RECONF_ACTION          "ReconfAction"

   // Values for AALCONF_RECONF_ACTION
   //======================================

   // Force the reconfiguration request, revoking the
   //  resource from current owner if necessary
   #define AALCONF_RECONF_ACTION_HONOR_REQUEST_ID   (static_cast<btUnsigned64bitInt>(0x0))
   #define AALCONF_RECONF_ACTION_HONOR_OWNER_ID     (static_cast<btUnsigned64bitInt>(0x1))

   // Bool key.  If false or not present AFU will be activated
   //  after reconfiguration.  If present and set to true the AFU
   //  will remain uinactive (invisible to the application) until
   //  a recongActivate() is called.
   #define AALCONF_REACTIVATE_DISABLED      "ReactivateState"


   /// @brief Deactivate an AFU in preparation for it being reconfigured.
   ///
   /// If there is an AFU currently instantiated and connected to an
   ///    application, then this will send an exception to the application indicating
   ///    that it should release the AFU. There can be a timeout option that specifies
   ///    that if the application does not Release within a particular time, then
   ///    the AFU will be yanked. Then, a CleanIt!(tm) AFU will be loaded to clear
   ///    out all the gates and clear the FPGA memory banks.
   ///
   /// @param[in]  rTranID Reference to the transaction ID.
   /// @param[in]  rInputArgs Reference to input Arguments.
   /// @return     void. Calls a Callback in IALIReconfigureClient to notify the
   ///             caller of the result of the call to reconfDeactivate().
   virtual void reconfDeactivate( TransactionID const &rTranID,
                                  NamedValueSet const &rInputArgs ) = 0;

   /// @brief Reconfigure an AFU.
   ///
   /// Download the defined bitstream to the PR region. Initially, the bitstream
   ///    is a file name. Later, it might be a goal record, and that is why the
   ///    parameter is an NVS. It is also possible in the NVS to specify a PR number
   ///    if that is relevant, e.g. for the PF driver.
   ///
   /// @param[in]  rTranID Reference to the transaction ID.
   /// @param[in]  rInputArgs Reference to input Arguments.
   /// @return     void. Calls a Callback in IALIReconfigureClient to notify the
   ///             caller of the result of the call to reconfConfigure().
   virtual void reconfConfigure( TransactionID const &rTranID,
                                 NamedValueSet const &rInputArgs ) = 0;

   /// @brief Activate an AFU after it has been reconfigured.
   ///
   /// Once the AFU has been reconfigured there needs to be a "probe" to load
   ///    the AFU configuration information, e.g. AFU_ID, so that the associated
   ///    service can be loaded and the whole shebang returned to the application.
   ///
   /// @param[in]  rTranID Reference to the transaction ID.
   /// @param[in]  rInputArgs Reference to input Arguments.
   /// @return     void. Calls a Callback in IALIReconfigureClient to notify the
   ///             caller of the result of the call to reconfActivate().
   virtual void reconfActivate( TransactionID const &rTranID,
                                NamedValueSet const &rInputArgs ) = 0;

}; // class IALIReconfigure


//-----------------------------------------------------------------------------
// IALIReconfigure_Client service client interface.
//-----------------------------------------------------------------------------
/// @brief  Reconfiguration Callbacks.
/// These callbacks are the mechanism that methods in IALIReconfigure use to notify
/// applications of the results of attempts to reconfigure the AFU.
///
/// @note   This interface is implemented by the client and set in the IBase
///         of the client object as an iidALI_CONF_Service_Client.
/// @code
///         SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client *>(this));
/// @endcode
class IALIReconfigure_Client
{
public:
   virtual ~IALIReconfigure_Client() {}

   /// @brief Notification callback for reconfDeactivate() succeeded.
   ///
   /// @param[in]  rTranID Reference to the Transaction ID from the original reconfDeactivate call.
   /// @return     void
   virtual void deactivateSucceeded( TransactionID const &rTranID ) = 0;

   /// @brief Notification callback for deactivate failed.
   ///
   /// Sent in response to a failed reconfDeactivate().
   ///
   /// @param[in]  rEvent  A reference to an IExceptionTransactionEvent describing the failure.
   /// @return     void
   virtual void deactivateFailed( IEvent const &rEvent ) = 0;

   /// @brief Notification callback for reconfConfigure() succeeded.
   ///
   /// @param[in]  rTranID Reference to the Transaction ID from the original reconfConfigure call.
   /// @return     void
   virtual void configureSucceeded( TransactionID const &rTranID ) = 0;

   /// @brief Notification callback for configure failed.
   ///
   /// Sent in response to a failed reconfConfigure().
   ///
   /// @param[in]  rEvent  A reference to an IExceptionTransactionEvent describing the failure.
   /// @return     void
   virtual void configureFailed( IEvent const &rEvent ) = 0;

   /// @brief Notification callback for reconfActivate() succeeded.
   ///
   /// @param[in]  rTranID Reference to the Transaction ID from the original reconfActivate() call.
   /// @return     void
   virtual void activateSucceeded( TransactionID const &rTranID ) = 0;

   /// @brief Notification callback for reconfActivate() failed.
   ///
   /// Sent in response to a failed reconfActivate().
   ///
   /// @param[in]  rEvent  A reference to an IExceptionTransactionEvent describing the failure.
   /// @return     void
   virtual void activateFailed( IEvent const &rEvent ) = 0;

}; // class IALIReconfigure_Client


//-----------------------------------------------------------------------------
// IALISignalTap interface.
//-----------------------------------------------------------------------------
/// @brief  Access Signal Tap PCIe mmio space.
///
/// @note This will be a service that is not typically exported by the ALI Service. Rather,
///       it will be allocated by requesting the STAP_ID (an AFU_ID associated with a Signal Tap)
///       along with if necessary additional meta information such as bus:function:number of
///       the PCIe device.
///
/// @note   This service interface is obtained from an IBase via iidALI_STAP_Service.
/// @code
///         m_pALISignalTapService = dynamic_ptr<IALISignalTap>(iidALI_STAP_Service, pServiceBase);
/// @endcode
class IALISignalTap
{
public:
   virtual ~IALISignalTap() {}

   /// @brief  Obtain an mmio map for the Signal Tap region.
   ///
   /// @retval User mode pointer to the mmio region if successful.
   /// @retval NULL if not successful.
   ///
   virtual btVirtAddr stpGetAddress( void ) = 0;

}; // class IALISignalTap


//-----------------------------------------------------------------------------
// IALIError interface.
//-----------------------------------------------------------------------------
/// @brief  Obtains error values set by an FPGA (synchronous).
class IALIError
{
public:
   virtual ~IALIError() {}

   /// @brief       Obtains errors from FPGA.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned errors set by FPGA.
   ///                       NULL if a problem.
   /// @retval      True if the errors are read successfully.
   /// @retval      False if the errors are not retrieved.
   virtual btBool errorGet( INamedValueSet &rResult ) = 0;


   /// @brief       Obtains first error from FPGA.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned first error set by FPGA.
   ///                       NULL if a problem.
   /// @retval      True if the error are read successfully.
   /// @retval      False if the error is not retrieved.
   virtual btBool errorGetOrder( INamedValueSet &rResult ) = 0;

   /// @brief       Obtains error mask from FPGA.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned error mask.
   ///                       NULL if a problem.
   /// @retval      True if the error mask is read successfully.
   /// @retval      False if the error mask is not retrieved.
   virtual btBool errorGetMask( INamedValueSet &rResult ) = 0;

   /// @brief       Sets error mask on FPGA.
   /// @note        Synchronous.
   /// @param[in]   rInputArgs  Error mask to set.
   /// @retval      True if the error mask is set successfully.
   /// @retval      False if the error mask is not set.
   virtual btBool errorSetMask(const INamedValueSet &rInputArgs) = 0;

   /// @brief       Clears error from FPGA.
   /// @note        Synchronous.
   /// @param[in]   rInputArgs  Error to clear.
   /// @retval      True if the error is cleared.
   /// @retval      False if the error is not cleared.
   virtual btBool errorClear( const INamedValueSet &rInputArgs) = 0;

   /// @brief       Clears all errors from FPGA.
   /// @note        Synchronous.
   /// @retval      True if the errors are cleared.
   /// @retval      False if the errors are not cleared.
   virtual btBool errorClearAll() = 0;

   /// @brief       Prints all errors from FPGA.
   /// @note        Synchronous.
   /// @retval      True if the errors are printed.
   /// @retval      False if the errors are not printed.
   virtual btBool printAllErrors() = 0;

}; // class IALIError

//-----------------------------------------------------------------------------
// IALIFMEError interface.
//-----------------------------------------------------------------------------
/// @brief  Obtains FME error values   (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_FMEERR_Service.
/// @code
///         m_pALIFMEErrService = dynamic_ptr<IALIFMEError>(iidALI_FMEERR_Service, pServiceBase);
/// @endcode
class IALIFMEError: public IALIError
{
public:

   // FME Error definitions
   #define AAL_ERR_FME_FAB_UNDERFLOW              "Fabric FIFO underflow"
   #define AAL_ERR_FME_FAB_OVERFLOW               "Fabric FIFO overflow"
   #define AAL_ERR_FME_PCIE0_POSION_DETECT        "PCIe0 Poison Detected"
   #define AAL_ERR_FME_PCIE1_POSION_DETECT        "PCIe0 Poison Detected"
   #define AAL_ERR_FME_IOMMU_PARIRY               "IOMMU Parity Error"
   #define AAL_ERR_FME_AFUMISMATCH_DETECT         "AFU Access Mismatch detected"


   #define AAL_ERR_PCIE0_FORMAT                   "PCIe0 TLP Format/type error"
   #define AAL_ERR_PCIE0_MWADDR                   "PCIe0 TLP MW Address error"
   #define AAL_ERR_PCIE0_MWLEN                    "PCIe0 TLP MW Length error"
   #define AAL_ERR_PCIE0_MRADDR                   "PCIe0 TLP MR Address error"
   #define AAL_ERR_PCIE0_MRLEN                    "PCIe0 TLP MR Length error"
   #define AAL_ERR_PCIE0_COMPTAG                  "PCIe0 TLP CPL TAP error"
   #define AAL_ERR_PCIE0_COMPSTAT                 "PCIe0 TLP CPL Status error"
   #define AAL_ERR_PCIE0_TIMEOUT                  "PCIe0 TLP CPL Timeout error"


   #define AAL_ERR_PCIE1_FORMAT                   "PCIe1 TLP Format/type error"
   #define AAL_ERR_PCIE1_MWADDR                   "PCIe1 TLP MW Address error"
   #define AAL_ERR_PCIE1_MWLEN                    "PCIe1 TLP MW Length error"
   #define AAL_ERR_PCIE1_MRADDR                   "PCIe1 TLP MR Address error"
   #define AAL_ERR_PCIE1_MRLEN                    "PCIe1 TLP MR Length error"
   #define AAL_ERR_PCIE1_COMPTAG                  "PCIe1 TLP CPL TAP error"
   #define AAL_ERR_PCIE1_COMPSTAT                 "PCIe1 TLP CPL Status error"
   #define AAL_ERR_PCIE1_TIMEOUT                  "PCIe1 TLP CPL Timeout error"

   #define AAL_ERR_PCIE_PHYFUNC                   "Physical function error"
   #define AAL_ERR_PCIE_VIRTFUNCR                 "Virtual function error"

   #define AAL_ERR_RAS_TEMPAP1                    "Thermal threshold Triggered AP1"
   #define AAL_ERR_RAS_TEMPAP2                    "Thermal threshold Triggered AP2"
   #define AAL_ERR_RAS_PCIE                       "PCIe Fatal Error"
   #define AAL_ERR_RAS_AFUFATAL                   "AFU Fatal error"
   #define AAL_ERR_RAS_AFUACCESS_MODE             "AFU access mode error"
   #define AAL_ERR_RAS_PCIEPOSION                 "PCIe poison port  error"
   #define AAL_ERR_RAS_GBCRC                      "Green bitstream CRC Error"
   #define AAL_ERR_RAS_TEMPAP6                    "Thremal threshold Triggered AP6"
   #define AAL_ERR_RAS_POWERAP1                   "Power threshold Triggered AP1"
   #define AAL_ERR_RAS_POWERAP2                   "Power threshold Triggered AP2"
   #define AAL_ERR_RAS_MDP                        "MBP error "

   #define AAL_ERR_RAS_KTILINK_FATAL              "KTI Link layer Fatal error"
   #define AAL_ERR_RAS_TAGCCH_FATAL               "tag-n-cache Fatal error"
   #define AAL_ERR_RAS_CCI_FATAL                  "CCI Fatal error"
   #define AAL_ERR_RAS_KTIPROTO_FATAL             "KTI Protocal Fatal error"
   #define AAL_ERR_RAS_DMA_FATAL                  "DMA Fatal error"
   #define AAL_ERR_RAS_INJ_FATAL                  "Injected Fatal error"
   #define AAL_ERR_RAS_IOMMU_FATAL                "IOMMU Fatal error"
   #define AAL_ERR_RAS_IOMMU_CATAS                "Catastrophic IOMMU Error"
   #define AAL_ERR_RAS_CRC_CATAS                  "Catastrophic CRC Error"
   #define AAL_ERR_RAS_THER_CATAS                 "Catastrophic Thermal Error"
   #define AAL_ERR_RAS_GB_FATAL                   "Green bitstream fatal event Error"
   #define AAL_ERR_RAS_INJ_CATAS_                 "Injected Catastrophic error"


   virtual ~IALIFMEError() {}

}; // class IALIFMEError


//-----------------------------------------------------------------------------
// IALIPortError interface.
//-----------------------------------------------------------------------------
/// @brief  Obtains Port error values (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_PORTERR_Service.
/// @code
///         m_pALIPortErrService = dynamic_ptr<IALIPower>(iidALI_PORTERR_Service, pServiceBase);
/// @endcode
class IALIPortError : public IALIError
{
public:

   // Port Error definitions
   #define AAL_ERR_PORT_TX_CH0_OVERFLOW           "Tx Channel0: Overflow"
   #define AAL_ERR_PORT_TX_CH0_INVALIDREQ         "Tx Channel0: Invalid request encoding"
   #define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN3        "Tx Channel0: Request with cl_len3"
   #define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN2        "Tx Channel0: Request with cl_len2"
   #define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN4        "Tx Channel0: Request with cl_len4"

   #define AAL_ERR_PORT_TX_CH1_OVERFLOW           "Tx Channel1: Overflow"
   #define AAL_ERR_PORT_TX_CH1_INVALIDREQ         "Tx Channel1: Invalid request encoding"
   #define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN3        "Tx Channel1: Request with cl_len3"
   #define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN2        "Tx Channel1: Request with cl_len2"
   #define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN4        "Tx Channel1: Request with cl_len4"
   #define AAL_ERR_PORT_TX_CH1_INSUFF_DATAPYL     "Tx Channel1: Insufficient data payload"
   #define AAL_ERR_PORT_TX_CH1_DATAPYL_OVERRUN    "Tx Channel1: Data payload overrun"
   #define AAL_ERR_PORT_TX_CH1_INCORR_ADDR        "Tx Channel1: Incorrect address"
   #define AAL_ERR_PORT_TX_CH1_SOP_DETECTED       "Tx Channel1: NON-Zero SOP Detected"
   #define AAL_ERR_PORT_TX_CH1_ATOMIC_REQ         "Tx Channel1: Illegal VC_SEL, atomic request VLO"

   #define AAL_ERR_PORT_MMIOREAD_TIMEOUT          "MMIO Read Timeout in AFU"
   #define AAL_ERR_PORT_TX_CH2_FIFO_OVERFLOW      "Tx Channel2: FIFO overflow"
   #define AAL_ERR_PORT_UNEXP_MMIORESP            "MMIO read response received, with no matching pending request"
   #define AAL_ERR_PORT_NUM_PENDREQ_OVERFLOW      "Number of pending Requests: counter overflow"

   #define AAL_ERR_PORT_LLPR_SMRR                 "Request with Address violating SMM Range"
   #define AAL_ERR_PORT_LLPR_SMRR2                "Request with Address violating second SMM Range"
   #define AAL_ERR_PORT_LLPR_MSG                  "Request with Address violating ME Stolen message"
   #define AAL_ERR_PORT_GENPORT_RANGE             "Request with Address violating Generic protect range"
   #define AAL_ERR_PORT_LEGRANGE_LOW              "Request with Address violating Legacy Range Low"
   #define AAL_ERR_PORT_LEGRANGE_HIGH             "Request with Address violating Legacy Range High"
   #define AAL_ERR_PORT_VGAMEM_RANGE              "Request with Address violating VGA memory range"
   #define AAL_ERR_PORT_PAGEFAULT                 "Page fault"
   #define AAL_ERR_PORT_PMRERROR                  "PMR Error"
   #define AAL_ERR_PORT_AP6EVENT                  "AP6 Event"
   #define AAL_ERR_PORT_VFFLR_ACCESS              "VF FLR detected on Port with PF access control"

   #define AAL_ERR_PORT_MALFORMED_REQ_0           "Port malformed request0"
   #define AAL_ERR_PORT_MALFORMED_REQ_1           "Port malformed request1"


   virtual ~IALIPortError() {}

   /// @brief       Obtains port malformed request value.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned port malformed request value.
   ///                       NULL if a problem.
   /// @retval      True if the read succeeded.
   /// @retval      False if the read failed.
   virtual btBool errorGetPortMalformedReq( INamedValueSet &rResult ) = 0;

}; // class IALIPortError


//-----------------------------------------------------------------------------
// IALITemperature interface.
//-----------------------------------------------------------------------------
/// @brief  Obtains Temperature and Threshold values  (not AFU-specific) (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_TEMP_Service.
/// @code
///         m_pALITempService = dynamic_ptr<IALIPower>(iidALI_TEMP_Service, pServiceBase);
/// @endcode
///
class IALITemperature
{
public:
   virtual ~IALITemperature() {}

   #define AALTEMP_DATATYPE                 btUnsigned64bitInt
   #define AALTEMP_FPGA_TEMP_SENSOR1        "Sensor1 Temperature  Celsius"
   #define AALTEMP_FPGA_TEMP_SENSOR2        "Sensor2 Temperature  Celsius"
   #define AALTEMP_READING_SEQNUM           "Temp_Reading_SeqNum"
   #define AALTEMP_READING_VALID            "Temp_Reading_Valid"

   #define AALTEMP_THRESHOLD1               "Temperature Threshold1"
   #define AALTEMP_THRESHOLD2               "Temperature Threshold2"

   #define AALTEMP_THERM_TRIP               "Thermal Trip Threshold"


   #define AALTEMP_THSHLD_STATUS1_AP1       "Threshold AP1 Policy set"
   #define AALTEMP_THSHLD_STATUS1_AP2       "Threshold AP1 Policy set"
   #define AALTEMP_THSHLD_STATUS1_AP6       "Threshold AP6 Policy set"

   #define AALTEMP_THSHLD_POLICY1            "AP1_STATE"
   #define AALTEMP_THSHLD_POLICY2            "AP2_STATE"
   #define AALTEMP_THSHLD_POLICY6            "AP6_STATE"

   /// @brief       Obtains threshold status and Trip value.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned threshold status and Trip value.
   ///                       NULL if a problem.
   /// @retval      True if the read succeeded.
   /// @retval      False if the read failed.
   virtual btBool thermalGetValues(INamedValueSet &rResult ) = 0;

}; // class IALITemperature


//-----------------------------------------------------------------------------
// IALIPower interface.
//-----------------------------------------------------------------------------
/// @brief  Obtains FPGA power   (not AFU-specific) (synchronous).
///
/// @note   This service interface is obtained from an IBase via iidALI_POWER_Service.
/// @code
///         m_pALIPowerService = dynamic_ptr<IALIPower>(iidALI_POWER_Service, pServiceBase);
/// @endcode
///
class IALIPower
{
public:
   virtual ~IALIPower() {}


   #define AALPOWER_DATATYPE            btUnsigned64bitInt
   #define AALPOWER_CONSUMPTION        "Power Consumption Value"

   /// @brief       Obtains Power consumption of FPGA.
   /// @note        Synchronous.
   /// @param[out]  rResult  Returned power consumption value.
   ///                       NULL if a problem.
   /// @retval      True if the read succeeded.
   /// @retval      False if the read failed.
   virtual btBool powerGetValues(INamedValueSet &rResult ) = 0;

}; // class IALIPower

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_IALIAFU_H__

