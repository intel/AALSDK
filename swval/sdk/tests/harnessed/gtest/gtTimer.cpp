// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"
#include "aalsdk/osal/Timer.h"

class TimerBasic : public ::testing::Test,
				   public Timer
{
public:
	TimerBasic(){}
// virtual void SetUp() { }
// virtual void TearDown() { }

};

TEST_F(TimerBasic, aal0139)
{
	//Tests accuracy of Timer Constructor

	//Get the Start timestamp
#if defined( __AAL_LINUX__ )

	struct timespec OS_Start;
	struct timeval tv;
    gettimeofday(&tv, NULL);
    OS_Start.tv_sec  = tv.tv_sec;
    OS_Start.tv_nsec = tv.tv_usec * 1000;

#elif   defined( __AAL_WINDOWS__ )

    LARGE_INTEGER OS_Start;
    OS_Start.QuadPart = 0;
    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
      cout << " QueryPerformanceFrequency failed \n";
    }

    QueryPerformanceCounter(&OS_Start);

#endif

    //Get the current timestamp
    Timer Now;
//Verify that the difference in time returned by the Timer constructor and the OS Timer is within the tolerance level.
#if defined( __AAL_LINUX__ )
//Tolerance level is 50 microseconds

    AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)OS_Start.tv_sec;
    AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)OS_Start.tv_nsec;
    s *= (1000ULL * 1000ULL * 1000ULL);
    AAL::btUnsigned64bitInt OSTimer = s + n;

    s = (AAL::btUnsigned64bitInt)((struct timespec)Now).tv_sec;
    n = (AAL::btUnsigned64bitInt)((struct timespec)Now).tv_nsec;
	s *= (1000ULL * 1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerNow = s + n;

	EXPECT_TRUE((TimerNow - OSTimer) >= 0 && (TimerNow - OSTimer) <= 50000);

#elif   defined( __AAL_WINDOWS__ )
	//Tolerance level is 1 second
	double Diff = (double)(((LARGE_INTEGER)Now).QuadPart - OS_Start.QuadPart)/(double)(Timer::sm_ClockFreq.QuadPart);
    EXPECT_TRUE((Diff >= 0) && (Diff <=1));

#endif
}

TEST_F(TimerBasic, aal0140)
{
	//Tests functionality of Addition operator.

	Timer time_one, time_two, time_result;

	//Initialise time_one and time_two.
#if defined( __AAL_LINUX__ )

	timespec tv;
	tv.tv_sec = 5;
	tv.tv_nsec = 50;
	time_one = Timer(&tv);

	tv.tv_sec = 6;
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

#elif   defined( __AAL_WINDOWS__ )

    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
      cout << " QueryPerformanceFrequency failed \n";
    }

    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

    ((LARGE_INTEGER)time_one).QuadPart = 2 * PCFreq;
    ((LARGE_INTEGER)time_two).QuadPart = 5 * PCFreq;

#endif

    //Add the two initialised times using the addition operator.
    time_result = time_one + time_two;

    //Verify that the time_result contains the sum of time_one and time_two.
#if defined( __AAL_LINUX__ )
    s = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_sec;
	n = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerSum = s + n;

	EXPECT_TRUE(TimerSum == (TimerOne+TimerTwo));

#elif   defined( __AAL_WINDOWS__ )

	double TimerSum = (double)((((LARGE_INTEGER)time_one).QuadPart) + (((LARGE_INTEGER)time_two).QuadPart))/PCFreq ;
	EXPECT_TRUE( (double)(((LARGE_INTEGER)time_result).QuadPart)/PCFreq) == TimerSum);

#endif


}


TEST_F(TimerBasic, aal0141)
{
	//Tests functionality of Subtraction operator.

	Timer time_one, time_two, time_result;

	//Initialise time_one and time_two.
#if defined( __AAL_LINUX__ )

	timespec tv;
	tv.tv_sec = 6;
	tv.tv_nsec = 100;
	time_one = Timer(&tv);

	tv.tv_sec = 10;
	tv.tv_nsec = 250;
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

#elif   defined( __AAL_WINDOWS__ )

    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
      cout << " QueryPerformanceFrequency failed \n";
    }

    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

    ((LARGE_INTEGER)time_one).QuadPart = 2 * PCFreq;
    ((LARGE_INTEGER)time_two).QuadPart = 5 * PCFreq;

#endif

    //Find the difference between the two times using the subtraction operator.
    time_result = time_two - time_one;

    //Verify that the time_result contains the difference between time_one and time_two.
#if defined( __AAL_LINUX__ )
    s = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_sec;
	n = (AAL::btUnsigned64bitInt)((struct timespec)time_result).tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerDiff = s + n;

	EXPECT_TRUE((TimerTwo - TimerOne) == TimerDiff);

#elif   defined( __AAL_WINDOWS__ )

	double TimerDiff = (double)((((LARGE_INTEGER)time_two).QuadPart) - (((LARGE_INTEGER)time_one).QuadPart))/PCFreq ;
	EXPECT_TRUE( (double)(((LARGE_INTEGER)time_result).QuadPart)/PCFreq) == TimerDiff);

#endif



}

TEST_F(TimerBasic, aal0142)
{
	//Tests the functionality of comparison operators.

	Timer time_one, time_two, time_three;

	//Initialise time_one, time_two and time_three. time_one and and time_two are initialised with same value.
	//time_three is initialised with a value greater than time_one and time_two.
#if defined( __AAL_LINUX__ )
	timespec tv;
	tv.tv_sec = 10;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
	time_two = Timer(&tv);

	AAL::btUnsigned64bitInt s = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_sec;
	AAL::btUnsigned64bitInt n = (AAL::btUnsigned64bitInt)((struct timespec)time_one).tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerOne = s + n;
	AAL::btUnsigned64bitInt TimerTwo = s + n;

	tv.tv_sec = 16;
	tv.tv_nsec = 100;
	time_three = Timer(&tv);

	s = (AAL::btUnsigned64bitInt)((struct timespec)time_three).tv_sec;
	n = (AAL::btUnsigned64bitInt)((struct timespec)time_three).tv_nsec;
	s *= 1000ULL;
	n /= (1000ULL * 1000ULL);
	AAL::btUnsigned64bitInt TimerThree = s + n;

#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 2 * PCFreq;
	    ((LARGE_INTEGER)time_two).QuadPart = 2 * PCFreq;
	    ((LARGE_INTEGER)time_three).QuadPart = 5 * PCFreq;
#endif


	//Test the functionality of == operator.
    EXPECT_TRUE(time_one == time_two);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerOne == TimerTwo);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_one).QuadPart == ((LARGE_INTEGER)time_two).QuadPart);
#endif


    //Test the functionality of != operator.
    EXPECT_TRUE(time_one != time_three);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerOne != TimerThree);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_one).QuadPart != ((LARGE_INTEGER)time_three).QuadPart);
#endif


    //Test the functionality of < operator.
    EXPECT_TRUE(time_one < time_three);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerOne < TimerThree);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_one).QuadPart < ((LARGE_INTEGER)time_three).QuadPart);
#endif


    //Test the functionality of <= operator.
    EXPECT_TRUE(time_one <= time_two);
    EXPECT_TRUE(time_one <= time_three);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerOne <= TimerTwo);
    EXPECT_TRUE(TimerOne <= TimerThree);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_one).QuadPart <= ((LARGE_INTEGER)time_two).QuadPart);
    EXPECT_TRUE(((LARGE_INTEGER)time_one).QuadPart <= ((LARGE_INTEGER)time_three).QuadPart);
#endif


    //Test the functionality of > operator.
    EXPECT_TRUE(time_three > time_one);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerThree > TimerOne);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_three).QuadPart > ((LARGE_INTEGER)time_one).QuadPart);
#endif


    //Test the functionality of >= operator.
    EXPECT_TRUE(time_two >= time_one);
    EXPECT_TRUE(time_three >= time_one);
#if defined( __AAL_LINUX__ )
    EXPECT_TRUE(TimerTwo >= TimerOne);
	EXPECT_TRUE(TimerThree >= TimerOne);
#elif   defined( __AAL_WINDOWS__ )
    EXPECT_TRUE(((LARGE_INTEGER)time_two).QuadPart >= ((LARGE_INTEGER)time_one).QuadPart);
    EXPECT_TRUE(((LARGE_INTEGER)time_three).QuadPart >= ((LARGE_INTEGER)time_one).QuadPart);
#endif
}

TEST_F(TimerBasic, aal0143)
{
	//Tests accuracy of AsSeconds(), AsmilliSeconds(), AsMicroSeconds(), AsNanoseconds().

	Timer time_one;
	AAL::btUnsigned64bitInt result;
	double dResult;

	//Initialise Timer time_one.
#if defined( __AAL_LINUX__ )
	timespec tv;
	tv.tv_sec = 10;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 10 * PCFreq;
#endif

	//Testing functions that accepts argument of AAL::btUnsigned64bitInt datatype
	time_one.AsSeconds(result);
	EXPECT_EQ(result, 10);

	time_one.AsMilliSeconds(result);
	EXPECT_EQ(result, 10000);

	time_one.AsMicroSeconds(result);
	EXPECT_EQ(result, 10000000);

	time_one.AsNanoSeconds(result);
#if defined( __AAL_LINUX__ )
	EXPECT_EQ(result, 10000000250);
#elif   defined( __AAL_WINDOWS__ )
	EXPECT_EQ(result, 10000000000);
#endif

	//Testing functions that accepts argument of double datatype
	time_one.AsSeconds(dResult);
#if defined( __AAL_LINUX__ )
	EXPECT_EQ(dResult, 10.000000249999999);
#elif   defined( __AAL_WINDOWS__ )
	EXPECT_EQ(dResult, 10.00);
#endif

	time_one.AsMilliSeconds(dResult);
#if defined( __AAL_LINUX__ )
	EXPECT_EQ(dResult, 10000.000249999999141);
#elif   defined( __AAL_WINDOWS__ )
	EXPECT_EQ(dResult, 10000.00);
#endif

	time_one.AsMicroSeconds(dResult);
#if defined( __AAL_LINUX__ )
	EXPECT_EQ(dResult, 10000000.250);
#elif   defined( __AAL_WINDOWS__ )
	EXPECT_EQ(dResult, 10000000.00);
#endif

	time_one.AsNanoSeconds(dResult);
#if defined( __AAL_LINUX__ )
	EXPECT_EQ(dResult, 10000000250.00);
#elif   defined( __AAL_WINDOWS__ )
	EXPECT_EQ(dResult, 10000000000.00);
#endif
}

TEST_F(TimerBasic, aal0144)
{
	//Tests accuracy of Now()

	Timer time_one;

	//Initialise Timer time_one with a default value.
#if defined( __AAL_LINUX__ )

	timespec tv;
	tv.tv_sec = 5;
	tv.tv_nsec = 50;
	time_one = Timer(&tv);

#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 2 * PCFreq;
#endif

	//Obtain the current timestamp using the Now() function.
	time_one = time_one.Now();

	//Verify that the difference in time returned by Now() and the OS Timer is within the tolerance level.
#if defined( __AAL_LINUX__ )
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

	EXPECT_TRUE((OSTimer - TimerNow) >= 0 && (OSTimer - TimerNow) <= 10000);

#elif defined ( __AAL_WINDOWS__ )
	//The tolerance level is 1 second.

	 LARGE_INTEGER OS_Now;
	 OS_Now.QuadPart = 0;
	    if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    QueryPerformanceCounter(&OS_Now);

	    double Diff = (double)(((LARGE_INTEGER)Now).QuadPart - OS_Now.QuadPart)/(double)(Timer::sm_ClockFreq.QuadPart);
		EXPECT_TRUE((Diff >= 0) && (Diff <=1));
#endif

}

TEST_F(TimerBasic, aal0145)
{
	//Tests accuracy of NormalisedUnits()

	Timer time_one;
	string NormalisedUnit;
	string units;

	// Initialize a timer with 5000 seconds.
#if defined( __AAL_LINUX__ )
	timespec tv;
	tv.tv_sec = 5000;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 5000 * PCFreq;
#endif

	//Verify that the normalised unit returned by the function is hours.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "Hours";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());

	//Initialize a timer with 100 seconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 100;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 100 * PCFreq;
#endif

	//Verify that the normalised unit returned by the function is minutes.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "Minutes";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());

	//Initialize a timer with 50 seconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 50;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 50 * PCFreq;
#endif

	//Verify that the normalised unit returned by the function is seconds.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "Seconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());

	//Initialize a timer with 2 milliseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2000000;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000;
#endif

	//Verify that the normalised unit returned by the function is milliseconds.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "MilliSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());

	//Initialize a timer with 2 microseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2000;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000000;
#endif

	//Verify that the normalised unit returned by the function is microseconds.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "MicroSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());

	//Initialize a timer with 2 nanoseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000000000;
#endif

	//Verify that the normalised unit returned by the function is nanoseconds.
	NormalisedUnit = time_one.NormalizedUnits();
	units = "NanoSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), units.c_str());
}

TEST_F(TimerBasic, aal0146)
{
	//Tests accuracy of Normalized()

	Timer time_one;
	string NormalisedUnit;
	string ExpectedResult;
	AAL::btUnsigned64bitInt i =0;
	double d = 0.0;

	//Initialize a timer with 3600 seconds.
#if defined( __AAL_LINUX__ )
	timespec tv;
	tv.tv_sec = 3600;
	tv.tv_nsec = 0;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 3600 * PCFreq;
#endif

	//Verify that the normalised result is in hours.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "1 Hour";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 1);
	EXPECT_EQ(d, 1);

	//Initialize a timer with 60 seconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 60;
	tv.tv_nsec = 0;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 60 * PCFreq;
#endif

	//Verify that the normalised result is in minutes.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "1 Minute";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 1);
	EXPECT_EQ(d, 1);

	//Initialize a timer with 50 seconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 50;
	tv.tv_nsec = 250;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = 50 * PCFreq;
#endif

	//Verify that the normalised result is in seconds.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "50 Seconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 50);
	EXPECT_EQ(d, 50.00000025);

	//Initialize a timer with 2 milliseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2000000;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000;
#endif

	//Verify that the normalised result is in milliseconds.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "2 MilliSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 2);
	EXPECT_EQ(d, 2);

	//Initialize a timer with 2 microseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2000;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000000;
#endif

	//Verify that the normalised result is in microseconds.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "2 MicroSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 2);
	EXPECT_EQ(d, 2);

	//Initialize a timer with 2 nanoseconds.
#if defined( __AAL_LINUX__ )
	tv.tv_sec = 0;
	tv.tv_nsec = 2;
	time_one = Timer(&tv);
#elif   defined( __AAL_WINDOWS__ )

	 if ( !QueryPerformanceFrequency(&Timer::sm_ClockFreq) ) {
	      cout << " QueryPerformanceFrequency failed \n";
	    }

	    double PCFreq = (double)(Timer::sm_ClockFreq.QuadPart);

	    ((LARGE_INTEGER)time_one).QuadPart = (2 * PCFreq)/1000000000;
#endif

	//Verify that the normalised result is in nanoseconds.
	NormalisedUnit = time_one.Normalized(NULL, NULL);
	ExpectedResult = "2 NanoSeconds";
	EXPECT_STRCASEEQ(NormalisedUnit.c_str(), ExpectedResult.c_str());

	//Verify that the double and AAL::btUnsigned64bitInt data returned by reference has the expected value.
	NormalisedUnit = time_one.Normalized(&i, &d);
	EXPECT_EQ(i, 2);
	EXPECT_EQ(d, 2);

}
