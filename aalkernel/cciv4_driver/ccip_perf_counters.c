//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
//  Copyright(c) 2011-2015, Intel Corporation.
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
/// @file ccip_def.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_fme_mmio.h
//     CREATED: Sept 24, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the definations of the CCIP FME
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///


#include "ccip_perf_counters.h"

#include "aalsdk/kernel/kosal.h"
#include "ccip_def.h"

#define MODULE_FLAGS CCIV4_DBG_MOD

// periodic timer
#define PERF_COUNTER_TIMER  60000

#define CACHE_EVENT_COUNTER_MAX_TRY 30

#define CACHE_EVENT_COUNTER_TIMEOUT 1000

struct FPMON_CACHE_CTL {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt port_counter :60;
         btUnsigned64bitInt event_code :4;
      }; //end struct
   }; // end union

}; //end struct FPMON_CACHE_CONTROL



// global performance Device feature list offset
#define byte_offset_FPMON_DFH_HDR   0x3000

#define PERF_COUNTERS_HR   "hr_0"
#define PERF_COUNTERS_BIN  "bin_0"

#define AALPERF_DATATYPE         btUnsigned64bitInt
#define AALPERF_VERSION          "Version         "  // Start with Version 0
#define AALPERF_PORT0_READ_HIT   "Port0_Read_Hit  "
#define AALPERF_PORT0_WRITE_HIT  "Port0_Write_Hit "
#define AALPERF_PORT0_READ_MISS  "Port0_Read_Miss "
#define AALPERF_PORT0_WRITE_MISS "Port0_Write_Miss"
#define AALPERF_PORT0_EVICTIONS  "Port0_Evictions "
#define AALPERF_PORT1_READ_HIT   "Port1_Read_Hit  "
#define AALPERF_PORT1_WRITE_HIT  "Port1_Write_Hit "
#define AALPERF_PORT1_READ_MISS  "Port1_Read_Miss "
#define AALPERF_PORT1_WRITE_MISS "Port1_Write_Miss"
#define AALPERF_PORT1_EVICTIONS  "Port1_Evictions "

#define AALPERF_NUM_COUNTERS     "Num of Counters "

// Performance counter  kernel object
struct kobject *perf_counterobj;

// Performance counter  timer
struct timer_list perf_counter_timer;

// performance counters structure
struct ccip_perf_counters {
   struct attribute attr;
   btUnsigned64bitInt num_counters;
   btUnsigned64bitInt version;
   btUnsigned64bitInt Port0_Read_Hit;
   btUnsigned64bitInt Port0_Write_Hit ;
   btUnsigned64bitInt Port0_Read_Miss ;
   btUnsigned64bitInt Port0_Write_Miss ;
   btUnsigned64bitInt Port0_Evictions ;
   btUnsigned64bitInt Port1_Read_Hit ;
   btUnsigned64bitInt Port1_Write_Hit ;
   btUnsigned64bitInt Port1_Read_Miss ;
   btUnsigned64bitInt Port1_Write_Miss ;
   btUnsigned64bitInt Port1_Evictions ;

};
// performance counters in Human readable version
static struct ccip_perf_counters ccip_perf_counters_hr = {
   .attr.name =PERF_COUNTERS_HR,
   .attr.mode = 0644,
   .num_counters =10,
   .version =0,
   .Port0_Read_Hit =0,
   .Port0_Write_Hit =0,
   .Port0_Read_Miss =0,
   .Port0_Write_Miss =0,
   .Port0_Evictions =0,
   .Port1_Read_Hit =0,
   .Port1_Write_Hit =0,
   .Port1_Read_Miss =0,
   .Port1_Write_Miss =0,
   .Port1_Evictions =0


};
// performance counters in Binary  version
static struct ccip_perf_counters ccip_perf_counters_bin = {
   .attr.name =PERF_COUNTERS_BIN,
   .attr.mode = 0644,
   .num_counters =10,
   .version =0,
   .Port0_Read_Hit =0,
   .Port0_Write_Hit =0,
   .Port0_Read_Miss =0,
   .Port0_Write_Miss =0,
   .Port0_Evictions =0,
   .Port1_Read_Hit =0,
   .Port1_Write_Hit =0,
   .Port1_Read_Miss =0,
   .Port1_Write_Miss =0,
   .Port1_Evictions =0


};
// performance counters attributes
static  struct attribute *ccip_perf_counter_attr[] = {
   &ccip_perf_counters_hr.attr,
   &ccip_perf_counters_bin.attr,
   NULL
};

// performance counters group
static struct attribute_group  ccip_perf_counter_attr_group = {
   .attrs =ccip_perf_counter_attr ,
};



// performance counters store
static ssize_t ccip_perf_counters_store(struct  kobject *kobj,
                                        struct attribute *attr,
                                        const char *buf,
                                        size_t size)
{
   return size;
}

// performance counters show
static ssize_t ccip_perf_counters_show(struct  kobject *kobj,
                                       struct attribute *attr,
                                       char *buf)
{
   struct ccip_perf_counters *pperfcounter  = container_of(attr,struct ccip_perf_counters,attr);

   if((NULL != pperfcounter) &&
     ( strncmp(attr->name ,PERF_COUNTERS_BIN,strlen(PERF_COUNTERS_HR)) == 0))  {

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
                                       "%lu  ",
                                        (unsigned long int) pperfcounter->num_counters,
                                        (unsigned long int) pperfcounter->version ,
                                        (unsigned long int) pperfcounter->Port0_Read_Hit ,
                                        (unsigned long int) pperfcounter->Port0_Write_Hit ,
                                        (unsigned long int) pperfcounter->Port0_Read_Miss ,
                                        (unsigned long int) pperfcounter->Port0_Write_Miss,
                                        (unsigned long int) pperfcounter->Port0_Evictions ,
                                        (unsigned long int) pperfcounter->Port1_Read_Hit ,
                                        (unsigned long int) pperfcounter->Port1_Write_Hit ,
                                        (unsigned long int) pperfcounter->Port1_Read_Miss ,
                                        (unsigned long int) pperfcounter->Port1_Write_Miss ,
                                        (unsigned long int) pperfcounter->Port1_Evictions
                                       ));

   }


   if((NULL != pperfcounter) &&
     (strncmp(attr->name ,PERF_COUNTERS_HR,strlen(PERF_COUNTERS_HR))== 0)) {

      return (snprintf(buf,PAGE_SIZE, "%s : %lu \n"
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
                                      AALPERF_NUM_COUNTERS,       (unsigned long int) pperfcounter->num_counters ,
                                      AALPERF_VERSION,            (unsigned long int) pperfcounter->version ,
                                      AALPERF_PORT0_READ_HIT,     (unsigned long int) pperfcounter->Port0_Read_Hit ,
                                      AALPERF_PORT0_WRITE_HIT,    (unsigned long int) pperfcounter->Port0_Write_Hit ,
                                      AALPERF_PORT0_READ_MISS,    (unsigned long int) pperfcounter->Port0_Read_Miss ,
                                      AALPERF_PORT0_WRITE_MISS,   (unsigned long int) pperfcounter->Port0_Write_Miss,
                                      AALPERF_PORT0_EVICTIONS,    (unsigned long int) pperfcounter->Port0_Evictions ,
                                      AALPERF_PORT1_READ_HIT,     (unsigned long int) pperfcounter->Port1_Read_Hit ,
                                      AALPERF_PORT1_WRITE_HIT,    (unsigned long int) pperfcounter->Port1_Write_Hit ,
                                      AALPERF_PORT1_READ_MISS,    (unsigned long int) pperfcounter->Port1_Read_Miss ,
                                      AALPERF_PORT1_WRITE_MISS,   (unsigned long int) pperfcounter->Port1_Write_Miss ,
                                      AALPERF_PORT1_EVICTIONS,    (unsigned long int) pperfcounter->Port1_Evictions
                                      ));
   }

   return 0;
}

static struct sysfs_ops ccip_perfcounterops = {

   .show = ccip_perf_counters_show,
   .store = ccip_perf_counters_store,
};

static struct kobj_type perf_counters_type = {

   .sysfs_ops = &ccip_perfcounterops,
   .default_attrs = ccip_perf_counter_attr,
};

/// @brief   Timer callback .
///
/// @param[in] data passed data to timer callback .
/// @return   void
void ccip_perf_counters_timer_callback(unsigned long data)
{

   if(perf_counterobj)
   {
      //Get snapshot of counters
      //ccip_perf_counters_snapshot();
      sysfs_update_group(perf_counterobj, &ccip_perf_counter_attr_group);
      mod_timer(&perf_counter_timer, jiffies + msecs_to_jiffies(PERF_COUNTER_TIMER));
   }

}



/// @brief   starts performance counters.
///
/// @param[in] kobjdevice device kernel object.
/// @return    error code
bt32bitInt ccip_start_perf_counters(struct kobject kobjdevice)
{
   bt32bitInt res =0;

   PINFO("Enter \n");

   perf_counterobj= kzalloc(sizeof(*perf_counterobj),GFP_KERNEL);

   if(NULL == perf_counterobj)	{
      res = -ENOMEM;
      goto ERR;
   }

   kobject_init(perf_counterobj,&perf_counters_type);

  // if(kobject_add(perf_counterobj,&kobjdevice,"%s","ccip_perf_counters"))
   if(kobject_add(perf_counterobj,NULL,"%s","cci_perf_counters"))
   {
      PERR("failed to add performance counter object \n");

      kobject_put(perf_counterobj);
      perf_counterobj= NULL;
      res = -ENODEV;
      goto ERR;
   }
   else
   {
      sysfs_create_group(perf_counterobj,&ccip_perf_counter_attr_group);
      sysfs_update_group(perf_counterobj, &ccip_perf_counter_attr_group);
   }

   // Start 1 minute timer
   setup_timer(&perf_counter_timer,ccip_perf_counters_timer_callback,0);
   mod_timer(&perf_counter_timer, jiffies + msecs_to_jiffies(PERF_COUNTER_TIMER));

   return res;

ERR:

   if(NULL != perf_counterobj)  {
      kfree(perf_counterobj);
     }

   PINFO("End \n");
   return res;
}

/// @brief   stops performance counters.
///
/// @param[in] void .
/// @return    void
void ccip_stop_perf_counters_free(void)
{
   PINFO("Enter \n");
   if(perf_counterobj)
   {
      kobject_put(perf_counterobj);
      kfree(perf_counterobj);
   }

   del_timer(&perf_counter_timer);
   PINFO("End \n");
}



// @brief   update cache event  performance counters.
///
/// @param[in] event_code cache event code.
/// @param[in] pkvp_fpmon_mmio Global performance mmio pointer.
/// @param[in] pperf_counters_hr pointer to performance counter  human readable format
/// @param[in] pperf_counters_bin pointer to performance counter binary format
/// @return    error code
bt32bitInt update_cache_event_counters(bt32bitInt event_code ,
                                       btVirtAddr pkvp_fpmon_mmio,
                                       struct ccip_perf_counters *pperf_counters_hr ,
                                       struct ccip_perf_counters *pperf_counters_bin)
{
   bt32bitInt res =0;
   bt32bitInt counter =0;
   /*
   btTime delay =10;
   btTime totaldelay =0;
    */
   struct FPMON_CACHE_CTL fpmon_cache_ctl_port0;
   struct FPMON_CACHE_CTL fpmon_cache_ctl_port1;
   struct CCIP_FPMON_CH_CTL ccip_fpmon_ch_ctl;



   ccip_fpmon_ch_ctl.csr = read_ccip_csr64(pkvp_fpmon_mmio,byte_offset_FPMON_CACHE_CTL);
   ccip_fpmon_ch_ctl.cache_event =event_code;

   write_ccip_csr64(pkvp_fpmon_mmio, byte_offset_FPMON_CACHE_CTL,ccip_fpmon_ch_ctl.csr);

   while(event_code != fpmon_cache_ctl_port0.event_code )
   {
      fpmon_cache_ctl_port0.csr = read_ccip_csr64(pkvp_fpmon_mmio,byte_offset_FPMON_CACHE_CTR_0);
      counter++;
      if( counter > CACHE_EVENT_COUNTER_MAX_TRY)
         break;

      /*
      // Sleep
      kosal_udelay(delay);

      // total dealy
      totaldelay = totaldelay + delay;

      // if total delay is more then 1 millisecond , return erroor
      if(totaldelay > CACHE_EVENT_COUNTER_TIMEOUT)   {
         res = -ETIME;
         break ;
      }
      */


   }

   counter =0;
   while(event_code != fpmon_cache_ctl_port1.event_code )
   {
      fpmon_cache_ctl_port1.csr = read_ccip_csr64(pkvp_fpmon_mmio,byte_offset_FPMON_CACHE_CTR_1);

      counter++;
      if( counter > CACHE_EVENT_COUNTER_MAX_TRY)
            break;

      /*
      // Sleep
      kosal_udelay(delay);

      // total dealy
      totaldelay = totaldelay + delay;

      // if total delay is more then 1 millisecond , return erroor
      if(totaldelay > CACHE_EVENT_COUNTER_TIMEOUT)   {
         res = -ETIME;
         break ;
      }
      */
   }


   switch (event_code)
   {

    case  Cache_Read_Hit:
    {
       pperf_counters_hr->Port0_Read_Hit = fpmon_cache_ctl_port0.port_counter;
       pperf_counters_bin->Port0_Read_Hit = fpmon_cache_ctl_port0.port_counter;

       pperf_counters_hr->Port1_Read_Hit = fpmon_cache_ctl_port1.port_counter;
       pperf_counters_bin->Port1_Read_Hit = fpmon_cache_ctl_port1.port_counter;
    }
    break;

    case  Cache_Write_Hit:
    {
       pperf_counters_hr->Port0_Write_Hit = fpmon_cache_ctl_port0.port_counter;
       pperf_counters_bin->Port0_Write_Hit = fpmon_cache_ctl_port0.port_counter;

       pperf_counters_hr->Port1_Write_Hit = fpmon_cache_ctl_port1.port_counter;
       pperf_counters_bin->Port1_Write_Hit = fpmon_cache_ctl_port1.port_counter;
    }
    break;

    case  Cache_Read_Miss:
    {
       pperf_counters_hr->Port0_Read_Miss = fpmon_cache_ctl_port0.port_counter;
       pperf_counters_bin->Port0_Read_Miss = fpmon_cache_ctl_port0.port_counter;

       pperf_counters_hr->Port1_Read_Miss = fpmon_cache_ctl_port1.port_counter;
       pperf_counters_bin->Port1_Read_Miss = fpmon_cache_ctl_port1.port_counter;
    }
    break;


    case  Cache_Write_Miss:
    {
       pperf_counters_hr->Port0_Write_Miss = fpmon_cache_ctl_port0.port_counter;
       pperf_counters_bin->Port0_Write_Miss = fpmon_cache_ctl_port0.port_counter;


       pperf_counters_hr->Port1_Write_Miss = fpmon_cache_ctl_port1.port_counter;
       pperf_counters_bin->Port1_Write_Miss = fpmon_cache_ctl_port1.port_counter;
    }
    break;

    case  Cache_Evictions:
    {
       pperf_counters_hr->Port0_Evictions = fpmon_cache_ctl_port0.port_counter;
       pperf_counters_bin->Port0_Evictions = fpmon_cache_ctl_port0.port_counter;

       pperf_counters_hr->Port1_Evictions = fpmon_cache_ctl_port1.port_counter;
       pperf_counters_bin->Port1_Evictions = fpmon_cache_ctl_port1.port_counter;
    }
    break;

    default:
    {
       // Error
       PERR("invalid Cache Event code  \n");
    }
    break;

   }

   return res;

}


// @brief   get snapshot of performance counters.
///
/// @param[in] void.
/// @return    error code
bt32bitInt ccip_perf_counters_snapshot(void)
{

   bt32bitInt res =0;
  struct fme_device *pfme_dev = NULL;
   btVirtAddr pkvp_fme_mmio = NULL;

   struct CCIP_FPMON_CH_CTL ccip_fpmon_ch_ctl;
   struct ccip_perf_counters *pperf_counters_hr = NULL ;
   struct ccip_perf_counters *pperf_counters_bin = NULL;

   PINFO("Enter \n");

   // FIX offset
   pkvp_fme_mmio = pkvp_fme_mmio + byte_offset_FPMON_DFH_HDR;
   ccip_fpmon_ch_ctl.csr = read_ccip_csr64(pkvp_fme_mmio,byte_offset_FPMON_CACHE_CTL);
   ccip_fpmon_ch_ctl.freeze =0x1;
   write_ccip_csr64(pkvp_fme_mmio, byte_offset_FPMON_CACHE_CTL,ccip_fpmon_ch_ctl.csr);

   //Cache_Read_Hit
   update_cache_event_counters(Cache_Read_Hit,pkvp_fme_mmio,pperf_counters_hr,pperf_counters_bin);

   // Cache_Write_Hit
    update_cache_event_counters(Cache_Write_Hit,pkvp_fme_mmio,pperf_counters_hr,pperf_counters_bin);

   // Cache_Read_Miss
   update_cache_event_counters(Cache_Read_Miss,pkvp_fme_mmio,pperf_counters_hr,pperf_counters_bin);

   // Cache_Write_Miss
   update_cache_event_counters(Cache_Write_Miss,pkvp_fme_mmio,pperf_counters_hr,pperf_counters_bin);

   //Cache_Evictions
   update_cache_event_counters(Cache_Evictions,pkvp_fme_mmio,pperf_counters_hr,pperf_counters_bin);

   //reset freeze
   ccip_fme_fpmon(pfme_dev)->ccip_fpmon_ch_ctl.freeze =0x0;
   write_ccip_csr64(pkvp_fme_mmio, byte_offset_FPMON_CACHE_CTL,ccip_fpmon_ch_ctl.csr);

   PINFO("End \n");

   return res;
}
