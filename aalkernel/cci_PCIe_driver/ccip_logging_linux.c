//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
/// @file ccip_logging_linux.c
/// @brief  Definitions for ccip logging.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_logging_linux.c
//     CREATED: June 07, 2016
//      AUTHOR: , Intel Corporation
//
//
// PURPOSE:
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD
#include "aalsdk/kernel/ccip_defs.h"
#include "ccip_logging_linux.h"
#include "ccip_logging.h"

BEGIN_NAMESPACE(AAL)

// Logging Timer struct
extern struct logging_msg   g_logging_msg;

///============================================================================
/// Name: ccip_logging_timervalue_attrib_show
/// @brief Writes ccip logging timer values  to sysfs
///
/// @param[in] pdriver - driver pointer
/// @param[in] buf     - char buffer.
/// @return    size of buffer
///============================================================================
static ssize_t ccip_logging_timervalue_attrib_show(struct device_driver *pdriver,
                                                   char *buf)
{
   kosal_sem_get_krnl(logging_msg_sem(g_logging_msg) );

   printk( "g_logging_timer_value = %ld\n",logging_msg_time(g_logging_msg));

   kosal_sem_put(logging_msg_sem(g_logging_msg) );

   return (snprintf(buf,PAGE_SIZE,   "%lu\n",(unsigned long int)g_logging_msg.m_logging_timer_value ));

}

///============================================================================
/// Name: ccip_logging_timervalue_attrib_store_debug
/// @brief Reads ccip logging timer values  from sysfs.
///
/// @param[in] pdriver - driver pointer
/// @param[in] buf     - char buffer.
/// @return    size of buffer
///============================================================================
static ssize_t ccip_logging_timervalue_attrib_store_debug(struct device_driver *pdriver,
                                                          const char *buf,
                                                          size_t size)
{
   kosal_sem_get_krnl(logging_msg_sem(g_logging_msg) );

   sscanf(buf,"%ld", &logging_msg_time(g_logging_msg));
   printk("g_logging_timer_value= %ld\n", g_logging_msg.m_logging_timer_value);

   kosal_sem_put( logging_msg_sem(g_logging_msg));
   return size;
}

DRIVER_ATTR(logging_timer,S_IRUGO|S_IWUSR|S_IWGRP, ccip_logging_timervalue_attrib_show,ccip_logging_timervalue_attrib_store_debug);


///============================================================================
/// Name: create_logging_timervalue_sysfs
/// @brief create logging timer entry in sysfs
///
/// @param[in] pdriver - pci driver pointer
/// @return    error code
///============================================================================
bt32bitInt create_logging_timervalue_sysfs(struct device_driver *pdriver)
{
   int res = 0;

   PTRACEIN;
   if((NULL == pdriver))  {

       PERR("Invalid input pointers \n");
       res = -EINVAL;
       goto ERR;
    }

   res = driver_create_file( pdriver, &driver_attr_logging_timer );

ERR:
   PTRACEOUT_INT(res);
   return  res;
}

///============================================================================
/// Name: remove_logging_timervalue_syfs
/// @brief remove logging timer entry in sysfs
///
/// @param[in] ppcidev  pci device pointer.
/// @return    error code
///============================================================================
bt32bitInt remove_logging_timervalue_syfs(struct device_driver *pdriver)
{
   int res = 0;

   PTRACEIN;
   if((NULL == pdriver)) {

       PERR("Invalid input pointers \n");
       res = -EINVAL;
       goto ERR;
    }

   driver_remove_file(pdriver,&driver_attr_logging_timer);

   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;
}

END_NAMESPACE(AAL)
