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
//        FILE: ahmpip.h
//     CREATED: 03/26/2008
//      AUTHOR: Alvin Chen,    Intel Corporation
//              Henry Mitchel, Intel Corporation
//              Joseph Grecco, Intel Corporation
//
// PURPOSE: Common interfaces for PIP driver.
//
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 03/26/08       AC       Initial version created
// 11/11/2008     JG       Added legal header
// 11/25/2008     HM       Large merge
// 12/10/2008     JG       Added support for bound wsmgr
// 12/16/2008     JG       Began support for abort and shutdown
//                            Added Support for WSID object
//                            Major interface changes.
// 12/23/2008     JG       Fixed mult session bug where owner session was in
//                            global PIP
//                            Removed some legacy structures
// 12/27/2008     JG       Support for TransactionID/tskID maps
// 01/04/2009     HM       Updated Copyright
// 02/02/2009     JG       Fixed bug in ahm_alloc_aperture where the
//                            memset_io caused problems with 5000x
// 02/26/2009     JG       Began dynamic config implementation
// 07/03/2009     HM       Fixed bug in ahm_free_aperture caused by earlier
//                            conversion from oversized CSR buffer to ensure
//                            alignment to perfect sizing using get_free_pages
// 01/22/2010     JG       Added assert_splch_reset() to properly follow
//                            reset protocol
// 01/27/2010     AC       Remove clearing WR region for assert_splch_reset
// 02/11/2010     JG       Support for kernel 2.6.31
// 09/27/2010     JG       Separated out SPL interface specifics to improve
//                             share-ability and abstraction
// 04/19/2012     TSW      Deprecate __func__ in favor of platform-agnostic
//                          __AAL_FUNC__.
//****************************************************************************
#ifndef __AALSDK_KERNEL_AHMPIP_H__
#define __AALSDK_KERNEL_AHMPIP_H__
#include <aalsdk/kernel/kosal.h>

#include <aalsdk/kernel/ahmpipdefs.h>
#include <aalsdk/kernel/aalbus.h>
#include <aalsdk/kernel/aalbus-device.h>
#include <aalsdk/kernel/splpip.h>
#include <aalsdk/kernel/fappip.h>


#if (1 == ENABLE_DEBUG)
#define GEN_DPRINTF(m...) do { \
    printk (KERN_DEBUG ":%s ", __AAL_FUNC__); \
    printk (m); \
} while (0)
#else
#define GEN_DPRINTF( m...) do {} while (0)
#endif // ENABLE_DEBUG


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//                             Macros
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////



struct ahm_device;
//=============================================================================
// Name: ahm_channel
// Description: Represents a physical AHM channel
//=============================================================================
struct ahm_channel {
   struct device           m_nativedev;      // Native device
   struct ahm_device      *m_pdev;           // Pointer to the board device
   struct aal_device      *m_pafudev;        // Pointer to the AAL device

   int                     m_index;          // the channel index in ahm_device array
   kosal_semaphore         m_sem;            // channel semphore
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
   struct delayed_work     task_handler;
#else
   struct work_struct      task_handler;
#endif

#ifdef _USE_INTERRUPTS
   int               m_intr_index;     /* interrupt table index */
#endif


   // hw task queues
   uint16_t                m_next_tgp;      // Next available task group index

   // SPL AFU config descriptor contains CSR space descriptor
   struct spl_afu_config_s m_config;

   // input descriptor buffer
   kosal_list_head         m_iready_list;
   kosal_semaphore         m_iready_sem;
   kosal_list_head         m_irunning_list;
   kosal_semaphore         m_irunning_sem;
   kosal_semaphore         m_isem;
   void                   *m_idesc_base;
   void                   *m_idescs;
   void                   *m_iheader;     // pointer to the first used descriptor
   void                   *m_itail;       // pointer to the first empty descriptor
   void                   *m_pgtblcache;  // pointer to the previous pagetable
   size_t                  m_pgtblcache_size;

   // output descriptor buffer
   kosal_list_head         m_oready_list;
   kosal_semaphore         m_oready_sem;
   kosal_list_head         m_orunning_list;
   kosal_semaphore         m_orunning_sem;
   kosal_semaphore         m_osem;
   void                   *m_odesc_base;
   void                   *m_odescs;
   void                   *m_oheader;     /* pointer to the first used descriptor */
   void                   *m_otail;       /* pointer to the first empty descriptor */

   ///////////////////////////////////////////////////////////////////////////////////
   int (*reset)( struct ahm_channel *ch);
   int (*flush_desc_queue)( struct ahm_channel *ch);
};

#define ahmch_to_aaldevp(dev)   ((dev)->m_pafudev)


#define ahmchp_spl_configp(dev)                 (&(dev)->m_config)

// Macros to access Control CSR space
#define ahmchp_ctrl_csr_rbase(dev)              ((dev)->m_config.m_ctrl_csr_rbase)
#define ahmchp_ctrl_csr_rsize(dev)              ((dev)->m_config.m_ctrl_csr_readsize)
#define ahmchp_ctrl_read_csr_size(dev)          ((dev)->m_config.m_ctrl_read_csr_size)
#define ahmchp_ctrl_read_csr_spacing(dev)       ((dev)->m_config.m_ctrl_read_csr_spacing)

#define ahmchp_ctrl_csr_wbase(dev)              ((dev)->m_config.m_ctrl_csr_wbase)
#define ahmchp_ctrl_csr_wsize(dev)              ((dev)->m_config.m_ctrl_csr_writesize)
#define ahmchp_ctrl_write_csr_size(dev)         ((dev)->m_config.m_ctrl_write_csr_size)
#define ahmchp_ctrl_write_csr_spacing(dev)      ((dev)->m_config.m_ctrl_write_csr_spacing)

// Macros to access Compute CSR space
#define ahmchp_csr_rbase(dev)                   ((dev)->m_config.m_csr_rbase)
#define ahmchp_csr_rsize(dev)                   ((dev)->m_config.m_csr_readsize)
#define ahmchp_read_csr_size(dev)               ((dev)->m_config.m_read_csr_size)
#define ahmchp_read_csr_spacing(dev)            ((dev)->m_config.m_read_csr_spacing)

#define ahmchp_csr_wbase(dev)                   ((dev)->m_config.m_csr_wbase)
#define ahmchp_csr_wsize(dev)                   ((dev)->m_config.m_csr_writesize)
#define ahmchp_write_csr_size(dev)              ((dev)->m_config.m_write_csr_size)
#define ahmchp_write_csr_spacing(dev)           ((dev)->m_config.m_write_csr_spacing)

//=============================================================================
// Name: ahm_device
// Description: AHM specific device object definition
//=============================================================================
struct ahm_device
{


   struct aal_device_id     m_AHM_id;        // ID for the board

   // native device - used to tie into Linux DDK
   struct device            m_nativedev;     // Native linux device

   // Context used by owner of AHM. Not used by SPL PIP
   void                    *m_context;       // Context used by owner

   // Aperture
   void __iomem            *m_base;          // aperture base
   void __iomem            *m_rbase;         // read CSRs
   size_t                   m_rsize;         // size of window
   void __iomem            *m_wbase;         // write CSRs, must be 2M aligned
   size_t                   m_wsize;         // size of window

   //
   // Work Queue used to service transactions on the board
   //
   struct workqueue_struct *m_wq;

   ///////////////////////////////////////////////////////////////////////


   kosal_semaphore          dev_sem;

   // implementation specific part, subject to change in the future
   struct ahm_channel       m_chs[AHMPIP_AHM_CHANNEL_NUMBER];
   unsigned int             m_numchannels;

   // Callback used for implementation specific initialization
   SPL_CH_INIT              m_pafu_init;

   // irq
   int                      m_irq;



   // CSR mutators that take index numbers
   CSR_GET_T            get_csr_idx;
   CSR_SET_T            set_csr_idx;

   // CSR mutators that take offsets (deprecated)
   CSR_GET_T            get_csr;
   CSR_SET_T            set_csr;
   char                 m_name[];
};

#define ahmdev_to_ahmchp(d,c)      (&(d)->m_chs[c])


////////////////////////////////////////////////////////////////////////////////////
//  inline functions define
////////////////////////////////////////////////////////////////////////////////////
//=============================================================================
// Name: ahm_set_page_reserved
// Description: Mark range of pages to reserved
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static void inline
ahm_set_page_reserved (struct page *base, size_t size)
{
   struct page *page;
   unsigned long offset;
   void *vbase;

   vbase = page_address (base);

   for (offset = 0; offset < size; offset += PAGE_SIZE) {
      page = virt_to_page (vbase + offset);
      SetPageReserved (page);
   }
}

//=============================================================================
// Name: ahm_clr_page_reserved
// Description: Clear reserved of pages
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static void inline
ahm_clr_page_reserved (struct page *base, size_t size)
{
   struct page *page;
   unsigned long offset;

   for (offset = 0; offset < size; offset += PAGE_SIZE) {
      page = virt_to_page (page_address (base) + offset);
      ClearPageReserved (page);
   }
}

// CSR base must be 2M aligned
#define CSR_BASE(base)  ( ( (unsigned long)base & 0x1FFFFF ) ?  (void __iomem *)( ( (unsigned long)base & ~0x1FFFFF) + AHMPIP_APERTURE_SIZE ) : base )
//=============================================================================
// Name: ahm_alloc_aperture
// Description: Allocate the aperature window
// Interface: private
// Inputs: none
// Outputs: none.
// Comments: CSR base must be 2M (AHMPIP_APERTURE_SIZE) aligned
//=============================================================================
static inline void __iomem *
ahm_alloc_aperture ( void )
{
   void __iomem *base = NULL;
   char *addr = NULL;

   int i=0;

   unsigned int order = 0;
#if defined(_FLUSH_APERTURE)
   unsigned long m;
#endif

   GEN_DPRINTF ( "Using dynamic aperature allocation & mapping\n");
   order = get_order (AHMPIP_APERTURE_SIZE );
   base = (void __iomem *) __get_free_pages (GFP_KERNEL, order);
   if( unlikely( base == NULL )) {
     GEN_DPRINTF ( "Could not allocate aperature %p\n",base);
     return NULL;
   }

   //////////////////////////////////////////////////////////////////////
   // check if the aperture is 2M aligned.
   if( unlikely( (unsigned long)base & 0x1FFFFF ) ) {
      GEN_DPRINTF ( "Error: CSR base must be 2M aligned: 0x%p\n",base);
      free_pages ((unsigned long)base, order);
      return NULL;
   }

   /* set page as reserved to enable mapping CSR into user space */
   ahm_set_page_reserved (virt_to_page (base), 1 << (order + PAGE_SHIFT));

#if defined(_FLUSH_APERTURE)
   /* clflush the whole aperture */
   for (m = (unsigned long) base;
      m < (unsigned long)(base + AHMPIP_APERTURE_SIZE);
      m += SMP_CACHE_BYTES)
   __asm__ ("clflush (%0)" ::"r" (m));
   __asm__ ("mfence" ::);
#endif

   /* Do initializing, otherwise the same memory may be allocated, and the
    previous data still available. This will cause issue when re-load the
    driver module.
   */
   if(base != NULL) {
      GEN_DPRINTF ( "doing memset %p\n",base);
      //memset_io( base, 0, AHMPIP_APERTURE_SIZE * 2 );
      for(addr = base, i = 0; i < AHMPIP_APERTURE_SIZE; i++){
         *(addr++) = 0;
      }
   }

   return base;
}
//=============================================================================
// Name: ahm_free_aperture
// Description: Free the aperature window
// Interface: private
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
static inline void
ahm_free_aperture( void __iomem *base )
{
   unsigned int order;

   order = get_order (AHMPIP_APERTURE_SIZE);
   /* clear page's reserved flag */
   ahm_clr_page_reserved (virt_to_page (base), 1 << (order + PAGE_SHIFT));
   free_pages ((unsigned long)base, order);
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/* ========== csr access ========== */
#define _ahm_csr_get(dev, off)      ( (dev)->get_csr((dev), (off)) )
#define _ahm_csr_set(dev, off, val) ( (dev)->set_csr((dev), (off), (val) ) )

#ifdef _32BIT_CSR
#define ahm_csr_get ahm_csr_get32
#define ahm_csr_set ahm_csr_set32

#else

// Note index versions will be replacing offset versions
#define ahm_csr_get_idx ahm_csr_get64_idx
#define ahm_csr_get ahm_csr_get64
#define ahm_csr_set_idx ahm_csr_set64_idx
#define ahm_csr_set ahm_csr_set64

#endif


//=============================================================================
// Name: assert_splch_reset
// Description: Get  32 Bit CSR value
// Interface: private
// Inputs: ch - channel
// Outputs: value.
// Comments:
//=============================================================================
static inline void assert_splch_reset(struct ahm_channel *ch )
{
   struct ahm_device  *pdev = ch->m_pdev;
   uint64_t *addr;

   // Special case where read CSR area is actually written to
   addr = pdev->m_rbase + AHMPIP_GCSR_AFUn_VERSION_ID(ch->m_index);
   *addr = 0;

   addr = pdev->m_rbase + AHMPIP_GCSR_SPL_CHn_STATUS(ch->m_index);
   *addr = 0;

   pdev->set_csr(pdev, AHMPIP_GCSR_SPL_CHn_CTRL(ch->m_index),AHMPIP_SPL_CHCMD_RESET);

   mdelay(1);
}

//=============================================================================
// Name: ahm_csr_get32
// Description: Get  32 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         off - CSR register value relative to CSR_BASE
// Outputs: value.
// Comments:
//=============================================================================
static inline uint32_t
ahm_csr_get32 (splpip_handle_t hdl, unsigned long off)
{
   struct ahm_device *dev = (struct ahm_device *)hdl;

   uint32_t *addr = ((struct ahm_device *)dev)->m_rbase + off;

#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   return *addr;
}



//=============================================================================
// Name: ahm_csr_get64_idx
// Description: Get 64 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         idx - CSR register index relative to base.
// Outputs: value.
// Comments:
//=============================================================================
static inline uint64_t
ahm_csr_get64_idx(splpip_handle_t hdl, unsigned long idx)
{
   struct ahm_device *dev = (struct ahm_device *)hdl;

   // Get the pointer to the CSR via the index
   uint64_t *addr = dev->m_rbase + (idx << 7);

   printk (KERN_DEBUG "ahm_csr_get64_idx :%s[%d] rbase = %p idx = %lx  CSR @ %p value %lld",
           __AAL_FUNC__,current->tgid, dev->m_rbase, idx ,addr, *addr );
#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   return *addr;
}


//=============================================================================
// Name: ahm_csr_get64
// Description: Get 64 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         off - CSR register value relative to CSR_BASE
// Outputs: value.
// Comments:
//=============================================================================
static inline uint64_t
ahm_csr_get64(splpip_handle_t hdl, unsigned long off)
{
         struct ahm_device *dev = (struct ahm_device *)hdl;

   uint64_t *addr = dev->m_rbase + off;

//   printk (KERN_DEBUG "ahm_csr_get64 :%s[%d] rbase = %p off = %lx  CSR @ %p value %lld", __AAL_FUNC__,current->tgid, dev->m_rbase, off ,addr, *addr );
#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   return *addr;
}

//=============================================================================
// Name: ahm_csr_set32
// Description: Set 32 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         off - CSR register value relative to CSR_BASE
// Outputs: none.
// Comments: Copies value to read section to emulate HW update
//=============================================================================
static inline void
ahm_csr_set32 (splpip_handle_t hdl, unsigned long off, uint32_t val)
{
  struct ahm_device *dev = (struct ahm_device *)hdl;

   uint32_t *addr;

#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   addr = dev->m_wbase + off;
   *addr = val;

   if(off < AHMPIP_AFU_BASE ) {
      addr = dev->m_rbase + off;
      *addr = val;
   }
}

//=============================================================================
// Name: ahm_csr_set64_idx
// Description: Set 64 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         idx - CSR register index relative to CSR_BASE
//         val - value to set it to
// Outputs: none.
// Comments: Copies value to read section to emulate HW update
//=============================================================================
static inline void
ahm_csr_set64_idx(  splpip_handle_t hdl, unsigned long idx, uint64_t val)
{
//   printk (KERN_DEBUG "ahm_csr_set64_idx %p",hdl);
   struct ahm_device *dev = (struct ahm_device *)hdl;
   uint64_t *addr;

//   printk (KERN_DEBUG "ahm_csr_set64_idx  dev %p",dev);
//   printk (KERN_DEBUG "ahm_csr_set64_idx :%s[%d] wbase = %p idx = %lx  CSR @ %p", __AAL_FUNC__,current->tgid, dev->m_wbase, idx , addr );
#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   addr = dev->m_wbase + (idx<<7);
   *addr = val;
//   printk (KERN_DEBUG "MIRROR ahm_csr_set64_idx :%s[%d] wbase = %p idx = %lx  CSR @ %p value %lld", __AAL_FUNC__,current->tgid, dev->m_wbase, idx , addr, *addr );
   if( (idx<<7) < AHMPIP_AFU_BASE ) {
     addr = dev->m_rbase + (idx<<7);
     *addr = val;
   }

//   printk (KERN_DEBUG "DONE ahm_csr_set64_idx :%s[%d] wbase = %p idx = %lx  CSR @ %p value %lld", __AAL_FUNC__,current->tgid, dev->m_wbase, idx , addr, *addr );
}


//=============================================================================
// Name: ahm_csr_set64
// Description: Set 64 Bit CSR value
// Interface: private
// Inputs: hdl - device
//         off - CSR register value relative to CSR_BASE
// Outputs: none.
// Comments: Copies value to read section to emulate HW update
//=============================================================================
static inline void
ahm_csr_set64 (  splpip_handle_t hdl, unsigned long off, uint64_t val)
{
//   printk (KERN_DEBUG "ahm_csr_set64 %p",hdl);
   struct ahm_device *dev = (struct ahm_device *)hdl;
   uint64_t *addr;
//   printk (KERN_DEBUG "ahm_csr_set64  dev %p",dev);
//   printk (KERN_DEBUG "ahm_csr_set64 :%s[%d] wbase = %p off = %lx  CSR @ %p", __AAL_FUNC__,current->tgid, dev->m_wbase, off , addr );
#if defined(_CSR_DELAY)
   udelay (_CSR_DELAY);
#endif

   addr = dev->m_wbase + off;
   *addr = val;
//   printk (KERN_DEBUG "MIRROR ahm_csr_set64 :%s[%d] wbase = %p off = %lx  CSR @ %p value %lld", __AAL_FUNC__,current->tgid, dev->m_wbase, off , addr, *addr );
   if(off < AHMPIP_AFU_BASE ) {
     addr = dev->m_rbase + off;
     *addr = val;
   }

//   printk (KERN_DEBUG "DONE ahm_csr_set64 :%s[%d] wbase = %p off = %lx  CSR @ %p value %lld", __AAL_FUNC__,current->tgid, dev->m_wbase, off , addr, *addr );
}

//=============================================================================
// Name: ahmpip_free_desc_buff
// Description: destroys the descriptor buffer
// Interface: public
// Inputs: base pointer of the descriptor buffer.
// Outputs: none.
// Comments: Called by subordinate module
//=============================================================================
inline static void ahmpip_free_desc_buff( void *base )
{
   unsigned int order = 0;

   order = get_order( AHMPIP_AHM_DESCRIPTOR_SIZE );
   /* clear page's reserved flag */
   ahm_clr_page_reserved (virt_to_page (base), 1 << (order + PAGE_SHIFT));
   free_pages ((unsigned long) base, order);
}


////////////////////////////////////////////////////////////////////////////////////
//  export functions
////////////////////////////////////////////////////////////////////////////////////

extern int ahmpip_free_ahm_device(  struct ahm_device *pdev);
extern int ahmpip_init_ahm_device(  struct ahm_device *pdev,
                                    struct aal_bus *pbus,
                                    struct aal_device_id *devID,
                                    struct aal_class_id *classID,
                                    u_int64_t pipGUID
                                 );

#ifdef _USE_INTERRUPTS
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
irqreturn_t ahmpip_work_intr (int irq, void *arg);
#else
irqreturn_t ahmpip_work_intr (int irq, void *arg, struct pt_regs *regs);
#endif

extern int ahmpip_get_irq( int user_irq );
extern void ahmpip_init_intr_table( struct ahm_device *pdev );
#endif


/*
 * common macros
 */

/*
 * uintptr_t should be used instead
 */
#ifndef POINTER_TO_UINT64
#define POINTER_TO_UINT64(p)  ((uint64_t) ((unsigned long) (p)))
#endif
#ifndef UINT64_TO_POINTER
#define UINT64_TO_POINTER(u)  ((void *) ((unsigned long) (u)))
#endif


/*
 * FIXME: the device interface should be refined as driver
 * registration/unregistration routines to support multiple instances of the
 * same device class
 */


//=============================================================================
// Name: acpm_session
// Description: Object that represents an instance of a session between an
//              owner of a device and the device itself. It holds state such
//              as the task list and AHM PIP interface. The AHM PIP interface
//              holds the owner session context and the PIP function interfaces.
// Comments: This object is specific to the AHM PIP. The ownerSession contains
//           the generic session context shared between the AHM PIP and the
//           AAL kernel services.
//=============================================================================
struct acpm_session
{
  struct ahm_device           *dev;   // Board level device

  // PIP contains all of the interfaces we use for communications
  struct ahm_ipip             *ipipp;

  // Owner Session hold shared session instance information
  struct aaldev_ownerSession  *m_pownerSess;

  // Tasks structires
  kosal_list_head              task_list;
  kosal_semaphore              task_sem;
  uint16_t                     m_tidgroup;
  uint16_t                     m_nexttid;

  // Used to translate AAL TranID numbers to taskid
  kosal_list_head              tidmap_list;

};
#define ahmsessp_to_pipp(s)   ((struct ahm_ipip         *)s->ipipp)



/* ========== memory area ========== */

#define ACPM_MEM_AREA_INVALID (0ULL)

struct acpm_mem_area
{
  kosal_list_head   ma_list;  /* session list */
  atomic_t          ma_cnt;   /* reference count */
  uint64_t          ma_id;
  size_t            ma_size;
  unsigned long     ma_cookie;
  void              *ma_base;
  struct page       *ma_page;
  unsigned int      ma_order;
};

static inline int
acpm_mem_area_sanity_check (struct acpm_mem_area *ma)
{
   return (POINTER_TO_UINT64(ma) == ma->ma_id) ? 0 : -EINVAL;
}

extern void __acpm_mem_area_free  (struct acpm_mem_area *);
extern int acpm_mem_area_alloc (struct acpm_mem_area **, size_t);

static inline void
acpm_mem_area_ref (struct acpm_mem_area *ma)
{
   atomic_inc (&ma->ma_cnt);
}

static inline void
acpm_mem_area_unref (struct acpm_mem_area *ma)
{
   if (atomic_dec_and_test (&ma->ma_cnt)){
      __acpm_mem_area_free (ma);
   }
}

static inline void
acpm_mem_area_free (struct acpm_mem_area *ma)
{
   acpm_mem_area_unref (ma);
}


//=============================================================================
// Name: acpm_hwtask
// Description: Object that describes a descriptor on a task queue
//=============================================================================
#define ACPM_HWTASKID_INVALID (0ULL)
struct acpm_hwtask
{
   kosal_list_head         ht_list;           // tasks belonging to the same session
   kosal_list_head         ht_ready_queue;    // ready queue
   kosal_list_head         ht_compl_queue;    // completion queue
   kosal_list_head         ht_running_queue;  // running queue
   atomic_t                ht_cnt;            // reference count
   uint16_t                ht_id;
   uint32_t                ht_status;
   uint32_t                ht_ch;             // the channel this task will submit to
   TDESC_TYPE              ht_type;           // INPUT or OUTPUT
   uint16_t                ht_delim;          // Delimiter
   uint8_t                 ht_mode;           // SLAVE: '2'    MASTER PHYS: '0'     MASTER VIRT: '1'
   void                   *ht_context;        // Descriptor context - User data associated with the descriptor
   stTransactionID_t       ht_tasktranID;     // Task transaction ID. This is a duplicate of mapped value

   // AFU configuration data
   uint64_t                ht_buff;
   uint64_t                ht_pgtblbase;
   size_t                  ht_size;
   unsigned int            ht_pgtblsize;      // Must be CLs
   unsigned int            ht_pgsizeorder;

   /* reference to session */
   kosal_semaphore         ht_sess_sem;
   struct acpm_session    *ht_sess;

   // Fields related to the return event notification
   stTransactionID_t        tranID;           // Transaction ID to identify result
   void                    *context;          // Optional token from user when sending this message
   uint16_t                 no_notify;        // 1 - do not notify client on completion

   struct aal_uiapi         *uiAPIp;          // UI  driver interface to return on
   void                     *MessageContext;

   /* reference to input/output descriptor header */
   struct ahm_output_desc_header  *ht_odesc;
   struct ahm_input_desc_header    *ht_idesc;
};

static inline int
acpm_hwtask_sanity_check (struct acpm_hwtask *ht)
{
   return (POINTER_TO_UINT64(ht) == ht->ht_id) ? 0 : -EINVAL;
}

extern int  acpm_hwtask_create    (struct acpm_hwtask **, struct acpm_session *sess);
extern void acpm_hwtask_destroy   (struct acpm_hwtask *);
extern int  __acpm_hwtask_destroy (struct acpm_hwtask *);

extern int  acpm_hwtask_submit    (struct acpm_hwtask *);
extern void acpm_hwtask_abort     (struct acpm_hwtask *);

static inline void
acpm_hwtask_ref (struct acpm_hwtask *ht)
{
   atomic_inc (&ht->ht_cnt);
}

static inline void
acpm_hwtask_unref (struct acpm_hwtask *ht)
{
   if (atomic_dec_and_test (&ht->ht_cnt)){
       __acpm_hwtask_destroy (ht);
   }
}

/////////////////////////////////////////////////////////////////////////////////////
#define RING_LEN  (8)
#define RING_FLEN(x, y, b)  (((x) > (y)) ? ((b)+(y)-(x)) : ((y) - (x)))
#define RING_FWD(x, s, b)   (((x) + (s)) % (b))
#define RING_BWD(x, s, b)   RING_FWD((x), ((b)-(s)), (b))

extern int                   acpm_session_destroy(struct acpm_session *);
extern struct acpm_session * acpm_session_create(struct aaldev_ownerSession *);



#endif // __AALSDK_KERNEL_AHMPIP_H__

