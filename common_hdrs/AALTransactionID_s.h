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
/// @file AALTransactionID_s.h
/// @brief Definition of the shared TransactionID_s structure.
/// @ingroup Events
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 11/17/2008     JG       Initial version
/// 01/04/2009     HM       Updated Copyright
/// 05/15/2015     JG       Added Support for IBase@endverbatim
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALTRANSACTIONID_S_H__
#define __AALSDK_KERNEL_AALTRANSACTIONID_S_H__

// This file is shared across user and kernel space.
#if defined( __AAL_KERNEL__ )
# include <aalsdk/kernel/aaltypes.h>
#else
# include <aalsdk/AALTypes.h>
#endif // __AAL_USER__


BEGIN_NAMESPACE(AAL)

#ifdef __cplusplus
   class IBase;
#else
   typedef struct IBase IBase;
#endif // __cplusplus

/// User/Kernel shared Transaction ID structure.
/// @ingroup Events
typedef struct stTransactionID_t
{
   btApplicationContext m_ID;
   btEventHandler       m_Handler;
   IBase               *m_IBase;
   btBool               m_Filter;
   bt32bitInt           m_intID;
} stTransactionID_t;


// Note: using an absolute number here (32, 24) to ensure that both user and kernel are getting
//       the same sized struct.
#if   (8 == sizeof_void_ptr)
   CASSERT(32 == sizeof(stTransactionID_t));
#elif (4 == sizeof_void_ptr)
   CASSERT(24 == sizeof(stTransactionID_t));
#else
#  error Add stTransactionID_t size check for unknown address size.
#endif // sizeof_void_ptr

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALTRANSACTIONID_S_H__

