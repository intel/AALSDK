#include <stdio.h>
#include "lib/sysdep.h"
#include "lib/pci.h"
#include"stdlib.h"
#include<math.h>
unsigned long convertToDecimal(char hex[]);
int main(int argc , char *argv[])
{
  struct pci_access *pacc;
  struct pci_dev *dev;
  if (argc != 4 && argc!=5){
 printf("INVALID ARGUMENTS (%d) \n write -> VENDOR DEVICE register hexvalue \n read -> VENDOR DEVICE register \n",argc);
return 0;}
  long c;
  int FLAG=1;
  pacc = pci_alloc();           /* Get the pci_access structure */
  
  pci_init(pacc);               /* initilization of PCI library */
  pci_scan_bus(pacc);           /* getting list of devices */
  
  for (dev=pacc->devices; dev; dev=dev->next)      
    {   if(dev->vendor_id== HexToDec(argv[1]) && dev->device_id == HexToDec(argv[2]) ){ ////selecting PCI device//////
///////////////////////////////////////////////////////////////////
/**/       if(argc==4){                                     	 //
//        READ FUNCTION                       			 // 
/**/	c = pci_read_long(dev,HexToDec(argv[3]));}               //
/**/       else if (argc==5){}                                   //
//        write FUNCTION                      			 //
/**/	//pci_write_long(dev,HexToDec(argv[3]),00000000)}        //    
//                                            			 //
///////////////////////////////////////////////////////////////////

	printf("vendor=%x device=%04x register value: %lx \n",dev->vendor_id, dev->device_id,c);

      FLAG=0;
         }
	}
if (FLAG==1){
printf("DEVICE NOT FOUND \n");
exit(0);}
  pci_cleanup(pacc);            /* Close everything */
  return 0;
}

unsigned long HexToDec(char hex[])
{
    char *hexreg;
    int reglen = 0;
    const int base = 16; 
    unsigned long decreg = 0;
    int i;
    for (hexreg = hex; *hexreg != '\0'; hexreg++)
    {
        reglen++;
    }
    hexreg = hex;
    for (i = 0; i < reglen && *hexreg != '\0'; hexreg++,i++ )
    {
        if (*hexreg >= 48 && *hexreg <= 57)   
        {
            decreg += (((int)(*hexreg)) - 48) * pow(base, reglen - i - 1);
        }
        else if ((*hexreg >= 65 && *hexreg <= 70))   
        {
            decreg += (((int)(*hexreg)) - 55) * pow(base, reglen - i - 1);
        }
        else if (*hexreg >= 97 && *hexreg <= 102)   
        {
            decreg += (((int)(*hexreg)) - 87) * pow(base, reglen - i - 1);
        }
        else
        {
            printf(" Invalid register \n");          
            exit(0);
        }
    }
    return decreg;
}

