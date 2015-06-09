#include "ase_common.h"

int rx_c2s_fifo;
int tx_s2c_fifo;
int rx_c2s_fifo2;
#define DUMMY_OPENCLOSE

void destroy()
{
  unlink("c2s_fifo1");
  unlink("c2s_fifo2");
  unlink("s2c_fifo1");
}

int main()
{
  destroy();

  // MQ Create
  mkfifo("c2s_fifo1", 0666);
  mkfifo("c2s_fifo2", 0666);
  mkfifo("s2c_fifo1", 0666);
  
  // MQ Open
  rx_c2s_fifo  = open("c2s_fifo1", O_RDONLY|O_NONBLOCK);
  rx_c2s_fifo2 = open("c2s_fifo2", O_RDONLY|O_NONBLOCK);
#ifdef DUMMY_OPENCLOSE
  int dummy;
  dummy = open("s2c_fifo1", O_RDONLY|O_NONBLOCK);
#endif
  tx_s2c_fifo  = open("s2c_fifo1", O_WRONLY);
  if (tx_s2c_fifo == -1) 
    {
      perror("open");
      exit(1);
    }

#ifdef DUMMY_OPENCLOSE
  close(dummy);
#endif

  printf("Server ON\n");
  
  // Server process
  int ret;
  char ibuf[16];
  int input;
  int output;
  char obuf[16];
  while(1)
    {
      memset(ibuf, '\0', 16);
      ret = read(rx_c2s_fifo, ibuf, 16);
      if (ret > 0)
	{
	  printf("Read %d bytes <= %s\n", ret, ibuf);
	  input = atoi(ibuf);
	  output = input * input;
	  sprintf(obuf, "%d", output);
	  write(tx_s2c_fifo, obuf, 16);
	}      
    }

  // MQ Close
  close(rx_c2s_fifo);
  close(rx_c2s_fifo2);
  close(tx_s2c_fifo);  

  // MQ destroy
  destroy();

  return 0;
}
