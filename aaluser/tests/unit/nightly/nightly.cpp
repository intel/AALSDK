// INTEL CONFIDENTIAL - for Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <stdio.h>
#include <fstream>

#include "gtest/gtest.h"

#define ASSERT_NONNULL(x) ASSERT_NE((void *)NULL, x)
#define ASSERT_NULL(x)    ASSERT_EQ((void *)NULL, x)
#define EXPECT_NONNULL(x) EXPECT_NE((void *)NULL, x)
#define EXPECT_NULL(x)    EXPECT_EQ((void *)NULL, x)

#include <aalsdk/AAL.h>
#include <aalsdk/xlRuntime.h>

const std::string SampleAFU1ConfigRecord("9 20 ConfigRecordIncluded\n \
      \t10\n \
          \t\t9 17 ServiceExecutable\n \
            \t\t\t9 13 libsampleafu1\n \
         \t\t9 18 _CreateSoftService\n \
         \t\t0 1\n \
   9 29 ---- End of embedded NVS ----\n \
      9999\n");


void TestCaseName(std::string &Test, std::string &TestCase);

#if __AAL_LINUX__
int RequireLD_LIBRARY_PATH(const char * );
int Require_NOKERNEL_Min_LD_LIBRARY_PATH();
std::ostream & LD_LIBRARY_PATH(std::ostream & );
#endif // __AAL_LINUX__


#include "NIGHTLY-TEST-LIST.h"

int main(int argc, char *argv[])
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}


void TestCaseName(std::string &Test, std::string &TestCase)
{
   const ::testing::TestInfo * const pInfo =
         ::testing::UnitTest::GetInstance()->current_test_info();

   Test     = std::string(pInfo->name());
   TestCase = std::string(pInfo->test_case_name());
}


#if __AAL_LINUX__
int RequireLD_LIBRARY_PATH(const char *path)
{
   int   res  = 1;
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL == pvar ) {
      // not found, so set it.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   char *pcopyvar  = strdup(pvar);
   char *psavecopy = pcopyvar;

   if ( NULL == pcopyvar ) {
      return res;
   }

   char *pcolon;
   while ( NULL != (pcolon = strchr(pcopyvar, ':')) ) {

      *pcolon = 0;

      if ( 0 == strcmp(pcopyvar, path) ) {
         // path already found in LD_LIBRARY_PATH
         res = 0;
         goto _DONE;
      }

      pcopyvar = pcolon + 1;

   }

   if ( 0 == strcmp(pcopyvar, path) ) {
      // path already found in LD_LIBRARY_PATH
      res = 0;
      goto _DONE;
   }

   // LD_LIBRARY_PATH exists, but does not contain path.

   free(psavecopy);

   if ( 0 == strcmp(pvar, "") ) {
      // LD_LIBRARY_PATH is defined, but empty.
      return setenv("LD_LIBRARY_PATH", path, 1);
   }

   psavecopy = (char *) malloc(strlen(pvar) + strlen(path) + 2);
   if ( NULL == psavecopy ) {
      return res;
   }

   sprintf(psavecopy, "%s:%s", pvar, path);

   res = setenv("LD_LIBRARY_PATH", psavecopy, 1);

_DONE:
   free(psavecopy);

   return res;
}

int Require_NOKERNEL_Min_LD_LIBRARY_PATH()
{
   const char *minpaths[] =
   {
      OSAL_LIBDIR,
      AAS_LIBDIR,
      XLRT_LIBDIR
   };

   int res;
   int i;

   for ( i = 0 ; i < sizeof(minpaths) / sizeof(minpaths[0]) ; ++i ) {
      res = RequireLD_LIBRARY_PATH(minpaths[i]);
      if ( 0 != res ) {
         return res;
      }
   }

   return res;
}

std::ostream & LD_LIBRARY_PATH(std::ostream &os)
{
   char *pvar = getenv("LD_LIBRARY_PATH");

   if ( NULL != pvar ) {
      os << pvar;
   }

   return os;
}
#endif // __AAL_LINUX__


