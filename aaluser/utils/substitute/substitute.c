#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
   int    res   = 0;
   FILE  *fpIn  = NULL;
   FILE  *fpOut = NULL;
   char   line[512];
   char   replacement[sizeof(line)];
   char  *marker;
   char  *prev;
   char  *rep;
   size_t line_len;
   size_t marker_len;
   size_t text_len;

   if ( argc < 5 ) {
      printf("\n");
      printf("Usage: substitute template_file output_file marker text\n");
      printf("\n");
      printf("\tOpen template_file and read it line-by-line, replacing each instance of marker with text.\n");
      printf("\tWrite the result to output_file, leaving template_file unchanged.\n");
      printf("\n");
      return 1;
   }

   fpIn = fopen(argv[1], "r");
   if ( NULL == fpIn ) {
      printf("Not found: %s\n", argv[1]);
      res = 2;
      goto DONE;
   }

   fpOut = fopen(argv[2], "w");
   if ( NULL == fpOut ) {
      printf("Failed to create: %s\n", argv[2]);
      res = 3;
      goto DONE;
   }

   marker_len = strlen(argv[3]);
   text_len   = strlen(argv[4]);

   while ( !feof(fpIn) ) {
      prev = fgets(line, sizeof(line), fpIn);

      if ( NULL == prev ) {
         break;
      }

      if ( ferror(fpIn) ) {
         printf("Error reading: %s\n", argv[1]);
         res = 5;
         goto DONE;
      }

      line_len = strlen(line);

      rep    = replacement;
      marker = strstr(prev, argv[3]);
      while ( NULL != marker ) {
         strncpy(rep, prev, marker - prev);
         rep      += marker - prev;
         line_len -= marker - prev;

         strncpy(rep, argv[4], text_len);
         rep += text_len;

         prev     = marker + marker_len;
         line_len -= marker_len;

         if ( 0 == line_len ) {
            break;
         }

         marker = strstr(prev, argv[3]);
      }

      strncpy(rep, prev, line_len + 1);

      res = fputs(replacement, fpOut);

      if ( ( 0 != res ) || ferror(fpOut) ) {
         printf("Error writing: %s\n", argv[2]);
         res = 6;
         goto DONE;
      }
   }

DONE:
   if ( NULL != fpIn ) {
      fclose(fpIn);
   }

   if ( NULL != fpOut ) {
      fclose(fpOut);
   }

   return res;
}

