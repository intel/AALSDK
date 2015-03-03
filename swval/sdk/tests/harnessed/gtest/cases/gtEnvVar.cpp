/*
 * gtEnvVar.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: sadrutac
 */
#include "aalsdk/osal/Env.h"

#if defined( __AAL_WINDOWS__ )
DWORD bufsize = 4096;
DWORD dwRet;
LPTSTR buff = (LPSTR) malloc(bufsize);
#endif

class EnvVarBasic : public ::testing::Test
{
protected:
	EnvVarBasic() {}
// virtual void SetUp() { }
// virtual void TearDown() { }
	Environment          env;
};

//Writes an Environment variable and verifies it.
TEST_F(EnvVarBasic, SetNewEnvVar)
{
#if defined( __AAL_LINUX__ )
        ASSERT_TRUE(getenv("my_var") == NULL) << "my_var already exists\n";

#elif defined( __AAL_WINDOWS__ )
        ASSERT_EQ(GetEnvironmentVariable("my_var", NULL,0), 0) << "my_var already exists\n"

#endif

	EXPECT_TRUE(env.setEnv("my_var", "my_val"));

#if defined( __AAL_LINUX__ )
        EXPECT_STREQ(getenv("my_var"), "my_val");

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
        EXPECT_STREQ(buff, "my_val");

#endif
}

//Accesses an Environment variable
TEST_F(EnvVarBasic, GetEnvVar)
{
#if defined( __AAL_LINUX__ )
        ASSERT_STREQ(getenv("my_var"), "my_val");

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
        ASSERT_STREQ(buff, "my_val");

#endif
	EXPECT_STREQ("my_val", env.getVal("my_var"));
}

//Overwrites an existing environment variable with overwrite flag set to true.
//Verifies that the env var has the new value.
TEST_F(EnvVarBasic, OverwriteEnvVar)
{
#if defined( __AAL_LINUX__ )
        ASSERT_FALSE(getenv("my_var") == NULL) << "my_var does not exist\n";

#elif defined( __AAL_WINDOWS__ )
        ASSERT_NE(GetEnvironmentVariable("my_var", NULL,0), 0) << "my_var does not exist\n"

#endif

	EXPECT_TRUE(env.setEnv("my_var", "new_val", true));
	EXPECT_STREQ("new_val", env.getVal("my_var"));

#if defined( __AAL_LINUX__ )
        EXPECT_STREQ(getenv("my_var"), "new_val");

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
        EXPECT_STREQ(buff, "new_val");
#endif
}

//Overwrites an existing environment variable with overwrite flag set to false.
//Verifies that the env var has the original value.
TEST_F(EnvVarBasic, NoOverwriteEnvVar)
{
	EXPECT_TRUE(env.setEnv("my_var", "initial_val", true));

#if defined( __AAL_LINUX__ )
		ASSERT_STREQ(getenv("my_var"), "initial_val") << "my_var failed to initialise \n";

#elif defined( __AAL_WINDOWS__ )
        dwRet = GetEnvironmentVariable("my_var", buff, bufsize);
		ASSERT_STREQ(buff, "initial_val") << "my_var failed to initialise \n";
#endif

	EXPECT_TRUE(env.setEnv("my_var", "another_val", false));
	EXPECT_STRNE("another_val", env.getVal("my_var"));
	EXPECT_STREQ("initial_val", env.getVal("my_var"));

#if defined( __AAL_LINUX__ )
		EXPECT_STREQ(getenv("my_var"), "initial_val");

#elif defined( __AAL_WINDOWS__ )
		bufsize = GetEnvironmentVariable("my_var", buff, bufsize);
		EXPECT_STREQ(buff, "initial_val") << "my_var failed to initialise \n";
#endif
}

//Accesses a non-existing environment variable
TEST_F(EnvVarBasic, GetNonExistingEnvVar)
{
#if defined( __AAL_LINUX__ )
	ASSERT_TRUE(getenv("absent_var") == NULL) << "absent_var already exists\n";

#elif defined( __AAL_WINDOWS__ )
	ASSERT_EQ(GetEnvironmentVariable("absent_var", NULL,0), 0) << "absent_var already exists\n";

#endif

	EXPECT_TRUE(env.getVal("absent_var") == NULL);
}
