// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

#if 0
// segfault in srand() for Windows with this version.
RepeatableRandomInt::RepeatableRandomInt(unsigned Seed) :
   m_Seed(Seed),
   m_CallCount(0),
   m_SaveCallCount(0)
{}

void RepeatableRandomInt::Snapshot()
{
   m_SaveCallCount = m_CallCount;
}

void RepeatableRandomInt::Replay()
{
   m_CallCount = m_SaveCallCount;
}

int RepeatableRandomInt::rng()
{
#if   defined( __AAL_WINDOWS__ )
   EnterCriticalSection(&RepeatableRandomInt::sm_Lock);
#elif defined( __AAL_LINUX__ )
   pthread_mutex_lock(&RepeatableRandomInt::sm_Lock);
#endif // OS

   srand(m_Seed);

   unsigned i;
   for ( i = 0 ; i < m_CallCount ; ++i ) {
      rand();
   }

   ++m_CallCount;
   int res = rand();

#if   defined( __AAL_WINDOWS__ )
   LeaveCriticalSection(&RepeatableRandomInt::sm_Lock);
#elif defined( __AAL_LINUX__ )
   pthread_mutex_unlock(&RepeatableRandomInt::sm_Lock);
#endif // OS

   return res;
}

#if   defined( __AAL_WINDOWS__ )
CRITICAL_SECTION RepeatableRandomInt::sm_Lock;
#elif defined( __AAL_LINUX__ )
pthread_mutex_t  RepeatableRandomInt::sm_Lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif // OS
#endif // 0


RepeatableRandomInt::RepeatableRandomInt(unsigned Seed) :
   m_Index(Seed % ( sizeof(RepeatableRandomInt::sm_RandomInts) / sizeof(RepeatableRandomInt::sm_RandomInts[0]) )),
   m_SaveIndex(m_Index)
{}

void RepeatableRandomInt::Snapshot()
{
   m_SaveIndex = m_Index;
}

void RepeatableRandomInt::Replay()
{
   m_Index = m_SaveIndex;
}

int RepeatableRandomInt::rng()
{
   int res = RepeatableRandomInt::sm_RandomInts[m_Index];

   m_Index = (m_Index + 1) % ( sizeof(RepeatableRandomInt::sm_RandomInts) / sizeof(RepeatableRandomInt::sm_RandomInts[0]) );

   return res;
}

int RepeatableRandomInt::sm_RandomInts[100] =
{
   696197551, 208428439, 513744428, 1258760647, 1585152743, 727456137, 1568531506, 44014684, 1480859204, 1296161569,
   1354141040, 345828541, 57410946, 7886591, 435854625, 1624876081, 838447260, 224858288, 387300067, 1876785414,
   1852058204, 374296490, 886806843, 2012752206, 871865560, 618338589, 27288415, 1710486121, 168129410, 1450422743,
   829786575, 279150677, 202782917, 621358136, 1912889897, 1091505917, 1183689041, 1185433760, 1670458059, 1139674357,
   1905521923, 751890022, 854786295, 1559693381, 694479075, 846899703, 1123838756, 1217086642, 8452443, 898980468,
   870608212, 1431313118, 2094869739, 496311721, 544506275, 82117532, 1771929809, 2073651333, 54829117, 61443687,
   881951334, 1687871458, 81067356, 602800657, 1485088541, 1607192868, 837394407, 393582623, 423503827, 1799444295,
   1426826023, 444606128, 975416768, 674936001, 1737303481, 1563207034, 2127940574, 890403778, 439368278, 910853931,
   487958473, 1511158764, 626807698, 1204129003, 1563772673, 130495977, 659622728, 1481655141, 506049816, 733455043,
   768991237, 452238876, 455830828, 1228603427, 371171520, 2000513819, 1890998534, 911462300, 1163119411, 1497415910
};

