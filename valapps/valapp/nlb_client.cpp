#include "nlb_client.h"
#include "afu_test.h"
#include <chrono>
#include <functional>
#include "client_factory.h"

using namespace std::chrono;
using namespace std::placeholders;
using namespace AAL;

namespace
{
    static bool b = client_factory::register_client<nlb_client>();
}

nlb_client::nlb_client()
{

}

nlb_client::~nlb_client()
{

}

void nlb_client::setup(uint32_t test_mode, uint32_t dsm_size, uint32_t buffer_size)
{
    dsm_size = dsm_size*afu_test::constants::MB;
    buffer_size = buffer_size*afu_test::constants::MB;
    dsm_ = allocate_buffer(dsm_size);
    inp_ = allocate_buffer(buffer_size);
    out_ = allocate_buffer(buffer_size);


    inp_->write<char>(0xA, 15);

    reset();
    // set dsm base, high then low
    mmio_write64(static_cast<uint32_t>(nlb_client::dsm::basel), dsm_->physical());
    // assert afu reset
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 0);
    // de-assert afu reset
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 1);
    // set input workspace address
    mmio_write64(static_cast<uint32_t>(nlb_client::csr::src_addr), CACHELINE_ALIGNED_ADDR(inp_->physical()));
    // set output workspace address
    mmio_write64(static_cast<uint32_t>(nlb_client::csr::dst_addr), CACHELINE_ALIGNED_ADDR(out_->physical()));
    // set number of cache lines for test
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::num_lines), buffer_size/CL(1));
    // set the test mode
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::cfg), test_mode);
}

bool nlb_client::loopback1( uint32_t dsm_size, uint32_t buffer_size)
{
    using usec = microseconds;
    setup(0x200, dsm_size, buffer_size);

    ::memset(dsm_->address(), 0,    dsm_size);
    ::memset(inp_->address(), 0xAF, buffer_size);
    ::memset(out_->address(), 0xBE, buffer_size);

    volatile bt32bitCSR *status_addr = 
        (volatile bt32bitCSR*)(dsm_->address() + static_cast<uint32_t>(nlb_client::dsm::test_complete));
  
    // start the test
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 3);

    while ( 0 == ((*status_addr)&0x1))
    {
        std::this_thread::sleep_for(usec(10));
    }
    // stop the device
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 7);

    int result = memcmp(out_->address(), inp_->address(), buffer_size);
    return 0 == result;
}

bool nlb_client::read_test(uint32_t dsm_size, uint32_t buffer_size)
{
    setup(0, dsm_size, buffer_size);


    return false;
}



std::future<bool> nlb_client::loopback1_async(uint32_t dsm_size, uint32_t buffer_size)
{
    std::packaged_task<bool(uint32_t, uint32_t)> task(
        std::bind(&nlb_client::loopback1, this, _1, _2));
    task(dsm_size, buffer_size);
    return task.get_future();
}
