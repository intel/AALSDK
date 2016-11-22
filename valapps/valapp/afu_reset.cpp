#include "afu_reset.h"
#include "test_manager.h"
#include "afu_client.h"
#include <memory>
#include <aalsdk/service/IALIAFU.h>

namespace
{
    bool r = test_manager::register_test<afu_reset>();
}

using namespace std;
const unsigned int csr_scratchpad0 = 0x0100;
const unsigned int csr_ctl = 0x0137;
const unsigned int ctl_stop = 0x07;

void afu_reset::register_tests()
{
    register_test("SW-RESET-01", &afu_reset::reset);
}

void afu_reset::setup()
{
    auto nlb0 = context()->get_service("NLB0");
    TEST_ERROR(nlb0->status() != service_client::allocated, "Service not allocated");

    afu_ = dynamic_pointer_cast<afu_client>(nlb0);
    TEST_ERROR(!afu_, "Could not cast service_client to afu_client");
    afu_->reset();
    auto value = afu_->mmio_read64(csr_scratchpad0);
    TEST_ERROR(0x0 != value, "Initial reset of AFU did not clear out scratchpad register: ", value);
}

void afu_reset::teardown()
{
    if (afu_)
    {
        afu_->mmio_write32(csr_ctl, ctl_stop);
    }
}

void afu_reset::reset(const arguments &args)
{
    Log() << "SW-RESET-01 test" << std::endl;

    afu_->mmio_write64(csr_scratchpad0, 0x01);
    TEST_FAIL(0x01 != afu_->mmio_read64(csr_scratchpad0), "Did not write to scratchpad 0");

    auto result = afu_->reset();
    TEST_FAIL(result != 0, "reset result is not zero 0: ", result);
    auto value = afu_->mmio_read64(csr_scratchpad0);
    TEST_FAIL(0x0 != value, "Scratchpad0 did not reset to 0. Value is: ", value);
}
