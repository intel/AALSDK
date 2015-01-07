//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2015, Intel Corporation.
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
//  Copyright(c) 2008-2015, Intel Corporation.
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
//        FILE: aalids.h
//     CREATED: 04/15/2008
//      AUTHOR: Alvin Chen - Intel
//
// PURPOSE: AAL IDs definitions.
//
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 04/15/10       AC       Initial version created
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALIDS_H__
#define __AALSDK_KERNEL_AALIDS_H__


/////////////////////////////////////////////////////////////////////////////////
// Device ID information
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// ASM related IDs
/////////////////////////////////////////////////////////////////////////////////
//#define  ASM_DEVICE_ID_AHM   (0xf0f0)       //Emulated device
//#define  ASM_DEVICE_ID_AFU   (0x0001)       //Emulated AFU
#define  ASM_AHM_GUID         (0x3333111133331111LL)

#define  ASM_AFU_GUID         (0x40040586)   //Emulated AHM GUID

#define  ASM_MAFU_GUID        (0xA0A0A0A0A0A0A0A0LL)

// Class Device ID informaton
#define ASM_CLASS_MAJVERSION  (0x00000001)
#define ASM_CLASS_MINVERSION  (0x00000000)
#define ASM_CLASS_RELEASE     (0x00000001)

/////////////////////////////////////////////////////////////////////////////////
// FSB related IDs
/////////////////////////////////////////////////////////////////////////////////
#define  FSB_AHM_GUID         (0x90808086)   //Emulated AHM GUID
#define  FSB_AFU_GUID         (0x50040586)   //Emulated AHM GUID

// Class Device ID information
#define FSB_CLASS_MAJVERSION  (0x00000002)
#define FSB_CLASS_MINVERSION  (0x00000000)
#define FSB_CLASS_RELEASE     (0x00000001)


/////////////////////////////////////////////////////////////////////////////////
// HOST AFU related IDs
/////////////////////////////////////////////////////////////////////////////////
#define  HOST_AHM_GUID        (0xffff0000ffff0000LL)


#define  HOST_MAFU_IID        (0xffff0000ffff0001LL)
#define  HOST_AFU_IID         (0xffff0000ffff0002LL)


#define  HOST_MAFUPIP_IID              (0xffff0000fffe0001LL)
#define  HOST_AFUPIP_IID               (0xffff0000fffe0002LL)
#define  HOST_SAMPLE_AFUPIP_IID        (0xffff0000fffe0003LL)

#define  HOST_EDGESAMPLE_AFUPIP_IID    (0xffff0000fffe0004LL)

#define  HOST_MAFUAPI_IID        (0xffff0000fffd0001LL)
#define  HOST_AFUAPI_IID         (0xffff0000fffd0002LL)
#define  HOST_HEALAFUAPI_IID     (0xffff0000fffd0003LL)
#define  HOST_SAMPLE_AFUAPI_IID  (0xffff0000fffd0004LL)


/////////////////////////////////////////////////////////////////////////////////
// QPI related IDs
/////////////////////////////////////////////////////////////////////////////////
#define  QPI_AHM_GUID        (0xffff0000dfff0000LL)


// AFU IDs, 0xffff0000dfffxxxx
#define  QPI_MAFU_IID        (0xffff0000dfff0001LL)
#define  QPI_CCIAFU_IID      (0xffff0000dfff0002LL)   //TODO This has to change
#define  QPI_CCISIMAFU_IID   (0xffff0000dfff0003LL)   //TODO This has to change
#define  QPI_SPLAFU_IID      (0xffff0000dfff0004LL)   //TODO This has to change

// PIP IDs, 0xffff0000dffexxxx
#define  QPI_MAFUPIP_IID      (0xffff0000dffe0001LL)
#define  QPI_CCIAFUPIP_IID    (0xffff0000dffe0002LL)
#define  QPI_CCISIMAFUPIP_IID (0xffff0000dffe0004LL)

// API IDs, 0xffff0000dffdxxxx
#define  QPI_MAFUAPI_IID      (0xffff0000dffd0001LL)
#define  QPI_CCIAFUAPI_IID    (0xffff0000dffd0002LL)
#define  QPI_SPLAFUAPI_IID    (0xffff0000dffd0003LL)
#define  QPI_CCISIMAFUAPI_IID (0xffff0000dffd0004LL)



#define SPL2_MAFUPIP_IID      (0xbb353944ae885dddLL)
#define SPL2_AFUPIP_IID       (0x8ee7ef537e245c28LL)
#define SPL2_AFUAPI_IID       (0xa579bdeabc56d3c5LL)

/////////////////////////////////////////////////////////////////////////////////
// QPI related IDs
/////////////////////////////////////////////////////////////////////////////////

#endif // __AALSDK_KERNEL_AALIDS_H__

