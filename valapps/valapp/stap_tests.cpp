#include "stap_tests.h"
#include "test_manager.h"
#include "afu_client.h"
#include <memory>
#include <aalsdk/service/IALIAFU.h>

namespace
{
    bool r = test_manager::register_test<stap_tests>();
}

using namespace std;

const unsigned int mm_debug_link_signature = 0x4170;
const unsigned int mm_debug_link_version = 0x4174;
const unsigned int mm_expected_signature = 0x53797343;
const unsigned int mm_expected_version = 1;

void stap_tests::register_tests()
{
    register_test("SW-STAP-01", &stap_tests::stap01)
                 ("stap-signature", 's', optional_argument)
                 ("stap-version", 'v', optional_argument);
}

void stap_tests::setup()
{

}

void stap_tests::teardown()
{
}

void stap_tests::stap01(const arguments &args)
{
    Log() << "SW-STAP-01 test" << std::endl;
    auto stap = context()->get_service("STAP");
    
    TEST_ERROR(stap->status() != service_client::allocated, "Service not allocated");

    auto afu = dynamic_pointer_cast<afu_client>(stap);    
    TEST_ERROR(!afu, "Could not cast service_client to afu_client");

    auto expected_signature = args.get_int("stap-signature", mm_expected_signature);
    auto expected_version   = args.get_int("stap-version", mm_expected_version);
    auto got_signature      = afu->mmio_read32(mm_debug_link_signature);
    auto got_version        = afu->mmio_read32(mm_debug_link_version);
    TEST_FAIL(expected_signature != got_signature, 
              "STAP signature ", std::hex, got_signature, " did not match expected signature: ", std::hex, expected_signature);
    TEST_FAIL(expected_version < got_version, 
              "STAP version ", std::hex, got_version, " is greater than expected version: ", std::hex, expected_version);
}
