/*
 * gtEnvVar.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: sadrutac
 */
#include "aalsdk/osal/Env.h"

class EnvVarBasic : public ::testing::Test
{
protected:
	EnvVarBasic() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
	Environment m_env;
};


TEST_F(EnvVarBasic, SetNewEnvVar /*TODO rename test case to test number per the wiki*/ )
{
   // Writes an Environment variable and verifies it.


   // TODO Comment describing your test methodology.


#if   defined( __AAL_LINUX__ )

   // TODO If my_var exists before executing the test, then the test will fail.
   // Instead of failing the test here, don't use an ASSERT_ or EXPECT_. Check
   //  for the existence of the variable and select another name until you find one
   //  that doesn't exist.

   ASSERT_TRUE(getenv("my_var") == NULL) << "my_var already exists\n";

#elif defined( __AAL_WINDOWS__ )

   ASSERT_EQ(GetEnvironmentVariable("my_var", NULL,0), 0) << "my_var already exists\n"

#endif

	EXPECT_TRUE(m_env.Set("my_var", "my_val"));

#if defined( __AAL_LINUX__ )

   EXPECT_STREQ(getenv("my_var"), "my_val");

#elif defined( __AAL_WINDOWS__ )

   dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
   EXPECT_STREQ(buff, "my_val");

#endif
}

TEST_F(EnvVarBasic, GetEnvVar /*TODO rename test case to test number per the wiki*/ )
{
   // Reads an Environment variable


   // TODO Comment describing your test methodology.


#if defined( __AAL_LINUX__ )

   // TODO don't assume that my_var will exist prior to this test case. That won't
   // always be the case, particularly when this case is run in isolation (--gtest_filter=*GetEnvVar*).

   ASSERT_STREQ(getenv("my_var"), "my_val");

#elif defined( __AAL_WINDOWS__ )

  dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
  ASSERT_STREQ(buff, "my_val");

#endif

	EXPECT_STREQ("my_val", m_env.Get("my_var"));
}

TEST_F(EnvVarBasic, OverwriteEnvVar /*TODO rename test case to test number per the wiki*/ )
{
   // Overwrites an existing environment variable with overwrite flag set to true.
   // Verifies that the env var has the new value.



   // TODO Comment describing your test methodology.


#if defined( __AAL_LINUX__ )

   // TODO don't assume that my_var will exist. Not always the case (--gtest_filter=*OverwriteEnvVar*).
   // Instead, use the OS-specific SDK to set a value to something you expect.

        ASSERT_FALSE(getenv("my_var") == NULL) << "my_var does not exist\n";

#elif defined( __AAL_WINDOWS__ )
        ASSERT_NE(GetEnvironmentVariable("my_var", NULL,0), 0) << "my_var does not exist\n"

#endif

	EXPECT_TRUE(m_env.Set("my_var", "new_val", true));
	EXPECT_STREQ("new_val", m_env.Get("my_var"));

#if defined( __AAL_LINUX__ )
        EXPECT_STREQ(getenv("my_var"), "new_val");

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
        EXPECT_STREQ(buff, "new_val");
#endif
}


TEST_F(EnvVarBasic, NoOverwriteEnvVar /*TODO rename test case to test number per the wiki*/ )
{
   //Overwrites an existing environment variable with overwrite flag set to false.
   //Verifies that the env var has the original value.


   // TODO Comment describing your test methodology.


   // TODO don't assume that my_var exists.

	EXPECT_TRUE(m_env.Set("my_var", "initial_val", true));

#if defined( __AAL_LINUX__ )
		ASSERT_STREQ(getenv("my_var"), "initial_val") << "my_var failed to initialise \n";

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
		ASSERT_STREQ(buff, "initial_val") << "my_var failed to initialise \n";
#endif

	EXPECT_TRUE(m_env.Set("my_var", "another_val", false));
	EXPECT_STRNE("another_val", m_env.Get("my_var"));
	EXPECT_STREQ("initial_val", m_env.Get("my_var"));

#if defined( __AAL_LINUX__ )
		EXPECT_STREQ(getenv("my_var"), "initial_val");

#elif defined( __AAL_WINDOWS__ )
		bufsize = GetEnvironmentVariable("my_var", buff, bufsize);
		EXPECT_STREQ(buff, "initial_val") << "my_var failed to initialise \n";
#endif
}

TEST_F(EnvVarBasic, GetNonExistingEnvVar /*TODO rename test case to test number per the wiki*/ )
{
   // Accesses a non-existing environment variable


   // TODO Comment describing your test methodology.


#if defined( __AAL_LINUX__ )

   // TODO Don't fail here is variable does exist - try another variable name.

	ASSERT_TRUE(getenv("absent_var") == NULL) << "absent_var already exists\n";

#elif defined( __AAL_WINDOWS__ )
	ASSERT_EQ(GetEnvironmentVariable("absent_var", NULL,0), 0) << "absent_var already exists\n";

#endif

	EXPECT_TRUE(m_env.Get("absent_var") == NULL);
}

