//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2014, Intel Corporation.
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
//  Copyright(c) 2008-2014, Intel Corporation.
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
/// @file AALWorkspace.h
/// @brief Definition of the shared(user/kernel) Workspace enums and structures.
/// @ingroup AALCore
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Alvin Chen,    Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/03/2008     HM       Initial version
/// 01/04/2009     HM       Updated Copyright
/// 06/10/2012     HM       aaltypes.h is in the kernel. Fixed access.
/// 06/11/2012     HM       backed out previous change. Everything breaks.
///                            re-opened redmine[110]@endverbatim
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALWORKSPACE__
#define __AALSDK_KERNEL_AALWORKSPACE__
#include <aalsdk/kernel/aaltypes.h>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// WARNING!! AHM_DESC_* MUST TRACK THE EQUIVALENT DEFINITIONS IN FAPPIPDEFS.H
//
// If for some reason they cannot, then a conversion layer is needed.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define AHM_DESC_SOT          (1<<5)
#define AHM_DESC_MOT          (1<<6)
#define AHM_DESC_EOT          (1<<7)


BEGIN_NAMESPACE(AAL)

   typedef enum
   {
      MASTER_PHYS_MODE    = 0,
      MASTER_VIRT_MODE    = 1,
      SLAVE_MODE          = 2,
   } TTASK_MODE;

   /// Task descriptor type.
   /// @ingroup BasicTypes
   typedef enum
   {
      INPUT_DESC    = 0, ///< Input descriptor.
      OUTPUT_DESC   = 1, ///< Output descriptor.
   } TDESC_TYPE;

   typedef enum
   {
      START_OF_TASK   = AHM_DESC_SOT,
      MIDDLE_OF_TASK  = AHM_DESC_MOT,
      END_OF_TASK     = AHM_DESC_EOT,
      COMPLETE_TASK   = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT
   } TDESC_POSITION;

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALWORKSPACE__
