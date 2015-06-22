// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "isolated.h"

void SelfTest()
{
   //ASSERT_TRUE(false);
   EXPECT_TRUE(false);
   //ASSERT_FALSE(true);
   EXPECT_FALSE(true);

   //ASSERT_NULL((void *)1);
   EXPECT_NULL((void *)1);
   //ASSERT_NONNULL((void *)0);
   EXPECT_NONNULL((void *)0);

   //ASSERT_EQ(0, 1);
   EXPECT_EQ(0, 1);
   //ASSERT_NE(2, 2);
   EXPECT_NE(2, 2);
}

void AssertFailed()
{
   exit(99);
}

static int gErrors = 0;
void ExpectFailed()
{
   ++gErrors;
}

void Execute(ITestFixture *f)
{
   f->SetUp();
   f->Run();
   f->TearDown();
}

#include "aal0014.h"
#include "aal0017.h"


int main(int argc, char* argv[])
{
   //SelfTest();

   int i;
   for ( i = 1 ; i < argc ; ++i ) {
      if ( 0 == std::string(argv[i]).compare("14") ) {
         aal0014();
      } else if ( 0 == std::string(argv[i]).compare("17") ) {
         aal0017();
      }
   }

   return gErrors;
}


