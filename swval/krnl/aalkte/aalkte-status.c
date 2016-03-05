// INTEL CONFIDENTIAL - For Intel Internal Use Only

// aalkte-status.c
//
// AAL Kernel Test Executor driver - status file

#include "aalkte.h"

static ssize_t aalkte_status_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
   const char *fail_pass[] = { "FAIL", "PASS" };

   struct aalkte_data *data = (struct aalkte_data *) filp->f_inode->i_private;

   enum aalkte_state st;
   bool test_pass_or_fail;

   const char *ans = NULL;
   size_t      len = 0;
   ssize_t     res = 0;

   if ( down_interruptible(&data->sem) ) {
      return -ERESTARTSYS;
   }

   st                = data->state;
   test_pass_or_fail = data->test_pass_or_fail;

   if ( AALKTE_ST_COMPLETE == st ) {
      ans = fail_pass[test_pass_or_fail];
      len = 4;
   } else {
      ans = aalkte_state_to_string(st);

      if ( NULL == ans ) {
         res = -EFAULT;
         goto _OUT;
      }

      len = strlen(ans);
   }

   if ( *offp >= len ) {
      // Requested offset is out of bounds.
      res = 0;
      goto _OUT;
   }

   if ( *offp + count > len ) {
      // Not that many remaining.
      count = len - *offp;
   }

   if ( copy_to_user(buff, ans + *offp, count) ) {
      res = -EFAULT;
      goto _OUT;
   }

   *offp += count;
   res = count;

_OUT:
   up(&data->sem);
   return res;
}

static struct file_operations status_fops =
{
   .owner = THIS_MODULE,
   .read  = aalkte_status_read,
};

struct dentry * aalkte_create_status_file(const char    *name,
                                          umode_t        mode,
                                          struct dentry *parent)
{
   return debugfs_create_file(name, mode, parent, aalkte_get_data(), &status_fops);
}

const char * aalkte_state_to_string(enum aalkte_state st)
{
   static const char *strs[] =
   {
      "IDLE",
      "RUNNING",
      "CANCELED",
      "COMPLETE"
   };
   int i = (int) st;

   if ( ( i < 0 ) || ( i >= (sizeof(strs) / sizeof(strs[0])) ) ) {
      return NULL;
   }

   return strs[i];
}

