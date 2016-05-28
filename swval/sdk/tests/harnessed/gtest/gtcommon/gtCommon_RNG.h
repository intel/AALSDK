// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_RNG_H__
#define __GTCOMMON_RNG_H__

// Sort an array of arbitrarily-typed objects.
template <typename X>
void MySort(X *p, AAL::btUnsignedInt n)
{
   // Selection Sort
   AAL::btUnsignedInt i;
   AAL::btUnsignedInt j;
   AAL::btUnsignedInt k;

   for ( i = 0 ; i < n ; ++i ) {
      k = i;
      for ( j = i + 1 ; j < n ; ++j ) {
         if ( *(p + j) < *(p + k) ) {
            k = j;
         }
      }
      std::swap(*(p + i), *(p + k));
   }
}

#if 0
template <typename X>
X UniqueIntRand(X *p, AAL::btUnsignedInt n, X mod, AAL::btUnsigned32bitInt *R)
{
   X                  res;
   AAL::btBool        unique;
   AAL::btUnsignedInt i;

   do
   {
      res = (X) AAL::GetRand(R);
      if ( mod > 0 ) {
         res %= mod;
      }

      unique = true;
      for ( i = 0 ; i < n ; ++i ) {
         if ( res == *(p + i) ) {
            unique = false;
            break;
         }
      }

   }while( !unique );

   return res;
}
#endif // 0

/*

In Linux, the thread-safe Random Number Generator int rand_r(unsigned int *seedp) uses
the data value stored at the input parameter as an input seed for generating the next
number. With this implementation of rand_r(), the sequence of random numbers is
repeatable.

  eg.
   unsigned s;
   int      r;

   s = 3;
   r = rand_r(&s); // say that r == 5
   r = rand_r(&s); // say that r == 1
   r = rand_r(&s); // say that r == 2
   ...
   s = 3;
   r = rand_r(&s); // we expect r == 5
   r = rand_r(&s); // we expect r == 1
   r = rand_r(&s); // we expect r == 2

This behavior enables snapshot and replay, which is essential for test cases that
rely on random numbers.

In Windows, the thread-safe RNG errno_t rand_s(unsigned int *randomValue) does not
consider the data value stored at the input parameter and does not produce a repeatable
sequence.

Because of this lack of repeatability in the RNG sequence as create by rand_s(), we
fall back to wrapping synchronization around srand() and rand() to enable repeatable
RNG sequences in both Linux and Windows.

*/

#if 0
// This implementation uses srand() and rand(), but there was a segfault in
// Windows when calling srand() repeatedly. Needs further debugging.

class RepeatableRandomInt
{
public:
   RepeatableRandomInt(unsigned Seed);

   void Snapshot();
   void Replay();

   int rng();

protected:
   unsigned m_Seed;
   unsigned m_CallCount;
   unsigned m_SaveCallCount;

#if   defined( __AAL_WINDOWS__ )
   static CRITICAL_SECTION sm_Lock;
#elif defined( __AAL_LINUX__ )
   static pthread_mutex_t  sm_Lock;
#endif // OS
};
#endif // 0

// This implementation cheats by creating a fixed sequence of random numbers, but
// it gets the job done.
class GTCOMMON_API RepeatableRandomInt
{
public:
   RepeatableRandomInt(unsigned Seed);

   void Snapshot();
   void Replay();

   int rng();

protected:
   unsigned m_Index;
   unsigned m_SaveIndex;

   static int sm_RandomInts[100];
};

#endif // __GTCOMMON_RNG_H__

