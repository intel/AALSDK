// INTEL CONFIDENTIAL - For Intel Internal Use Only

// aalkte-control.c
//
// AAL Kernel Test Executor driver - control file

#include "aalkte.h"

static void do_control_command(struct aalkte_data * , enum aalkte_command );

static ssize_t aalkte_control_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
   struct aalkte_data *data = (struct aalkte_data *) filp->f_inode->i_private;

   enum aalkte_state state;

   char        file_contents[32];
   size_t      content_len = 0;
   const char *str;
   ssize_t     res = 0;

   if ( down_interruptible(&data->sem) ) {
      return -ERESTARTSYS;
   }

   state = data->state;
 
   str = aalkte_state_to_string(state);

   if ( NULL == str ) {
      res = -EFAULT;
      goto _OUT;
   }

   memset(file_contents, 0, sizeof(file_contents));

   content_len = strlen(str);
   strncpy(file_contents, str, content_len);
   strncat(file_contents, "\n", 1);
   ++content_len;
 
   if ( *offp >= content_len ) {
      // Requested offset is out of bounds.
      res = 0;
      goto _OUT;
   }

   if ( *offp + count > content_len ) {
      // Not that many bytes remaining. Lop off the count..
      count = content_len - *offp;
   }

   if ( copy_to_user(buff, &file_contents[*offp], count) ) {
      res = -EFAULT;
      goto _OUT;
   }

   *offp += count;
   res = count;

_OUT:
   up(&data->sem);
   return res;
}

static ssize_t aalkte_control_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
   struct aalkte_data *data = (struct aalkte_data *) filp->f_inode->i_private;

   enum aalkte_command cmd;

   ssize_t res = 0;

   if ( down_interruptible(&data->sem) ) {
      return -ERESTARTSYS;
   }

   if ( *offp >= sizeof(data->control_file_write_buf) ) {
      // Attempt to write past end of acceptible file size.
      res = -EFAULT;
      goto _OUT;
   }

   if ( *offp + count > sizeof(data->control_file_write_buf) ) {
      // Not that many bytes available in the file buffer. Lop off the count..
      count = sizeof(data->control_file_write_buf) - *offp;
   }

   if ( copy_from_user(&data->control_file_write_buf[*offp], buff, count) ) {
      res = -EFAULT;
      goto _OUT;
   }

   *offp += count;
   res = count;

   cmd = aalkte_command_from_string(data->control_file_write_buf);
   if ( AALKTE_CMD_INVALID != cmd ) {
      do_control_command(data, cmd);
   }

_OUT:
   up(&data->sem);
   return res;
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

enum aalkte_command aalkte_command_from_string(const char *str)
{
   if ( 0 == strncmp(str, "EXECUTE", 7) ) {
      return AALKTE_CMD_EXECUTE;
   } else if ( 0 == strncmp(str, "CANCEL", 6) ) {
      return AALKTE_CMD_CANCEL;
   } else if ( 0 == strncmp(str, "CLEARCANCEL", 11) ) {
      return AALKTE_CMD_CLEARCANCEL;
   } else if ( 0 == strncmp(str, "CLEARCOMPLETE", 13) ) {
      return AALKTE_CMD_CLEARCOMPLETE;
   }

   return AALKTE_CMD_INVALID;
}

const char * aalkte_command_to_string(enum aalkte_command c)
{
   static const char *strs[] =
   {
      "EXECUTE",
      "CANCEL",
      "CLEARCANCEL",
      "CLEARCOMPLETE"
   };
   int i = (int) c;

   if ( ( i < 0 ) || ( i >= (sizeof(strs) / sizeof(strs[0])) ) ) {
      return NULL;
   }

   return strs[i];
}

static void do_control_command(struct aalkte_data *data, enum aalkte_command cmd)
{
   // assert: c is a valid enumeration value
   // assert: c != AALKTE_CMD_INVALID
   // assert: data->sem is locked

   switch ( data->state ) {

      case AALKTE_ST_IDLE : {

         if ( AALKTE_CMD_EXECUTE == cmd ) {
            // IDLE -> RUNNING

            data->state = AALKTE_ST_RUNNING;



         } else {
            printk(KERN_WARNING "%s: ignorning invalid state transition from IDLE via %s\n",
                                DRV_NAME, aalkte_command_to_string(cmd));
         }

      } break;

      case AALKTE_ST_RUNNING : {

         if ( AALKTE_CMD_CANCEL == cmd ) {
            // RUNNING -> CANCELED

            data->state = AALKTE_ST_CANCELED;



         } else {
            printk(KERN_WARNING "%s: ignorning invalid state transition from RUNNING via %s\n",
                                DRV_NAME, aalkte_command_to_string(cmd));
         }

      } break;

      case AALKTE_ST_CANCELED : {

         if ( AALKTE_CMD_CLEARCANCEL == cmd ) {
            // CANCELED -> IDLE

            data->state = AALKTE_ST_IDLE;



         } else {
            printk(KERN_WARNING "%s: ignorning invalid state transition from CANCELED via %s\n",
                                DRV_NAME, aalkte_command_to_string(cmd));
         }

      } break;

      case AALKTE_ST_COMPLETE : {

         if ( AALKTE_CMD_CLEARCOMPLETE == cmd ) {
            // COMPLETE -> IDLE

            data->state = AALKTE_ST_IDLE;


         } else {
            printk(KERN_WARNING "%s: ignorning invalid state transition from COMPLETE via %s\n",
                                DRV_NAME, aalkte_command_to_string(cmd));
         }

      } break;

   }
}

