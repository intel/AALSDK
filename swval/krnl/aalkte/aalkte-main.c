// INTEL CONFIDENTIAL - For Intel Internal Use Only

//
// AAL Kernel Test Executor driver
//

#include "aalkte.h"

#define DRV_DESCRIPTION "AAL Kernel Test Executor driver"
#define DRV_AUTHOR      "Tim Whisonant <tim.whisonant@intel.com>"
#define DRV_LICENSE     "GPL"
#define DRV_COPYRIGHT   "INTEL CONFIDENTIAL - For Intel Internal Use Only"

MODULE_VERSION    (DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR     (DRV_AUTHOR);
MODULE_LICENSE    (DRV_LICENSE);

static int  aalkte_init(void);
module_init(aalkte_init);

static void aalkte_exit(void);
module_exit(aalkte_exit);

static struct dentry *pRootDebugFSDir = NULL;

static int aalkte_init(void)
{
   printk(KERN_INFO "%s: %s\n", DRV_NAME, DRV_DESCRIPTION);
   printk(KERN_INFO "%s: Version %s\n", DRV_NAME, DRV_VERSION);
   printk(KERN_INFO "%s: %s\n", DRV_NAME, DRV_COPYRIGHT);

   sema_init(&aalkte_get_data()->sem, 1);    // data structure is unlocked.
   sema_init(&aalkte_get_data()->thrsem, 0); // no thread is active.

   pRootDebugFSDir = debugfs_create_dir("aalkte", NULL);
   if ( NULL == pRootDebugFSDir ) {
      printk(KERN_ERR "%s: debugfs_create_dir() returned NULL. debugfs support included in kernel?\n", DRV_NAME);
      return 1;
   }

   if ( NULL == aalkte_create_control_file("control", AALKTE_CONTROL_MODE, pRootDebugFSDir) ) {
      printk(KERN_ERR "%s: Failed to create control file.\n", DRV_NAME);
      return 2;
   }

   if ( NULL == aalkte_create_status_file("status", AALKTE_STATUS_MODE, pRootDebugFSDir) ) {
      printk(KERN_ERR "%s: Failed to create status file.\n", DRV_NAME);
      return 3;
   }

   return 0;
}

static void aalkte_exit(void)
{
   aalkte_thr_stop(true);

   debugfs_remove_recursive(pRootDebugFSDir);

   printk(KERN_INFO "%s: exiting\n", DRV_NAME);
}

static struct aalkte_data driver_data =
{
   .state = AALKTE_ST_IDLE,
};

struct aalkte_data * aalkte_get_data(void)
{
   return &driver_data;
}

/*
 KERN_EMERG   0
 KERN_ALERT   1
 KERN_CRIT    2
 KERN_ERR     3
 KERN_WARNING 4 
 KERN_NOTICE  5
 KERN_INFO    6
 KERN_DEBUG   7
 KERN_DEFAULT 8
 KERN_CONT    9
*/

