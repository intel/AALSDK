#include "ase_common.h"

int tx_c2s_fifo;
int rx_s2c_fifo;

int i;
char obuf[16];
char ibuf[16];
int ret;

int main()
{
  tx_c2s_fifo = open("c2s_fifo1", O_WRONLY);
  rx_s2c_fifo = open("s2c_fifo1", O_RDONLY);
  
  for(i = 0; i < 10; i++)
    {      
      sprintf(obuf, "%d", i);
      write(tx_c2s_fifo, obuf, 16);
      memset(ibuf, '\0', 16);
      ret = read(rx_s2c_fifo, ibuf, 16);
      printf("RXed => %s\n", ibuf);      
    }
    
  close(tx_c2s_fifo);
  close(rx_s2c_fifo);

  return 0;
}
