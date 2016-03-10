// INTEL CONFIDENTIAL - For Intel Internal Use Only

// aalkte-status.c
//
// AAL Kernel Test Executor driver - status file

#include "aalkte.h"

static ssize_t aalkte_status_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
   const char *fail_pass[] = { "FAIL", "PASS" };

   struct aalkte_data *data = (struct aalkte_data *) filp->f_inode->i_private;

   enum aalkte_state state;
   bool test_pass_or_fail;

   char        file_contents[32];
   size_t      content_len = 0;
   const char *str;
   ssize_t     res = 0;

   if ( down_interruptible(&data->sem) ) {
      return -ERESTARTSYS;
   }

   state             = data->state;
   test_pass_or_fail = data->test_pass_or_fail;

   memset(file_contents, 0, sizeof(file_contents));

   if ( AALKTE_ST_COMPLETE == state ) {
      strncpy(file_contents, fail_pass[test_pass_or_fail], 4);
      strncat(file_contents, "\n", 1);
      content_len = 5;
   } else {
      // printk(KERN_INFO "%s: status_read(): file offset is %llu\n", DRV_NAME, *offp);

      str = aalkte_state_to_string(state);

      if ( NULL == str ) {
         res = -EFAULT;
         goto _OUT;
      }

      content_len = strlen(str);
      // printk(KERN_INFO "%s: status_read(): state is \"%s\" len = %lu\n", DRV_NAME, str, content_len);
      strncpy(file_contents, str, content_len);
      strncat(file_contents, "\n", 1);
      ++content_len;
      // printk(KERN_INFO "%s: status_read(): buffer has %s len = %lu\n", DRV_NAME, file_contents, content_len);
   }

   if ( *offp >= content_len ) {
      // Requested offset is out of bounds.
      res = 0;
      // printk(KERN_INFO "%s: status_read(): requested offset %llu is out of bounds (EOF)\n", DRV_NAME, *offp);
      goto _OUT;
   }

   if ( *offp + count > content_len ) {
      // Not that many bytes remaining. Lop off the count..
      count = content_len - *offp;
      // printk(KERN_INFO "%s: status_read(): truncating count to %lu\n", DRV_NAME, count);
   }

   if ( copy_to_user(buff, &file_contents[*offp], count) ) {
      res = -EFAULT;
      goto _OUT;
   }

   *offp += count;
   res = count;

   // printk(KERN_INFO "%s: status_read(): new file offset / return is %llu / %lu\n", DRV_NAME, *offp, res);

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

