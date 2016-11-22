#include <aalsdk/AALTypes.h>
#include <aalsdk/service/IALIAFU.h>
#include "afu_client.h"
#include <thread>
#include "service_manager.h"

class nlb_client : public afu_client
{
    public:
        enum class csr : uint32_t
        {
            scratchpad0 = 0x0100,
            ctl         = 0x0138,
            cfg         = 0x0140,
            num_lines   = 0x0130,
            src_addr    = 0x0120,
            dst_addr    = 0x0128
        };

        enum class dsm : uint32_t
        {
            basel = 0x0110,
            baseh = 0x0114,
            test_complete = 0x040
        };

        nlb_client();

        virtual ~nlb_client();

        void setup(uint32_t test_mode, uint32_t dsm_size, uint32_t buffer_size);

        bool loopback1( uint32_t dsm_size, uint32_t buffer_size );

        std::future<bool> loopback1_async( uint32_t dsm_size, uint32_t buffer_size );

        bool read_test( uint32_t dsm_size, uint32_t buffer_size);

    private:
        mmio_buffer::ptr_t dsm_;
        mmio_buffer::ptr_t inp_;
        mmio_buffer::ptr_t out_;

};
