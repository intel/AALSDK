// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_STATUS_H__
#define __GTCOMMON_STATUS_H__

// Uses the ConsoleColorizer to report the end status of the entire Google Test application.
// Supported end states for the application process are:
//   * Pass - all Google Test cases reported a passing result.
//   * Fail - some Google Test assertion reported a failing result.
//   * SegFault - a segmentation fault signal was received, forcing a test result of Fail.
//                The process exit status will be 97.
//   * Terminated - the user forcibly terminated (Ctrl-C) the test process, forcing a test
//                  result of Fail. The process exit status will be 98.
//   * KeepaliveTimeout - the keep-alive timer expired. The process is assumed to be stuck in
//                        infinite processing, so the test result is Fail. The process exit
//                        status will be 99.
class GTCOMMON_API TestStatus
{
public:
   enum Status
   {
      STATUS_PASS,
      STATUS_FAIL,
      STATUS_SEGFAULT,
      STATUS_TERMINATED,
      STATUS_KEEPALIVE_TIMEOUT
   };

   static void Report(Status st);

   static void HaltOnSegFault(bool );
   static void HaltOnKeepaliveTimeout(bool );

protected:
   static void OnPass();
   static void OnFail();
   static void OnSegFault();
   static void OnTerminated();
   static void OnKeepaliveTimeout();

   static bool sm_HaltOnSegFault;
   static bool sm_HaltOnKeepaliveTimeout;
};

#endif // __GTCOMMON_STATUS_H__

