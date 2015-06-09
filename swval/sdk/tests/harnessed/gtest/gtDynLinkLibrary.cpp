// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "swvalmod.h"
#include "swvalsvcmod.h"

TEST(OSAL_DynLinkLibrary, aal0209)
{
   // When DynLinkLibrary::DynLinkLibrary() cannot find the loadable library specified by
   // LibraryName parameter, DynLinkLibrary::IsOK() returns false, indicating the error.

   std::string LibName("libnonexist");

   LibName += std::string(AAL_SVC_MOD_EXT);

   DynLinkLibrary Lib(LibName);

   EXPECT_FALSE(Lib.IsOK());
}

TEST(OSAL_DynLinkLibrary, aal0210)
{
   // When DynLinkLibrary::DynLinkLibrary() cannot find the loadable library specified by
   // LibraryName parameter, DynLinkLibrary::GetSymAddress() returns NULL.

   std::string LibName("libnonexist");
   std::string SymName("somesymbol");

   LibName += std::string(AAL_SVC_MOD_EXT);

   DynLinkLibrary Lib(LibName);

   EXPECT_NULL(Lib.GetSymAddress(SymName));
}

TEST(OSAL_DynLinkLibrary, aal0211)
{
   // When the loadable library exists, but the SymbolName parameter given to
   // DynLinkLibrary::GetSymAddress() is not found, GetSymAddress() returns NULL.

   std::string LibName("libswvalmod");
   std::string SymName("somesymbol");

   LibName += std::string(AAL_SVC_MOD_EXT);

   DynLinkLibrary Lib(LibName);

   EXPECT_TRUE(Lib.IsOK());
   EXPECT_NULL(Lib.GetSymAddress(SymName));
}

TEST(OSAL_DynLinkLibrary, aal0212)
{
   // When the loadable library exists, and the SymbolName parameter given to
   // DynLinkLibrary::GetSymAddress() is found, GetSymAddress() returns the appropriate pointer.

   std::string LibName("libswvalmod");
   std::string SymName(LibName);

   LibName += std::string(AAL_SVC_MOD_EXT);
   SymName += std::string(AAL_SVC_MOD_ENTRY_SUFFIX);

   DynLinkLibrary Lib(LibName);

   ASSERT_TRUE(Lib.IsOK());

   AALSvcEntryPoint fn = (AALSvcEntryPoint) Lib.GetSymAddress(SymName);

   ASSERT_NONNULL(fn);

   AAL::btUnsigned32bitInt cur = 0;
   AAL::btUnsigned32bitInt rev = 0;
   AAL::btUnsigned32bitInt age = 0;

   EXPECT_EQ(0, fn(AAL_SVC_CMD_VER_CURRENT,  &cur));
   EXPECT_EQ(0, fn(AAL_SVC_CMD_VER_REVISION, &rev));
   EXPECT_EQ(0, fn(AAL_SVC_CMD_VER_AGE,      &age));

   EXPECT_EQ(SWVALMOD_VERSION_CURRENT,  cur);
   EXPECT_EQ(SWVALMOD_VERSION_CURRENT,  3);

   EXPECT_EQ(SWVALMOD_VERSION_REVISION, rev);
   EXPECT_EQ(SWVALMOD_VERSION_REVISION, 2);

   EXPECT_EQ(SWVALMOD_VERSION_AGE,      age);
   EXPECT_EQ(SWVALMOD_VERSION_AGE,      1);
}

TEST(OSAL_OSLib, aal0213)
{
   // libOSAL implements a default loadable module entry point.

   OSServiceModule m;

   OSServiceModuleInit(&m, "libOSAL");

   EXPECT_EQ(0, OSServiceModuleOpen(&m));
   ASSERT_NONNULL(m.entry_point_fn);

   AAL::btUnsigned32bitInt cur = 0;
   AAL::btUnsigned32bitInt rev = 0;
   AAL::btUnsigned32bitInt age = 0;

   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_CURRENT,  &cur));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_REVISION, &rev));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_AGE,      &age));

   EXPECT_EQ(0, OSServiceModuleClose(&m));
}

TEST(OSAL_OSLib, aal0214)
{
   // FindLowestBitSet64() returns the one-based index of the least significant bit equal to
   // one in the 64-bit input parameter, or zero if the input is zero.

   AAL::btUnsigned64bitInt u;
   unsigned long           i;

   u = 3;
   for ( i = 0 ; i < 64 ; ++i, u <<= 1 ) {
      EXPECT_EQ(i + 1, FindLowestBitSet64(u));
   }

   u = 0;
   EXPECT_EQ(0, FindLowestBitSet64(u));
}

