//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2010-2014, Intel Corporation.
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
//  Copyright(c) 2010-2014, Intel Corporation.
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
//        FILE: splpip.h
//     CREATED: 09-13-10
//      AUTHOR: Jospeh Grecco - Intel Corp.
//
// PURPOSE: SPL PIP public interface definitions for kernel modules
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALSDK_KERNEL_SPLPIP_H__
#define __AALSDK_KERNEL_SPLPIP_H__
#include <aalsdk/kernel/aalbus-device.h>

//Interface Identifier
#define  AALAHM_DEV_SERVICES       (0xfb00000000001000)

//Interface Identifier
#define  AALAHM_API_MAJVERSION     (0x00000001)
#define  AALAHM_API_MINVERSION     (0x00000000)
#define  AALAHM_API_RELEASE        (0x00000000)

////////////////////////////////////////////////////////////////////////////////////
//  structure definitions
////////////////////////////////////////////////////////////////////////////////////


// Abstraction of internal SPL PIP structures
typedef void *splpip_handle_t;


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
struct ahm_device;

typedef btCSRValue (*CSR_GET_T)(splpip_handle_t , btCSROffset );
typedef void       (*CSR_SET_T)(splpip_handle_t , btCSROffset , btCSRValue );


//=============================================================================
// Name: spl_afuconfig_s
// Description: AFU Configuration descriptor
//=============================================================================
struct spl_afu_config_s
{
   // Control CSR read/write aperture
   void       *m_ctrl_csr_rphys;          // Base address if aperture
   void       *m_ctrl_csr_rbase;          // Base address if aperture
   size_t      m_ctrl_csr_readsize;       // Size of aperture
   size_t      m_ctrl_read_csr_size;      // Size of CSR in octets
   size_t      m_ctrl_read_csr_spacing;   // Spacing between start of CSR

   void       *m_ctrl_csr_wphys;          // Base address if aperture
   void       *m_ctrl_csr_wbase;          // Base address if aperture
   size_t      m_ctrl_csr_writesize;      // Size of aperture
   size_t      m_ctrl_write_csr_size;     // Size of CSR in octets
   size_t      m_ctrl_write_csr_spacing;  // Spacing between start of CSR

   // Compute CSR read/write aperture
   void       *m_csr_rphys;               // Base address if aperture
   void       *m_csr_rbase;               // Base address if aperture
   size_t      m_csr_readsize;            // Size of aperture
   size_t      m_read_csr_size;           // Size of CSR in octets
   size_t      m_read_csr_spacing;        // Spacing between start of CSR

   void       *m_csr_wphys;               // Base address if aperture
   void       *m_csr_wbase;               // Base address if aperture
   size_t      m_csr_writesize;           // Size of aperture
   size_t      m_write_csr_size;          // Size of CSR in octets
   size_t      m_write_csr_spacing;       // Spacing between start of CSR
};


// Macros to access Control CSR space
#define splafu_cfg_ctrl_csr_rphys(dev)          ((dev)->m_ctrl_csr_rphys)
#define splafu_cfg_ctrl_csr_rbase(dev)          ((dev)->m_ctrl_csr_rbase)
#define splafu_cfg_ctrl_csr_rsize(dev)          ((dev)->m_ctrl_csr_readsize)
#define splafu_cfg_ctrl_read_csr_size(dev)      ((dev)->m_ctrl_read_csr_size)
#define splafu_cfg_ctrl_read_csr_spacing(dev)   ((dev)->m_ctrl_read_csr_spacing)

#define splafu_cfg_ctrl_csr_wphys(dev)          ((dev)->m_ctrl_csr_wphys)
#define splafu_cfg_ctrl_csr_wbase(dev)          ((dev)->m_ctrl_csr_wbase)
#define splafu_cfg_ctrl_csr_wsize(dev)          ((dev)->m_ctrl_csr_writesize)
#define splafu_cfg_ctrl_write_csr_size(dev)     ((dev)->m_ctrl_write_csr_size)
#define splafu_cfg_ctrl_write_csr_spacing(dev)  ((dev)->m_ctrl_write_csr_spacing)

// Macros to access Compute CSR space
#define splafu_cfg_csr_rphys(dev)               ((dev)->m_csr_rphys)
#define splafu_cfg_csr_rbase(dev)               ((dev)->m_csr_rbase)
#define splafu_cfg_csr_rsize(dev)               ((dev)->m_csr_readsize)
#define splafu_cfg_read_csr_size(dev)           ((dev)->m_read_csr_size)
#define splafu_cfg_read_csr_spacing(dev)        ((dev)->m_read_csr_spacing)

#define splafu_cfg_csr_wphys(dev)               ((dev)->m_csr_wphys)
#define splafu_cfg_csr_wbase(dev)               ((dev)->m_csr_wbase)
#define splafu_cfg_csr_wsize(dev)               ((dev)->m_csr_writesize)
#define splafu_cfg_write_csr_size(dev)          ((dev)->m_write_csr_size)
#define splafu_cfg_write_csr_spacing(dev)       ((dev)->m_write_csr_spacing)

// Callback invoked each time an new AFU is created
typedef int (*SPL_CH_INIT)(void *,struct spl_afu_config_s *, unsigned index);

//=============================================================================
// Name: spl_boardParms_s
// Description: Initialization parameters for SPL based devices
//=============================================================================
struct spl_boardParms_s
{
   // Structure description
   uint64_t             version;
   size_t               size;

   struct aal_device_id AHM_id;

   // device specific CSR accessor/mutators (index versions)
   CSR_GET_T            fcsrget_idx;
   CSR_SET_T            fcsrset_idx;

   // device specific CSR accessor/mutators (offset versions deprecated)
   CSR_GET_T            fcsrget;
   CSR_SET_T            fcsrset;

   // Aperture
   void                *basemem;
   void                *rbase;
   size_t               readsize;
   void                *wbase;
   size_t               writesize;

   // Callback issued when AFU is initialized
   SPL_CH_INIT          pafu_init;

   // Device name
   char                 name[];
};

#define SPL_BD_PARM_DATE      0x11142010L
#define SPL_BD_PARM_VER(s)   ((s)->version = (SPL_BD_PARM_DATE<< 32) + offsetof(struct spl_boardParms_s,name) )

//=============================================================================
// Name: create_splBoardparms_s
// Description: Create a parameters structure
//   Inputs: AHM_id  - Board's device ID
//           name    - Name of the object
//=============================================================================
inline static struct spl_boardParms_s *
                           create_splBoardparms( struct aal_device_id *pAHM_id,
                                                 void                 *basemem,
                                                 void                 *readbase,
                                                 size_t                readsize,
                                                 void                 *writebase,
                                                 size_t                writesize,
                                                 CSR_GET_T             fcsrget_idx,
                                                 CSR_SET_T             fcsrset_idx,
                                                 CSR_GET_T             fcsrget,
                                                 CSR_SET_T             fcsrset,
                                                 SPL_CH_INIT           pinitfcn,
                                                 char                 *name)
{
   // Size of structure
   size_t size=sizeof(struct spl_boardParms_s) + strlen(name)+1;

   // Allocate the structure
   struct spl_boardParms_s *pparms  = kmalloc(size,
                                              GFP_KERNEL );
   if(unlikely(pparms == NULL)){
      return NULL;
   }

   // Intialize and version stamp the structure
   memset(pparms,0, size);
   SPL_BD_PARM_VER(pparms);
   pparms->size= size;


   // Copy the ID
   pparms->AHM_id = *pAHM_id;

   // CSR accesors (index versions)
   pparms->fcsrget_idx = fcsrget_idx;
   pparms->fcsrset_idx = fcsrset_idx;

   // CSR accesors
   pparms->fcsrget = fcsrget;
   pparms->fcsrset = fcsrset;

   // Aperture windows
   pparms->basemem = basemem;

   pparms->rbase = readbase;
   pparms->readsize=readsize;

   pparms->wbase = writebase;
   pparms->writesize=writesize;

   // Called when an AFU is initialized
   pparms->pafu_init=pinitfcn;

   // Limit the copy. to only the string size
   strncpy( pparms->name,
            name,
            size-offsetof(struct spl_boardParms_s,name));
   return pparms;
};

//=============================================================================
// Name: delete_splBoardparms
// Description: Create a parameters structure
//   Inputs: Name of the object
//=============================================================================
inline static void delete_splBoardparms(struct spl_boardParms_s *pparms)
{
   kfree(pparms);
}


//=============================================================================
// Name: spl_pip_dev_services
// Description: Interface to the SPL PIP device services.  These are a set
//              of services used by SPL based devices to create, activate,
//              deactivate and destroy AFU devices
//=============================================================================
struct spl_pip_dev_services {

   splpip_handle_t (*init_board)(void * context, struct spl_boardParms_s *pparms);

   int (*destroy_board)(splpip_handle_t);

   struct aal_device *(*setup_channel)(splpip_handle_t,
                                       int,
                                       u_int64_t pipGUID );
   void (*free_channel)(splpip_handle_t,
                        struct aal_device *AFUdevp);

   int (*activate_channel)( struct aal_device *pdev);
   int (*deactivate_channel)( struct aal_device *pdev);
   void *(*get_context)(splpip_handle_t hndl);
};

#endif // __AALSDK_KERNEL_SPLPIP_H__
