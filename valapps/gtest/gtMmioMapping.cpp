#include "afu_test.h"
#include <iostream>
#include <chrono>
#include <functional>
#include "gtest/gtest.h"
#include "nlb_client.h"

using namespace std;
using namespace std::chrono;
using namespace std::placeholders;
using namespace AAL;

const uint32_t dummy4B = 0x12345678;
const uint64_t dummy8B = 0x1122334455667788;
const uint32_t mmio_size = 256*1024;
const uint32_t sas_mmio_size = 256*1024;
const uint32_t target_feature_offset = 0x38;

class mmio_mapping_f : public test_context, public ::testing::TestWithParam<int>
{
    protected:
        mmio_mapping_f()
        {
        }

        virtual ~mmio_mapping_f()
        {
        }

        virtual void SetUp()
        {
        }

        virtual void TearDown()
        {
        }
};


TEST_F(mmio_mapping_f, sw_mmio_01)
{
    using usec = microseconds;
    using cacheline_t = unsigned char[16];
    
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";

    unsigned long dsm_size = afu_test::constants::MB*4;
    unsigned long buffer_size = afu_test::constants::MB*2;
    mmio_buffer::ptr_t dsm = nlb0->allocate_buffer(dsm_size);
    auto inp = nlb0->allocate_buffer(buffer_size);
    auto out = nlb0->allocate_buffer(buffer_size);

    ::memset(dsm->address(), 0,    dsm_size);
    ::memset(inp->address(), 0xAF, buffer_size);
    ::memset(out->address(), 0xBE, buffer_size);

    uint8_t *cl = inp->address();
    cl[15] = 0xA;

    nlb0->reset();
    // set dsm base, high then low
    nlb0->mmio_write64(static_cast<uint32_t>(nlb_client::dsm::basel), dsm->physical());
    // assert afu reset
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 0);
    // de-assert afu reset
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 1);
    // set input workspace address
    nlb0->mmio_write64(static_cast<uint32_t>(nlb_client::csr::src_addr), CACHELINE_ALIGNED_ADDR(inp->physical()));
    // set output workspace address
    nlb0->mmio_write64(static_cast<uint32_t>(nlb_client::csr::dst_addr), CACHELINE_ALIGNED_ADDR(out->physical()));
    // set number of cache lines for test
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::num_lines), buffer_size/CL(1));
    // set the test mode
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::cfg), 0x200); // Rdline_I

    volatile bt32bitCSR *status_addr =
        (volatile bt32bitCSR*)(dsm->address() + static_cast<uint32_t>(nlb_client::dsm::test_complete));

    // start the test
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 3);

    while ( 0 == ((*status_addr)&0x1)){
        this_thread::sleep_for(usec(10));
    }

    // stop the device
    nlb0->mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 7);

    auto result = memcmp(out->address(), inp->address(), buffer_size);

    dsm->release();
    inp->release();
    out->release();
    ASSERT_EQ(0, result) << "Output does NOT match input, at offset";
}

TEST_P(mmio_mapping_f, sw_mmio_02)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";
    
    uint32_t data = dummy4B;
    uint32_t start_address = 0x0;
    bool status = false;
    uint32_t data32b = 0x0;


    int width = GetParam();
    bool b32 = width == 32;
    auto write32 = std::bind(&afu_client::mmio_write32, nlb0.get(), _1, _2);
    auto write64 = std::bind(&afu_client::mmio_write64, nlb0.get(), _1, _2);

    auto read32 = std::bind(&afu_client::mmio_read32, nlb0.get(), _1);
    auto read64 = std::bind(&afu_client::mmio_read64, nlb0.get(), _1);

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
        ASSERT_EQ(data, write_value) <<  "Data read," << data << " does not match expected,"  << write_value;
    }

    offset = start_address + (width == 32 ? mmio_size : 0x40000);
    for (uint32_t j = 0; j < 20; ++j, offset -= step)
    {
        auto data = b32 ? read32(offset) : read64(offset);
        ASSERT_EQ(data, write_value) <<  "Data read," <<  data << " does not match expected," << write_value;
    }
}

TEST_F(mmio_mapping_f, sw_mmio_03)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";
    auto address = nlb0->mmio_address();
    auto length = nlb0->mmio_length();

    auto overflow = address + length;
    ASSERT_EXIT(nlb0->mmio_write32(length, 0xAA), ::testing::KilledBySignal(SIGSEGV), ".*");
}


TEST_F(mmio_mapping_f, sw_mmio_04)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";

    auto mmio_size = nlb0->mmio_length();
    ASSERT_EQ(sas_mmio_size, mmio_size) << "Reported mmio size from afu, " << mmio_size << " does not match size in SAS, " << sas_mmio_size;

}

TEST_F(mmio_mapping_f, sw_mmio_05)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";
    
    nlb0->reset();
    uint32_t offset;
    uint32_t feature_id = 0x12;
    bool have_offset = nlb0->feature_id_offset(feature_id, offset);
    TEST_FAIL(!have_offset, "Did not get offset for feature: ", hex, feature_id);
    ASSERT_EQ(target_feature_offset, offset) << "Offset read does not match expected offset: " << hex << ", " << target_feature_offset;
}

TEST_F(mmio_mapping_f, sw_mmio_06)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";
    
    nlb0->reset();
    uint32_t feature_type = ALI_DFH_TYPE_BBB;
    uint32_t offset;
    bool have_offset = nlb0->feature_type_offset(feature_type, offset);
    ASSERT_TRUE(have_offset) <<  "Corrupt DFH discovery";
}

TEST_F(mmio_mapping_f, sw_mmio_07)
{
    auto nlb0 = get_service<nlb_client>("NLB0");
    ASSERT_EQ(service_client::allocated, nlb0->status())  << "NLB0 has not been allocated";

    auto mmio = nlb0->get_interface<IALIMMIO>();
    uint32_t offset;
    long long unsigned int data64;
    for(uint64_t j = 0; j < 13; ++j)
    {
        mmio->mmioRead64(offset, &data64);
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
        ASSERT_TRUE(result) << "Failed to get feature address";
        auto fid_result = output.Get(ALI_GETFEATURE_TYPE_KEY, &fid);
        auto ftype_result = output.Get(ALI_GETFEATURE_TYPE_KEY, &ftype);

        ASSERT_TRUE(fid_result) << "Feature id not found in DFH";
        ASSERT_TRUE(ftype_result) << "Feature type not found in DFH";
        ASSERT_EQ(ftype, type_id_pair.first) <<  "Feature type, " <<  ftype << " does not match expected feature type";
        ASSERT_EQ(fid, type_id_pair.second) <<  "Feature id, " << fid << " does not match expected feature id";
    });
}

INSTANTIATE_TEST_CASE_P(mmio_width, mmio_mapping_f, ::testing::Values(32));

