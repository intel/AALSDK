// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

TEST(OSAL, aal0800)
{
   // When libOSAL is opened via OSServiceModuleOpen(), the value queried by the
   // standard entry point command C matches the value created by the configure.ac
   // macro M, where {C, M} are {AAL_SVC_CMD_VER_STR, OSAL_VERSION},
   // {AAL_SVC_CMD_VER_CURRENT, OSAL_VERSION_CURRENT},
   // {AAL_SVC_CMD_VER_REVISION, OSAL_VERSION_REVISION},
   // {AAL_SVC_CMD_VER_AGE, OSAL_VERSION_AGE}.

#if   defined( __AAL_LINUX__ )
   std::string lt_objdir(LT_OBJDIR);
   if ( ( lt_objdir.length() > 0 ) && ( '/' == lt_objdir[lt_objdir.length() - 1] ) ) {
      lt_objdir = lt_objdir.substr(0, lt_objdir.length() - 1);
   }

   std::string path(GlobalTestConfig::GetInstance().Vpath() +
                    std::string("/aas/OSAL/")               +
                    lt_objdir);

   //std::cerr << "Before, LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

   EXPECT_EQ(0, RequireLD_LIBRARY_PATH(path.c_str()));
   //std::cerr << "During, LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

#elif defined( __AAL_WINDOWS__ )
# error Implement path resolution for Windows.
#endif // OS

   OSServiceModule mod;

   OSServiceModuleInit(&mod, "libOSAL");

   if ( 0 != OSServiceModuleOpen(&mod) ) {
#if   defined( __AAL_LINUX__ )
      UnRequireLD_LIBRARY_PATH(path.c_str());
#endif // OS
      FAIL() << "couldn't open libOSAL";
   }

   btUnsigned32bitInt cur = 0;
   btUnsigned32bitInt rev = 0;
   btUnsigned32bitInt age = 0;

   char VerStr[AAL_SVC_MOD_VER_STR_MAX + 1] = { 0, };

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_CURRENT, (AAL::btAny)&cur));
   EXPECT_EQ(OSAL_VERSION_CURRENT, cur);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_REVISION, (AAL::btAny)&rev));
   EXPECT_EQ(OSAL_VERSION_REVISION, rev);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_AGE, (AAL::btAny)&age));
   EXPECT_EQ(OSAL_VERSION_AGE, age);

   EXPECT_EQ(0, mod.entry_point_fn(AAL_SVC_CMD_VER_STR, (AAL::btAny)VerStr));
   EXPECT_STREQ(OSAL_VERSION, VerStr);

   EXPECT_EQ(0, OSServiceModuleClose(&mod));

#if   defined( __AAL_LINUX__ )

   EXPECT_EQ(0, UnRequireLD_LIBRARY_PATH(path.c_str()));
   //std::cerr << "After,  LD_LIBRARY_PATH=" << LD_LIBRARY_PATH << std::endl;

#elif defined( __AAL_WINDOWS__ )
# error Implement path resolution for Windows.
#endif // OS
}

