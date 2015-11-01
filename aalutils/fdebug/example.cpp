/*
 *	The PCI Library -- Example of use (simplistic lister of PCI devices)
 *
 *	Written by Martin Mares and put to public domain. You can do
 *	with it anything you want, but I don't give you any warranty.
 */

#include <stdio.h>
#include <stdint.h>              /* uintptr_t */
#include <string.h>              /* memset */

#ifdef __cplusplus
extern "C" {
#endif
#  include <pci/pci.h>           /* allow this code to be C++ */
#ifdef __cplusplus
}
#endif

u16 gvendor_id = 0x8086;          /* Identity of the device */
u16 gdevice_id = 0xBCBD;


#define NUM_BUFS   4             /* how many buffers do you want? */
#define MAX_BOARDS 2             /* will not track more than this number of boards */

struct buffer {
   void  *pbase;                 /* user virtual pointer if it exists */
   uintptr_t physaddr;           /* physical address of buffer */
   int       fd;                 /* file descriptor, 0 if none */
};
struct buffer gbuffers[NUM_BUFS]; /* START HERE FOR BUFFERS */


struct mmioRegion {
   void  *pbase;                 /* pointer to region, NULL if not available */
   size_t len;                   /* length in bytes, 0 if not there */
   int    fd;                    /* file descriptor, 0 if none */
};

struct board {
   struct pci_dev *dev;          /* NULL if there is not a board. */
                                 /* can RW config space with this dev if want to */
   struct mmioRegion bar[6];     /* BAR pointers if exist, Un-Cached */
   struct mmioRegion wc_bar[6];  /* BAR pointers if exist, Write-Combining*/
};

struct board gboards[MAX_BOARDS]; /* START HERE FOR BOARDS */
int gnum_boards = 0;


/* internal function forward definitions */
static int init_boards( void );  /* initialized global gboards[] */



int main(void)
{
   struct pci_access *pacc;
   struct pci_dev *dev;
   unsigned int c;
   char namebuf[1024], *name;
   char resbuf[1024];
   int retval = 0;

   memset( gboards, 0, sizeof(gboards));
   memset( gbuffers, 0, sizeof(gbuffers));

   pacc = pci_alloc();     /* Get the pci_access structure */
   /* Set all options you want -- here we stick with the defaults */
   pci_init(pacc);         /* Initialize the PCI library */
   pci_scan_bus(pacc);     /* We want to get the list of devices */
   for (dev = pacc->devices; dev; dev = dev->next) /* Iterate over all devices */
   {
      pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); /* Fill in header info we need */

      /* only looking for specific device and vendor */
      if ( gvendor_id == dev->vendor_id /* && gdevice_id == dev->device_id */) {

         /* Remember this board */
         int this_board = gnum_boards;
         if (gnum_boards < MAX_BOARDS) {
            ++gnum_boards;
         } else {
            /* ran over our array, abort the loop */
            fprintf( stderr, "More boards than expected. Only handling %d.\n", MAX_BOARDS);
            break;
         }
         gboards[this_board].dev = dev;

         c = pci_read_byte(dev, PCI_INTERRUPT_PIN); /* Read config register directly */
         printf("%04x:%02x:%02x.%d vendor=%04x device=%04x class=%04x irq=%d (pin %d) base0=%lx\n",
            dev->domain, dev->bus, dev->dev, dev->func, dev->vendor_id, dev->device_id,
            dev->device_class, dev->irq, c, (long) dev->base_addr[0]);


         /* create the file name for the resources and see if they exist */
         /* WARNING: reusing c and namebuf[] for other purposes */
         c = snprintf( namebuf, sizeof(namebuf),
                   "/sys/bus/pci/devices/%04x:%02x:%02x.%d/resource",
                   dev->domain, dev->bus, dev->dev, dev->func);
         if (c < 0 && c >= sizeof(namebuf)) {
            fprintf( stderr, "String too long, aborting with code 1.\n" );
            retval = 1;
            goto exit0;
         }





      } /* end of if (vendor_id == ... */
   } /* end of for (dev = pacc->devices ... loop */


   /* put your code here */
   /* note that you have all the necessary information in boards array and buffers array */
   {

   }



exit0:
   pci_cleanup(pacc); /* Close everything */
   return retval;
}
