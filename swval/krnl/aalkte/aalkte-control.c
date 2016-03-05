// INTEL CONFIDENTIAL - For Intel Internal Use Only

//
// AAL Kernel Test Executor driver - control file
//

#include "aalkte.h"

static ssize_t aalkte_control_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{

   return 0;
}

static ssize_t aalkte_control_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{

   return 0;
}

static struct file_operations control_fops =
{
   .owner = THIS_MODULE,
   .read  = aalkte_control_read,
   .write = aalkte_control_write,
};

struct dentry * aalkte_create_control_file(const char    *name,
                                           umode_t        mode,
                                           struct dentry *parent)
{
   return debugfs_create_file(name, mode, parent, aalkte_get_data(), &control_fops);
}

