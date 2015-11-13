// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __ISOLATED_H__
#define __ISOLATED_H__
#include <iostream>
#include <cstdlib>

#include <aalsdk/AAL.h>
USING_NAMESPACE(AAL);

#if defined( __AAL_LINUX__ )
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <signal.h>
#endif // __AAL_LINUX__

#if   defined( __AAL_WINDOWS__ )
# define cpu_yield()       ::Sleep(0)
# define sleep_millis(__x) ::Sleep(__x)
#elif defined( __AAL_LINUX__ )
# define cpu_yield()       ::usleep(0)
# define sleep_millis(__x) ::usleep((__x) * 1000)
#endif // OS

#define YIELD_WHILE(__predicate) \
do                               \
{                                \
   while ( __predicate ) {       \
      cpu_yield();               \
   }                             \
}while(0)

#define YIELD_X(__x)                                 \
do                                                   \
{                                                    \
   AAL::btUIntPtr       __i;                         \
   const AAL::btUIntPtr __N = (AAL::btUIntPtr)(__x); \
   for ( __i = 0 ; __i < __N ; ++__i ) {             \
      cpu_yield();                                   \
   }                                                 \
}while(0)

void AssertFailed();
void ExpectFailed();

#define _ASSERT_FAILED(__file, __line, __message) \
do                                                \
{                                                 \
   std::cerr << "ASSERT ( "  << __message         \
             << " ) FAILED " << __file << ":"     \
             << __line << std::endl;              \
   AssertFailed();                                \
}while(0)

#define _EXPECT_FAILED(__file, __line, __message) \
do                                                \
{                                                 \
   std::cerr << "EXPECT ( "  << __message         \
             << " ) FAILED " << __file << ":"     \
             << __line << std::endl;              \
   ExpectFailed();                                \
}while(0)



#define _ASSERT_TRUE(__file, __line, __expr)   \
do                                             \
{                                              \
   if ( __expr ) {                             \
      ;                                        \
   } else {                                    \
      _ASSERT_FAILED(__file, __line, #__expr); \
   }                                           \
}while(0)
#define ASSERT_TRUE(__expr) _ASSERT_TRUE(__FILE__, __LINE__, __expr)

#define _EXPECT_TRUE(__file, __line, __expr)   \
do                                             \
{                                              \
   if ( __expr ) {                             \
      ;                                        \
   } else {                                    \
      _EXPECT_FAILED(__file, __line, #__expr); \
   }                                           \
}while(0)
#define EXPECT_TRUE(__expr) _EXPECT_TRUE(__FILE__, __LINE__, __expr)



#define _ASSERT_FALSE(__file, __line, __expr)       \
do                                                  \
{                                                   \
   if ( __expr ) {                                  \
      _ASSERT_FAILED(__file, __line, "! " #__expr); \
   }                                                \
}while(0)
#define ASSERT_FALSE(__expr) _ASSERT_FALSE(__FILE__, __LINE__, __expr)

#define _EXPECT_FALSE(__file, __line, __expr)       \
do                                                  \
{                                                   \
   if ( __expr ) {                                  \
      _EXPECT_FAILED(__file, __line, "! " #__expr); \
   }                                                \
}while(0)
#define EXPECT_FALSE(__expr) _EXPECT_FALSE(__FILE__, __LINE__, __expr)



#define _ASSERT_NULL(__file, __line, __x)              \
do                                                     \
{                                                      \
   if ( (__x) != (void *)0 ) {                         \
      _ASSERT_FAILED(__file, __line, #__x " == NULL"); \
   }                                                   \
}while(0)
#define ASSERT_NULL(__x) _ASSERT_NULL(__FILE__, __LINE__, __x)

#define _EXPECT_NULL(__file, __line, __x)              \
do                                                     \
{                                                      \
   if ( (__x) != (void *)0 ) {                         \
      _EXPECT_FAILED(__file, __line, #__x " == NULL"); \
   }                                                   \
}while(0)
#define EXPECT_NULL(__x) _EXPECT_NULL(__FILE__, __LINE__, __x)




#define _ASSERT_NONNULL(__file, __line, __x)           \
do                                                     \
{                                                      \
   if ( (__x) == (void *)0 ) {                         \
      _ASSERT_FAILED(__file, __line, #__x " != NULL"); \
   }                                                   \
}while(0)
#define ASSERT_NONNULL(__x) _ASSERT_NONNULL(__FILE__, __LINE__, __x)

#define _EXPECT_NONNULL(__file, __line, __x)           \
do                                                     \
{                                                      \
   if ( (__x) == (void *)0 ) {                         \
      _EXPECT_FAILED(__file, __line, #__x " != NULL"); \
   }                                                   \
}while(0)
#define EXPECT_NONNULL(__x) _EXPECT_NONNULL(__FILE__, __LINE__, __x)


#define _ASSERT_EQ(__file, __line, __x, __y)            \
do                                                      \
{                                                       \
   if ( (__x) != (__y) ) {                              \
      _ASSERT_FAILED(__file, __line, #__x " == " #__y); \
   }                                                    \
}while(0)
#define ASSERT_EQ(__x, __y) _ASSERT_EQ(__FILE__, __LINE__, __x, __y)

#define _EXPECT_EQ(__file, __line, __x, __y)            \
do                                                      \
{                                                       \
   if ( (__x) != (__y) ) {                              \
      _EXPECT_FAILED(__file, __line, #__x " == " #__y); \
   }                                                    \
}while(0)
#define EXPECT_EQ(__x, __y) _EXPECT_EQ(__FILE__, __LINE__, __x, __y)



#define _ASSERT_NE(__file, __line, __x, __y)            \
do                                                      \
{                                                       \
   if ( (__x) == (__y) ) {                              \
      _ASSERT_FAILED(__file, __line, #__x " != " #__y); \
   }                                                    \
}while(0)
#define ASSERT_NE(__x, __y) _ASSERT_NE(__FILE__, __LINE__, __x, __y)

#define _EXPECT_NE(__file, __line, __x, __y)            \
do                                                      \
{                                                       \
   if ( (__x) == (__y) ) {                              \
      _EXPECT_FAILED(__file, __line, #__x " != " #__y); \
   }                                                    \
}while(0)
#define EXPECT_NE(__x, __y) _EXPECT_NE(__FILE__, __LINE__, __x, __y)

////////////////////////////////////////////////////////////////////////////////

class ITestFixture
{
public:
   virtual ~ITestFixture() {}

   virtual void SetUp()    = 0;
   virtual void Run()      = 0;
   virtual void TearDown() = 0;
};

#if defined( __AAL_LINUX__ )
BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)
AAL::btUIntPtr DbgOSLThreadCount();
   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)
#endif // __AAL_LINUX__

class TestFixture : public ITestFixture
{
public:
   TestFixture()  {}
   ~TestFixture() {}

   virtual void SetUp()    {}
   virtual void Run()      {}
   virtual void TearDown() {}

   AAL::btUnsignedInt CurrentThreads() const
   {
#if defined( __AAL_LINUX__ )
      return AAL::Testing::DbgOSLThreadCount();
#endif // OS
   }
};

void Execute(ITestFixture * );

#endif // __ISOLATED_H__

