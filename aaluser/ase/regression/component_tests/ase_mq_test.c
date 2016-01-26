#include "ase_common.h"

struct mq_attr attr;
struct mq_attr new_attr;
mqd_t mq;

int main()
{
  mq = mq_open ("/test_ase", O_RDWR|O_CREAT );
  if (mq == -1)
    {
      perror("mq_open");
      exit(1);
    }
  
  if(mq_getattr(mq, &attr) == -1)
    {
      perror("mq_getattr");
      exit(1);
    }
  printf("MQ attributes =>\n");
  printf("flags   = %d\n", attr.mq_flags);
  printf("maxmsg  = %d\n", attr.mq_maxmsg);
  printf("msgsize = %d\n", attr.mq_msgsize);
  printf("curmsgs = %d\n", attr.mq_curmsgs);

  mq_close(mq);

  new_attr.mq_flags = 0;
  new_attr.mq_maxmsg = 10;
  new_attr.mq_msgsize = 1024;
  new_attr.mq_curmsgs = 0;

  printf("New MQ attributes =>\n");
  printf("flags   = %d\n", new_attr.mq_flags);
  printf("maxmsg  = %d\n", new_attr.mq_maxmsg);
  printf("msgsize = %d\n", new_attr.mq_msgsize);
  printf("curmsgs = %d\n", new_attr.mq_curmsgs);
  
  printf("Opening new message queue =>\n");
  mq = mq_open ("/test", O_RDWR|O_CREAT, 0660, &new_attr);
  if (mq == -1)
    {
      perror("mq_open");
      exit(1);
    }
  
  if(mq_getattr(mq, &attr) == -1)
    {
      perror("mq_getattr");
      exit(1);
    }
  printf("MQ attributes =>\n");
  printf("flags   = %d\n", attr.mq_flags);
  printf("maxmsg  = %d\n", attr.mq_maxmsg);
  printf("msgsize = %d\n", attr.mq_msgsize);
  printf("curmsgs = %d\n", attr.mq_curmsgs);

  mq_close(mq);

  return 0;
}
