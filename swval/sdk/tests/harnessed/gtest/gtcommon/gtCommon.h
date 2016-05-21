// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_GTCOMMON_H__
#define __GTCOMMON_GTCOMMON_H__
#include <cstdio>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
#include <list>
#include <map>

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
using namespace AAL;

#include "swvalmod.h"
#include "swvalsvcmod.h"

#include "gtest/gtest.h"

#if   defined( __AAL_WINDOWS__ )
# ifdef GTCOMMON_EXPORTS
#    define GTCOMMON_API __declspec(dllexport)
# else
#    define GTCOMMON_API __declspec(dllimport)
# endif // GTCOMMON_EXPORTS
#elif defined( __AAL_LINUX__ )
# define GTCOMMON_API    __declspec(0)
#endif // OS

#include "gtCommon_Config.h"
#include "gtCommon_RNG.h"
#include "gtCommon_Stream.h"
#include "gtCommon_Console.h"
#include "gtCommon_Status.h"
#include "gtCommon_Signals.h"
#include "gtCommon_GTest.h"
#include "gtCommon_Mocks.h"

template <typename X>
X PassReturnByValue(X x) { return x; }

#if defined( __AAL_LINUX__ )

// Make sure that the given path appears in LD_LIBRARY_PATH, preventing duplication.
// Return non-zero on error.
int RequireLD_LIBRARY_PATH(const char *path);

// Remove the given path from LD_LIBRARY_PATH.
// Return non-zero on error.
int UnRequireLD_LIBRARY_PATH(const char *path);

// Print streamer for LD_LIBRARY_PATH.
// Ex.
//   cout << LD_LIBRARY_PATH << endl;
std::ostream & LD_LIBRARY_PATH(std::ostream &os);

#endif // __AAL_LINUX__

#endif // __GTCOMMON_H__

