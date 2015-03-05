// INTEL CONFIDENTIAL - For Intel Internal Use Only

// generic Google Test application main().
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <cstdio>
#include <string>
#include <fstream>
#include <list>
#include <map>

#include <aalsdk/AAL.h>
#include <aalsdk/xlRuntime.h>
using namespace AAL;

#include "gtest/gtest.h"
#include "cases/gtCommon.cpp"

#if defined( TEST_SUITE_BAT )
# include "BAT-TEST-LIST.h"
void Version() { cout << "bat 1.1.1" << endl; }
#elif defined( TEST_SUITE_NIGHTLY )
# include "NIGHTLY-TEST-LIST.h"
void Version() { cout << "nightly 1.1.1" << endl; }
#elif defined( TEST_SUITE_WEEKLY )
# include "WEEKLY-TEST-LIST.h"
void Version() { cout << "weekly 1.1.1" << endl; }
#endif // test suite

int main(int argc, char *argv[])
{
   if ( ( argc > 1 ) &&
        ( 0 == std::string(argv[1]).compare("--version") ) ) {
      Version();
      return 0;
   }

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

