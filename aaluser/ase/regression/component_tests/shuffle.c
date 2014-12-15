#include <stdio.h>
#include <stdlib.h>

void shuffle(int *array, int n)
{
  int i, j, t;

  srand(time(NULL));  

  if (n > 1) 
    {
      for (i = 0; i < n - 1; i++) 
        {
          j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
  
  for(i = 0; i < n-1; i++)
    {
      printf("%d ", array[i]);      
    }
  printf("\n");
}


int main()
{
  int test[] = {5,4,2,3,7};

  shuffle(test, 5);

  return 0;
}

