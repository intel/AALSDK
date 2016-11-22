#include "perf_counters_tests.h"
#include "test_manager.h"
#include "utils.h"
#include "afu_client.h"
#include "nlb_client.h"
#include <aalsdk/service/IALIAFU.h>
#include <memory>
#include <random>

namespace
{
    bool r = test_manager::register_test<perf_counters_tests>();
}

using namespace std;
using namespace AAL;

static vector<const char*> perf_keys =
{
    AALPERF_VERSION,
    AALPERF_READ_HIT,
    AALPERF_WRITE_HIT,
    AALPERF_READ_MISS,
    AALPERF_WRITE_MISS,
    AALPERF_EVICTIONS,
    AALPERF_PCIE0_READ,
    AALPERF_PCIE0_WRITE,
    AALPERF_PCIE1_READ,
    AALPERF_PCIE1_WRITE,
    AALPERF_UPI_READ,
    AALPERF_UPI_WRITE
};

void perf_counters_tests::register_tests()
{
    register_test("SW-PERFC-01", &perf_counters_tests::perfc01);
    register_test("SW-PERFC-02", &perf_counters_tests::perfc02);
    register_test("SW-PERFC-03", &perf_counters_tests::perfc03)
                 ("interations", 'i');
}

void perf_counters_tests::setup()
{
    nlb0_ = context()->get_service<afu_client>("NLB0");
    fme_ = context()->get_service<afu_client>("FME");
}

void perf_counters_tests::teardown()
{

}

void perf_counters_tests::perfc01(const arguments &args)
{
    Log() << "SW-PERFC-01 test" << std::endl;

    auto perf_if = fme_->get_interface<IALIPerf>();
    TEST_FAIL(!perf_if, "Could not get IALIPerf interface");

    NamedValueSet nvs1, nvs2, opt_args;
    auto success = perf_if->performanceCountersGet(&nvs1);
    TEST_FAIL(!success, "performanceCountersGet() failed");

    success = perf_if->performanceCountersGet(&nvs2, opt_args);
    TEST_FAIL(!success, "performanceCountersGet() failed");

    auto are_equal = validate(nvs1, nvs2);
    TEST_FAIL(!are_equal,"Counters don't match.");

}

void perf_counters_tests::perfc02(const arguments &args)
{
    Log() << "SW-PERFC-02 test" << std::endl;
    
    auto perf_if = fme_->get_interface<IALIPerf>();
    TEST_FAIL(!perf_if, "Could not get IALIPerf interface");
    
    NamedValueSet nvs1, nvs2;
    auto success = perf_if->performanceCountersGet(&nvs1);
    TEST_FAIL(!success, "performanceCountersGet() failed");
    
    auto nlb = context()->get_service<nlb_client>("NLB0");
    auto lb_success = nlb->loopback1(2, 4);
    TEST_ERROR(!lb_success, "Loopback1 was unsuccessful");

    success = perf_if->performanceCountersGet(&nvs2);
    TEST_FAIL(!success, "performanceCountersGet() failed");

    bool are_equal = validate(nvs1, nvs2);
    TEST_FAIL(are_equal, "Counters are equal after loopback");
}

void perf_counters_tests::perfc03(const arguments &args)
{
    Log() << "SW-PERFC-03 test" << std::endl;
    // randomly select and call test 01/02 N (default to 1000) times
    auto iterations = args.get_int("iterations", 1000);
    utils::random<1,2> rand;
    for (int i = 0; i < iterations; ++i)
    {
        if (rand()%2)
        {
            perfc01(args); 
        }
        else
        {
            perfc02(args);
        }
    }
}

bool perf_counters_tests::validate(const NamedValueSet &lhs, const NamedValueSet &rhs)
{
    uint32_t value1, value2;
    for (auto key : perf_keys)
    {
        TEST_FAIL(!lhs.Get(key, &value1), key, " not found in NVS1");
        TEST_FAIL(!rhs.Get(key, &value2), key, " not found in NVS2");
        TEST_FAIL(value1 == UINT32_MAX, " junk found in perfcounter for ", key);
        TEST_FAIL(value2 == UINT32_MAX, " junk found in perfcounter for ", key);

        if (value1 != value2)
        {
            return false;
        }
    }
    return true;
}
