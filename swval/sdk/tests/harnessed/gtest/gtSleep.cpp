// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "aalsdk/osal/Timer.h"
#include "aalsdk/osal/Sleep.h"

#if defined( __AAL_WINDOWS__ )
	LARGE_INTEGER Timer::sm_ClockFreq = { 0, 0 };
#elif defined( __AAL_LINUX__ )
	struct timeval tv;
#endif

class SleepBasic : public ::testing::Test
{
public:
	SleepBasic(){}
// virtual void SetUp() { }
// virtual void TearDown() { }

#if   defined( __AAL_WINDOWS__ )

   LARGE_INTEGER m_Start;
   LARGE_INTEGER m_End;

#elif defined( __AAL_LINUX__ )

   struct timespec m_Start;
   struct timespec m_End;

#endif // __AAL_LINUX__
};

TEST_F(SleepBasic, aal0134)
{
	//Tests accuracy of SleepSec()

	//Get the Start timestamp
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_Start.tv_sec  = tv.tv_sec;
    m_Start.tv_nsec  = (tv.tv_usec * 1000);

#elif   defined( __AAL_WINDOWS__ )

    m_Start.QuadPart = 0;
    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
      cout << " QueryPerformanceFrequency failed \n";
    }

    QueryPerformanceCounter(&m_Start);

#endif

    //Sleep for 2 seconds.
    SleepSec(2);

    //Get the end timestamp.
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_End.tv_sec  = tv.tv_sec;
    m_End.tv_nsec  = (tv.tv_usec * 1000);

    AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
    AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
    n /= (1000ULL * 1000ULL * 1000ULL);
    AAL::btUnsigned64bitInt StartTime = s + n;

    s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
	n /= (1000ULL * 1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt EndTime = s + n;

#elif   defined( __AAL_WINDOWS__ )

    m_End.QuadPart = 0;
    QueryPerformanceCounter(&m_End);
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart)/(double)(Timer::sm_ClockFreq.QuadPart);

#endif

#if defined( __AAL_LINUX__ )

    //Verify that the difference in timestamps is 2 seconds.
    EXPECT_TRUE(2 == (EndTime - StartTime));

#elif   defined( __AAL_WINDOWS__ )

    //Verify that the difference in timestamps is within the tolerance level of 1 second.
    EXPECT_TRUE(2 <= diff);
    EXPECT_TRUE(3 >= diff);

#endif
}

TEST_F(SleepBasic, aal0135)
{
	//Tests accuracy of SleepMilli()

	//Get the Start timestamp
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_Start.tv_sec  = tv.tv_sec;
    m_Start.tv_nsec = (tv.tv_usec * 1000);

#elif   defined( __AAL_WINDOWS__ )

    m_Start.QuadPart = 0;

    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) )
    {
       cout << " QueryPerformanceFrequency failed \n";
    }

    QueryPerformanceCounter(&m_Start);

#endif

    //Sleep for 2 milli seconds.
    SleepMilli(2);

    //Get the end timestamp.
#if defined( __AAL_LINUX__ )

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

#elif   defined( __AAL_WINDOWS__ )

    m_End.QuadPart = 0;

    QueryPerformanceCounter(&m_End);
    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart)/1000;
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart)/PCFreq;

#endif


#if defined( __AAL_LINUX__ )
    //Verify that the difference in timestamps is within the tolerance level of 3 milliseconds.
    EXPECT_TRUE(2 <= (EndTime - StartTime));
    EXPECT_TRUE(5 >= (EndTime - StartTime));

#elif   defined( __AAL_WINDOWS__ )
    //Verify that the difference in timestamps is within the tolerance level of 18 milliseconds.
    EXPECT_TRUE(2 <= diff);
    EXPECT_TRUE(20 >= diff);

#endif
}

TEST_F(SleepBasic, aal0136)
{
	//Tests accuracy of SleepMicro()

	//Get the start timestamp
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_Start.tv_sec  = tv.tv_sec;
    m_Start.tv_nsec  = (tv.tv_usec * 1000);

#elif   defined( __AAL_WINDOWS__ )

    m_Start.QuadPart = 0;

    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) )
    {
       cout << " QueryPerformanceFrequency failed \n";
    }

    QueryPerformanceCounter(&m_Start);

#endif

    //Sleep for 2 microseconds.
    SleepMicro(2);

    //Get the end timestamp.
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_End.tv_sec  = tv.tv_sec;
    m_End.tv_nsec  = (tv.tv_usec * 1000);

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

#elif   defined( __AAL_WINDOWS__ )

    m_End.QuadPart = 0;

    QueryPerformanceCounter(&m_End);
    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart)/100000;
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart)/PCFreq;
#endif

#if defined( __AAL_LINUX__ )
    //Verify that the difference in timestamps is within the tolerance level of 1 millisecond.
    EXPECT_TRUE(2 <= (EndTime - StartTime));
    EXPECT_TRUE(1000 >= (EndTime - StartTime));

#elif   defined( __AAL_WINDOWS__ )
    //Verify that the difference in timestamps is within the tolerance level of 10 milliseconds.
    EXPECT_TRUE(2 <= diff);
    EXPECT_TRUE(10000 >= diff);
#endif
}

TEST_F(SleepBasic, aal0137)
{
	//Tests accuracy of SleepNano()

	//Get the start timestamp.
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_Start.tv_sec  = tv.tv_sec;
    m_Start.tv_nsec  = (tv.tv_usec * 1000);

#elif   defined( __AAL_WINDOWS__ )

    m_Start.QuadPart = 0;

    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) )
    {
       cout << " QueryPerformanceFrequency failed \n";
    }

    QueryPerformanceCounter(&m_Start);


#endif

    //Sleep for 2 nanoseconds.
    SleepNano(2);

    //Get the end timestamp.
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_End.tv_sec  = tv.tv_sec;
    m_End.tv_nsec  = (tv.tv_usec * 1000);

    AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)m_Start.tv_sec;
   	AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)m_Start.tv_nsec;
   	s *= (1000ULL * 1000ULL * 1000ULL);
   	AAL::btUnsigned64bitInt StartTime = s + n;

   	s = (AAL::btUnsigned64bitInt)m_End.tv_sec;
   	n = (AAL::btUnsigned64bitInt)m_End.tv_nsec;
   	s *= (1000ULL * 1000ULL * 1000ULL);
   	AAL::btUnsigned64bitInt EndTime = s + n;

#elif   defined( __AAL_WINDOWS__ )

    m_End.QuadPart = 0;

    QueryPerformanceCounter(&m_End);
    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart)/100000000;
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart)/PCFreq;

#endif

#if defined( __AAL_LINUX__ )
    //Verify that the difference in timestamps is within the tolerance level of 0.5 millisecond.
    EXPECT_TRUE(2 <= (EndTime - StartTime));
    EXPECT_TRUE(500000 >= (EndTime - StartTime));

    #elif   defined( __AAL_WINDOWS__ )
    //Verify that the difference in timestamps is within the tolerance level of 10 milliseconds.
    EXPECT_TRUE(2 <= diff);
    EXPECT_TRUE(10000000 >= diff);
#endif
}

TEST_F(SleepBasic, aal0138)
{
	//Tests accuracy of SleepZero() in milliseconds.

	//Get the start timestamp.
#if defined( __AAL_LINUX__ )

    gettimeofday(&tv, NULL);
    m_Start.tv_sec  = tv.tv_sec;
    m_Start.tv_nsec  = (tv.tv_usec * 1000);

#elif   defined( __AAL_WINDOWS__ )

    m_Start.QuadPart = 0;
	if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) )
	{
	   cout << " QueryPerformanceFrequency failed \n";
	}
	QueryPerformanceCounter(&m_Start);

#endif

	//Sleep for zero seconds.
	SleepZero();

	//Get the end timestamp.
#if defined( __AAL_LINUX__ )

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

#elif   defined( __AAL_WINDOWS__ )

    m_End.QuadPart = 0;
    QueryPerformanceCounter(&m_End);
    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart)/1000;
    double diff = (double)(m_End.QuadPart - m_Start.QuadPart)/PCFreq;

#endif

    //Verify that the difference in timestamps is within the tolerance level of 1 millisecond.
#if defined( __AAL_LINUX__ )

    EXPECT_TRUE(0 <= (EndTime - StartTime)); //very high tolerance level of 118000 is required for comparison in Nanosecs. Hence comparison in millisecs -
    EXPECT_TRUE(1 >= (EndTime - StartTime)); //also to keep the units similar to windows specific code.

#elif   defined( __AAL_WINDOWS__ )

    EXPECT_TRUE(0 <= diff);
    EXPECT_TRUE(1 >= diff);

#endif
}
