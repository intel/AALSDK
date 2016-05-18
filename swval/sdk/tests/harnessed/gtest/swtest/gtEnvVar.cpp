/*
 * gtEnvVar.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: sadrutac
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#include "aalsdk/osal/Env.h"

class EnvVarBasic : public ::testing::Test
{
protected:
	EnvVarBasic() {}

   virtual void SetUp()
   {
      memset(m_Buf, 0, sizeof(m_Buf));
   }
   virtual void TearDown()
   {
      Environment::ReleaseObj();
   }

   char m_Buf[256];
};

TEST_F(EnvVarBasic, aal0043)
{
   // Writes an Environment variable and verifies it.

   char        EnvVar[20] = "my_var";
   std::string StrVar;
   int         randomNum;

   // Check for the existence of the variable and select another name until we find one
   // that doesn't exist.
#if   defined( __AAL_LINUX__ )

   while (getenv(EnvVar) != NULL)
   {
	   randomNum = rand()%100;
	   sprintf(EnvVar, "my_var_%d", randomNum);
   }

#elif defined( __AAL_WINDOWS__ )

   while (GetEnvironmentVariable(EnvVar, NULL,0) != 0)
   {
	   randomNum = rand();
	   sprintf_s(EnvVar, sizeof(EnvVar), "my_var_%d", randomNum);
   }

#endif // OS

   //Set the environment variable with a value
   StrVar.assign(EnvVar);
	EXPECT_TRUE(Environment::GetObj()->Set(StrVar, "my_val"));

	//Verify that the environment variable has the expected value.
	//Unset the environment variable after the verification.
#if   defined( __AAL_LINUX__ )

   EXPECT_STREQ(getenv(EnvVar), "my_val");
   unsetenv(EnvVar);
   EXPECT_TRUE(getenv(EnvVar) == NULL);

#elif defined( __AAL_WINDOWS__ )

   DWORD dwRet = GetEnvironmentVariable(EnvVar, m_Buf, sizeof(m_Buf));
   EXPECT_EQ(6, dwRet);
   EXPECT_STREQ(m_Buf, "my_val");

   // Remove the env var.
   EXPECT_NE(0, SetEnvironmentVariable(EnvVar, NULL));

   // Verify that it was removed.
   EXPECT_EQ(0, GetEnvironmentVariable(EnvVar, m_Buf, sizeof(m_Buf)));
   EXPECT_EQ(ERROR_ENVVAR_NOT_FOUND, GetLastError());

#endif // OS
}

TEST_F(EnvVarBasic, aal0044 )
{
   // Reads an Environment variable

	std::string retVal;

	//Set an environment variable with a value.
	//If the environment variable does not exist, a new one will be created
	//Verify that the environment variable has the expected value

#if   defined( __AAL_LINUX__ )

   ASSERT_TRUE(0 == setenv("my_var", "my_val", 1));
   ASSERT_STREQ(getenv("my_var"), "my_val");

#elif defined( __AAL_WINDOWS__ )

   ASSERT_NE(0, SetEnvironmentVariable("my_var", "my_val"));
   DWORD dwRet = GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf));
   EXPECT_EQ(6, dwRet);
   ASSERT_STREQ(m_Buf, "my_val");

#endif // OS

   //Verify that Get() returns the expected value.
   EXPECT_TRUE(Environment::GetObj()->Get("my_var", retVal));
   EXPECT_STREQ("my_val", retVal.c_str());

   //Unset the Environment variable.
#if   defined( __AAL_LINUX__ )

   unsetenv("my_var");
   EXPECT_TRUE(getenv("my_var") == NULL);

#elif defined( __AAL_WINDOWS__ )

   // Remove the env var.
   EXPECT_NE(0, SetEnvironmentVariable("my_var", NULL));

   // Verify that it was removed.
   EXPECT_EQ(0, GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf)));
   EXPECT_EQ(ERROR_ENVVAR_NOT_FOUND, GetLastError());

#endif // OS
}

TEST_F(EnvVarBasic, aal0045 )
{
   // Overwrites an existing environment variable with overwrite flag set to true.
   // Verifies that the env var has the new value.

	std::string retVal;

	//Set an environment variable with an initial value.
	//If the environment variable does not exist, a new one will be created.
	//Verify that the environment variable exists.
#if   defined( __AAL_LINUX__ )

	ASSERT_TRUE(0 == setenv("my_var", "initial_val", 1));
	ASSERT_FALSE(getenv("my_var") == NULL) << "my_var does not exist\n";

#elif defined( __AAL_WINDOWS__ )

	ASSERT_NE(0, SetEnvironmentVariable("my_var", "initial_val"));
	ASSERT_NE(GetEnvironmentVariable("my_var", NULL,0), 0) << "my_var does not exist\n";

#endif // OS

    //Overwrite the environment variable with a new value.
	EXPECT_TRUE(Environment::GetObj()->Set("my_var", "new_val", true));

	//Verify that the environment variable successfully overwritten.
	//Unset the environment variable after the verification.
#if   defined( __AAL_LINUX__ )

	EXPECT_STREQ(getenv("my_var"), "new_val");
	unsetenv("my_var");
   EXPECT_TRUE(getenv("my_var") == NULL);

#elif defined( __AAL_WINDOWS__ )

	DWORD dwRet = GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf));
	EXPECT_STREQ(m_Buf, "new_val");

   // Remove the env var.
   EXPECT_NE(0, SetEnvironmentVariable("my_var", NULL));

   // Verify that it was removed.
   EXPECT_EQ(0, GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf)));
   EXPECT_EQ(ERROR_ENVVAR_NOT_FOUND, GetLastError());

#endif // OS
}

TEST_F(EnvVarBasic, aal0046)
{
   //Overwrites an existing environment variable with overwrite flag set to false.
   //Verifies that the env var has the original value.

	std::string retVal;

	//Set an environment variable with an initial value.
	//If the environment variable does not exist, a new one will be created.
	//Verify that the environment variable is initialised.
#if   defined( __AAL_LINUX__ )

	ASSERT_TRUE(0 == setenv("my_var", "initial_val", 1));
	ASSERT_STREQ(getenv("my_var"), "initial_val") << "my_var failed to initialise \n";

#elif defined( __AAL_WINDOWS__ )

	ASSERT_NE(0, SetEnvironmentVariable("my_var", "initial_val"));
	DWORD dwRet = GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf));
   EXPECT_EQ(11, dwRet);

	ASSERT_STREQ(m_Buf, "initial_val") << "my_var failed to initialise\n";

#endif // OS

	//Overwrite the environment variable with a new value (overwrite flag is set to false).
	EXPECT_FALSE(Environment::GetObj()->Set("my_var", "another_val", false));

	//Verify that the environment variable was not overwritten.
	//Unset the Environment variable after the verification.
#if   defined( __AAL_LINUX__ )

	EXPECT_STREQ(getenv("my_var"), "initial_val");
	unsetenv("my_var");
	EXPECT_TRUE(getenv("my_var") == NULL);

#elif defined( __AAL_WINDOWS__ )

   memset(m_Buf, 0, sizeof(m_Buf));
	dwRet = GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf));
   EXPECT_EQ(11, dwRet);

	EXPECT_STREQ(m_Buf, "initial_val") << "my_var was overwritten when the'overwrite' flag was set to false\n";

   // Remove the env var.
   EXPECT_NE(0, SetEnvironmentVariable("my_var", NULL));

   // Verify that it was removed.
   EXPECT_EQ(0, GetEnvironmentVariable("my_var", m_Buf, sizeof(m_Buf)));
   EXPECT_EQ(ERROR_ENVVAR_NOT_FOUND, GetLastError());

#endif // OS
}

TEST_F(EnvVarBasic, aal0047 )
{
   // Accesses a non-existing environment variable

	char        EnvVar[20] = "absent_var";
	int         randomNum;
	std::string StrVar, retVal;

	 // Find a variable name that does not exist.
#if   defined( __AAL_LINUX__ )

	 while (getenv(EnvVar) != NULL)
    {
	    randomNum = rand()%100;
	    sprintf(EnvVar, "absent_var_%d", randomNum);
    }

#elif defined( __AAL_WINDOWS__ )

	 while (GetEnvironmentVariable(EnvVar, NULL,0) != 0)
	 {
	 	randomNum = rand();
	 	sprintf_s(EnvVar, sizeof(EnvVar), "absent_var_%d", randomNum);
	 }

#endif // 0

	//Verify that Get() return NULL when it is used to access a non-existing environment variable.
	 StrVar.assign(EnvVar);
	 EXPECT_FALSE(Environment::GetObj()->Get(StrVar, retVal));
}

TEST_F(EnvVarBasic, aal0131 )
{
   // Sends an empty environment variable to be fetched by Get()
	std::string retVal;

	//Verify that Get() return NULL when it is used to access a non-existing environment variable.
	 EXPECT_FALSE(Environment::GetObj()->Get("", retVal));
}

TEST_F(EnvVarBasic, aal0132)
{
   // Sends invalid arguments to Set()

	char        EnvVar[20] = "my_var";
	int         randomNum;
   std::string StrVar;

	 // Find a variable name that does not exist.
#if   defined( __AAL_LINUX__ )

	 while (getenv(EnvVar) != NULL)
    {
	    randomNum = rand()%100;
	    sprintf(EnvVar, "my_var_%d", randomNum);
    }

#elif defined( __AAL_WINDOWS__ )

	 while (GetEnvironmentVariable(EnvVar, NULL,0) != 0)
	 {
	 	randomNum = rand();
	 	sprintf_s(EnvVar, sizeof(EnvVar), "my_var_%d", randomNum);
	 }

#endif

   //Send an empty environment variable and an empty value as arguments to Set().
   EXPECT_FALSE(Environment::GetObj()->Set("", ""));

   //Send a valid environment variable and an empty value as arguments to Set().
   StrVar.assign(EnvVar);
   EXPECT_TRUE(Environment::GetObj()->Set(StrVar, ""));

	 //unset the environment variable
#if   defined( __AAL_LINUX__ )

   unsetenv(EnvVar);
   EXPECT_TRUE(getenv(EnvVar) == NULL);

#elif defined( __AAL_WINDOWS__ )

    // Remove the env var.
   EXPECT_NE(0, SetEnvironmentVariable(EnvVar, NULL));

   // Verify that it was removed.
   EXPECT_EQ(0, GetEnvironmentVariable(EnvVar, m_Buf, sizeof(m_Buf)));
   EXPECT_EQ(ERROR_ENVVAR_NOT_FOUND, GetLastError());

#endif
}
