// INTEL CONFIDENTIAL - For Intel Internal Use Only

// aalkte-thr.c
//
// AAL Kernel Test Executor driver - kernel thread

#include "aalkte.h"

static int aalkte_thread(void *p)
{
   struct aalkte_data *data = (struct aalkte_data *)p;

   while ( !kthread_should_stop() ) {


   }
  
   down(&data->sem);
   data->thr = NULL;  // thread is exiting.
   up(&data->sem);

   up(&data->thrsem); // wake any waiters.

   do_exit(0);
   return 0;
}

bool aalkte_thr_start(void)
{
   struct aalkte_data *data = aalkte_get_data();
   bool                res  = false;

   down(&data->sem);

   if ( NULL != data->thr ) {
      // A thread is already running.
      up(&data->sem);
      return false;
   }

   sema_init(&data->thrsem, 0);

   data->thr = kthread_run(aalkte_thread, data, "%s%lu", DRV_NAME, data->thrnum++);

   res = (NULL != data->thr);

   up(&data->sem);

   return res;
}

void aalkte_thr_stop(bool bWait)
{
   struct aalkte_data *data = aalkte_get_data();

   down(&data->sem);

   if ( NULL == data->thr ) {
      // no active thread.
      up(&data->sem);
      return;
   }

   kthread_stop(data->thr);

   up(&data->sem);

   if ( bWait ) {
      down(&data->thrsem);
      up(&data->thrsem); // don't block other waiters.
   }
}

