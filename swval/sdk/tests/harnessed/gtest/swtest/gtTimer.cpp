// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "aalsdk/osal/Timer.h"

class TimerBasic : public ::testing::Test
{
public:
	TimerBasic() {}

   virtual void SetUp()
   {
#if defined( __AAL_WINDOWS__ )
      QueryPerformanceFrequency(&m_Freq);
      m_dFreq = (double) m_Freq.QuadPart;
      m_OS_Start.QuadPart = 0;
#endif // __AAL_WINDOWS__
   }

// virtual void TearDown() { }

#if   defined( __AAL_LINUX__ )

   struct timespec m_OS_Start;

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER m_Freq;
   double        m_dFreq;
   LARGE_INTEGER m_OS_Start;

#endif // OS
};

TEST_F(TimerBasic, aal0139)
{
	//Tests accuracy of Timer Constructor

	//Get the Start timestamp
#if   defined( __AAL_LINUX__ )

   struct timespec OS_Start;
   struct timeval tv;
   gettimeofday(&tv, NULL);
   OS_Start.tv_sec  = tv.tv_sec;
   OS_Start.tv_nsec = tv.tv_usec * 1000;

#elif defined( __AAL_WINDOWS__ )

   QueryPerformanceCounter(&m_OS_Start);

#endif // OS

   //Get the current timestamp
   Timer Now;

//Verify that the difference in time returned by the Timer constructor and the OS Timer is within the tolerance level.
#if   defined( __AAL_LINUX__ )
//Tolerance level is 50 microseconds

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)OS_Start.tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)OS_Start.tv_nsec;
   s *= (1000ULL * 1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt OSTimer = s + n;

   s = (AAL::btUnsigned64bitInt)((struct timespec)Now).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)Now).tv_nsec;
	s *= (1000ULL * 1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerNow = s + n;

   EXPECT_GE(TimerNow - OSTimer, 0);
   EXPECT_LE(TimerNow - OSTimer, 50000);

#elif defined( __AAL_WINDOWS__ )

	//Tolerance level is 1 second
   double Diff = (double)(((LARGE_INTEGER)Now).QuadPart - m_OS_Start.QuadPart) / m_dFreq;

   EXPECT_GE(Diff, 0.0);
   EXPECT_LE(Diff, 1.0);

#endif // OS
}

TEST_F(TimerBasic, aal0140)
{
	//Tests functionality of Addition operator.

	Timer time_one, time_two, time_result;

	//Initialise time_one and time_two.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 5;
   tv.tv_nsec = 50;
   time_one = Timer(&tv);

   tv.tv_sec  = 6;
   tv.tv_nsec = 100;
   time_two = Timer(&tv);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerOne = s + n;

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_two).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_two).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerTwo = s + n;

#elif defined( __AAL_WINDOWS__ )

    LARGE_INTEGER one;
    LARGE_INTEGER two;

    one.QuadPart = 2 * m_Freq.QuadPart;
    time_one = one;

    two.QuadPart = 5 * m_Freq.QuadPart;
    time_two = two;

#endif // OS

   //Add the two initialised times using the addition operator.
   time_result = time_one + time_two;

   //Verify that the time_result contains the sum of time_one and time_two.
#if   defined( __AAL_LINUX__ )

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerSum = s + n;

   EXPECT_EQ(TimerSum, TimerOne + TimerTwo);

#elif defined( __AAL_WINDOWS__ )

   double TimerSum = (double)((((LARGE_INTEGER)time_one).QuadPart) + (((LARGE_INTEGER)time_two).QuadPart)) / m_dFreq;

   EXPECT_EQ(TimerSum, (double)(((LARGE_INTEGER)time_result).QuadPart) / m_dFreq);

#endif // OS
}

TEST_F(TimerBasic, aal0141)
{
   //Tests functionality of Subtraction operator.

   Timer time_one, time_two, time_result;

	//Initialise time_one and time_two.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 6;
   tv.tv_nsec = 100;
   time_one   = Timer(&tv);

   tv.tv_sec  = 10;
   tv.tv_nsec = 250;
   time_two   = Timer(&tv);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerOne = s + n;

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_two).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_two).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerTwo = s + n;

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;
   LARGE_INTEGER two;

   one.QuadPart = 2 * m_Freq.QuadPart;
   time_one = one;

   two.QuadPart = 5 * m_Freq.QuadPart;
   time_two = two;

#endif // OS

   //Find the difference between the two times using the subtraction operator.
   time_result = time_two - time_one;

   //Verify that the time_result contains the difference between time_one and time_two.
#if   defined( __AAL_LINUX__ )

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerDiff = s + n;

   EXPECT_EQ(TimerDiff, TimerTwo - TimerOne);

#elif defined( __AAL_WINDOWS__ )

   double TimerDiff = (double)((((LARGE_INTEGER)time_two).QuadPart) - (((LARGE_INTEGER)time_one).QuadPart)) / m_dFreq;

   EXPECT_EQ(TimerDiff, (double)(((LARGE_INTEGER)time_result).QuadPart) / m_dFreq);

#endif // OS
}

TEST_F(TimerBasic, aal0142)
{
   //Tests the functionality of comparison operators.

   Timer time_one, time_two, time_three;

   //Initialise time_one, time_two and time_three. time_one and and time_two are initialised with same value.
   //time_three is initialised with a value greater than time_one and time_two.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 10;
   tv.tv_nsec = 250;
   time_one = Timer(&tv);
   time_two = Timer(&tv);

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerOne = s + n;
   AAL::btUnsigned64bitInt TimerTwo = s + n;

   tv.tv_sec  = 16;
   tv.tv_nsec = 100;
   time_three = Timer(&tv);

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_three).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_three).tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerThree = s + n;

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;
   LARGE_INTEGER two;
   LARGE_INTEGER three;

   one.QuadPart = 2 * m_Freq.QuadPart;
   time_one = one;

   two.QuadPart = 2 * m_Freq.QuadPart;
   time_two = two;

   three.QuadPart = 5 * m_Freq.QuadPart;
   time_three = three;

#endif // OS

	//Test the functionality of == operator.
   EXPECT_TRUE(time_one == time_two);
#if   defined( __AAL_LINUX__ )
   EXPECT_EQ(TimerOne, TimerTwo);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_EQ(((LARGE_INTEGER)time_one).QuadPart, ((LARGE_INTEGER)time_two).QuadPart);
#endif // OS


   //Test the functionality of != operator.
   EXPECT_TRUE(time_one != time_three);
#if   defined( __AAL_LINUX__ )
   EXPECT_NE(TimerOne, TimerThree);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_NE(((LARGE_INTEGER)time_one).QuadPart, ((LARGE_INTEGER)time_three).QuadPart);
#endif // OS


   //Test the functionality of < operator.
   EXPECT_TRUE(time_one < time_three);
#if   defined( __AAL_LINUX__ )
   EXPECT_LT(TimerOne, TimerThree);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_LT(((LARGE_INTEGER)time_one).QuadPart, ((LARGE_INTEGER)time_three).QuadPart);
#endif // OS


   //Test the functionality of <= operator.
   EXPECT_TRUE(time_one <= time_two);
   EXPECT_TRUE(time_one <= time_three);
#if   defined( __AAL_LINUX__ )
   EXPECT_LE(TimerOne, TimerTwo);
   EXPECT_LE(TimerOne, TimerThree);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_LE(((LARGE_INTEGER)time_one).QuadPart, ((LARGE_INTEGER)time_two).QuadPart);
   EXPECT_LE(((LARGE_INTEGER)time_one).QuadPart, ((LARGE_INTEGER)time_three).QuadPart);
#endif // OS


   //Test the functionality of > operator.
   EXPECT_TRUE(time_three > time_one);
#if   defined( __AAL_LINUX__ )
   EXPECT_GT(TimerThree, TimerOne);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_GT(((LARGE_INTEGER)time_three).QuadPart, ((LARGE_INTEGER)time_one).QuadPart);
#endif // OS


   //Test the functionality of >= operator.
   EXPECT_TRUE(time_two >= time_one);
   EXPECT_TRUE(time_three >= time_one);
#if   defined( __AAL_LINUX__ )
   EXPECT_GE(TimerTwo, TimerOne);
	EXPECT_GE(TimerThree, TimerOne);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_GE(((LARGE_INTEGER)time_two).QuadPart, ((LARGE_INTEGER)time_one).QuadPart);
   EXPECT_GE(((LARGE_INTEGER)time_three).QuadPart, ((LARGE_INTEGER)time_one).QuadPart);
#endif // OS
}

TEST_F(TimerBasic, aal0143)
{
   //Tests accuracy of AsSeconds(), AsmilliSeconds(), AsMicroSeconds(), AsNanoseconds().

   Timer                   time_one;
   AAL::btUnsigned64bitInt result;
   double                  dResult;

   //Initialise Timer time_one.
#if   defined( __AAL_LINUX__ )

	timespec tv;
	tv.tv_sec  = 10;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;

   one.QuadPart = 10 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Testing functions that accepts argument of AAL::btUnsigned64bitInt datatype

   result = 0;
   time_one.AsSeconds(result);
   EXPECT_EQ(result, 10);

   result = 0;
   time_one.AsMilliSeconds(result);
   EXPECT_EQ(result, 10000);

   result = 0;
   time_one.AsMicroSeconds(result);
   EXPECT_EQ(result, 10000000);

   result = 0;
	time_one.AsNanoSeconds(result);
#if   defined( __AAL_LINUX__ )
   EXPECT_EQ(10000000250, result);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_EQ(10000000000, result);
#endif // OS

   //Testing functions that accepts argument of double datatype

   dResult = 0.0;
   time_one.AsSeconds(dResult);
#if   defined( __AAL_LINUX__ )
   EXPECT_DOUBLE_EQ(10.00000025, dResult);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_DOUBLE_EQ(10.0, dResult);
#endif // OS

   dResult = 0.0;
   time_one.AsMilliSeconds(dResult);
#if   defined( __AAL_LINUX__ )
   EXPECT_DOUBLE_EQ(10000.00025, dResult);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_DOUBLE_EQ(10000.0, dResult);
#endif // OS

   dResult = 0.0;
   time_one.AsMicroSeconds(dResult);
#if   defined( __AAL_LINUX__ )
   EXPECT_DOUBLE_EQ(10000000.25, dResult);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_DOUBLE_EQ(10000000.0, dResult);
#endif // OS

   dResult = 0.0;
   time_one.AsNanoSeconds(dResult);
#if   defined( __AAL_LINUX__ )
   EXPECT_DOUBLE_EQ(10000000250.0, dResult);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_DOUBLE_EQ(10000000000.0, dResult);
#endif // OS
}

TEST_F(TimerBasic, aal0144)
{
   //Tests accuracy of Now()

   Timer time_one;

   //Initialise Timer time_one with a default value.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 5;
   tv.tv_nsec = 50;
   time_one   = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;

   one.QuadPart = 2 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Obtain the current timestamp using the Now() function.
   time_one = time_one.Now();

   //Verify that the difference in time returned by Now() and the OS Timer is within the tolerance level.
#if   defined( __AAL_LINUX__ )
   // The tolerance level is 10 microseconds.

   struct timespec OS_Now;
   struct timeval ts;
   gettimeofday(&ts, NULL);
   OS_Now.tv_sec  = ts.tv_sec;
   OS_Now.tv_nsec = ts.tv_usec * 1000;

   AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)OS_Now.tv_sec;
   AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)OS_Now.tv_nsec;
   s *= 1000ULL;
   n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt  OSTimer = s + n;

   s = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_sec;
   n = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_nsec;
   s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
   AAL::btUnsigned64bitInt TimerNow = s + n;

   EXPECT_LE(0, OSTimer - TimerNow);
   EXPECT_GE(10000, OSTimer - TimerNow);

#elif defined ( __AAL_WINDOWS__ )
   //The tolerance level is 1 second.

   LARGE_INTEGER OS_Now;
   OS_Now.QuadPart = 0;

   QueryPerformanceCounter(&OS_Now);

   double Diff = (double)(OS_Now.QuadPart - ((LARGE_INTEGER)time_one).QuadPart) / m_dFreq;

   EXPECT_LE(0.0, Diff);
   EXPECT_GE(1.0, Diff);

#endif // OS
}

TEST_F(TimerBasic, aal0145)
{
	//Tests accuracy of NormalisedUnits()

	Timer time_one;

	// Initialize a timer with 5000 seconds.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 5000;
   tv.tv_nsec = 250;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;

   one.QuadPart = 5000 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is hours.
   EXPECT_STRCASEEQ("Hours", time_one.NormalizedUnits().c_str());

   //Initialize a timer with 100 seconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 100;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = 100 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is minutes.
   EXPECT_STRCASEEQ("Minutes", time_one.NormalizedUnits().c_str());

	//Initialize a timer with 50 seconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 50;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = 50 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is seconds.
   EXPECT_STRCASEEQ("Seconds", time_one.NormalizedUnits().c_str());

   //Initialize a timer with 2 milliseconds.
#if   defined( __AAL_LINUX__ )

   tv.tv_sec  = 0;
   tv.tv_nsec = 2000000;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is milliseconds.
   EXPECT_STRCASEEQ("MilliSeconds", time_one.NormalizedUnits().c_str());

   //Initialize a timer with 2 microseconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 0;
	tv.tv_nsec = 2000;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000000;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is microseconds.
   EXPECT_STRCASEEQ("MicroSeconds", time_one.NormalizedUnits().c_str());

   //Initialize a timer with 2 nanoseconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 0;
	tv.tv_nsec = 2;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000000000;
   time_one = one;

#endif // OS

   //Verify that the normalised unit returned by the function is nanoseconds.
   EXPECT_STRCASEEQ("NanoSeconds", time_one.NormalizedUnits().c_str());
}

TEST_F(TimerBasic, aal0146)
{
	//Tests accuracy of Normalized()

	Timer                   time_one;
	AAL::btUnsigned64bitInt i;
	double                  d;

	//Initialize a timer with 3600 seconds.
#if   defined( __AAL_LINUX__ )

   timespec tv;
   tv.tv_sec  = 3600;
   tv.tv_nsec = 0;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   LARGE_INTEGER one;

   one.QuadPart = 3600 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   EXPECT_STRCASEEQ("1 Hour", time_one.Normalized().c_str());

   //Verify that the normalised result is in hours.
   i = 0;
   d = 0.0;
   EXPECT_STRCASEEQ("1 Hour", time_one.Normalized(&i, &d).c_str());

   //Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
   EXPECT_EQ(1, i);
   EXPECT_DOUBLE_EQ(1.0, d);

   //Initialize a timer with 60 seconds.
#if   defined( __AAL_LINUX__ )

   tv.tv_sec  = 60;
   tv.tv_nsec = 0;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = 60 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Verify that the normalised result is in minutes.
   i = 0;
   d = 0.0;
   EXPECT_STRCASEEQ("1 Minute", time_one.Normalized(&i, &d).c_str());

   //Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
   EXPECT_EQ(1, i);
   EXPECT_DOUBLE_EQ(1.0, d);

   //Initialize a timer with 50 seconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 50;
	tv.tv_nsec = 0;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = 50 * m_Freq.QuadPart;
   time_one = one;

#endif // OS

   //Verify that the normalised result is in seconds.
   i = 0;
   d = 0.0;
   EXPECT_STRCASEEQ("50 Seconds", time_one.Normalized(&i, &d).c_str());

   //Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
   EXPECT_EQ(50, i);
   EXPECT_DOUBLE_EQ(50.0, d);

   //Initialize a timer with 2 milliseconds.
#if   defined( __AAL_LINUX__ )

	tv.tv_sec  = 0;
	tv.tv_nsec = 2000000;
	time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000;
   time_one = one;

#endif // OS

	//Verify that the normalised result is in milliseconds.
   i = 0;
   d = 0.0;
#if   defined( __AAL_LINUX__ )
   // The calculation is based on double and becomes fractional in Windows: "1.99978 MilliSeconds"
   EXPECT_STRCASEEQ("2 MilliSeconds", time_one.Normalized(&i, &d).c_str());
#elif defined( __AAL_WINDOWS__ )
   time_one.Normalized(&i, &d);
#endif // __AAL_LINUX__

   //Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
#if   defined( __AAL_LINUX__ )
   EXPECT_EQ(2, i);
   EXPECT_DOUBLE_EQ(2.0, d);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_LE(1, i);
   EXPECT_GE(2, i);
   EXPECT_NEAR(2.0, d, 0.1);
#endif // OS

	//Initialize a timer with 2 microseconds.
#if   defined( __AAL_LINUX__ )

   tv.tv_sec  = 0;
   tv.tv_nsec = 2000;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000000;
   time_one = one;

#endif // OS

   //Verify that the normalised result is in microseconds.
   i = 0;
   d = 0.0;
#if   defined( __AAL_LINUX__ )
   // The calculation is based on double and becomes fractional in Windows: "1.64219 MicroSeconds"
   EXPECT_STRCASEEQ("2 MicroSeconds", time_one.Normalized(&i, &d).c_str());
#elif defined( __AAL_WINDOWS__ )
   time_one.Normalized(&i, &d);
#endif // __AAL_LINUX__

#if   defined( __AAL_LINUX__ )
   EXPECT_EQ(2, i);
   EXPECT_DOUBLE_EQ(2.0, d);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_LE(1, i);
   EXPECT_GE(2, i);
   EXPECT_NEAR(2.0, d, 0.5);
#endif // OS


   //Initialize a timer with 2 nanoseconds.
#if   defined( __AAL_LINUX__ )

   tv.tv_sec  = 0;
   tv.tv_nsec = 2;
   time_one = Timer(&tv);

#elif defined( __AAL_WINDOWS__ )

   one.QuadPart = (2 * m_Freq.QuadPart) / 1000000000;
   time_one = one;

#endif // OS

   //Verify that the normalised result is in nanoseconds.
   i = 0;
   d = 0.0;
#if   defined( __AAL_LINUX__ )
   EXPECT_STRCASEEQ("2 NanoSeconds", time_one.Normalized(&i, &d).c_str());
#elif defined( __AAL_WINDOWS__ )
   // The Windows mechanism is not granular down to nanoseconds.
   EXPECT_STRCASEEQ("0 NanoSeconds", time_one.Normalized(&i, &d).c_str());
#endif // __AAL_LINUX__

#if   defined( __AAL_LINUX__ )
   EXPECT_EQ(2, i);
   EXPECT_DOUBLE_EQ(2.0, d);
#elif defined( __AAL_WINDOWS__ )
   EXPECT_EQ(0, i);
   EXPECT_DOUBLE_EQ(0.0, d);
#endif // OS
}

