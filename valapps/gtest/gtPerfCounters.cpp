#include <gtest/gtest.h>
#include <vector>
#include "test_context.h"
#include "afu_client.h"
#include "nlb_client.h"
#include <aalsdk/service/IALIAFU.h>

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

class perf_counters_f : public test_context, public ::testing::TestWithParam<int>
{
    protected:
        perf_counters_f()
        {
        }

        virtual ~perf_counters_f()
        {
        }

        virtual void SetUp()
        {
        }

        virtual void TearDown()
        {
        }

        bool validate(const NamedValueSet &lhs, const NamedValueSet &rhs)
        {
            uint32_t value1, value2;
            for (const char* key : perf_keys)
            {
                EXPECT_TRUE(lhs.Get(key, &value1)) <<  key << " not found in NVS1" << endl;
                EXPECT_TRUE(rhs.Get(key, &value2)) <<  key << " not found in NVS2" << endl;
                EXPECT_NE(value1, UINT32_MAX) << "Junk found in perfcounter in lhs for " << key;
                EXPECT_NE(value2, UINT32_MAX) << "Junk found in perfcounter in rhs for " << key;

                if (value1 != value2)
                {
                    return false;
                }
            }
            return true;
        }

        void perfc01()
        {
            auto fme = get_service<afu_client>("FME");
            auto fme_status = fme->status();
            ASSERT_EQ(service_client::status_t::allocated, fme_status);

            auto perf_if = fme->get_interface<IALIPerf>();
            ASSERT_NE(nullptr, perf_if) << "Could not get IALIPerf interface" << endl;

            NamedValueSet nvs1, nvs2, opt_args;
            
            auto success = perf_if->performanceCountersGet(&nvs1);
            ASSERT_TRUE(success) << "Could not get performace counters" << endl;

            success = perf_if->performanceCountersGet(&nvs2, opt_args);
            ASSERT_TRUE(success) << "Could not get performace counters" << endl;

            auto are_equal = validate(nvs1, nvs2);
            ASSERT_TRUE(are_equal) << "Counters don't match" << endl;
        }

        void perfc02()
        {
            auto fme = get_service<afu_client>("FME");
            auto fme_status = fme->status();
            ASSERT_EQ(service_client::status_t::allocated, fme_status);

            auto perf_if = fme->get_interface<IALIPerf>();
            ASSERT_NE(nullptr, perf_if) << "Could not get IALIPerf interface" << endl;

            NamedValueSet nvs1, nvs2;
            auto success = perf_if->performanceCountersGet(&nvs1);
            ASSERT_TRUE(success) << "performanceCountersGet() failed" << endl;
            
            auto nlb = get_service<nlb_client>("NLB0");
            auto lb_success = nlb->loopback1(2, 4);
            ASSERT_TRUE(lb_success) <<  "Loopback1 was unsuccessful" << endl;

            success = perf_if->performanceCountersGet(&nvs2);
            ASSERT_TRUE(success) << "performanceCountersGet() failed" << endl;

            bool are_equal = validate(nvs1, nvs2);
            ASSERT_TRUE(are_equal) <<  "Counters are equal after loopback" << endl;
        }
};

TEST_F(perf_counters_f, sw_perfc_01)
{
    SCOPED_TRACE("sw_perfc_01");
    perfc01();
}

TEST_F(perf_counters_f, sw_perfc_02)
{
    SCOPED_TRACE("sw_perfc_02");
    perfc02();
}

TEST_P(perf_counters_f, sw_perfc_03)
{
    SCOPED_TRACE("sw_perfc_03");
    // randomly select and call test 01/02 N (default to 1000) times
    auto iterations = GetParam();
    utils::random<1,2> rand;
    for (int i = 0; i < iterations; ++i)
    {
        if (rand()%2)
        {
            perfc01(); 
        }
        else
        {
            perfc02();
        }
    }
}

INSTANTIATE_TEST_CASE_P(sw_perfc_03, perf_counters_f, ::testing::Values(1000));
