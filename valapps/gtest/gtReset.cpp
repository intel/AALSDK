#include "afu_test.h"
#include <iostream>
#include "gtest/gtest.h"
#include "reconfigure_client.h"
#include "process.h"
#include "nlb_client.h"

using namespace std;
using action = reconfigure_client::action_t;

class reset_f : public test_context, public ::testing::Test
{
    protected:
        reset_f()
        {
        }

        virtual ~reset_f()
        {
        }

        virtual void SetUp()
        {
        }
        
        virtual void TearDown()
        {
        }

    private:
        reconfigure_client::ptr_t pr_;
};

TEST_F(reset_f, sw_reset_01)
{
    using csr_t = nlb_client::csr;

    auto nlb0 = get_service<nlb_client>("NLB0");
    auto afu_status = nlb0->status();
    ASSERT_EQ(afu_status, service_client::status_t::allocated) << "NLB0 not allocated" << endl;

    nlb0->mmio_write64(static_cast<unsigned int>(csr_t::scratchpad0), 0x01);
    auto value = nlb0->mmio_read64(static_cast<unsigned int>(nlb_client::csr::scratchpad0));
    EXPECT_EQ(value, 0x01) << "Did not write to scratchpad0" << std::endl;

    auto result = nlb0->reset();
    EXPECT_EQ(0, result) << "Reset result is not zero" << std::endl;
    
    value = 1;
    value = nlb0->mmio_read64(static_cast<unsigned int>(nlb_client::csr::scratchpad0));
    ASSERT_EQ(value, 0x0) << "Scratchpad0 not reset to 0" << std::endl;
}
