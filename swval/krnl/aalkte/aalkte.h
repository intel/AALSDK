// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __AALKTE_AALKTE_H__
#define __AALKTE_AALKTE_H__
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/aer.h>
#include <linux/debugfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>

enum aalkte_state
{
   AALKTE_ST_IDLE = 0,
   AALKTE_ST_RUNNING,
   AALKTE_ST_CANCELED,
   AALKTE_ST_COMPLETE,
};

enum aalkte_command
{
   AALKTE_CMD_EXEC = 0,
   AALKTE_CMD_CANCEL,
   AALKTE_CMD_CLEARCANCEL,
   AALKTE_CMD_CLEARCOMPLETE,
};

struct aalkte_data
{
   struct semaphore    sem;    // Protects this data structure.
   enum aalkte_state   state;
   unsigned long       thrnum;
   struct task_struct *thr;
   struct semaphore    thrsem; // User process blocks on this until thr is done.

   bool                test_pass_or_fail;
};

struct aalkte_data * aalkte_get_data(void);

const char * aalkte_state_to_string(enum aalkte_state );

bool aalkte_thr_start(void);   // true if thread started
void aalkte_thr_stop(bool bWait);

/*
control/status State Machine

                              AALKTE_CMD_CLEARCANCEL
                                        \
   -----------------------------------------------------------------------------------
 |                                                                                    |
 |               AALKTE_CMD_EXEC             AALKTE_CMD_CANCEL                        |
 |                      \                            \                                |
 |     (control: IDLE)   \       (control: RUNNING)   \       (control: CANCELED)     |
 |     (status:  IDLE)    \      (status:  RUNNING)    \      (status:  CANCELED)     |
 |     ----------------    \     -------------------    \     --------------------    |
  --> | AALKTE_ST_IDLE | -----> | AALKTE_ST_RUNNING | -----> | AALKTE_ST_CANCELED | --
 |     ----------------          -------------------          --------------------
 |                                      |
 |                                      | (tests complete)
 |                                      v
 |                               (control: COMPLETE)
 |   AALKTE_CMD_CLEARCOMPLETE    (status:  PASS or FAIL)
 |              \                --------------------
  ----------------------------- | AALKTE_ST_COMPLETE |
                                 --------------------
  
*/


#define AALKTE_CONTROL_MODE ( S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH )
struct dentry * aalkte_create_control_file(const char    *name,
                                           umode_t        mode,
                                           struct dentry *parent);

#define AALKTE_STATUS_MODE  ( S_IRUSR | S_IRGRP | S_IROTH )
struct dentry *  aalkte_create_status_file(const char    *name,
                                           umode_t        mode,
                                           struct dentry *parent);

#endif // __AALKTE_AALKTE_H__

