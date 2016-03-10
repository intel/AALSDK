//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2007-2016, Intel Corporation.
//
//  This program  is  free software;  you  can redistribute it  and/or  modify
//  it  under  the  terms of  version 2 of  the GNU General Public License  as
//  published by the Free Software Foundation.
//
//  This  program  is distributed  in the  hope that it  will  be useful,  but
//  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
//  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
//  General Public License for more details.
//
//  The  full  GNU  General Public License is  included in  this  distribution
//  in the file called README.GPLV2-LICENSE.TXT.
//
//  Contact Information:
//  Henry Mitchel, henry.mitchel at intel.com
//  77 Reed Rd., Hudson, MA  01749
//
//                                BSD LICENSE
//
//  Copyright(c) 2007-2016, Intel Corporation.
//
//  Redistribution and  use  in source  and  binary  forms,  with  or  without
//  modification,  are   permitted  provided  that  the  following  conditions
//  are met:
//
//    * Redistributions  of  source  code  must  retain  the  above  copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in  binary form  must  reproduce  the  above copyright
//      notice,  this  list of  conditions  and  the  following disclaimer  in
//      the   documentation   and/or   other   materials   provided  with  the
//      distribution.
//    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
//      contributors  may  be  used  to  endorse  or promote  products derived
//      from this software without specific prior written permission.
//
//  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
//  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
//  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
//  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
//  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
//  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
//  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
//  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
//  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
//******************************************************************************
//****************************************************************************
//        FILE: ahmpipdefs.h module interface
//     CREATED: 11-12-07
//      AUTHOR: Jospeh Grecco - Intel Corp.
//
// PURPOSE: FAP PIP module hardware interface, i.e., the interface to device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 12/27/2008     JG       removed cacheline alignment from desc headers
// 01/04/2009     HM       Updated Copyright
// 02/07/2009     HM       Cleaned up descriptions of descriptor fields
// 03/17/2009     JG       Added macros for AFU CSRs
// 05/17/2009     Alvin    Update the output descriptor structure
//                            according to the HW changes
// 02/11/2010     JG       Support for kernel 2.6.31
// 06/13/2010     AC       Fixed the AFU CSR address with offset, not the AFU CSR sequence
// 09/19/2010     HM       Added AHMPIP_AHM_CL_SHIFT
// 10/21/2011     JG       Added code for Windows compatibility
//****************************************************************************
#ifndef __AALSDK_KERNEL_AHMPIPDEFS_H__
#define __AALSDK_KERNEL_AHMPIPDEFS_H__

#ifdef __AAL_USER__
#include <stdint.h>
#include <aalsdk/AALTypes.h>
#else
#include <aalsdk/kernel/aaltypes.h>
#endif // __AAL_USER__

BEGIN_NAMESPACE(AAL)

//=============================================================================
// PIP Memory map
//=============================================================================
// Write memory region offset from CSR Base
#define AHMPIP_APERTURE_WRITEOFF    (0)
#define AHMPIP_APERTURE_WRITESIZE   (1 << 20)

// Read memory region
#define AHMPIP_APERTURE_READOFF     (AHMPIP_APERTURE_WRITESIZE)
#define AHMPIP_APERTURE_READSIZE    (1 << 20)

#define AHMPIP_APERTURE_SIZE  (AHMPIP_APERTURE_READSIZE + AHMPIP_APERTURE_WRITESIZE)

#define AHMPIP_AHM_CHANNEL_NUMBER            (4)
#define AHMPIP_SPL_MAXCHANNELS               AHMPIP_AHM_CHANNEL_NUMBER
#define AHMPIP_AHM_CL_SIZE                   (64)  /* Cache line size Bytes*/
#define AHMPIP_AHM_CL_SHIFT                  (6)   /* Cache line bit shift value*/
#define AHMPIP_AHM_DESCRIPTOR_MAX_SIZE       (1024 * AHMPIP_AHM_CL_SIZE)
#define AHMPIP_AHM_DESCRIPTOR_MIN_SIZE       (2 * AHMPIP_AHM_CL_SIZE)
#define AHMPIP_AHM_DESCRIPTOR_SIZE           AHMPIP_AHM_DESCRIPTOR_MAX_SIZE

//////////////////////////////////////////////////////////////////////////////////////////////
static inline
unsigned char * ahmpip_desc_forward( unsigned char *base, unsigned char *ptr, int n  )
{
    if( (ptr + n*AHMPIP_AHM_CL_SIZE) < (base + AHMPIP_AHM_DESCRIPTOR_SIZE) ) {
        return ptr + n*AHMPIP_AHM_CL_SIZE;
    }
    else {
        return (ptr + n*AHMPIP_AHM_CL_SIZE) - AHMPIP_AHM_DESCRIPTOR_SIZE;
    }
}


//==============================================================================
// Control Status Registers (CSR)
//==============================================================================

// Global CSR offset
#define AHMPIP_GCSR_BASE            (0x00000)


// System Protocol Layer Base
#define AHMPIP_SPL_BASE             (0x01000)

// SPL configuration Channel Base
#define AHMPIP_SPL_CFG_BASE         (0x02000)

// SPL Channel Base offset
#define  AHMPIP_SPL_CH_BASE         (0x02000)
#define  AHMPIP_SPL_CH_SIZE         (0x01000)

//SPL Channel n Base offset
#define  AHMPIP_SPL_CHn_BASE(n)     (AHMPIP_SPL_CH_BASE + ((n)*AHMPIP_SPL_CH_SIZE))

// Configuration AFU Base
#define  AHMPIP_AFU_CFG_BASE        (0x10000)
// AFU Base offset
#define  AHMPIP_AFU_BASE            AHMPIP_AFU_CFG_BASE
#define  AHMPIP_AFU_SIZE            (0x10000)

// AFU n Base offset
#define  AHMPIP_REGISTER_SIZE       (0x80)
#define  AHMPIP_AFUn_BASE(n)        (AHMPIP_AFU_BASE + ((n)*AHMPIP_AFU_SIZE))
#define  AHMPIP_MAX_REGISTERS       (AHMPIP_AFU_SIZE/AHMPIP_REGISTER_SIZE)
#define  AHMPIP_MAX_AFU_CSR_INDEX   (AHMPIP_MAX_REGISTERS-1)
#define  AHMPIP_AFUn_CSR(n,offset)  (AHMPIP_AFUn_BASE(n) + (offset))

// Peformance Monitoring Base
#define  AHMPIP_PERFMON_BASE        (0xA0000)

// Debug Base
#define AHMPIP_DBG_BASE             (0xB0000)

// Onboard Memory Base
#define AHMPIP_LCL_MEM_BASE         (0xC0000)

// GCSR_BASE : Global CSR Base Address : Base address of the Global CSR window
// Bit    Attr   Default Description
// 63:32  RV     0x00    Reserved
// 31:12  RW     0x00    Global CSR Base Address : Defines the base address of the
//                       window in system memory to which the CSR space is mapped.
//                       This address must be 4K page aligned, hence the lower
//                       12bits are all zeros
// 11:0   RV     0x00    Reserved
#define AHMPIP_GCSR_BASE_ADDR       (AHMPIP_GCSR_BASE + 0x00000)

// GCSR_SIZE : Global CSR Window Size
// Bit    Attr   Default Description
// 63:32  RV     0x00    Reserved
// 31:20  RV     0x00    Reserved
// 19:0   RW     0x4000 Global CSR Window Size : Specifies the size of the whole
//                       CSR window. The default = 1MB = 0x4000 CLs
#define AHMPIP_GCSR_SIZE             (AHMPIP_GCSR_BASE + 0x0080)

// AHM_CONTROL :  Accelerator Hardwware Module Control. Implements the coarse
//                global control functions of the AHM.
// Bit    Attr   Default Description
// 63:18  RV     0x00    Reserved
//   17   RV     0x00    Reserved
//   16   RV     0x00    Reserved
// 15:1   RV     0x00    Reserved
//   0    RW     0x00    AHM Hard Reset: Writing this bit causes the whole AHM
//                       (including the PHY, FPL, SPL and management AFU, other AFUs) and all
//                       other components( i.e. CPLD, PROM, SRAM etc) on the AHM to be reset. This
//                       method uses the Ct_SPLtoAFU#_Reset_n signal to reset components outside
//                       of the FPGA. The following sequence is used to reset the AHM via this
//                       method.
//                          1.  Software writes AHM_CONTROL[0]=1
//                          2.  AHM hardware stops issuing new requests on the FSB and completes
//                              all pending transactions and then resets all internal logic. This
//                              state is identical to the initial power on state which means that the
//                              AHM will need to be reconfigured i.e. CSR_BAR init, Interrupt init
//                              etc.
//                          3.  AHM hardware resets all CSRs including the GCSR_BASE registers. No
//                              CSR update is performed to reflect the reset values of the CSRs.
//                       Software waits for 250msec before initiating the initialization sequence.
#define AHMPIP_GCSR_AHM_CONTROL             (AHMPIP_GCSR_BASE + 0x0100)

// AHM_CORE_VERSION_ID : Version ID used to identify the RTL release. Used by the
//                       driver to check the version of the FSB PHY,
//                       FSB Protocol and SPL modules. Reading this
//                       register results in a 32bit value in the following format
//                       ProjID-Year-Mon-Day-Version-
//                       Type. Once the CSR initialization process is complete the
//                       hardware updates this CSR to system memory so that the
//                       driver can identify that the AHM has completed the CSR init
//                       process.
// Bit      Attr   Default    Description
// 63:32    RV     0x00       Reserved
// 31:28    RO     0x#        Project ID : One nibble to encode project name
// 27:24    RO     0x#        One nibble representing the year the RTL was released e.g.
//                            0x6 indicates the year 2006
// 23:20    RO     0x#        One nibble representing the month the RTL was released,
//                            e.g. 0xA indicates the 10th month of the year i.e. October
// 19:12    RO     0x##       Two nibbles representing the day the RTL was released. E.g.
//                            0x13 indicates the 19th day of the month
// 11:8     RO     0x#        Version : One nibble to indicate any other release
//                            information required to differentiate multiple releases within
//                            the single day. Subsequent releases within the same day will
//                            have sequentially increasing numbers
// 7:4      RO     0x#        Release Type :
//                               0x0 : Regular release
//                               0x1 : Debug release
//                               Others : Reserved
// 3:1      RV     0x0        Reserved
//  0       RO     0x0        Valid : Indicates the validity of the contents of this register.
//                               0 : AHM_CORE_VERSION_ID invalid
//                               1 : AHM_CORE_VERSION_ID valid
#define AHMPIP_GCSR_AHM_CORE_VERSION_ID   (AHMPIP_SPL_BASE + 0x0000)
#define AHMPIP_GLOBAL_ID (0x0000000008913011)

#define AHMPIP_AHM_CORE_VERSION_VALID(n)  ( (n &        0x01) )
#define AHMPIP_AHM_CORE_VERSION_REL(n)    ( (n &       0x0f0) >>  4 )
#define AHMPIP_AHM_CORE_VERSION_VER(n)    ( (n &      0x0f00) >>  8 )
#define AHMPIP_AHM_CORE_VERSION_DAY(n)    ( (n &    0x0ff000) >> 12 )
#define AHMPIP_AHM_CORE_VERSION_MONTH(n)  ( (n &   0x0f00000) >> 20 )
#define AHMPIP_AHM_CORE_VERSION_YEAR(n)   ( (n &  0x0f000000) >> 24 )
#define AHMPIP_AHM_CORE_VERSION_NAME(n)   ( (n & 0x0f0000000) >> 28 )


// AHM_CORE_PARAMS : Accelerator Parameters : Specifies the configuration of the
//              accelerator hardware. Upon successful initialization.
//              Note : Not supported in first release of hardware
// Bit     Attr   Default    Description
// 63:32   RV     0x00       Reserved
// 31:8    RV     0x00       Reserved
// 7:4     RO     0x##       Number of AFUs : Specifies the number of AFUs
//                           supported in the version of the hardware
// 3:1     RV     0x0        Reserved
// 0:0     RO     0x0        Valid : Indicates the validity of the AHM_PARAMS
//                           register. This bit is set once the GCSR BAR initialization has
//                           successfully completed. The driver polls on this bit during
//                           initialization to get information about the accelerator
//                           hardware.
//                           0x0 : AHM_PARAMS invalid
//                           0x1 : AHM_PARAMS valid
#define AHMPIP_GCSR_AHM_CORE_PARAMS             (AHMPIP_SPL_BASE + 0x0080)


// AHM_STATUS : Accelerator Status : Indicates the status of the accelerator
//              hardware
//              Note : Not supported in first release of hardware
// Bit    Attr  Default    Description
// 63:2   RV    0x0        Reserved
// 1      RO    0x0        Reserved
// 0      RO    0x0        Reserved
#define AHMPIP_GCSR_AHM_STATUS            (AHMPIP_SPL_BASE + 0x0100)


// AHM_INTR_TABLE : AHM Interrupt Vector Table : This table is built using 7 CSRs
//                  that each hold an interrupt vector. The software initializes this table with the available
//                  interrupt vectors and also programs the SPL channels with indexes that enable the
//                  SPL to issue interrupt requests with different vectors. Refer to Section
// Bit     Attr    Default   Description
// 63:2    RV      0x0       Reserved
// 31      RW      0         Valid : Indicates the validity of the rest of the fields in the
//                           register. This bit is set to 0 for unimplemented entries of the
//                           table
// 19:13   RW      0         Used in Address[19:4] of the interrupt; corresponds to the
//                           interrupt target (the “zzzz” field)
// 12:8    RW      0         Used in Address[3:0] of the interrupt; corresponds to the “Y”
//                           field.
// 7:0     RW      0         Used in the data[7:0] of the interrupt; corresponds to the
//                           vector

// Global Interrupt Control Base
#define AHMPIP_INTR_TABLE_BASE      (AHMPIP_SPL_BASE + 0x0180)
#define AHMPIP_INTR_TABLE_SIZE      (0x0080)

// Interrupt vector table entries
#define AHMPIP_GCSR_AHM_INTR_TABLE_ENTRYS       (7)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT(x)      (AHMPIP_INTR_TABLE_BASE + (x*AHMPIP_INTR_TABLE_SIZE))
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT0        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(0)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT1        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(1)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT2        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(2)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT3        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(3)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT4        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(4)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT5        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(5)
#define AHMPIP_GCSR_AHM_INTR_TABLE_VECT6        AHMPIP_GCSR_AHM_INTR_TABLE_VECT(6)

// SPL_CONTROL : SPL Control Register
// Bit    Attr   Default  Description
// 63:2   RV     0x0      Reserved
// 32:9   RV     0        Reserved
//  8     RV     0        Reserved
// 7:4    RV     0        Reserved
// 3:0    RV     0        Reserved
#define AHMPIP_GCSR_SPL_CONTROL                  (AHMPIP_SPL_BASE + 0x0500)

// SPL_STATUS : SPL Status. Indicates the status/configuration of the SPL.
// Bit   Attr   Default  Description
// 63:20  RV     0x0      Reserved
// 19:12  RV       0      Reserved
// 11: 8  RV       0      Reserved
// 7:4    RO       0      Indicates the number of AFUs that are connected to
//                        the SPL. The SPL uses the Ct_AFU#toSPL_AFU_Presence signal to determine if
//                        a AFU is present. An AFU that is present drives this signal high.
//                        0000: No AFUs implemented
//                        0001: AFU0 implemented
//                        1011: AFU0, AFU1, AFU3 are implmented, the other AFUs are not implemented.
// 3:1     RV       0      Reserved
// 0       RO       0      Valid: Indicates the validity of the contents of
//                         this register.
//                         0: SPL_CONFIG_STATUS invalid
//                         1: SPL_CONFIG_STATUS valid
#define AHMPIP_GCSR_SPL_STATUS              (AHMPIP_SPL_BASE + 0x0580)
#define AHMPIP_NO_AFUS                      0x0
#define AHMPIP_VALID_SPL_STATUS_IMP         0x1
#define AHMPIP_AFU0_IMP                     (0x1<<4)
#define AHMPIP_AFU1_IMP                     (0x1<<5)
#define AHMPIP_AFU2_IMP                     (0x1<<6)
#define AHMPIP_AFU3_IMP                     (0x1<<7)
#define AHMPIP_AFUn_IMP(n)                  (0x1<<(4+n))

// SPL_CH#_CTRL : SPL Channel Control : Provides access to the SPL DMA engine
// Bit     Attr   Default    Description
// 63:8    RV     0x00       Reserved
// 7:0     RV     0          Command: Specifies commands for the SPL Channel.
//
//                           0x0: NOP. Has no effect on the SPL Channel.
//                           0x1: SPL Channel Enable. Enable SPL Channel. Not
//                                meant to be used dynamically, should be used only
//                                during startup.
//                           0x2: SPL Channel Disable. Disable SPL Channel. Not
//                                meant to be used dynamically, should be used only
//                                during startup.
//                           0x3: SPL Channel Reset. Causes the SPL and the
//                                associated AFU to  be reset. All descriptors in
//                                flight will be lost, no status update is
//                                performed for the lost descriptors. The AFU# ID
//                                needs ot be updated.
//                           Others: Reserved.
#define AHMPIP_GCSR_SPL_CHn_CTRL(n)             (AHMPIP_SPL_CHn_BASE(n) + 0x0000)
#define AHMPIP_SPL_CHCMD_ENABLE                 (0x1)
#define AHMPIP_SPL_CHCMD_DISABLE                (0x2)
#define AHMPIP_SPL_CHCMD_RESET                  (0x3)


// SPL_CH#_STATUS : SPL Channel Status register : Reflects the state of the SPL CH#
// Bit   Attr   Default  Description
// 63:4  RV     0        Rserved
// 3:2   RV     0        Rserved
// 1     RO     0        SPL Channel Reset Complete: Indicates the completion
//                       of a SPL Channel reset sequeue initiated via SPL_CH#_CTRL[7:0]
//
//                       0x0: SPL channel reset not initiated, OR, if SPL
//                            channel
//                       0x1: SPL channel reset completed
// 0     RO     0        SPL Channel Status: Indicates if the SPL channel is
//                       enabled or disabled.
//                       0x0: SPL Channel disabled
//                       0x0: SPL Channel enabled
#define AHMPIP_GCSR_SPL_CHn_STATUS(n)           (AHMPIP_SPL_CHn_BASE(n) + 0x0080)
#define AHMPIP_SPL_CHSTS_DISABLED               (0x0)
#define AHMPIP_SPL_CHSTS_ENABLED                (0x1)
#define AHMPIP_SPL_CHSTS_RESET_COMPLETED        (0x2)

// SPL_CH#_IN_DESC_BASE: Base address of the Input descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Input Descriptor Queue Base Address: the Input
//                       descriptor queue base address(physical). This address has to be 64KB
//                       aligned.
#define AHMPIP_GCSR_SPL_CHn_IN_DESC_BASE(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0100)

// SPL_CH#_IN_DESC_SIZE: Size of the Input descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:16 RV     0        Rserved
// 15:0  RW     0        Input Descriptor Queue Size: Specifies the size of
//                       the input descriptor queue in bytes. The size has to be a multiple of a CL
//                       which means that bits[5:0] are always zero
//
//                       Min: 0x80:     128B = 2CL
//                       Max: 0xFFFF:   64KB = 1024CL
#define AHMPIP_GCSR_SPL_CHn_IN_DESC_SIZE(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0180)

// SPL_CH#_IN_DESC_TAIL: Tail pointer of the Input descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Input Descriptor Queue Tail Pointer: This register
//                       holds the value of the tail pointer for the input descriptor queue. The
//                       tail pointer points to the start of the first invalid cacheline which is
//                       the location for the next enqueued descriptor. Software writes the physical
//                       address of cacheline into this register but HW uses only bits[15:6] as an
//                       offset relative to the SPL_CH#_IN_DESC_BASE. This register is updated by
//                       software when the new descriptor has been added to the input descriptor
//                       queue. Refer to Section 1.1 for more deatails.
#define AHMPIP_GCSR_SPL_CHn_IN_DESC_TAIL(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0200)

// SPL_CH#_IN_DESC_HEAD: Head pointer of the Input descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Input Descriptor Queue Head Pointer: This register
//                       holds the value of the head pointer for the input descriptor queue. The
//                       head pointer points to the descriptor that is
//                       currently being processed by the AHM. This register is updated by
//                       Hardware when it completes processing a descriptor.
//                       Hardware updates only bits[15:6]. The bits[15:6] are
//                       used as an offset relative to the SPL_CH#_IN_DESC_BASE.
#define AHMPIP_GCSR_SPL_CHn_IN_DESC_HEAD(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0280)


// SPL_CH#_OUT_DESC_BASE: Base address of the Output descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Output Descriptor Queue Base Address: the Output
//                       descriptor queue base address(physical). This address has to be 64KB
//                       aligned.
#define AHMPIP_GCSR_SPL_CHn_OUT_DESC_BASE(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0300)

// SPL_CH#_OUT_DESC_SIZE: Size of the Output descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:16 RV     0        Rserved
// 15:0  RW     0        Output Descriptor Queue Size: Specifies the size of
//                       the Output descriptor queue in bytes. The size has to be a multiple of a CL
//                       which means that bits[5:0] are always zero
//
//                       Min: 0x80:     128B = 2CL
//                       Max: 0xFFFF:   64KB = 1024CL
#define AHMPIP_GCSR_SPL_CHn_OUT_DESC_SIZE(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0380)

// SPL_CH#_OUT_DESC_TAIL: Tail pointer of the Output descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Output Descriptor Queue Tail Pointer: This register
//                       holds the value of the tail pointer for the Output descriptor queue. The
//                       tail pointer points to the start of the first invalid cacheline which is
//                       the location for the next enqueued descriptor. Software writes the physical
//                       address of cacheline into this register but HW uses only bits[15:6] as an
//                       offset relative to the SPL_CH#_OUT_DESC_BASE. This register is updated by
//                       software when the new descriptor has been added to the input descriptor
//                       queue. Refer to Section 1.1 for more deatails.
#define AHMPIP_GCSR_SPL_CHn_OUT_DESC_TAIL(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0400)

// SPL_CH#_OUT_DESC_HEAD: Head pointer of the Output descriptor queue for CH#
// Bit   Attr   Default  Description
// 63:40 RV     0        Rserved
// 39:0  RW     0        Output Descriptor Queue Head Pointer: This register
//                       holds the value of the head pointer for the output descriptor queue. The
//                       head pointer points to the descriptor that is
//                       currently being processed by the AHM. This register is updated by
//                       Hardware when it completes processing a descriptor.
//                       Hardware updates only bits[15:6]. The bits[15:6] are
//                       used as an offset relative to the SPL_CH#_OUT_DESC_BASE.
#define AHMPIP_GCSR_SPL_CHn_OUT_DESC_HEAD(n)    (AHMPIP_SPL_CHn_BASE(n) + 0x0480)

// SPL_CH#_INTR_PENDING:  Task Completion Interrupt Pending: Indicates if a
//                        interrupt issued by the SPL channel to indicate the completion of one or
//                        more tasks is pending.
// Bit   Attr   Default  Description
// 63:36 RV     0         Rserved
// 15:6  RO     0         Completed task pointer: Head pointer value of the
//                        output descriptor queue. The value written in this field should point to
//                        the last descriptor (i.e. output descriptor with EOT set) of the last
//                        completed task that requires an interrupt to be issued. This fiedl is
//                        updated by hardware only upon completion of a task that requires and
//                        interrrupt to be issued.
// 5:1   RV     0         Rserved
// 0     RO     0         Interrupt Pending: This bit indicates whether an
//                        outstanding interrupt has been issued and is waiting to be serviced by the
//                        software. This bit is set (i.e. 0x1) by hardware upon completion of a task
//                        that requires and interrupt to be issued. The hardware uses this bit to
//                        determine if a pending interrupt has been serviced. The software clears
//                        this bit when the interrupt has been serviced. The hardware clears is no
//                        tallowed to issue any subsequent interrupts till this bit is cleared.
//                        Software clears this bit/register by writing SPL_CH#_INTR_CLR register. See
//                        SPL_CH#_INTR_CLR[0] for algorithm used to clear this register.
#define AHMPIP_GCSR_SPL_CHn_INTR_PENDING(n)        (AHMPIP_SPL_CHn_BASE(n) + 0x0500)

// SPL_CH#_INTR_CLR: Task Completion Interrrupt Pending Clear: Register that
//                   indicates to the hardware that the software has finished servicing the
//                   interrupt that indicates completion of a task.
// Bit   Attr   Default  Description
// 63:16 RV     0         Rserved
// 15:6  RW     0         Completed task pointer: This field contains the
//                        value of the output descriptor head pointer at the time that the software
//                        read the SPL_CH#_INTR_PENDING. Software writes this count which is used by
//                        hardware to identify if any more tasks were completed by hardware since the
//                        software read the SPL_CH#_INTR_PENDING register.
// 5:1   RV     0         Rserved
// 0     RW     0         Interrupt Pending Clear: Software writes this bit to
//                        indicate it has completed processing an interrupt.
//                        HW algorithm to process SPL_CH#_INTR_CLR and SPL_CH#_INTR_PENDING
//                            if (SPL_CH#_INTR_CLR[0] = 1 && SPL_CH#_INTR_PENDING[15:6]==SPL_CH#_INTR_CLR[15:6] )
//                                a. SPL_CH#_INTR_PENDING = 0
//                                b. SPL_CH#_INTR_CLR = 0
//                                c. HW does NOT issue interrupt since no new tasks have completed
//                                   since SW last serviced the interrupt.
//                            else
//                                a. SPL_CH#_INTR_PENDING[0] = 1
//                                b. Update SPL_CHE#_INTR_PENDING[15:6] with latest completion task count
//                                c. HW issue new interrupt since more tasks have been completed
//                                   since SW serviced the last interrupt.
#define AHMPIP_GCSR_SPL_CHn_INTR_CLR(n)           (AHMPIP_SPL_CHn_BASE(n) + 0x0580)

// SPL_CH#_INTR_INDX: SPL Channel Interrupt Index register: Specifies the
//                    interrupt index for a SPL interrupt request to the FSB module. The FSB
//                    module uses this index as a lookup into a table to get the actual interrupt
//                    vector issued on the FSB. This is a channel specific register which allows
//                    the SPL to issue interrupts with different vector for the various channels.
//                    This will make software service of the interrupts more efficient.
// Bit   Attr   Default  Description
// 63:32  RV     0         Rserved
// 31:3   RV     0         Rserved
// 2:0    RW     0         3 bit index allows SPL to issue interrupts with
//                         unique vectors for each channel. This index is used by the SPL in its
//                         internal request to the FSB module.
#define AHMPIP_GCSR_SPL_CHn_INTR_INDX(n)          (AHMPIP_SPL_CHn_BASE(n) + 0x0600)


// AFU#_VERSION_ID: Used by SW to identify the AFUs present in the hardware. This enables the SW to load
//                  the AFU specific services depending on the specific AFU.
// Bit   Attr   Default  Description
// 63:0   RO     0x00    This is a unique (preferably random) 64bit ID the enables the software to identify
//                       the AFU. The validity of this register is indicated by SPL_STATUS[12][13][14][15]
//                       depending on the CH#. This register is updated (in HW and system memory) when a
//                       "AHM reset" or "SPL Channel Reset" or "AFU reset" occurs.
//                  NOTE:This ID has to be a non zero value if Ct_AFU#toSPL Presence signal is asserted.
#define AHMPIP_GCSR_AFUn_VERSION_ID(n)              (AHMPIP_AFUn_BASE(n) + 0x0000)


// AFUn_CTRL: Control register used to implement functionality like AFU reset.
// Bit   Attr   Default  Description
// 63:20  RV     0         Reserved
// 19:4   RW     0         Task Abort TID (Optional): This field contains the Task ID (TID) of the task to be aborted.
//                         SW writes this field along with AFU_CONTROL[1]=1. Upon completion of the abort the AFU resets
//                         this bit along with this whole register.
//  3:2   RV     0         Reserved
//  1     RW     0         Task abort (Optional): This bit is used to abort a task (based on TID). SW writes this bit if
//                         it wants to abort the task that is currently being worked on by the HW. The current task is based
//                         on the input descriptor being processed. The AFU should implement all HW hooks necessary perform the
//                         abort as described below:
#define AHMPIP_GCSR_AFUn_CTRL(n)                    (AHMPIP_AFUn_BASE(n) + 0x0080)

#define AHMPIP_AFU_CTRL_ABORT_TASK(t)               ((t<<3) & 0x1 )

#ifndef __linux__
#define __attribute__(p)
#pragma pack(1)
#endif

//==================================
//  ahmpip_desc - Descriptor structure
//==================================
struct ahm_input_desc_header {
   btUnsigned16bitInt  m_size;             // Descriptor size in CL
   btUnsigned16bitInt  m_ctrl;             // Control Bits
#define AHM_IN_DESC_MASTER         1
#define AHM_IN_DESC_DMA            (1<<1)
#define AHM_IN_DESC_VIRT           (1<<2)
#define AHM_IN_DESC_PGTBL_PRE      (1<<3)
#define AHM_IN_DESC_COMP_INTR_EN   (1<<4)
#define AHM_IN_DESC_SOT            (1<<5)
#define AHM_IN_DESC_MOT            (1<<6)
#define AHM_IN_DESC_EOT            (1<<7)
   btByte              m_pgsize;             // VMM Super-Page size in CL
   btUnsigned64bitInt  m_pgtbl;              // VMM Page table Physical Address
   btUnsigned16bitInt  m_pgtblsize;          // VMM Page Table size, 1=64 entries
   btByte              m_rsvd8_1;            // reserved
   btUnsigned64bitInt  m_input;              // Physical Byte Address of Input Buffer
   btUnsigned64bitInt  m_isize:18;           // Input buffer size in CL
   btUnsigned64bitInt  m_rsv6:6;           
   btUnsigned64bitInt  m_status:16;          // Status
#define AHM_IN_DESC_COMPLETE(s)  ( (s) & 1 )
#define AHM_IN_DESC_PENDING(s)   ( !(AHM_IN_DESC_COMPLETE(s)) )
#define AHM_IN_DESC_ERROR(s)     ( (s) & (1<<1) )
#define AHM_IN_DESC_SUCCESS(s)   ( !( AHM_IN_DESC_ERROR(s) ))
   btUnsigned64bitInt  m_func_id:4;          // Function ID
   btUnsigned64bitInt  m_tid:16;             // Task ID
   btUnsigned64bitInt  m_rsvd4:4;            // Reserved
} __attribute__ ((packed));



struct ahm_output_desc_header {
   btUnsigned64bitInt  m_dsize:18;            // Number of CL's written to this buffer
   btUnsigned64bitInt  m_rsv6_1:6;            
   btUnsigned64bitInt  m_ctrl:16;             // Control Bits
#define AHM_OUT_DESC_COMP_INTR_EN   (1<<4)  /* Completion Interrupt Enable */
#define AHM_OUT_DESC_SOT            (1<<5)
#define AHM_OUT_DESC_MOT            (1<<6)
#define AHM_OUT_DESC_EOT            (1<<7)
   btUnsigned64bitInt  m_rsvd24:24;           // reserved
   btUnsigned64bitInt  m_rsvd64_1;         // reserved
   btUnsigned64bitInt  m_output;           // Physical Byte Address of Output Buffer
   btUnsigned64bitInt  m_osize:18;              // Output Buffer size in CL
   btUnsigned64bitInt  m_rsv6_2:6;            
   btUnsigned64bitInt  m_status:16;           // Status
#define AHM_OUT_DESC_COMPLETE(s) ( (s) & 1 )
#define AHM_OUT_DESC_PENDING(s)  ( !(AHM_IN_DESC_COMPLETE(s)) )
#define AHM_OUT_DESC_ERROR(s)    ( (s) & (1<<1) )
#define AHM_OUT_DESC_SUCCESS(s)  ( !( AHM_IN_DESC_ERROR(s) ))
   btUnsigned64bitInt  m_func_id:4;         // Function ID
   btUnsigned64bitInt  m_tid:16;           // Task ID
   btUnsigned64bitInt  m_rsvd4:4;          // Reserved
} __attribute__ ((packed));

#define AHM_OUT_AFU_STATUS(p)    (unsigned char*)p+sizeof(struct ahm_output_desc_header)
#define AHM_AFU_AFU_STATUS_LEN   32

#define AHMPIP_MAX_BUFFER_SIZE               ( ((1<<16) - 1) * AHMPIP_AHM_CL_SIZE )


END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AHMPIPDEFS_H__

