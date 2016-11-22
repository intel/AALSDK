// Copyright(c) 2015-2016, Intel Corporation
// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __AALSDK_TEST_DMABUFFER5_H_
#define __AALSDK_TEST_DMABUFFER5_H_

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
/// @file DMABuffer5.h
/// @brief header file for 5th DMA buffer test app
/// @ingroup DMA_Buffer
/// @verbatim
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/09/2016     RP       Initial version started based on older sample
//****************************************************************************

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>

#include <string.h>
#include "arguments.h"

//****************************************************************************
// UN-COMMENT appropriate #define in order to enable either Hardware or ASE.
//    DEFAULT is to use Software Simulation.
//****************************************************************************
//#define  HWAFU
//#define  ASEAFU

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

#ifndef CL
# define CL(x)                     ((x) * 64)
#endif // CL
#ifndef LOG2_CL
# define LOG2_CL                   6
#endif // LOG2_CL
#ifndef MB
# define MB(x)                     ((x) * 1024 * 1024)
#endif // MB
#define LPBK1_BUFFER_SIZE        MB(2)
#define MMIO_SIZE                (256 * 1024) //(32 bit hdr data * 2^16) is the size of WHOLE AFU MMMIO region
#define MMIO_Start_Address       0x00
#define SAS_HW_MMIO_SIZE         (256 * 1024)
#define Data_4_Byte              0x12345678
#define Data_8_Byte              0x1122334455667788
#define TargetFeatureOffSet      0x38


#define LPBK1_DSM_SIZE           MB(4)
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define CSR_NUM_LINES            0x0130
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114
#define NLB_TEST_MODE_PCIE0      0x2000

/// @addtogroup DMA_Buffer
/// @{


/// @brief   Since this is a simple testing application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class DMABuffer5: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

    DMABuffer5();
    ~DMABuffer5();

    btInt  run(const arguments &args); ///< Return 0 if success
    btInt  testVAPATranslation();

    // <begin IServiceClient interface>
    void serviceAllocated(IBase *pServiceBase,
                          TransactionID const &rTranID);

    void serviceAllocateFailed(const IEvent &rEvent);

    void serviceReleased(const AAL::TransactionID&);
    void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
    void serviceReleaseFailed(const AAL::IEvent&);

    void serviceEvent(const IEvent &rEvent);
    // <end IServiceClient interface>

    // <begin IRuntimeClient interface>
    void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};    // Not Used

    void runtimeStarted(IRuntime *pRuntime,
                        const NamedValueSet &rConfigParms);

    void runtimeStopped(IRuntime *pRuntime);

    void runtimeStartFailed(const IEvent &rEvent);

    void runtimeStopFailed(const IEvent &rEvent);

    void runtimeAllocateServiceFailed(IEvent const &rEvent);

    void runtimeAllocateServiceSucceeded(IBase *pClient,
                                         TransactionID const &rTranID);

    void runtimeEvent(const IEvent &rEvent);

    btBool isOK() {return m_bIsOK;}

    // <end IRuntimeClient interface>
protected:
    Runtime        m_Runtime;           ///< AAL Runtime
    IBase         *m_pAALService;       ///< The generic AAL Service interface for the AFU.
    IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
    IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
    IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
    CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
    btInt          m_Result;            ///< Returned result value; 0 if success

    // Workspace info
    btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
    btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
    btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
    btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
    btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
    btWSSize       m_InputSize;      ///< Input workspace size in bytes.
    btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
    btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
    btWSSize       m_OutputSize;     ///< Output workspace size in bytes.

    //run time environment flag
    btString       m_strPlatform;       /// test platform selected by user

    //get service routine for internal use only
    btBool _getALIMMIOService(const arguments &args);

};






#endif /* __AALSDK_TEST_DMABUFFER5_H_ */
