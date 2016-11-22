#include "mmio_mapping.h"
#include "test_manager.h"
#include "afu_client.h"

#include <chrono>
#include <thread>
#include <cstdint>

namespace
{
    bool r = test_manager::register_test<mmio_mapping>();
}

using namespace AAL;
using namespace std;
using namespace std::chrono;
using namespace std::placeholders;

const int32_t csr_ctl = 0x0137,
              csr_cfg = 0x0140,
              csr_num_lines = 0x0130,
              csr_src_addr = 0x0120,
              csr_dst_addr = 0x0128,
              csr_dsm_basel = 0x0110,
              csr_dsm_baseh = 0x0114,
              dsm_status_test_complete = 0x40;

const uint32_t dummy4B = 0x12345678;
const uint64_t dummy8B = 0x1122334455667788;
const uint32_t mmio_size = 256*1024;
const uint32_t sas_mmio_size = 256*1024;
const uint32_t target_feature_offset = 0x38;


struct CacheLine {                           // Operate on cache lines
     btUnsigned32bitInt uint[16];
};

void mmio_mapping::register_tests()
{
    register_test("SW-MMIO-01", &mmio_mapping::mmio_write_onecl)
                 ("service-name", 's', optional_argument);
    register_test("SW-MMIO-02", &mmio_mapping::mmio_whole_region)
                 ("width", 'w', required_argument);
    register_test("SW-MMIO-03", &mmio_mapping::mmio_bounds_check)
                 ("service-name", 's', optional_argument);
    register_test("SW-MMIO-04", &mmio_mapping::mmio_length)
                 ("mmio-size", 'S');
    register_test("SW-MMIO-05", &mmio_mapping::dfh_feature);
    register_test("SW-MMIO-06", &mmio_mapping::corrupt_dfh);
    register_test("SW-MMIO-07", &mmio_mapping::mmio_stress);
}

void mmio_mapping::setup()
{
    nlb0_ = get_afu_client("NLB0");
}

void mmio_mapping::teardown()
{

}

void mmio_mapping::mmio_write_onecl(const arguments &args)
{
    using usec = microseconds;

    Log() << "MMIO 1 CL Transfer " << std::endl;

    unsigned long dsm_size = constants::MB*4;
    unsigned long buffer_size = constants::MB*2;
    mmio_buffer::ptr_t dsm = nlb0_->allocate_buffer(dsm_size);
    auto inp = nlb0_->allocate_buffer(buffer_size);
    auto out = nlb0_->allocate_buffer(buffer_size);

    ::memset(dsm->address(), 0,    dsm_size);
    ::memset(inp->address(), 0xAF, buffer_size);
    ::memset(out->address(), 0xBE, buffer_size);

    struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(inp->address());
    pCL[0].uint[15] = 0xA;

    nlb0_->reset();
    // set dsm base, high then low
    nlb0_->mmio_write64(csr_dsm_basel , dsm->physical());
    // assert afu reset
    nlb0_->mmio_write32(csr_ctl, 0);
    // de-assert afu reset
    nlb0_->mmio_write32(csr_ctl, 1);
    // set input workspace address
    nlb0_->mmio_write64(csr_src_addr, CACHELINE_ALIGNED_ADDR(inp->physical()));
    // set output workspace address
    nlb0_->mmio_write64(csr_dst_addr, CACHELINE_ALIGNED_ADDR(out->physical()));
    // set number of cache lines for test
    nlb0_->mmio_write32(csr_num_lines, buffer_size/CL(1));
    // set the test mode
    nlb0_->mmio_write32(csr_cfg, 0x200); // Rdline_I

    volatile bt32bitCSR *status_addr =
        (volatile bt32bitCSR*)(dsm->address() + dsm_status_test_complete);

    // start the test
    nlb0_->mmio_write32(csr_ctl, 3);

    while ( 0 == ((*status_addr)&0x1)){
        this_thread::sleep_for(usec(10));
    }

    Log() << "Done running test" << std::endl;
    // stop the device
    nlb0_->mmio_write32(csr_ctl, 7);

    auto result = memcmp(out->address(), inp->address(), buffer_size);

    dsm->release();
    inp->release();
    out->release();
    TEST_FAIL(0 != result, "Output does NOT match input, at offset");
}

void mmio_mapping::mmio_whole_region(const arguments& args)
{
    uint32_t data = dummy4B;
    uint32_t start_address = 0x0;
    bool status = false;
    uint32_t data32b = 0x0;


    auto width = args.get_int("width", 32);
    bool b32 = width == 32;
    auto write32 = std::bind(&afu_client::mmio_write32, nlb0_.get(), _1, _2);
    auto write64 = std::bind(&afu_client::mmio_write64, nlb0_.get(), _1, _2);

    auto read32 = std::bind(&afu_client::mmio_read32, nlb0_.get(), _1);
    auto read64 = std::bind(&afu_client::mmio_read64, nlb0_.get(), _1);

    auto write_value = width == 32 ? dummy4B : dummy8B;
    uint32_t step = width == 32 ? 4 : 8;
    uint32_t offset = 0x0;
    for (uint32_t j = 0; j < mmio_size/step; ++j, offset+=step)
    {
        b32 ? write32(offset, write_value) :
              write64(offset, write_value);
    }

    offset = start_address;
    for (uint32_t j = 0; j < 20; ++j, offset += step)
    {
        auto data = b32 ? read32(offset) : read64(offset);
        Log() << "0x" << hex <<  ": " << data << endl;
        TEST_FAIL(data != write_value, "Data read,", data,
                  " does not match expected,", write_value);
    }

    offset = start_address + (width == 32 ? mmio_size : 0x40000);
    for (uint32_t j = 0; j < 20; ++j, offset -= step)
    {
        auto data = b32 ? read32(offset) : read64(offset);
        Log() << "0x" << hex <<  ": " << data << endl;
        TEST_FAIL(data != write_value, "Data read,", data,
                  " does not match expected,", write_value);
    }
}

void mmio_mapping::mmio_bounds_check(const arguments& args)
{
    TEST_FAIL(service_client::allocated != nlb0_->status(),
              "NLB0 has not been allocated");

    auto address = nlb0_->mmio_address();
    auto length = nlb0_->mmio_length();

    auto overflow = address + length;
    EXPECT_SIGNAL(SIGSEGV, nlb0_->mmio_write32(length, 0xAA));
}


void mmio_mapping::mmio_length(const arguments& args)
{
    TEST_FAIL(service_client::allocated != nlb0_->status(),
              "NLB0 has not been allocated");

    auto mmio_size = nlb0_->mmio_length();
    auto sas_size = args.get_int("mmio-size", sas_mmio_size);
    TEST_FAIL(sas_size != mmio_size,
              "Reported mmio size from afu, ", mmio_size,
              " does not match size in SAS, ", sas_size)

}

void mmio_mapping::dfh_feature(const arguments& args)
{
    nlb0_->reset();
    uint32_t offset;
    uint32_t feature_id = 0x12;
    bool have_offset = nlb0_->feature_id_offset(feature_id, offset);
    TEST_FAIL(!have_offset, "Did not get offset for feature: ", hex, feature_id);
    TEST_FAIL(target_feature_offset != offset,
              "Offset read does not match expected offset: ", hex, target_feature_offset);
}

void mmio_mapping::corrupt_dfh(const arguments& args)
{
    nlb0_->reset();
    uint32_t feature_type = ALI_DFH_TYPE_BBB;
    uint32_t offset;
    bool have_offset = nlb0_->feature_type_offset(feature_type, offset);
    TEST_FAIL(!have_offset, "Corrupt DFH discovery");
}

void mmio_mapping::mmio_stress(const arguments& args)
{
    auto mmio = nlb0_->get_interface<IALIMMIO>();
    uint32_t offset;
    long long unsigned int data64;
    for(uint64_t j = 0; j < 13; ++j)
    {
        mmio->mmioRead64(offset, &data64);
        Log() << "0x" << hex << offset << " " << hex << data64 << endl;
    }

    vector<pair<uint32_t, uint32_t>> type_id_list =
    { make_pair(ALI_DFH_TYPE_AFU,     0xA),
      make_pair(ALI_DFH_TYPE_BBB,     0x12),
      make_pair(ALI_DFH_TYPE_PRIVATE, 0xBB) };

    for_each(type_id_list.begin(),
             type_id_list.end(),
             [mmio](pair<uint32_t, uint32_t> type_id_pair)
    {
        uint32_t ftype, fid;
        unsigned char* feature_address;
        NamedValueSet request, output;
        request.Add(ALI_GETFEATURE_TYPE_KEY,
                    static_cast<ALI_GETFEATURE_TYPE_DATATYPE> (type_id_pair.first));
        auto result = mmio->mmioGetFeatureAddress(&feature_address, request, output);
        TEST_FAIL(!result, "Failed to get feature address");
        auto fid_result = output.Get(ALI_GETFEATURE_TYPE_KEY, &fid);
        auto ftype_result = output.Get(ALI_GETFEATURE_TYPE_KEY, &ftype);

        TEST_FAIL(fid_result, "Feature id not found in DFH");
        TEST_FAIL(ftype_result, "Feature type not found in DFH");
        TEST_FAIL(ftype != type_id_pair.first, "Feature type, ", ftype,
                  " does not match expected feature type");
        TEST_FAIL(fid != type_id_pair.second, "Feature id, ", fid,
                  " does not match expected feature id");
    });
}
