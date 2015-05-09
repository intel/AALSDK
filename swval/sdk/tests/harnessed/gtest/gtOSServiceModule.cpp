// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "swvalmod.h"
#include "swvalsvcmod.h"

TEST(OSAL_OSServiceModule, aal0203)
{
   // OSServiceModuleInit() initializes the OSServiceModule structure according to the DLL
   // mechanisms and conventions of the current OS.

   std::string mod_root("mymod");
   std::string s;

   OSServiceModule m;

   OSServiceModuleInit(&m, mod_root.c_str());

   EXPECT_NULL(m.handle);
   EXPECT_STREQ(mod_root.c_str(), m.root_name);

   s = mod_root + std::string(AAL_SVC_MOD_EXT);
   EXPECT_STREQ(s.c_str(), m.full_name);

   s = mod_root + std::string(AAL_SVC_MOD_ENTRY_SUFFIX);
   EXPECT_STREQ(s.c_str(), m.entry_point_name);
   EXPECT_STREQ("AALSvcMod", AAL_SVC_MOD_ENTRY_SUFFIX);

#if   defined( __AAL_LINUX__ )

   s = mod_root + std::string(".so");
   EXPECT_STREQ(s.c_str(), m.full_name);

#elif defined( __AAL_WINDOWS__ )

   s = mod_root + std::string(".dll");
   EXPECT_STREQ(s.c_str(), m.full_name);

#endif // OS

   EXPECT_NULL(m.entry_point_fn);
   EXPECT_NULL(m.error_msg);
}

TEST(OSAL_OSServiceModule, aal0204)
{
   // When an OSServiceModule is found to already be open or uninitialized, as determined by a
   // non-NULL entry_point_fn field, OSServiceModuleOpen() fails with return code 1.

   OSServiceModule m;

   m.entry_point_fn = (AALSvcEntryPoint) 0xdeadbeef;

   EXPECT_EQ(1, OSServiceModuleOpen(&m));
   EXPECT_NONNULL(m.error_msg);
}

TEST(OSAL_OSServiceModule, aal0205)
{
   // When the given load library cannot be found, OSServiceModuleOpen() fails with return code 2.

   OSServiceModule m;

   OSServiceModuleInit(&m, "libnonexist");

   EXPECT_NULL(m.error_msg);
   EXPECT_EQ(2, OSServiceModuleOpen(&m));
   EXPECT_NONNULL(m.error_msg);
}

TEST(OSAL_OSServiceModule, aal0206)
{
   // When the module entry point cannot be found within the load library, OSServiceModuleOpen()
   // fails with return code 3.

   OSServiceModule m;

   OSServiceModuleInit(&m, "libswvalmod");

   // Corrupt entry_point_name so that it's not found within the module.
   strncpy(m.entry_point_name, "junk", 4);

   EXPECT_NULL(m.error_msg);
   EXPECT_EQ(3, OSServiceModuleOpen(&m));
   EXPECT_NONNULL(m.error_msg);
}

TEST(OSAL_OSServiceModule, aal0207)
{
   // When successful, OSServiceModuleOpen() returns 0. Upon return, the module will be open and
   // its entry point resolved.

   OSServiceModule m;

   OSServiceModuleInit(&m, "libswvalmod");

   EXPECT_EQ(0, OSServiceModuleOpen(&m));
   ASSERT_NONNULL(m.entry_point_fn);

   char ver[AAL_SVC_MOD_VER_STR_MAX];

   AAL::btUnsigned32bitInt cur = 0;
   AAL::btUnsigned32bitInt rev = 0;
   AAL::btUnsigned32bitInt age = 0;

   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_STR,       ver));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_CURRENT,  &cur));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_REVISION, &rev));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_AGE,      &age));

   EXPECT_STREQ(SWVALMOD_VERSION,       ver);
   EXPECT_STREQ(SWVALMOD_VERSION,       "3.2.1");

   EXPECT_EQ(SWVALMOD_VERSION_CURRENT,  cur);
   EXPECT_EQ(SWVALMOD_VERSION_CURRENT,  3);

   EXPECT_EQ(SWVALMOD_VERSION_REVISION, rev);
   EXPECT_EQ(SWVALMOD_VERSION_REVISION, 2);

   EXPECT_EQ(SWVALMOD_VERSION_AGE,      age);
   EXPECT_EQ(SWVALMOD_VERSION_AGE,      1);

   char hello[6] = { 0, 0, 0, 0, 0, 0 };
   EXPECT_EQ(0, m.entry_point_fn(SWVALMOD_CMD_SAY_HELLO, hello));
   EXPECT_STREQ("hello", hello);

   swvalmod_obj *obj = NULL;
   EXPECT_EQ(0, m.entry_point_fn(SWVALMOD_CMD_MALLOC_OBJ, &obj));
   ASSERT_NONNULL(obj);

   EXPECT_EQ(3, obj->intfield);
   EXPECT_STREQ("Message from libswvalmod.", obj->msg);
   EXPECT_STREQ("This variable is in the mod.", obj->something_in_the_mod);

   free(obj);

   obj = (swvalmod_obj *) malloc(sizeof(swvalmod_obj));
   ASSERT_NONNULL(obj);

   const char *in_the_app = "In the app.";

   obj->intfield = 4;
   strncpy(obj->msg, "goodbye", 7);
   obj->something_in_the_mod = in_the_app;

   EXPECT_EQ(0, m.entry_point_fn(SWVALMOD_CMD_FREE, obj));

   EXPECT_EQ(0, OSServiceModuleClose(&m));
}

TEST(OSAL_OSServiceModule, aal0208)
{
   // (Service Module) When successful, OSServiceModuleOpen() returns 0. Upon return, the
   // module will be open and its entry point resolved.

   OSServiceModule m;

   OSServiceModuleInit(&m, "libswvalsvcmod");

   EXPECT_EQ(0, OSServiceModuleOpen(&m));
   ASSERT_NONNULL(m.entry_point_fn);

   char ver[AAL_SVC_MOD_VER_STR_MAX];

   AAL::btUnsigned32bitInt cur = 0;
   AAL::btUnsigned32bitInt rev = 0;
   AAL::btUnsigned32bitInt age = 0;

   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_STR,       ver));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_CURRENT,  &cur));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_REVISION, &rev));
   EXPECT_EQ(0, m.entry_point_fn(AAL_SVC_CMD_VER_AGE,      &age));

   EXPECT_STREQ(SWVALSVCMOD_VERSION,       ver);
   EXPECT_STREQ(SWVALSVCMOD_VERSION,       "6.5.4");

   EXPECT_EQ(SWVALSVCMOD_VERSION_CURRENT,  cur);
   EXPECT_EQ(SWVALSVCMOD_VERSION_CURRENT,  6);

   EXPECT_EQ(SWVALSVCMOD_VERSION_REVISION, rev);
   EXPECT_EQ(SWVALSVCMOD_VERSION_REVISION, 5);

   EXPECT_EQ(SWVALSVCMOD_VERSION_AGE,      age);
   EXPECT_EQ(SWVALSVCMOD_VERSION_AGE,      4);

   char hello[6] = { 0, 0, 0, 0, 0, 0 };
   EXPECT_EQ(0, m.entry_point_fn(SWVALSVCMOD_CMD_SAY_HELLO, hello));
   EXPECT_STREQ("hello", hello);

   swvalsvcmod_obj *obj = NULL;
   EXPECT_EQ(0, m.entry_point_fn(SWVALSVCMOD_CMD_MALLOC_STRUCT, &obj));
   ASSERT_NONNULL(obj);

   EXPECT_STREQ("This variable is in the service mod.", obj->something_in_the_mod);
   EXPECT_STREQ("Message from libswvalsvcmod.", obj->msg);
   EXPECT_EQ(7, obj->intfield);

   free(obj);

   obj = (swvalsvcmod_obj *) malloc(sizeof(swvalsvcmod_obj));
   ASSERT_NONNULL(obj);

   const char *in_the_app = "In the test.";

   obj->something_in_the_mod = in_the_app;
   strncpy(obj->msg, "abc", 3);
   obj->intfield = 6;

   EXPECT_EQ(0, m.entry_point_fn(SWVALSVCMOD_CMD_FREE, obj));

   EXPECT_EQ(0, OSServiceModuleClose(&m));
}


