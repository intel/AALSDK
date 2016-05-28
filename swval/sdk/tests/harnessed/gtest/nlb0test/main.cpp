// INTEL CONFIDENTIAL - For Intel Internal Use Only

// generic Google Test application main().
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

static const char *gAppName    = "nlb0test";
static const char *gAppVersion = "0.0.0";

void Version()
{
   std::cout << gAppName << " " << gAppVersion << std::endl;
}

void Help()
{
   std::cout << gAppName << " --vpath=<path> [--halt-on-segv] [--halt-on-timeout] [--no-timeout] [--version] [--help]" << std::endl
             << "\t" << "--halt-on-segv    : enter a wait loop on memory access violation."         << std::endl
             << "\t" << "--halt-on-timeout : enter a wait loop on keep-alive timeout."              << std::endl
             << "\t" << "--no-timeout      : disable keep-alive timeout."                           << std::endl
             << "\t" << "--vpath=<path>    : Sets the root of the configured build tree to <path>." << std::endl
             << std::endl;
}

int main(int argc, char *argv[])
{
   int    i;
   btBool KeepAlive = true;

   for ( i = 1 ; i < argc ; ++i ) {
      std::string arg(argv[i]);

      if ( 0 == arg.compare("--version") ) {
         Version();
         return 0;
      } else if ( 0 == arg.compare("--help") ) {
         Help();
         ::testing::InitGoogleTest(&argc, argv);
         return 0;
      } else if ( 0 == arg.compare("--halt-on-segv") ) {
         TestStatus::HaltOnSegFault(true);
      } else if ( 0 == arg.compare("--halt-on-timeout") ) {
         TestStatus::HaltOnKeepaliveTimeout(true);
      } else if ( 0 == arg.compare("--no-timeout") ) {
         KeepAlive = false;
      } else if ( ( 0 == arg.compare(0, 8, "--vpath=") ) &&
                  ( arg.length() > 8 ) ) {
         GlobalTestConfig::GetInstance().Vpath(arg.substr(8));
      }
   }

   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGSEGV);
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGINT);
#ifdef __AAL_LINUX__
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGIO);
#endif // __AAL_LINUX__
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGUSR1);
   SignalHelper::GetInstance().Install(SignalHelper::IDX_SIGUSR2);

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


