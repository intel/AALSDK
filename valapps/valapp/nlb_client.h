#include <aalsdk/AALTypes.h>
#include <aalsdk/service/IALIAFU.h>
#include "afu_client.h"
#include <thread>
#include "service_manager.h"

class nlb_client : public afu_client
{
    public:
        typedef std::tuple<uint32_t, uint32_t, uint32_t, uint8_t*, uint8_t*> cmdq_entry_t;
        typedef std::deque<cmdq_entry_t> cmdq_t;

        enum class csr : uint32_t
        {
            scratchpad0 = 0x0100,
            ctl         = 0x0138,
            cfg         = 0x0140,
            num_lines   = 0x0130,
            src_addr    = 0x0120,
            dst_addr    = 0x0128,
            cmdq_sw     = 0x0190,
            cmdq_hw     = 0x0194
        };

        enum class dsm : uint32_t
        {
            basel = 0x0110,
            baseh = 0x0114,
            test_complete = 0x0040,
            test_error = 0x0044,
            num_clocks = 0x0048,
            num_reads = 0x0050,
            num_writes = 0x0054,
            start_overhead = 0x0058,
            end_overhead = 0x005c
        };

        enum class test_mode_t : uint32_t
        {
            loopback   = 0x00,
            cont       = 0x02,
            read       = 0x04,
            write      = 0x08,
            throughput = 0x0c
        };

        struct run_stat
        {
            uint32_t iteration;
            uint32_t test_mode;
            bool     passed;
            uint32_t dsm_errors;
            uint32_t dsm_clocks;
            uint32_t dsm_reads;
            uint32_t dsm_writes;
            uint32_t dsm_start;
            uint32_t dsm_end;
            uint32_t rdhit;
            uint32_t wrhit;
            uint32_t rdmiss;
            uint32_t wrmiss;
            double   read_bw;
            double   write_bw;
        };

        const uint32_t dsm_size = 2048;

        nlb_client();

        virtual ~nlb_client();

        virtual void release();

        void configure(const std::string & test_mode, const std::string &config_file);

        void enable_perf();

        void setup(uint32_t test_mode, uint32_t dsm_size, uint32_t buffer_size);

        bool loopback1( uint32_t dsm_size, uint32_t buffer_size );

        std::future<bool> loopback1_async( uint32_t dsm_size, uint32_t buffer_size );

        bool read_test( uint32_t dsm_size, uint32_t buffer_size);

        
        void copy_command(const cmdq_entry_t &entry);
        void command_queue(uint32_t cache_lines, uint32_t allocations);
        void command_queue(const std::string &input);
        bool verify(const cmdq_entry_t &entry);

        void do_commands(cmdq_t &fifo);
        uint32_t hwvalid_loop(cmdq_t &fifo1, cmdq_t &fifo2);
        uint32_t swvalid_loop(cmdq_t &fifo1, cmdq_t &fifo2);
        bool wait_for_done(uint32_t allocations);
        /// @brief wait for ctrl bits to be set
        bool wait_for_register(uint32_t offset, uint32_t mask, uint32_t value);
        const std::vector<run_stat> & stats() const
        {
            return stats_;
        };

        void calc_bw();
        void write_stats(std::ostream & stream);
        void write_summary(std::ostream & stream);
        void save_stat(uint32_t iteration, bool passed);
    private:
        void add_option(uint32_t opt);
        void add_option(test_mode_t opt);
        void add_option(const std::string &opt);
        uint32_t cfg_;
        double timeout_;
        bool continuous_;
        uint32_t frequency_;
        std::atomic_bool cancel_;
        std::thread timeout_thread_;
        AAL::IALIPerf *perf_;
        std::vector<run_stat> stats_;
        run_stat summary_;
        mmio_buffer::ptr_t dsm_;
        mmio_buffer::ptr_t inp_;
        mmio_buffer::ptr_t out_;

};

std::ostream& operator << (std::ostream& os, const nlb_client::run_stat & stat)
{
    os << stat.iteration  << ", ";
    os << stat.test_mode  << ", ";
    os << stat.passed     << ", ";
    os << stat.dsm_errors << ", ";
    os << stat.dsm_clocks << ", ";
    os << stat.dsm_reads  << ", ";
    os << stat.dsm_writes << ", ";
    os << stat.read_bw    << ", ";
    os << stat.write_bw   << ", ";
    os << stat.rdhit      << ", ";
    os << stat.wrhit      << ", ";
    os << stat.rdmiss     << ", ";
    os << stat.wrmiss;
}
