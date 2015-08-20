// INTEL CONFIDENTIAL - For Intel Internal Use Only

// generic Google Test application main().
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

static const char *gAppName =
#if   defined( TEST_SUITE_BAT )
   "bat"
#elif defined( TEST_SUITE_NIGHTLY )
   "nightly"
#elif defined( TEST_SUITE_WEEKLY )
   "weekly"
#endif // test suite
;

static const char *gAppVersion =
#if   defined( TEST_SUITE_BAT )
   "1.1.1"
#elif defined( TEST_SUITE_NIGHTLY )
   "1.1.1"
#elif defined( TEST_SUITE_WEEKLY )
   "1.1.1"
#endif // test suite
;

void Version()
{
   std::cout << gAppName << " " << gAppVersion << std::endl;
}

void Help()
{
   std::cout << gAppName << " [--halt-on-segv] [--halt-on-timeout] [--no-timeout] [--version] [--help]" << std::endl
             << "\t" << "--halt-on-segv    : enter a wait loop on memory access violation."             << std::endl
             << "\t" << "--halt-on-timeout : enter a wait loop on keep-alive timeout."                  << std::endl
             << "\t" << "--no-timeout      : disabled keep-alive timeout."                              << std::endl
             << std::endl;
}

int main(int argc, char *argv[])
{
   int    i;
   btBool KeepAlive = true;

   for ( i = 1 ; i < argc ; ++i ) {
      if ( 0 == std::string(argv[i]).compare("--version") ) {
         Version();
         return 0;
      } else if ( 0 == std::string(argv[i]).compare("--help") ) {
         Help();
         ::testing::InitGoogleTest(&argc, argv);
         return 0;
      } else if ( 0 == std::string(argv[i]).compare("--halt-on-segv") ) {
         TestStatus::HaltOnSegFault(true);
      } else if ( 0 == std::string(argv[i]).compare("--halt-on-timeout") ) {
         TestStatus::HaltOnKeepaliveTimeout(true);
      } else if ( 0 == std::string(argv[i]).compare("--no-timeout") ) {
         KeepAlive = false;
      }
   }

   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGSEGV);
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGINT);
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGIO);
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGUSR1);

   ::testing::InitGoogleTest(&argc, argv);

   if ( KeepAlive ) {
      ::testing::UnitTest::GetInstance()->listeners().Append(new KeepAliveTestListener());
      ::testing::AddGlobalTestEnvironment(KeepAliveTimerEnv::GetInstance());
   }

   int res = RUN_ALL_TESTS();

   if ( 0 == res ) {
      TestStatus::Report(TestStatus::STATUS_PASS);
   } else {
      TestStatus::Report(TestStatus::STATUS_FAIL);
   }

   return res;
}

