#include "dma_buffer.h"
#include "test_manager.h"
#include "afu_client.h"
#include "mmio_buffer.h"
#include <aalsdk/service/IALIAFU.h>
#include <memory>
#include <thread>
#include <chrono>


bool r = test_manager::register_test<dma_buffer>();

using namespace AAL;
using namespace std;

void dma_buffer::register_tests()
{
    register_test("SW-BUF-01", &dma_buffer::allocate_hold)
                 ("size", 's')
                 ("duration", 'd');
}

void dma_buffer::setup()
{
    afu_ = get_afu_client("NLB0");
}

void dma_buffer::teardown()
{
}

void dma_buffer::allocate_hold(const arguments &args)
{
    Log() << "allocate and hold test" << std::endl;
    auto result = afu_->reset();
    
    TEST_FAIL(result != IALIReset::e_OK, "Could not reset AFU");

    auto size = args.get_int("size");
    auto duration = args.get_int("duration");
    
    mmio_buffer::ptr_t buffer;
    try
    {
        buffer = afu_->allocate_buffer(size);
    }
    catch(buffer_allocate_error &e)
    {
        TEST_FAIL(true, "buffer allocation failed");
    }

    Log() << "Writing to buffer" << std::endl;

    struct CacheLine {
        btUnsigned32bitInt uint[16];
    };

    struct CacheLine* pCacheLine = reinterpret_cast<struct CacheLine*>(buffer->address());
    for (btUnsigned32bitInt i = 0; i < size / CL(1); ++i)
    {
        pCacheLine[i].uint[15] = i;
    }
    Log() << "Holding the buffer for " << duration << std::endl;
    this_thread::sleep_for(chrono::seconds(duration));
    buffer->release();
}
