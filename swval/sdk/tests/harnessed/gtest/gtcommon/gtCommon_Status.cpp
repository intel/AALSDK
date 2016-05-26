// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

bool TestStatus::sm_HaltOnSegFault         = false;
bool TestStatus::sm_HaltOnKeepaliveTimeout = false;

void TestStatus::HaltOnSegFault(bool b)
{
   TestStatus::sm_HaltOnSegFault = b;
}

void TestStatus::HaltOnKeepaliveTimeout(bool b)
{
   TestStatus::sm_HaltOnKeepaliveTimeout = b;
}

void TestStatus::Report(TestStatus::Status st)
{
   switch ( st ) {
      case STATUS_PASS :
         TestStatus::OnPass();
      break;

      case STATUS_FAIL :
         TestStatus::OnFail();
      break;

      case STATUS_SEGFAULT :
         TestStatus::OnSegFault();
      break;

      case TestStatus::STATUS_TERMINATED :
         TestStatus::OnTerminated();
      break;

      case STATUS_KEEPALIVE_TIMEOUT :
         TestStatus::OnKeepaliveTimeout();
      break;
   }
}

void TestStatus::OnPass()
{
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Green(ConsoleColorizer::STD_COUT);
   std::cout << "\nPASS\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Green(ConsoleColorizer::STD_CERR);
   std::cerr << "\nPASS\n";
   color.Reset(ConsoleColorizer::STD_CERR);
}

void TestStatus::OnFail()
{
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nFAIL\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nFAIL\n";
   color.Reset(ConsoleColorizer::STD_CERR);
}

void TestStatus::OnSegFault()
{
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nSegmentation Fault during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nSegmentation Fault during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_CERR);

   if ( TestStatus::sm_HaltOnSegFault ) {

      KeepAliveTimerEnv::GetInstance()->StopThread();

      int i = 0;
      while ( TestStatus::sm_HaltOnSegFault ) {
         if ( 0 == (i % (5 * 60)) ) {
            color.Blue(ConsoleColorizer::STD_CERR);
            std::cerr << "Halted for debugger attach.\n";
            color.Reset(ConsoleColorizer::STD_CERR);

            i = 0;
         }
         sleep_sec(1);
         ++i;
      }

   }

   ::exit(97);
}

void TestStatus::OnTerminated()
{
   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nProcess Terminated\n";
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nProcess Terminated\n";
   color.Reset(ConsoleColorizer::STD_CERR);

   ::exit(98);
}

void TestStatus::OnKeepaliveTimeout()
{
   std::string testcase;
   std::string test;

   TestCaseName(testcase, test);

   ConsoleColorizer & color = ConsoleColorizer::GetInstance();

   color.Red(ConsoleColorizer::STD_COUT);
   std::cout << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_COUT);

   color.Red(ConsoleColorizer::STD_CERR);
   std::cerr << "\nKeep-alive Timer Expired during " << testcase << "." << test << std::endl;
   color.Reset(ConsoleColorizer::STD_CERR);

   int i = 0;
   while ( TestStatus::sm_HaltOnKeepaliveTimeout ) {
      if ( 0 == (i % (5 * 60)) ) {
         color.Blue(ConsoleColorizer::STD_CERR);
         std::cerr << "Halted for debugger attach.\n";
         color.Reset(ConsoleColorizer::STD_CERR);

         i = 0;
      }
      sleep_sec(1);
      ++i;
   }

   ::exit(99);
}

