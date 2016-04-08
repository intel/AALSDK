// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "aalsdk/osal/Timer.h"
#include "aalsdk/osal/Sleep.h"

class SleepBasic : public ::testing::Test
{
public:
	SleepBasic() {}

   virtual void SetUp()
   {
#if   defined( __AAL_WINDOWS__ )

      QueryPerformanceFrequency(&m_Freq);
      m_Start.QuadPart = 0;
      m_End.QuadPart   = 0;

#endif // OS
   }

// virtual void TearDown() { }

#if   defined( __AAL_WINDOWS__ )

   LARGE_INTEGER m_Freq;
   LARGE_INTEGER m_Start;
   LARGE_INTEGER m_End;

#elif defined( __AAL_LINUX__ )

   struct timespec m_Start;
   struct timespec m_End;

#endif // __AAL_LINUX__
};

TEST_F(SleepBasic, DISABLED_aal0134)
{
	//Tests accuracy of SleepSec()

	//Get the Start timestamp
#if   defined( __AAL_LINUX__ )

   struct timeval tv;

   gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec = (tv.tv_usec * 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_Start);

#endif // OS

   //Sleep for 2 seconds.
   SleepSec(2);

   //Get the end timestamp.
#if   defined( __AAL_LINUX__ )

   gettimeofday(&tv, NULL);
   m_End.tv_sec  = tv.tv_sec;
   m_End.tv_nsec = (tv.tv_usec * 1000);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
   n /= (1000ULL * 1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt StartTime = s + n;

   s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
	n /= (1000ULL * 1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt EndTime = s + n;

   //Verify that the difference in timestamps is 2 seconds.
   EXPECT_EQ(2, EndTime - StartTime);

#elif defined( __AAL_WINDOWS__ )

    QueryPerformanceCounter(&m_End);
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart) / (double)(m_Freq.QuadPart);

    //Verify that the difference in timestamps is within the tolerance level of 1 second.
    EXPECT_GE(diff, 2.0);
    EXPECT_LE(diff, 3.0);

#endif // OS
}

TEST_F(SleepBasic, DISABLED_aal0135)
{
   //Tests accuracy of SleepMilli()

   //Get the Start timestamp
#if   defined( __AAL_LINUX__ )

   struct timeval tv;

   gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec = (tv.tv_usec * 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_Start);

#endif // OS

   //Sleep for 2 milli seconds.
   SleepMilli(2);

   //Get the end timestamp.
#if   defined( __AAL_LINUX__ )

   gettimeofday(&tv, NULL);
   m_End.tv_sec  = tv.tv_sec;
   m_End.tv_nsec  = (tv.tv_usec * 1000);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt StartTime = s + n;

   s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt EndTime = s + n;

   //Verify that the difference in timestamps is within the tolerance level of 3 milliseconds.
   EXPECT_GE(EndTime - StartTime, 2);
   EXPECT_LE(EndTime - StartTime, 5);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_End);

   double PCFreq = (double)(m_Freq.QuadPart) / 1000;
   double diff   = (double)(m_End.QuadPart - m_Start.QuadPart) / PCFreq;

   //Verify that the difference in timestamps is within the tolerance level of 18 milliseconds.
   EXPECT_GE(diff, 2.0);
   EXPECT_LE(diff, 20.0);

#endif // OS
}

TEST_F(SleepBasic, DISABLED_aal0136)
{
	//Tests accuracy of SleepMicro()

	//Get the start timestamp
#if   defined( __AAL_LINUX__ )

   struct timeval tv;

   gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec = (tv.tv_usec * 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_Start);

#endif // OS

   //Sleep for 2 microseconds.
   SleepMicro(2);

   //Get the end timestamp.
#if   defined( __AAL_LINUX__ )

   gettimeofday(&tv, NULL);
   m_End.tv_sec  = tv.tv_sec;
   m_End.tv_nsec = (tv.tv_usec * 1000);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
	AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
	s *= (1000ULL * 1000ULL);
   n /= 1000ULL;
	AAL::btUnsigned64bitInt StartTime = s + n;

	s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
	s *= (1000ULL * 1000ULL);
	n /= 1000ULL;
	AAL::btUnsigned64bitInt EndTime = s + n;

   //Verify that the difference in timestamps is within the tolerance level of 1 millisecond.
   EXPECT_GE(EndTime - StartTime, 2);
   EXPECT_LE(EndTime - StartTime, 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_End);

   double PCFreq = (double)(m_Freq.QuadPart) / 1000000;
   double diff   = (double)(m_End.QuadPart - m_Start.QuadPart) / PCFreq;

   //Verify that the difference in timestamps is within the tolerance level of 10 milliseconds.
   EXPECT_GE(diff, 0.5);
   EXPECT_LE(diff, 10000.0);

#endif // OS
}

TEST_F(SleepBasic, DISABLED_aal0137)
{
   //Tests accuracy of SleepNano()

   //Get the start timestamp.
#if   defined( __AAL_LINUX__ )

   struct timeval tv;

   gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec  = (tv.tv_usec * 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_Start);

#endif // OS

   //Sleep for 2 nanoseconds.
   SleepNano(2);

   //Get the end timestamp.
#if   defined( __AAL_LINUX__ )

   gettimeofday(&tv, NULL);
   m_End.tv_sec  = tv.tv_sec;
   m_End.tv_nsec = (tv.tv_usec * 1000);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
   s *= (1000ULL * 1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt StartTime = s + n;

   s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
   n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
   s *= (1000ULL * 1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt EndTime = s + n;

   //Verify that the difference in timestamps is within the tolerance level of 1.0 millisecond.
   EXPECT_GE(EndTime - StartTime, 2);
   EXPECT_LE(EndTime - StartTime, 1000000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_End);

   double PCFreq = (double)(m_Freq.QuadPart) / 100000000;
   double diff   = (double)(m_End.QuadPart - m_Start.QuadPart) / PCFreq;

   // Verify that the difference in timestamps is within the tolerance level of 10 milliseconds.
   EXPECT_GE(diff, 2.0);
   EXPECT_LE(diff, 10000000.0);

#endif // OS
}

TEST_F(SleepBasic, DISABLED_aal0138)
{
	//Tests accuracy of SleepZero() in milliseconds.

	//Get the start timestamp.
#if   defined( __AAL_LINUX__ )

   struct timeval tv;

   gettimeofday(&tv, NULL);
   m_Start.tv_sec  = tv.tv_sec;
   m_Start.tv_nsec = (tv.tv_usec * 1000);

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_Start);

#endif

   //Sleep for zero seconds.
   SleepZero();

	//Get the end timestamp.
#if   defined( __AAL_LINUX__ )

   gettimeofday(&tv, NULL);
   m_End.tv_sec  = tv.tv_sec;
   m_End.tv_nsec = (tv.tv_usec * 1000);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
	AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt StartTime = s + n;

	s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt EndTime = s + n;

   //Verify that the difference in timestamps is within the tolerance level of 1 millisecond.

   EXPECT_GE(EndTime - StartTime, 0); //very high tolerance level of 118000 is required for comparison in Nanosecs. Hence comparison in millisecs -
   EXPECT_LE(EndTime - StartTime, 1); //also to keep the units similar to windows specific code.

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_End);

   double PCFreq = (double)(m_Freq.QuadPart) / 1000;
   double diff   = (double)(m_End.QuadPart - m_Start.QuadPart) / PCFreq;

   EXPECT_GE(diff, 0.0);
   EXPECT_LE(diff, 1.0);

#endif // OS
}
