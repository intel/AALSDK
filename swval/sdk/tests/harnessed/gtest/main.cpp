// INTEL CONFIDENTIAL - For Intel Internal Use Only

// generic Google Test application main().
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#if   defined( TEST_SUITE_BAT )
void Version() { std::cout << "bat 1.1.1"     << std::endl; }
#elif defined( TEST_SUITE_NIGHTLY )
void Version() { std::cout << "nightly 1.1.1" << std::endl; }
#elif defined( TEST_SUITE_WEEKLY )
void Version() { std::cout << "weekly 1.1.1"  << std::endl; }
#endif // test suite

int main(int argc, char *argv[])
{
   if ( argc > 1 ) {
      if ( 0 == std::string(argv[1]).compare("--version") ) {
         Version();
         return 0;
      }
   }

   SignalHelper::GlobalInstance().Install(SIGSEGV, SignalHelper::SIGSEGVHandler, true);
   SignalHelper::GlobalInstance().Install(SIGINT,  SignalHelper::SIGINTHandler,  true);

   ::testing::InitGoogleTest(&argc, argv);

   ::testing::UnitTest::GetInstance()->listeners().Append(new KeepAliveTestListener());
   ::testing::AddGlobalTestEnvironment(KeepAliveTimerEnv::GetInstance());

   int res = RUN_ALL_TESTS();

   if ( 0 == res ) {
      TestStatus::Report(TestStatus::STATUS_PASS);
   } else {
      TestStatus::Report(TestStatus::STATUS_FAIL);
   }

   return res;
}

