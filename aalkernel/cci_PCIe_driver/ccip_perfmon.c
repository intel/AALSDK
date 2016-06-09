//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2016, Intel Corporation.
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
//  Copyright(c) 2011-2016, Intel Corporation.
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
/// @file ccip_perfmon.c
/// @brief  CCI-P FME performance counters.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_perfmon.c
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel Corporation
//              Joseph Grecco, Intel Corporation
//
// PURPOSE:   This file contains the implementation of the CCIP FME
//            performance counters
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD
#include "aalsdk/kernel/ccip_defs.h"

#include "ccip_perfmon.h"
#include "ccip_fme.h"


///============================================================================
/// Name: get_perfmonitor_snapshot
/// @brief get snap shot of Performance counters
///
/// @param[in] pfme_dev - fme device pointer
/// @param[in] pPerf - Performance counters pointer.
/// @return    error code
///============================================================================
bt32bitInt get_perfmonitor_snapshot(struct fme_device *pfme_dev,
                                    struct CCIP_PERF_COUNTERS* pPerf)
{
  
   bt32bitInt res =0;

   PTRACEIN;

   if((NULL == pfme_dev)&& (NULL == pPerf))  {
      PERR("Invalid Input pointers  \n");
      res= -EINVAL;
      goto ERR;
   }
   pPerf->version.value = PERF_MONITOR_VERSION;
   pPerf->num_counters.value = PERF_MONITOR_COUNT;

   // freeze
   ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.freeze =0x1;

   Set64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr);

   //Cache_Read_Hit
   update_cache_event_counters(Cache_Read_Hit,pfme_dev,pPerf);

   // Cache_Write_Hit
   update_cache_event_counters(Cache_Write_Hit,pfme_dev,pPerf);

   // Cache_Read_Miss
   update_cache_event_counters(Cache_Read_Miss,pfme_dev,pPerf);

   // Cache_Write_Miss
   update_cache_event_counters(Cache_Write_Miss,pfme_dev,pPerf);

   //Cache_Evictions
   update_cache_event_counters(Cache_Evictions,pfme_dev,pPerf);


   //pcie0 Read
   update_fabric_event_counters(Fabric_PCIe0_Read,pfme_dev,pPerf);

   // pcie0 write
   update_fabric_event_counters(Fabric_PCIe0_Write,pfme_dev,pPerf);

   // pcie1 Read
   update_fabric_event_counters(Fabric_PCIe1_Read,pfme_dev,pPerf);

   // pcie1 write
   update_fabric_event_counters(Fabric_PCIe1_Write,pfme_dev,pPerf);

   //UPI Read
   update_fabric_event_counters(Fabric_UPI_Read,pfme_dev,pPerf);

   //UPI Write
   update_fabric_event_counters(Fabric_UPI_Write,pfme_dev,pPerf);

   //un freeze
   ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.freeze =0x0;
   Set64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr);
 
   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;
}

///============================================================================
/// Name: update_fabric_event_counters
/// @brief get snap shot of fabric Performance counters
///
/// @param[in] event_code - fabric counters code.
/// @param[in] pfme_dev - fme device pointer
/// @param[in] pPerf - Performance counters pointer.
/// @return    0 = success
///============================================================================

bt32bitInt update_fabric_event_counters(bt32bitInt event_code ,
                                       struct fme_device *pfme_dev,
                                       struct CCIP_PERF_COUNTERS* pPerf)
{
   bt32bitInt res       = 0;
   bt32bitInt counter   = 0;

   PTRACEIN;

   ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctl.fabric_evt_code =event_code;
   Set64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctl.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctl.csr);

   while (event_code != ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.event_code)  {

      Get64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.csr);
      counter++;
      if (counter > CACHE_EVENT_COUNTER_MAX_TRY)    {
         PERR("Max Try \n");
         res = 1;
         goto ERR;
      }

   } // end while

   Get64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.csr);
   switch (event_code)
   {

      case  Fabric_PCIe0_Read:
      {
       pPerf->pcie0_read.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;

      case  Fabric_PCIe0_Write:
      {
       pPerf->pcie0_write.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;

      case  Fabric_PCIe1_Read:
      {
       pPerf->pcie1_read.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;


      case  Fabric_PCIe1_Write:
      {
       pPerf->pcie1_write.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;

      case  Fabric_UPI_Read:
      {
       pPerf->upi_read.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;

      case  Fabric_UPI_Write:
      {
       pPerf->upi_write.value= ccip_fme_perf(pfme_dev)->ccip_fpmon_fab_ctr.fabric_counter;
      }
      break;

      default:
      {
       // Error
       PERR("Invalid Cache Event code  \n");
       res= -EINVAL;
      }
      break;

      }

   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;

}

///============================================================================
/// Name: update_cache_event_counters
/// @brief get snap shot of cache Performance counters
///
/// @param[in] event_code - cache counters code.
/// @param[in] pfme_dev - fme device pointer
/// @param[in] pPerf - Performance counters pointer.
/// @return    0 = success
///============================================================================
bt32bitInt update_cache_event_counters(bt32bitInt event_code ,
                                       struct fme_device *pfme_dev,
                                       struct CCIP_PERF_COUNTERS* pPerf)
{
   bt32bitInt res =0;
   bt32bitInt counter =0;
   btUnsigned64bitInt total =0;

   PTRACEIN;

   ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.cache_event =event_code;
   Set64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctl.csr);

   while (event_code != ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.event_code)   {

      Get64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.csr);
      counter++;
      if (counter > CACHE_EVENT_COUNTER_MAX_TRY)   {
         PERR("Max Try \n");
         res = 1;
         goto ERR;
      }

   } // end while

   Get64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.csr);
   Get64CSR(&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_1.csr,&ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_1.csr);

   total= ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_0.cache_counter + ccip_fme_perf(pfme_dev)->ccip_fpmon_ch_ctr_1.cache_counter;

   switch (event_code)
   {

    case  Cache_Read_Hit:
    {
      pPerf->read_hit.value= total;
    }
    break;

    case  Cache_Write_Hit:
    {
      pPerf->write_hit.value = total;
    }
    break;

    case  Cache_Read_Miss:
    {
      pPerf->read_miss.value = total;
    }
    break;

    case  Cache_Write_Miss:
    {
      pPerf->write_miss.value= total;
    }
    break;

    case  Cache_Evictions:
    {
      pPerf->evictions.value= total;
    }
    break;

    default:
    {
       // Error
       PERR("Invalid Cache Event code  \n");
       res= -EINVAL;
    }
    break;

   }

   PTRACEOUT_INT(res);
   return res;
ERR:
   PTRACEOUT_INT(res);
   return  res;

}

///============================================================================
/// Name:    get_perfmon_counters
/// @brief   get  performance counters
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf performance counters pointer
/// @return    error code
///============================================================================
bt32bitInt get_perfmon_counters(struct fme_device* pfme_dev,
                                struct CCIP_PERF_COUNTERS* pPerfCounter)
{
   int res =0;

   PTRACEIN;

   if( (NULL == pfme_dev) && (NULL == pPerfCounter))  {
      PERR("Invalid input pointers \n");
      res =-EINVAL;
      goto ERR;
   }

   memset(pPerfCounter,0,sizeof(struct CCIP_PERF_COUNTERS));

   strncpy(pPerfCounter->num_counters.name ,NUM_COUNTERS,sizeof(NUM_COUNTERS));
   strncpy(pPerfCounter->version.name ,PMONITOR_VERSION,sizeof(PMONITOR_VERSION));

   strncpy(pPerfCounter->read_hit.name ,CACHE_READ_HIT,sizeof(CACHE_READ_HIT));
   strncpy(pPerfCounter->write_hit.name ,CACHE_WRITE_HIT,sizeof(CACHE_WRITE_HIT));

   strncpy(pPerfCounter->read_miss.name ,CACHE_READ_MISS,sizeof(CACHE_READ_MISS));
   strncpy(pPerfCounter->write_miss.name ,CACHE_WRITE_MISS,sizeof(CACHE_WRITE_MISS));

   strncpy(pPerfCounter->evictions.name ,CACHE_EVICTIONS,sizeof(CACHE_EVICTIONS));

   strncpy(pPerfCounter->pcie0_read.name ,FABRIC_PCIE0_READ,sizeof(FABRIC_PCIE0_READ));
   strncpy(pPerfCounter->pcie0_write.name ,FABRIC_PCIE0_WRITE,sizeof(FABRIC_PCIE0_WRITE));

   strncpy(pPerfCounter->pcie1_read.name ,FABRIC_PCIE1_READ,sizeof(FABRIC_PCIE1_READ));
   strncpy(pPerfCounter->pcie1_write.name ,FABRIC_PCIE1_WRITE,sizeof(FABRIC_PCIE1_WRITE));

   strncpy(pPerfCounter->upi_read.name ,FABRIC_UPI_READ,sizeof(FABRIC_UPI_READ));
   strncpy(pPerfCounter->upi_write.name ,FABRIC_UPI_WRITE,sizeof(FABRIC_UPI_WRITE));

   pPerfCounter->num_counters.value=PERF_MONITOR_COUNT;
   pPerfCounter->num_counters.value=PERF_MONITOR_VERSION;

   res= get_perfmonitor_snapshot(pfme_dev,pPerfCounter);

   PTRACEOUT_INT(res);
   return res;

ERR:
   PTRACEOUT_INT(res);
   return  res;

}




























