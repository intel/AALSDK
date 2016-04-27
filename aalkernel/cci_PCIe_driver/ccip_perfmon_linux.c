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
/// @file ccip_perfmon_linux.c
/// @brief  Definitions for ccip performance monitor sysfs support.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_perfmon_linux.c
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel Corporation
//              Joseph Grecco, Intel Corporation
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD
#include "aalsdk/kernel/ccip_defs.h"

#include "ccip_perfmon_linux.h"
#include "ccip_fme.h"


///============================================================================
/// Name: perf_monitor_attrib_show_hr
/// @brief Writes and show CCIP Performance counters to sysfs
///
/// @param[in] pdev - device pointer
/// @param[in] attr - device attribute.
/// @param[in] buf - char buffer.
/// @return    size of buffer
///============================================================================
static ssize_t perf_monitor_attrib_show_hr(struct device *pdev,
                                           struct device_attribute *attr,
                                           char *buf)
{
   struct CCIP_PERF_COUNTERS perf_mon;
   struct fme_device* pfme_dev = dev_get_drvdata(pdev);

   if(NULL == pfme_dev )  {
     return (snprintf(buf,PAGE_SIZE,"%d\n",0));
   }

   get_perfmonitor_snapshot(pfme_dev, &perf_mon);

   return (snprintf(buf,PAGE_SIZE,   "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n"
                                     "%s : %lu \n",
                                     NUM_COUNTERS,               (unsigned long int) perf_mon.num_counters.value ,
                                     PMONITOR_VERSION,           (unsigned long int) perf_mon.version.value ,
                                     CACHE_READ_HIT,             (unsigned long int) perf_mon.read_hit.value ,
                                     CACHE_WRITE_HIT,            (unsigned long int) perf_mon.write_hit.value ,
                                     CACHE_READ_MISS,            (unsigned long int) perf_mon.read_miss.value,
                                     CACHE_WRITE_MISS,           (unsigned long int) perf_mon.write_miss.value,
                                     CACHE_EVICTIONS,            (unsigned long int) perf_mon.evictions.value,
                                     FABRIC_PCIE0_READ,          (unsigned long int) perf_mon.pcie0_read.value ,
                                     FABRIC_PCIE0_WRITE,         (unsigned long int) perf_mon.pcie0_write.value ,
                                     FABRIC_PCIE1_READ,          (unsigned long int) perf_mon.pcie1_read.value ,
                                     FABRIC_PCIE1_WRITE,         (unsigned long int) perf_mon.pcie1_write.value ,
                                     FABRIC_UPI_READ,            (unsigned long int) perf_mon.upi_read.value ,
                                     FABRIC_UPI_WRITE,           (unsigned long int) perf_mon.upi_write.value
                                     ));

}

DEVICE_ATTR(perfmon_hr_0,0444, perf_monitor_attrib_show_hr,NULL);

///============================================================================
/// Name: perf_monitor_attrib_show_bin
/// @brief shows CCIP Performance counters
///
/// @param[in] pdev - device pointer
/// @param[in] attr - device attribute.
/// @param[in] buf - char buffer.
/// @return    size of buffer
///============================================================================
static ssize_t perf_monitor_attrib_show_bin(struct device *pdev,
                                            struct device_attribute *bin_attr,
                                            char *buf)
{
   struct CCIP_PERF_COUNTERS perf_mon;
   struct fme_device* pfme_dev= dev_get_drvdata(pdev);

   if(NULL == pfme_dev )  {
      return (snprintf(buf,PAGE_SIZE,"%d\n",0));
   }

   get_perfmonitor_snapshot(pfme_dev, &perf_mon);

   return (snprintf(buf,PAGE_SIZE,  "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  "
                                    "%lu  ",
                                     (unsigned long int) perf_mon.num_counters.value,
                                     (unsigned long int) perf_mon.version.value ,
                                     (unsigned long int) perf_mon.read_hit.value ,
                                     (unsigned long int) perf_mon.write_hit.value,
                                     (unsigned long int) perf_mon.read_miss.value ,
                                     (unsigned long int) perf_mon.write_miss.value,
                                     (unsigned long int) perf_mon.evictions.value ,
                                     (unsigned long int) perf_mon.pcie0_read.value ,
                                     (unsigned long int) perf_mon.pcie0_write.value,
                                     (unsigned long int) perf_mon.pcie1_read.value ,
                                     (unsigned long int) perf_mon.pcie1_write.value ,
                                     (unsigned long int) perf_mon.upi_read.value ,
                                     (unsigned long int) perf_mon.upi_write.value
                                    ));

 }


DEVICE_ATTR(perfmon_bin_0,0444, perf_monitor_attrib_show_bin,NULL);



///============================================================================
/// Name: create_perfmonitor
/// @brief create Performance counters
///
/// @param[in] ppcidev - pci device pointer
/// @param[in] pfme_dev - fme device pointer
/// @return    error code
///============================================================================
bt32bitInt create_perfmonitor(kosal_pci_dev* ppcidev,
                              struct fme_device* pfme_dev)
{
   int res =0;

   PTRACEIN;
   if( (NULL == ppcidev) && (NULL == pfme_dev))  {

       PERR("Invalid input pointers \n");
       res = -EINVAL;
       goto ERR;
    }

   device_create_file(&(ppcidev->dev),&dev_attr_perfmon_hr_0);
   device_create_file(&(ppcidev->dev),&dev_attr_perfmon_bin_0);

   dev_set_drvdata(&(ppcidev->dev),pfme_dev);

   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;
}

///============================================================================
/// Name: remove_perfmonitor
/// @brief remove Performance counters
///
/// @param[in] ppcidev  pci device pointer.
/// @return    error code
///============================================================================
bt32bitInt remove_perfmonitor(kosal_pci_dev* ppcidev)
{
   int res =0;

   PTRACEIN;

   if( (NULL == ppcidev) ) {
       PERR("Invalid input pointers \n");
       res = -EINVAL;
       goto ERR;
    }

   device_remove_file(&(ppcidev->dev),&dev_attr_perfmon_hr_0);
   device_remove_file(&(ppcidev->dev),&dev_attr_perfmon_bin_0);

   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;
}




























