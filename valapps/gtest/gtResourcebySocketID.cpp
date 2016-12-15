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

class Resource_SelectbySocketID_f : public test_context, public ::testing::Test
{
    protected:
        Resource_SelectbySocketID_f()
        {
        }

        virtual ~Resource_SelectbySocketID_f()
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

        void Resource_SelectbySocketID_01()
        {
            // Allocate resource by socket ID
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

};

TEST_F(Resource_SelectbySocketID_f, Resource_SelectbySocketID_01)
{
   SCOPED_TRACE("Resource Selection by SocketID_01");
   Resource_SelectbySocketID_01();
}

