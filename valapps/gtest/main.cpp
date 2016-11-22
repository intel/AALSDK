#include "gtest/gtest.h"
#include <iostream>
#include "arguments.h"
#include "service_manager.h"
#include "test_manager.h"
using ::testing::TestCase;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
using namespace std;

int main(int argc, char **argv)
{
    int argcount = argc; // save the original argcount
    arguments argparse;
    argparse("services",   's', optional_argument)
            ("testlib",    't', required_argument)
            ("gtest_help", 'H', no_argument);

    bool parsed = argparse.parse(argc, argv, false);

    if (argparse.have("gtest_help"))
    {
        // becasue argparse.parse moifies the arg count
        // use the original arg count in this call
        // this should result in gtest printing help
        ::testing::InitGoogleTest(&argcount, argv);
    }
    else if (!parsed)
    {
        argparse.print_usage(cerr,  "  -- [test arguments] | [gtest arguments]");
        return -1;
    }

    auto testlib = argparse.get_string("testlib");
    if (testlib.substr(0, 3) != "lib")
    {
        testlib = "lib" + testlib;
    }

    if (testlib.find(".so") == string::npos)
    {
        testlib += ".so";
    }

    auto tm = test_manager::instance();
    if (!tm->load_testlib(testlib))
    {
        cerr << "Error loading testlib: " << argparse.get_string("testlib") << endl;
        return 1;
    }

    auto services_config = argparse.get_string("services", "services.json");
    auto sm = service_manager::instance();
    sm->define_services(services_config);


    ::testing::InitGoogleTest(&argc, argv);

    tm->register_arguments(argc, argv);

    int value = RUN_ALL_TESTS();
    sm->shutdown();
    tm->shutdown();
    return value;
}
