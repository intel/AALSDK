/*
 *	The PCI Library -- Example of use (simplistic lister of PCI devices)
 *
 *	Written by Martin Mares and put to public domain. You can do
 *	with it anything you want, but I don't give you any warranty.
 */

#include <sys/stat.h>                        /* stat */
#include <sys/mman.h>                        /* mmap */
#include <stdio.h>
#include <stdint.h>                          /* uintptr_t */
#include <errno.h>                           /* errno */
#include <string.h>                          /* memset, strerror */
#include <fcntl.h>                           /* open */
#include <unistd.h>                          /* close */

#ifdef __cplusplus
extern "C" {
#endif
#  include <pci/pci.h>                       /* allow this code to be C++ */
#ifdef __cplusplus
}
#endif

u16 gvendor_id = 0x8086;                     /* Identity of the device */
u16 gdevice_id = 0xBCBD;

#define NUM_BUFS                4            /* how many buffers do you want? */
#define MAX_BOARDS              2            /* will not track more than this number of boards */
#define MAX_FILE_NAME_LENGTH 1024            /* probably should use system value, but reasonable for now */
#define NUM_BARS                6            /* constant, but, you know ... */

struct buffer {
   void     *pbase;                          /* user virtual pointer if it exists */
   size_t    len;                            /* length in bytes, 0 if not there */
   uintptr_t physaddr;                       /* physical address of buffer */
   int       fd;                             /* file descriptor, 0 if none */
   char      filename[MAX_FILE_NAME_LENGTH]; /* file name of this huge buffer */
};
struct buffer gbuffers[NUM_BUFS];            /* START HERE FOR BUFFERS */


struct mmioRegion {
   void     *pbase;                          /* pointer to region, NULL if not available */
   size_t    len;                            /* length in bytes, 0 if not there */
   int       fd;                             /* file descriptor, 0 if none */
   unsigned  flags;                          /* 0x01 if WC */
#define mmioRegion_flags_is_wc 0x01
   char      filename[MAX_FILE_NAME_LENGTH]; /* file name of this mmio region */
};

struct board {
   struct pci_dev *pdev;                     /* NULL if there is not a board. */
                                             /* can RW config space with this dev if want to */
   struct mmioRegion bar[NUM_BARS];                 /* BAR pointers if exist, Un-Cached */
   struct mmioRegion wc_bar[NUM_BARS];              /* BAR pointers if exist, Write-Combining*/
};

struct board gboards[MAX_BOARDS];            /* START HERE FOR BOARDS */
int gnum_boards = 0;


/* internal utility function forward definitions */
static int init_boards( struct board *pboards, int num_boards );  /* initialized global gboards[] */ /* TODO */
static int init_mmioRegion( struct pci_dev *pdev, struct mmioRegion *pbar, int numbar, unsigned flags);
static int close_and_clean_mmioRegion( struct mmioRegion *pmmio);


int main(void)
{
   struct pci_access *pacc;
   struct pci_dev *pdev;
   int c;
   char namebuf[MAX_FILE_NAME_LENGTH], *name;
   char resbuf[MAX_FILE_NAME_LENGTH];
   int retval = 0;

   memset( gboards, 0, sizeof(gboards));
   memset( gbuffers, 0, sizeof(gbuffers));

   pacc = pci_alloc();     /* Get the pci_access structure */
   /* Set all options you want -- here we stick with the defaults */
   pci_init(pacc);         /* Initialize the PCI library */
   pci_scan_bus(pacc);     /* We want to get the list of devices */
   for (pdev = pacc->devices; pdev; pdev = pdev->next) /* Iterate over all devices */
   {
      pci_fill_info(pdev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); /* Fill in header info we need */

      /* only looking for specific device and vendor */
      if ( gvendor_id == pdev->vendor_id && gdevice_id == pdev->device_id ) {
//      if ( 0x00 == pdev->bus && 0x0f == pdev->dev && 0x0 == pdev->func ) {  /* debug device */

         /* Remember this board */
         int this_board = gnum_boards;
         if (gnum_boards < MAX_BOARDS) {
            ++gnum_boards;
         } else {
            /* ran over our array, abort the loop */
            fprintf( stderr, "More boards than expected. Only handling %d.\n", MAX_BOARDS);
            break;
         }
         gboards[this_board].pdev = pdev;

         c = pci_read_byte(pdev, PCI_INTERRUPT_PIN); /* Read config register directly */
         printf("%04x:%02x:%02x.%d vendor=%04x device=%04x class=%04x irq=%d (pin %d) base0=%lx\n",
            pdev->domain, pdev->bus, pdev->dev, pdev->func, pdev->vendor_id, pdev->device_id,
            pdev->device_class, pdev->irq, c, (long) pdev->base_addr[0]);

         /* get the bars if they exist */

         for ( int i = 0; i<NUM_BARS ; ++i ) {
            init_mmioRegion( gboards[this_board].pdev, &gboards[this_board].bar[i], i, 0x00);
            init_mmioRegion( gboards[this_board].pdev, &gboards[this_board].bar[i], i, 0x01);
         } /* end of bar loop */
      } /* end of if (vendor_id == ... ) selection criteria */
   } /* end of for (dev = pacc->devices ... loop */


   /* put your code here */
   /* note that you have all the necessary information in boards array and buffers array */
   {
      for ( int board = 0; board < gnum_boards; ++board) {
         for ( int barnum = 0; barnum < NUM_BARS; ++barnum) {
            if ( gboards[board].bar[barnum].pbase) {
               /* bar exists, print first 4 fields */
               unsigned long long *ullbase = (unsigned long long *)gboards[board].bar[barnum].pbase;
               printf( "Board %d: Bar %d: WC %d\n", board, barnum, gboards[board].bar[barnum].flags);
               printf( "\t%08llx %08llx %08llx %08llx\n",
                       *(ullbase), *(ullbase+1), *(ullbase+2),  *(ullbase+3)
                       );
            }
         } /* for each bar */
      } /* for each board */

   }


exit1:   /* clean up open files in bars */
   for ( int board = 0; board < gnum_boards; ++board) {
      for ( int barnum = 0; barnum < NUM_BARS; ++barnum) {
            close_and_clean_mmioRegion( &gboards[board].bar[barnum] );
            close_and_clean_mmioRegion( &gboards[board].wc_bar[barnum] );
      } /* for each bar */
   } /* for each board */

exit0:   /* Normal exit */
   pci_cleanup(pacc); /* Close everything */
   return retval;
}



/// @brief           Initialize a bar region.
/// @param[in]       pdev is the device pointer from pciutils call
/// @param[in/out]   pbar points to the mmio region struct to be initialized. It will be cleared.
///                     Then flags will be copied in, the file opened if possible. If not, then
///                     fd will be 0, pbase will be zero, len will be zero, filename will be null.
/// @param[in]       numbar is the number of the mmio region to open
/// @param[in]       flags. 0x01 means open the write-combining region, else open un-cached region.
/// @return          0 if clean open or file not found; other error. 1 if filename too long.
///                     2 if could not open. 3 if could not mmap.
int init_mmioRegion( struct pci_dev *pdev, struct mmioRegion *pbar, int numbar, unsigned flags)
{
   int c = 0;
   int retval = 0;
   struct stat fstatus;

   memset( pbar, 0, sizeof( *pbar ) );
   pbar->flags = flags;                                     /* pbar->flags have been initialized */

   c = snprintf( pbar->filename, sizeof( pbar->filename),   /* pbar->filename has been initialized */
             flags & mmioRegion_flags_is_wc ?
                "/sys/bus/pci/devices/%04x:%02x:%02x.%d/resource%d_wc" :
                "/sys/bus/pci/devices/%04x:%02x:%02x.%d/resource%d",
             pdev->domain, pdev->bus, pdev->dev, pdev->func, numbar);

   if (c < 0 || c >= sizeof( pbar->filename )) {
      fprintf( stderr, "Filename string too long for resource %d, %s.\n",
         numbar,
         flags & mmioRegion_flags_is_wc ?
                         "write-combining" :
                         "un-cached"
      );
      retval = 1;
      goto exit_bad;
   }


   /* debug */
//   printf("resource file test name is %s\n", pbar->filename);

   if ( stat( pbar->filename, &fstatus) ) {   /* no file, clean up and get out */
      retval = 0;
      goto exit_bad;    /* not in fact an error return, but do want to clean up */
   }

   if(( pbar->fd = open( pbar->filename, O_RDWR | O_SYNC)) == -1) {
      fprintf( stderr, "File %s could not be opened, error: %s.\n",
         pbar->filename, strerror( errno ) );
      retval = 2;
      goto exit_bad;
   }                                                        /* pbar->fd has been initialized */

   /* Map the whole thing */
   pbar->len = pdev->size[numbar];                          /* pbar->len has been initialized */
   pbar->pbase = mmap( 0, pbar->len, PROT_READ | PROT_WRITE, MAP_SHARED, pbar->fd, 0 );
   if( ((void *)-1) == pbar->pbase || NULL == pbar->pbase ) {
      fprintf( stderr, "File %s could not be mapped, error: %s.\n",
         pbar->filename, strerror( errno ) );
      close( pbar->fd );
      retval = 3;
      goto exit_bad;
   }                                                        /* pbar->pbase has been initialized */

   return 0;                                                /* good return */

exit_bad:
   memset( pbar, 0, sizeof( *pbar ) );                      /* re-initialize */
   return retval;
} /* init_mmioRegion */


/// @brief           Close out a bar region opened by init_mmioRegion() and re-initialize the structure.
/// @param[in/out]   pmmio points to region to be cleaned up. Will be returned in clean state.
///                     Correctly handles situation where MMIO region was never opened in the first place.
///                     Does expect fields of unopened structure to be zeroed out.
/// @return          0x01 flag means could not unmap(). 0x02 flag means could not close().
int close_and_clean_mmioRegion( struct mmioRegion *pmmio)
{
   int retval = 0;
   /* if mapped, unmap */
   if ( pmmio->pbase ) {
      if ( munmap( pmmio->pbase, pmmio->len) ) {
         fprintf( stderr, "File %s did not unmap(), error message: %s\n", pmmio->filename, strerror( errno ) );
         retval |= 1;
      }
      pmmio->pbase = 0;
      pmmio->len = 0;
   }
   /* if open, close */
   if ( pmmio->fd > 0) {
      if ( close( pmmio->fd ) ) {
         fprintf( stderr, "File %s did not close(), error message: %s\n", pmmio->filename, strerror( errno ) );
         retval |= 2;
      }
      pmmio->fd = 0;
   }
   memset( pmmio->filename, 0, sizeof( pmmio->filename ) );
   pmmio->flags = 0;
   return retval;
} /* close_and_clean_mmioRegion */






















