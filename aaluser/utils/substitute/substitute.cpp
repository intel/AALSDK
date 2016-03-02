#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <Windows.h>

int  substitule_string(char* fpInput, char* fpOutput, char *inputStr, char* subStr);
int main(int argc, char *argv[])
{
   int    res   = 0;
   char   temp_file[512] = { 0 };
  
   if ( argc < 5 ) {
      printf("\n");
      printf("Usage: substitute template_file output_file marker text\n");
      printf("\n");
      printf("\tOpen template_file and read it line-by-line, replacing each instance of marker with text.\n");
      printf("\tWrite the result to output_file, leaving template_file unchanged.\n");
      printf("\n");
      return 1;
   }

   strcpy(temp_file, argv[1]);
   strcat(temp_file, "temp");
   CopyFile(argv[1], temp_file, 0);

   for ( int i = 3; (i+2) <= argc; i=i + 2)
   {
      substitule_string(temp_file, argv[2], argv[i], argv[i + 1]);
      CopyFile(argv[2], temp_file, 0);
   }
   remove(temp_file);

   return res;
}


int  substitule_string(char* fpInput, char* fpOutput, char *inputStr, char* subStr)
{
   int    res = 0;
   FILE  *fpIn = NULL;
   FILE  *fpOut = NULL;
   char   line[512] = {0};
   char   blank[512] = { 0};
   char   replacement[sizeof(line)] = {0};;
   char  *marker;
   char  *prev;
   char  *rep;
   size_t line_len;
   size_t marker_len;
   size_t text_len;

   fpIn = fopen(fpInput, "r");
   if (NULL == fpIn) {
      printf("Not found: %s\n", fpInput);
      res = 2;
      goto DONE;
   }
 
   fpOut = fopen(fpOutput, "w");
   if (NULL == fpOut) {
      printf("Failed to create: %s\n", fpOutput);
      res = 3;
      goto DONE;
   }

   prev = fgets(line, sizeof(line), fpIn);
     
   marker_len = strlen(inputStr);
   text_len = strlen(subStr);

   while (!feof(fpIn)) {

      prev = fgets(line, sizeof(line), fpIn);

      if (NULL == prev) {
         break;
      }

      if (ferror(fpIn)) {
         printf("Error reading: %s\n", fpInput);
         res = 5;
         goto DONE;
      }

      line_len = strlen(line);

      rep = replacement;
      marker = strstr(prev, inputStr);
     
      while (NULL != marker) {
         strncpy(rep, prev, marker - prev);
         rep += marker - prev;
         line_len -= marker - prev;

         strncpy(rep, subStr, text_len);
         rep += text_len;

         prev = marker + marker_len;
         line_len -= marker_len;

         if (0 == line_len) {
            break;
         }

         marker = strstr(prev, inputStr);
      } // End of while 

      strncpy(rep, prev, line_len + 1);

      res = fputs(replacement, fpOut);

      if ((0 != res) || ferror(fpOut)) {
         printf("Error writing: %s\n", fpOutput);
         res = 6;
         goto DONE;
      }
   } // End of while

DONE:
   if (NULL != fpIn) {
      fclose(fpIn);
   }

   if (NULL != fpOut) {
      fclose(fpOut);
   }

   return res;
}
