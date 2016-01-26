#include "ase_common.h"

int main()
{
  char *out_str ;
  out_str = malloc (20);

  int i;

  put_timestamp();
  out_str = get_timestamp();
  printf("%s\n",get_timestamp());
  printf("%s\n",out_str);
  
  for (i = 0; i < 20; i++) 
    {
      printf("%c ", out_str[i]);
    }
  printf("\n");

  return 0;
}
